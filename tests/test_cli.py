#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import tempfile


def run(command):
    return subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def main():
    compiler = pathlib.Path(sys.argv[1])
    with tempfile.TemporaryDirectory(prefix="sysy-cli-") as directory:
        root = pathlib.Path(directory)
        source = root / "input.sysy"
        output = root / "output.s"
        source.write_text("int main(){return 7;}\n", encoding="utf-8")

        missing = run([str(compiler)])
        assert missing.returncode != 0
        assert "missing input file" in missing.stderr

        no_assembly = run([str(compiler), str(source), "-o", str(output)])
        assert no_assembly.returncode != 0
        assert "-S option is required" in no_assembly.stderr

        no_output = run([str(compiler), str(source), "-S"])
        assert no_output.returncode != 0
        assert "missing -o output path" in no_output.stderr

        dangling_output = run([str(compiler), str(source), "-S", "-o"])
        assert dangling_output.returncode != 0
        assert "requires an output path" in dangling_output.stderr

        multiple = run([str(compiler), str(source), str(source), "-S", "-o", str(output)])
        assert multiple.returncode != 0
        assert "multiple input files" in multiple.stderr

        unreadable = run([str(compiler), str(root / "missing.sy"), "-S", "-o", str(output)])
        assert unreadable.returncode != 0
        assert "cannot read input file" in unreadable.stderr

        invalid = run([str(compiler), str(source), "-S", "-o", str(output), "--bad"])
        assert invalid.returncode != 0
        assert not output.exists()

        success = run([str(compiler), "-O1", str(source), "-o", str(output), "-S"])
        assert success.returncode == 0, success.stderr
        assembly = output.read_text(encoding="utf-8")
        assert ".globl main" in assembly
        assert "li a0, 7" in assembly

        source.write_text("int main(){ return unknown; }\n", encoding="utf-8")
        failed = run([str(compiler), str(source), "-S", "-o", str(output)])
        assert failed.returncode != 0
        assert "input.sysy:1:" in failed.stderr
        assert "unknown" in failed.stderr
        assert output.read_text(encoding="utf-8") == assembly

        source.write_text("int main(){return 0;}\n", encoding="utf-8")
        directory_output = run([str(compiler), str(source), "-S", "-o", str(root)])
        assert directory_output.returncode != 0
        assert "cannot replace output file" in directory_output.stderr


if __name__ == "__main__":
    main()
