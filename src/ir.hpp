#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace sysy::detail {
struct Program;
}

namespace sysy::detail::ir {

using ValueId = std::uint32_t;
using BlockId = std::uint32_t;

enum class Type { Void, I32, F32, Ptr };

enum class Opcode {
  ConstInt,
  ConstFloat,
  Copy,
  Add,
  Sub,
  Mul,
  Div,
  Rem,
  Compare,
  IntToFloat,
  FloatToInt,
  Address,
  Load,
  Store,
  Call,
  Branch,
  CondBranch,
  Return
};

struct Instruction {
  Opcode opcode{Opcode::Copy};
  Type type{Type::Void};
  ValueId result{};
  std::vector<ValueId> operands;
  std::int32_t integer{};
  float floating{};
  std::string symbol;
};

struct Block {
  BlockId id{};
  std::vector<Instruction> instructions;
  std::vector<BlockId> successors;
};

struct Function {
  std::string name;
  BlockId entry{};
  std::vector<Block> blocks;
};

struct Loop {
  BlockId preheader{};
  std::vector<BlockId> blocks;
};

bool is_pure(const Instruction& instruction);
void propagate_constants_and_copies(Function& function);
void simplify_algebra(Function& function);
void remove_unreachable_blocks(Function& function);
void eliminate_dead_code(Function& function);
void eliminate_local_common_subexpressions(Function& function);
void hoist_loop_invariants(Function& function, const Loop& loop);

struct Allocation {
  enum class Kind { IntegerRegister, FloatRegister, Stack } kind{Kind::Stack};
  std::size_t index{};
};

std::unordered_map<ValueId, Allocation> linear_scan_allocate(
    const Function& function, std::size_t integer_registers, std::size_t float_registers);

struct RegisterPlan {
  std::unordered_map<std::string, std::size_t> integers;
  std::unordered_map<std::string, std::size_t> floats;
};

std::unordered_map<std::string, RegisterPlan> plan_source_registers(
    const Program& program, std::size_t integer_registers, std::size_t float_registers);

}  // namespace sysy::detail::ir
