#pragma once

#include "internal.hpp"

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif
#include <istream>
#include <string>

namespace sysy::detail {

enum class RawToken : int {
  End = 0,
  Identifier = 256,
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
  Equal,
  NotEqual,
  LessEqual,
  GreaterEqual,
  AndAnd,
  OrOr,
};

struct RawLexeme {
  RawToken token{RawToken::End};
  Loc loc;
  std::string text;
};

class FlexScanner final : public yyFlexLexer {
 public:
  explicit FlexScanner(std::istream* input) : yyFlexLexer(input) {}
  RawLexeme next();
  int yylex() override;

  void begin_token(const char* text, int length);
  Loc token_loc() const { return token_loc_; }

 private:
  Loc token_loc_{};
  std::size_t line_{1};
  std::size_t column_{1};
};

}  // namespace sysy::detail
