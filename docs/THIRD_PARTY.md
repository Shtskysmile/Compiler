# 第三方代码与生成物

## Flex

`src/generated/sysy_scanner.cpp` 由 Flex 2.6.4 根据
`src/frontend/scanner.l` 生成。`src/third_party/FlexLexer.h` 来自 Flex 2.6.4，
版权归 The Regents of the University of California，按 BSD-2-Clause 条款再分发；
文件内保留版权、再分发条件和免责声明。

## Bison

`src/generated/sysy_parser.cpp`、`src/generated/sysy_parser.hpp` 和
`src/generated/location.hh` 由 GNU Bison 根据 `src/frontend/parser.y` 生成。
生成骨架包含 Bison 的特别例外条款，允许将该骨架生成的较大作品按作品自身条款分发。

## 参考工程

`extra_code/Compiler` 不链接、不复制进提交编译器，也不由根目录 CMake 构建。项目仅
参考其诊断、控制流和后端模块划分思路；根目录实现为独立代码。
