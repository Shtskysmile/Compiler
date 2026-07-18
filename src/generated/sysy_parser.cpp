// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "sysy_parser.hpp"


// Unqualified %code blocks.
#line 19 "src/frontend/parser.y"

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

#line 80 "src/generated/sysy_parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 4 "src/frontend/parser.y"
namespace sysy { namespace detail {
#line 173 "src/generated/sysy_parser.cpp"

  /// Build a parser object.
  GeneratedParser::GeneratedParser (ParseContext& ctx_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      ctx (ctx_yyarg)
  {}

  GeneratedParser::~GeneratedParser ()
  {}

  GeneratedParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  GeneratedParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  GeneratedParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  GeneratedParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  GeneratedParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  GeneratedParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  GeneratedParser::symbol_kind_type
  GeneratedParser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  GeneratedParser::stack_symbol_type::stack_symbol_type ()
  {}

  GeneratedParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_base_type: // base_type
        value.YY_MOVE_OR_COPY< BaseType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression: // expression
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_logical_or: // logical_or
      case symbol_kind::S_logical_and: // logical_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
        value.YY_MOVE_OR_COPY< ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_function_definition: // function_definition
        value.YY_MOVE_OR_COPY< Function > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_initializer_opt: // initializer_opt
      case symbol_kind::S_initializer: // initializer
        value.YY_MOVE_OR_COPY< InitPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parameter: // parameter
        value.YY_MOVE_OR_COPY< ParamDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_block: // block
      case symbol_kind::S_block_item: // block_item
      case symbol_kind::S_statement: // statement
        value.YY_MOVE_OR_COPY< StmtPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_declarator: // declarator
        value.YY_MOVE_OR_COPY< VarDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT: // "floating literal"
        value.YY_MOVE_OR_COPY< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INTEGER: // "integer"
        value.YY_MOVE_OR_COPY< std::int32_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string literal"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_dimensions: // dimensions
      case symbol_kind::S_trailing_dimensions: // trailing_dimensions
      case symbol_kind::S_argument_opt: // argument_opt
      case symbol_kind::S_arguments: // arguments
        value.YY_MOVE_OR_COPY< std::vector<ExprPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_initializer_items_opt: // initializer_items_opt
      case symbol_kind::S_initializer_items: // initializer_items
        value.YY_MOVE_OR_COPY< std::vector<InitPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parameter_opt: // parameter_opt
      case symbol_kind::S_parameters: // parameters
        value.YY_MOVE_OR_COPY< std::vector<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_block_items: // block_items
        value.YY_MOVE_OR_COPY< std::vector<StmtPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_declaration: // declaration
      case symbol_kind::S_declarator_list: // declarator_list
        value.YY_MOVE_OR_COPY< std::vector<VarDecl> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  GeneratedParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_base_type: // base_type
        value.move< BaseType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression: // expression
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_logical_or: // logical_or
      case symbol_kind::S_logical_and: // logical_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
        value.move< ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_function_definition: // function_definition
        value.move< Function > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_initializer_opt: // initializer_opt
      case symbol_kind::S_initializer: // initializer
        value.move< InitPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parameter: // parameter
        value.move< ParamDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_block: // block
      case symbol_kind::S_block_item: // block_item
      case symbol_kind::S_statement: // statement
        value.move< StmtPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_declarator: // declarator
        value.move< VarDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT: // "floating literal"
        value.move< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INTEGER: // "integer"
        value.move< std::int32_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string literal"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_dimensions: // dimensions
      case symbol_kind::S_trailing_dimensions: // trailing_dimensions
      case symbol_kind::S_argument_opt: // argument_opt
      case symbol_kind::S_arguments: // arguments
        value.move< std::vector<ExprPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_initializer_items_opt: // initializer_items_opt
      case symbol_kind::S_initializer_items: // initializer_items
        value.move< std::vector<InitPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parameter_opt: // parameter_opt
      case symbol_kind::S_parameters: // parameters
        value.move< std::vector<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_block_items: // block_items
        value.move< std::vector<StmtPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_declaration: // declaration
      case symbol_kind::S_declarator_list: // declarator_list
        value.move< std::vector<VarDecl> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  GeneratedParser::stack_symbol_type&
  GeneratedParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_base_type: // base_type
        value.copy< BaseType > (that.value);
        break;

      case symbol_kind::S_expression: // expression
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_logical_or: // logical_or
      case symbol_kind::S_logical_and: // logical_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
        value.copy< ExprPtr > (that.value);
        break;

      case symbol_kind::S_function_definition: // function_definition
        value.copy< Function > (that.value);
        break;

      case symbol_kind::S_initializer_opt: // initializer_opt
      case symbol_kind::S_initializer: // initializer
        value.copy< InitPtr > (that.value);
        break;

      case symbol_kind::S_parameter: // parameter
        value.copy< ParamDecl > (that.value);
        break;

      case symbol_kind::S_block: // block
      case symbol_kind::S_block_item: // block_item
      case symbol_kind::S_statement: // statement
        value.copy< StmtPtr > (that.value);
        break;

      case symbol_kind::S_declarator: // declarator
        value.copy< VarDecl > (that.value);
        break;

      case symbol_kind::S_FLOAT: // "floating literal"
        value.copy< float > (that.value);
        break;

      case symbol_kind::S_INTEGER: // "integer"
        value.copy< std::int32_t > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string literal"
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_dimensions: // dimensions
      case symbol_kind::S_trailing_dimensions: // trailing_dimensions
      case symbol_kind::S_argument_opt: // argument_opt
      case symbol_kind::S_arguments: // arguments
        value.copy< std::vector<ExprPtr> > (that.value);
        break;

      case symbol_kind::S_initializer_items_opt: // initializer_items_opt
      case symbol_kind::S_initializer_items: // initializer_items
        value.copy< std::vector<InitPtr> > (that.value);
        break;

      case symbol_kind::S_parameter_opt: // parameter_opt
      case symbol_kind::S_parameters: // parameters
        value.copy< std::vector<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_block_items: // block_items
        value.copy< std::vector<StmtPtr> > (that.value);
        break;

      case symbol_kind::S_declaration: // declaration
      case symbol_kind::S_declarator_list: // declarator_list
        value.copy< std::vector<VarDecl> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  GeneratedParser::stack_symbol_type&
  GeneratedParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_base_type: // base_type
        value.move< BaseType > (that.value);
        break;

      case symbol_kind::S_expression: // expression
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_logical_or: // logical_or
      case symbol_kind::S_logical_and: // logical_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
        value.move< ExprPtr > (that.value);
        break;

      case symbol_kind::S_function_definition: // function_definition
        value.move< Function > (that.value);
        break;

      case symbol_kind::S_initializer_opt: // initializer_opt
      case symbol_kind::S_initializer: // initializer
        value.move< InitPtr > (that.value);
        break;

      case symbol_kind::S_parameter: // parameter
        value.move< ParamDecl > (that.value);
        break;

      case symbol_kind::S_block: // block
      case symbol_kind::S_block_item: // block_item
      case symbol_kind::S_statement: // statement
        value.move< StmtPtr > (that.value);
        break;

      case symbol_kind::S_declarator: // declarator
        value.move< VarDecl > (that.value);
        break;

      case symbol_kind::S_FLOAT: // "floating literal"
        value.move< float > (that.value);
        break;

      case symbol_kind::S_INTEGER: // "integer"
        value.move< std::int32_t > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string literal"
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_dimensions: // dimensions
      case symbol_kind::S_trailing_dimensions: // trailing_dimensions
      case symbol_kind::S_argument_opt: // argument_opt
      case symbol_kind::S_arguments: // arguments
        value.move< std::vector<ExprPtr> > (that.value);
        break;

      case symbol_kind::S_initializer_items_opt: // initializer_items_opt
      case symbol_kind::S_initializer_items: // initializer_items
        value.move< std::vector<InitPtr> > (that.value);
        break;

      case symbol_kind::S_parameter_opt: // parameter_opt
      case symbol_kind::S_parameters: // parameters
        value.move< std::vector<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_block_items: // block_items
        value.move< std::vector<StmtPtr> > (that.value);
        break;

      case symbol_kind::S_declaration: // declaration
      case symbol_kind::S_declarator_list: // declarator_list
        value.move< std::vector<VarDecl> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  GeneratedParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  GeneratedParser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  GeneratedParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  GeneratedParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  GeneratedParser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  GeneratedParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  GeneratedParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  GeneratedParser::debug_level_type
  GeneratedParser::debug_level () const
  {
    return yydebug_;
  }

  void
  GeneratedParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  GeneratedParser::state_type
  GeneratedParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  GeneratedParser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  GeneratedParser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  GeneratedParser::operator() ()
  {
    return parse ();
  }

  int
  GeneratedParser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yy_lac_discard_ ("init");

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (ctx));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;

        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    yy_lac_discard_ ("shift");
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_base_type: // base_type
        yylhs.value.emplace< BaseType > ();
        break;

      case symbol_kind::S_expression: // expression
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_logical_or: // logical_or
      case symbol_kind::S_logical_and: // logical_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
        yylhs.value.emplace< ExprPtr > ();
        break;

      case symbol_kind::S_function_definition: // function_definition
        yylhs.value.emplace< Function > ();
        break;

      case symbol_kind::S_initializer_opt: // initializer_opt
      case symbol_kind::S_initializer: // initializer
        yylhs.value.emplace< InitPtr > ();
        break;

      case symbol_kind::S_parameter: // parameter
        yylhs.value.emplace< ParamDecl > ();
        break;

      case symbol_kind::S_block: // block
      case symbol_kind::S_block_item: // block_item
      case symbol_kind::S_statement: // statement
        yylhs.value.emplace< StmtPtr > ();
        break;

      case symbol_kind::S_declarator: // declarator
        yylhs.value.emplace< VarDecl > ();
        break;

      case symbol_kind::S_FLOAT: // "floating literal"
        yylhs.value.emplace< float > ();
        break;

      case symbol_kind::S_INTEGER: // "integer"
        yylhs.value.emplace< std::int32_t > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string literal"
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_dimensions: // dimensions
      case symbol_kind::S_trailing_dimensions: // trailing_dimensions
      case symbol_kind::S_argument_opt: // argument_opt
      case symbol_kind::S_arguments: // arguments
        yylhs.value.emplace< std::vector<ExprPtr> > ();
        break;

      case symbol_kind::S_initializer_items_opt: // initializer_items_opt
      case symbol_kind::S_initializer_items: // initializer_items
        yylhs.value.emplace< std::vector<InitPtr> > ();
        break;

      case symbol_kind::S_parameter_opt: // parameter_opt
      case symbol_kind::S_parameters: // parameters
        yylhs.value.emplace< std::vector<ParamDecl> > ();
        break;

      case symbol_kind::S_block_items: // block_items
        yylhs.value.emplace< std::vector<StmtPtr> > ();
        break;

      case symbol_kind::S_declaration: // declaration
      case symbol_kind::S_declarator_list: // declarator_list
        yylhs.value.emplace< std::vector<VarDecl> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 4: // external_list: external_list declaration
#line 88 "src/frontend/parser.y"
                              {
      for (auto& declaration : yystack_[0].value.as < std::vector<VarDecl> > ()) ctx.program.globals.push_back(std::move(declaration));
    }
#line 980 "src/generated/sysy_parser.cpp"
    break;

  case 5: // external_list: external_list function_definition
#line 91 "src/frontend/parser.y"
                                      { ctx.program.functions.push_back(std::move(yystack_[0].value.as < Function > ())); }
#line 986 "src/generated/sysy_parser.cpp"
    break;

  case 6: // base_type: "int"
#line 95 "src/frontend/parser.y"
        { yylhs.value.as < BaseType > () = BaseType::Int; }
#line 992 "src/generated/sysy_parser.cpp"
    break;

  case 7: // base_type: "float"
#line 96 "src/frontend/parser.y"
             { yylhs.value.as < BaseType > () = BaseType::Float; }
#line 998 "src/generated/sysy_parser.cpp"
    break;

  case 8: // declaration: base_type declarator_list ';'
#line 100 "src/frontend/parser.y"
                                  {
      yylhs.value.as < std::vector<VarDecl> > () = std::move(yystack_[1].value.as < std::vector<VarDecl> > ());
      for (auto& declaration : yylhs.value.as < std::vector<VarDecl> > ()) {
        declaration.is_const = false;
        declaration.base = yystack_[2].value.as < BaseType > ();
      }
    }
#line 1010 "src/generated/sysy_parser.cpp"
    break;

  case 9: // declaration: "const" base_type declarator_list ';'
#line 107 "src/frontend/parser.y"
                                        {
      yylhs.value.as < std::vector<VarDecl> > () = std::move(yystack_[1].value.as < std::vector<VarDecl> > ());
      for (auto& declaration : yylhs.value.as < std::vector<VarDecl> > ()) {
        declaration.is_const = true;
        declaration.base = yystack_[2].value.as < BaseType > ();
        if (!declaration.initializer)
          throw CompileError(declaration.loc, "const declaration requires an initializer");
      }
    }
#line 1024 "src/generated/sysy_parser.cpp"
    break;

  case 10: // declarator_list: declarator
#line 119 "src/frontend/parser.y"
               { yylhs.value.as < std::vector<VarDecl> > ().push_back(std::move(yystack_[0].value.as < VarDecl > ())); }
#line 1030 "src/generated/sysy_parser.cpp"
    break;

  case 11: // declarator_list: declarator_list ',' declarator
#line 120 "src/frontend/parser.y"
                                   {
      yylhs.value.as < std::vector<VarDecl> > () = std::move(yystack_[2].value.as < std::vector<VarDecl> > ());
      yylhs.value.as < std::vector<VarDecl> > ().push_back(std::move(yystack_[0].value.as < VarDecl > ()));
    }
#line 1039 "src/generated/sysy_parser.cpp"
    break;

  case 12: // declarator: "identifier" dimensions initializer_opt
#line 127 "src/frontend/parser.y"
                                          {
      ctx.count_node(ast_loc(yylhs.location));
      yylhs.value.as < VarDecl > ().loc = ast_loc(yystack_[2].location);
      yylhs.value.as < VarDecl > ().name = std::move(yystack_[2].value.as < std::string > ());
      yylhs.value.as < VarDecl > ().dimensions = std::move(yystack_[1].value.as < std::vector<ExprPtr> > ());
      yylhs.value.as < VarDecl > ().initializer = std::move(yystack_[0].value.as < InitPtr > ());
    }
#line 1051 "src/generated/sysy_parser.cpp"
    break;

  case 13: // dimensions: %empty
#line 137 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<ExprPtr> > () = std::vector<ExprPtr>(); }
#line 1057 "src/generated/sysy_parser.cpp"
    break;

  case 14: // dimensions: dimensions '[' expression ']'
#line 138 "src/frontend/parser.y"
                                  {
      yylhs.value.as < std::vector<ExprPtr> > () = std::move(yystack_[3].value.as < std::vector<ExprPtr> > ());
      yylhs.value.as < std::vector<ExprPtr> > ().push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
    }
#line 1066 "src/generated/sysy_parser.cpp"
    break;

  case 15: // initializer_opt: %empty
#line 145 "src/frontend/parser.y"
           { yylhs.value.as < InitPtr > () = nullptr; }
#line 1072 "src/generated/sysy_parser.cpp"
    break;

  case 16: // initializer_opt: '=' initializer
#line 146 "src/frontend/parser.y"
                    { yylhs.value.as < InitPtr > () = std::move(yystack_[0].value.as < InitPtr > ()); }
#line 1078 "src/generated/sysy_parser.cpp"
    break;

  case 17: // initializer: expression
#line 150 "src/frontend/parser.y"
               {
      yylhs.value.as < InitPtr > () = node<Initializer>(ctx, yylhs.location);
      yylhs.value.as < InitPtr > ()->expression = std::move(yystack_[0].value.as < ExprPtr > ());
    }
#line 1087 "src/generated/sysy_parser.cpp"
    break;

  case 18: // initializer: '{' initializer_items_opt '}'
#line 154 "src/frontend/parser.y"
                                  {
      yylhs.value.as < InitPtr > () = node<Initializer>(ctx, yylhs.location);
      yylhs.value.as < InitPtr > ()->elements = std::move(yystack_[1].value.as < std::vector<InitPtr> > ());
    }
#line 1096 "src/generated/sysy_parser.cpp"
    break;

  case 19: // initializer_items_opt: %empty
#line 161 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<InitPtr> > () = std::vector<InitPtr>(); }
#line 1102 "src/generated/sysy_parser.cpp"
    break;

  case 20: // initializer_items_opt: initializer_items
#line 162 "src/frontend/parser.y"
                      { yylhs.value.as < std::vector<InitPtr> > () = std::move(yystack_[0].value.as < std::vector<InitPtr> > ()); }
#line 1108 "src/generated/sysy_parser.cpp"
    break;

  case 21: // initializer_items: initializer
#line 166 "src/frontend/parser.y"
                { yylhs.value.as < std::vector<InitPtr> > ().push_back(std::move(yystack_[0].value.as < InitPtr > ())); }
#line 1114 "src/generated/sysy_parser.cpp"
    break;

  case 22: // initializer_items: initializer_items ',' initializer
#line 167 "src/frontend/parser.y"
                                      {
      yylhs.value.as < std::vector<InitPtr> > () = std::move(yystack_[2].value.as < std::vector<InitPtr> > ());
      yylhs.value.as < std::vector<InitPtr> > ().push_back(std::move(yystack_[0].value.as < InitPtr > ()));
    }
#line 1123 "src/generated/sysy_parser.cpp"
    break;

  case 23: // function_definition: base_type "identifier" '(' parameter_opt ')' block
#line 174 "src/frontend/parser.y"
                                                     {
      ctx.count_node(ast_loc(yylhs.location));
      yylhs.value.as < Function > ().loc = ast_loc(yystack_[4].location);
      yylhs.value.as < Function > ().return_type = yystack_[5].value.as < BaseType > ();
      yylhs.value.as < Function > ().name = std::move(yystack_[4].value.as < std::string > ());
      yylhs.value.as < Function > ().params = std::move(yystack_[2].value.as < std::vector<ParamDecl> > ());
      yylhs.value.as < Function > ().body = std::move(yystack_[0].value.as < StmtPtr > ());
    }
#line 1136 "src/generated/sysy_parser.cpp"
    break;

  case 24: // function_definition: "void" "identifier" '(' parameter_opt ')' block
#line 182 "src/frontend/parser.y"
                                                {
      ctx.count_node(ast_loc(yylhs.location));
      yylhs.value.as < Function > ().loc = ast_loc(yystack_[4].location);
      yylhs.value.as < Function > ().return_type = BaseType::Void;
      yylhs.value.as < Function > ().name = std::move(yystack_[4].value.as < std::string > ());
      yylhs.value.as < Function > ().params = std::move(yystack_[2].value.as < std::vector<ParamDecl> > ());
      yylhs.value.as < Function > ().body = std::move(yystack_[0].value.as < StmtPtr > ());
    }
#line 1149 "src/generated/sysy_parser.cpp"
    break;

  case 25: // parameter_opt: %empty
#line 193 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 1155 "src/generated/sysy_parser.cpp"
    break;

  case 26: // parameter_opt: parameters
#line 194 "src/frontend/parser.y"
               { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[0].value.as < std::vector<ParamDecl> > ()); }
#line 1161 "src/generated/sysy_parser.cpp"
    break;

  case 27: // parameters: parameter
#line 198 "src/frontend/parser.y"
              { yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(yystack_[0].value.as < ParamDecl > ())); }
#line 1167 "src/generated/sysy_parser.cpp"
    break;

  case 28: // parameters: parameters ',' parameter
#line 199 "src/frontend/parser.y"
                             {
      yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[2].value.as < std::vector<ParamDecl> > ());
      yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(yystack_[0].value.as < ParamDecl > ()));
    }
#line 1176 "src/generated/sysy_parser.cpp"
    break;

  case 29: // parameter: base_type "identifier"
#line 206 "src/frontend/parser.y"
                         {
      ctx.count_node(ast_loc(yylhs.location));
      yylhs.value.as < ParamDecl > ().loc = ast_loc(yystack_[0].location);
      yylhs.value.as < ParamDecl > ().base = yystack_[1].value.as < BaseType > ();
      yylhs.value.as < ParamDecl > ().name = std::move(yystack_[0].value.as < std::string > ());
    }
#line 1187 "src/generated/sysy_parser.cpp"
    break;

  case 30: // parameter: base_type "identifier" '[' ']' trailing_dimensions
#line 212 "src/frontend/parser.y"
                                                     {
      ctx.count_node(ast_loc(yylhs.location));
      yylhs.value.as < ParamDecl > ().loc = ast_loc(yystack_[3].location);
      yylhs.value.as < ParamDecl > ().base = yystack_[4].value.as < BaseType > ();
      yylhs.value.as < ParamDecl > ().name = std::move(yystack_[3].value.as < std::string > ());
      yylhs.value.as < ParamDecl > ().is_array = true;
      yylhs.value.as < ParamDecl > ().trailing_dimensions = std::move(yystack_[0].value.as < std::vector<ExprPtr> > ());
    }
#line 1200 "src/generated/sysy_parser.cpp"
    break;

  case 31: // trailing_dimensions: %empty
#line 223 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<ExprPtr> > () = std::vector<ExprPtr>(); }
#line 1206 "src/generated/sysy_parser.cpp"
    break;

  case 32: // trailing_dimensions: trailing_dimensions '[' expression ']'
#line 224 "src/frontend/parser.y"
                                           {
      yylhs.value.as < std::vector<ExprPtr> > () = std::move(yystack_[3].value.as < std::vector<ExprPtr> > ());
      yylhs.value.as < std::vector<ExprPtr> > ().push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
    }
#line 1215 "src/generated/sysy_parser.cpp"
    break;

  case 33: // block: '{' block_items '}'
#line 231 "src/frontend/parser.y"
                        {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Block;
      yylhs.value.as < StmtPtr > ()->statements = std::move(yystack_[1].value.as < std::vector<StmtPtr> > ());
    }
#line 1225 "src/generated/sysy_parser.cpp"
    break;

  case 34: // block_items: %empty
#line 239 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<StmtPtr> > () = std::vector<StmtPtr>(); }
#line 1231 "src/generated/sysy_parser.cpp"
    break;

  case 35: // block_items: block_items block_item
#line 240 "src/frontend/parser.y"
                           {
      yylhs.value.as < std::vector<StmtPtr> > () = std::move(yystack_[1].value.as < std::vector<StmtPtr> > ());
      yylhs.value.as < std::vector<StmtPtr> > ().push_back(std::move(yystack_[0].value.as < StmtPtr > ()));
    }
#line 1240 "src/generated/sysy_parser.cpp"
    break;

  case 36: // block_item: declaration
#line 247 "src/frontend/parser.y"
                {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Declaration;
      yylhs.value.as < StmtPtr > ()->declarations = std::move(yystack_[0].value.as < std::vector<VarDecl> > ());
    }
#line 1250 "src/generated/sysy_parser.cpp"
    break;

  case 37: // block_item: statement
#line 252 "src/frontend/parser.y"
              { yylhs.value.as < StmtPtr > () = std::move(yystack_[0].value.as < StmtPtr > ()); }
#line 1256 "src/generated/sysy_parser.cpp"
    break;

  case 38: // statement: ';'
#line 256 "src/frontend/parser.y"
        {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Empty;
    }
#line 1265 "src/generated/sysy_parser.cpp"
    break;

  case 39: // statement: expression ';'
#line 260 "src/frontend/parser.y"
                   {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      if (yystack_[1].value.as < ExprPtr > ()->kind == ExprKind::Binary && yystack_[1].value.as < ExprPtr > ()->text == "=") {
        yylhs.value.as < StmtPtr > ()->kind = StmtKind::Assignment;
        yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[1].value.as < ExprPtr > ()->left);
        yylhs.value.as < StmtPtr > ()->value = std::move(yystack_[1].value.as < ExprPtr > ()->right);
      } else {
        yylhs.value.as < StmtPtr > ()->kind = StmtKind::Expression;
        yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[1].value.as < ExprPtr > ());
      }
    }
#line 1281 "src/generated/sysy_parser.cpp"
    break;

  case 40: // statement: block
#line 271 "src/frontend/parser.y"
          { yylhs.value.as < StmtPtr > () = std::move(yystack_[0].value.as < StmtPtr > ()); }
#line 1287 "src/generated/sysy_parser.cpp"
    break;

  case 41: // statement: "if" '(' expression ')' statement
#line 272 "src/frontend/parser.y"
                                                          {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::If;
      yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[2].value.as < ExprPtr > ());
      yylhs.value.as < StmtPtr > ()->first = std::move(yystack_[0].value.as < StmtPtr > ());
    }
#line 1298 "src/generated/sysy_parser.cpp"
    break;

  case 42: // statement: "if" '(' expression ')' statement "else" statement
#line 278 "src/frontend/parser.y"
                                                   {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::If;
      yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[4].value.as < ExprPtr > ());
      yylhs.value.as < StmtPtr > ()->first = std::move(yystack_[2].value.as < StmtPtr > ());
      yylhs.value.as < StmtPtr > ()->second = std::move(yystack_[0].value.as < StmtPtr > ());
    }
#line 1310 "src/generated/sysy_parser.cpp"
    break;

  case 43: // statement: "while" '(' expression ')' statement
#line 285 "src/frontend/parser.y"
                                       {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::While;
      yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[2].value.as < ExprPtr > ());
      yylhs.value.as < StmtPtr > ()->first = std::move(yystack_[0].value.as < StmtPtr > ());
    }
#line 1321 "src/generated/sysy_parser.cpp"
    break;

  case 44: // statement: "break" ';'
#line 291 "src/frontend/parser.y"
              {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Break;
    }
#line 1330 "src/generated/sysy_parser.cpp"
    break;

  case 45: // statement: "continue" ';'
#line 295 "src/frontend/parser.y"
                 {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Continue;
    }
#line 1339 "src/generated/sysy_parser.cpp"
    break;

  case 46: // statement: "return" ';'
#line 299 "src/frontend/parser.y"
               {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Return;
    }
#line 1348 "src/generated/sysy_parser.cpp"
    break;

  case 47: // statement: "return" expression ';'
#line 303 "src/frontend/parser.y"
                          {
      yylhs.value.as < StmtPtr > () = node<Stmt>(ctx, yylhs.location);
      yylhs.value.as < StmtPtr > ()->kind = StmtKind::Return;
      yylhs.value.as < StmtPtr > ()->expression = std::move(yystack_[1].value.as < ExprPtr > ());
    }
#line 1358 "src/generated/sysy_parser.cpp"
    break;

  case 48: // expression: assignment
#line 311 "src/frontend/parser.y"
               { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1364 "src/generated/sysy_parser.cpp"
    break;

  case 49: // assignment: logical_or
#line 315 "src/frontend/parser.y"
               { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1370 "src/generated/sysy_parser.cpp"
    break;

  case 50: // assignment: logical_or '=' assignment
#line 316 "src/frontend/parser.y"
                              { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "=", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1376 "src/generated/sysy_parser.cpp"
    break;

  case 51: // logical_or: logical_and
#line 320 "src/frontend/parser.y"
                { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1382 "src/generated/sysy_parser.cpp"
    break;

  case 52: // logical_or: logical_or "||" logical_and
#line 321 "src/frontend/parser.y"
                              { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "||", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1388 "src/generated/sysy_parser.cpp"
    break;

  case 53: // logical_and: equality
#line 325 "src/frontend/parser.y"
             { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1394 "src/generated/sysy_parser.cpp"
    break;

  case 54: // logical_and: logical_and "&&" equality
#line 326 "src/frontend/parser.y"
                             { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "&&", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1400 "src/generated/sysy_parser.cpp"
    break;

  case 55: // equality: relational
#line 330 "src/frontend/parser.y"
               { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1406 "src/generated/sysy_parser.cpp"
    break;

  case 56: // equality: equality "==" relational
#line 331 "src/frontend/parser.y"
                           { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "==", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1412 "src/generated/sysy_parser.cpp"
    break;

  case 57: // equality: equality "!=" relational
#line 332 "src/frontend/parser.y"
                           { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "!=", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1418 "src/generated/sysy_parser.cpp"
    break;

  case 58: // relational: additive
#line 336 "src/frontend/parser.y"
             { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1424 "src/generated/sysy_parser.cpp"
    break;

  case 59: // relational: relational '<' additive
#line 337 "src/frontend/parser.y"
                            { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "<", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1430 "src/generated/sysy_parser.cpp"
    break;

  case 60: // relational: relational '>' additive
#line 338 "src/frontend/parser.y"
                            { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, ">", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1436 "src/generated/sysy_parser.cpp"
    break;

  case 61: // relational: relational "<=" additive
#line 339 "src/frontend/parser.y"
                           { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "<=", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1442 "src/generated/sysy_parser.cpp"
    break;

  case 62: // relational: relational ">=" additive
#line 340 "src/frontend/parser.y"
                           { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, ">=", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1448 "src/generated/sysy_parser.cpp"
    break;

  case 63: // additive: multiplicative
#line 344 "src/frontend/parser.y"
                   { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1454 "src/generated/sysy_parser.cpp"
    break;

  case 64: // additive: additive '+' multiplicative
#line 345 "src/frontend/parser.y"
                                { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "+", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1460 "src/generated/sysy_parser.cpp"
    break;

  case 65: // additive: additive '-' multiplicative
#line 346 "src/frontend/parser.y"
                                { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "-", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1466 "src/generated/sysy_parser.cpp"
    break;

  case 66: // multiplicative: unary
#line 350 "src/frontend/parser.y"
          { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1472 "src/generated/sysy_parser.cpp"
    break;

  case 67: // multiplicative: multiplicative '*' unary
#line 351 "src/frontend/parser.y"
                             { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "*", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1478 "src/generated/sysy_parser.cpp"
    break;

  case 68: // multiplicative: multiplicative '/' unary
#line 352 "src/frontend/parser.y"
                             { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "/", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1484 "src/generated/sysy_parser.cpp"
    break;

  case 69: // multiplicative: multiplicative '%' unary
#line 353 "src/frontend/parser.y"
                             { yylhs.value.as < ExprPtr > () = binary(ctx, yystack_[1].location, "%", std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1490 "src/generated/sysy_parser.cpp"
    break;

  case 70: // unary: postfix
#line 357 "src/frontend/parser.y"
            { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1496 "src/generated/sysy_parser.cpp"
    break;

  case 71: // unary: '+' unary
#line 358 "src/frontend/parser.y"
              {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Unary; yylhs.value.as < ExprPtr > ()->text = "+"; yylhs.value.as < ExprPtr > ()->left = std::move(yystack_[0].value.as < ExprPtr > ());
    }
#line 1504 "src/generated/sysy_parser.cpp"
    break;

  case 72: // unary: '-' unary
#line 361 "src/frontend/parser.y"
              {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Unary; yylhs.value.as < ExprPtr > ()->text = "-"; yylhs.value.as < ExprPtr > ()->left = std::move(yystack_[0].value.as < ExprPtr > ());
    }
#line 1512 "src/generated/sysy_parser.cpp"
    break;

  case 73: // unary: '!' unary
#line 364 "src/frontend/parser.y"
              {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Unary; yylhs.value.as < ExprPtr > ()->text = "!"; yylhs.value.as < ExprPtr > ()->left = std::move(yystack_[0].value.as < ExprPtr > ());
    }
#line 1520 "src/generated/sysy_parser.cpp"
    break;

  case 74: // postfix: primary
#line 370 "src/frontend/parser.y"
            { yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ()); }
#line 1526 "src/generated/sysy_parser.cpp"
    break;

  case 75: // postfix: postfix '[' expression ']'
#line 371 "src/frontend/parser.y"
                               {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location);
      yylhs.value.as < ExprPtr > ()->kind = ExprKind::Subscript;
      yylhs.value.as < ExprPtr > ()->left = std::move(yystack_[3].value.as < ExprPtr > ());
      yylhs.value.as < ExprPtr > ()->right = std::move(yystack_[1].value.as < ExprPtr > ());
    }
#line 1537 "src/generated/sysy_parser.cpp"
    break;

  case 76: // primary: "integer"
#line 380 "src/frontend/parser.y"
            {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Int; yylhs.value.as < ExprPtr > ()->int_value = yystack_[0].value.as < std::int32_t > ();
    }
#line 1545 "src/generated/sysy_parser.cpp"
    break;

  case 77: // primary: "floating literal"
#line 383 "src/frontend/parser.y"
          {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Float; yylhs.value.as < ExprPtr > ()->float_value = yystack_[0].value.as < float > ();
    }
#line 1553 "src/generated/sysy_parser.cpp"
    break;

  case 78: // primary: "string literal"
#line 386 "src/frontend/parser.y"
           {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::String; yylhs.value.as < ExprPtr > ()->text = std::move(yystack_[0].value.as < std::string > ());
    }
#line 1561 "src/generated/sysy_parser.cpp"
    break;

  case 79: // primary: "identifier"
#line 389 "src/frontend/parser.y"
               {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location); yylhs.value.as < ExprPtr > ()->kind = ExprKind::Name; yylhs.value.as < ExprPtr > ()->text = std::move(yystack_[0].value.as < std::string > ());
    }
#line 1569 "src/generated/sysy_parser.cpp"
    break;

  case 80: // primary: "identifier" '(' argument_opt ')'
#line 392 "src/frontend/parser.y"
                                    {
      yylhs.value.as < ExprPtr > () = node<Expr>(ctx, yylhs.location);
      yylhs.value.as < ExprPtr > ()->kind = ExprKind::Call;
      yylhs.value.as < ExprPtr > ()->text = std::move(yystack_[3].value.as < std::string > ());
      yylhs.value.as < ExprPtr > ()->args = std::move(yystack_[1].value.as < std::vector<ExprPtr> > ());
    }
#line 1580 "src/generated/sysy_parser.cpp"
    break;

  case 81: // primary: '(' expression ')'
#line 398 "src/frontend/parser.y"
                       { yylhs.value.as < ExprPtr > () = std::move(yystack_[1].value.as < ExprPtr > ()); }
#line 1586 "src/generated/sysy_parser.cpp"
    break;

  case 82: // argument_opt: %empty
#line 402 "src/frontend/parser.y"
           { yylhs.value.as < std::vector<ExprPtr> > () = std::vector<ExprPtr>(); }
#line 1592 "src/generated/sysy_parser.cpp"
    break;

  case 83: // argument_opt: arguments
#line 403 "src/frontend/parser.y"
              { yylhs.value.as < std::vector<ExprPtr> > () = std::move(yystack_[0].value.as < std::vector<ExprPtr> > ()); }
#line 1598 "src/generated/sysy_parser.cpp"
    break;

  case 84: // arguments: expression
#line 407 "src/frontend/parser.y"
               { yylhs.value.as < std::vector<ExprPtr> > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ())); }
#line 1604 "src/generated/sysy_parser.cpp"
    break;

  case 85: // arguments: arguments ',' expression
#line 408 "src/frontend/parser.y"
                             {
      yylhs.value.as < std::vector<ExprPtr> > () = std::move(yystack_[2].value.as < std::vector<ExprPtr> > ());
      yylhs.value.as < std::vector<ExprPtr> > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
    }
#line 1613 "src/generated/sysy_parser.cpp"
    break;


#line 1617 "src/generated/sysy_parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      yy_lac_discard_ ("error recovery");
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  GeneratedParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  GeneratedParser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "identifier", "integer",
  "floating literal", "string literal", "const", "int", "float", "void",
  "if", "else", "while", "break", "continue", "return", "==", "!=", "<=",
  ">=", "&&", "||", "LOWER_THAN_ELSE", "';'", "','", "'['", "']'", "'='",
  "'{'", "'}'", "'('", "')'", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'",
  "'%'", "'!'", "$accept", "translation_unit", "external_list",
  "base_type", "declaration", "declarator_list", "declarator",
  "dimensions", "initializer_opt", "initializer", "initializer_items_opt",
  "initializer_items", "function_definition", "parameter_opt",
  "parameters", "parameter", "trailing_dimensions", "block", "block_items",
  "block_item", "statement", "expression", "assignment", "logical_or",
  "logical_and", "equality", "relational", "additive", "multiplicative",
  "unary", "postfix", "primary", "argument_opt", "arguments", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // GeneratedParser::context.
  GeneratedParser::context::context (const GeneratedParser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  GeneratedParser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

#if YYDEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST (symbol_kind_type, yyx);
        if (yysym != symbol_kind::S_YYerror
            && yysym != symbol_kind::S_YYUNDEF
            && yyparser_.yy_lac_check_ (yysym))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
      }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }




  bool
  GeneratedParser::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear ();
    // Reduce until we encounter a shift and thereby accept the token.
#if YYDEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name (yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
      {
        state_type top_state = (yylac_stack_.empty ()
                                ? yystack_[lac_top].state
                                : yylac_stack_.back ());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_ (yyrule)
            || (yyrule += yytoken) < 0 || yylast_ < yyrule
            || yycheck_[yyrule] != yytoken)
          {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
              {
                YYCDEBUG << " Err\n";
                return false;
              }
          }
        else
          {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_ (yyrule))
              {
                YYCDEBUG << " Err\n";
                return false;
              }
            if (0 < yyrule)
              {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
              }
            yyrule = -yyrule;
          }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
          std::ptrdiff_t yylen = yyr2_[yyrule];
          // First pop from the LAC stack as many tokens as possible.
          std::ptrdiff_t lac_size = std::ptrdiff_t (yylac_stack_.size ());
          if (yylen < lac_size)
            {
              yylac_stack_.resize (std::size_t (lac_size - yylen));
              yylen = 0;
            }
          else if (lac_size)
            {
              yylac_stack_.clear ();
              yylen -= lac_size;
            }
          // Only afterwards look at the main stack.
          // We simulate popping elements by incrementing lac_top.
          lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty ()
                     ? yystack_[lac_top].state
                     : yylac_stack_.back ());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_ (top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int (state);
        yylac_stack_.push_back (state);
      }
  }

  // Establish the initial context if no initial context currently exists.
  bool
  GeneratedParser::yy_lac_establish_ (symbol_kind_type yytoken)
  {
    /* Establish the initial context for the current lookahead if no initial
       context is currently established.

       We define a context as a snapshot of the parser stacks.  We define
       the initial context for a lookahead as the context in which the
       parser initially examines that lookahead in order to select a
       syntactic action.  Thus, if the lookahead eventually proves
       syntactically unacceptable (possibly in a later context reached via a
       series of reductions), the initial context can be used to determine
       the exact set of tokens that would be syntactically acceptable in the
       lookahead's place.  Moreover, it is the context after which any
       further semantic actions would be erroneous because they would be
       determined by a syntactically unacceptable token.

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (yy_lac_established_)
      return true;
    else
      {
#if YYDEBUG
        YYCDEBUG << "LAC: initial context established for "
                 << symbol_name (yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_ (yytoken);
      }
  }

  // Discard any previous initial lookahead context.
  void
  GeneratedParser::yy_lac_discard_ (const char* event)
  {
   /* Discard any previous initial lookahead context because of Event,
      which may be a lookahead change or an invalidation of the currently
      established initial context for the current lookahead.

      The most common example of a lookahead change is a shift.  An example
      of both cases is syntax error recovery.  That is, a syntax error
      occurs when the lookahead is syntactically erroneous for the
      currently established initial context, so error recovery manipulates
      the parser stacks to try to find a new initial context in which the
      current lookahead is syntactically acceptable.  If it fails to find
      such a context, it discards the lookahead.  */
    if (yy_lac_established_)
      {
        YYCDEBUG << "LAC: initial context discarded due to "
                 << event << '\n';
        yy_lac_established_ = false;
      }
  }


  int
  GeneratedParser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  GeneratedParser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short GeneratedParser::yypact_ninf_ = -134;

  const signed char GeneratedParser::yytable_ninf_ = -1;

  const short
  GeneratedParser::yypact_[] =
  {
    -134,    33,    21,  -134,    45,  -134,  -134,    37,    39,  -134,
    -134,    47,    25,    34,    -4,  -134,  -134,    48,    45,    45,
       6,  -134,    47,  -134,    64,    46,    63,  -134,    62,   122,
      12,  -134,  -134,    54,    67,    45,    67,    68,  -134,  -134,
    -134,   122,   122,   122,   122,    71,  -134,    -9,    86,    57,
     -11,    41,   -13,  -134,    82,  -134,    12,  -134,  -134,    83,
    -134,  -134,  -134,  -134,   122,    77,  -134,  -134,  -134,  -134,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,  -134,    91,    87,  -134,    55,
    -134,    90,    98,  -134,    86,  -134,    57,   -11,   -11,    41,
      41,    41,    41,   -13,   -13,  -134,  -134,  -134,   103,  -134,
      12,   106,   102,   108,   110,   113,   114,  -134,  -134,    47,
    -134,  -134,  -134,  -134,   117,  -134,   122,  -134,  -134,   122,
     122,   122,  -134,  -134,  -134,   118,  -134,  -134,   116,   112,
     115,  -134,  -134,   100,   100,   134,  -134,   100,  -134
  };

  const signed char
  GeneratedParser::yydefact_[] =
  {
       3,     0,     2,     1,     0,     6,     7,     0,     0,     4,
       5,     0,     0,    13,     0,    10,    13,     0,    25,    25,
      15,     8,     0,     9,     0,     0,    26,    27,     0,     0,
       0,    12,    11,    29,     0,     0,     0,    79,    76,    77,
      78,     0,     0,     0,     0,     0,    48,    49,    51,    53,
      55,    58,    63,    66,    70,    74,    19,    16,    17,     0,
      34,    24,    28,    23,    82,     0,    71,    72,    73,    14,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,    20,    31,     0,
      84,     0,    83,    81,    52,    50,    54,    56,    57,    61,
      62,    59,    60,    64,    65,    67,    68,    69,     0,    18,
       0,    30,     0,     0,     0,     0,     0,    38,    33,     0,
      36,    40,    35,    37,     0,    80,     0,    75,    22,     0,
       0,     0,    44,    45,    46,     0,    39,    85,     0,     0,
       0,    47,    32,     0,     0,    41,    43,     0,    42
  };

  const short
  GeneratedParser::yypgoto_[] =
  {
    -134,  -134,  -134,     0,    59,   140,   130,  -134,  -134,   -53,
    -134,  -134,  -134,   136,  -134,   121,  -134,    15,  -134,  -134,
    -133,   -29,    88,  -134,    93,    89,     9,   -39,    13,   -37,
    -134,  -134,  -134,  -134
  };

  const signed char
  GeneratedParser::yydefgoto_[] =
  {
       0,     1,     2,    24,     9,    14,    15,    20,    31,    57,
      86,    87,    10,    25,    26,    27,   111,   121,    89,   122,
     123,   124,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    91,    92
  };

  const unsigned char
  GeneratedParser::yytable_[] =
  {
      45,    58,     8,    85,    11,    66,    67,    68,    75,    76,
     145,   146,    65,    70,   148,    37,    38,    39,    40,    71,
      21,    22,    77,    78,    81,    82,    83,    58,     4,     5,
       6,     7,    29,     3,    30,    90,    99,   100,   101,   102,
      12,    56,    13,    41,   105,   106,   107,    42,    43,    61,
      16,    63,    44,     5,     6,   108,    18,   128,    37,    38,
      39,    40,     4,     5,     6,    19,   112,    33,   113,   114,
     115,   116,    23,    22,    73,    74,    79,    80,    34,   117,
      59,    58,    97,    98,    60,   118,    41,   135,    35,   119,
      42,    43,   103,   104,    36,    44,    60,   137,    69,    64,
     138,   139,   140,    37,    38,    39,    40,    72,    84,    93,
      88,   112,   110,   113,   114,   115,   116,    37,    38,    39,
      40,   109,   125,   126,   117,    37,    38,    39,    40,    60,
     127,    41,   129,   130,   132,    42,    43,   133,   134,   131,
      44,   136,   141,   142,   143,    41,   147,   144,   120,    42,
      43,    17,    32,    41,    44,    28,    62,    42,    43,    95,
       0,    96,    44,    94
  };

  const short
  GeneratedParser::yycheck_[] =
  {
      29,    30,     2,    56,     4,    42,    43,    44,    19,    20,
     143,   144,    41,    22,   147,     3,     4,     5,     6,    28,
      24,    25,    33,    34,    37,    38,    39,    56,     7,     8,
       9,    10,    26,     0,    28,    64,    75,    76,    77,    78,
       3,    29,     3,    31,    81,    82,    83,    35,    36,    34,
       3,    36,    40,     8,     9,    84,    31,   110,     3,     4,
       5,     6,     7,     8,     9,    31,    11,     3,    13,    14,
      15,    16,    24,    25,    17,    18,    35,    36,    32,    24,
      26,   110,    73,    74,    29,    30,    31,   116,    25,    89,
      35,    36,    79,    80,    32,    40,    29,   126,    27,    31,
     129,   130,   131,     3,     4,     5,     6,    21,    26,    32,
      27,    11,    25,    13,    14,    15,    16,     3,     4,     5,
       6,    30,    32,    25,    24,     3,     4,     5,     6,    29,
      27,    31,    26,    31,    24,    35,    36,    24,    24,    31,
      40,    24,    24,    27,    32,    31,    12,    32,    89,    35,
      36,    11,    22,    31,    40,    19,    35,    35,    36,    71,
      -1,    72,    40,    70
  };

  const signed char
  GeneratedParser::yystos_[] =
  {
       0,    42,    43,     0,     7,     8,     9,    10,    44,    45,
      53,    44,     3,     3,    46,    47,     3,    46,    31,    31,
      48,    24,    25,    24,    44,    54,    55,    56,    54,    26,
      28,    49,    47,     3,    32,    25,    32,     3,     4,     5,
       6,    31,    35,    36,    40,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    29,    50,    62,    26,
      29,    58,    56,    58,    31,    62,    70,    70,    70,    27,
      22,    28,    21,    17,    18,    19,    20,    33,    34,    35,
      36,    37,    38,    39,    26,    50,    51,    52,    27,    59,
      62,    73,    74,    32,    65,    63,    66,    67,    67,    68,
      68,    68,    68,    69,    69,    70,    70,    70,    62,    30,
      25,    57,    11,    13,    14,    15,    16,    24,    30,    44,
      45,    58,    60,    61,    62,    32,    25,    27,    50,    26,
      31,    31,    24,    24,    24,    62,    24,    62,    62,    62,
      62,    24,    27,    32,    32,    61,    61,    12,    61
  };

  const signed char
  GeneratedParser::yyr1_[] =
  {
       0,    41,    42,    43,    43,    43,    44,    44,    45,    45,
      46,    46,    47,    48,    48,    49,    49,    50,    50,    51,
      51,    52,    52,    53,    53,    54,    54,    55,    55,    56,
      56,    57,    57,    58,    59,    59,    60,    60,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    62,    63,
      63,    64,    64,    65,    65,    66,    66,    66,    67,    67,
      67,    67,    67,    68,    68,    68,    69,    69,    69,    69,
      70,    70,    70,    70,    71,    71,    72,    72,    72,    72,
      72,    72,    73,    73,    74,    74
  };

  const signed char
  GeneratedParser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     2,     1,     1,     3,     4,
       1,     3,     3,     0,     4,     0,     2,     1,     3,     0,
       1,     1,     3,     6,     6,     0,     1,     1,     3,     2,
       5,     0,     4,     3,     0,     2,     1,     1,     1,     2,
       1,     5,     7,     5,     2,     2,     2,     3,     1,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       1,     2,     2,     2,     1,     4,     1,     1,     1,     1,
       4,     3,     0,     1,     1,     3
  };




#if YYDEBUG
  const short
  GeneratedParser::yyrline_[] =
  {
       0,    83,    83,    87,    88,    91,    95,    96,   100,   107,
     119,   120,   127,   137,   138,   145,   146,   150,   154,   161,
     162,   166,   167,   174,   182,   193,   194,   198,   199,   206,
     212,   223,   224,   231,   239,   240,   247,   252,   256,   260,
     271,   272,   278,   285,   291,   295,   299,   303,   311,   315,
     316,   320,   321,   325,   326,   330,   331,   332,   336,   337,
     338,   339,   340,   344,   345,   346,   350,   351,   352,   353,
     357,   358,   361,   364,   370,   371,   380,   383,   386,   389,
     392,   398,   402,   403,   407,   408
  };

  void
  GeneratedParser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  GeneratedParser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 4 "src/frontend/parser.y"
} } // sysy::detail
#line 2284 "src/generated/sysy_parser.cpp"

#line 414 "src/frontend/parser.y"


void sysy::detail::GeneratedParser::error(const location_type& where,
                                          const std::string& message) {
  throw CompileError(ast_loc(where), message);
}
