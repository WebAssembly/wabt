/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         WABT_WAST_PARSER_STYPE
#define YYLTYPE         WABT_WAST_PARSER_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         wabt_wast_parser_parse
#define yylex           wabt_wast_parser_lex
#define yyerror         wabt_wast_parser_error
#define yydebug         wabt_wast_parser_debug
#define yynerrs         wabt_wast_parser_nerrs


/* Copy the first part of user declarations.  */
#line 17 "src/wast-parser.y" /* yacc.c:339  */

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <utility>

#include "binary-error-handler.h"
#include "binary-reader.h"
#include "binary-reader-ir.h"
#include "literal.h"
#include "wast-parser.h"
#include "wast-parser-lexer-shared.h"

#define YYDEBUG 1

#define RELOCATE_STACK(type, array, stack_base, old_size, new_size)   \
  do {                                                                \
    type* new_stack = new type[new_size]();                           \
    std::move((stack_base), (stack_base) + (old_size), (new_stack));  \
    if ((stack_base) != (array)) {                                    \
      delete[](stack_base);                                           \
    } else {                                                          \
      for (size_t i = 0; i < (old_size); ++i) {                       \
        (stack_base)[i].~type();                                      \
      }                                                               \
    }                                                                 \
    /* Cache the pointer in the parser struct to be deleted later. */ \
    parser->array = (stack_base) = new_stack;                         \
  } while (0)

#define yyoverflow(message, ss, ss_size, vs, vs_size, ls, ls_size, new_size) \
  do {                                                                       \
    size_t old_size = *(new_size);                                           \
    *(new_size) *= 2;                                                        \
    RELOCATE_STACK(yytype_int16, yyssa, *(ss), old_size, *(new_size));       \
    RELOCATE_STACK(YYSTYPE, yyvsa, *(vs), old_size, *(new_size));            \
    RELOCATE_STACK(YYLTYPE, yylsa, *(ls), old_size, *(new_size));            \
  } while (0)

#define DUPTEXT(dst, src)                                \
  (dst).start = wabt_strndup((src).start, (src).length); \
  (dst).length = (src).length

#define YYLLOC_DEFAULT(Current, Rhs, N)                       \
  do                                                          \
    if (N) {                                                  \
      (Current).filename = YYRHSLOC(Rhs, 1).filename;         \
      (Current).line = YYRHSLOC(Rhs, 1).line;                 \
      (Current).first_column = YYRHSLOC(Rhs, 1).first_column; \
      if (YYRHSLOC(Rhs, N).line == (Current).line)            \
        (Current).last_column = YYRHSLOC(Rhs, N).last_column; \
      else                                                    \
        (Current).last_column = YYRHSLOC(Rhs, 1).last_column; \
    } else {                                                  \
      (Current).filename = nullptr;                           \
      (Current).line = YYRHSLOC(Rhs, 0).line;                 \
      (Current).first_column = (Current).last_column =        \
          YYRHSLOC(Rhs, 0).last_column;                       \
    }                                                         \
  while (0)

#define CHECK_END_LABEL(loc, begin_label, end_label)                       \
  do {                                                                     \
    if (!string_slice_is_empty(&(end_label))) {                            \
      if (string_slice_is_empty(&(begin_label))) {                         \
        wast_parser_error(&loc, lexer, parser,                             \
                          "unexpected label \"" PRIstringslice "\"",       \
                          WABT_PRINTF_STRING_SLICE_ARG(end_label));        \
      } else if (!string_slices_are_equal(&(begin_label), &(end_label))) { \
        wast_parser_error(&loc, lexer, parser,                             \
                          "mismatching label \"" PRIstringslice            \
                          "\" != \"" PRIstringslice "\"",                  \
                          WABT_PRINTF_STRING_SLICE_ARG(begin_label),       \
                          WABT_PRINTF_STRING_SLICE_ARG(end_label));        \
      }                                                                    \
      destroy_string_slice(&(end_label));                                  \
    }                                                                      \
  } while (0)

#define CHECK_ALLOW_EXCEPTIONS(loc, opcode_name)                       \
  do {                                                                 \
    if (!parser->options->allow_exceptions) {                          \
      wast_parser_error(loc, lexer, parser, "opcode not allowed: %s",  \
                        opcode_name);                                  \
    }                                                                  \
 } while (0)

#define YYMALLOC(size) new char [size]
#define YYFREE(p) delete [] (p)

#define USE_NATURAL_ALIGNMENT (~0)

namespace wabt {

static ExprList join_exprs1(Location* loc, Expr* expr1);
static ExprList join_exprs2(Location* loc, ExprList* expr1, Expr* expr2);
<<<<<<< HEAD
static ExprList join_expr_lists(Location* loc, ExprList* expr1,
                                ExprList* expr2);
static ExprList prepend_expr(Location* loc, Expr* expr1, Expr* expr2);
=======
static ExprList join_expr_lists(ExprList* expr1, ExprList* expr2);
>>>>>>> master

static Result parse_const(Type type,
                          LiteralType literal_type,
                          const char* s,
                          const char* end,
                          Const* out);
static void dup_text_list(TextList* text_list,
                          char** out_data,
                          size_t* out_size);

static void reverse_bindings(TypeVector*, BindingHash*);

static bool is_empty_signature(const FuncSignature* sig);

static void check_import_ordering(Location* loc,
                                  WastLexer* lexer,
                                  WastParser* parser,
                                  Module* module,
                                  ModuleField* first);
static void append_module_fields(Module*, ModuleField*);

class BinaryErrorHandlerModule : public BinaryErrorHandler {
 public:
  BinaryErrorHandlerModule(Location* loc, WastLexer* lexer, WastParser* parser);
  bool OnError(Offset offset, const std::string& error) override;

 private:
  Location* loc_;
  WastLexer* lexer_;
  WastParser* parser_;
};

#define wabt_wast_parser_lex(...) lexer->GetToken(__VA_ARGS__, parser)
#define wabt_wast_parser_error wast_parser_error


<<<<<<< HEAD
#line 202 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:339  */
=======
#line 210 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:339  */
>>>>>>> master

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "wast-parser-gen.hh".  */
#ifndef YY_WABT_WAST_PARSER_SRC_PREBUILT_WAST_PARSER_GEN_HH_INCLUDED
# define YY_WABT_WAST_PARSER_SRC_PREBUILT_WAST_PARSER_GEN_HH_INCLUDED
/* Debug traces.  */
#ifndef WABT_WAST_PARSER_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define WABT_WAST_PARSER_DEBUG 1
#  else
#   define WABT_WAST_PARSER_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define WABT_WAST_PARSER_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined WABT_WAST_PARSER_DEBUG */
#if WABT_WAST_PARSER_DEBUG
extern int wabt_wast_parser_debug;
#endif

/* Token type.  */
#ifndef WABT_WAST_PARSER_TOKENTYPE
# define WABT_WAST_PARSER_TOKENTYPE
  enum wabt_wast_parser_tokentype
  {
    WABT_TOKEN_TYPE_EOF = 0,
    WABT_TOKEN_TYPE_LPAR = 258,
    WABT_TOKEN_TYPE_RPAR = 259,
    WABT_TOKEN_TYPE_NAT = 260,
    WABT_TOKEN_TYPE_INT = 261,
    WABT_TOKEN_TYPE_FLOAT = 262,
    WABT_TOKEN_TYPE_TEXT = 263,
    WABT_TOKEN_TYPE_VAR = 264,
    WABT_TOKEN_TYPE_VALUE_TYPE = 265,
    WABT_TOKEN_TYPE_ANYFUNC = 266,
    WABT_TOKEN_TYPE_MUT = 267,
    WABT_TOKEN_TYPE_NOP = 268,
    WABT_TOKEN_TYPE_DROP = 269,
    WABT_TOKEN_TYPE_BLOCK = 270,
    WABT_TOKEN_TYPE_END = 271,
    WABT_TOKEN_TYPE_IF = 272,
    WABT_TOKEN_TYPE_THEN = 273,
    WABT_TOKEN_TYPE_ELSE = 274,
    WABT_TOKEN_TYPE_LOOP = 275,
    WABT_TOKEN_TYPE_BR = 276,
    WABT_TOKEN_TYPE_BR_IF = 277,
    WABT_TOKEN_TYPE_BR_TABLE = 278,
<<<<<<< HEAD
    WABT_TOKEN_TYPE_CALL = 279,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 280,
    WABT_TOKEN_TYPE_RETURN = 281,
    WABT_TOKEN_TYPE_GET_LOCAL = 282,
    WABT_TOKEN_TYPE_SET_LOCAL = 283,
    WABT_TOKEN_TYPE_TEE_LOCAL = 284,
    WABT_TOKEN_TYPE_GET_GLOBAL = 285,
    WABT_TOKEN_TYPE_SET_GLOBAL = 286,
    WABT_TOKEN_TYPE_LOAD = 287,
    WABT_TOKEN_TYPE_STORE = 288,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 289,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 290,
    WABT_TOKEN_TYPE_CONST = 291,
    WABT_TOKEN_TYPE_UNARY = 292,
    WABT_TOKEN_TYPE_BINARY = 293,
    WABT_TOKEN_TYPE_COMPARE = 294,
    WABT_TOKEN_TYPE_CONVERT = 295,
    WABT_TOKEN_TYPE_SELECT = 296,
    WABT_TOKEN_TYPE_UNREACHABLE = 297,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 298,
    WABT_TOKEN_TYPE_GROW_MEMORY = 299,
    WABT_TOKEN_TYPE_FUNC = 300,
    WABT_TOKEN_TYPE_START = 301,
    WABT_TOKEN_TYPE_TYPE = 302,
    WABT_TOKEN_TYPE_PARAM = 303,
    WABT_TOKEN_TYPE_RESULT = 304,
    WABT_TOKEN_TYPE_LOCAL = 305,
    WABT_TOKEN_TYPE_GLOBAL = 306,
    WABT_TOKEN_TYPE_MODULE = 307,
    WABT_TOKEN_TYPE_TABLE = 308,
    WABT_TOKEN_TYPE_ELEM = 309,
    WABT_TOKEN_TYPE_MEMORY = 310,
    WABT_TOKEN_TYPE_DATA = 311,
    WABT_TOKEN_TYPE_OFFSET = 312,
    WABT_TOKEN_TYPE_IMPORT = 313,
    WABT_TOKEN_TYPE_EXPORT = 314,
    WABT_TOKEN_TYPE_EXCEPT = 315,
    WABT_TOKEN_TYPE_REGISTER = 316,
    WABT_TOKEN_TYPE_INVOKE = 317,
    WABT_TOKEN_TYPE_GET = 318,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 319,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 320,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 321,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 322,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 323,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 324,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 325,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 326,
    WABT_TOKEN_TYPE_LOW = 327
=======
    WABT_TOKEN_TYPE_TRY = 279,
    WABT_TOKEN_TYPE_CATCH = 280,
    WABT_TOKEN_TYPE_CATCH_ALL = 281,
    WABT_TOKEN_TYPE_THROW = 282,
    WABT_TOKEN_TYPE_RETHROW = 283,
    WABT_TOKEN_TYPE_CALL = 284,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 285,
    WABT_TOKEN_TYPE_RETURN = 286,
    WABT_TOKEN_TYPE_GET_LOCAL = 287,
    WABT_TOKEN_TYPE_SET_LOCAL = 288,
    WABT_TOKEN_TYPE_TEE_LOCAL = 289,
    WABT_TOKEN_TYPE_GET_GLOBAL = 290,
    WABT_TOKEN_TYPE_SET_GLOBAL = 291,
    WABT_TOKEN_TYPE_LOAD = 292,
    WABT_TOKEN_TYPE_STORE = 293,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 294,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 295,
    WABT_TOKEN_TYPE_CONST = 296,
    WABT_TOKEN_TYPE_UNARY = 297,
    WABT_TOKEN_TYPE_BINARY = 298,
    WABT_TOKEN_TYPE_COMPARE = 299,
    WABT_TOKEN_TYPE_CONVERT = 300,
    WABT_TOKEN_TYPE_SELECT = 301,
    WABT_TOKEN_TYPE_UNREACHABLE = 302,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 303,
    WABT_TOKEN_TYPE_GROW_MEMORY = 304,
    WABT_TOKEN_TYPE_FUNC = 305,
    WABT_TOKEN_TYPE_START = 306,
    WABT_TOKEN_TYPE_TYPE = 307,
    WABT_TOKEN_TYPE_PARAM = 308,
    WABT_TOKEN_TYPE_RESULT = 309,
    WABT_TOKEN_TYPE_LOCAL = 310,
    WABT_TOKEN_TYPE_GLOBAL = 311,
    WABT_TOKEN_TYPE_MODULE = 312,
    WABT_TOKEN_TYPE_TABLE = 313,
    WABT_TOKEN_TYPE_ELEM = 314,
    WABT_TOKEN_TYPE_MEMORY = 315,
    WABT_TOKEN_TYPE_DATA = 316,
    WABT_TOKEN_TYPE_OFFSET = 317,
    WABT_TOKEN_TYPE_IMPORT = 318,
    WABT_TOKEN_TYPE_EXPORT = 319,
    WABT_TOKEN_TYPE_REGISTER = 320,
    WABT_TOKEN_TYPE_INVOKE = 321,
    WABT_TOKEN_TYPE_GET = 322,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 323,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 324,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 325,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 326,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 327,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 328,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 329,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 330,
    WABT_TOKEN_TYPE_LOW = 331
>>>>>>> master
  };
#endif

/* Value type.  */
#if ! defined WABT_WAST_PARSER_STYPE && ! defined WABT_WAST_PARSER_STYPE_IS_DECLARED
typedef ::wabt::Token WABT_WAST_PARSER_STYPE;
# define WABT_WAST_PARSER_STYPE_IS_TRIVIAL 1
# define WABT_WAST_PARSER_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined WABT_WAST_PARSER_LTYPE && ! defined WABT_WAST_PARSER_LTYPE_IS_DECLARED
typedef struct WABT_WAST_PARSER_LTYPE WABT_WAST_PARSER_LTYPE;
struct WABT_WAST_PARSER_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define WABT_WAST_PARSER_LTYPE_IS_DECLARED 1
# define WABT_WAST_PARSER_LTYPE_IS_TRIVIAL 1
#endif



int wabt_wast_parser_parse (::wabt::WastLexer* lexer, ::wabt::WastParser* parser);

#endif /* !YY_WABT_WAST_PARSER_SRC_PREBUILT_WAST_PARSER_GEN_HH_INCLUDED  */

/* Copy the second part of user declarations.  */

<<<<<<< HEAD
#line 348 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */
=======
#line 360 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */
>>>>>>> master

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined WABT_WAST_PARSER_LTYPE_IS_TRIVIAL && WABT_WAST_PARSER_LTYPE_IS_TRIVIAL \
             && defined WABT_WAST_PARSER_STYPE_IS_TRIVIAL && WABT_WAST_PARSER_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  51
/* YYLAST -- Last index in YYTABLE.  */
<<<<<<< HEAD
#define YYLAST   793

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  75
/* YYNRULES -- Number of rules.  */
#define YYNRULES  192
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  423
=======
#define YYLAST   886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  80
/* YYNRULES -- Number of rules.  */
#define YYNRULES  203
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  451
>>>>>>> master

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
<<<<<<< HEAD
#define YYMAXUTOK   327
=======
#define YYMAXUTOK   331
>>>>>>> master

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
<<<<<<< HEAD
      65,    66,    67,    68,    69,    70,    71,    72
=======
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76
>>>>>>> master
};

#if WABT_WAST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
<<<<<<< HEAD
       0,   244,   244,   250,   260,   261,   265,   283,   284,   290,
     293,   298,   305,   308,   309,   314,   321,   329,   335,   341,
     346,   353,   359,   370,   374,   378,   385,   390,   397,   398,
     404,   405,   408,   412,   413,   417,   418,   428,   429,   440,
     441,   442,   445,   448,   451,   454,   457,   460,   463,   466,
     469,   472,   475,   478,   481,   484,   487,   490,   493,   496,
     509,   512,   515,   518,   521,   524,   529,   534,   539,   544,
     552,   555,   560,   567,   571,   574,   579,   584,   592,   600,
     603,   607,   611,   615,   619,   623,   630,   631,   639,   640,
     648,   653,   661,   675,   682,   687,   697,   705,   716,   723,
     724,   729,   735,   745,   752,   753,   758,   764,   774,   781,
     785,   790,   802,   805,   809,   818,   832,   846,   852,   860,
     868,   887,   896,   910,   924,   929,   937,   945,   968,   982,
     988,   996,  1009,  1017,  1025,  1031,  1037,  1046,  1056,  1064,
    1069,  1074,  1079,  1086,  1095,  1105,  1112,  1123,  1131,  1132,
    1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,  1145,
    1146,  1150,  1155,  1163,  1183,  1194,  1214,  1221,  1226,  1234,
    1244,  1254,  1260,  1266,  1272,  1278,  1284,  1289,  1294,  1300,
    1309,  1314,  1315,  1320,  1329,  1333,  1340,  1352,  1353,  1360,
    1363,  1423,  1435
=======
       0,   254,   254,   260,   270,   271,   275,   293,   294,   300,
     303,   308,   315,   318,   319,   324,   331,   339,   345,   351,
     356,   363,   369,   380,   384,   388,   395,   400,   407,   408,
     414,   415,   418,   422,   423,   427,   428,   438,   439,   450,
     451,   452,   456,   459,   462,   465,   468,   471,   474,   477,
     480,   483,   486,   489,   492,   495,   498,   501,   504,   507,
     520,   523,   526,   529,   532,   535,   538,   541,   547,   552,
     557,   562,   568,   576,   579,   584,   591,   595,   602,   603,
     609,   613,   616,   621,   626,   632,   640,   643,   649,   657,
     660,   664,   668,   672,   676,   680,   687,   692,   698,   704,
     705,   713,   714,   722,   727,   741,   748,   753,   763,   771,
     782,   789,   790,   795,   801,   811,   818,   819,   824,   830,
     840,   847,   851,   856,   868,   871,   875,   884,   898,   912,
     918,   926,   934,   953,   962,   976,   990,   995,  1003,  1011,
    1034,  1048,  1054,  1062,  1075,  1083,  1091,  1097,  1103,  1112,
    1122,  1130,  1135,  1140,  1145,  1152,  1161,  1171,  1178,  1189,
    1197,  1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,
    1210,  1211,  1215,  1220,  1228,  1248,  1259,  1279,  1286,  1291,
    1299,  1309,  1319,  1325,  1331,  1337,  1343,  1349,  1354,  1359,
    1365,  1374,  1379,  1380,  1385,  1394,  1398,  1405,  1417,  1418,
    1425,  1428,  1488,  1500
>>>>>>> master
};
#endif

#if WABT_WAST_PARSER_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "NAT", "INT",
  "FLOAT", "TEXT", "VAR", "VALUE_TYPE", "ANYFUNC", "MUT", "NOP", "DROP",
  "BLOCK", "END", "IF", "THEN", "ELSE", "LOOP", "BR", "BR_IF", "BR_TABLE",
<<<<<<< HEAD
  "CALL", "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "TEE_LOCAL",
  "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE", "OFFSET_EQ_NAT",
  "ALIGN_EQ_NAT", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT",
  "SELECT", "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY", "FUNC",
  "START", "TYPE", "PARAM", "RESULT", "LOCAL", "GLOBAL", "MODULE", "TABLE",
  "ELEM", "MEMORY", "DATA", "OFFSET", "IMPORT", "EXPORT", "EXCEPT",
  "REGISTER", "INVOKE", "GET", "ASSERT_MALFORMED", "ASSERT_INVALID",
  "ASSERT_UNLINKABLE", "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
=======
  "TRY", "CATCH", "CATCH_ALL", "THROW", "RETHROW", "CALL", "CALL_INDIRECT",
  "RETURN", "GET_LOCAL", "SET_LOCAL", "TEE_LOCAL", "GET_GLOBAL",
  "SET_GLOBAL", "LOAD", "STORE", "OFFSET_EQ_NAT", "ALIGN_EQ_NAT", "CONST",
  "UNARY", "BINARY", "COMPARE", "CONVERT", "SELECT", "UNREACHABLE",
  "CURRENT_MEMORY", "GROW_MEMORY", "FUNC", "START", "TYPE", "PARAM",
  "RESULT", "LOCAL", "GLOBAL", "MODULE", "TABLE", "ELEM", "MEMORY", "DATA",
  "OFFSET", "IMPORT", "EXPORT", "REGISTER", "INVOKE", "GET",
  "ASSERT_MALFORMED", "ASSERT_INVALID", "ASSERT_UNLINKABLE",
  "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
>>>>>>> master
  "ASSERT_RETURN_ARITHMETIC_NAN", "ASSERT_TRAP", "ASSERT_EXHAUSTION",
  "LOW", "$accept", "text_list", "text_list_opt", "quoted_text",
  "value_type_list", "elem_type", "global_type", "func_type", "func_sig",
  "table_sig", "memory_sig", "limits", "type_use", "nat", "literal", "var",
  "var_list", "bind_var_opt", "bind_var", "labeling_opt", "offset_opt",
  "align_opt", "instr", "plain_instr", "block_instr", "block_sig", "block",
<<<<<<< HEAD
  "expr", "expr1", "if_block", "if_", "instr_list", "expr_list",
  "const_expr", "except", "func", "func_fields", "func_fields_import",
  "func_fields_import1", "func_fields_body", "func_fields_body1",
  "func_body", "func_body1", "offset", "elem", "table", "table_fields",
  "data", "memory", "memory_fields", "global", "global_fields",
  "import_desc", "import", "inline_import", "export_desc", "export",
  "inline_export", "type_def", "start", "module_field",
=======
  "catch_instr", "catch_instr_list", "expr", "expr1", "catch_list",
  "if_block", "if_", "rethrow_check", "throw_check", "try_check",
  "instr_list", "expr_list", "const_expr", "func", "func_fields",
  "func_fields_import", "func_fields_import1", "func_fields_body",
  "func_fields_body1", "func_body", "func_body1", "offset", "elem",
  "table", "table_fields", "data", "memory", "memory_fields", "global",
  "global_fields", "import_desc", "import", "inline_import", "export_desc",
  "export", "inline_export", "type_def", "start", "module_field",
>>>>>>> master
  "module_fields_opt", "module_fields", "raw_module", "module",
  "inline_module", "script_var_opt", "action", "assertion", "cmd",
  "cmd_list", "const", "const_list", "script", "script_start", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
<<<<<<< HEAD
     325,   326,   327
};
# endif

#define YYPACT_NINF -325

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-325)))
=======
     325,   326,   327,   328,   329,   330,   331
};
# endif

#define YYPACT_NINF -379

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-379)))
>>>>>>> master

#define YYTABLE_NINF -30

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
<<<<<<< HEAD
      69,   722,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
    -325,  -325,  -325,  -325,    97,  -325,  -325,  -325,  -325,  -325,
    -325,   157,  -325,   144,   180,   221,    65,   180,   180,   180,
     138,   180,   138,   183,   183,  -325,   183,   210,   210,   208,
     208,   208,   226,   226,   226,   241,   226,   141,  -325,   102,
    -325,  -325,  -325,   391,  -325,  -325,  -325,  -325,   217,   229,
     272,   274,    42,   190,    85,   346,   275,  -325,  -325,   174,
     275,   277,  -325,   183,   279,   112,   210,  -325,   183,   183,
     231,   183,   183,   183,   -20,  -325,   282,   283,    51,   183,
     183,   183,   306,  -325,  -325,   180,   180,   180,   221,   221,
    -325,   221,   221,  -325,   221,   221,   221,   221,   221,   254,
     254,   247,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
     423,   455,  -325,  -325,  -325,  -325,   285,  -325,  -325,  -325,
    -325,   288,   391,  -325,   289,  -325,   290,    22,  -325,   455,
     291,   107,    42,  -325,    52,   292,    97,    74,  -325,   294,
    -325,   287,   298,   307,   298,    85,   180,   180,   180,   455,
     309,   310,  -325,    80,   199,  -325,  -325,   314,   298,   174,
     277,   316,   318,   337,    76,   347,  -325,  -325,   348,  -325,
     353,   354,   358,   376,    55,  -325,  -325,   377,   387,   388,
     221,   180,  -325,   180,   183,   183,  -325,   487,   487,   487,
    -325,  -325,   221,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
    -325,   258,   258,  -325,  -325,  -325,  -325,   551,  -325,   721,
    -325,  -325,   212,   338,  -325,  -325,  -325,   187,   389,  -325,
     385,  -325,  -325,  -325,   384,  -325,  -325,  -325,  -325,  -325,
     343,  -325,  -325,  -325,  -325,  -325,   487,   395,   487,   396,
     309,  -325,  -325,   152,  -325,  -325,   277,  -325,  -325,  -325,
     397,  -325,    83,   398,   221,   221,   221,   221,  -325,  -325,
     259,  -325,  -325,  -325,  -325,   363,  -325,  -325,  -325,  -325,
    -325,   403,   148,   399,   170,   171,   400,   183,   421,   656,
     487,   425,  -325,   104,   426,   238,  -325,  -325,  -325,   180,
    -325,   220,  -325,  -325,  -325,  -325,   435,  -325,  -325,   623,
     395,   454,  -325,  -325,  -325,  -325,  -325,  -325,   467,  -325,
     180,   180,   180,   180,  -325,   469,   470,   485,   499,  -325,
     247,  -325,   423,   501,   519,   519,   502,   517,  -325,  -325,
    -325,   180,   180,   180,   180,   202,   525,   203,   204,   206,
    -325,   228,   455,  -325,   689,   309,  -325,   534,   107,   298,
     298,  -325,  -325,  -325,  -325,   549,  -325,   423,   589,  -325,
    -325,   519,  -325,   214,  -325,  -325,   455,  -325,   338,   550,
    -325,   535,  -325,  -325,   563,   455,  -325,   224,   565,   566,
     581,   582,   592,  -325,  -325,  -325,  -325,   591,  -325,   338,
     548,   595,   601,  -325,  -325,  -325,  -325,  -325,   180,  -325,
    -325,   604,   605,  -325,   218,   455,   615,  -325,   620,   455,
    -325,   631,  -325
=======
      65,   811,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,    89,  -379,  -379,  -379,  -379,  -379,  -379,
     103,  -379,   109,   108,   173,   124,   108,   108,   108,   163,
     108,   163,   120,   120,   120,   158,   158,   189,   189,   189,
     226,   226,   226,   232,   226,   157,  -379,   131,  -379,  -379,
    -379,   430,  -379,  -379,  -379,  -379,   234,   193,   241,   249,
      87,    88,    90,   393,   253,  -379,  -379,   282,   253,   257,
    -379,   120,   276,   158,  -379,   120,   120,   229,   120,   120,
     120,   -18,  -379,   286,   295,    -4,   120,   120,   120,   348,
    -379,  -379,   108,   108,   108,   173,   173,  -379,  -379,  -379,
    -379,   173,   173,  -379,   173,   173,   173,   173,   173,   261,
     261,   205,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
     467,   504,  -379,  -379,  -379,   173,   173,   108,  -379,   297,
    -379,  -379,  -379,  -379,   299,   430,  -379,   300,  -379,   302,
      11,  -379,   504,   303,   102,    87,  -379,   186,   304,    89,
      73,  -379,   301,  -379,   305,   306,   308,   306,    90,   108,
     108,   108,   504,   307,   309,   311,  -379,    42,   203,  -379,
    -379,   314,   306,   282,   257,   312,   315,   318,   -22,   321,
     325,  -379,   326,   330,   333,   336,   143,  -379,  -379,   342,
     343,   344,   173,   108,  -379,   108,   120,   120,  -379,   541,
     541,   541,  -379,  -379,   173,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,   275,   275,  -379,  -379,  -379,  -379,   615,
    -379,   810,  -379,  -379,  -379,   541,  -379,   230,   319,  -379,
    -379,  -379,   101,   345,  -379,   340,  -379,  -379,  -379,   339,
    -379,  -379,  -379,  -379,  -379,   293,  -379,  -379,  -379,  -379,
    -379,   541,   350,   541,   351,   307,  -379,  -379,   341,   227,
    -379,  -379,   257,  -379,  -379,  -379,   360,  -379,    82,   362,
     173,   173,   173,   173,  -379,  -379,   266,  -379,  -379,  -379,
    -379,   313,  -379,  -379,  -379,  -379,  -379,   363,   147,   364,
     166,   169,   377,   120,   369,   735,   541,   372,  -379,   146,
     382,   237,  -379,  -379,  -379,   270,   108,  -379,   244,  -379,
    -379,  -379,  -379,   395,  -379,  -379,   697,   350,   401,  -379,
    -379,  -379,  -379,  -379,   108,  -379,   405,  -379,   108,   108,
     108,   108,  -379,   414,   415,   428,   442,  -379,   205,  -379,
     467,  -379,   444,   578,   578,   445,   452,  -379,  -379,  -379,
     108,   108,   108,   108,   173,   173,   270,   389,   170,   459,
     179,   181,   183,  -379,   250,   504,  -379,   773,   307,   541,
    -379,   480,   102,   306,   306,  -379,  -379,  -379,  -379,   481,
    -379,   467,   658,  -379,  -379,   578,  -379,   215,  -379,  -379,
     504,  -379,   504,   504,  -379,   108,   319,   482,  -379,   489,
    -379,  -379,   502,   504,  -379,   516,   240,   518,   519,   525,
     526,   539,  -379,  -379,  -379,  -379,   477,  -379,  -379,  -379,
    -379,   319,   503,   556,   562,   557,  -379,  -379,  -379,  -379,
    -379,   108,  -379,  -379,   548,   577,   270,  -379,  -379,   223,
     504,   575,   592,  -379,   593,   504,   557,  -379,   599,  -379,
    -379
>>>>>>> master
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
<<<<<<< HEAD
     189,     0,   158,   152,   153,   150,   154,   151,   149,   156,
     157,   148,   155,   161,   166,   165,   182,   191,   180,   181,
     184,   190,   192,     0,    30,     0,     0,    30,    30,    30,
       0,    30,     0,     0,     0,     7,     0,   167,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   162,     0,
     185,     1,    32,    86,    31,    22,    27,    26,     0,     0,
       0,     0,     0,   159,     0,     0,     0,   113,    28,     0,
       0,     4,     6,     0,     0,     0,   167,   168,     0,     0,
       0,     0,     0,     0,     0,   187,     0,     0,     0,     0,
       0,     0,     0,    43,    44,    33,    33,    33,     0,     0,
      28,     0,     0,    49,     0,     0,     0,     0,     0,    35,
      35,     0,    60,    61,    62,    63,    45,    42,    64,    65,
      86,    86,    39,    40,    41,   109,     0,    94,   103,   104,
     108,    99,    86,   147,    13,   145,     0,     0,    10,    86,
       0,     0,     0,     2,     0,     0,   160,     0,     9,     0,
     117,     0,    19,     0,     0,     0,    33,    33,    33,    86,
      88,     0,    28,     0,     0,   124,    18,     0,     0,     0,
       4,     5,     0,     0,     0,     0,    91,     8,     0,   187,
       0,     0,     0,     0,     0,   176,   177,     0,     0,     0,
       0,     7,     7,     7,     0,     0,    34,    86,    86,    86,
      46,    47,     0,    50,    51,    52,    53,    54,    55,    56,
      36,    37,    37,    23,    24,    25,    59,     0,    93,     0,
      87,    92,     0,    99,    96,    98,    97,     0,     0,   146,
       0,    90,   129,   128,     0,   130,   131,   164,     3,   163,
       0,    17,    20,   116,   118,   119,    86,     0,    86,     0,
      88,    74,    73,     0,   115,    29,     4,   123,   125,   126,
       0,   122,     0,     0,     0,     0,     0,     0,   143,   183,
       0,   170,   171,   172,   173,     0,   175,   188,   174,   178,
     179,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      86,     0,    72,     0,     0,    48,    38,    57,    58,     7,
       7,     0,    95,     7,     7,    12,     0,    28,    75,     0,
       0,     0,    77,    79,    76,   112,    89,   114,     0,   121,
      30,    30,    30,    30,   137,     0,     0,     0,     0,   169,
       0,    21,    86,     0,    86,    86,     0,     0,   144,     7,
      71,    33,    33,    33,    33,     0,     0,     0,     0,     0,
      11,     0,    86,    78,     0,    85,   127,    13,     0,     0,
       0,   139,   142,   140,   141,     0,   106,    86,     0,   105,
     110,    86,   138,     0,    66,    68,    86,    67,    99,     0,
     100,    14,    16,   120,     0,    86,    84,     0,     0,     0,
       0,     0,     0,   186,   107,   111,    70,     0,   101,    99,
       0,    81,     0,   133,   132,   136,   134,   135,    33,   102,
       7,     0,    83,    69,     0,    86,     0,    15,     0,    86,
      80,     0,    82
=======
     200,     0,   164,   165,   162,   166,   163,   161,   168,   169,
     160,   167,   172,   177,   176,   193,   202,   191,   192,   195,
     201,   203,     0,    30,     0,     0,    30,    30,    30,     0,
      30,     0,     0,     0,     0,   178,   178,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   173,     0,   196,     1,
      32,    99,    31,    22,    27,    26,     0,     0,     0,     0,
       0,   170,     0,     0,     0,   125,    28,     0,     0,     4,
       6,     0,     0,   178,   179,     0,     0,     0,     0,     0,
       0,     0,   198,     0,     0,     0,     0,     0,     0,     0,
      43,    44,    33,    33,    33,     0,     0,    28,    98,    97,
      96,     0,     0,    49,     0,     0,     0,     0,     0,    35,
      35,     0,    60,    61,    62,    63,    45,    42,    64,    65,
      99,    99,    39,    40,    41,     0,     0,    33,   121,     0,
     106,   115,   116,   120,   111,    99,   159,    13,   157,     0,
       0,    10,    99,     0,     0,     0,     2,     0,     0,   171,
       0,     9,     0,   129,     0,    19,     0,     0,     0,    33,
      33,    33,    99,   101,     0,     0,    28,     0,     0,   136,
      18,     0,     0,     0,     4,     5,     0,     0,     0,     0,
       0,   198,     0,     0,     0,     0,     0,   187,   188,     0,
       0,     0,     0,     7,     7,     7,     0,     0,    34,    99,
      99,    99,    46,    47,     0,    50,    51,    52,    53,    54,
      55,    56,    36,    37,    37,    23,    24,    25,    59,     0,
     105,     0,   100,    67,    66,    99,   104,     0,   111,   108,
     110,   109,     0,     0,   158,     0,   103,   141,   140,     0,
     142,   143,   175,     3,   174,     0,    17,    20,   128,   130,
     131,    99,     0,    99,     0,   101,    81,    80,     0,     0,
     127,    29,     4,   135,   137,   138,     0,   134,     0,     0,
       0,     0,     0,     0,   155,   194,     0,   181,   182,   183,
     184,     0,   186,   199,   185,   189,   190,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,     0,    75,     0,
       0,    48,    38,    57,    58,     0,     7,     7,     0,   107,
       7,     7,    12,     0,    28,    82,     0,     0,     0,    84,
      89,    83,   124,   102,    33,   126,     0,   133,    30,    30,
      30,    30,   149,     0,     0,     0,     0,   180,     0,    21,
      99,     8,     0,    99,    99,     0,     0,   156,     7,    74,
      33,    33,    33,    33,     0,     0,    78,     0,     0,     0,
       0,     0,     0,    11,     0,    99,    88,     0,    95,    99,
     139,    13,     0,     0,     0,   151,   154,   152,   153,     0,
     118,    99,     0,   117,   122,    99,   150,     0,    68,    70,
      99,    69,    99,    99,    79,    33,   111,     0,   112,    14,
      16,   132,     0,    99,    94,     0,     0,     0,     0,     0,
       0,     0,   197,   119,   123,    73,     0,    76,    77,    72,
     113,   111,     0,    91,     0,     0,   145,   144,   148,   146,
     147,    33,   114,     7,     0,    93,     0,    85,    71,     0,
      99,     0,     0,    15,     0,    99,    86,    90,     0,    87,
      92
>>>>>>> master
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
<<<<<<< HEAD
    -325,   579,  -141,   -12,  -154,   506,  -124,   597,   311,  -136,
    -145,   -50,  -126,   -43,   344,     0,   -80,   -21,   -15,   -93,
     580,   463,  -325,   -64,  -325,  -216,  -151,   -30,  -325,   381,
    -325,  -106,   451,   564,  -325,  -325,   576,   514,  -324,   619,
    -291,   390,  -285,    17,  -325,  -325,   585,  -325,  -325,   586,
    -325,   614,  -325,  -325,   -18,  -325,  -325,    -7,  -325,  -325,
      13,  -325,   703,   164,  -325,  -325,    -3,   196,  -325,   749,
    -325,  -325,   593,  -325,  -325
=======
    -379,   543,  -158,    43,  -162,   463,  -131,   559,   260,  -143,
    -160,   -57,  -127,   -47,   296,   -23,   -86,    14,    29,   -90,
     523,   426,  -379,   -58,  -379,  -231,  -101,   218,   285,   -29,
    -379,   209,   349,  -379,  -379,  -379,   -44,  -112,   410,   505,
    -379,   542,   446,  -378,   563,  -315,   354,  -318,    30,  -379,
    -379,   540,  -379,  -379,   511,  -379,   531,  -379,  -379,   -38,
    -379,  -379,    -2,  -379,  -379,     4,  -379,   647,   121,  -379,
    -379,    -9,   233,  -379,   689,  -379,  -379,   535,  -379,  -379
>>>>>>> master
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
<<<<<<< HEAD
      -1,   171,   172,    73,    75,   149,   139,    60,   228,   150,
     165,   151,   120,    57,   216,   255,   163,    53,   196,   197,
     211,   297,   121,   122,   123,   290,   291,   124,   161,   312,
     313,   125,   251,   232,     2,     3,   126,   224,   225,   127,
     128,   129,   130,    68,     4,     5,   153,     6,     7,   167,
       8,   140,   263,     9,   131,   175,    10,   132,    11,    12,
      13,   145,    14,    15,    16,    17,    78,    18,    19,    20,
      21,   277,   184,    22,    23
=======
      -1,   175,   176,    71,   288,   152,   142,    58,   233,   153,
     169,   154,   120,    55,   218,   261,   167,    51,   198,   199,
     213,   303,   121,   122,   123,   296,   297,   356,   357,   124,
     164,   437,   319,   320,   125,   126,   127,   128,   256,   237,
       2,   129,   229,   230,   130,   131,   132,   133,    66,     3,
       4,   156,     5,     6,   171,     7,   143,   269,     8,   134,
     179,     9,   135,    10,    11,    12,   148,    13,    14,    15,
      16,    75,    17,    18,    19,    20,   283,   186,    21,    22
>>>>>>> master
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
<<<<<<< HEAD
      67,   160,    67,   198,   199,   223,    62,    63,    64,    54,
      69,    61,    54,    54,    54,   220,    54,   235,   244,   166,
     202,   152,    74,   258,    76,    58,   152,    48,   160,   260,
      66,   310,    70,   231,   230,    79,    67,   282,   284,   285,
      67,   366,    37,    38,   141,   137,   154,   293,   294,    71,
     370,   168,   138,   231,   398,   142,   237,   155,   275,   276,
     238,   173,   169,   246,   247,   248,   179,   180,    59,   181,
     182,   183,     1,   178,    52,   409,   394,   187,   188,   189,
     194,   195,   253,   162,   254,    55,   395,   170,   147,    56,
      55,   292,   292,   292,   310,   308,   148,   314,   200,   201,
      47,   203,   204,    28,   205,   206,   207,   208,   209,   242,
     234,   152,   152,    37,    38,   318,   176,   138,   166,   166,
     342,   264,   177,   343,   141,   152,   152,   265,   320,   266,
     250,   267,   194,   195,   321,   142,   322,   154,   323,   340,
     292,    65,   292,    55,    51,   345,   347,    56,   155,   348,
     349,   168,   332,   160,    28,   160,   317,    55,   177,    48,
      49,    56,   169,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,   334,   335,   283,   164,   286,    55,
     177,   177,   287,   288,   292,   373,    24,    25,    26,    52,
     281,    72,    27,    47,    29,    30,    31,    32,   143,    33,
      34,    35,   295,    81,    82,    83,   378,   380,   381,    89,
     382,    80,   177,   177,   177,   392,   177,   311,   396,    77,
     250,   133,   417,   391,   177,   160,    55,   351,   177,    84,
      56,   389,   383,    55,   390,   303,   304,    56,    85,    86,
      87,    90,    91,   -29,    88,   160,   384,   -29,   374,   375,
     376,   377,   213,   214,   215,   256,   414,   194,   195,   190,
     299,   300,   275,   329,   325,   326,   327,   328,   299,   300,
     397,   190,   303,   304,   134,   337,   135,    59,    65,   402,
     311,   355,   174,    28,   346,   143,   185,   186,   210,   221,
     160,   222,   227,   296,   229,   233,   239,   240,   148,   357,
     358,   359,   360,    55,   160,    54,    54,    54,    54,   418,
     166,   243,   219,   421,   252,   413,   152,   152,   257,    93,
      94,   156,   261,   157,   238,   386,   158,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     262,   301,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   268,   269,   190,   191,   192,   193,   271,   272,    93,
      94,   156,   273,   157,   194,   195,   158,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     274,   278,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   279,   280,   305,    92,   306,   230,   307,   309,   330,
     315,   319,   324,   159,    93,    94,    95,   331,    96,   333,
     336,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   338,   217,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    93,    94,    95,   350,
      96,   341,   344,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   354,   219,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    93,    94,
      95,   356,    96,   361,   362,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   363,
     289,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      93,    94,    95,   364,    96,   367,   371,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   372,   368,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    93,    94,    95,   379,    96,   387,   400,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   393,   399,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    93,    94,   156,   401,   157,   403,
     404,   158,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   405,   406,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   407,   410,   411,   191,
     192,   193,    93,    94,   156,   412,   157,   408,   416,   158,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   415,   420,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   419,   422,    93,    94,   156,   193,
     157,   352,   144,   158,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   241,   136,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   388,    93,
      94,   156,   339,   157,   365,   298,   158,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     212,   353,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   316,    93,    94,   156,   339,   157,   385,   226,   158,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   249,   369,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    93,    94,   156,   302,   157,   218,
     245,   158,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   259,   236,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   146,    24,    25,    26,
      50,     0,   270,    27,    28,    29,    30,    31,    32,     0,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46
=======
      65,    56,    65,   200,   201,   163,    64,   228,    68,   222,
     170,   204,   264,   240,   249,   155,   266,    46,   420,   165,
     155,   317,   144,   235,   157,   380,   384,    76,   270,   172,
     236,   163,   290,   291,   271,    65,   272,   225,   273,    65,
      60,    61,    62,   432,    67,   165,   260,    53,    35,    36,
     236,    54,    52,    27,    59,    52,    52,    52,   145,    52,
     158,    69,    35,    36,   180,   173,   413,   414,     1,   251,
     252,   253,   202,   203,   196,   197,    72,    73,   205,   206,
     259,   207,   208,   209,   210,   211,   317,   298,   298,   298,
     140,    45,    45,   150,   166,    53,   146,   141,   174,   299,
     300,   151,   223,   224,   326,   239,    47,   144,   247,    49,
     155,   155,   141,   298,   177,   170,   170,    50,   181,   182,
     157,   183,   184,   185,   305,   155,   155,    57,    70,   189,
     190,   191,   328,    50,   255,   172,   196,   197,   329,   298,
     330,   298,   331,   145,   358,   360,   281,   282,   361,   362,
     315,   340,   321,    46,   310,   311,   158,   341,    78,    79,
      80,   163,   351,   163,    86,   352,    63,    74,    53,   287,
     343,   173,    54,   344,   396,   165,   341,   165,    53,   341,
     341,   301,    54,   398,   298,   399,   387,   400,    27,   341,
     242,   341,    77,   341,   243,   349,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    23,    24,    25,
     215,   216,   217,    26,   411,    28,    29,    30,    31,   415,
      32,    33,   289,   318,   292,   341,   255,   443,   364,    81,
     410,   325,    53,   341,   369,    85,    54,   163,   136,   293,
     294,   409,   -29,   137,   408,   138,   -29,   333,   334,   335,
     336,   165,    57,   402,   401,    53,    63,   298,   163,    54,
     388,   389,   390,   391,   262,   146,   196,   197,   405,   281,
     337,   439,   165,    82,    83,    84,    87,    88,   416,   178,
     417,   418,   192,   306,   307,   168,    27,    53,   318,   368,
     187,   424,   192,   310,   311,   354,   355,   306,   307,   188,
     212,   226,   227,   232,   245,   419,   234,   238,   244,   163,
     221,    53,   248,   257,   258,   302,   151,   170,   263,   267,
     243,   268,   308,   165,   163,   274,   155,   155,   444,   275,
     277,   392,   393,   448,   278,   359,   346,   279,   165,   404,
     280,   438,   371,   372,   373,   374,   284,   285,   286,   312,
     313,   235,   314,   316,   338,   322,   324,    52,    52,    52,
      52,    90,    91,   159,   327,   160,   332,   339,   161,    95,
      96,    97,    98,   347,   342,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   345,   350,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   353,   363,
     192,   193,   194,   195,   367,   395,    90,    91,   159,   370,
     160,   196,   197,   161,    95,    96,    97,    98,   375,   376,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   377,    89,   111,   112,   113,   114,   115,   116,
     117,   118,   119,    90,    91,    92,   378,    93,   381,   385,
      94,    95,    96,    97,    98,   162,   386,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   397,
     219,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      90,    91,    92,   406,    93,   412,   421,    94,    95,    96,
      97,    98,   422,   431,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   423,   221,   111,   112,
     113,   114,   115,   116,   117,   118,   119,    90,    91,    92,
     425,    93,   426,   427,    94,    95,    96,    97,    98,   428,
     429,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   430,   295,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    90,    91,    92,   433,    93,   434,
     436,    94,    95,    96,    97,    98,   435,   440,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     441,   382,   111,   112,   113,   114,   115,   116,   117,   118,
     119,    90,    91,    92,   445,    93,   446,   447,    94,    95,
      96,    97,    98,   450,   147,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   246,   139,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    90,    91,
     159,   407,   160,   214,   379,   161,    95,    96,    97,    98,
     304,   394,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   442,   449,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   323,   366,   254,   193,   194,
     195,    90,    91,   159,   309,   160,   241,   231,   161,    95,
      96,    97,    98,   220,   265,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   383,   250,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   149,    48,
      90,    91,   159,   195,   160,   365,   276,   161,    95,    96,
      97,    98,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,   112,
     113,   114,   115,   116,   117,   118,   119,     0,    90,    91,
     159,   348,   160,     0,     0,   161,    95,    96,    97,    98,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,     0,    90,    91,   159,   348,
     160,   403,     0,   161,    95,    96,    97,    98,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,   112,   113,   114,   115,   116,
     117,   118,   119,    90,    91,   159,     0,   160,     0,     0,
     161,    95,    96,    97,    98,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   112,   113,   114,   115,   116,   117,   118,   119,
       0,    23,    24,    25,     0,     0,     0,    26,    27,    28,
      29,    30,    31,     0,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44
>>>>>>> master
};

static const yytype_int16 yycheck[] =
{
<<<<<<< HEAD
      30,    65,    32,    96,    97,   131,    27,    28,    29,    24,
      31,    26,    27,    28,    29,   121,    31,   141,   154,    69,
     100,    64,    34,   168,    36,    25,    69,    14,    92,   170,
      30,   247,    32,   139,    12,    38,    66,   191,   192,   193,
      70,   332,    62,    63,    62,     3,    64,   198,   199,    32,
     335,    69,    10,   159,   378,    62,     4,    64,     3,     4,
       8,    73,    69,   156,   157,   158,    78,    79,     3,    81,
      82,    83,     3,    76,     9,   399,   367,    89,    90,    91,
      58,    59,   162,    66,     4,     5,   371,    70,     3,     9,
       5,   197,   198,   199,   310,   246,    11,   248,    98,    99,
       3,   101,   102,    52,   104,   105,   106,   107,   108,   152,
       3,   154,   155,    62,    63,   256,     4,    10,   168,   169,
      16,    45,    10,    19,   142,   168,   169,    51,    45,    53,
     160,    55,    58,    59,    51,   142,    53,   155,    55,   290,
     246,     3,   248,     5,     0,   299,   300,     9,   155,   303,
     304,   169,     4,   217,    52,   219,     4,     5,    10,   146,
       3,     9,   169,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     4,     4,   191,     3,   193,     5,
      10,    10,   194,   195,   290,   339,    45,    46,    47,     9,
     190,     8,    51,     3,    53,    54,    55,    56,     8,    58,
      59,    60,   202,    39,    40,    41,     4,     4,     4,    45,
       4,     3,    10,    10,    10,   360,    10,   247,     4,     9,
     250,     4,     4,   359,    10,   289,     5,   307,    10,     3,
       9,   357,     4,     5,   358,    48,    49,     9,    42,    43,
      44,    45,    46,     5,     3,   309,   352,     9,   341,   342,
     343,   344,     5,     6,     7,    56,   410,    58,    59,    47,
      48,    49,     3,     4,   264,   265,   266,   267,    48,    49,
     376,    47,    48,    49,    45,   287,     4,     3,     3,   385,
     310,   311,     3,    52,   299,     8,     4,     4,    34,     4,
     354,     3,     3,    35,     4,     4,     4,     3,    11,   320,
     321,   322,   323,     5,   368,   320,   321,   322,   323,   415,
     360,     4,     3,   419,     4,   408,   359,   360,     4,    13,
      14,    15,     4,    17,     8,   355,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
       3,     3,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     4,     4,    47,    48,    49,    50,     4,     4,    13,
      14,    15,     4,    17,    58,    59,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
       4,     4,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     4,     4,     4,     3,    10,    12,    54,     3,    36,
       4,     4,     4,    57,    13,    14,    15,     4,    17,    10,
      10,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,     4,     3,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    13,    14,    15,     4,
      17,    16,    16,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,     3,     3,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    13,    14,
      15,     4,    17,     4,     4,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,     4,
       3,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      13,    14,    15,     4,    17,     4,     4,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,     4,     3,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    13,    14,    15,    10,    17,     3,     3,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,     4,     4,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    13,    14,    15,     4,    17,     4,
       4,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,     4,     4,    36,    37,    38,
      39,    40,    41,    42,    43,    44,     4,    49,     3,    48,
      49,    50,    13,    14,    15,     4,    17,    16,     3,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    19,     4,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    19,     4,    13,    14,    15,    50,
      17,    18,    63,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,   151,    61,    36,
      37,    38,    39,    40,    41,    42,    43,    44,   357,    13,
      14,    15,    49,    17,   330,   212,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     110,   310,    36,    37,    38,    39,    40,    41,    42,    43,
      44,   250,    13,    14,    15,    49,    17,    18,   132,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,   159,   334,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    13,    14,    15,   223,    17,   120,
     155,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,   169,   142,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    63,    45,    46,    47,
      21,    -1,   179,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71
=======
      29,    24,    31,    93,    94,    63,    29,   134,    31,   121,
      67,    97,   172,   144,   157,    62,   174,    13,   396,    63,
      67,   252,    60,    12,    62,   340,   344,    36,    50,    67,
     142,    89,   194,   195,    56,    64,    58,   127,    60,    68,
      26,    27,    28,   421,    30,    89,     4,     5,    66,    67,
     162,     9,    23,    57,    25,    26,    27,    28,    60,    30,
      62,    31,    66,    67,    73,    67,   381,   385,     3,   159,
     160,   161,    95,    96,    63,    64,    33,    34,   101,   102,
     166,   104,   105,   106,   107,   108,   317,   199,   200,   201,
       3,     3,     3,     3,    64,     5,     8,    10,    68,   200,
     201,    11,   125,   126,   262,     3,     3,   145,   155,     0,
     157,   158,    10,   225,    71,   172,   173,     9,    75,    76,
     158,    78,    79,    80,   225,   172,   173,     3,     8,    86,
      87,    88,    50,     9,   163,   173,    63,    64,    56,   251,
      58,   253,    60,   145,   306,   307,     3,     4,   310,   311,
     251,     4,   253,   149,    53,    54,   158,    10,    37,    38,
      39,   219,    16,   221,    43,    19,     3,     9,     5,   192,
       4,   173,     9,     4,     4,   219,    10,   221,     5,    10,
      10,   204,     9,     4,   296,     4,   348,     4,    57,    10,
       4,    10,     3,    10,     8,   296,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    50,    51,    52,
       5,     6,     7,    56,   374,    58,    59,    60,    61,     4,
      63,    64,   193,   252,   195,    10,   255,     4,   314,     3,
     373,     4,     5,    10,   324,     3,     9,   295,     4,   196,
     197,   372,     5,    50,   371,     4,     9,   270,   271,   272,
     273,   295,     3,   365,     4,     5,     3,   369,   316,     9,
     350,   351,   352,   353,    61,     8,    63,    64,   369,     3,
       4,   433,   316,    40,    41,    42,    43,    44,   390,     3,
     392,   393,    52,    53,    54,     3,    57,     5,   317,   318,
       4,   403,    52,    53,    54,    25,    26,    53,    54,     4,
      39,     4,     3,     3,     3,   395,     4,     4,     4,   367,
       3,     5,     4,     4,     3,    40,    11,   374,     4,     4,
       8,     3,     3,   367,   382,     4,   373,   374,   440,     4,
       4,   354,   355,   445,     4,   306,   293,     4,   382,   368,
       4,   431,   328,   329,   330,   331,     4,     4,     4,     4,
      10,    12,    59,     3,    41,     4,    15,   328,   329,   330,
     331,    13,    14,    15,     4,    17,     4,     4,    20,    21,
      22,    23,    24,     4,    10,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    10,    16,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    16,     4,
      52,    53,    54,    55,     3,    16,    13,    14,    15,     4,
      17,    63,    64,    20,    21,    22,    23,    24,     4,     4,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,     4,     3,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    13,    14,    15,     4,    17,     4,     4,
      20,    21,    22,    23,    24,    62,     4,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    10,
       3,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      13,    14,    15,     3,    17,     4,     4,    20,    21,    22,
      23,    24,     3,    16,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,     4,     3,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    13,    14,    15,
       4,    17,     4,     4,    20,    21,    22,    23,    24,     4,
       4,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,     4,     3,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    13,    14,    15,    54,    17,     3,
       3,    20,    21,    22,    23,    24,     4,    19,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
       3,     3,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    13,    14,    15,    19,    17,     4,     4,    20,    21,
      22,    23,    24,     4,    61,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,   154,    59,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    13,    14,
      15,   371,    17,   110,   338,    20,    21,    22,    23,    24,
     214,   356,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,   436,   446,    41,    42,    43,    44,
      45,    46,    47,    48,    49,   255,   317,   162,    53,    54,
      55,    13,    14,    15,   228,    17,   145,   135,    20,    21,
      22,    23,    24,   120,   173,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,   343,   158,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    61,    20,
      13,    14,    15,    55,    17,    18,   181,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    13,    14,
      15,    54,    17,    -1,    -1,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    13,    14,    15,    54,
      17,    18,    -1,    20,    21,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    50,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    60,    61,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75
>>>>>>> master
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
<<<<<<< HEAD
       0,     3,   107,   108,   117,   118,   120,   121,   123,   126,
     129,   131,   132,   133,   135,   136,   137,   138,   140,   141,
     142,   143,   146,   147,    45,    46,    47,    51,    52,    53,
      54,    55,    56,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     3,   133,     3,
     142,     0,     9,    90,    91,     5,     9,    86,    88,     3,
      80,    91,    90,    90,    90,     3,    88,   100,   116,    90,
      88,   116,     8,    76,    76,    77,    76,     9,   139,   139,
       3,   136,   136,   136,     3,   140,   140,   140,     3,   136,
     140,   140,     3,    13,    14,    15,    17,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      85,    95,    96,    97,   100,   104,   109,   112,   113,   114,
     115,   127,   130,     4,    45,     4,    80,     3,    10,    79,
     124,   127,   130,     8,    74,   134,   135,     3,    11,    78,
      82,    84,    86,   119,   127,   130,    15,    17,    20,    57,
      96,   101,   116,    89,     3,    83,    84,   122,   127,   130,
     116,    74,    75,    76,     3,   128,     4,    10,   139,    76,
      76,    76,    76,    76,   145,     4,     4,    76,    76,    76,
      47,    48,    49,    50,    58,    59,    91,    92,    92,    92,
      88,    88,    89,    88,    88,    88,    88,    88,    88,    88,
      34,    93,    93,     5,     6,     7,    87,     3,   112,     3,
     104,     4,     3,    85,   110,   111,   109,     3,    81,     4,
      12,   104,   106,     4,     3,    79,   124,     4,     8,     4,
       3,    78,    86,     4,    82,   119,    92,    92,    92,   106,
     100,   105,     4,    89,     4,    88,    56,     4,    83,   122,
      75,     4,     3,   125,    45,    51,    53,    55,     4,     4,
     145,     4,     4,     4,     4,     3,     4,   144,     4,     4,
       4,    88,    77,    91,    77,    77,    91,    76,    76,     3,
      98,    99,   104,    99,    99,    88,    35,    94,    94,    48,
      49,     3,   110,    48,    49,     4,    10,    54,    99,     3,
      98,   100,   102,   103,    99,     4,   105,     4,    75,     4,
      45,    51,    53,    55,     4,    88,    88,    88,    88,     4,
      36,     4,     4,    10,     4,     4,    10,    76,     4,    49,
      99,    16,    16,    19,    16,    77,    91,    77,    77,    77,
       4,    89,    18,   102,     3,   100,     4,    90,    90,    90,
      90,     4,     4,     4,     4,    87,   113,     4,     3,   114,
     115,     4,     4,    77,    92,    92,    92,    92,     4,    10,
       4,     4,     4,     4,   104,    18,   100,     3,    81,    85,
      79,    82,    83,     4,   113,   115,     4,   104,   111,     4,
       3,     4,   104,     4,     4,     4,     4,     4,    16,   111,
      49,     3,     4,    92,    77,    19,     3,     4,   104,    19,
       4,   104,     4
=======
       0,     3,   117,   126,   127,   129,   130,   132,   135,   138,
     140,   141,   142,   144,   145,   146,   147,   149,   150,   151,
     152,   155,   156,    50,    51,    52,    56,    57,    58,    59,
      60,    61,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,     3,   142,     3,   151,     0,
       9,    94,    95,     5,     9,    90,    92,     3,    84,    95,
      94,    94,    94,     3,    92,   106,   125,    94,    92,   125,
       8,    80,    80,    80,     9,   148,   148,     3,   145,   145,
     145,     3,   149,   149,   149,     3,   145,   149,   149,     3,
      13,    14,    15,    17,    20,    21,    22,    23,    24,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      89,    99,   100,   101,   106,   111,   112,   113,   114,   118,
     121,   122,   123,   124,   136,   139,     4,    50,     4,    84,
       3,    10,    83,   133,   136,   139,     8,    78,   143,   144,
       3,    11,    82,    86,    88,    90,   128,   136,   139,    15,
      17,    20,    62,   100,   107,   113,   125,    93,     3,    87,
      88,   131,   136,   139,   125,    78,    79,    80,     3,   137,
     148,    80,    80,    80,    80,    80,   154,     4,     4,    80,
      80,    80,    52,    53,    54,    55,    63,    64,    95,    96,
      96,    96,    92,    92,    93,    92,    92,    92,    92,    92,
      92,    92,    39,    97,    97,     5,     6,     7,    91,     3,
     121,     3,   114,    92,    92,    96,     4,     3,    89,   119,
     120,   118,     3,    85,     4,    12,   114,   116,     4,     3,
      83,   133,     4,     8,     4,     3,    82,    90,     4,    86,
     128,    96,    96,    96,   116,   106,   115,     4,     3,    93,
       4,    92,    61,     4,    87,   131,    79,     4,     3,   134,
      50,    56,    58,    60,     4,     4,   154,     4,     4,     4,
       4,     3,     4,   153,     4,     4,     4,    92,    81,    95,
      81,    81,    95,    80,    80,     3,   102,   103,   114,   103,
     103,    92,    40,    98,    98,   103,    53,    54,     3,   119,
      53,    54,     4,    10,    59,   103,     3,   102,   106,   109,
     110,   103,     4,   115,    15,     4,    79,     4,    50,    56,
      58,    60,     4,    92,    92,    92,    92,     4,    41,     4,
       4,    10,    10,     4,     4,    10,    80,     4,    54,   103,
      16,    16,    19,    16,    25,    26,   104,   105,    81,    95,
      81,    81,    81,     4,    93,    18,   109,     3,   106,    96,
       4,    94,    94,    94,    94,     4,     4,     4,     4,    91,
     122,     4,     3,   123,   124,     4,     4,    81,    96,    96,
      96,    96,    92,    92,   105,    16,     4,    10,     4,     4,
       4,     4,   114,    18,   106,   103,     3,    85,    89,    83,
      86,    87,     4,   122,   124,     4,   114,   114,   114,    96,
     120,     4,     3,     4,   114,     4,     4,     4,     4,     4,
       4,    16,   120,    54,     3,     4,     3,   108,    96,    81,
      19,     3,   104,     4,   114,    19,     4,     4,   114,   108,
       4
>>>>>>> master
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
<<<<<<< HEAD
       0,    73,    74,    74,    75,    75,    76,    77,    77,    78,
      79,    79,    80,    81,    81,    81,    81,    82,    83,    84,
      84,    85,    86,    87,    87,    87,    88,    88,    89,    89,
      90,    90,    91,    92,    92,    93,    93,    94,    94,    95,
      95,    95,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    97,    97,    97,    97,
      98,    99,    99,   100,   101,   101,   101,   101,   102,   102,
     103,   103,   103,   103,   103,   103,   104,   104,   105,   105,
     106,   107,   108,   109,   109,   109,   109,   109,   110,   111,
     111,   111,   111,   112,   113,   113,   113,   113,   114,   115,
     115,   115,   116,   116,   117,   117,   118,   119,   119,   119,
     119,   120,   120,   121,   122,   122,   122,   122,   123,   124,
     124,   124,   125,   125,   125,   125,   125,   126,   127,   128,
     128,   128,   128,   129,   130,   131,   131,   132,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   134,
     134,   135,   135,   136,   136,   137,   138,   139,   139,   140,
     140,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     142,   142,   142,   142,   143,   143,   144,   145,   145,   146,
     146,   146,   147
=======
       0,    77,    78,    78,    79,    79,    80,    81,    81,    82,
      83,    83,    84,    85,    85,    85,    85,    86,    87,    88,
      88,    89,    90,    91,    91,    91,    92,    92,    93,    93,
      94,    94,    95,    96,    96,    97,    97,    98,    98,    99,
      99,    99,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   101,   101,
     101,   101,   101,   102,   103,   103,   104,   104,   105,   105,
     106,   107,   107,   107,   107,   107,   108,   108,   109,   109,
     110,   110,   110,   110,   110,   110,   111,   112,   113,   114,
     114,   115,   115,   116,   117,   118,   118,   118,   118,   118,
     119,   120,   120,   120,   120,   121,   122,   122,   122,   122,
     123,   124,   124,   124,   125,   125,   126,   126,   127,   128,
     128,   128,   128,   129,   129,   130,   131,   131,   131,   131,
     132,   133,   133,   133,   134,   134,   134,   134,   134,   135,
     136,   137,   137,   137,   137,   138,   139,   140,   140,   141,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     143,   143,   144,   144,   145,   145,   146,   147,   148,   148,
     149,   149,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   151,   151,   151,   151,   152,   152,   153,   154,   154,
     155,   155,   155,   156
>>>>>>> master
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     0,     2,     1,
       1,     4,     4,     0,     4,     8,     4,     2,     1,     1,
       2,     4,     1,     1,     1,     1,     1,     1,     0,     2,
       0,     1,     1,     0,     1,     0,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     1,
       2,     2,     2,     2,     2,     2,     2,     3,     3,     2,
<<<<<<< HEAD
       1,     1,     1,     1,     1,     1,     5,     5,     5,     8,
       4,     2,     1,     3,     2,     3,     3,     3,     2,     1,
       8,     4,     9,     5,     3,     2,     0,     2,     0,     2,
       1,     4,     5,     2,     1,     3,     2,     2,     1,     0,
       4,     5,     6,     1,     1,     5,     5,     6,     1,     1,
       5,     6,     4,     1,     6,     5,     5,     1,     2,     2,
       5,     6,     5,     5,     1,     2,     2,     4,     5,     2,
       2,     2,     5,     5,     5,     5,     5,     6,     5,     4,
       4,     4,     4,     5,     4,     4,     5,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     2,     5,     5,     1,     1,     0,     1,     6,
       5,     5,     5,     5,     5,     5,     4,     4,     5,     5,
       1,     1,     1,     5,     1,     2,     4,     0,     2,     0,
       1,     1,     1
=======
       1,     1,     1,     1,     1,     1,     2,     2,     5,     5,
       5,     8,     6,     4,     2,     1,     3,     3,     1,     2,
       3,     2,     3,     3,     3,     7,     3,     4,     2,     1,
       8,     4,     9,     5,     3,     2,     1,     1,     1,     0,
       2,     0,     2,     1,     5,     2,     1,     3,     2,     2,
       1,     0,     4,     5,     6,     1,     1,     5,     5,     6,
       1,     1,     5,     6,     4,     1,     6,     5,     5,     1,
       2,     2,     5,     6,     5,     5,     1,     2,     2,     4,
       5,     2,     2,     2,     5,     5,     5,     5,     5,     6,
       5,     4,     4,     4,     4,     5,     4,     4,     5,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     2,     5,     5,     1,     1,     0,     1,
       6,     5,     5,     5,     5,     5,     5,     4,     4,     5,
       5,     1,     1,     1,     5,     1,     2,     4,     0,     2,
       0,     1,     1,     1
>>>>>>> master
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, lexer, parser, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if WABT_WAST_PARSER_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined WABT_WAST_PARSER_LTYPE_IS_TRIVIAL && WABT_WAST_PARSER_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, lexer, parser); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ::wabt::WastLexer* lexer, ::wabt::WastParser* parser)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (lexer);
  YYUSE (parser);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ::wabt::WastLexer* lexer, ::wabt::WastParser* parser)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, lexer, parser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ::wabt::WastLexer* lexer, ::wabt::WastParser* parser)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , lexer, parser);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, lexer, parser); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !WABT_WAST_PARSER_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !WABT_WAST_PARSER_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ::wabt::WastLexer* lexer, ::wabt::WastParser* parser)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (lexer);
  YYUSE (parser);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 5: /* NAT  */
<<<<<<< HEAD
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1629 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1635 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1641 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1647 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1653 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 34: /* OFFSET_EQ_NAT  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1659 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 35: /* ALIGN_EQ_NAT  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1665 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 74: /* text_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1671 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 75: /* text_list_opt  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1677 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 76: /* quoted_text  */
#line 209 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1683 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 77: /* value_type_list  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1689 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 79: /* global_type  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1695 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 80: /* func_type  */
#line 221 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1701 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 81: /* func_sig  */
#line 221 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1707 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 83: /* memory_sig  */
#line 224 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1713 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 85: /* type_use  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1719 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 87: /* literal  */
#line 210 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1725 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* var  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1731 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* var_list  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1737 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 90: /* bind_var_opt  */
#line 209 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1743 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* bind_var  */
#line 209 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1749 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 92: /* labeling_opt  */
#line 209 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1755 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* instr  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1761 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* plain_instr  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1767 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* block_instr  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1773 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* block_sig  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1779 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* block  */
#line 212 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1785 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* expr  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1791 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* expr1  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1797 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* if_block  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1803 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* if_  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1809 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* instr_list  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1815 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* expr_list  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1821 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* const_expr  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1827 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* func  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1833 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* func_fields  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1839 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* func_fields_import  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1845 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* func_fields_import1  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1851 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* func_fields_body  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1857 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* func_fields_body1  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1863 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* func_body  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1869 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 115: /* func_body1  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1875 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 116: /* offset  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1881 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* table  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1887 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* table_fields  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1893 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 121: /* memory  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 122: /* memory_fields  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1905 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* global  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1911 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* global_fields  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1917 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* import_desc  */
#line 223 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1923 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* inline_import  */
#line 223 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1929 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* export_desc  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 1935 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* inline_export  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 1941 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* module_field  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1947 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* module_fields_opt  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* module_fields  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1959 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* raw_module  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).raw_module); }
#line 1965 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 137: /* module  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1971 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* inline_module  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* script_var_opt  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1983 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 140: /* action  */
#line 211 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 1989 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 141: /* assertion  */
#line 213 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 1995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* cmd  */
#line 213 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2001 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 143: /* cmd_list  */
#line 214 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2007 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* const_list  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2013 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 146: /* script  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
=======
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1674 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1680 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1686 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1692 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1698 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 39: /* OFFSET_EQ_NAT  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1704 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 40: /* ALIGN_EQ_NAT  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1710 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 78: /* text_list  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1716 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 79: /* text_list_opt  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1722 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 80: /* quoted_text  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1728 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 81: /* value_type_list  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1734 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 83: /* global_type  */
#line 232 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1740 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* func_type  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1746 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 85: /* func_sig  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1752 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 87: /* memory_sig  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1758 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* type_use  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1764 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* literal  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1770 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 92: /* var  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1776 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* var_list  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1782 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 94: /* bind_var_opt  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1788 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* bind_var  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* labeling_opt  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1800 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* instr  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1806 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* plain_instr  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1812 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* block_instr  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1818 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* block_sig  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1824 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* block  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1830 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* catch_instr  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1836 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* catch_instr_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1842 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* expr  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1848 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* expr1  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1854 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* catch_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1860 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* if_block  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1866 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* if_  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1872 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* instr_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1878 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 115: /* expr_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1884 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 116: /* const_expr  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1890 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 117: /* func  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1896 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* func_fields  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1902 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* func_fields_import  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1908 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* func_fields_import1  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1914 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 121: /* func_fields_body  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1920 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 122: /* func_fields_body1  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1926 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* func_body  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1932 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* func_body1  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1938 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* offset  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1944 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* table  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1950 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* table_fields  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1956 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* memory  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1962 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* memory_fields  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1968 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* global  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1974 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* global_fields  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1980 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* import_desc  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1986 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* inline_import  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1992 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 137: /* export_desc  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 1998 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* inline_export  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2004 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* module_field  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2010 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 143: /* module_fields_opt  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2016 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 144: /* module_fields  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2022 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* raw_module  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).raw_module); }
#line 2028 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 146: /* module  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2034 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 147: /* inline_module  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 148: /* script_var_opt  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 2046 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 149: /* action  */
#line 221 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 2052 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 150: /* assertion  */
#line 223 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2058 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 151: /* cmd  */
#line 223 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2064 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 152: /* cmd_list  */
#line 224 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2070 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 154: /* const_list  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2076 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 155: /* script  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2082 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
>>>>>>> master
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (::wabt::WastLexer* lexer, ::wabt::WastParser* parser)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined WABT_WAST_PARSER_LTYPE_IS_TRIVIAL && WABT_WAST_PARSER_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
<<<<<<< HEAD
#line 244 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 254 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
<<<<<<< HEAD
#line 2318 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 250 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2381 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 260 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
<<<<<<< HEAD
#line 2331 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 260 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2337 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 265 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2394 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 270 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2400 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 275 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      TextListNode node;
      node.text = (yyvsp[0].text);
      node.next = nullptr;
      TextList text_list;
      text_list.first = &node;
      text_list.last = &node;
      char* data;
      size_t size;
      dup_text_list(&text_list, &data, &size);
      (yyval.text).start = data;
      (yyval.text).length = size;
    }
<<<<<<< HEAD
#line 2355 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 283 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2361 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 284 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2418 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 293 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2424 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 294 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].type));
    }
<<<<<<< HEAD
#line 2370 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 290 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2376 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 293 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2433 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 300 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2439 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 303 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].type);
      (yyval.global)->mutable_ = false;
    }
<<<<<<< HEAD
#line 2386 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 298 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2449 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 308 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].type);
      (yyval.global)->mutable_ = true;
    }
<<<<<<< HEAD
#line 2396 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 305 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2402 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 308 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2408 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 309 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2459 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 315 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2465 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 318 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2471 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 319 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->param_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 2418 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 314 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2481 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 324 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->param_types = std::move(*(yyvsp[-5].types));
      delete (yyvsp[-5].types);
      (yyval.func_sig)->result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 2430 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 321 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2493 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 331 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 2440 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 329 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2503 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 339 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
<<<<<<< HEAD
#line 2449 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 335 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2512 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 345 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
<<<<<<< HEAD
#line 2458 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 341 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2521 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 351 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
<<<<<<< HEAD
#line 2468 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 346 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2531 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 356 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
<<<<<<< HEAD
#line 2478 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 353 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2484 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 359 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2541 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 363 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2547 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 369 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      if (WABT_FAILED(parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid int " PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
<<<<<<< HEAD
#line 2497 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 370 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2560 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 380 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
<<<<<<< HEAD
#line 2506 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 374 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2569 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 384 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
<<<<<<< HEAD
#line 2515 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 378 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2578 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 388 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
<<<<<<< HEAD
#line 2524 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 385 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2587 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 395 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Index;
      (yyval.var).index = (yyvsp[0].u64);
    }
<<<<<<< HEAD
#line 2534 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 390 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2597 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 400 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 2544 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 397 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2550 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 398 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2607 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 407 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2613 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 408 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->push_back((yyvsp[0].var));
    }
<<<<<<< HEAD
#line 2559 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 404 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2565 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 408 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2571 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 412 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2577 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 35:
#line 417 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2583 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 418 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2622 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 414 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2628 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 418 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2634 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 422 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2640 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 35:
#line 427 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2646 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 428 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
    if (WABT_FAILED(parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64),
                                ParseIntType::SignedAndUnsigned))) {
      wast_parser_error(&(yylsp[0]), lexer, parser,
                        "invalid offset \"" PRIstringslice "\"",
                        WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
<<<<<<< HEAD
#line 2596 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 428 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2602 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 429 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2659 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 438 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2665 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 439 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
    if (WABT_FAILED(parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                ParseIntType::UnsignedOnly))) {
      wast_parser_error(&(yylsp[0]), lexer, parser,
                        "invalid alignment \"" PRIstringslice "\"",
                        WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
<<<<<<< HEAD
#line 2615 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 440 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2621 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 441 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2627 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 442 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 2633 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 445 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2641 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 448 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2649 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 451 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2657 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 454 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2665 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 457 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr((yyvsp[0].var));
    }
#line 2673 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 460 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf((yyvsp[0].var));
    }
#line 2681 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 463 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), (yyvsp[0].var));
    }
#line 2689 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 466 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2697 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 469 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall((yyvsp[0].var));
    }
#line 2705 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 472 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect((yyvsp[0].var));
    }
#line 2713 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 475 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal((yyvsp[0].var));
    }
#line 2721 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 478 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal((yyvsp[0].var));
    }
#line 2729 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 481 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal((yyvsp[0].var));
    }
#line 2737 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 484 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal((yyvsp[0].var));
    }
#line 2745 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 487 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal((yyvsp[0].var));
    }
#line 2753 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 490 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2761 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 493 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2769 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 496 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2678 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 450 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2684 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 451 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2690 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 456 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2698 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 459 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2706 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 462 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2714 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 465 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2722 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 468 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr((yyvsp[0].var));
    }
#line 2730 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 471 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf((yyvsp[0].var));
    }
#line 2738 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 474 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), (yyvsp[0].var));
    }
#line 2746 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 477 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2754 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 480 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall((yyvsp[0].var));
    }
#line 2762 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 483 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect((yyvsp[0].var));
    }
#line 2770 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 486 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal((yyvsp[0].var));
    }
#line 2778 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 489 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal((yyvsp[0].var));
    }
#line 2786 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 492 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal((yyvsp[0].var));
    }
#line 2794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 495 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal((yyvsp[0].var));
    }
#line 2802 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 498 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal((yyvsp[0].var));
    }
#line 2810 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 501 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2818 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 504 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2826 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 507 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Const const_;
      WABT_ZERO_MEMORY(const_);
      const_.loc = (yylsp[-1]);
      if (WABT_FAILED(parse_const((yyvsp[-1].type), (yyvsp[0].literal).type, (yyvsp[0].literal).text.start,
                                  (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &const_))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      delete [] (yyvsp[0].literal).text.start;
      (yyval.expr) = Expr::CreateConst(const_);
    }
<<<<<<< HEAD
#line 2787 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 509 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2795 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 512 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 2803 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 515 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 2811 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 518 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 2819 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 521 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 2827 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 524 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 2835 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 529 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2844 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 520 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2852 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 523 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 2860 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 526 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 2868 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 529 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 2876 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 532 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 2884 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 535 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 2892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 538 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateThrow((yyvsp[0].var));
    }
#line 2900 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 541 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateRethrow((yyvsp[0].var));
    }
#line 2908 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 547 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr) = Expr::CreateBlock((yyvsp[-2].block));
      (yyval.expr)->block->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block->label, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 2845 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 534 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2918 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 552 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr) = Expr::CreateLoop((yyvsp[-2].block));
      (yyval.expr)->loop->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->loop->label, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 2855 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 539 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2928 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 557 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-2].block), nullptr);
      (yyval.expr)->if_.true_->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 2865 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 544 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2938 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 562 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-5].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->if_.true_->label = (yyvsp[-6].text);
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->if_.true_->label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 2876 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 552 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 2882 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 555 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2949 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 568 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-3].block)->label = (yyvsp[-4].text);
      (yyval.expr) = Expr::CreateTry((yyvsp[-3].block), (yyvsp[-2].expr_list).first);
      CHECK_END_LABEL((yylsp[0]), (yyvsp[-3].block)->label, (yyvsp[0].text));
    }
#line 2959 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 576 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 2965 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 579 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.block) = (yyvsp[0].block);
      (yyval.block)->sig.insert((yyval.block)->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 2892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 560 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2975 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 584 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.block) = new Block();
      (yyval.block)->first = (yyvsp[0].expr_list).first;
    }
<<<<<<< HEAD
#line 2901 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 567 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 2907 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 571 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 2915 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 574 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 2984 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 591 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatch((yyvsp[-1].var), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2993 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 595 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatchAll((yyvsp[-1].var), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3002 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 79:
#line 603 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-1].expr_list), &(yyvsp[0].expr_list));
    }
#line 3010 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 609 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 3016 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 613 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 3024 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 616 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateBlock((yyvsp[0].block));
      expr->block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
<<<<<<< HEAD
#line 2925 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 579 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3034 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 621 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateLoop((yyvsp[0].block));
      expr->loop->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
<<<<<<< HEAD
#line 2935 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 584 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3044 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 626 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      if_->if_.true_->label = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 2946 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 592 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3055 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 632 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-2].block)->label = (yyvsp[-3].text);
      Expr* try_ = Expr::CreateTry((yyvsp[-2].block), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-6]), try_);
    }
#line 3065 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 640 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3073 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 643 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-2].expr_list), &(yyvsp[0].expr_list));
    }
#line 3081 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 649 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->if_.true_;
      true_->sig.insert(true_->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 2959 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 603 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 90:
#line 660 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
<<<<<<< HEAD
#line 2968 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 607 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3103 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 664 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
<<<<<<< HEAD
#line 2977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 611 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3112 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 668 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
<<<<<<< HEAD
#line 2986 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 615 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3121 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 672 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
<<<<<<< HEAD
#line 2995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 619 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3130 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 676 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
<<<<<<< HEAD
#line 3004 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 623 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3139 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 680 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[0].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
<<<<<<< HEAD
#line 3013 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 630 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 631 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3148 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 687 "src/wast-parser.y" /* yacc.c:1646  */
    {
     CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "rethrow");
    }
#line 3156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 692 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "throw");
    }
#line 3164 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 698 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "try");      
    }
#line 3172 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 704 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3178 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 705 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
<<<<<<< HEAD
#line 3030 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 639 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3036 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 640 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3189 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 713 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3195 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 714 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
<<<<<<< HEAD
#line 3047 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 653 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Except);
      (yyval.module_field)->except.sig = std::move(*(yyvsp[-1].types));
    }
#line 3056 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 661 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3206 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 104:
#line 727 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main = (yyval.module_fields).first;
      main->loc = (yylsp[-3]);
      if (main->type == ModuleFieldType::Func) {
        main->func->name = (yyvsp[-2].text);
      } else {
        assert(main->type == ModuleFieldType::Import);
        main->import->func->name = (yyvsp[-2].text);
      }
    }
<<<<<<< HEAD
#line 3072 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 675 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3222 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 741 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3084 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 682 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3234 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 748 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 687 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3244 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 753 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-2]);
      field->import = (yyvsp[-2].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      field->import->func->decl.has_func_type = true;
      field->import->func->decl.type_var = (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3109 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 697 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3259 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 763 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3122 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 705 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3272 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 771 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
<<<<<<< HEAD
#line 3135 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 716 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3285 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 782 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
<<<<<<< HEAD
#line 3144 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 723 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3150 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 724 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3294 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 789 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3300 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 790 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = new Func();
      (yyval.func)->decl.sig.result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
<<<<<<< HEAD
#line 3160 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 729 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3310 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 795 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
<<<<<<< HEAD
#line 3171 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 735 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3321 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 801 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
<<<<<<< HEAD
#line 3183 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 745 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3333 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 811 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
<<<<<<< HEAD
#line 3192 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 753 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3342 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 819 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types = std::move(*(yyvsp[-2].types));
      delete (yyvsp[-2].types);
    }
<<<<<<< HEAD
#line 3202 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 758 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3352 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 824 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
<<<<<<< HEAD
#line 3213 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 764 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3363 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 830 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
<<<<<<< HEAD
#line 3225 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 774 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3375 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 120:
#line 840 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
<<<<<<< HEAD
#line 3234 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 781 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3384 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 847 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = new Func();
      (yyval.func)->first_expr = (yyvsp[0].expr_list).first;
    }
<<<<<<< HEAD
#line 3243 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 785 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3393 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 851 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
<<<<<<< HEAD
#line 3253 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 790 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3403 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 856 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].type));
    }
<<<<<<< HEAD
#line 3265 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 802 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3273 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 809 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3415 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 868 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3423 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 875 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::ElemSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->elem_segment = new ElemSegment();
      (yyval.module_field)->elem_segment->table_var = (yyvsp[-3].var);
      (yyval.module_field)->elem_segment->offset = (yyvsp[-2].expr_list).first;
      (yyval.module_field)->elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
    }
<<<<<<< HEAD
#line 3287 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 818 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3437 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 884 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::ElemSegment);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->elem_segment = new ElemSegment();
      (yyval.module_field)->elem_segment->table_var.loc = (yylsp[-3]);
      (yyval.module_field)->elem_segment->table_var.type = VarType::Index;
      (yyval.module_field)->elem_segment->table_var.index = 0;
      (yyval.module_field)->elem_segment->offset = (yyvsp[-2].expr_list).first;
      (yyval.module_field)->elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
    }
<<<<<<< HEAD
#line 3303 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 832 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3453 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 898 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main = (yyval.module_fields).first;
      main->loc = (yylsp[-3]);
      if (main->type == ModuleFieldType::Table) {
        main->table->name = (yyvsp[-2].text);
      } else {
        assert(main->type == ModuleFieldType::Import);
        main->import->table->name = (yyvsp[-2].text);
      }
    }
<<<<<<< HEAD
#line 3319 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 846 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3469 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 912 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Table);
      field->loc = (yylsp[0]);
      field->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3330 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 852 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3480 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 130:
#line 918 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3343 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 860 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3493 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 926 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
<<<<<<< HEAD
#line 3356 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 120:
#line 868 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3506 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 934 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* table_field = new ModuleField(ModuleFieldType::Table);
      Table* table = table_field->table = new Table();
      table->elem_limits.initial = (yyvsp[-1].vars)->size();
      table->elem_limits.max = (yyvsp[-1].vars)->size();
      table->elem_limits.has_max = true;
      ModuleField* elem_field = new ModuleField(ModuleFieldType::ElemSegment);
      elem_field->loc = (yylsp[-2]);
      ElemSegment* elem_segment = elem_field->elem_segment = new ElemSegment();
      elem_segment->offset = Expr::CreateConst(Const(Const::I32(), 0));
      elem_segment->offset->loc = (yylsp[-2]);
      elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
      (yyval.module_fields).first = table_field;
      (yyval.module_fields).last = table_field->next = elem_field;
    }
<<<<<<< HEAD
#line 3377 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 887 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3527 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 953 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::DataSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->data_segment = new DataSegment();
      (yyval.module_field)->data_segment->memory_var = (yyvsp[-3].var);
      (yyval.module_field)->data_segment->offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.module_field)->data_segment->data, &(yyval.module_field)->data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
<<<<<<< HEAD
#line 3391 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 896 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3541 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 962 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::DataSegment);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->data_segment = new DataSegment();
      (yyval.module_field)->data_segment->memory_var.loc = (yylsp[-3]);
      (yyval.module_field)->data_segment->memory_var.type = VarType::Index;
      (yyval.module_field)->data_segment->memory_var.index = 0;
      (yyval.module_field)->data_segment->offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.module_field)->data_segment->data, &(yyval.module_field)->data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
<<<<<<< HEAD
#line 3407 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 910 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3557 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 976 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main = (yyval.module_fields).first;
      main->loc = (yylsp[-3]);
      if (main->type == ModuleFieldType::Memory) {
        main->memory->name = (yyvsp[-2].text);
      } else {
        assert(main->type == ModuleFieldType::Import);
        main->import->memory->name = (yyvsp[-2].text);
      }
    }
<<<<<<< HEAD
#line 3423 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 924 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3573 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 990 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Memory);
      field->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3433 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 929 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3583 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 995 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3446 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 937 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3596 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1003 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
<<<<<<< HEAD
#line 3459 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 945 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3609 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1011 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* data_field = new ModuleField(ModuleFieldType::DataSegment);
      data_field->loc = (yylsp[-2]);
      DataSegment* data_segment = data_field->data_segment = new DataSegment();
      data_segment->offset = Expr::CreateConst(Const(Const::I32(), 0));
      data_segment->offset->loc = (yylsp[-2]);
      dup_text_list(&(yyvsp[-1].text_list), &data_segment->data, &data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
      uint32_t byte_size = WABT_ALIGN_UP_TO_PAGE(data_segment->size);
      uint32_t page_size = WABT_BYTES_TO_PAGES(byte_size);

      ModuleField* memory_field = new ModuleField(ModuleFieldType::Memory);
      memory_field->loc = (yylsp[-2]);
      Memory* memory = memory_field->memory = new Memory();
      memory->page_limits.initial = page_size;
      memory->page_limits.max = page_size;
      memory->page_limits.has_max = true;
      (yyval.module_fields).first = memory_field;
      (yyval.module_fields).last = memory_field->next = data_field;
    }
<<<<<<< HEAD
#line 3484 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 968 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3634 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1034 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main = (yyval.module_fields).first;
      main->loc = (yylsp[-3]);
      if (main->type == ModuleFieldType::Global) {
        main->global->name = (yyvsp[-2].text);
      } else {
        assert(main->type == ModuleFieldType::Import);
        main->import->global->name = (yyvsp[-2].text);
      }
    }
<<<<<<< HEAD
#line 3500 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3650 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1048 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Global);
      field->global = (yyvsp[-1].global);
      field->global->init_expr = (yyvsp[0].expr_list).first;
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3511 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 130:
#line 988 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3661 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1054 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
<<<<<<< HEAD
#line 3524 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 996 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3674 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1062 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
<<<<<<< HEAD
#line 3537 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 1009 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3687 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1075 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3550 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 1017 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3700 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1083 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
<<<<<<< HEAD
#line 3563 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 1025 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3713 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1091 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = (yyvsp[-2].text);
    }
<<<<<<< HEAD
#line 3574 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 1031 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3724 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1097 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = (yyvsp[-2].text);
    }
<<<<<<< HEAD
#line 3585 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 1037 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3735 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1103 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = (yyvsp[-2].text);
    }
<<<<<<< HEAD
#line 3596 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 1046 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3746 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1112 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Import);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->import = (yyvsp[-1].import);
      (yyval.module_field)->import->module_name = (yyvsp[-3].text);
      (yyval.module_field)->import->field_name = (yyvsp[-2].text);
    }
<<<<<<< HEAD
#line 3608 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1056 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3758 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1122 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3618 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1064 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3768 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1130 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3628 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1069 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3778 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1135 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3638 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1074 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3788 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1140 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3648 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1079 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3798 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1145 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3658 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1086 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3808 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1152 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Export);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->export_ = (yyvsp[-1].export_);
      (yyval.module_field)->export_->name = (yyvsp[-2].text);
    }
<<<<<<< HEAD
#line 3669 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1095 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3819 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1161 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3678 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1105 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3828 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1171 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
<<<<<<< HEAD
#line 3690 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1112 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3840 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1178 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->name = (yyvsp[-2].text);
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
<<<<<<< HEAD
#line 3703 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1123 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3853 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1189 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Start);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->start = (yyvsp[-1].var);
    }
<<<<<<< HEAD
#line 3713 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1131 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3719 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1136 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3725 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1137 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3731 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1138 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3737 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1139 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3743 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1140 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3749 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1141 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3755 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1145 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 3761 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1150 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3863 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1197 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3869 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1202 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3875 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1203 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3881 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1204 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3887 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1205 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3893 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1206 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1210 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 3905 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1215 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module) = new Module();
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
<<<<<<< HEAD
#line 3771 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1155 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3915 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1220 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.module) = (yyvsp[-1].module);
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
<<<<<<< HEAD
#line 3781 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1163 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3925 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1228 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.raw_module) = new RawModule();
      (yyval.raw_module)->type = RawModuleType::Text;
      (yyval.raw_module)->text = (yyvsp[-1].module);
      (yyval.raw_module)->text->name = (yyvsp[-2].text);
      (yyval.raw_module)->text->loc = (yylsp[-3]);

      /* resolve func type variables where the signature was not specified
       * explicitly */
      for (Func* func: (yyvsp[-1].module)->funcs) {
        if (decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          FuncType* func_type =
              get_func_type_by_var((yyvsp[-1].module), &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
          }
        }
      }
    }
<<<<<<< HEAD
#line 3806 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1183 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3950 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1248 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.raw_module) = new RawModule();
      (yyval.raw_module)->type = RawModuleType::Binary;
      (yyval.raw_module)->binary.name = (yyvsp[-2].text);
      (yyval.raw_module)->binary.loc = (yylsp[-3]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.raw_module)->binary.data, &(yyval.raw_module)->binary.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
<<<<<<< HEAD
#line 3819 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1194 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3963 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 176:
#line 1259 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      if ((yyvsp[0].raw_module)->type == RawModuleType::Text) {
        (yyval.module) = (yyvsp[0].raw_module)->text;
        (yyvsp[0].raw_module)->text = nullptr;
      } else {
        assert((yyvsp[0].raw_module)->type == RawModuleType::Binary);
        (yyval.module) = new Module();
        ReadBinaryOptions options = WABT_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorHandlerModule error_handler(&(yyvsp[0].raw_module)->binary.loc, lexer, parser);
        read_binary_ir((yyvsp[0].raw_module)->binary.data, (yyvsp[0].raw_module)->binary.size, &options,
                       &error_handler, (yyval.module));
        (yyval.module)->name = (yyvsp[0].raw_module)->binary.name;
        (yyval.module)->loc = (yyvsp[0].raw_module)->binary.loc;
        WABT_ZERO_MEMORY((yyvsp[0].raw_module)->binary.name);
      }
      delete (yyvsp[0].raw_module);
    }
<<<<<<< HEAD
#line 3841 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1221 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3985 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1286 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Index;
      (yyval.var).index = kInvalidIndex;
    }
<<<<<<< HEAD
#line 3851 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1226 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 3995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 179:
#line 1291 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
<<<<<<< HEAD
#line 3861 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1234 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4005 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1299 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-4]);
      (yyval.action)->module_var = (yyvsp[-3].var);
      (yyval.action)->type = ActionType::Invoke;
      (yyval.action)->name = (yyvsp[-2].text);
      (yyval.action)->invoke = new ActionInvoke();
      (yyval.action)->invoke->args = std::move(*(yyvsp[-1].consts));
      delete (yyvsp[-1].consts);
    }
<<<<<<< HEAD
#line 3876 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1244 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4020 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1309 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = (yyvsp[-2].var);
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3888 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 171:
#line 1254 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4032 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1319 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertMalformed;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1260 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4043 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 183:
#line 1325 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertInvalid;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3910 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1266 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4054 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1331 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUnlinkable;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3921 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1272 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4065 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1337 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUninstantiable;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3932 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1278 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4076 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1343 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturn;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
<<<<<<< HEAD
#line 3943 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 176:
#line 1284 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4087 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1349 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnCanonicalNan;
      (yyval.command)->assert_return_canonical_nan.action = (yyvsp[-1].action);
    }
<<<<<<< HEAD
#line 3953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 177:
#line 1289 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4097 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1354 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnArithmeticNan;
      (yyval.command)->assert_return_arithmetic_nan.action = (yyvsp[-1].action);
    }
<<<<<<< HEAD
#line 3963 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1294 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4107 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1359 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertTrap;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3974 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 179:
#line 1300 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4118 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1365 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertExhaustion;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
<<<<<<< HEAD
#line 3985 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1309 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4129 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 191:
#line 1374 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Action;
      (yyval.command)->action = (yyvsp[0].action);
    }
<<<<<<< HEAD
#line 3995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1315 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4139 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 193:
#line 1380 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Module;
      (yyval.command)->module = (yyvsp[0].module);
    }
<<<<<<< HEAD
#line 4005 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 183:
#line 1320 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4149 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 194:
#line 1385 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Register;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
<<<<<<< HEAD
#line 4017 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1329 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4161 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 195:
#line 1394 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
<<<<<<< HEAD
#line 4026 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1333 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4170 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 196:
#line 1398 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
<<<<<<< HEAD
#line 4035 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1340 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4179 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 197:
#line 1405 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (WABT_FAILED(parse_const((yyvsp[-2].type), (yyvsp[-1].literal).type, (yyvsp[-1].literal).text.start,
                                  (yyvsp[-1].literal).text.start + (yyvsp[-1].literal).text.length, &(yyval.const_)))) {
        wast_parser_error(&(yylsp[-1]), lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[-1].literal).text));
      }
      delete [] (yyvsp[-1].literal).text.start;
    }
<<<<<<< HEAD
#line 4050 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1352 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4056 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1353 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4194 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 198:
#line 1417 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4200 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 199:
#line 1418 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
<<<<<<< HEAD
#line 4065 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1360 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4073 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1363 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4209 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 200:
#line 1425 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4217 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 201:
#line 1428 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.script) = new Script();
      (yyval.script)->commands = std::move(*(yyvsp[0].commands));
      delete (yyvsp[0].commands);

      int last_module_index = -1;
      for (size_t i = 0; i < (yyval.script)->commands.size(); ++i) {
        Command& command = *(yyval.script)->commands[i].get();
        Var* module_var = nullptr;
        switch (command.type) {
          case CommandType::Module: {
            last_module_index = i;

            /* Wire up module name bindings. */
            Module* module = command.module;
            if (module->name.length == 0)
              continue;

            (yyval.script)->module_bindings.emplace(string_slice_to_string(module->name),
                                        Binding(module->loc, i));
            break;
          }

          case CommandType::AssertReturn:
            module_var = &command.assert_return.action->module_var;
            goto has_module_var;
          case CommandType::AssertReturnCanonicalNan:
            module_var =
                &command.assert_return_canonical_nan.action->module_var;
            goto has_module_var;
          case CommandType::AssertReturnArithmeticNan:
            module_var =
                &command.assert_return_arithmetic_nan.action->module_var;
            goto has_module_var;
          case CommandType::AssertTrap:
          case CommandType::AssertExhaustion:
            module_var = &command.assert_trap.action->module_var;
            goto has_module_var;
          case CommandType::Action:
            module_var = &command.action->module_var;
            goto has_module_var;
          case CommandType::Register:
            module_var = &command.register_.var;
            goto has_module_var;

          has_module_var: {
            /* Resolve actions with an invalid index to use the preceding
             * module. */
            if (module_var->type == VarType::Index &&
                module_var->index == kInvalidIndex) {
              module_var->index = last_module_index;
            }
            break;
          }

          default:
            break;
        }
      }
    }
<<<<<<< HEAD
#line 4138 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 191:
#line 1423 "src/wast-parser.y" /* yacc.c:1646  */
=======
#line 4282 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 202:
#line 1488 "src/wast-parser.y" /* yacc.c:1646  */
>>>>>>> master
    {
      (yyval.script) = new Script();
      Command* command = new Command();
      command->type = CommandType::Module;
      command->module = (yyvsp[0].module);
      (yyval.script)->commands.emplace_back(command);
    }
<<<<<<< HEAD
#line 4150 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 192:
#line 1435 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4160 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
=======
#line 4294 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 203:
#line 1500 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4300 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4304 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
>>>>>>> master
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, lexer, parser, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, lexer, parser, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, lexer, parser);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, lexer, parser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, lexer, parser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, lexer, parser);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, lexer, parser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
<<<<<<< HEAD
#line 1438 "src/wast-parser.y" /* yacc.c:1906  */
=======
#line 1503 "src/wast-parser.y" /* yacc.c:1906  */
>>>>>>> master


void append_expr_list(ExprList* expr_list, ExprList* expr) {
  if (!expr->first)
    return;
  if (expr_list->last)
    expr_list->last->next = expr->first;
  else
    expr_list->first = expr->first;
  expr_list->last = expr->last;
  expr_list->size += expr->size;
}

void append_expr(ExprList* expr_list, Expr* expr) {
  if (expr_list->last)
    expr_list->last->next = expr;
  else
    expr_list->first = expr;
  expr_list->last = expr;
  expr_list->size++;
}

ExprList join_exprs1(Location* loc, Expr* expr1) {
  ExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr(&result, expr1);
  expr1->loc = *loc;
  return result;
}

ExprList join_exprs2(Location* loc, ExprList* expr1, Expr* expr2) {
  ExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr(&result, expr2);
  expr2->loc = *loc;
  return result;
}

<<<<<<< HEAD
ExprList join_expr_lists(Location* loc, ExprList* expr1, ExprList* expr2) {
  if (expr1->size == 0)
    return *expr2;
  if (expr2->size == 0)
    return *expr1;
  ExprList result;
  result.first = expr1->first;
  result.last = expr2->last;
  result.size = expr1->size + expr2->size;
  expr1->last->next = expr2->first;
  return result;
}

ExprList prepend_expr(Location* loc, Expr* expr1, Expr* expr2) {
  return join_expr_lists(join_exprs2(loc, expr1), expr2);
}

=======
ExprList join_expr_lists(ExprList* expr1, ExprList* expr2) {
  ExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  return result;
}

>>>>>>> master
Result parse_const(Type type,
                   LiteralType literal_type,
                   const char* s,
                   const char* end,
                   Const* out) {
  out->type = type;
  switch (type) {
    case Type::I32:
      return parse_int32(s, end, &out->u32, ParseIntType::SignedAndUnsigned);
    case Type::I64:
      return parse_int64(s, end, &out->u64, ParseIntType::SignedAndUnsigned);
    case Type::F32:
      return parse_float(literal_type, s, end, &out->f32_bits);
    case Type::F64:
      return parse_double(literal_type, s, end, &out->f64_bits);
    default:
      assert(0);
      break;
  }
  return Result::Error;
}

size_t copy_string_contents(StringSlice* text, char* dest) {
  const char* src = text->start + 1;
  const char* end = text->start + text->length - 1;

  char* dest_start = dest;

  while (src < end) {
    if (*src == '\\') {
      src++;
      switch (*src) {
        case 'n':
          *dest++ = '\n';
          break;
        case 'r':
          *dest++ = '\r';
          break;
        case 't':
          *dest++ = '\t';
          break;
        case '\\':
          *dest++ = '\\';
          break;
        case '\'':
          *dest++ = '\'';
          break;
        case '\"':
          *dest++ = '\"';
          break;
        default: {
          // The string should be validated already, so we know this is a hex
          // sequence.
          uint32_t hi;
          uint32_t lo;
          if (WABT_SUCCEEDED(parse_hexdigit(src[0], &hi)) &&
              WABT_SUCCEEDED(parse_hexdigit(src[1], &lo))) {
            *dest++ = (hi << 4) | lo;
          } else {
            assert(0);
          }
          src++;
          break;
        }
      }
      src++;
    } else {
      *dest++ = *src++;
    }
  }
  /* return the data length */
  return dest - dest_start;
}

void dup_text_list(TextList* text_list, char** out_data, size_t* out_size) {
  /* walk the linked list to see how much total space is needed */
  size_t total_size = 0;
  for (TextListNode* node = text_list->first; node; node = node->next) {
    /* Always allocate enough space for the entire string including the escape
     * characters. It will only get shorter, and this way we only have to
     * iterate through the string once. */
    const char* src = node->text.start + 1;
    const char* end = node->text.start + node->text.length - 1;
    size_t size = (end > src) ? (end - src) : 0;
    total_size += size;
  }
  char* result = new char [total_size];
  char* dest = result;
  for (TextListNode* node = text_list->first; node; node = node->next) {
    size_t actual_size = copy_string_contents(&node->text, dest);
    dest += actual_size;
  }
  *out_data = result;
  *out_size = dest - result;
}

void reverse_bindings(TypeVector* types, BindingHash* bindings) {
  for (auto& pair : *bindings) {
    pair.second.index = types->size() - pair.second.index - 1;
  }
}

bool is_empty_signature(const FuncSignature* sig) {
  return sig->result_types.empty() && sig->param_types.empty();
}

void append_implicit_func_declaration(Location* loc,
                                      Module* module,
                                      FuncDeclaration* decl) {
  if (decl_has_func_type(decl))
    return;

  int sig_index = get_func_type_index_by_decl(module, decl);
  if (sig_index == -1) {
    append_implicit_func_type(loc, module, &decl->sig);
  } else {
    decl->sig = module->func_types[sig_index]->sig;
  }
}

void check_import_ordering(Location* loc, WastLexer* lexer, WastParser* parser,
                           Module* module, ModuleField* first) {
  for (ModuleField* field = first; field; field = field->next) {
    if (field->type == ModuleFieldType::Import) {
      if (module->funcs.size() != module->num_func_imports ||
          module->tables.size() != module->num_table_imports ||
          module->memories.size() != module->num_memory_imports ||
          module->globals.size() != module->num_global_imports) {
        wast_parser_error(
            loc, lexer, parser,
            "imports must occur before all non-import definitions");
      }
    }
  }
}

void append_module_fields(Module* module, ModuleField* first) {
  ModuleField* main_field = first;
  Index main_index = kInvalidIndex;

  for (ModuleField* field = first; field; field = field->next) {
    StringSlice* name = nullptr;
    BindingHash* bindings = nullptr;
    Index index = kInvalidIndex;

    switch (field->type) {
      case ModuleFieldType::Func:
        append_implicit_func_declaration(&field->loc, module,
                                         &field->func->decl);
        name = &field->func->name;
        bindings = &module->func_bindings;
        index = module->funcs.size();
        module->funcs.push_back(field->func);
        break;

      case ModuleFieldType::Global:
        name = &field->global->name;
        bindings = &module->global_bindings;
        index = module->globals.size();
        module->globals.push_back(field->global);
        break;

      case ModuleFieldType::Import:
        switch (field->import->kind) {
          case ExternalKind::Func:
            append_implicit_func_declaration(&field->loc, module,
                                             &field->import->func->decl);
            name = &field->import->func->name;
            bindings = &module->func_bindings;
            index = module->funcs.size();
            module->funcs.push_back(field->import->func);
            ++module->num_func_imports;
            break;
          case ExternalKind::Table:
            name = &field->import->table->name;
            bindings = &module->table_bindings;
            index = module->tables.size();
            module->tables.push_back(field->import->table);
            ++module->num_table_imports;
            break;
          case ExternalKind::Memory:
            name = &field->import->memory->name;
            bindings = &module->memory_bindings;
            index = module->memories.size();
            module->memories.push_back(field->import->memory);
            ++module->num_memory_imports;
            break;
          case ExternalKind::Global:
            name = &field->import->global->name;
            bindings = &module->global_bindings;
            index = module->globals.size();
            module->globals.push_back(field->import->global);
            ++module->num_global_imports;
            break;
        }
        module->imports.push_back(field->import);
        break;

      case ModuleFieldType::Export:
        if (field != main_field) {
          // If this is not the main field, it must be an inline export.
          field->export_->var.type = VarType::Index;
          field->export_->var.index = main_index;
        }
        name = &field->export_->name;
        bindings = &module->export_bindings;
        index = module->exports.size();
        module->exports.push_back(field->export_);
        break;

      case ModuleFieldType::FuncType:
        name = &field->func_type->name;
        bindings = &module->func_type_bindings;
        index = module->func_types.size();
        module->func_types.push_back(field->func_type);
        break;

      case ModuleFieldType::Table:
        name = &field->table->name;
        bindings = &module->table_bindings;
        index = module->tables.size();
        module->tables.push_back(field->table);
        break;

      case ModuleFieldType::ElemSegment:
        if (field != main_field) {
          // If this is not the main field, it must be an inline elem segment.
          field->elem_segment->table_var.type = VarType::Index;
          field->elem_segment->table_var.index = main_index;
        }
        module->elem_segments.push_back(field->elem_segment);
        break;

      case ModuleFieldType::Memory:
        name = &field->memory->name;
        bindings = &module->memory_bindings;
        index = module->memories.size();
        module->memories.push_back(field->memory);
        break;

      case ModuleFieldType::DataSegment:
        if (field != main_field) {
          // If this is not the main field, it must be an inline data segment.
          field->data_segment->memory_var.type = VarType::Index;
          field->data_segment->memory_var.index = main_index;
        }
        module->data_segments.push_back(field->data_segment);
        break;

      case ModuleFieldType::Start:
        module->start = &field->start;
        break;
    }

    if (field == main_field)
      main_index = index;

    if (module->last_field)
      module->last_field->next = field;
    else
      module->first_field = field;
    module->last_field = field;

    if (name && bindings) {
      // Exported names are allowed to be empty; other names aren't.
      if (bindings == &module->export_bindings ||
          !string_slice_is_empty(name)) {
        bindings->emplace(string_slice_to_string(*name),
                          Binding(field->loc, index));
      }
    }
  }
}

Result parse_wast(WastLexer* lexer, Script** out_script,
                  SourceErrorHandler* error_handler,
                  WastParseOptions* options) {
  WastParser parser;
  WABT_ZERO_MEMORY(parser);
  static WastParseOptions default_options;
  if (options == nullptr)
    options = &default_options;
  parser.options = options;
  parser.error_handler = error_handler;
  wabt_wast_parser_debug = int(options->debug_parsing);
  int result = wabt_wast_parser_parse(lexer, &parser);
  delete [] parser.yyssa;
  delete [] parser.yyvsa;
  delete [] parser.yylsa;
  *out_script = parser.script;
  return result == 0 && parser.errors == 0 ? Result::Ok : Result::Error;
}

BinaryErrorHandlerModule::BinaryErrorHandlerModule(
    Location* loc, WastLexer* lexer, WastParser* parser)
  : loc_(loc), lexer_(lexer), parser_(parser) {}

bool BinaryErrorHandlerModule::OnError(Offset offset,
                                       const std::string& error) {
  if (offset == kInvalidOffset) {
    wast_parser_error(loc_, lexer_, parser_, "error in binary module: %s",
                      error.c_str());
  } else {
    wast_parser_error(loc_, lexer_, parser_,
                      "error in binary module: @0x%08" PRIzx ": %s", offset,
                      error.c_str());
  }
  return true;
}

}  // namespace wabt
