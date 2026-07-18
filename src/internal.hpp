#pragma once

#include "../include/sysy/compiler.hpp"

#include <cstdint>
#include <memory>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sysy::detail {

constexpr std::size_t kMaxSourceBytes = 64U * 1024U * 1024U;
constexpr std::size_t kMaxIdentifierBytes = 1024U * 1024U;
constexpr std::size_t kMaxAstNodes = 5U * 1024U * 1024U;
constexpr std::uint64_t kMaxObjectBytes = 1ULL << 31;

struct Loc {
  std::size_t line{1};
  std::size_t column{1};
};

class CompileError final : public std::runtime_error {
 public:
  CompileError(Loc where, std::string message)
      : std::runtime_error(std::move(message)), where_(where) {}
  Loc where() const noexcept { return where_; }

 private:
  Loc where_;
};

enum class TokenKind {
  End,
  Identifier,
  Integer,
  Float,
  String,
  KwConst,
  KwInt,
  KwFloat,
  KwVoid,
  KwIf,
  KwElse,
  KwWhile,
  KwBreak,
  KwContinue,
  KwReturn,
  LParen,
  RParen,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  Comma,
  Semicolon,
  Plus,
  Minus,
  Star,
  Slash,
  Percent,
  Assign,
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  Not,
  AndAnd,
  OrOr,
};

struct Token {
  TokenKind kind{TokenKind::End};
  Loc loc;
  std::string text;
  std::int32_t int_value{};
  float float_value{};
};

enum class BaseType { Int, Float, Void, VectorReserved };

struct Expr;
struct Initializer;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using InitPtr = std::unique_ptr<Initializer>;
using StmtPtr = std::unique_ptr<Stmt>;

enum class ExprKind { Int, Float, String, Name, Unary, Binary, Call, Subscript };

struct Expr {
  ExprKind kind{ExprKind::Int};
  Loc loc;
  std::int32_t int_value{};
  float float_value{};
  std::string text;
  ExprPtr left;
  ExprPtr right;
  std::vector<ExprPtr> args;
};

struct Initializer {
  Loc loc;
  ExprPtr expression;
  std::vector<InitPtr> elements;
  bool is_list() const noexcept { return !expression; }
};

struct VarDecl {
  Loc loc;
  BaseType base{BaseType::Int};
  bool is_const{};
  std::string name;
  std::vector<ExprPtr> dimensions;
  InitPtr initializer;
};

struct ParamDecl {
  Loc loc;
  BaseType base{BaseType::Int};
  std::string name;
  bool is_array{};
  std::vector<ExprPtr> trailing_dimensions;
};

enum class StmtKind {
  Empty,
  Expression,
  Assignment,
  Declaration,
  Block,
  If,
  While,
  Break,
  Continue,
  Return,
};

struct Stmt {
  StmtKind kind{StmtKind::Empty};
  Loc loc;
  ExprPtr expression;
  ExprPtr value;
  std::vector<VarDecl> declarations;
  std::vector<StmtPtr> statements;
  StmtPtr first;
  StmtPtr second;
};

struct Function {
  Loc loc;
  BaseType return_type{BaseType::Int};
  std::string name;
  std::vector<ParamDecl> params;
  StmtPtr body;
};

struct Program {
  std::vector<VarDecl> globals;
  std::vector<Function> functions;
};

class FlexScanner;

struct ParseContext {
  Program program;
  std::unique_ptr<std::istream> input;
  std::unique_ptr<FlexScanner> scanner;
  std::size_t ast_nodes{};

  explicit ParseContext(std::string_view source);
  ~ParseContext();
  void count_node(Loc loc);
};

class Parser {
 public:
  explicit Parser(std::string_view source) : source_(source) {}
  Program parse_program();

 private:
  std::string source_;
};

std::string generate_rv64(const Program& program, const CompileOptions& options);

}  // namespace sysy::detail
