# SysY2026 RV64GC Compiler

面向 2026 编译系统实现赛道的 SysY 编译器。前端使用 Flex/Bison，输出遵循
RISC-V psABI LP64D 的 RV64GC 汇编，地址模型兼容 `-mcmodel=medany`。

## 提交构建

提交环境只需 Clang 18，不需要安装 Flex、Bison 或第三方库：

```sh
./build.sh
./compiler input.sysy -S -o output.s
./compiler input.sysy -S -o output.s -O1
```

`build.sh` 直接调用 `clang++ --std=c++17 -O2 -lm`。仓库保留 `.l`、`.y`
源文件和已生成的 C++ 文件，比赛构建使用后者。

## 开发构建

```sh
cmake -S . -B build
cmake --build build -j 2
ctest --test-dir build --output-on-failure
```

本机完整验收使用 Spike/pk 和 `riscv64-unknown-elf-gcc`：

```sh
python3 tests/run_public_suite.py build/compiler \
  docs/compiler2026-main/2026初赛RISCV赛道功能用例.zip \
  --spike /opt/homebrew/bin/spike --pk /path/to/pk --timeout 120

python3 tests/run_public_suite.py build/compiler \
  docs/compiler2026-main/2026初赛RISCV赛道性能用例.zip \
  --spike /opt/homebrew/bin/spike --pk /path/to/pk \
  --optimize --embedded-input --timeout 120
```

当前验证结果：功能样例 `-O0` 为 140/140，`-O1` 为 140/140；性能样例
`-O1` 为 60/60。四项核心源码覆盖率分别为 region 88.61%、函数 97.50%、
行 91.50%、分支 81.21%。本机 Spike 性能套件总运行时间由 371.14 秒降至
306.70 秒，其中 `conv2d-1` 由 22.34 秒降至约 13.07 秒。

设计与合规说明见 [design.md](docs/design.md)、[AI_USAGE.md](docs/AI_USAGE.md)
和 [THIRD_PARTY.md](docs/THIRD_PARTY.md)。2026 向量语法尚未正式发布，当前版本
不猜测其语法或语义。
