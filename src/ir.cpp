#include "ir.hpp"
#include "internal.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_set>

namespace sysy::detail::ir {
namespace {

struct Constant {
  Type type{Type::I32};
  std::int32_t integer{};
  float floating{};
};

std::optional<Constant> fold(const Instruction& instruction,
                             const std::unordered_map<ValueId, Constant>& constants) {
  if (instruction.opcode == Opcode::ConstInt)
    return Constant{Type::I32, instruction.integer, 0.0F};
  if (instruction.opcode == Opcode::ConstFloat)
    return Constant{Type::F32, 0, instruction.floating};
  if (instruction.operands.empty()) return std::nullopt;
  auto get = [&](std::size_t index) -> const Constant* {
    if (index >= instruction.operands.size()) return nullptr;
    auto found = constants.find(instruction.operands[index]);
    return found == constants.end() ? nullptr : &found->second;
  };
  const Constant* left = get(0);
  if (!left) return std::nullopt;
  if (instruction.opcode == Opcode::Copy) return *left;
  if (instruction.opcode == Opcode::IntToFloat)
    return Constant{Type::F32, 0, static_cast<float>(left->integer)};
  if (instruction.opcode == Opcode::FloatToInt)
    return Constant{Type::I32, static_cast<std::int32_t>(left->floating), 0.0F};
  const Constant* right = get(1);
  if (!right) return std::nullopt;
  if (instruction.type == Type::F32) {
    float value{};
    if (instruction.opcode == Opcode::Add) value = left->floating + right->floating;
    else if (instruction.opcode == Opcode::Sub) value = left->floating - right->floating;
    else if (instruction.opcode == Opcode::Mul) value = left->floating * right->floating;
    else if (instruction.opcode == Opcode::Div && right->floating != 0.0F)
      value = left->floating / right->floating;
    else
      return std::nullopt;
    return Constant{Type::F32, 0, value};
  }
  const std::uint32_t a = static_cast<std::uint32_t>(left->integer);
  const std::uint32_t b = static_cast<std::uint32_t>(right->integer);
  std::int32_t value{};
  if (instruction.opcode == Opcode::Add) value = static_cast<std::int32_t>(a + b);
  else if (instruction.opcode == Opcode::Sub) value = static_cast<std::int32_t>(a - b);
  else if (instruction.opcode == Opcode::Mul) value = static_cast<std::int32_t>(a * b);
  else if (instruction.opcode == Opcode::Div && right->integer != 0)
    value = left->integer == std::numeric_limits<std::int32_t>::min() &&
                    right->integer == -1
                ? left->integer
                : left->integer / right->integer;
  else if (instruction.opcode == Opcode::Rem && right->integer != 0)
    value = left->integer == std::numeric_limits<std::int32_t>::min() &&
                    right->integer == -1
                ? 0
                : left->integer % right->integer;
  else
    return std::nullopt;
  return Constant{Type::I32, value, 0.0F};
}

std::string expression_key(const Instruction& instruction) {
  std::ostringstream key;
  key << static_cast<int>(instruction.opcode) << ':' << static_cast<int>(instruction.type);
  for (ValueId operand : instruction.operands) key << ':' << operand;
  key << ':' << instruction.integer << ':' << instruction.symbol;
  return key.str();
}

Block* find_block(Function& function, BlockId id) {
  for (Block& block : function.blocks)
    if (block.id == id) return &block;
  return nullptr;
}

}  // namespace

std::string source_register_key(const std::string& name, std::size_t line,
                                std::size_t column) {
  return name + "@" + std::to_string(line) + ":" + std::to_string(column);
}

bool is_pure(const Instruction& instruction) {
  switch (instruction.opcode) {
    case Opcode::ConstInt:
    case Opcode::ConstFloat:
    case Opcode::Copy:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::Div:
    case Opcode::Rem:
    case Opcode::Compare:
    case Opcode::IntToFloat:
    case Opcode::FloatToInt:
    case Opcode::Address: return true;
    default: return false;
  }
}

void propagate_constants_and_copies(Function& function) {
  std::unordered_map<ValueId, ValueId> copies;
  std::unordered_map<ValueId, Constant> constants;
  auto resolve = [&](ValueId value) {
    std::unordered_set<ValueId> seen;
    while (copies.count(value) && seen.insert(value).second) value = copies[value];
    return value;
  };
  for (Block& block : function.blocks) {
    for (Instruction& instruction : block.instructions) {
      for (ValueId& operand : instruction.operands) operand = resolve(operand);
      auto constant = fold(instruction, constants);
      if (instruction.opcode == Opcode::Copy && !instruction.operands.empty())
        copies[instruction.result] = instruction.operands.front();
      if (constant && instruction.result != 0) {
        constants[instruction.result] = *constant;
        instruction.operands.clear();
        instruction.symbol.clear();
        if (constant->type == Type::F32) {
          instruction.opcode = Opcode::ConstFloat;
          instruction.type = Type::F32;
          instruction.floating = constant->floating;
        } else {
          instruction.opcode = Opcode::ConstInt;
          instruction.type = Type::I32;
          instruction.integer = constant->integer;
        }
      }
    }
  }
}

void simplify_algebra(Function& function) {
  std::unordered_map<ValueId, std::int32_t> integers;
  for (Block& block : function.blocks) {
    for (Instruction& instruction : block.instructions) {
      if (instruction.opcode == Opcode::ConstInt) integers[instruction.result] = instruction.integer;
      if (instruction.operands.size() != 2 || instruction.type != Type::I32) continue;
      const ValueId left = instruction.operands[0];
      const ValueId right = instruction.operands[1];
      const bool right_zero = integers.count(right) && integers[right] == 0;
      const bool right_one = integers.count(right) && integers[right] == 1;
      const bool left_zero = integers.count(left) && integers[left] == 0;
      if ((instruction.opcode == Opcode::Add && right_zero) ||
          (instruction.opcode == Opcode::Sub && right_zero) ||
          (instruction.opcode == Opcode::Mul && right_one) ||
          (instruction.opcode == Opcode::Div && right_one)) {
        instruction.opcode = Opcode::Copy;
        instruction.operands = {left};
      } else if (instruction.opcode == Opcode::Add && left_zero) {
        instruction.opcode = Opcode::Copy;
        instruction.operands = {right};
      }
    }
  }
}

void remove_unreachable_blocks(Function& function) {
  std::unordered_set<BlockId> reachable;
  std::queue<BlockId> pending;
  pending.push(function.entry);
  while (!pending.empty()) {
    const BlockId id = pending.front();
    pending.pop();
    if (!reachable.insert(id).second) continue;
    for (const Block& block : function.blocks)
      if (block.id == id)
        for (BlockId successor : block.successors) pending.push(successor);
  }
  function.blocks.erase(
      std::remove_if(function.blocks.begin(), function.blocks.end(),
                     [&](const Block& block) { return !reachable.count(block.id); }),
      function.blocks.end());
}

void eliminate_dead_code(Function& function) {
  bool changed = true;
  while (changed) {
    changed = false;
    std::unordered_map<ValueId, std::size_t> uses;
    for (const Block& block : function.blocks)
      for (const Instruction& instruction : block.instructions)
        for (ValueId operand : instruction.operands) ++uses[operand];
    for (Block& block : function.blocks) {
      auto old_size = block.instructions.size();
      block.instructions.erase(
          std::remove_if(block.instructions.begin(), block.instructions.end(),
                         [&](const Instruction& instruction) {
                           return instruction.result != 0 && is_pure(instruction) &&
                                  uses[instruction.result] == 0;
                         }),
          block.instructions.end());
      changed = changed || old_size != block.instructions.size();
    }
  }
}

void eliminate_local_common_subexpressions(Function& function) {
  for (Block& block : function.blocks) {
    std::unordered_map<std::string, ValueId> available;
    for (Instruction& instruction : block.instructions) {
      if (!is_pure(instruction) || instruction.opcode == Opcode::ConstInt ||
          instruction.opcode == Opcode::ConstFloat || instruction.opcode == Opcode::Copy)
        continue;
      const std::string key = expression_key(instruction);
      auto found = available.find(key);
      if (found != available.end()) {
        instruction.opcode = Opcode::Copy;
        instruction.operands = {found->second};
        instruction.symbol.clear();
      } else {
        available[key] = instruction.result;
      }
    }
  }
}

void hoist_loop_invariants(Function& function, const Loop& loop) {
  Block* preheader = find_block(function, loop.preheader);
  if (!preheader) return;
  std::unordered_set<BlockId> loop_blocks(loop.blocks.begin(), loop.blocks.end());
  std::unordered_set<ValueId> defined_in_loop;
  for (const Block& block : function.blocks)
    if (loop_blocks.count(block.id))
      for (const Instruction& instruction : block.instructions)
        if (instruction.result) defined_in_loop.insert(instruction.result);
  std::unordered_set<ValueId> invariant;
  std::vector<Instruction> hoisted;
  bool changed = true;
  while (changed) {
    changed = false;
    for (Block& block : function.blocks) {
      if (!loop_blocks.count(block.id)) continue;
      for (auto it = block.instructions.begin(); it != block.instructions.end();) {
        if (!is_pure(*it) || it->opcode == Opcode::ConstInt || it->opcode == Opcode::ConstFloat) {
          ++it;
          continue;
        }
        const bool ready = std::all_of(it->operands.begin(), it->operands.end(),
                                       [&](ValueId value) {
                                         return !defined_in_loop.count(value) || invariant.count(value);
                                       });
        if (!ready) {
          ++it;
          continue;
        }
        invariant.insert(it->result);
        hoisted.push_back(*it);
        it = block.instructions.erase(it);
        changed = true;
      }
    }
  }
  auto position = preheader->instructions.end();
  if (position != preheader->instructions.begin()) {
    auto last = std::prev(position);
    if (last->opcode == Opcode::Branch || last->opcode == Opcode::CondBranch ||
        last->opcode == Opcode::Return)
      position = last;
  }
  preheader->instructions.insert(position, hoisted.begin(), hoisted.end());
}

std::unordered_map<ValueId, Allocation> linear_scan_allocate(
    const Function& function, std::size_t integer_registers, std::size_t float_registers) {
  struct Interval { ValueId value; Type type; std::size_t begin; std::size_t end; };
  std::unordered_map<ValueId, Interval> by_value;
  std::size_t position = 0;
  for (const Block& block : function.blocks) {
    for (const Instruction& instruction : block.instructions) {
      if (instruction.result)
        by_value[instruction.result] = {instruction.result, instruction.type, position, position};
      for (ValueId operand : instruction.operands) {
        auto found = by_value.find(operand);
        if (found != by_value.end()) found->second.end = position;
      }
      ++position;
    }
  }
  std::vector<Interval> intervals;
  for (const auto& item : by_value)
    if (item.second.type == Type::I32 || item.second.type == Type::Ptr ||
        item.second.type == Type::F32)
      intervals.push_back(item.second);
  std::sort(intervals.begin(), intervals.end(),
            [](const Interval& a, const Interval& b) { return a.begin < b.begin; });
  struct Active { Interval interval; std::size_t reg; };
  std::vector<Active> integer_active, float_active;
  std::unordered_map<ValueId, Allocation> result;
  std::size_t stack_slot = 0;
  for (const Interval& interval : intervals) {
    auto& active = interval.type == Type::F32 ? float_active : integer_active;
    const std::size_t limit = interval.type == Type::F32 ? float_registers : integer_registers;
    active.erase(std::remove_if(active.begin(), active.end(),
                                [&](const Active& item) { return item.interval.end < interval.begin; }),
                 active.end());
    std::set<std::size_t> used;
    for (const Active& item : active) used.insert(item.reg);
    std::size_t reg = 0;
    while (reg < limit && used.count(reg)) ++reg;
    if (reg < limit) {
      result[interval.value] = {interval.type == Type::F32
                                    ? Allocation::Kind::FloatRegister
                                    : Allocation::Kind::IntegerRegister,
                                reg};
      active.push_back({interval, reg});
    } else {
      result[interval.value] = {Allocation::Kind::Stack, stack_slot++};
    }
  }
  return result;
}

namespace {

class SourceRegisterPlanner {
 public:
  SourceRegisterPlanner(const sysy::detail::Function& source, std::size_t integer_registers,
                        std::size_t float_registers)
      : source_(source), integer_registers_(integer_registers),
        float_registers_(float_registers) {
    function_.name = source.name;
    function_.entry = 0;
    function_.blocks.push_back(Block{0, {}, {}});
    scopes_.emplace_back();
  }

  RegisterPlan run() {
    for (const ParamDecl& param : source_.params) {
      if (param.is_array)
        pointer_parameters_.insert(param.name);
      else
        declare(param.name, param.base, param.loc);
    }
    walk_statement(*source_.body);
    propagate_constants_and_copies(function_);
    simplify_algebra(function_);
    eliminate_local_common_subexpressions(function_);
    eliminate_dead_code(function_);
    auto allocations =
        linear_scan_allocate(function_, integer_registers_, float_registers_);
    RegisterPlan plan;
    for (const auto& item : variables_) {
      const Variable& variable = item.second;
      if (variable.uses == 0) continue;
      auto allocation = allocations.find(variable.value);
      if (allocation == allocations.end()) continue;
      if (allocation->second.kind == Allocation::Kind::IntegerRegister)
        plan.integers[variable.key] = allocation->second.index;
      else if (allocation->second.kind == Allocation::Kind::FloatRegister)
        plan.floats[variable.key] = allocation->second.index;
    }
    std::set<std::size_t> used_integer_registers;
    for (const auto& item : plan.integers) used_integer_registers.insert(item.second);
    for (const ParamDecl& param : source_.params) {
      if (has_nonlocal_name_) break;
      if (!param.is_array) continue;
      std::size_t reg = 0;
      while (reg < integer_registers_ && used_integer_registers.count(reg)) ++reg;
      if (reg == integer_registers_) break;
      plan.integers[source_register_key(param.name, param.loc.line, param.loc.column)] = reg;
      used_integer_registers.insert(reg);
    }
    return plan;
  }

 private:
  struct Variable {
    std::string name;
    std::string key;
    ValueId value{};
    Type type{Type::I32};
    std::size_t uses{};
  };

  void declare(const std::string& name, BaseType base, Loc loc) {
    const ValueId value = next_value_++;
    const Type type = base == BaseType::Float ? Type::F32 : Type::I32;
    scopes_.back()[name] = value;
    variables_[value] = Variable{
        name, source_register_key(name, loc.line, loc.column), value, type, 0};
    Instruction definition;
    definition.opcode = Opcode::Address;
    definition.type = type;
    definition.result = value;
    definition.symbol = name;
    block().instructions.push_back(std::move(definition));
  }

  ValueId find(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
      auto found = scope->find(name);
      if (found != scope->end()) return found->second;
    }
    return 0;
  }

  void use(const std::string& name) {
    const ValueId value = find(name);
    if (!value) {
      if (!pointer_parameters_.count(name)) has_nonlocal_name_ = true;
      return;
    }
    for (auto& loop : active_loop_uses_) loop.insert(value);
    use(value);
  }

  void use(ValueId value) {
    ++variables_.at(value).uses;
    Instruction use;
    use.opcode = Opcode::Store;
    use.type = Type::Void;
    use.operands = {value};
    use.symbol = variables_.at(value).name;
    block().instructions.push_back(std::move(use));
  }

  void walk_expression(const Expr& expression) {
    std::vector<const Expr*> pending{&expression};
    while (!pending.empty()) {
      const Expr* current = pending.back();
      pending.pop_back();
      if (current->kind == ExprKind::Name) use(current->text);
      for (auto argument = current->args.rbegin(); argument != current->args.rend(); ++argument)
        pending.push_back(argument->get());
      if (current->right) pending.push_back(current->right.get());
      if (current->left) pending.push_back(current->left.get());
    }
  }

  void walk_initializer(const Initializer& initializer) {
    if (initializer.expression) walk_expression(*initializer.expression);
    for (const auto& element : initializer.elements) walk_initializer(*element);
  }

  void walk_statement(const Stmt& statement) {
    if (statement.kind == StmtKind::While) {
      active_loop_uses_.emplace_back();
      walk_expression(*statement.expression);
      walk_statement(*statement.first);
      std::vector<ValueId> loop_uses(active_loop_uses_.back().begin(),
                                     active_loop_uses_.back().end());
      active_loop_uses_.pop_back();
      std::sort(loop_uses.begin(), loop_uses.end());
      for (ValueId value : loop_uses) {
        for (auto& outer_loop : active_loop_uses_) outer_loop.insert(value);
        use(value);
      }
      return;
    }
    const bool block_scope = statement.kind == StmtKind::Block;
    if (block_scope) scopes_.emplace_back();
    if (statement.kind == StmtKind::Declaration) {
      for (const VarDecl& declaration : statement.declarations) {
        for (const auto& dimension : declaration.dimensions) walk_expression(*dimension);
        if (declaration.dimensions.empty())
          declare(declaration.name, declaration.base, declaration.loc);
        if (declaration.initializer) walk_initializer(*declaration.initializer);
      }
    } else {
      if (statement.expression) walk_expression(*statement.expression);
      if (statement.value) walk_expression(*statement.value);
    }
    for (const auto& child : statement.statements) walk_statement(*child);
    if (statement.first) walk_statement(*statement.first);
    if (statement.second) walk_statement(*statement.second);
    if (block_scope) scopes_.pop_back();
  }

  Block& block() { return function_.blocks.front(); }

  const sysy::detail::Function& source_;
  std::size_t integer_registers_{};
  std::size_t float_registers_{};
  Function function_;
  ValueId next_value_{1};
  std::vector<std::unordered_map<std::string, ValueId>> scopes_;
  std::unordered_map<ValueId, Variable> variables_;
  std::vector<std::unordered_set<ValueId>> active_loop_uses_;
  std::unordered_set<std::string> pointer_parameters_;
  bool has_nonlocal_name_{};
};

}  // namespace

std::unordered_map<std::string, RegisterPlan> plan_source_registers(
    const Program& program, std::size_t integer_registers, std::size_t float_registers) {
  std::unordered_map<std::string, RegisterPlan> plans;
  for (const sysy::detail::Function& function : program.functions)
    plans[function.name] =
        SourceRegisterPlanner(function, integer_registers, float_registers).run();
  return plans;
}

}  // namespace sysy::detail::ir
