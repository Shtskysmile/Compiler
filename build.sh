#!/usr/bin/env sh
set -eu

exec clang++ --std=c++17 -O2 -lm \
  -Iinclude -Isrc/third_party -Isrc \
  src/main.cpp src/compiler.cpp src/ir.cpp src/frontend/parse.cpp src/rv64.cpp \
  src/generated/sysy_parser.cpp src/generated/sysy_scanner.cpp \
  -o compiler
