%skeleton "lalr1.cc"
%require "3.8"
%language "c++"
%define api.namespace {sysy::detail}
%define api.parser.class {GeneratedParser}
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
%define parse.lac full
%locations

%code requires {
#include "internal.hpp"
}

%parse-param { ParseContext& ctx }
%lex-param { ParseContext& ctx }

%code {
#include <utility>

sysy::detail::GeneratedParser::symbol_type yylex(sysy::detail::ParseContext& ctx);

namespace {
sysy::detail::Loc ast_loc(const sysy::detail::GeneratedParser::location_type& value) {
  return {static_cast<std::size_t>(value.begin.line),
          static_cast<std::size_t>(value.begin.column)};
}

template <class T>
std::unique_ptr<T> node(sysy::detail::ParseContext& ctx,
                        const sysy::detail::GeneratedParser::location_type& where) {
  ctx.count_node(ast_loc(where));
  auto result = std::make_unique<T>();
  result->loc = ast_loc(where);
  return result;
}

sysy::detail::ExprPtr binary(sysy::detail::ParseContext& ctx,
                             const sysy::detail::GeneratedParser::location_type& where,
                             std::string op, sysy::detail::ExprPtr lhs,
                             sysy::detail::ExprPtr rhs) {
  auto result = node<sysy::detail::Expr>(ctx, where);
  result->kind = sysy::detail::ExprKind::Binary;
  result->text = std::move(op);
  result->left = std::move(lhs);
  result->right = std::move(rhs);
  return result;
}
}
}

%token END 0 "end of file"
%token <std::string> IDENTIFIER "identifier"
%token <std::int32_t> INTEGER "integer"
%token <float> FLOAT "floating literal"
%token <std::string> STRING "string literal"
%token CONST "const" INT "int" FLOAT_KW "float" VOID "void"
%token IF "if" ELSE "else" WHILE "while" BREAK "break" CONTINUE "continue" RETURN "return"
%token EQ "==" NE "!=" LE "<=" GE ">=" AND "&&" OR "||"

%precedence LOWER_THAN_ELSE
%precedence ELSE

%type <BaseType> base_type
%type <std::vector<VarDecl>> declaration declarator_list
%type <VarDecl> declarator
%type <std::vector<ExprPtr>> dimensions trailing_dimensions arguments argument_opt
%type <InitPtr> initializer initializer_opt
%type <std::vector<InitPtr>> initializer_items initializer_items_opt
%type <Function> function_definition
%type <ParamDecl> parameter
%type <std::vector<ParamDecl>> parameters parameter_opt
%type <StmtPtr> block statement block_item
%type <std::vector<StmtPtr>> block_items
%type <ExprPtr> expression assignment logical_or logical_and equality relational additive multiplicative unary primary postfix

%start translation_unit

%%

translation_unit
  : external_list
  ;

external_list
  : %empty
  | external_list declaration {
      for (auto& declaration : $2) ctx.program.globals.push_back(std::move(declaration));
    }
  | external_list function_definition { ctx.program.functions.push_back(std::move($2)); }
  ;

base_type
  : INT { $$ = BaseType::Int; }
  | FLOAT_KW { $$ = BaseType::Float; }
  ;

declaration
  : base_type declarator_list ';' {
      $$ = std::move($2);
      for (auto& declaration : $$) {
        declaration.is_const = false;
        declaration.base = $1;
      }
    }
  | CONST base_type declarator_list ';' {
      $$ = std::move($3);
      for (auto& declaration : $$) {
        declaration.is_const = true;
        declaration.base = $2;
        if (!declaration.initializer)
          throw CompileError(declaration.loc, "const declaration requires an initializer");
      }
    }
  ;

declarator_list
  : declarator { $$.push_back(std::move($1)); }
  | declarator_list ',' declarator {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

declarator
  : IDENTIFIER dimensions initializer_opt {
      ctx.count_node(ast_loc(@$));
      $$.loc = ast_loc(@1);
      $$.name = std::move($1);
      $$.dimensions = std::move($2);
      $$.initializer = std::move($3);
    }
  ;

dimensions
  : %empty { $$ = std::vector<ExprPtr>(); }
  | dimensions '[' expression ']' {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

initializer_opt
  : %empty { $$ = nullptr; }
  | '=' initializer { $$ = std::move($2); }
  ;

initializer
  : expression {
      $$ = node<Initializer>(ctx, @$);
      $$->expression = std::move($1);
    }
  | '{' initializer_items_opt '}' {
      $$ = node<Initializer>(ctx, @$);
      $$->elements = std::move($2);
    }
  ;

initializer_items_opt
  : %empty { $$ = std::vector<InitPtr>(); }
  | initializer_items { $$ = std::move($1); }
  ;

initializer_items
  : initializer { $$.push_back(std::move($1)); }
  | initializer_items ',' initializer {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

function_definition
  : base_type IDENTIFIER '(' parameter_opt ')' block {
      ctx.count_node(ast_loc(@$));
      $$.loc = ast_loc(@2);
      $$.return_type = $1;
      $$.name = std::move($2);
      $$.params = std::move($4);
      $$.body = std::move($6);
    }
  | VOID IDENTIFIER '(' parameter_opt ')' block {
      ctx.count_node(ast_loc(@$));
      $$.loc = ast_loc(@2);
      $$.return_type = BaseType::Void;
      $$.name = std::move($2);
      $$.params = std::move($4);
      $$.body = std::move($6);
    }
  ;

parameter_opt
  : %empty { $$ = std::vector<ParamDecl>(); }
  | parameters { $$ = std::move($1); }
  ;

parameters
  : parameter { $$.push_back(std::move($1)); }
  | parameters ',' parameter {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

parameter
  : base_type IDENTIFIER {
      ctx.count_node(ast_loc(@$));
      $$.loc = ast_loc(@2);
      $$.base = $1;
      $$.name = std::move($2);
    }
  | base_type IDENTIFIER '[' ']' trailing_dimensions {
      ctx.count_node(ast_loc(@$));
      $$.loc = ast_loc(@2);
      $$.base = $1;
      $$.name = std::move($2);
      $$.is_array = true;
      $$.trailing_dimensions = std::move($5);
    }
  ;

trailing_dimensions
  : %empty { $$ = std::vector<ExprPtr>(); }
  | trailing_dimensions '[' expression ']' {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

block
  : '{' block_items '}' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Block;
      $$->statements = std::move($2);
    }
  ;

block_items
  : %empty { $$ = std::vector<StmtPtr>(); }
  | block_items block_item {
      $$ = std::move($1);
      $$.push_back(std::move($2));
    }
  ;

block_item
  : declaration {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Declaration;
      $$->declarations = std::move($1);
    }
  | statement { $$ = std::move($1); }
  ;

statement
  : ';' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Empty;
    }
  | expression ';' {
      $$ = node<Stmt>(ctx, @$);
      if ($1->kind == ExprKind::Binary && $1->text == "=") {
        $$->kind = StmtKind::Assignment;
        $$->expression = std::move($1->left);
        $$->value = std::move($1->right);
      } else {
        $$->kind = StmtKind::Expression;
        $$->expression = std::move($1);
      }
    }
  | block { $$ = std::move($1); }
  | IF '(' expression ')' statement %prec LOWER_THAN_ELSE {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::If;
      $$->expression = std::move($3);
      $$->first = std::move($5);
    }
  | IF '(' expression ')' statement ELSE statement {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::If;
      $$->expression = std::move($3);
      $$->first = std::move($5);
      $$->second = std::move($7);
    }
  | WHILE '(' expression ')' statement {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::While;
      $$->expression = std::move($3);
      $$->first = std::move($5);
    }
  | BREAK ';' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Break;
    }
  | CONTINUE ';' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Continue;
    }
  | RETURN ';' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Return;
    }
  | RETURN expression ';' {
      $$ = node<Stmt>(ctx, @$);
      $$->kind = StmtKind::Return;
      $$->expression = std::move($2);
    }
  ;

expression
  : assignment { $$ = std::move($1); }
  ;

assignment
  : logical_or { $$ = std::move($1); }
  | logical_or '=' assignment { $$ = binary(ctx, @2, "=", std::move($1), std::move($3)); }
  ;

logical_or
  : logical_and { $$ = std::move($1); }
  | logical_or OR logical_and { $$ = binary(ctx, @2, "||", std::move($1), std::move($3)); }
  ;

logical_and
  : equality { $$ = std::move($1); }
  | logical_and AND equality { $$ = binary(ctx, @2, "&&", std::move($1), std::move($3)); }
  ;

equality
  : relational { $$ = std::move($1); }
  | equality EQ relational { $$ = binary(ctx, @2, "==", std::move($1), std::move($3)); }
  | equality NE relational { $$ = binary(ctx, @2, "!=", std::move($1), std::move($3)); }
  ;

relational
  : additive { $$ = std::move($1); }
  | relational '<' additive { $$ = binary(ctx, @2, "<", std::move($1), std::move($3)); }
  | relational '>' additive { $$ = binary(ctx, @2, ">", std::move($1), std::move($3)); }
  | relational LE additive { $$ = binary(ctx, @2, "<=", std::move($1), std::move($3)); }
  | relational GE additive { $$ = binary(ctx, @2, ">=", std::move($1), std::move($3)); }
  ;

additive
  : multiplicative { $$ = std::move($1); }
  | additive '+' multiplicative { $$ = binary(ctx, @2, "+", std::move($1), std::move($3)); }
  | additive '-' multiplicative { $$ = binary(ctx, @2, "-", std::move($1), std::move($3)); }
  ;

multiplicative
  : unary { $$ = std::move($1); }
  | multiplicative '*' unary { $$ = binary(ctx, @2, "*", std::move($1), std::move($3)); }
  | multiplicative '/' unary { $$ = binary(ctx, @2, "/", std::move($1), std::move($3)); }
  | multiplicative '%' unary { $$ = binary(ctx, @2, "%", std::move($1), std::move($3)); }
  ;

unary
  : postfix { $$ = std::move($1); }
  | '+' unary {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Unary; $$->text = "+"; $$->left = std::move($2);
    }
  | '-' unary {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Unary; $$->text = "-"; $$->left = std::move($2);
    }
  | '!' unary {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Unary; $$->text = "!"; $$->left = std::move($2);
    }
  ;

postfix
  : primary { $$ = std::move($1); }
  | postfix '[' expression ']' {
      $$ = node<Expr>(ctx, @$);
      $$->kind = ExprKind::Subscript;
      $$->left = std::move($1);
      $$->right = std::move($3);
    }
  ;

primary
  : INTEGER {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Int; $$->int_value = $1;
    }
  | FLOAT {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Float; $$->float_value = $1;
    }
  | STRING {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::String; $$->text = std::move($1);
    }
  | IDENTIFIER {
      $$ = node<Expr>(ctx, @$); $$->kind = ExprKind::Name; $$->text = std::move($1);
    }
  | IDENTIFIER '(' argument_opt ')' {
      $$ = node<Expr>(ctx, @$);
      $$->kind = ExprKind::Call;
      $$->text = std::move($1);
      $$->args = std::move($3);
    }
  | '(' expression ')' { $$ = std::move($2); }
  ;

argument_opt
  : %empty { $$ = std::vector<ExprPtr>(); }
  | arguments { $$ = std::move($1); }
  ;

arguments
  : expression { $$.push_back(std::move($1)); }
  | arguments ',' expression {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

%%

void sysy::detail::GeneratedParser::error(const location_type& where,
                                          const std::string& message) {
  throw CompileError(ast_loc(where), message);
}
