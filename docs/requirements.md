# Compiler System Design Competition 2026 — Requirements

## 1. Overview

Build a **SysY2026 compiler from scratch** for the 2026 National College Student Computer System Capability Competition (Huawei Bi Sheng Cup). The compiler must compile SysY2026 programs into assembly code targeting **ARMv8-A 64-bit** and/or **64-bit RISC-V (RV64GC)** platforms.

---

## 2. Source Language: SysY2026

SysY2026 is an extended subset of C. Development language spec is SysY2022 plus vector types (added for 2026).

### 2.1 Types

| Type | Description |
|------|-------------|
| `int` | 32-bit signed integer |
| `float` | 32-bit single-precision float (IEEE 754) |
| `const` | Compile-time constant qualifier |
| Multi-dimensional arrays | Row-major storage, element = `int` or `float`, all dimensions must be explicit |
| Vector types | New in 2026 (details TBD via competition committee) |
| `void` | Only as function return type |

Implicit `int` ↔ `float` conversions are supported. **No explicit cast syntax.**

### 2.2 Language Features

- **Single source file** (`.sy` extension, one `int main()` with empty params, return value must be output)
- Variables/constants: global + local, must be declared before use
- Functions: params can be `int`, `float`, or arrays; return type = `int`/`float`/`void`
- Array params: first dimension may omit length, passed by address
- Statements: assignment, expression stmt, blocks, `if`/`else` (nearest-else matching), `while`, `break`, `continue`, `return`
- Expressions: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `!`, `&&`, `||` (short-circuit)
- Operator precedence and associativity: same as C
- C-style comments (`//` and `/* */`)
- Numeric constants: decimal, octal, hex integers; floating-point constants (per C standard, no suffixes)
- Scoping: local shadows global; same-name locals cannot overlap; variable name may match function name

### 2.3 Full EBNF Grammar

```
CompUnit     -> [ CompUnit ] ( Decl | FuncDef )
Decl         -> ConstDecl | VarDecl
ConstDecl    -> 'const' BType ConstDef { ',' ConstDef } ';'
BType        -> 'int' | 'float'
ConstDef     -> Ident { '[' ConstExp ']' } '=' ConstInitVal
ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
VarDecl      -> BType VarDef { ',' VarDef } ';'
VarDef       -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
InitVal      -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
FuncDef      -> FuncType Ident '(' [FuncFParams] ')' Block
FuncType     -> 'void' | 'int' | 'float'
FuncFParams  -> FuncFParam { ',' FuncFParam }
FuncFParam   -> BType Ident ['[' ']' { '[' Exp ']' }]
Block        -> '{' { BlockItem } '}'
BlockItem    -> Decl | Stmt
Stmt         -> LVal '=' Exp ';' | [Exp] ';' | Block
             | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
             | 'while' '(' Cond ')' Stmt
             | 'break' ';' | 'continue' ';' | 'return' [Exp] ';'
Exp          -> AddExp
Cond         -> LOrExp
LVal         -> Ident {'[' Exp ']'}
PrimaryExp   -> '(' Exp ')' | LVal | Number
Number       -> IntConst | floatConst
UnaryExp     -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
UnaryOp      -> '+' | '-' | '!'
FuncRParams  -> Exp { ',' Exp }
MulExp       -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
AddExp       -> MulExp | AddExp ('+' | '-') MulExp
RelExp       -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
EqExp        -> RelExp | EqExp ('==' | '!=') RelExp
LAndExp      -> EqExp | LAndExp '&&' EqExp
LOrExp       -> LAndExp | LOrExp '||' LAndExp
ConstExp     -> AddExp
```

### 2.4 Semantic Constraints (non-exhaustive)

- Exactly one `int main()` with no parameters; return value must be output
- No identifier redeclaration at top level in `CompUnit`
- Scope: declaration point → end of file/block
- Array initializers: `{}` = all zeros; partial initialization fills remaining with 0/0.0; cannot exceed declared element count
- Global variables: initializer must be constant expression; uninitialized → all zeros
- Local variables: uninitialized → indeterminate value
- `int`/`float` params: pass by value; array params: pass starting address
- When return type is `int`/`float`: every branch should have `return Exp`; absent branch → undefined
- When return type is `void`: only bare `return;` allowed
- `Exp` (defined as `AddExp`) does NOT include `!`; `Cond` (defined as `LOrExp`) may include `!`
- Array-index expressions must evaluate to non-negative integer
- LVal left of `=` must denote a mutable variable
- Function call: actual argument types and count must exactly match formal params

### 2.5 Implicit Type Conversion Rules

| Direction | Rule |
|-----------|------|
| `float` → `int` | Fractional part discarded; undefined if integral part out of range |
| `int` → `float` | Value preserved |
| ARM ABI | Call `__aeabi_i2f(int)` for int→float conversion |

---

## 3. Runtime Library (SysY2022)

The compiler must recognize calls to these functions without explicit declarations in the source. Some accept types beyond SysY itself (e.g., strings in `putf`).

### I/O Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `getint` | `int getint()` | Read and return an integer |
| `getch` | `int getch()` | Read a character, return ASCII code |
| `getfloat` | `float getfloat()` | Read and return a float |
| `getarray` | `int getarray(int[])` | Read int sequence; 1st int = count, rest go into array; returns count |
| `getfarray` | `int getfarray(float[])` | Same for float sequence |
| `putint` | `void putint(int)` | Output integer |
| `putch` | `void putch(int)` | Output int as ASCII character |
| `putfloat` | `void putfloat(float)` | Output float |
| `putarray` | `void putarray(int, int[])` | Output `N: elem1 elem2 ...` |
| `putfarray` | `void putfarray(int, float[])` | Output `N: elem1 elem2 ...` for floats |
| `putf` | `void putf(char[], ...)` | Format-string output (supports `%d`, `%c`, `%f`) |

### Timing Functions

| Function | Description |
|----------|-------------|
| `starttime()` / `_sysy_starttime(line)` | Start timer (macro wraps `__LINE__`) |
| `stoptime()` / `_sysy_stoptime(line)` | Stop timer (macro wraps `__LINE__`) |
| `before_main()` | Init timer state, called before `main` |
| `after_main(unsigned long)` | Output all timer results + return value, called after `main` |

Note: `starttime`/`stoptime` do NOT support nesting. The runtime library is provided as `libsysy.a` (static linking used for evaluation).

---

## 4. Compiler Specifications

### 4.1 Build Environment

| Parameter | Value |
|-----------|-------|
| Compiler host OS | Ubuntu 24.04 (64-bit), Docker container |
| CPU | x86-64 (AMD64) |
| Memory | 8 GB |
| C | clang-18 `--std=c11 -O2 -lm` |
| C++ | clang++-18 `--std=c++17 -O2 -lm` |
| Java | openjdk 24, `javac -encoding utf-8`, main class: `Compiler.java` (no package) |
| Rust | rustc 1.85.0 `-O2` |

### 4.2 Compiler CLI Interface

Binary must be named `compiler`.

```
# Functional test
compiler testcase.sysy -S -o testcase.s

# Performance test (with -O1 flag)
compiler testcase.sysy -S -o testcase.s -O1
```

### 4.3 Restrictions

- Built from scratch. **Not allowed**: GCC, LLVM frameworks, or any code derived from them
- **Allowed**: Lex, YACC, Bison, JavaCC, JavaCUP, ANTLR (parser generators)
- **AI tools**: must be documented — tool name, what was generated, manual modifications
- **No** test-case-specific optimization (detecting inputs/functions/patterns), no hardcoded results, no environment probing
- **Academic integrity**: code originality ≥ 50% (excluding parser-generator output); must explain any borrowed techniques

---

## 5. Target Platforms (two options)

### 5.1 ARM Platform

| Parameter | Value |
|-----------|-------|
| Hardware | CG-FPGA15EG (Xilinx XCZU15EG) |
| CPU | ARM Cortex-A53 MPCore, 4 cores (cores 2,3 isolated for benchmarking) |
| ISA | ARMv8-A 64-bit |
| L1 I-cache | 32 KB, 2-way |
| L1 D-cache | 32 KB, 4-way |
| L2 cache | 1 MB shared, 16-way |
| NEON / FP | Supported (single + double precision) |
| Memory | 4 GB DDR4 |
| OS | Ubuntu 22.04 64-bit |
| Assembler/Linker | `gcc 11.2.0 -march=armv8-a` |

### 5.2 RISC-V Platform

| Parameter | Value |
|-----------|-------|
| Hardware | CG-FPGA15EG (FPGA soft core) |
| CPU | BOOM v3 (Berkeley Out-of-Order Machine) |
| ISA | RISC-V 64GC |
| Frequency | 50 MHz |
| Microarchitecture | Out-of-order, dual-issue, superscalar |
| FPU | IEEE 754 compliant |
| Memory | 2 GB |
| SIMD | **Not allowed in preliminaries**; allowed in finals per SIMD docs |
| Memory model | `-mcmodel=medany` (GCC convention) |
| Assembler/Linker | `gcc 13.3.0 -march=rv64gc` |
| Debug | GDB + OpenOCD |

---

## 6. Scoring

### 6.1 Preliminary Round (100 pts)

| Component | Weight | Description |
|-----------|--------|-------------|
| Functional | **50%** | Avg of all functional test case scores. Each case: `100 × passed_checkpoints / total_checkpoints` (or 0 if fails to compile) |
| Performance | **50%** | Geometric mean of per-case scores. Each case: `100 × fastest_time / our_time` |

### 6.2 Final Round (100 pts)

| Component | Weight | Description |
|-----------|--------|-------------|
| Functional (new cases) | **20%** | Same scoring method |
| Performance (new cases) | **70%** | Same method, benchmarked against GCC 11.2.0 `-O2` |
| Teamwork + Docs + Defense | **10%** | Presentation quality, documentation, collaboration |

### 6.3 Certification Levels

| Level | Criteria |
|-------|----------|
| **Level 1** (highest) | All mandatory functional cases pass, performance ≥ 90 (vs GCC -O2) |
| **Level 2** | All mandatory functional cases pass, 40 ≤ performance < 90 |
| **Level 3** | Functional score ≥ 60 |
| **Entry** | Compiles, passes ≥ 5 functional cases |

---

## 7. Deliverables

1. **Complete source code** (all project files), submitted to competition platform; at least one valid functional or performance test record
2. **Design document** covering: system architecture, module breakdown, optimization strategies
3. **AI tool usage disclosure** (if applicable): tool name, scope of generated code, manual modifications
4. **Third-party code attribution**: documented in both design doc and source headers

---

## 8. Provided Materials (in `docs/compiler2026-main/`)

| File | Description |
|------|-------------|
| `SysY2022-Language-Definition-V1-English.pdf` | Full language spec with EBNF and semantics |
| `SysY2022-Runtime-Library-V1-English.pdf` | Runtime library API specification |
| `2026年全国大学生计算机系统能力大赛-编译系统设计赛-编译系统实现赛道技术方案.pdf` | Competition technical rules |
| `关于编译优化合理性及相关违规行为认定的说明.pdf` | Rules on legitimate vs. prohibited optimizations |
| `2026初赛ARM赛道功能用例.zip` | ARM functional test cases (preliminary) |
| `2026初赛ARM赛道性能用例.zip` | ARM performance test cases (preliminary) |
| `2026初赛RISCV赛道功能用例.zip` | RISC-V functional test cases (preliminary) |
| `2026初赛RISCV赛道性能用例.zip` | RISC-V performance test cases (preliminary) |
| `newlib-4.5.0.20241231.zip` | Newlib for RISC-V target |
| `runtime/` | Runtime support files: `sylib.h`, `sylib.c`, `crt0.S`, `toby.ld`, `glue.h`, `tobylib.h`, `tobylib.c`, `libzzy.c` |

---

## 9. Key Implementation Tasks Summary

1. **Frontend**: Lexer + Parser (SysY2026 EBNF) → AST
2. **Semantic analysis**: type checking, scope management, implicit conversions, constant folding
3. **Intermediate representation** (optional but recommended for optimizations)
4. **Optimizations**: general-purpose only (loop unrolling/interchange/fusion/LICM, instruction scheduling, register allocation, function inlining); must NOT target specific test cases
5. **Code generation**: ARMv8-A 64-bit assembly and/or RISC-V 64GC assembly; must handle `-mcmodel=medany` on RISC-V; proper ABI conventions
6. **Runtime library integration**: recognize and correctly call all 13+ runtime functions, handle string args for `putf`
7. **CLI**: parse `-S -o <outfile> [-O1]` flags
8. **Error handling**: accurate error location reporting for syntax/semantic errors
9. **Testing**: validate against provided functional and performance test cases
10. **Documentation**: architecture, modules, optimization passes, AI tool usage if any
