%option c++ noyywrap nodefault nounput noinput batch yyclass="sysy::detail::FlexScanner"
%x COMMENT

%{
#include "frontend/scanner.hpp"

#define YY_USER_ACTION begin_token(YYText(), YYLeng());

using sysy::detail::CompileError;
using sysy::detail::RawToken;
%}

DIGIT       [0-9]
HEX         [0-9a-fA-F]
ID_START    [A-Za-z_]
ID_CONT     [A-Za-z0-9_]
DEC_EXP     [eE][+-]?{DIGIT}+
HEX_EXP     [pP][+-]?{DIGIT}+
DEC_FLOAT   (({DIGIT}+\.{DIGIT}*|\.{DIGIT}+){DEC_EXP}?|{DIGIT}+{DEC_EXP})
HEX_FLOAT   0[xX](({HEX}+\.{HEX}*|{HEX}*\.{HEX}+|{HEX}+)){HEX_EXP}

%%

[ \t\r\n]+                 { }
"//"[^\n]*                 { }
"/*"                       { BEGIN(COMMENT); }
<COMMENT>"*/"              { BEGIN(INITIAL); }
<COMMENT>[^*\n]+            { }
<COMMENT>"*"                { }
<COMMENT>\n                 { }
<COMMENT><<EOF>>            { throw CompileError(token_loc(), "unterminated block comment"); }

"const"                    { return static_cast<int>(RawToken::KwConst); }
"int"                      { return static_cast<int>(RawToken::KwInt); }
"float"                    { return static_cast<int>(RawToken::KwFloat); }
"void"                     { return static_cast<int>(RawToken::KwVoid); }
"if"                       { return static_cast<int>(RawToken::KwIf); }
"else"                     { return static_cast<int>(RawToken::KwElse); }
"while"                    { return static_cast<int>(RawToken::KwWhile); }
"break"                    { return static_cast<int>(RawToken::KwBreak); }
"continue"                 { return static_cast<int>(RawToken::KwContinue); }
"return"                   { return static_cast<int>(RawToken::KwReturn); }

"=="                       { return static_cast<int>(RawToken::Equal); }
"!="                       { return static_cast<int>(RawToken::NotEqual); }
"<="                       { return static_cast<int>(RawToken::LessEqual); }
">="                       { return static_cast<int>(RawToken::GreaterEqual); }
"&&"                       { return static_cast<int>(RawToken::AndAnd); }
"||"                       { return static_cast<int>(RawToken::OrOr); }

{HEX_FLOAT}                 { return static_cast<int>(RawToken::Float); }
{DEC_FLOAT}                 { return static_cast<int>(RawToken::Float); }
0[xX]{HEX}+                 { return static_cast<int>(RawToken::Integer); }
0[0-7]*                     { return static_cast<int>(RawToken::Integer); }
[1-9]{DIGIT}*               { return static_cast<int>(RawToken::Integer); }
0[0-9]+                     { throw CompileError(token_loc(), "invalid octal integer literal"); }
0[xX][A-Za-z0-9.]*          { throw CompileError(token_loc(), "invalid hexadecimal literal"); }

{ID_START}{ID_CONT}*        {
                              if (static_cast<std::size_t>(YYLeng()) > sysy::detail::kMaxIdentifierBytes)
                                throw CompileError(token_loc(), "identifier is too long");
                              return static_cast<int>(RawToken::Identifier);
                            }

\"([^\\\n\"]|\\.)*\"    { return static_cast<int>(RawToken::String); }
\"([^\\\n\"]|\\.)*       { throw CompileError(token_loc(), "unterminated string literal"); }

[(){}\[\],;+*/%=<>!-]      { return static_cast<unsigned char>(YYText()[0]); }
\.                          { throw CompileError(token_loc(), "unexpected character '.'"); }
.                           { throw CompileError(token_loc(), std::string("unexpected character '") + YYText()[0] + "'"); }
<<EOF>>                     { return static_cast<int>(RawToken::End); }

%%
