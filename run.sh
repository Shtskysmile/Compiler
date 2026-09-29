#!/usr/bin/env sh
set -eu

usage() {
  echo "usage: bash run.sh <source.sysy>" >&2
  exit 2
}

if [ "$#" -ne 1 ]; then
  usage
fi

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE=$1
if [ ! -f "$SOURCE" ]; then
  echo "run.sh: source file not found: $SOURCE" >&2
  exit 1
fi

CROSS_CC=${CROSS_CC:-riscv64-unknown-elf-gcc}
SPIKE=${SPIKE:-spike}

if ! command -v "$CROSS_CC" >/dev/null 2>&1; then
  echo "run.sh: RISC-V compiler not found: $CROSS_CC" >&2
  exit 1
fi
if ! command -v "$SPIKE" >/dev/null 2>&1; then
  echo "run.sh: Spike not found: $SPIKE" >&2
  exit 1
fi

if [ -z "${PK:-}" ]; then
  if command -v pk >/dev/null 2>&1; then
    PK=$(command -v pk)
  elif [ -x /opt/homebrew/opt/riscv-pk/riscv64-unknown-elf/bin/pk ]; then
    PK=/opt/homebrew/opt/riscv-pk/riscv64-unknown-elf/bin/pk
  else
    echo "run.sh: RISC-V pk not found; set PK=/path/to/pk" >&2
    exit 1
  fi
fi
if [ ! -x "$PK" ]; then
  echo "run.sh: pk is not executable: $PK" >&2
  exit 1
fi

WORK=$(mktemp -d "${TMPDIR:-/tmp}/sysy-run.XXXXXX")
trap 'rm -rf "$WORK"' EXIT HUP INT TERM
ASSEMBLY="$WORK/program.s"
EXECUTABLE="$WORK/program.elf"

echo "[1/4] Building compiler"
(cd "$ROOT" && ./build.sh)

echo "[2/4] Compiling $SOURCE"
"$ROOT/compiler" "$SOURCE" -S -o "$ASSEMBLY" -O1

echo "[3/4] Linking RISC-V executable"
"$CROSS_CC" \
  -static \
  -march=rv64gc \
  -mabi=lp64d \
  -mcmodel=medany \
  "$ASSEMBLY" \
  -x c "$ROOT/tests/rv_runtime.c.test" \
  -x none \
  -lm \
  -o "$EXECUTABLE"

echo "[4/4] Running with Spike"
set +e
"$SPIKE" "$PK" "$EXECUTABLE"
STATUS=$?
set -e
echo
echo "[run.sh] exit code: $STATUS"
exit "$STATUS"
