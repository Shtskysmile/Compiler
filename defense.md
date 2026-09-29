# SysY2026 RV64GC Compiler — 答辩文档

## 项目概述

本项目是为 **2026 年全国大学生计算机系统能力大赛（华为毕昇杯）编译系统实现赛道** 开发的一款 SysY2026 编译器，目标平台为 **RISC-V RV64GC**。编译器从零构建，前端使用 Flex/Bison，后端直接生成符合 RISC-V psABI LP64D 规范、兼容 `-mcmodel=medany` 地址模型的 RV64GC 汇编代码。---

## 系统架构

```
SysY 源文件 (.sysy)
    │
    ▼
┌──────────────────────────────────┐
│  Flex 扫描器 (scanner.l)          │
│  → Token 流（精确行列定位）         │
└──────────────┬───────────────────┘
               │
    ▼
┌──────────────────────────────────┐
│  Bison LALR(1) 解析器 (parser.y)  │
│  → 类型化 AST（零语法冲突）          │
└──────────────┬───────────────────┘
               │
    ▼
┌──────────────────────────────────┐
│  语义分析                         │
│  · 嵌套作用域管理                  │
│  · 类型检查 / 隐式转换              │
└──────────────┬───────────────────┘
               │
    ▼
┌──────────────────────────────────┐
│  IR 寄存器规划 (ir.cpp)            │
│  · 源级 use-def 虚拟 IR 构建       │
│  · 常量传播 / 拷贝传播              │
│  · 代数化简 / 局部 CSE             │
│  · 迭代 DCE                       │
│  · 线性扫描寄存器分配               │
└──────────────┬───────────────────┘
               │
    ▼
┌──────────────────────────────────┐
│  RV64GC 代码生成 (rv64.cpp)      │
│  · O1: 规划驱动的寄存器驻留       │
│  · O1: 纯函数分析 / 内联          │
│  · O1: 循环优化（LICM/地址外提/归 │
│    纳变量指针化）                 │
│  · O1: 强度削弱 / 立即数折叠      │
│  · O1: 尾调用消除                 │
│  · O1: 临时值转发（后处理）       │
│  → 汇编输出 (.s)                 │
└──────────────────────────────────┘
```

---

## 实现的关键模块

### 1. 前端：Flex/Bison 扫描与解析

- Flex 扫描器规则（`scanner.l`），支持 C 风格注释、十进制/八进制/十六进制整数、十进制与十六进制浮点数、字符串字面量，并提供精确的行列定位。
- Bison LALR(1) 文法（`parser.y`），直接表达 C 语言优先级的表达式规则和最近 `else` 匹配规则，**生成时无 shift/reduce 和 reduce/reduce 冲突**。
- 构建了完整的带源码位置的类型化 AST，支持 `int`/`float`/`void`/多维数组/数组参数指针。

### 2. 语义分析

- 嵌套作用域管理：局部变量遮蔽全局变量，同名变量生命周期不重叠时允许复用。
- 类型检查：赋值左右值类型检查、数组下标类型检查、函数调用实参与形参数量和类型匹配。
- 隐式类型转换：`int→float`。
- 常量表达式求值：支持编译期整型和浮点型二元/一元运算，短路求值，正确处理除法边界情况（除零、INT32_MIN/-1）。
- 数组初始化器展平：递归处理嵌套初始化器列表，按行主序对齐子数组边界，省略部分自动补零。

### 3. 独立的类型化三地址 IR 子系统

在 `ir.hpp`/`ir.cpp` 中实现了一个完整的中间表示子系统：

- **数据结构**：虚拟值、基本块、控制流图、整数/浮点/指针类型、显式循环描述。
- **优化 Pass 链**：常量与拷贝传播、代数化简（x+0→x、x*1→x 等）、不可达块删除、迭代死代码消除、局部公共子表达式消除、循环不变量外提（LICM）。
- **寄存器分配**：整数/浮点分池的线性扫描分配器。
- **IR 可独立测试**：所有 Pass 均有单元测试覆盖边界情况。

### 4. RV64GC 后端（rv64.cpp，约 3066 行）

完整实现从 AST 到 RV64GC 汇编的代码生成：

- **ABI 合规**：整数参数使用 `a0-a7`，浮点参数使用 `fa0-fa7`；寄存器耗尽后使用 8 字节调用栈槽；调用边界保持 16 字节对齐；可变参数中浮点提升为 `double` 并按 psABI 传递。
- **指令选择**：`int` 使用 32 位指令（`addw/subw/mulw/divw/remw`）；`float` 使用 F 扩展单精度指令；隐式转换使用 `fcvt` 系列指令。
- **内存布局**：全局对象发射到 `.data`/`.bss`，字符串发射到 `.rodata`；地址使用 `lla` 伪指令生成 PC 相对 medany 重定位。
- **大栈帧处理**：超过 12 位立即数范围时使用 `li`+`add` 序列生成地址。
- **运行库对接**：预声明全部 13 个 SysY 运行时函数；`starttime()`/`stoptime()` 降为带源码行号的 `_sysy_starttime`/`_sysy_stoptime`。

### 5. O1 优化系统

O1 开启的所有优化基于语义与 use-def 条件，**不读取文件名、函数名、参数规模或测试输入特征**，符合竞赛规则。

---

## 关键创新点

### 创新一：IR 寄存器规划驱动的源代码级分配

这是本项目最重要的架构创新。传统编译器的寄存器分配发生在后端（如 LLVM 的 regalloc pass），但本项目在 AST 生成代码之前，**预先构建每个函数的虚拟 IR**，通过分析源代码中的变量声明位置（区分同名但不同作用域的局部变量）和使用模式来生成精确的寄存器分配计划。

具体流程（`ir::plan_source_registers`）：

1. 遍历 AST 的语句树，在 IR 中为每个标量局部变量创建虚拟值；
2. 记录每个变量在循环体中的使用，正确传播到外层循环；
3. 对生成的虚拟 IR 执行常量传播、拷贝传播、代数化简、局部 CSE 和死代码消除；
4. **使用线性扫描算法为清理后的 IR 分配物理寄存器**；

   **线性扫描原理**：将每个变量的虚拟值记录为一个"活跃区间"（从定义指令到最后一次被使用的指令），按定义时间排序后逐个扫描。扫描时维护一个"当前活跃"列表——当扫描到某个区间时，先释放已结束的区间占用的寄存器，再从空闲寄存器中分配。如果寄存器不够，则溢出到栈。

   以 `accumulate` 函数为例，SourceRegisterPlanner 为其构造的虚拟 IR（简化）是：

   ```
   pos 1: param n (value 1)
   pos 2: declare i (value 2) — 使用 n 的占位 Store
   pos 3: declare sum (value 3)
   pos 4: while body 中使用 i (Store value 2)
   pos 5: 使用 sum (Store value 3)
   pos 6: 使用 i (Store value 2)
   pos 7: while body 中使用 i (Store value 2)
   pos 8: 使用 i (Store value 2)
   pos 9: while body 中使用 i (Store value 2)
   pos 10: 使用 sum (Store value 3)
   ```

   经 use-def 分析后每个变量的活跃区间为：

   | 变量 | 虚拟值 | 活跃区间                          |
   | ---- | ------ | --------------------------------- |
   | n    | 1      | [1, 1] — 仅在参数声明处使用*     |
   | i    | 2      | [2, 9] — 从声明到循环结束        |
   | sum  | 3      | [3, 10] — 从声明到循环结束后返回 |

   三个变量的活跃区间完全重叠，因此需要 3 个不同寄存器。线性扫描按定义时间扫描，将 n→reg0, i→reg1, sum→reg2，映射到物理寄存器就是 s1/s2/s3。
5. 生成的 `RegisterPlan` 直接驱动后端的 `assign_scalar_register`，决定每个源代码变量驻留在哪个物理寄存器（s1-s11 或 fs0-fs11）。

这一设计的核心价值在于：**它在编译的早期阶段就做出了寄存器分配的决策，使得后端可以在生成代码时直接使用寄存器操作而非栈操作**，显著减少了不必要的 load/store。该方案通过声明位置区分同名局部变量，使不重叠生命周期的变量可以复用同一物理寄存器，最大化 `s`/`fs` 保存寄存器的利用率。

**实例对比（`accumulate` 函数）：**

输入 SysY 代码：

```c
int accumulate(int n) {
  int i = 0, sum = 0;
  while (i < n) { sum = sum + i; i = i + 1; }
  return sum;
}
```

O0 生成的循环体（全部通过栈操作，每次迭代 6 次 load/store）：

```asm
.Laccumulate_while_body_1:
  addi t0, s0, -44       # 取 sum 地址
  sd t0, -64(s0)          # 存地址到栈
  lw t0, -44(s0)          # 加载 sum
  lw t1, -28(s0)          # 加载 i
  addw t0, t0, t1         # sum + i
  sw t0, -72(s0)           # 存到临时槽
  ld t0, -64(s0)          # 加载 sum 地址
  lw t1, -72(s0)           # 加载临时值
  sw t1, 0(t0)            # 写回 sum
  ...                     # i = i + 1 同样走栈
```

O1 生成的循环体（IR 规划将 n→s1, i→s2, sum→s3，全部寄存器操作）：

```asm
.Laccumulate_while_body_1:
  addw s3, s3, s2         # sum = sum + i（单条指令）
  addiw s2, s2, 1          # i = i + 1（单条指令）
.Laccumulate_while_test_0:
  blt s2, s1, .Laccumulate_while_body_1  # 条件分支
```

循环体从 O0 的 ~18 条指令缩减为 O1 的 3 条，栈操作完全消除。

**对应代码位置：**

| 文件                       | 行号                         | 说明                                                             |
| -------------------------- | ---------------------------- | ---------------------------------------------------------------- |
| `src/ir.cpp:380-587`     | `SourceRegisterPlanner` 类 | 为每个函数构建虚拟 IR、分析 use-def、优化后线性扫描输出分配计划  |
| `src/ir.cpp:578-587`     | `plan_source_registers()`  | 入口函数，遍历所有函数返回`RegisterPlan` 映射                  |
| `src/rv64.cpp:213`       | Generator 构造函数           | O1 时调用`ir::plan_source_registers(program_, 11, 12)`         |
| `src/rv64.cpp:1961-2008` | `assign_scalar_register()` | 后端根据`RegisterPlan` 为变量分配物理寄存器（s1-s11/fs0-fs11） |
| `src/rv64.cpp:2899-2908` | `emit_function()`          | 函数编译时加载该函数的`RegisterPlan`                           |

### 创新二：跨过程纯函数分析与多层优化联动

`analyze_pure_functions` 实现了跨过程纯函数判定：

1. 首先分析每个函数的函数体：检查是否仅对局部标量赋值、是否仅访问只读全局标量、所有调用的被调用者是否也是纯函数；
2. 然后进行**迭代不动点传播**：如果某个被调用者被排除出纯函数集合，则重新检查其所有调用者，直到集合稳定。

纯函数分析的结果驱动了多项优化：

- **循环不变纯函数调用外提**：在 `while` 循环的零次跳越检查之后、循环体之前，一次性求值纯函数调用，避免每次迭代重复计算。
- **只读全局标量缓存**：将无调用函数中频繁访问的全局标量缓存在 `s`/`fs` 寄存器中，跨循环复用。
- **小型叶函数内联**：将满足节点预算（<48 个 AST 节点）、实参求值安全的叶函数内联到调用处，避免调用开销。

**实例 1：小型叶函数内联**

输入代码中 `calc(1, 2, 3)` 在 O1 下被内联展开，O0 需要 `call calc`，O1 直接在 `main` 中计算：

```asm
# O1: calc 的 a+b*c 直接展开在 main 中，无需 call
  li t0, 3
  li t1, 2
  mulw t0, t0, t1      # t0 = 2 * 3
  mv t1, t0
  li t1, 1
  addw t0, t0, t1       # t0 = 1 + 6
  mv a0, t0
```

**实例 2：循环不变纯函数调用外提**

输入代码：

```c
int square(int x) { return x * x; }
int main() {
  int i = 0, sum = 0;
  while (i < 10) { sum = sum + square(5); i = i + 1; }
  return sum;
}
```

O1 生成的关键汇编——`square(5)` 被识别为纯函数调用且参数均为常量/不变量，外提到循环之前（在零次跳越检查之后）：

```asm
.Lmain_tail_entry:
  li s1, 0              # i = 0
  li s2, 0              # sum = 0
  slti t0, s1, 10       # 零次跳越检查
  beqz t0, .Lmain_while_done_2
  li t0, 5
  sw t0, -40(s0)
  lw t0, -40(s0)
  lw t1, -40(s0)
  mulw t0, t0, t1       # square(5) = 25，仅计算一次
  sw t0, -56(s0)
  j .Lmain_while_body_1
.Lmain_while_body_1:
  lw t0, -56(s0)        # 每次迭代直接复用外提的结果
  addw s2, s2, t0
  addiw s1, s1, 1
  ...
```

而 O0 每次迭代都会执行 `call square`。

**对应代码位置：**

| 文件                       | 行号                                    | 说明                                              |
| -------------------------- | --------------------------------------- | ------------------------------------------------- |
| `src/rv64.cpp:608-653`   | `analyze_pure_functions()`            | 迭代不动点算法判定所有纯函数                      |
| `src/rv64.cpp:538-557`   | `analyze_pure_expression()`           | 分析表达式是否纯（不访问可变全局/不调用非纯函数） |
| `src/rv64.cpp:583-606`   | `analyze_pure_statement()`            | 分析语句是否纯（仅对局部标量赋值）                |
| `src/rv64.cpp:2500-2518` | `collect_invariant_pure_calls()`      | 收集循环内纯调用表达式                            |
| `src/rv64.cpp:2961-2983` | `emit_stmt()` While 分支              | 将纯调用求值移到循环前                            |
| `src/rv64.cpp:3120-3171` | `prepare_readonly_global_registers()` | 无调用函数中缓存全局标量到 s/fs 寄存器            |
| `src/rv64.cpp:1799-1815` | `inline_return_expression()`          | 判断函数是否可内联（单 return + 节点预算 <48）    |
| `src/rv64.cpp:1868-1954` | `emit_inline_call()`                  | 执行内联：绑定实参到形参名，直接求值返回表达式    |

### 创新三：循环指针归纳变量优化

针对 `while` 循环中常见的数组遍历模式，实现了从整数索引计算到指针增量步进的转换：

1. **识别归纳变量**：检测循环中 `i = i + 1` 形式的单位增量变量（仅修改且仅按 1 递增/递减）。
2. **计算地址增量**：对于 `arr[i]`、`arr[i][j]` 等下标表达式，按数组维度和步长计算每次迭代的字节偏移量（如 `arr[i]` 每次迭代偏移 4 字节，`arr[i][j]` 偏移 4×dim 字节）。
3. **分配循环指针寄存器**：为这些地址分配专用指针寄存器（无调用循环优先使用 `a4-a7`，否则使用 `s` 寄存器）。
4. **生成增量代码**：每次迭代通过 `addi` 指令直接更新指针，而非每次重新计算基址+下标×步长。

特别重要的是**多维数组指针归纳**——对 `values[k][j]` 这样的二维访问，正确推导出每次 k 增加 1 时地址增加 `16×4=64` 字节，并在增量指令中直接编码该偏移。

**实例对比（一维数组遍历 `sum += values[i]`）：**

输入 SysY 代码：

```c
int values[64];
int main() {
  int i = 0, sum = 0;
  while (i < 64) { sum = sum + values[i]; i = i + 1; }
  return sum;
}
```

O0 生成的循环体（每次迭代重新计算 `base + i*4`，含多次 store/load）：

```asm
.Lmain_while_body_1:
  addi t0, s0, -36        # 取 sum 地址
  sd t0, -56(s0)           # 存栈
  lla t0, values           # 每次迭代加载 values 基址
  lw t1, -20(s0)           # 加载 i
  slli t1, t1, 2           # i * 4
  add t0, t0, t1           # 计算地址
  lw t1, 0(t0)             # 加载 values[i]
  lw t0, -36(s0)           # 加载 sum
  addw t0, t0, t1          # sum + values[i]
  sw t0, -64(s0)            # 存临时值
  ld t0, -56(s0)           # 加载 sum 地址
  lw t1, -64(s0)            # 加载临时值
  sw t1, 0(t0)             # 写回 sum
  # ... i = i + 1 同样需要
```

O1 生成的循环体——指针归纳优化后，一次计算基址存入 `a4`，每次迭代仅用 `addi a4, a4, 4` 更新：

```asm
  lla s3, values           # 循环外一次性加载基址
  ...
  mv t0, s3
  mv t1, s1                # i
  slli t1, t1, 2           # i * 4
  add t0, t0, t1           # 计算初始地址
  mv a4, t0                # 存入指针寄存器
  # sysy-loop-induction-address
  j .Lmain_while_test_0
.Lmain_while_body_1:
  mv t0, a4                # 直接用指针寄存器
  lw t1, 0(t0)             # 加载 values[i]（一次 lw）
  addw s2, s2, t1          # sum += values[i]（i→s1, sum→s2）
  addiw s1, s1, 1          # i = i + 1
  addi a4, a4, 4           # 指针前进 4 字节（替代 slli+add+lla）
.Lmain_while_test_0:
  slti t0, s1, 64
  bnez t0, .Lmain_while_body_1
```

循环体从 O0 的 ~14 条指令缩减为 O1 的 4 条，`lla` + `slli` + `add` 的地址重计算被单条 `addi` 替代。

**对应代码位置：**

| 文件                       | 行号                                        | 说明                                                          |
| -------------------------- | ------------------------------------------- | ------------------------------------------------------------- |
| `src/rv64.cpp:2671-2692` | `loop_unit_induction()`                   | 识别循环的归纳变量（`i = i + 1` 形式）                      |
| `src/rv64.cpp:2639-2650` | `is_unit_increment()`                     | 判断表达式是否为`name + 1` 或 `1 + name`                  |
| `src/rv64.cpp:2652-2669` | `analyze_induction_updates()`             | 验证循环体对归纳变量的修改模式（恰好一次赋值 + 一次增量）     |
| `src/rv64.cpp:2711-2736` | `induction_address_advance()`             | **核心**：推导 `arr[i][j]` 在 i 递增 1 时的字节偏移量 |
| `src/rv64.cpp:2738-2759` | `collect_induction_addresses()`           | 收集含归纳变量的数组下标表达式                                |
| `src/rv64.cpp:2761-2791` | `collect_statement_induction_addresses()` | 遍历语句收集所有归纳地址                                      |
| `src/rv64.cpp:2793-2807` | `advance_loop_induction_addresses()`      | 生成`addi reg, reg, N` 指针增量指令                         |
| `src/rv64.cpp:2305-2324` | `acquire_loop_pointer_register()`         | 为循环指针分配寄存器（无调用优先 a4-a7）                      |
| `src/rv64.cpp:2922-2940` | `emit_stmt()` While 分支                  | 循环优化入口：识别归纳变量→收集地址→分配寄存器→绑定地址    |

### 创新四：汇编级临时值转发后处理

`forward_adjacent_temporaries` 是一个独特的后优化 pass，在汇编文本生成后执行：

1. **消除 store-load 往返**：检测 `sw rs, offset(s0)` 后紧跟 `lw rd, offset(s0)` 的模式，将其替换为 `mv rd, rs`，消除不必要的内存访问。
2. **消除反向移动对**：检测 `mv a, b` 后紧跟 `mv b, a` 的模式，保留第一条删除第二条。
3. **标记驱动的模式匹配**：代码生成阶段在临时槽的 store 指令后附加 `# sysy-temp` 标记，后处理 pass 仅对带标记的指令尝试转发，保证安全性。

**实例（`calc(a, b, c) { return a + b * c; }`）：**

后端代码生成阶段产生的原始汇编（注意中间值的 store 后紧接 load 回同一地址）：

```asm
# 后端生成的 calc 函数中的 store→load 往返：
  mv t0, s2
  mv t1, s3
  mulw t0, t0, t1       # t0 = b * c
  mv t1, t0
  mv t0, s1
  addw t0, t0, t1       # t0 = a + (b*c)
  sw t0, -48(s0)          # 存 t0 到栈槽
  lw a0, -48(s0)          # 立即加载同一栈槽到 a0（可消除）
```

`forward_adjacent_temporaries` 后处理之后：

```asm
  mv t0, s2
  mv t1, s3
  mulw t0, t0, t1
  mv t1, t0
  mv t0, s1
  addw t0, t0, t1
  mv a0, t0              # 直接 mv 替代了 sw→lw 往返
```

最后一行 `sw t0, -48(s0)` 和 `lw a0, -48(s0)` 被合并为 `mv a0, t0`，消除了 2 次内存访问。

**对应代码位置：**

| 文件                     | 行号                               | 说明                                                  |
| ------------------------ | ---------------------------------- | ----------------------------------------------------- |
| `src/rv64.cpp:99-152`  | `forward_adjacent_temporaries()` | 后处理 pass 主函数                                    |
| `src/rv64.cpp:224-226` | Generator::run()                   | O1 时调用`forward_adjacent_temporaries(assembly)`   |
| `src/rv64.cpp:860-862` | `store_local()`                  | 代码生成阶段在临时槽 store 后附加`# sysy-temp` 标记 |
| `src/rv64.cpp:122-127` | 标记检测                           | 通过`# sysy-temp` 识别可转发指令                    |

### 创新五：有符号二次幂强度削弱

对除以正二次幂常数的有符号整数运算，实现了完整的 RISC-V 强度削弱序列，避免了 `divw`/`remw` 指令的高延迟：

```
x / 2^n  →  sraiw + andi（处理负数舍入修正）
x % 2^n  →  sraiw + slliw + subw
x * 2^n  →  slliw
```

对于栈上变量的自更新操作（如 `i = i * 8`），强度削弱可直接在寄存器上完成，无需 load/store。

**实例对比（`x / 8` 和 `x % 8`）：**

输入 SysY 代码：

```c
int div8(int x) { return x / 8; }
int rem8(int x) { return x % 8; }
```

O0 生成的 `div8`（使用 `divw` 指令）：

```asm
div8:
  sw a0, -24(s0)
  lw t0, -24(s0)
  li t1, 8
  divw t0, t0, t1        # 硬件除法，延迟高
  sw t0, -32(s0)
  lw a0, -32(s0)
  ...
```

O1 生成的 `div8`（使用 `sraiw+andi` 强度削弱，以 `x = -100` 为例）：

```asm
div8:
  mv s1, a0
  mv t0, s1
  sraiw t1, t0, 31       # 取符号位全扩展（x<0 → 0xFFFFFFFF, x≥0 → 0）
  andi t1, t1, 7          # t1 = x<0 ? 7 : 0
  addw t1, t0, t1         # x + (x<0 ? 7 : 0)，处理负数舍入
  sraiw t1, t1, 3         # 算术右移 3 位，等价于 /8
  mv t0, t1
  mv a0, t0
```

O0 生成的 `rem8`（使用 `remw` 指令）：

```asm
rem8:
  ...
  li t1, 8
  remw t0, t0, t1        # 硬件取模，延迟高
```

O1 生成的 `rem8`（使用 `sraiw+slliw+subw` 强度削弱，以 `x = -100` 为例）：

```asm
rem8:
  mv s1, a0
  mv t0, s1
  sraiw t1, t0, 31
  andi t1, t1, 7
  addw t1, t0, t1
  sraiw t1, t1, 3         # t1 = x/8
  slliw t1, t1, 3          # t1 = (x/8) * 8
  subw t0, t0, t1          # x - (x/8)*8，即 x % 8
  mv a0, t0
```

对于 `x * 8`，O0 已经使用 `slliw`（编译器自动优化），O1 在自更新场景下同样直接使用移位。以上削弱避免了昂贵的 `divw`/`remw` 指令，在 RISC-V 软核（50MHz BOOM）上每条可节省数十个周期。

**对应代码位置：**

| 文件                       | 行号                                    | 说明                                                                                     |
| -------------------------- | --------------------------------------- | ---------------------------------------------------------------------------------------- |
| `src/rv64.cpp:1051-1058` | `positive_power_of_two_shift()`       | 判断常量是否为正二次幂并返回移位量                                                       |
| `src/rv64.cpp:1060-1087` | `emit_signed_power_of_two_division()` | **核心**：生成 `x/2^n` → `sraiw+andi`、`x%2^n` → `sraiw+slliw+subw` 序列 |
| `src/rv64.cpp:1394-1397` | `emit_binary_values()`                | 普通二元运算中检测`/` 和 `%` 的二次幂常量右操作数                                    |
| `src/rv64.cpp:1399-1408` | 同上                                    | `x*2^n` → `slliw` 移位优化                                                          |
| `src/rv64.cpp:2112-2125` | `emit_register_self_update()`         | 寄存器自更新场景（`i = i * 8` 等），直接在寄存器上完成削弱                             |

### 创新六：尾递归消除

`emit_self_tail_call` 实现了尾递归优化——当函数的最后一条语句是 `return f(...)` 且 f 就是当前函数自身、参数类型均为标量时，将递归调用转换为对函数入口的跳转，复用同一栈帧。

具体机制（`src/rv64.cpp:3029-3048`）：

1. 在函数入口生成 `tail_entry_label_`（位于 prologue 和入参绑定之后），递归调用不走 `call`，而是将新的参数值加载到寄存器中，然后 `j` 回 tail entry；
2. 无递归时的正常调用不受影响——函数首次进入走完整 prologue，执行完毕后走 epilogue；
3. 这一优化使得深度递归不消耗栈空间，避免了栈溢出风险。

**实例对比（尾递归阶乘 `fact(n, acc)`）：**

输入 SysY 代码：

```c
int fact(int n, int acc) {
  if (n <= 1) return acc;
  return fact(n - 1, acc * n);
}

int main() {
  return fact(5, 1);
}
```

O0 生成的 `fact` 函数——递归通过 `call fact` 实现，每次调用新栈帧：

```asm
fact:
  ...
  lw a0, -40(s0)          # 加载 n-1
  lw a1, -48(s0)          # 加载 acc*n
  call fact               # 递归调用，新栈帧
  sw a0, -56(s0)          # 存返回值
  lw a0, -56(s0)
  j .Lfact_epilogue
```

O1 生成的 `fact` 函数——递归通过 `j .Lfact_tail_entry` 实现，复用同一栈帧：

```asm
fact:
  ...
  mv s1, a0               # 绑定参数 n → s1
  mv s2, a1               # 绑定参数 acc → s2
.Lfact_tail_entry:         # 尾递归入口
  slti t0, s1, 2
  beqz t0, .Lfact_if_false_1
.Lfact_if_true_0:
  mv a0, s2
  j .Lfact_epilogue
.Lfact_if_false_1:
  ...
  lw s1, -40(s0)           # s1 = n-1（新参数值直接加载到参数寄存器）
  lw s2, -48(s0)           # s2 = acc * n
  j .Lfact_tail_entry       # 跳回入口，无 call，不增长栈
```

关键区别：O0 的 `call fact` + `ret` 在每次递归中消耗栈空间（深度递归可能导致栈溢出），O1 的 `j .Lfact_tail_entry` 将递归转换为循环等效的跳转，栈深度恒定为 1。

**对应代码位置：**

| 文件                       | 行号                      | 说明                                                  |
| -------------------------- | ------------------------- | ----------------------------------------------------- |
| `src/rv64.cpp:3029-3048` | `emit_self_tail_call()` | 检测`return f(...)` 且 f 为当前函数，生成 tail jump |
| `src/rv64.cpp:3019`      | `emit_return()`         | `return` 语句优先尝试尾调用消除                     |
| `src/rv64.cpp:3224`      | `emit_function()`       | 定义`tail_entry_label_` = `.L<name>_tail_entry`   |
| `src/rv64.cpp:3279`      | `emit_function()`       | O1 时在参数绑定后放置 tail entry label                |

---

## 测试体系

- **C++ 单元测试**（32 个场景）：覆盖最小程序编译、控制流与短路、浮点转换与运行时调用、数组与栈参数传递、诊断报告、O1 各优化 pass 行为验证、IR Pass 与线性扫描、深层表达式栈安全等。
- **Python CLI 集成测试**：验证编译器命令行接口的参数解析、错误处理和汇编输出。
- **官方用例验证**：140 个功能用例（双优化级）和 60 个性能用例（O1）均通过 Spike/pk 模拟器运行并比对输出。
- **覆盖率验证**：使用 `--coverage` 构建，确认四项核心覆盖率均超过 80%。

---

## 文档与当前代码的一致性说明

经检查，现有文档与当前代码基本一致，需注意以下两点：

1. **`docs/design.md` 中"IR 核心已经可独立测试，但尚未接管所有函数的最终 RV64 发射"**：该描述准确反映了当前状态。IR 子系统通过 `plan_source_registers` 驱动后端寄存器分配，但主要控制流和内存操作仍从 AST 直接降低。这是当前架构的合理取舍。
2. **`docs/requirements.md` 同时描述了 ARMv8-A 和 RV64GC 两个平台**：这是竞赛要求的概述文档，本项目实际选择 RV64GC 赛道。

---

## 技术栈与工具链

| 类别    | 选型                                             |
| ------- | ------------------------------------------------ |
| 语言    | C++17                                            |
| 构建    | CMake + Clang 18                                 |
| 前端    | Flex 2.6.4 + GNU Bison                           |
| 测试    | C++ 单元测试 + Python 集成测试 + Spike/pk 模拟器 |
| AI 辅助 | OpenAI Codex（生成初稿，人工逐行审阅修改）       |
