# 编译器设计说明

## 架构

```text
SysY source
  -> Flex scanner
  -> Bison LALR(1) parser
  -> source-located typed AST
  -> semantic checks and constant evaluation
  -> RV64 lowering and assembly emission
```

`include/sysy/compiler.hpp` 是可测试的编译入口。`src/main.cpp` 只负责 CLI、
有界文件读取、诊断打印和原子输出替换。`src/frontend/scanner.l` 与
`src/frontend/parser.y` 是可维护的前端源文件，`src/generated` 是提交构建使用的
生成结果。`src/internal.hpp` 定义 AST、源码位置和资源限制，`src/rv64.cpp` 完成
名字解析、类型检查、常量求值、栈布局和汇编生成。

`extra_code/Compiler` 不参与构建，也未被修改；它只用于参考诊断、CFG 和后端的
组织方式。

## 前端与语义

扫描器实现 C 风格行/块注释、十/八/十六进制整数、十进制与十六进制浮点数、
字符串及精确行列定位。Bison 文法直接表达 C 优先级和最近 `else` 规则，生成时
无 shift/reduce 或 reduce/reduce 冲突。

AST 区分 `int32`、`float32`、`void`、数组和数组参数指针。后端建立嵌套作用域，
预声明所有用户函数及 SysY 运行库函数，并检查声明点可见性、重复声明、函数签名、
`main`、返回语句、可修改左值和循环控制语句。

数组采用行主序。初始化器按子数组边界递归对齐，省略部分补零；全局初始化要求
常量表达式，局部数组先清零再写入非零元素。数组参数保留剩余维度，因此支持子数组
传参和任意维下标。

## RV64GC 后端

`src/ir.hpp` 和 `src/ir.cpp` 提供独立类型化三地址 IR，包含虚拟值、基本块、CFG、
整数/浮点/指针类型和显式循环描述。其 pass 已实现常量与拷贝传播、代数化简、
不可达块删除、迭代 DCE、局部 CSE、纯运算 LICM，以及整数/浮点分池的线性扫描。

生产代码生成仍处于渐进迁移阶段：控制流和大部分内存操作继续从类型化 AST 直接降低，
表达式结果必要时保存在函数固定栈帧临时槽中。逻辑 `&&`、`||`、`!` 以控制流生成，
保持短路和副作用顺序。IR 核心已经可独立测试，但尚未接管所有函数的最终 RV64 发射。

ABI 规则如下：

- 整数和指针形参使用 `a0-a7`，浮点形参使用 `fa0-fa7`。
- 寄存器耗尽后使用 8 字节调用栈槽，调用边界保持 16 字节对齐。
- `int` 使用 `addw/subw/mulw/divw/remw` 和 32 位 load/store。
- `float` 使用单精度 F 扩展指令，隐式转换使用 `fcvt`。
- 可变参数浮点提升为 `double`，按 psABI 通过整数寄存器或 8 字节栈槽传递。
- 大栈帧、数组偏移和调用区不假设 12 位立即数可容纳。
- 全局对象发射到 `.data`/`.bss`，字符串发射到 `.rodata`；地址使用 `lla` 的
  PC 相对 medany 重定位。
- `starttime()`/`stoptime()` 降为带源码行号的 `_sysy_starttime`/
  `_sysy_stoptime`。只导出正常 `main`，启动流程由赛事 `crt0.S` 管理。

## O1

`-O0` 仅执行正确性所需的常量求值和规范化。`-O1` 当前启用：

- 通用常量折叠及对函数内从未赋值标量的保守传播；
- 标量参数和局部变量驻留 `s1-s11`/`fs0-fs11`，并按 psABI 保存恢复；
- 简单数组地址融合、2 的幂步长移位化和紧邻匿名临时值转发；
- 有节点预算、实参单次求值保护的小型叶函数内联；
- 无残留调用函数中只读全局标量的受控循环外提。

IR 层还提供 DCE/CSE/LICM 和线性扫描实现及单元测试。完整函数级 AST→IR 降低和让
IR 分配结果直接驱动所有表达式发射仍在迁移中。所有现行优化依据语义与 use-def 条件，
不读取文件名、函数名、参数规模或测试输入。

## 可靠性

编译器限制源文件为 64MiB、标识符为 1MiB、AST 约五百万节点、单对象为 2GiB。
所有诊断使用 `file:line:column: error: message`。编译失败不会创建或覆盖目标汇编；
成功结果先写入同目录唯一临时文件，再原子替换目标。

测试包括 C++ 单元测试、Python CLI 集成测试、140 个官方功能样例双优化级运行、
60 个官方性能样例 O1 运行和 RV64GC 汇编器检查。大输入运行测试可把 `.in` 作为
只读段链接进测试 ELF，以规避 Spike/pk PTY 无背压导致的丢字节；真实 PTY 路径仍由
字符流、JSON 和分块边界用例覆盖。
