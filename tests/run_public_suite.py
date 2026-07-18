#!/usr/bin/env python3
import argparse
import hashlib
import os
import pathlib
import pty
import select
import signal
import subprocess
import tempfile
import termios
import time
import zipfile


def normalize_newlines(data):
    while b"\r\n" in data:
        data = data.replace(b"\r\n", b"\n")
    return data


def run_spike(command, input_data, timeout):
    pid, master = pty.fork()
    if pid == 0:
        os.execv(command[0], command)
    attributes = termios.tcgetattr(master)
    attributes[3] &= ~(termios.ECHO | termios.ICANON)
    attributes[6][termios.VMIN] = 1
    attributes[6][termios.VTIME] = 0
    termios.tcsetattr(master, termios.TCSANOW, attributes)
    os.set_blocking(master, False)
    output = bytearray()
    pending = memoryview(input_data + b"\x04")
    sent = 0
    deadline = time.monotonic() + timeout
    write_ready_at = time.monotonic() + 1.5
    try:
        # Spike configures the controlling terminal during startup and flushes pending input.
        # Read output immediately, but defer writes until that initialization has settled.
        status = None
        while status is None:
            if time.monotonic() >= deadline:
                os.kill(pid, signal.SIGKILL)
                os.waitpid(pid, 0)
                raise subprocess.TimeoutExpired(command, timeout)
            write_list = ([master] if sent < len(pending) and
                          time.monotonic() >= write_ready_at else [])
            readable, writable, _ = select.select([master], write_list, [], 0.05)
            if writable:
                try:
                    sent += os.write(master, pending[sent:sent + 256])
                    time.sleep(0.02)
                except BlockingIOError:
                    pass
                except OSError:
                    sent = len(pending)
            if readable:
                try:
                    chunk = os.read(master, 65536)
                except OSError:
                    chunk = b""
                if chunk:
                    output.extend(chunk)
            waited, wait_status = os.waitpid(pid, os.WNOHANG)
            if waited == pid:
                status = wait_status
        while True:
            readable, _, _ = select.select([master], [], [], 0)
            if not readable:
                break
            try:
                chunk = os.read(master, 65536)
            except OSError:
                break
            if not chunk:
                break
            output.extend(chunk)
    finally:
        os.close(master)
    return os.waitstatus_to_exitcode(status), bytes(output), b""


def execute_case(args, archive_path, name, source, input_data, expected, root):
    stem = hashlib.sha256(name.encode("utf-8")).hexdigest()[:16]
    source_path = root / f"{stem}.sy"
    asm_path = root / f"{stem}.s"
    elf_path = root / f"{stem}.elf"
    source_path.write_bytes(source)
    compile_command = [str(args.compiler), str(source_path), "-S", "-o", str(asm_path)]
    if args.optimize:
        compile_command.append("-O1")
    compiled = subprocess.run(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if compiled.returncode:
        return name, "compiler failed: " + compiled.stderr.decode(errors="replace")

    link_command = [args.cross_cc, "-static", "-march=rv64gc", "-mabi=lp64d",
                    "-mcmodel=medany", str(asm_path), str(args.runtime)]
    if args.embedded_input:
        input_path = root / f"{stem}.in"
        embedded_path = root / f"{stem}_input.S"
        input_path.write_bytes(input_data)
        embedded_path.write_text(
            '.section .rodata\n.globl __sysy_input_start\n.globl __sysy_input_end\n'
            f'__sysy_input_start:\n  .incbin "{input_path}"\n'
            '__sysy_input_end:\n  .byte 0\n', encoding="ascii")
        link_command.extend(["-DSYSY_EMBEDDED_INPUT", str(embedded_path)])
    link_command.extend(["-lm", "-o", str(elf_path)])
    linked = subprocess.run(
        link_command,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if linked.returncode:
        return name, "link failed: " + linked.stderr.decode(errors="replace")

    try:
        returncode, stdout, stderr = run_spike(
            [str(args.spike), str(args.pk), str(elf_path)],
            b"" if args.embedded_input else input_data, args.timeout)
    except subprocess.TimeoutExpired:
        return name, f"execution exceeded {args.timeout} seconds"
    actual = normalize_newlines(stdout)
    if actual and not actual.endswith(b"\n"):
        actual += b"\n"
    actual += f"{returncode & 255}\n".encode()
    expected = normalize_newlines(expected)
    if actual != expected:
        return name, f"output mismatch\nexpected={expected[:500]!r}\nactual={actual[:500]!r}\nstderr={stderr[:500]!r}"
    return name, ""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("compiler", type=pathlib.Path)
    parser.add_argument("archive", type=pathlib.Path)
    parser.add_argument("--cross-cc", default="riscv64-unknown-elf-gcc")
    parser.add_argument("--spike", type=pathlib.Path, default=pathlib.Path("spike"))
    parser.add_argument("--pk", type=pathlib.Path, required=True)
    parser.add_argument("--runtime", type=pathlib.Path, default=pathlib.Path(__file__).with_name("rv_runtime.c"))
    parser.add_argument("--optimize", action="store_true")
    parser.add_argument("--embedded-input", action="store_true",
                        help="link case input into the test runtime instead of using a PTY")
    parser.add_argument("--jobs", type=int, default=1,
                        help="reserved for non-PTY runners; Spike execution requires 1")
    parser.add_argument("--timeout", type=int, default=20)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--case", action="append", dest="cases")
    parser.add_argument("--prefix")
    args = parser.parse_args()
    if args.jobs != 1:
        parser.error("Spike PTY execution is process-global and requires --jobs 1")
    args.compiler = args.compiler.resolve()
    args.runtime = args.runtime.resolve()
    args.spike = args.spike.resolve()
    args.pk = args.pk.resolve()

    with zipfile.ZipFile(args.archive) as archive:
        names = [name for name in archive.namelist() if name.endswith(".sy")]
        if args.prefix:
            names = [name for name in names if name.startswith(args.prefix)]
        if args.cases:
            selected = set(args.cases)
            names = [name for name in names if name in selected]
            missing = selected.difference(names)
            if missing:
                parser.error("archive does not contain: " + ", ".join(sorted(missing)))
        if args.limit:
            names = names[:args.limit]
        cases = []
        for name in names:
            base = name[:-3]
            input_name = base + ".in"
            output_name = base + ".out"
            cases.append((name, archive.read(name),
                          archive.read(input_name) if input_name in archive.namelist() else b"",
                          archive.read(output_name)))

    failures = []
    with tempfile.TemporaryDirectory(prefix="sysy-run-") as directory:
        root = pathlib.Path(directory)
        for case in cases:
            name, error = execute_case(args, args.archive, *case, root)
            if error:
                failures.append((name, error))
    if failures:
        for name, error in sorted(failures):
            print(f"{name}: {error}")
        raise SystemExit(f"{len(failures)} of {len(cases)} cases failed")
    print(f"executed {len(cases)} cases from {args.archive.name}")


if __name__ == "__main__":
    main()
