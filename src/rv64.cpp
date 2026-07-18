#include "internal.hpp"
#include "ir.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace sysy::detail {
namespace {

std::uint64_t align_up(std::uint64_t value, std::uint64_t alignment) {
  return (value + alignment - 1) / alignment * alignment;
}

struct Type {
  BaseType base{BaseType::Int};
  std::vector<std::int64_t> dimensions;
  bool is_pointer{};

  bool is_void() const { return base == BaseType::Void; }
  bool is_scalar() const { return !is_pointer && dimensions.empty() && !is_void(); }
  bool is_array_like() const { return is_pointer || !dimensions.empty(); }
};

struct Constant {
  BaseType base{BaseType::Int};
  std::int32_t integer{};
  float floating{};
};

Constant make_int(std::int32_t value) {
  Constant c;
  c.integer = value;
  return c;
}

Constant make_float(float value) {
  Constant c;
  c.base = BaseType::Float;
  c.floating = value;
  return c;
}

bool truthy(const Constant& value) {
  return value.base == BaseType::Float ? value.floating != 0.0F : value.integer != 0;
}

std::int32_t as_int(const Constant& value) {
  return value.base == BaseType::Float ? static_cast<std::int32_t>(value.floating)
                                       : value.integer;
}

float as_float(const Constant& value) {
  return value.base == BaseType::Float ? value.floating : static_cast<float>(value.integer);
}

std::int32_t wrap_add(std::int32_t a, std::int32_t b) {
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) +
                                   static_cast<std::uint32_t>(b));
}

std::int32_t wrap_sub(std::int32_t a, std::int32_t b) {
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) -
                                   static_cast<std::uint32_t>(b));
}

std::int32_t wrap_mul(std::int32_t a, std::int32_t b) {
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) *
                                   static_cast<std::uint32_t>(b));
}

std::string escape_asm_string(const std::string& text) {
  std::ostringstream out;
  for (unsigned char c : text) {
    switch (c) {
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      case '\\': out << "\\\\"; break;
      case '"': out << "\\\""; break;
      default:
        if (c >= 32 && c < 127)
          out << static_cast<char>(c);
        else
          out << "\\" << std::oct << std::setw(3) << std::setfill('0')
              << static_cast<unsigned>(c) << std::dec;
    }
  }
  return out.str();
}

std::string forward_adjacent_temporaries(const std::string& assembly) {
  constexpr const char* kMarker = " # sysy-temp";
  std::vector<std::string> lines;
  std::istringstream input(assembly);
  for (std::string line; std::getline(input, line);) lines.push_back(std::move(line));

  std::ostringstream output;
  for (std::size_t i = 0; i < lines.size(); ++i) {
    std::string line = lines[i];
    if (line.rfind("  mv ", 0) == 0) {
      const std::size_t comma = line.find(", ");
      if (comma != std::string::npos) {
        const std::string destination = line.substr(5, comma - 5);
        const std::string source = line.substr(comma + 2);
        if (destination == source) continue;
        if (i + 1 < lines.size() && lines[i + 1] == "  mv " + source + ", " + destination) {
          output << line << '\n';
          ++i;
          continue;
        }
      }
    }
    const std::size_t marker = line.find(kMarker);
    if (marker == std::string::npos) {
      output << line << '\n';
      continue;
    }
    line.erase(marker);
    const bool floating = line.rfind("  fsw ", 0) == 0;
    const bool integer = line.rfind("  sw ", 0) == 0;
    if (i + 1 < lines.size() && (floating || integer)) {
      const std::string& next = lines[i + 1];
      const std::string load_prefix = floating ? "  flw " : "  lw ";
      const std::size_t comma = line.find(", ");
      const std::size_t next_comma = next.find(", ");
      if (next.rfind(load_prefix, 0) == 0 && comma != std::string::npos &&
          next_comma != std::string::npos &&
          line.substr(comma + 2) == next.substr(next_comma + 2)) {
        const std::size_t store_prefix = floating ? 6 : 5;
        const std::size_t load_size = load_prefix.size();
        const std::string source = line.substr(store_prefix, comma - store_prefix);
        const std::string destination =
            next.substr(load_size, next_comma - load_size);
        if (source != destination)
          output << "  " << (floating ? "fmv.s " : "mv ") << destination << ", "
                 << source << '\n';
        ++i;
        continue;
      }
    }
    output << line << '\n';
  }
  return output.str();
}

struct FunctionSig {
  Type result;
  std::vector<Type> params;
  bool variadic{};
};

enum class Storage { Local, Global, ParamPointer, IntRegister, FloatRegister };

struct Symbol {
  Type type;
  Storage storage{Storage::Local};
  std::uint64_t offset{};
  std::string label;
  std::string reg;
  bool is_const{};
  std::optional<Constant> constant;
};

struct GlobalObject {
  Symbol symbol;
  std::vector<Constant> values;
  bool all_zero{true};
};

enum class ValueKind {
  Void,
  ImmediateInt,
  ImmediateFloat,
  Local,
  Global,
  PointerSlot,
  String,
  IntRegister,
  FloatRegister
};

struct Value {
  ValueKind kind{ValueKind::Void};
  Type type;
  std::int32_t integer{};
  float floating{};
  std::uint64_t offset{};
  std::string label;
};

class Generator {
 public:
  Generator(const Program& program, const CompileOptions& options)
      : program_(program), options_(options) {
    install_builtins();
    if (options_.optimization == OptimizationLevel::O1)
      register_plans_ = ir::plan_source_registers(program_, 11, 12);
  }

  std::string run() {
    prepare_top_level();
    validate_main();
    emit_globals();
    for (const Function& function : program_.functions) emit_function(function);
    emit_strings();
    const std::string assembly = output_.str();
    return options_.optimization == OptimizationLevel::O1
               ? forward_adjacent_temporaries(assembly)
               : assembly;
  }

 private:
  [[noreturn]] void fail(Loc loc, const std::string& message) const {
    throw CompileError(loc, message);
  }

  void install_builtins() {
    auto scalar = [](BaseType base) { return Type{base, {}, false}; };
    auto pointer = [](BaseType base) { return Type{base, {-1}, true}; };
    functions_["getint"] = {scalar(BaseType::Int), {}};
    functions_["getch"] = {scalar(BaseType::Int), {}};
    functions_["getfloat"] = {scalar(BaseType::Float), {}};
    functions_["getarray"] = {scalar(BaseType::Int), {pointer(BaseType::Int)}};
    functions_["getfarray"] = {scalar(BaseType::Int), {pointer(BaseType::Float)}};
    functions_["putint"] = {scalar(BaseType::Void), {scalar(BaseType::Int)}};
    functions_["putch"] = {scalar(BaseType::Void), {scalar(BaseType::Int)}};
    functions_["putfloat"] = {scalar(BaseType::Void), {scalar(BaseType::Float)}};
    functions_["putarray"] = {scalar(BaseType::Void),
                               {scalar(BaseType::Int), pointer(BaseType::Int)}};
    functions_["putfarray"] = {scalar(BaseType::Void),
                                {scalar(BaseType::Int), pointer(BaseType::Float)}};
    functions_["putf"] = {scalar(BaseType::Void), {pointer(BaseType::Int)}, true};
    functions_["starttime"] = {scalar(BaseType::Void), {}};
    functions_["stoptime"] = {scalar(BaseType::Void), {}};
  }

  std::optional<Constant> eval_const(const Expr& expression) const {
    struct Frame {
      const Expr* expression{};
      bool expanded{};
    };
    std::vector<Frame> pending{{&expression, false}};
    std::unordered_map<const Expr*, std::optional<Constant>> values;
    while (!pending.empty()) {
      const Frame frame = pending.back();
      pending.pop_back();
      const Expr& current = *frame.expression;
      if (!frame.expanded) {
        if (current.kind == ExprKind::Int) {
          values[&current] = make_int(current.int_value);
        } else if (current.kind == ExprKind::Float) {
          values[&current] = make_float(current.float_value);
        } else if (current.kind == ExprKind::Name) {
          const Symbol* symbol = find_symbol(current.text);
          values[&current] = symbol && symbol->constant ? symbol->constant : std::nullopt;
        } else if (current.kind == ExprKind::Unary) {
          pending.push_back({&current, true});
          pending.push_back({current.left.get(), false});
        } else if (current.kind == ExprKind::Binary) {
          pending.push_back({&current, true});
          pending.push_back({current.right.get(), false});
          pending.push_back({current.left.get(), false});
        } else {
          values[&current] = std::nullopt;
        }
        continue;
      }

      if (current.kind == ExprKind::Unary) {
        const auto& inner = values.at(current.left.get());
        if (!inner) {
          values[&current] = std::nullopt;
        } else if (current.text == "+") {
          values[&current] = inner;
        } else if (current.text == "!") {
          values[&current] = make_int(!truthy(*inner));
        } else if (inner->base == BaseType::Float) {
          values[&current] = make_float(-inner->floating);
        } else {
          values[&current] = make_int(wrap_sub(0, inner->integer));
        }
        continue;
      }

      const auto& left = values.at(current.left.get());
      const auto& right = values.at(current.right.get());
      if (current.text == "&&" && left && !truthy(*left)) {
        values[&current] = make_int(0);
        continue;
      }
      if (current.text == "||" && left && truthy(*left)) {
        values[&current] = make_int(1);
        continue;
      }
      if (!left || !right) {
        values[&current] = std::nullopt;
        continue;
      }
      if (current.text == "&&" || current.text == "||") {
        values[&current] = make_int(truthy(*right));
        continue;
      }
      const bool use_float = left->base == BaseType::Float || right->base == BaseType::Float;
      if (current.text == "+")
        values[&current] = use_float ? make_float(as_float(*left) + as_float(*right))
                                     : make_int(wrap_add(left->integer, right->integer));
      else if (current.text == "-")
        values[&current] = use_float ? make_float(as_float(*left) - as_float(*right))
                                     : make_int(wrap_sub(left->integer, right->integer));
      else if (current.text == "*")
        values[&current] = use_float ? make_float(as_float(*left) * as_float(*right))
                                     : make_int(wrap_mul(left->integer, right->integer));
      else if (current.text == "/") {
        if ((use_float && as_float(*right) == 0.0F) || (!use_float && right->integer == 0))
          values[&current] = std::nullopt;
        else if (use_float)
          values[&current] = make_float(as_float(*left) / as_float(*right));
        else if (left->integer == std::numeric_limits<std::int32_t>::min() &&
                 right->integer == -1)
          values[&current] = make_int(left->integer);
        else
          values[&current] = make_int(left->integer / right->integer);
      } else if (current.text == "%") {
        if (use_float || right->integer == 0)
          values[&current] = std::nullopt;
        else if (left->integer == std::numeric_limits<std::int32_t>::min() &&
                 right->integer == -1)
          values[&current] = make_int(0);
        else
          values[&current] = make_int(left->integer % right->integer);
      } else if (current.text == "==")
        values[&current] = make_int(use_float ? as_float(*left) == as_float(*right)
                                              : left->integer == right->integer);
      else if (current.text == "!=")
        values[&current] = make_int(use_float ? as_float(*left) != as_float(*right)
                                              : left->integer != right->integer);
      else if (current.text == "<")
        values[&current] = make_int(use_float ? as_float(*left) < as_float(*right)
                                              : left->integer < right->integer);
      else if (current.text == "<=")
        values[&current] = make_int(use_float ? as_float(*left) <= as_float(*right)
                                              : left->integer <= right->integer);
      else if (current.text == ">")
        values[&current] = make_int(use_float ? as_float(*left) > as_float(*right)
                                              : left->integer > right->integer);
      else if (current.text == ">=")
        values[&current] = make_int(use_float ? as_float(*left) >= as_float(*right)
                                              : left->integer >= right->integer);
      else
        values[&current] = std::nullopt;
    }
    return values.at(&expression);
  }

  const Symbol* find_symbol(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
      auto found = it->find(name);
      if (found != it->end()) return &found->second;
    }
    auto global = globals_by_name_.find(name);
    return global == globals_by_name_.end() ? nullptr : &global->second;
  }

  Symbol* find_symbol(const std::string& name) {
    return const_cast<Symbol*>(static_cast<const Generator*>(this)->find_symbol(name));
  }

  std::int64_t require_dimension(const Expr& expression) const {
    auto value = eval_const(expression);
    if (!value || value->base == BaseType::Float)
      fail(expression.loc, "array dimension must be an integer constant expression");
    if (value->integer <= 0) fail(expression.loc, "array dimension must be positive");
    return value->integer;
  }

  Type resolve_decl_type(BaseType base, const std::vector<ExprPtr>& dimensions) const {
    Type type{base, {}, false};
    std::uint64_t elements = 1;
    for (const auto& dimension : dimensions) {
      const std::int64_t value = require_dimension(*dimension);
      if (elements > kMaxObjectBytes / 4 / static_cast<std::uint64_t>(value))
        fail(dimension->loc, "array object is too large");
      elements *= static_cast<std::uint64_t>(value);
      type.dimensions.push_back(value);
    }
    return type;
  }

  Type resolve_param_type(const ParamDecl& param) const {
    Type type{param.base, {}, param.is_array};
    if (param.is_array) type.dimensions.push_back(-1);
    for (const auto& dimension : param.trailing_dimensions)
      type.dimensions.push_back(require_dimension(*dimension));
    return type;
  }

  std::uint64_t element_count(const Type& type) const {
    std::uint64_t count = 1;
    for (std::int64_t dimension : type.dimensions) {
      if (dimension < 0) continue;
      count *= static_cast<std::uint64_t>(dimension);
    }
    return count;
  }

  Constant convert_constant(Constant value, BaseType destination, Loc loc) const {
    if (destination == BaseType::Float) return make_float(as_float(value));
    if (destination == BaseType::Int) return make_int(as_int(value));
    fail(loc, "invalid constant conversion");
  }

  void flatten_initializer(const Initializer& initializer, const Type& type,
                           std::vector<const Expr*>& values) const {
    const std::uint64_t total = type.dimensions.empty() ? 1 : element_count(type);
    values.assign(static_cast<std::size_t>(total), nullptr);
    std::uint64_t cursor = 0;
    flatten_level(initializer, type.dimensions, 0, 0, total, cursor, values);
    if (cursor > total) fail(initializer.loc, "too many array initializers");
  }

  void flatten_level(const Initializer& initializer, const std::vector<std::int64_t>& dims,
                     std::size_t depth, std::uint64_t start, std::uint64_t extent,
                     std::uint64_t& cursor, std::vector<const Expr*>& values) const {
    if (!initializer.is_list()) {
      if (cursor >= start + extent) fail(initializer.loc, "too many array initializers");
      values[static_cast<std::size_t>(cursor++)] = initializer.expression.get();
      return;
    }
    if (depth >= dims.size()) {
      if (initializer.elements.size() > 1)
        fail(initializer.loc, "scalar initializer has too many elements");
      if (!initializer.elements.empty())
        flatten_level(*initializer.elements.front(), dims, depth, start, extent, cursor, values);
      return;
    }
    for (const auto& child : initializer.elements) {
      if (child->is_list()) {
        if (cursor >= start + extent) fail(child->loc, "too many array initializers");
        std::size_t child_depth = depth + 1;
        std::uint64_t sub_extent = 1;
        for (std::size_t candidate = depth + 1; candidate < dims.size(); ++candidate) {
          std::uint64_t candidate_extent = 1;
          for (std::size_t i = candidate; i < dims.size(); ++i)
            candidate_extent *= static_cast<std::uint64_t>(dims[i]);
          if ((cursor - start) % candidate_extent == 0) {
            child_depth = candidate;
            sub_extent = candidate_extent;
            break;
          }
        }
        const std::uint64_t sub_start = cursor;
        flatten_level(*child, dims, child_depth, sub_start, sub_extent, cursor, values);
        cursor = sub_start + sub_extent;
      } else {
        flatten_level(*child, dims, dims.size(), start, extent, cursor, values);
      }
      if (cursor > start + extent) fail(child->loc, "too many array initializers");
    }
    cursor = start + extent;
  }

  const Expr* scalar_initializer(const Initializer& init) const {
    if (!init.is_list()) return init.expression.get();
    if (init.elements.empty()) return nullptr;
    if (init.elements.size() != 1) fail(init.loc, "scalar initializer has too many elements");
    return scalar_initializer(*init.elements.front());
  }

  void prepare_top_level() {
    for (const VarDecl& declaration : program_.globals) {
      if (globals_by_name_.count(declaration.name) || functions_.count(declaration.name))
        fail(declaration.loc, "duplicate top-level name '" + declaration.name + "'");
      GlobalObject object;
      object.symbol.type = resolve_decl_type(declaration.base, declaration.dimensions);
      object.symbol.storage = Storage::Global;
      object.symbol.label = declaration.name;
      object.symbol.is_const = declaration.is_const;
      const std::uint64_t count = object.symbol.type.dimensions.empty()
                                      ? 1
                                      : element_count(object.symbol.type);
      object.values.assign(static_cast<std::size_t>(count),
                           declaration.base == BaseType::Float ? make_float(0) : make_int(0));
      if (declaration.initializer) {
        std::vector<const Expr*> flat;
        flatten_initializer(*declaration.initializer, object.symbol.type, flat);
        for (std::size_t i = 0; i < flat.size(); ++i) {
          if (!flat[i]) continue;
          auto value = eval_const(*flat[i]);
          if (!value) fail(flat[i]->loc, "global initializer must be constant");
          object.values[i] = convert_constant(*value, declaration.base, flat[i]->loc);
        }
      }
      for (const Constant& value : object.values)
        object.all_zero &= value.base == BaseType::Float ? value.floating == 0.0F
                                                         : value.integer == 0;
      if (declaration.is_const && object.symbol.type.dimensions.empty())
        object.symbol.constant = object.values.front();
      globals_by_name_[declaration.name] = object.symbol;
      globals_.push_back(std::move(object));
    }

    for (const Function& function : program_.functions) {
      if (globals_by_name_.count(function.name) || user_functions_.count(function.name) ||
          (functions_.count(function.name) && function.name != "main"))
        fail(function.loc, "duplicate function '" + function.name + "'");
      FunctionSig signature;
      signature.result = Type{function.return_type, {}, false};
      for (const ParamDecl& param : function.params)
        signature.params.push_back(resolve_param_type(param));
      functions_[function.name] = signature;
      user_functions_.insert(function.name);
      function_defs_[function.name] = &function;
    }
  }

  void validate_main() const {
    auto it = functions_.find("main");
    if (it == functions_.end() || !user_functions_.count("main"))
      fail(Loc{1, 1}, "program must define int main()");
    if (it->second.result.base != BaseType::Int || !it->second.params.empty())
      fail(Loc{1, 1}, "main must have signature int main()");
  }

  void emit_globals() {
    for (const GlobalObject& object : globals_) {
      const std::uint64_t bytes = object.values.size() * 4ULL;
      if (object.all_zero) {
        output_ << ".section .bss\n.align 2\n.globl " << object.symbol.label << '\n'
                << object.symbol.label << ":\n  .zero " << bytes << "\n";
      } else {
        output_ << ".section .data\n.align 2\n.globl " << object.symbol.label << '\n'
                << object.symbol.label << ":\n";
        for (const Constant& value : object.values) {
          std::uint32_t bits{};
          if (value.base == BaseType::Float)
            std::memcpy(&bits, &value.floating, sizeof(bits));
          else
            bits = static_cast<std::uint32_t>(value.integer);
          output_ << "  .word " << bits << '\n';
        }
      }
      output_ << ".size " << object.symbol.label << ", " << bytes << "\n\n";
    }
  }

  void emit_strings() {
    if (strings_.empty()) return;
    output_ << ".section .rodata\n";
    for (const auto& item : strings_)
      output_ << item.first << ":\n  .asciz \"" << escape_asm_string(item.second) << "\"\n";
  }

  void line(const std::string& text) { *current_stream_ << "  " << text << '\n'; }
  void label(const std::string& text) { *current_stream_ << text << ":\n"; }

  std::string new_label(const std::string& prefix) {
    return ".L" + current_function_ + "_" + prefix + "_" + std::to_string(label_counter_++);
  }

  std::uint64_t allocate(std::uint64_t bytes, std::uint64_t alignment = 8) {
    next_offset_ = align_up(next_offset_, alignment);
    if (bytes > kMaxObjectBytes || next_offset_ > kMaxObjectBytes - bytes)
      fail(current_function_loc_, "function stack frame is too large");
    next_offset_ += bytes;
    return next_offset_;
  }

  std::uint64_t allocate_temp() { return allocate(8); }

  void address_from_s0(std::int64_t displacement, const std::string& destination) {
    if (displacement >= -2048 && displacement <= 2047) {
      line("addi " + destination + ", s0, " + std::to_string(displacement));
    } else {
      line("li " + destination + ", " + std::to_string(displacement));
      line("add " + destination + ", s0, " + destination);
    }
  }

  void load_local(const std::string& instruction, const std::string& destination,
                  std::uint64_t offset) {
    const std::int64_t displacement = -static_cast<std::int64_t>(offset);
    if (displacement >= -2048) {
      line(instruction + " " + destination + ", " + std::to_string(displacement) + "(s0)");
    } else {
      address_from_s0(displacement, "t6");
      line(instruction + " " + destination + ", 0(t6)");
    }
  }

  void store_local(const std::string& instruction, const std::string& source,
                   std::uint64_t offset, bool temporary = false) {
    const std::string marker = temporary ? " # sysy-temp" : "";
    const std::int64_t displacement = -static_cast<std::int64_t>(offset);
    if (displacement >= -2048) {
      line(instruction + " " + source + ", " + std::to_string(displacement) + "(s0)" +
           marker);
    } else {
      address_from_s0(displacement, "t6");
      line(instruction + " " + source + ", 0(t6)" + marker);
    }
  }

  void load_base_offset(const std::string& instruction, const std::string& destination,
                        const std::string& base, std::uint64_t offset) {
    if (offset <= 2047) {
      line(instruction + " " + destination + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
      line("li t6, " + std::to_string(offset));
      line("add t6, " + base + ", t6");
      line(instruction + " " + destination + ", 0(t6)");
    }
  }

  void store_base_offset(const std::string& instruction, const std::string& source,
                         const std::string& base, std::uint64_t offset) {
    if (offset <= 2047) {
      line(instruction + " " + source + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
      line("li t6, " + std::to_string(offset));
      line("add t6, " + base + ", t6");
      line(instruction + " " + source + ", 0(t6)");
    }
  }

  Value immediate(Constant constant) const {
    Value value;
    value.type = Type{constant.base, {}, false};
    if (constant.base == BaseType::Float) {
      value.kind = ValueKind::ImmediateFloat;
      value.floating = constant.floating;
    } else {
      value.kind = ValueKind::ImmediateInt;
      value.integer = constant.integer;
    }
    return value;
  }

  Value local_value(const Symbol& symbol) const {
    Value value;
    value.type = symbol.type;
    value.offset = symbol.offset;
    if (symbol.storage == Storage::ParamPointer)
      value.kind = ValueKind::PointerSlot;
    else if (symbol.storage == Storage::IntRegister) {
      value.kind = ValueKind::IntRegister;
      value.label = symbol.reg;
    } else if (symbol.storage == Storage::FloatRegister) {
      value.kind = ValueKind::FloatRegister;
      value.label = symbol.reg;
    }
    else if (symbol.storage == Storage::Global) {
      value.kind = ValueKind::Global;
      value.label = symbol.label;
    } else
      value.kind = ValueKind::Local;
    return value;
  }

  void load_int(const Value& value, const std::string& reg) {
    if (value.type.base == BaseType::Float && value.type.is_scalar()) {
      load_float(value, "ft0");
      line("fcvt.w.s " + reg + ", ft0, rtz");
      return;
    }
    switch (value.kind) {
      case ValueKind::ImmediateInt: line("li " + reg + ", " + std::to_string(value.integer)); break;
      case ValueKind::ImmediateFloat: break;
      case ValueKind::Local: load_local("lw", reg, value.offset); break;
      case ValueKind::IntRegister:
        if (reg != value.label) line("mv " + reg + ", " + value.label);
        break;
      case ValueKind::Global:
        line("lla t6, " + value.label);
        line("lw " + reg + ", 0(t6)");
        break;
      default: fail(current_function_loc_, "integer value expected");
    }
  }

  void load_pointer(const Value& value, const std::string& reg) {
    if (value.kind == ValueKind::PointerSlot) {
      load_local("ld", reg, value.offset);
    } else if (value.kind == ValueKind::String || value.kind == ValueKind::Global) {
      line("lla " + reg + ", " + value.label);
    } else {
      fail(current_function_loc_, "pointer value expected");
    }
  }

  void load_float(const Value& value, const std::string& reg) {
    if (value.kind == ValueKind::ImmediateFloat) {
      std::uint32_t bits{};
      std::memcpy(&bits, &value.floating, sizeof(bits));
      line("li t5, " + std::to_string(bits));
      line("fmv.w.x " + reg + ", t5");
    } else if (value.kind == ValueKind::ImmediateInt || value.type.base == BaseType::Int) {
      load_int(value, "t5");
      line("fcvt.s.w " + reg + ", t5");
    } else if (value.kind == ValueKind::Local) {
      load_local("flw", reg, value.offset);
    } else if (value.kind == ValueKind::FloatRegister) {
      if (reg != value.label) line("fmv.s " + reg + ", " + value.label);
    } else if (value.kind == ValueKind::Global) {
      line("lla t6, " + value.label);
      line("flw " + reg + ", 0(t6)");
    } else {
      fail(current_function_loc_, "floating value expected");
    }
  }

  Value store_int_temp(const std::string& reg, bool forwardable = true) {
    Value value;
    value.kind = ValueKind::Local;
    value.type = Type{BaseType::Int, {}, false};
    value.offset = allocate_temp();
    store_local("sw", reg, value.offset, forwardable);
    return value;
  }

  Value store_float_temp(const std::string& reg, bool forwardable = true) {
    Value value;
    value.kind = ValueKind::Local;
    value.type = Type{BaseType::Float, {}, false};
    value.offset = allocate_temp();
    store_local("fsw", reg, value.offset, forwardable);
    return value;
  }

  Value make_pointer_temp(const Type& type, const std::string& reg) {
    Value value;
    value.kind = ValueKind::PointerSlot;
    value.type = type;
    value.type.is_pointer = true;
    value.offset = allocate_temp();
    store_local("sd", reg, value.offset);
    return value;
  }

  Value transient_int(const std::string& reg) const {
    Value value;
    value.kind = ValueKind::IntRegister;
    value.type = Type{BaseType::Int, {}, false};
    value.label = reg;
    return value;
  }

  Value transient_float(const std::string& reg) const {
    Value value;
    value.kind = ValueKind::FloatRegister;
    value.type = Type{BaseType::Float, {}, false};
    value.label = reg;
    return value;
  }

  Value finish_int_result(const std::string& reg, bool transient) {
    if (!transient) return store_int_temp(reg);
    if (reg != "t1") line("mv t1, " + reg);
    return transient_int("t1");
  }

  Value finish_float_result(const std::string& reg, bool transient) {
    if (!transient) return store_float_temp(reg);
    if (reg != "ft1") line("fmv.s ft1, " + reg);
    return transient_float("ft1");
  }

  bool is_simple_index(const Expr& expression) const {
    if (expression.kind == ExprKind::Int || expression.kind == ExprKind::Float ||
        expression.kind == ExprKind::Name)
      return true;
    return options_.optimization == OptimizationLevel::O1 && eval_const(expression).has_value();
  }

  Type address_to_register(const Expr& expression) {
    if (expression.kind == ExprKind::Name) {
      for (auto it = inline_bindings_.rbegin(); it != inline_bindings_.rend(); ++it) {
        auto binding = it->find(expression.text);
        if (binding != it->end()) {
          if (!binding->second.type.is_array_like())
            fail(expression.loc, "subscript requires an array");
          load_pointer(binding->second, "t0");
          return binding->second.type;
        }
      }
      Symbol* symbol = find_symbol(expression.text);
      if (!symbol) fail(expression.loc, "undefined identifier '" + expression.text + "'");
      if (symbol->storage == Storage::Global) {
        line("lla t0, " + symbol->label);
      } else if (symbol->storage == Storage::ParamPointer) {
        load_local("ld", "t0", symbol->offset);
      } else {
        address_from_s0(-static_cast<std::int64_t>(symbol->offset), "t0");
      }
      return symbol->type;
    }
    if (expression.kind != ExprKind::Subscript)
      fail(expression.loc, "left side of assignment is not an lvalue");
    Type base_type;
    Value index;
    bool index_loaded = false;
    if (!is_simple_index(*expression.right) && expression.left->kind == ExprKind::Name) {
      index = emit_expr(*expression.right);
      if (index.type.base != BaseType::Int || !index.type.is_scalar())
        fail(expression.right->loc, "array index must have int type");
      load_int(index, "t1");
      index_loaded = true;
      base_type = address_to_register(*expression.left);
    } else {
      base_type = address_to_register(*expression.left);
      if (!base_type.is_array_like()) fail(expression.loc, "subscript requires an array");
    }
    if (!index_loaded && is_simple_index(*expression.right)) {
      index = emit_expr(*expression.right);
    } else if (!index_loaded) {
      Value saved_base = make_pointer_temp(base_type, "t0");
      index = emit_expr(*expression.right);
      load_pointer(saved_base, "t0");
    }
    if (!base_type.is_array_like()) fail(expression.loc, "subscript requires an array");
    if (index.type.base != BaseType::Int || !index.type.is_scalar())
      fail(expression.right->loc, "array index must have int type");
    if (!index_loaded) load_int(index, "t1");
    std::uint64_t stride = 4;
    std::vector<std::int64_t> remaining = base_type.dimensions;
    if (!remaining.empty()) remaining.erase(remaining.begin());
    for (std::int64_t dimension : remaining) stride *= static_cast<std::uint64_t>(dimension);
    if ((stride & (stride - 1)) == 0) {
      unsigned shift = 0;
      for (std::uint64_t value = stride; value > 1; value >>= 1) ++shift;
      if (shift != 0) line("slli t1, t1, " + std::to_string(shift));
    } else {
      line("li t2, " + std::to_string(stride));
      line("mul t1, t1, t2");
    }
    line("add t0, t0, t1");
    return Type{base_type.base, std::move(remaining), !remaining.empty()};
  }

  Value address_of(const Expr& expression) {
    Type type = address_to_register(expression);
    return make_pointer_temp(type, "t0");
  }

  Value emit_name(const Expr& expression) {
    for (auto it = inline_bindings_.rbegin(); it != inline_bindings_.rend(); ++it) {
      auto binding = it->find(expression.text);
      if (binding != it->end()) return binding->second;
    }
    auto global_binding = global_register_bindings_.find(expression.text);
    if (global_binding != global_register_bindings_.end()) return global_binding->second;
    Symbol* symbol = find_symbol(expression.text);
    if (!symbol) fail(expression.loc, "undefined identifier '" + expression.text + "'");
    if (symbol->constant && options_.optimization == OptimizationLevel::O1)
      return immediate(*symbol->constant);
    if (symbol->type.is_array_like()) return address_of(expression);
    return local_value(*symbol);
  }

  Value emit_expr(const Expr& expression) {
    if (options_.optimization == OptimizationLevel::O1 && inline_bindings_.empty()) {
      auto folded = eval_const(expression);
      if (folded) return immediate(*folded);
    }
    switch (expression.kind) {
      case ExprKind::Int: return immediate(make_int(expression.int_value));
      case ExprKind::Float: return immediate(make_float(expression.float_value));
      case ExprKind::String: {
        Value value;
        value.kind = ValueKind::String;
        value.type = Type{BaseType::Int, {-1}, true};
        value.label = ".Lstr_" + std::to_string(strings_.size());
        strings_.push_back({value.label, expression.text});
        return value;
      }
      case ExprKind::Name: return emit_name(expression);
      case ExprKind::Subscript: {
        Type type = address_to_register(expression);
        if (!type.dimensions.empty()) return make_pointer_temp(type, "t0");
        if (type.base == BaseType::Float) {
          line("flw ft0, 0(t0)");
          return store_float_temp("ft0");
        }
        line("lw t1, 0(t0)");
        return store_int_temp("t1");
      }
      case ExprKind::Unary: return emit_unary(expression);
      case ExprKind::Binary: return emit_binary(expression, false);
      case ExprKind::Call: return emit_call(expression);
    }
    fail(expression.loc, "unsupported expression");
  }

  Value emit_expr_transient(const Expr& expression) {
    if (expression.kind == ExprKind::Subscript) {
      Type type = address_to_register(expression);
      if (!type.dimensions.empty()) return make_pointer_temp(type, "t0");
      if (type.base == BaseType::Float) {
        line("flw ft1, 0(t0)");
        return transient_float("ft1");
      }
      line("lw t1, 0(t0)");
      return transient_int("t1");
    }
    if (expression.kind == ExprKind::Binary && expression.text != "&&" &&
        expression.text != "||")
      return emit_binary(expression, true);
    if (expression.kind == ExprKind::Unary && expression.text != "!") {
      Value inner = emit_expr_transient(*expression.left);
      if (!inner.type.is_scalar()) fail(expression.loc, "unary operator requires a scalar");
      if (expression.text == "+") return inner;
      if (inner.type.base == BaseType::Float) {
        load_float(inner, "ft0");
        line("fneg.s ft0, ft0");
        return finish_float_result("ft0", true);
      }
      load_int(inner, "t0");
      line("negw t0, t0");
      return finish_int_result("t0", true);
    }
    return emit_expr(expression);
  }

  Value emit_unary(const Expr& expression) {
    if (expression.text == "!") return materialize_bool(expression);
    Value inner = emit_expr(*expression.left);
    if (!inner.type.is_scalar()) fail(expression.loc, "unary operator requires a scalar");
    if (expression.text == "+") return inner;
    if (inner.type.base == BaseType::Float) {
      load_float(inner, "ft0");
      line("fneg.s ft0, ft0");
      return store_float_temp("ft0");
    }
    load_int(inner, "t0");
    line("negw t0, t0");
    return store_int_temp("t0");
  }

  Value emit_binary(const Expr& expression, bool transient_result) {
    if (expression.text == "&&" || expression.text == "||")
      return materialize_bool(expression);
    std::vector<const Expr*> left_spine;
    const Expr* first = &expression;
    while (first->kind == ExprKind::Binary && first->text != "&&" && first->text != "||") {
      left_spine.push_back(first);
      first = first->left.get();
    }
    Value result = emit_expr(*first);
    for (auto node = left_spine.rbegin(); node != left_spine.rend(); ++node) {
      Value right = emit_expr_transient(*(*node)->right);
      const bool final = std::next(node) == left_spine.rend();
      result = emit_binary_values(**node, result, right, final && transient_result);
    }
    return result;
  }

  Value emit_binary_values(const Expr& expression, Value left, Value right,
                           bool transient_result) {
    if (!left.type.is_scalar() || !right.type.is_scalar())
      fail(expression.loc, "binary operator requires scalar operands");
    const bool comparison = expression.text == "==" || expression.text == "!=" ||
                            expression.text == "<" || expression.text == "<=" ||
                            expression.text == ">" || expression.text == ">=";
    const bool use_float = left.type.base == BaseType::Float || right.type.base == BaseType::Float;
    if (expression.text == "%" && use_float)
      fail(expression.loc, "operator % requires integer operands");
    if (use_float) {
      load_float(left, "ft0");
      load_float(right, "ft1");
      if (comparison) {
        if (expression.text == "==") line("feq.s t0, ft0, ft1");
        else if (expression.text == "!=") {
          line("feq.s t0, ft0, ft1");
          line("xori t0, t0, 1");
        } else if (expression.text == "<") line("flt.s t0, ft0, ft1");
        else if (expression.text == "<=") line("fle.s t0, ft0, ft1");
        else if (expression.text == ">") line("flt.s t0, ft1, ft0");
        else line("fle.s t0, ft1, ft0");
        return finish_int_result("t0", transient_result);
      }
      if (expression.text == "+") line("fadd.s ft0, ft0, ft1");
      else if (expression.text == "-") line("fsub.s ft0, ft0, ft1");
      else if (expression.text == "*") line("fmul.s ft0, ft0, ft1");
      else if (expression.text == "/") line("fdiv.s ft0, ft0, ft1");
      else fail(expression.loc, "unsupported floating operator");
      return finish_float_result("ft0", transient_result);
    }
    if (!comparison) {
      if (right.kind == ValueKind::ImmediateInt) {
        const std::int32_t constant = right.integer;
        if ((expression.text == "+" || expression.text == "-") && constant == 0)
          return left;
        if ((expression.text == "*" || expression.text == "/") && constant == 1)
          return left;
        if (expression.text == "*" && constant == 0) return immediate(make_int(0));
        const std::int64_t delta = expression.text == "+"
                                       ? constant
                                       : expression.text == "-"
                                             ? -static_cast<std::int64_t>(constant)
                                             : 4096;
        if ((expression.text == "+" || expression.text == "-") && delta >= -2048 &&
            delta <= 2047) {
          load_int(left, "t0");
          line("addiw t0, t0, " + std::to_string(delta));
          return finish_int_result("t0", transient_result);
        }
        const std::uint32_t bits = static_cast<std::uint32_t>(constant);
        if (expression.text == "*" && constant > 0 && (bits & (bits - 1)) == 0) {
          unsigned shift = 0;
          for (std::uint32_t value = bits; value > 1; value >>= 1) ++shift;
          load_int(left, "t0");
          line("slliw t0, t0, " + std::to_string(shift));
          return finish_int_result("t0", transient_result);
        }
      }
      if (left.kind == ValueKind::ImmediateInt) {
        if (expression.text == "+" && left.integer == 0) return right;
        if (expression.text == "*" && left.integer == 0) return immediate(make_int(0));
        if (expression.text == "*" && left.integer == 1) return right;
        if (expression.text == "+" || expression.text == "*") std::swap(left, right);
      }
    }
    load_int(left, "t0");
    load_int(right, "t1");
    if (comparison) {
      if (expression.text == "==") {
        line("xor t0, t0, t1");
        line("seqz t0, t0");
      } else if (expression.text == "!=") {
        line("xor t0, t0, t1");
        line("snez t0, t0");
      } else if (expression.text == "<") line("slt t0, t0, t1");
      else if (expression.text == ">") line("slt t0, t1, t0");
      else if (expression.text == "<=") {
        line("slt t0, t1, t0");
        line("xori t0, t0, 1");
      } else {
        line("slt t0, t0, t1");
        line("xori t0, t0, 1");
      }
      return finish_int_result("t0", transient_result);
    }
    if (expression.text == "+") line("addw t0, t0, t1");
    else if (expression.text == "-") line("subw t0, t0, t1");
    else if (expression.text == "*") line("mulw t0, t0, t1");
    else if (expression.text == "/") line("divw t0, t0, t1");
    else if (expression.text == "%") line("remw t0, t0, t1");
    else fail(expression.loc, "unsupported integer operator");
    return finish_int_result("t0", transient_result);
  }

  bool emit_comparison_branch_false(const Expr& expression,
                                    const std::string& false_label) {
    if (expression.kind != ExprKind::Binary ||
        (expression.text != "==" && expression.text != "!=" && expression.text != "<" &&
         expression.text != "<=" && expression.text != ">" && expression.text != ">="))
      return false;
    Value left = emit_expr(*expression.left);
    Value right = emit_expr_transient(*expression.right);
    if (!left.type.is_scalar() || !right.type.is_scalar())
      fail(expression.loc, "binary operator requires scalar operands");
    const bool use_float = left.type.base == BaseType::Float || right.type.base == BaseType::Float;
    if (use_float) {
      const std::string left_reg =
          left.type.base == BaseType::Float && left.kind == ValueKind::FloatRegister
              ? left.label
              : "ft0";
      const std::string right_reg =
          right.type.base == BaseType::Float && right.kind == ValueKind::FloatRegister
              ? right.label
              : "ft1";
      if (left_reg == "ft0") load_float(left, left_reg);
      if (right_reg == "ft1") load_float(right, right_reg);
      if (expression.text == "==") {
        line("feq.s t0, " + left_reg + ", " + right_reg);
        line("beqz t0, " + false_label);
      } else if (expression.text == "!=") {
        line("feq.s t0, " + left_reg + ", " + right_reg);
        line("bnez t0, " + false_label);
      } else if (expression.text == "<") {
        line("flt.s t0, " + left_reg + ", " + right_reg);
        line("beqz t0, " + false_label);
      } else if (expression.text == "<=") {
        line("fle.s t0, " + left_reg + ", " + right_reg);
        line("beqz t0, " + false_label);
      } else if (expression.text == ">") {
        line("flt.s t0, " + right_reg + ", " + left_reg);
        line("beqz t0, " + false_label);
      } else {
        line("fle.s t0, " + right_reg + ", " + left_reg);
        line("beqz t0, " + false_label);
      }
      return true;
    }
    if (left.kind == ValueKind::IntRegister && right.kind == ValueKind::ImmediateInt) {
      const std::int64_t constant = right.integer;
      if (expression.text == "==" && constant == 0) {
        line("bnez " + left.label + ", " + false_label);
        return true;
      }
      if (expression.text == "!=" && constant == 0) {
        line("beqz " + left.label + ", " + false_label);
        return true;
      }
      const bool inclusive = expression.text == "<=" || expression.text == ">";
      const std::int64_t threshold = constant + (inclusive ? 1 : 0);
      if ((expression.text == "<" || expression.text == "<=" || expression.text == ">" ||
           expression.text == ">=") &&
          threshold >= -2048 && threshold <= 2047) {
        line("slti t0, " + left.label + ", " + std::to_string(threshold));
        const bool false_when_less = expression.text == ">" || expression.text == ">=";
        line(std::string(false_when_less ? "bnez t0, " : "beqz t0, ") + false_label);
        return true;
      }
    }
    const std::string left_reg =
        left.kind == ValueKind::IntRegister ? left.label : "t0";
    const std::string right_reg =
        right.kind == ValueKind::IntRegister ? right.label : "t1";
    if (left_reg == "t0") load_int(left, left_reg);
    if (right_reg == "t1") load_int(right, right_reg);
    const std::string instruction =
        expression.text == "=="   ? "bne"
        : expression.text == "!=" ? "beq"
        : expression.text == "<"  ? "bge"
        : expression.text == "<=" ? "bgt"
        : expression.text == ">"  ? "ble"
                                    : "blt";
    line(instruction + " " + left_reg + ", " + right_reg + ", " + false_label);
    return true;
  }

  void emit_cond(const Expr& expression, const std::string& true_label,
                 const std::string& false_label) {
    if (expression.kind == ExprKind::Binary && expression.text == "&&") {
      const std::string next = new_label("and_rhs");
      emit_cond(*expression.left, next, false_label);
      label(next);
      emit_cond(*expression.right, true_label, false_label);
      return;
    }
    if (expression.kind == ExprKind::Binary && expression.text == "||") {
      const std::string next = new_label("or_rhs");
      emit_cond(*expression.left, true_label, next);
      label(next);
      emit_cond(*expression.right, true_label, false_label);
      return;
    }
    if (expression.kind == ExprKind::Unary && expression.text == "!") {
      emit_cond(*expression.left, false_label, true_label);
      return;
    }
    if (emit_comparison_branch_false(expression, false_label)) {
      line("j " + true_label);
      return;
    }
    Value value = emit_expr(expression);
    if (!value.type.is_scalar()) fail(expression.loc, "condition must be scalar");
    if (value.type.base == BaseType::Float) {
      load_float(value, "ft0");
      line("fmv.w.x ft1, zero");
      line("feq.s t0, ft0, ft1");
      line("bnez t0, " + false_label);
    } else {
      load_int(value, "t0");
      line("beqz t0, " + false_label);
    }
    line("j " + true_label);
  }

  void emit_cond_fallthrough_true(const Expr& expression, const std::string& false_label) {
    if (expression.kind == ExprKind::Binary && expression.text == "&&") {
      emit_cond_fallthrough_true(*expression.left, false_label);
      emit_cond_fallthrough_true(*expression.right, false_label);
      return;
    }
    if (expression.kind == ExprKind::Binary && expression.text == "||") {
      const std::string rhs = new_label("or_rhs");
      const std::string pass = new_label("or_true");
      emit_cond(*expression.left, pass, rhs);
      label(rhs);
      emit_cond_fallthrough_true(*expression.right, false_label);
      label(pass);
      return;
    }
    if (expression.kind == ExprKind::Unary && expression.text == "!") {
      const std::string pass = new_label("not_true");
      emit_cond(*expression.left, false_label, pass);
      label(pass);
      return;
    }
    if (emit_comparison_branch_false(expression, false_label)) return;
    Value value = emit_expr(expression);
    if (!value.type.is_scalar()) fail(expression.loc, "condition must be scalar");
    if (value.type.base == BaseType::Float) {
      load_float(value, "ft0");
      line("fmv.w.x ft1, zero");
      line("feq.s t0, ft0, ft1");
      line("bnez t0, " + false_label);
    } else {
      load_int(value, "t0");
      line("beqz t0, " + false_label);
    }
  }

  bool emit_comparison_branch_true(const Expr& expression,
                                   const std::string& true_label) {
    if (expression.kind != ExprKind::Binary ||
        (expression.text != "==" && expression.text != "!=" && expression.text != "<" &&
         expression.text != "<=" && expression.text != ">" && expression.text != ">="))
      return false;
    Value left = emit_expr(*expression.left);
    Value right = emit_expr_transient(*expression.right);
    if (!left.type.is_scalar() || !right.type.is_scalar())
      fail(expression.loc, "binary operator requires scalar operands");
    const bool use_float = left.type.base == BaseType::Float || right.type.base == BaseType::Float;
    if (use_float) {
      const std::string left_reg =
          left.type.base == BaseType::Float && left.kind == ValueKind::FloatRegister
              ? left.label
              : "ft0";
      const std::string right_reg =
          right.type.base == BaseType::Float && right.kind == ValueKind::FloatRegister
              ? right.label
              : "ft1";
      if (left_reg == "ft0") load_float(left, left_reg);
      if (right_reg == "ft1") load_float(right, right_reg);
      if (expression.text == "==")
        line("feq.s t0, " + left_reg + ", " + right_reg);
      else if (expression.text == "!=") {
        line("feq.s t0, " + left_reg + ", " + right_reg);
        line("xori t0, t0, 1");
      } else if (expression.text == "<")
        line("flt.s t0, " + left_reg + ", " + right_reg);
      else if (expression.text == "<=")
        line("fle.s t0, " + left_reg + ", " + right_reg);
      else if (expression.text == ">")
        line("flt.s t0, " + right_reg + ", " + left_reg);
      else
        line("fle.s t0, " + right_reg + ", " + left_reg);
      line("bnez t0, " + true_label);
      return true;
    }
    if (left.kind == ValueKind::IntRegister && right.kind == ValueKind::ImmediateInt) {
      const std::int64_t constant = right.integer;
      if (expression.text == "==" && constant == 0) {
        line("beqz " + left.label + ", " + true_label);
        return true;
      }
      if (expression.text == "!=" && constant == 0) {
        line("bnez " + left.label + ", " + true_label);
        return true;
      }
      const bool inclusive = expression.text == "<=" || expression.text == ">";
      const std::int64_t threshold = constant + (inclusive ? 1 : 0);
      if ((expression.text == "<" || expression.text == "<=" || expression.text == ">" ||
           expression.text == ">=") &&
          threshold >= -2048 && threshold <= 2047) {
        line("slti t0, " + left.label + ", " + std::to_string(threshold));
        const bool true_when_less = expression.text == "<" || expression.text == "<=";
        line(std::string(true_when_less ? "bnez t0, " : "beqz t0, ") + true_label);
        return true;
      }
    }
    const std::string left_reg =
        left.kind == ValueKind::IntRegister ? left.label : "t0";
    const std::string right_reg =
        right.kind == ValueKind::IntRegister ? right.label : "t1";
    if (left_reg == "t0") load_int(left, left_reg);
    if (right_reg == "t1") load_int(right, right_reg);
    const std::string instruction =
        expression.text == "=="   ? "beq"
        : expression.text == "!=" ? "bne"
        : expression.text == "<"  ? "blt"
        : expression.text == "<=" ? "ble"
        : expression.text == ">"  ? "bgt"
                                    : "bge";
    line(instruction + " " + left_reg + ", " + right_reg + ", " + true_label);
    return true;
  }

  void emit_cond_branch_true(const Expr& expression, const std::string& true_label) {
    if (expression.kind == ExprKind::Binary && expression.text == "&&") {
      const std::string skip = new_label("and_false");
      emit_cond_fallthrough_true(*expression.left, skip);
      emit_cond_branch_true(*expression.right, true_label);
      label(skip);
      return;
    }
    if (expression.kind == ExprKind::Binary && expression.text == "||") {
      emit_cond_branch_true(*expression.left, true_label);
      emit_cond_branch_true(*expression.right, true_label);
      return;
    }
    if (expression.kind == ExprKind::Unary && expression.text == "!") {
      const std::string pass = new_label("not_false");
      emit_cond(*expression.left, pass, true_label);
      label(pass);
      return;
    }
    if (emit_comparison_branch_true(expression, true_label)) return;
    Value value = emit_expr(expression);
    if (!value.type.is_scalar()) fail(expression.loc, "condition must be scalar");
    if (value.type.base == BaseType::Float) {
      load_float(value, "ft0");
      line("fmv.w.x ft1, zero");
      line("feq.s t0, ft0, ft1");
      line("beqz t0, " + true_label);
    } else {
      const std::string reg = value.kind == ValueKind::IntRegister ? value.label : "t0";
      if (reg == "t0") load_int(value, reg);
      line("bnez " + reg + ", " + true_label);
    }
  }

  Value materialize_bool(const Expr& expression) {
    const std::uint64_t slot = allocate_temp();
    const std::string yes = new_label("bool_true");
    const std::string no = new_label("bool_false");
    const std::string done = new_label("bool_done");
    emit_cond(expression, yes, no);
    label(yes);
    line("li t0, 1");
    store_local("sw", "t0", slot);
    line("j " + done);
    label(no);
    store_local("sw", "zero", slot);
    label(done);
    Value result;
    result.kind = ValueKind::Local;
    result.type = Type{BaseType::Int, {}, false};
    result.offset = slot;
    return result;
  }

  struct ArgPlacement {
    enum class Kind { Gpr, Fpr, Stack } kind{Kind::Stack};
    int index{};
    std::uint64_t stack_offset{};
    bool float_bits_in_gpr{};
    bool variadic_double{};
  };

  std::vector<ArgPlacement> classify_args(const std::vector<Type>& types,
                                          std::size_t fixed_count,
                                          std::uint64_t& stack_bytes) const {
    int gpr = 0;
    int fpr = 0;
    std::uint64_t stack = 0;
    std::vector<ArgPlacement> result;
    for (std::size_t i = 0; i < types.size(); ++i) {
      const bool variadic = i >= fixed_count;
      const bool is_float = types[i].base == BaseType::Float && types[i].is_scalar();
      ArgPlacement place;
      if (is_float && !variadic && fpr < 8) {
        place.kind = ArgPlacement::Kind::Fpr;
        place.index = fpr++;
      } else if (gpr < 8) {
        place.kind = ArgPlacement::Kind::Gpr;
        place.index = gpr++;
        place.float_bits_in_gpr = is_float;
        place.variadic_double = is_float && variadic;
      } else {
        place.kind = ArgPlacement::Kind::Stack;
        place.stack_offset = stack;
        place.float_bits_in_gpr = is_float;
        place.variadic_double = is_float && variadic;
        stack += 8;
      }
      result.push_back(place);
    }
    stack_bytes = align_up(stack, 16);
    return result;
  }

  bool inline_expression_allowed(const Expr& expression,
                                 const std::unordered_set<std::string>& parameters,
                                 std::size_t& nodes) const {
    if (++nodes > 48 || expression.kind == ExprKind::Call ||
        expression.kind == ExprKind::String)
      return false;
    if (expression.kind == ExprKind::Name && !parameters.count(expression.text) &&
        !globals_by_name_.count(expression.text))
      return false;
    if (expression.left && !inline_expression_allowed(*expression.left, parameters, nodes))
      return false;
    if (expression.right && !inline_expression_allowed(*expression.right, parameters, nodes))
      return false;
    for (const auto& argument : expression.args)
      if (!inline_expression_allowed(*argument, parameters, nodes)) return false;
    return true;
  }

  const Expr* inline_return_expression(const Function& function) const {
    if (function.return_type == BaseType::Void || !function.body ||
        function.body->kind != StmtKind::Block || function.body->statements.size() != 1)
      return nullptr;
    const Stmt& statement = *function.body->statements.front();
    if (statement.kind != StmtKind::Return || !statement.expression) return nullptr;
    std::unordered_set<std::string> parameters;
    for (const ParamDecl& param : function.params) parameters.insert(param.name);
    std::size_t nodes = 0;
    return inline_expression_allowed(*statement.expression, parameters, nodes)
               ? statement.expression.get()
               : nullptr;
  }

  void collect_inline_uses(const Expr& expression,
                           std::unordered_map<std::string, std::size_t>& uses) const {
    if (expression.kind == ExprKind::Name) ++uses[expression.text];
    if (expression.left) collect_inline_uses(*expression.left, uses);
    if (expression.right) collect_inline_uses(*expression.right, uses);
    for (const auto& argument : expression.args) collect_inline_uses(*argument, uses);
  }

  bool expression_contains_call(const Expr& expression) const {
    if (expression.kind == ExprKind::Call) return true;
    if (expression.left && expression_contains_call(*expression.left)) return true;
    if (expression.right && expression_contains_call(*expression.right)) return true;
    for (const auto& argument : expression.args)
      if (expression_contains_call(*argument)) return true;
    return false;
  }

  Value emit_inline_call(const Expr& expression, const Function& function,
                         const FunctionSig& signature, const Expr& return_expression) {
    if (expression.args.size() != signature.params.size())
      fail(expression.loc, "wrong number of arguments in call to '" + function.name + "'");
    std::unordered_map<std::string, Value> bindings;
    std::unordered_map<std::string, std::size_t> uses;
    collect_inline_uses(return_expression, uses);
    std::vector<bool> later_call(expression.args.size(), false);
    bool has_later_call = false;
    for (std::size_t i = expression.args.size(); i-- > 0;) {
      later_call[i] = has_later_call;
      has_later_call = has_later_call || expression_contains_call(*expression.args[i]);
    }
    for (std::size_t i = 0; i < expression.args.size(); ++i) {
      Value actual = emit_expr(*expression.args[i]);
      const Type& expected = signature.params[i];
      if (expected.is_array_like() != actual.type.is_array_like())
        fail(expression.args[i]->loc, "argument type does not match parameter");
      if (expected.is_array_like()) {
        if (expected.base != actual.type.base)
          fail(expression.args[i]->loc, "array element type does not match parameter");
      } else {
        const bool type_conversion = expected.base != actual.type.base;
        const bool repeated = uses[function.params[i].name] > 1;
        const bool mutable_global = actual.kind == ValueKind::Global && later_call[i];
        if (type_conversion || repeated || mutable_global) {
          if (expected.base == BaseType::Float) {
            load_float(actual, "ft0");
            actual = store_float_temp("ft0", false);
          } else {
            load_int(actual, "t0");
            actual = store_int_temp("t0", false);
          }
        }
      }
      actual.type = expected;
      bindings[function.params[i].name] = actual;
    }
    inline_bindings_.push_back(std::move(bindings));
    Value result = emit_expr(return_expression);
    inline_bindings_.pop_back();
    if (signature.result.base == BaseType::Float && result.type.base != BaseType::Float) {
      load_float(result, "ft0");
      result = store_float_temp("ft0");
    } else if (signature.result.base == BaseType::Int && result.type.base != BaseType::Int) {
      load_int(result, "t0");
      result = store_int_temp("t0");
    }
    return result;
  }

  Value emit_call(const Expr& expression) {
    std::string callee = expression.text;
    auto found = functions_.find(callee);
    if (found == functions_.end()) fail(expression.loc, "undefined function '" + callee + "'");
    FunctionSig signature = found->second;
    if (options_.optimization == OptimizationLevel::O1) {
      auto definition = function_defs_.find(callee);
      if (definition != function_defs_.end()) {
        const Expr* inline_expression = inline_return_expression(*definition->second);
        if (inline_expression)
          return emit_inline_call(expression, *definition->second, signature,
                                  *inline_expression);
      }
    }
    std::vector<Value> arguments;
    std::vector<Type> argument_types;
    if (callee == "starttime" || callee == "stoptime") {
      if (!expression.args.empty()) fail(expression.loc, callee + " takes no arguments");
      arguments.push_back(immediate(make_int(static_cast<std::int32_t>(expression.loc.line))));
      argument_types.push_back(Type{BaseType::Int, {}, false});
      callee = callee == "starttime" ? "_sysy_starttime" : "_sysy_stoptime";
      signature.params = argument_types;
    } else {
      if ((!signature.variadic && expression.args.size() != signature.params.size()) ||
          (signature.variadic && expression.args.size() < signature.params.size()))
        fail(expression.loc, "wrong number of arguments in call to '" + callee + "'");
      for (const auto& argument : expression.args) {
        Value value = emit_expr(*argument);
        arguments.push_back(value);
        argument_types.push_back(value.type);
      }
      for (std::size_t i = 0; i < signature.params.size(); ++i) {
        const Type& expected = signature.params[i];
        const Type& actual = argument_types[i];
        if (expected.is_array_like() != actual.is_array_like())
          fail(expression.args[i]->loc, "argument type does not match parameter");
        if (expected.is_array_like() && expected.base != actual.base)
          fail(expression.args[i]->loc, "array element type does not match parameter");
      }
    }
    std::uint64_t stack_bytes = 0;
    std::vector<Type> passing_types = argument_types;
    for (std::size_t i = 0; i < signature.params.size() && i < passing_types.size(); ++i)
      passing_types[i] = signature.params[i];
    auto placements = classify_args(passing_types, signature.params.size(), stack_bytes);
    if (stack_bytes) {
      line("li t6, " + std::to_string(stack_bytes));
      line("sub sp, sp, t6");
    }
    for (std::size_t i = 0; i < arguments.size(); ++i) {
      const Value& value = arguments[i];
      const ArgPlacement& place = placements[i];
      const Type& passing_type = passing_types[i];
      const bool pointer = passing_type.is_array_like() || value.kind == ValueKind::String;
      if (place.kind == ArgPlacement::Kind::Fpr) {
        load_float(value, "fa" + std::to_string(place.index));
      } else if (place.kind == ArgPlacement::Kind::Gpr) {
        const std::string reg = "a" + std::to_string(place.index);
        if (pointer) {
          load_pointer(value, reg);
        } else if (place.float_bits_in_gpr) {
          load_float(value, "ft0");
          if (place.variadic_double) {
            line("fcvt.d.s ft0, ft0");
            line("fmv.x.d " + reg + ", ft0");
          } else {
            line("fmv.x.w " + reg + ", ft0");
          }
        } else {
          load_int(value, reg);
        }
      } else {
        if (pointer) {
          load_pointer(value, "t0");
          store_base_offset("sd", "t0", "sp", place.stack_offset);
        } else if (place.float_bits_in_gpr) {
          load_float(value, "ft0");
          if (place.variadic_double) {
            line("fcvt.d.s ft0, ft0");
            store_base_offset("fsd", "ft0", "sp", place.stack_offset);
          } else {
            store_base_offset("fsw", "ft0", "sp", place.stack_offset);
          }
        } else {
          load_int(value, "t0");
          store_base_offset("sd", "t0", "sp", place.stack_offset);
        }
      }
    }
    line("call " + callee);
    if (stack_bytes) {
      line("li t6, " + std::to_string(stack_bytes));
      line("add sp, sp, t6");
    }
    if (signature.result.is_void()) return Value{};
    if (signature.result.base == BaseType::Float) return store_float_temp("fa0");
    return store_int_temp("a0");
  }

  void store_to_address(const Value& address, const Value& value, Loc loc) {
    if (!address.type.dimensions.empty()) fail(loc, "cannot assign to an array");
    load_pointer(address, "t0");
    if (address.type.base == BaseType::Float) {
      load_float(value, "ft0");
      line("fsw ft0, 0(t0)");
    } else {
      load_int(value, "t1");
      line("sw t1, 0(t0)");
    }
  }

  void store_to_current_address(const Type& type, const Value& value, Loc loc) {
    if (!type.dimensions.empty()) fail(loc, "cannot assign to an array");
    if (type.base == BaseType::Float) {
      load_float(value, "ft0");
      line("fsw ft0, 0(t0)");
    } else {
      load_int(value, "t1");
      line("sw t1, 0(t0)");
    }
  }

  void store_to_symbol(Symbol& symbol, const Value& value, Loc loc) {
    if (symbol.storage == Storage::IntRegister) {
      load_int(value, symbol.reg);
      return;
    }
    if (symbol.storage == Storage::FloatRegister) {
      load_float(value, symbol.reg);
      return;
    }
    Value address = address_of_name(symbol);
    store_to_address(address, value, loc);
  }

  bool emit_register_self_update(Symbol& symbol, const std::string& name,
                                 const Expr& expression) {
    if (expression.kind != ExprKind::Binary || !expression.left || !expression.right ||
        expression.left->kind != ExprKind::Name || expression.left->text != name)
      return false;
    const std::string& op = expression.text;
    if (op != "+" && op != "-" && op != "*" && op != "/") return false;

    Value right = emit_expr_transient(*expression.right);
    if (!right.type.is_scalar()) fail(expression.loc, "binary operator requires scalar operands");
    if (symbol.storage == Storage::IntRegister && right.type.base == BaseType::Int) {
      if (op == "+" || op == "-") {
        auto constant = eval_const(*expression.right);
        if (constant && constant->base == BaseType::Int) {
          const std::int64_t delta = op == "+" ? constant->integer
                                                : -static_cast<std::int64_t>(constant->integer);
          if (delta >= -2048 && delta <= 2047) {
            line("addiw " + symbol.reg + ", " + symbol.reg + ", " +
                 std::to_string(delta));
            return true;
          }
        }
      }
      const std::string right_reg =
          right.kind == ValueKind::IntRegister ? right.label : "t0";
      if (right_reg == "t0") load_int(right, right_reg);
      const std::string instruction =
          op == "+" ? "addw" : op == "-" ? "subw" : op == "*" ? "mulw" : "divw";
      line(instruction + " " + symbol.reg + ", " + symbol.reg + ", " + right_reg);
      return true;
    }

    if (symbol.storage == Storage::FloatRegister) {
      load_float(right, "ft0");
      const std::string instruction =
          op == "+" ? "fadd.s" : op == "-" ? "fsub.s" : op == "*" ? "fmul.s" : "fdiv.s";
      line(instruction + " " + symbol.reg + ", " + symbol.reg + ", ft0");
      return true;
    }

    if (symbol.storage == Storage::IntRegister && right.type.base == BaseType::Float) {
      Value current = local_value(symbol);
      load_float(current, "ft0");
      load_float(right, "ft1");
      const std::string instruction =
          op == "+" ? "fadd.s" : op == "-" ? "fsub.s" : op == "*" ? "fmul.s" : "fdiv.s";
      line(instruction + " ft0, ft0, ft1");
      line("fcvt.w.s " + symbol.reg + ", ft0, rtz");
      return true;
    }
    return false;
  }

  struct SavedRegister {
    std::string name;
    std::uint64_t offset{};
    bool floating{};
  };

  void assign_scalar_register(Symbol& symbol, const std::string& name) {
    if (options_.optimization != OptimizationLevel::O1 || !symbol.type.is_scalar() ||
        symbol.is_const)
      return;
    std::optional<std::size_t> planned;
    if (current_register_plan_ && !name.empty()) {
      const auto& category = symbol.type.base == BaseType::Float
                                 ? current_register_plan_->floats
                                 : current_register_plan_->integers;
      auto found = category.find(name);
      if (found == category.end()) return;
      planned = found->second;
    }
    if (symbol.type.base == BaseType::Int) {
      std::size_t index = planned.value_or(11);
      if (!planned) {
        for (std::size_t candidate = 0; candidate < 11; ++candidate)
          if (!saved_int_registers_[candidate] && !planned_int_registers_.count(candidate)) {
            index = candidate;
            break;
          }
      }
      if (index >= 11) return;
      symbol.storage = Storage::IntRegister;
      symbol.reg = "s" + std::to_string(index + 1);
      if (!saved_int_registers_[index]) {
        saved_int_registers_[index] = true;
        saved_registers_.push_back(SavedRegister{symbol.reg, allocate(8), false});
      }
    } else if (symbol.type.base == BaseType::Float) {
      std::size_t index = planned.value_or(12);
      if (!planned) {
        for (std::size_t candidate = 0; candidate < 12; ++candidate)
          if (!saved_float_registers_[candidate] && !planned_float_registers_.count(candidate)) {
            index = candidate;
            break;
          }
      }
      if (index >= 12) return;
      symbol.storage = Storage::FloatRegister;
      symbol.reg = "fs" + std::to_string(index);
      if (!saved_float_registers_[index]) {
        saved_float_registers_[index] = true;
        saved_registers_.push_back(SavedRegister{symbol.reg, allocate(8), true});
      }
    }
  }

  void declare_local(const VarDecl& declaration) {
    auto& scope = scopes_.back();
    if (scope.count(declaration.name))
      fail(declaration.loc, "duplicate local declaration '" + declaration.name + "'");
    Symbol symbol;
    symbol.type = resolve_decl_type(declaration.base, declaration.dimensions);
    symbol.is_const = declaration.is_const;
    const std::uint64_t count = symbol.type.dimensions.empty() ? 1 : element_count(symbol.type);
    assign_scalar_register(symbol, declaration.name);
    if (symbol.storage == Storage::Local) symbol.offset = allocate(count * 4, 8);
    scope[declaration.name] = symbol;
    Symbol& stored = scope[declaration.name];
    if (!declaration.initializer) return;
    if (stored.type.dimensions.empty()) {
      const Expr* init = scalar_initializer(*declaration.initializer);
      if (!init) {
        store_to_symbol(stored, immediate(declaration.base == BaseType::Float ? make_float(0)
                                                                              : make_int(0)),
                        declaration.loc);
        stored.constant = declaration.base == BaseType::Float ? make_float(0) : make_int(0);
        return;
      }
      Value value = emit_expr(*init);
      store_to_symbol(stored, value, init->loc);
      auto constant = eval_const(*init);
      if (declaration.is_const && !constant)
        fail(init->loc, "const initializer must be constant");
      if (constant &&
          (declaration.is_const ||
           (options_.optimization == OptimizationLevel::O1 &&
            assigned_names_.count(declaration.name) == 0))) {
        stored.constant = convert_constant(*constant, declaration.base, init->loc);
      }
      return;
    }
    std::vector<const Expr*> flat;
    flatten_initializer(*declaration.initializer, stored.type, flat);
    Value base = address_of_name(stored);
    load_pointer(base, "a0");
    line("li a1, 0");
    line("li a2, " + std::to_string(count * 4));
    line("call memset");
    for (std::size_t i = 0; i < flat.size(); ++i) {
      if (!flat[i]) continue;
      auto constant = eval_const(*flat[i]);
      if (constant && !truthy(convert_constant(*constant, declaration.base, flat[i]->loc)))
        continue;
      Value value = emit_expr(*flat[i]);
      load_pointer(base, "t0");
      if (i != 0) {
        line("li t1, " + std::to_string(i * 4ULL));
        line("add t0, t0, t1");
      }
      if (declaration.base == BaseType::Float) {
        load_float(value, "ft0");
        line("fsw ft0, 0(t0)");
      } else {
        load_int(value, "t1");
        line("sw t1, 0(t0)");
      }
    }
  }

  Value address_of_name(const Symbol& symbol) {
    if (symbol.storage == Storage::Global)
      line("lla t0, " + symbol.label);
    else if (symbol.storage == Storage::ParamPointer)
      load_local("ld", "t0", symbol.offset);
    else
      address_from_s0(-static_cast<std::int64_t>(symbol.offset), "t0");
    return make_pointer_temp(symbol.type, "t0");
  }

  void emit_stmt(const Stmt& statement) {
    switch (statement.kind) {
      case StmtKind::Empty: return;
      case StmtKind::Expression:
        (void)emit_expr(*statement.expression);
        return;
      case StmtKind::Assignment: {
        Symbol* direct = statement.expression->kind == ExprKind::Name
                             ? find_symbol(statement.expression->text)
                             : nullptr;
        if (direct && direct->is_const)
          fail(statement.loc, "cannot assign to const '" + statement.expression->text + "'");
        if (direct) direct->constant.reset();
        auto global_binding = statement.expression->kind == ExprKind::Name
                                  ? global_register_bindings_.find(statement.expression->text)
                                  : global_register_bindings_.end();
        if (global_binding != global_register_bindings_.end()) {
          Symbol cached;
          cached.type = global_binding->second.type;
          cached.reg = global_binding->second.label;
          cached.storage = cached.type.base == BaseType::Float ? Storage::FloatRegister
                                                               : Storage::IntRegister;
          if (emit_register_self_update(cached, statement.expression->text, *statement.value))
            return;
          Value value = emit_expr(*statement.value);
          store_to_symbol(cached, value, statement.loc);
          return;
        }
        if (direct && (direct->storage == Storage::IntRegister ||
                       direct->storage == Storage::FloatRegister)) {
          if (emit_register_self_update(*direct, statement.expression->text, *statement.value))
            return;
          Value value = emit_expr(*statement.value);
          store_to_symbol(*direct, value, statement.loc);
          return;
        }
        if (!direct && statement.expression->kind == ExprKind::Subscript &&
            is_simple_index(*statement.value)) {
          Type type = address_to_register(*statement.expression);
          Value value = emit_expr(*statement.value);
          store_to_current_address(type, value, statement.loc);
          return;
        }
        Value address = address_of(*statement.expression);
        Value value = emit_expr(*statement.value);
        store_to_address(address, value, statement.loc);
        return;
      }
      case StmtKind::Declaration:
        for (const VarDecl& declaration : statement.declarations) declare_local(declaration);
        return;
      case StmtKind::Block:
        scopes_.emplace_back();
        for (const auto& child : statement.statements) emit_stmt(*child);
        scopes_.pop_back();
        return;
      case StmtKind::If: {
        const std::string yes = new_label("if_true");
        const std::string no = new_label("if_false");
        const std::string done = new_label("if_done");
        emit_cond_fallthrough_true(*statement.expression, no);
        label(yes);
        emit_stmt(*statement.first);
        line("j " + done);
        label(no);
        if (statement.second) emit_stmt(*statement.second);
        label(done);
        return;
      }
      case StmtKind::While: {
        const std::string test = new_label("while_test");
        const std::string body = new_label("while_body");
        const std::string done = new_label("while_done");
        line("j " + test);
        label(body);
        break_labels_.push_back(done);
        continue_labels_.push_back(test);
        emit_stmt(*statement.first);
        continue_labels_.pop_back();
        break_labels_.pop_back();
        label(test);
        emit_cond_branch_true(*statement.expression, body);
        label(done);
        return;
      }
      case StmtKind::Break:
        if (break_labels_.empty()) fail(statement.loc, "break is only valid inside a loop");
        line("j " + break_labels_.back());
        return;
      case StmtKind::Continue:
        if (continue_labels_.empty()) fail(statement.loc, "continue is only valid inside a loop");
        line("j " + continue_labels_.back());
        return;
      case StmtKind::Return:
        emit_return(statement);
        return;
    }
  }

  void emit_return(const Stmt& statement) {
    if (current_return_.is_void()) {
      if (statement.expression) fail(statement.loc, "void function cannot return a value");
    } else {
      if (!statement.expression) fail(statement.loc, "non-void function must return a value");
      Value value = emit_expr(*statement.expression);
      if (current_return_.base == BaseType::Float)
        load_float(value, "fa0");
      else
        load_int(value, "a0");
    }
    line("j " + epilogue_label_);
  }

  struct IncomingParam {
    Symbol* symbol{};
    Type type;
    ArgPlacement placement;
  };

  void collect_assigned_names(const Stmt& statement) {
    if (statement.kind == StmtKind::Assignment && statement.expression &&
        statement.expression->kind == ExprKind::Name)
      assigned_names_.insert(statement.expression->text);
    for (const auto& child : statement.statements)
      collect_assigned_names(*child);
    if (statement.first) collect_assigned_names(*statement.first);
    if (statement.second) collect_assigned_names(*statement.second);
  }

  void collect_declared_names(const Stmt& statement,
                              std::unordered_set<std::string>& names) const {
    for (const VarDecl& declaration : statement.declarations) names.insert(declaration.name);
    for (const auto& child : statement.statements) collect_declared_names(*child, names);
    if (statement.first) collect_declared_names(*statement.first, names);
    if (statement.second) collect_declared_names(*statement.second, names);
  }

  void collect_global_reads(const Expr& expression, std::unordered_set<std::string>& reads,
                            bool& has_call) const {
    if (expression.kind == ExprKind::Name) {
      auto global = globals_by_name_.find(expression.text);
      if (global != globals_by_name_.end() && global->second.type.is_scalar() &&
          !global->second.is_const)
        reads.insert(expression.text);
    } else if (expression.kind == ExprKind::Call) {
      auto definition = function_defs_.find(expression.text);
      if (definition == function_defs_.end() ||
          !inline_return_expression(*definition->second))
        has_call = true;
    }
    if (expression.left) collect_global_reads(*expression.left, reads, has_call);
    if (expression.right) collect_global_reads(*expression.right, reads, has_call);
    for (const auto& argument : expression.args)
      collect_global_reads(*argument, reads, has_call);
  }

  void collect_initializer_global_reads(const Initializer& initializer,
                                        std::unordered_set<std::string>& reads,
                                        bool& has_call) const {
    if (initializer.expression)
      collect_global_reads(*initializer.expression, reads, has_call);
    for (const auto& element : initializer.elements)
      collect_initializer_global_reads(*element, reads, has_call);
  }

  void collect_statement_global_reads(const Stmt& statement,
                                      std::unordered_set<std::string>& reads,
                                      bool& has_call) const {
    if (statement.expression) collect_global_reads(*statement.expression, reads, has_call);
    if (statement.value) collect_global_reads(*statement.value, reads, has_call);
    for (const VarDecl& declaration : statement.declarations) {
      for (const auto& dimension : declaration.dimensions)
        collect_global_reads(*dimension, reads, has_call);
      if (declaration.initializer)
        collect_initializer_global_reads(*declaration.initializer, reads, has_call);
    }
    for (const auto& child : statement.statements)
      collect_statement_global_reads(*child, reads, has_call);
    if (statement.first) collect_statement_global_reads(*statement.first, reads, has_call);
    if (statement.second) collect_statement_global_reads(*statement.second, reads, has_call);
  }

  void prepare_readonly_global_registers(const Function& function) {
    if (options_.optimization != OptimizationLevel::O1) return;
    std::unordered_set<std::string> declared;
    for (const ParamDecl& param : function.params) declared.insert(param.name);
    collect_declared_names(*function.body, declared);
    std::unordered_set<std::string> reads;
    bool has_call = false;
    collect_statement_global_reads(*function.body, reads, has_call);
    if (has_call) return;
    for (const std::string& name : assigned_names_) {
      auto global = globals_by_name_.find(name);
      if (!declared.count(name) && global != globals_by_name_.end() &&
          global->second.type.is_scalar() && !global->second.is_const) {
        reads.insert(name);
        global_writeback_names_.push_back(name);
      }
    }
    std::sort(global_writeback_names_.begin(), global_writeback_names_.end());
    std::vector<std::string> ordered(reads.begin(), reads.end());
    std::sort(ordered.begin(), ordered.end());
    for (const std::string& name : ordered) {
      if (declared.count(name)) continue;
      const Symbol& global = globals_by_name_.at(name);
      Symbol cached;
      cached.type = global.type;
      assign_scalar_register(cached, "");
      if (cached.storage == Storage::IntRegister) {
        load_int(local_value(global), cached.reg);
      } else if (cached.storage == Storage::FloatRegister) {
        load_float(local_value(global), cached.reg);
      } else {
        continue;
      }
      global_register_bindings_[name] = local_value(cached);
    }
  }

  void emit_param_spills(const std::vector<IncomingParam>& params, std::ostringstream& stream) {
    std::ostringstream* previous = current_stream_;
    current_stream_ = &stream;
    for (const IncomingParam& incoming : params) {
      const ArgPlacement& place = incoming.placement;
      if (place.kind == ArgPlacement::Kind::Fpr) {
        const std::string reg = "fa" + std::to_string(place.index);
        if (incoming.symbol->storage == Storage::FloatRegister)
          line("fmv.s " + incoming.symbol->reg + ", " + reg);
        else
          store_local("fsw", reg, incoming.symbol->offset);
      } else if (place.kind == ArgPlacement::Kind::Gpr) {
        const std::string reg = "a" + std::to_string(place.index);
        if (incoming.symbol->storage == Storage::FloatRegister)
          line("fmv.w.x " + incoming.symbol->reg + ", " + reg);
        else if (incoming.symbol->storage == Storage::IntRegister)
          line("mv " + incoming.symbol->reg + ", " + reg);
        else
          store_local(incoming.type.is_array_like() ? "sd" : "sw", reg, incoming.symbol->offset);
      } else {
        if (incoming.type.is_array_like()) {
          load_base_offset("ld", "t0", "s0", place.stack_offset);
          store_local("sd", "t0", incoming.symbol->offset);
        } else if (incoming.type.base == BaseType::Float) {
          load_base_offset("flw", "ft0", "s0", place.stack_offset);
          if (incoming.symbol->storage == Storage::FloatRegister)
            line("fmv.s " + incoming.symbol->reg + ", ft0");
          else
            store_local("fsw", "ft0", incoming.symbol->offset);
        } else {
          load_base_offset("lw", "t0", "s0", place.stack_offset);
          if (incoming.symbol->storage == Storage::IntRegister)
            line("mv " + incoming.symbol->reg + ", t0");
          else
            store_local("sw", "t0", incoming.symbol->offset);
        }
      }
    }
    current_stream_ = previous;
  }

  void emit_function(const Function& function) {
    current_function_ = function.name;
    current_function_loc_ = function.loc;
    current_return_ = Type{function.return_type, {}, false};
    epilogue_label_ = ".L" + function.name + "_epilogue";
    label_counter_ = 0;
    next_offset_ = 16;
    auto plan = register_plans_.find(function.name);
    current_register_plan_ = plan == register_plans_.end() ? nullptr : &plan->second;
    planned_int_registers_.clear();
    planned_float_registers_.clear();
    if (current_register_plan_) {
      for (const auto& item : current_register_plan_->integers)
        planned_int_registers_.insert(item.second);
      for (const auto& item : current_register_plan_->floats)
        planned_float_registers_.insert(item.second);
    }
    next_int_register_ = 0;
    next_float_register_ = 0;
    saved_int_registers_.fill(false);
    saved_float_registers_.fill(false);
    saved_registers_.clear();
    global_register_bindings_.clear();
    global_writeback_names_.clear();
    assigned_names_.clear();
    collect_assigned_names(*function.body);
    scopes_.clear();
    scopes_.emplace_back();
    std::ostringstream body;
    current_stream_ = &body;

    std::vector<Type> param_types;
    std::vector<Symbol*> param_symbols;
    for (const ParamDecl& param : function.params) {
      if (scopes_.back().count(param.name))
        fail(param.loc, "duplicate parameter '" + param.name + "'");
      Symbol symbol;
      symbol.type = resolve_param_type(param);
      symbol.storage = param.is_array ? Storage::ParamPointer : Storage::Local;
      assign_scalar_register(symbol, param.name);
      if (symbol.storage == Storage::Local || symbol.storage == Storage::ParamPointer)
        symbol.offset = allocate(8);
      scopes_.back()[param.name] = symbol;
      param_types.push_back(symbol.type);
      param_symbols.push_back(&scopes_.back()[param.name]);
    }
    std::uint64_t ignored_stack = 0;
    auto placements = classify_args(param_types, param_types.size(), ignored_stack);
    std::vector<IncomingParam> incoming;
    for (std::size_t i = 0; i < param_symbols.size(); ++i)
      incoming.push_back(IncomingParam{param_symbols[i], param_types[i], placements[i]});

    prepare_readonly_global_registers(function);
    emit_stmt(*function.body);
    if (function.return_type == BaseType::Float)
      line("fmv.w.x fa0, zero");
    else if (function.return_type == BaseType::Int)
      line("li a0, 0");
    line("j " + epilogue_label_);

    std::ostringstream param_stream;
    emit_param_spills(incoming, param_stream);
    std::ostringstream save_stream;
    std::ostringstream restore_stream;
    std::ostringstream global_writeback_stream;
    std::ostringstream* previous = current_stream_;
    current_stream_ = &save_stream;
    for (const SavedRegister& saved : saved_registers_)
      store_local(saved.floating ? "fsd" : "sd", saved.name, saved.offset);
    current_stream_ = &restore_stream;
    for (auto it = saved_registers_.rbegin(); it != saved_registers_.rend(); ++it)
      load_local(it->floating ? "fld" : "ld", it->name, it->offset);
    current_stream_ = &global_writeback_stream;
    for (const std::string& name : global_writeback_names_) {
      auto binding = global_register_bindings_.find(name);
      if (binding == global_register_bindings_.end()) continue;
      const Symbol& global = globals_by_name_.at(name);
      line("lla t6, " + global.label);
      if (global.type.base == BaseType::Float)
        line("fsw " + binding->second.label + ", 0(t6)");
      else
        line("sw " + binding->second.label + ", 0(t6)");
    }
    current_stream_ = previous;
    const std::uint64_t frame = align_up(next_offset_, 16);
    const std::string body_text = body.str();
    const bool leaf = body_text.find("  call ") == std::string::npos;
    output_ << ".section .text\n.align 2\n.globl " << function.name << '\n'
            << ".type " << function.name << ", @function\n" << function.name << ":\n"
            << "  li t0, " << frame << "\n"
            << "  sub sp, sp, t0\n"
            << "  add t1, sp, t0\n";
    if (!leaf) output_ << "  sd ra, -8(t1)\n";
    output_
            << "  sd s0, -16(t1)\n"
            << "  mv s0, t1\n"
            << save_stream.str() << param_stream.str() << body_text << epilogue_label_ << ":\n"
            << global_writeback_stream.str() << restore_stream.str();
    if (!leaf) output_ << "  ld ra, -8(s0)\n";
    output_ << "  ld t0, -16(s0)\n"
            << "  mv sp, s0\n"
            << "  mv s0, t0\n"
            << "  ret\n.size " << function.name << ", .-" << function.name << "\n\n";
  }

  const Program& program_;
  CompileOptions options_;
  std::unordered_map<std::string, FunctionSig> functions_;
  std::unordered_map<std::string, const Function*> function_defs_;
  std::unordered_set<std::string> user_functions_;
  std::unordered_map<std::string, Symbol> globals_by_name_;
  std::vector<GlobalObject> globals_;
  std::vector<std::unordered_map<std::string, Symbol>> scopes_;
  std::vector<std::pair<std::string, std::string>> strings_;
  std::ostringstream output_;
  std::ostringstream* current_stream_{&output_};
  std::string current_function_;
  Loc current_function_loc_;
  Type current_return_;
  std::string epilogue_label_;
  std::uint64_t next_offset_{16};
  std::uint64_t label_counter_{};
  std::vector<std::string> break_labels_;
  std::vector<std::string> continue_labels_;
  std::unordered_set<std::string> assigned_names_;
  std::size_t next_int_register_{};
  std::size_t next_float_register_{};
  std::array<bool, 11> saved_int_registers_{};
  std::array<bool, 12> saved_float_registers_{};
  std::vector<SavedRegister> saved_registers_;
  std::vector<std::unordered_map<std::string, Value>> inline_bindings_;
  std::unordered_map<std::string, Value> global_register_bindings_;
  std::vector<std::string> global_writeback_names_;
  std::unordered_map<std::string, ir::RegisterPlan> register_plans_;
  const ir::RegisterPlan* current_register_plan_{};
  std::unordered_set<std::size_t> planned_int_registers_;
  std::unordered_set<std::size_t> planned_float_registers_;
};

}  // namespace

std::string generate_rv64(const Program& program, const CompileOptions& options) {
  return Generator(program, options).run();
}

}  // namespace sysy::detail
