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
`-O1` 为 60/60。四项核心源码覆盖率分别为 region 90.82%、函数 98.82%、
行 92.70%、分支 83.70%。本机完整性能套件墙钟时间（含 60 次编译、链接和
Spike 启动）为 233.70 秒；代表性单用例为 `many_mat_cal-1` 20.43 秒、
`knapsack_naive-1` 8.87 秒。

`-O1` 会对有符号二次幂常量乘除和取模进行强度削弱，并执行局部数组加载
CSE、全局数组基址缓存、循环不变地址外提和多维数组指针归纳。无调用循环
优先使用 `a4-a7` 保存短生命周期地址，跨调用区间使用 `s` 寄存器；直接递归
函数不缓存全局数组基址，避免扩大每层递归栈帧。寄存器规划以声明位置区分
同名局部变量，使不重叠生命周期可以复用物理寄存器。

官方资料、公开用例和裸机运行库保存在本地 `docs/`、`runtime/` 目录，不纳入 Git。
2026 向量语法尚未正式发布，当前版本不猜测其语法或语义。

## 进一步完成的

尝试完成强度削弱以及寄存器优化。
