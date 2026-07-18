#!/usr/bin/env python3
import argparse
import concurrent.futures
import contextlib
import hashlib
import pathlib
import subprocess
import tempfile
import zipfile


def compile_one(compiler, item, optimize, output_dir, assembler):
    name, source = item
    stem = hashlib.sha256(name.encode("utf-8")).hexdigest()[:16]
    source_path = output_dir / f"{stem}.sy"
    asm_path = source_path.with_suffix(".s")
    source_path.write_bytes(source)
    command = [str(compiler), str(source_path), "-S", "-o", str(asm_path)]
    if optimize:
        command.append("-O1")
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        return name, result.stderr.strip()
    text = asm_path.read_text(encoding="utf-8")
    if ".globl main" not in text or "x86" in text:
        return name, "generated assembly is incomplete"
    if assembler:
        object_path = asm_path.with_suffix(".o")
        assembled = subprocess.run(
            [assembler, "-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany",
             "-c", str(asm_path), "-o", str(object_path)],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if assembled.returncode != 0:
            return name, assembled.stderr.strip()
    return name, ""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("compiler", type=pathlib.Path)
    parser.add_argument("archive", type=pathlib.Path)
    parser.add_argument("--optimize", action="store_true")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--assembler")
    parser.add_argument("--case", action="append", dest="cases")
    parser.add_argument("--output-dir", type=pathlib.Path)
    args = parser.parse_args()
    args.compiler = args.compiler.resolve()

    with zipfile.ZipFile(args.archive) as archive:
        names = [name for name in archive.namelist() if name.endswith(".sy")]
        if args.cases:
            selected = set(args.cases)
            names = [name for name in names if name in selected]
        items = [(name, archive.read(name)) for name in names]
    failures = []
    directory_context = (contextlib.nullcontext(str(args.output_dir.resolve()))
                         if args.output_dir else tempfile.TemporaryDirectory(prefix="sysy-suite-"))
    with directory_context as directory:
        output_dir = pathlib.Path(directory)
        output_dir.mkdir(parents=True, exist_ok=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
            futures = [executor.submit(compile_one, args.compiler, item, args.optimize,
                                       output_dir, args.assembler)
                       for item in items]
            for future in concurrent.futures.as_completed(futures):
                name, error = future.result()
                if error:
                    failures.append((name, error))
    if failures:
        for name, error in sorted(failures):
            print(f"{name}: {error}")
        raise SystemExit(f"{len(failures)} of {len(items)} sources failed")
    print(f"compiled {len(items)} sources from {args.archive.name}")


if __name__ == "__main__":
    main()
