#include "../internal.hpp"
#include "scanner.hpp"
#include "../generated/sysy_parser.hpp"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace sysy::detail {
namespace {

std::string unescape_string(const RawLexeme& raw) {
  std::string result;
  for (std::size_t i = 1; i + 1 < raw.text.size(); ++i) {
    char c = raw.text[i];
    if (c != '\\') {
      result.push_back(c);
      continue;
    }
    if (++i + 1 > raw.text.size()) throw CompileError(raw.loc, "invalid string escape");
    switch (raw.text[i]) {
      case 'n': result.push_back('\n'); break;
      case 'r': result.push_back('\r'); break;
      case 't': result.push_back('\t'); break;
      case '\\': result.push_back('\\'); break;
      case '"': result.push_back('"'); break;
      default: throw CompileError(raw.loc, "unsupported string escape");
    }
  }
  return result;
}

GeneratedParser::location_type parser_loc(Loc loc, std::size_t length = 1) {
  GeneratedParser::location_type result;
  result.begin.line = static_cast<int>(loc.line);
  result.begin.column = static_cast<int>(loc.column);
  result.end = result.begin;
  result.end.column += static_cast<int>(length);
  return result;
}

std::int32_t parse_integer(const RawLexeme& raw) {
  errno = 0;
  char* end = nullptr;
  const int base = raw.text.size() > 2 && raw.text[0] == '0' &&
                           (raw.text[1] == 'x' || raw.text[1] == 'X')
                       ? 16
                       : (raw.text.size() > 1 && raw.text[0] == '0' ? 8 : 10);
  const unsigned long long value = std::strtoull(raw.text.c_str(), &end, base);
  if (errno == ERANGE || end != raw.text.c_str() + raw.text.size() ||
      value > std::numeric_limits<std::uint32_t>::max())
    throw CompileError(raw.loc, "integer literal is out of 32-bit range");
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

float parse_float(const RawLexeme& raw) {
  errno = 0;
  char* end = nullptr;
  const float value = std::strtof(raw.text.c_str(), &end);
  if (errno == ERANGE || end != raw.text.c_str() + raw.text.size())
    throw CompileError(raw.loc, "floating literal is out of range");
  return value;
}

}  // namespace

ParseContext::ParseContext(std::string_view source) {
  input = std::make_unique<std::istringstream>(std::string(source));
  scanner = std::make_unique<FlexScanner>(input.get());
}

ParseContext::~ParseContext() = default;

void ParseContext::count_node(Loc loc) {
  if (++ast_nodes > kMaxAstNodes) throw CompileError(loc, "program is too complex");
}

Program Parser::parse_program() {
  ParseContext context(source_);
  GeneratedParser parser(context);
  if (parser.parse() != 0) throw CompileError({1, 1}, "parse failed");
  if (context.program.functions.empty() && context.program.globals.empty())
    throw CompileError({1, 1}, "translation unit is empty");
  return std::move(context.program);
}

RawLexeme FlexScanner::next() {
  const int raw = yylex();
  return RawLexeme{static_cast<RawToken>(raw), token_loc_, YYText() ? YYText() : ""};
}

void FlexScanner::begin_token(const char* text, int length) {
  token_loc_ = {line_, column_};
  for (int i = 0; i < length; ++i) {
    if (text[i] == '\n') {
      ++line_;
      column_ = 1;
    } else {
      ++column_;
    }
  }
}

GeneratedParser::symbol_type lex_symbol(ParseContext& ctx) {
  using P = GeneratedParser;
  using R = RawToken;
  RawLexeme raw = ctx.scanner->next();
  auto location = parser_loc(raw.loc, raw.text.size());
  switch (raw.token) {
    case R::End: return P::make_END(location);
    case R::Identifier: return P::make_IDENTIFIER(std::move(raw.text), location);
    case R::Integer: return P::make_INTEGER(parse_integer(raw), location);
    case R::Float: return P::make_FLOAT(parse_float(raw), location);
    case R::String: return P::make_STRING(unescape_string(raw), location);
    case R::KwConst: return P::make_CONST(location);
    case R::KwInt: return P::make_INT(location);
    case R::KwFloat: return P::make_FLOAT_KW(location);
    case R::KwVoid: return P::make_VOID(location);
    case R::KwIf: return P::make_IF(location);
    case R::KwElse: return P::make_ELSE(location);
    case R::KwWhile: return P::make_WHILE(location);
    case R::KwBreak: return P::make_BREAK(location);
    case R::KwContinue: return P::make_CONTINUE(location);
    case R::KwReturn: return P::make_RETURN(location);
    case R::Equal: return P::make_EQ(location);
    case R::NotEqual: return P::make_NE(location);
    case R::LessEqual: return P::make_LE(location);
    case R::GreaterEqual: return P::make_GE(location);
    case R::AndAnd: return P::make_AND(location);
    case R::OrOr: return P::make_OR(location);
    default: return P::symbol_type(static_cast<P::token_kind_type>(static_cast<int>(raw.token)), location);
  }
}

}  // namespace sysy::detail

sysy::detail::GeneratedParser::symbol_type yylex(sysy::detail::ParseContext& ctx) {
  return sysy::detail::lex_symbol(ctx);
}
