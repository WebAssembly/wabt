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

#define CHECK_ALLOW_EXCEPTIONS(loc, opcode_name)                      \
  do {                                                                \
    if (!parser->options->allow_exceptions) {                         \
      wast_parser_error(loc, lexer, parser, "opcode not allowed: %s", \
                        opcode_name);                                 \
    }                                                                 \
 } while (0)

#define YYMALLOC(size) new char [size]
#define YYFREE(p) delete [] (p)

#define USE_NATURAL_ALIGNMENT (~0)

namespace wabt {

static bool is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static ExprList join_exprs1(Location* loc, Expr* expr1);
static ExprList join_exprs2(Location* loc, ExprList* expr1, Expr* expr2);
static ExprList join_expr_lists(ExprList* expr1, ExprList* expr2);

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


#line 214 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:339  */

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
    WABT_TOKEN_TYPE_TRY = 279,
    WABT_TOKEN_TYPE_CATCH = 280,
    WABT_TOKEN_TYPE_CATCH_ALL = 281,
    WABT_TOKEN_TYPE_THROW = 282,
    WABT_TOKEN_TYPE_RETHROW = 283,
    WABT_TOKEN_TYPE_LPAR_CATCH = 284,
    WABT_TOKEN_TYPE_LPAR_CATCH_ALL = 285,
    WABT_TOKEN_TYPE_CALL = 286,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 287,
    WABT_TOKEN_TYPE_RETURN = 288,
    WABT_TOKEN_TYPE_GET_LOCAL = 289,
    WABT_TOKEN_TYPE_SET_LOCAL = 290,
    WABT_TOKEN_TYPE_TEE_LOCAL = 291,
    WABT_TOKEN_TYPE_GET_GLOBAL = 292,
    WABT_TOKEN_TYPE_SET_GLOBAL = 293,
    WABT_TOKEN_TYPE_LOAD = 294,
    WABT_TOKEN_TYPE_STORE = 295,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 296,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 297,
    WABT_TOKEN_TYPE_CONST = 298,
    WABT_TOKEN_TYPE_UNARY = 299,
    WABT_TOKEN_TYPE_BINARY = 300,
    WABT_TOKEN_TYPE_COMPARE = 301,
    WABT_TOKEN_TYPE_CONVERT = 302,
    WABT_TOKEN_TYPE_SELECT = 303,
    WABT_TOKEN_TYPE_UNREACHABLE = 304,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 305,
    WABT_TOKEN_TYPE_GROW_MEMORY = 306,
    WABT_TOKEN_TYPE_FUNC = 307,
    WABT_TOKEN_TYPE_START = 308,
    WABT_TOKEN_TYPE_TYPE = 309,
    WABT_TOKEN_TYPE_PARAM = 310,
    WABT_TOKEN_TYPE_RESULT = 311,
    WABT_TOKEN_TYPE_LOCAL = 312,
    WABT_TOKEN_TYPE_GLOBAL = 313,
    WABT_TOKEN_TYPE_TABLE = 314,
    WABT_TOKEN_TYPE_ELEM = 315,
    WABT_TOKEN_TYPE_MEMORY = 316,
    WABT_TOKEN_TYPE_DATA = 317,
    WABT_TOKEN_TYPE_OFFSET = 318,
    WABT_TOKEN_TYPE_IMPORT = 319,
    WABT_TOKEN_TYPE_EXPORT = 320,
    WABT_TOKEN_TYPE_EXCEPT = 321,
    WABT_TOKEN_TYPE_MODULE = 322,
    WABT_TOKEN_TYPE_BIN = 323,
    WABT_TOKEN_TYPE_QUOTE = 324,
    WABT_TOKEN_TYPE_REGISTER = 325,
    WABT_TOKEN_TYPE_INVOKE = 326,
    WABT_TOKEN_TYPE_GET = 327,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 328,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 329,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 330,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 331,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 332,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 333,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 334,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 335,
    WABT_TOKEN_TYPE_LOW = 336
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

#line 369 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */

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
#define YYFINAL  52
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1101

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  90
/* YYNRULES -- Number of rules.  */
#define YYNRULES  220
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  483

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   336

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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81
};

#if WABT_WAST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   264,   264,   270,   280,   281,   285,   303,   304,   310,
     313,   318,   326,   330,   331,   336,   345,   346,   354,   360,
     366,   371,   378,   384,   395,   399,   403,   410,   414,   422,
     423,   430,   431,   434,   438,   439,   443,   444,   460,   461,
     476,   477,   478,   482,   485,   488,   491,   494,   498,   502,
     506,   509,   513,   517,   521,   525,   529,   533,   537,   540,
     543,   556,   559,   562,   565,   568,   571,   574,   578,   585,
     590,   595,   600,   606,   614,   617,   622,   629,   636,   643,
     644,   648,   649,   655,   659,   662,   667,   672,   678,   686,
     692,   696,   700,   713,   716,   722,   723,   729,   737,   740,
     744,   748,   752,   756,   760,   767,   772,   778,   784,   785,
     793,   794,   802,   807,   815,   824,   838,   846,   851,   862,
     870,   881,   888,   889,   895,   905,   906,   915,   922,   923,
     929,   939,   940,   949,   956,   960,   965,   977,   980,   984,
     994,  1008,  1022,  1028,  1036,  1044,  1064,  1074,  1088,  1102,
    1107,  1115,  1123,  1147,  1161,  1167,  1175,  1188,  1197,  1205,
    1211,  1217,  1223,  1231,  1241,  1249,  1255,  1261,  1267,  1273,
    1281,  1290,  1300,  1307,  1318,  1327,  1328,  1329,  1330,  1331,
    1332,  1333,  1334,  1335,  1336,  1337,  1341,  1342,  1346,  1351,
    1359,  1379,  1386,  1389,  1397,  1415,  1423,  1434,  1445,  1456,
    1462,  1468,  1474,  1480,  1486,  1491,  1496,  1502,  1511,  1516,
    1517,  1522,  1532,  1536,  1543,  1555,  1556,  1563,  1566,  1626,
    1638
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
  "TRY", "CATCH", "CATCH_ALL", "THROW", "RETHROW", "LPAR_CATCH",
  "LPAR_CATCH_ALL", "CALL", "CALL_INDIRECT", "RETURN", "GET_LOCAL",
  "SET_LOCAL", "TEE_LOCAL", "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE",
  "OFFSET_EQ_NAT", "ALIGN_EQ_NAT", "CONST", "UNARY", "BINARY", "COMPARE",
  "CONVERT", "SELECT", "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY",
  "FUNC", "START", "TYPE", "PARAM", "RESULT", "LOCAL", "GLOBAL", "TABLE",
  "ELEM", "MEMORY", "DATA", "OFFSET", "IMPORT", "EXPORT", "EXCEPT",
  "MODULE", "BIN", "QUOTE", "REGISTER", "INVOKE", "GET",
  "ASSERT_MALFORMED", "ASSERT_INVALID", "ASSERT_UNLINKABLE",
  "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
  "ASSERT_RETURN_ARITHMETIC_NAN", "ASSERT_TRAP", "ASSERT_EXHAUSTION",
  "LOW", "$accept", "text_list", "text_list_opt", "quoted_text",
  "value_type_list", "elem_type", "global_type", "func_type", "func_sig",
  "func_sig_result", "table_sig", "memory_sig", "limits", "type_use",
  "nat", "literal", "var", "var_list", "bind_var_opt", "bind_var",
  "labeling_opt", "offset_opt", "align_opt", "instr", "plain_instr",
  "block_instr", "block_sig", "block", "plain_catch", "plain_catch_all",
  "catch_instr", "catch_instr_list", "expr", "expr1", "try_",
  "try_instr_list", "catch_sexp", "catch_sexp_list", "if_block", "if_",
  "rethrow_check", "throw_check", "try_check", "instr_list", "expr_list",
  "const_expr", "exception", "exception_field", "func", "func_fields",
  "func_fields_import", "func_fields_import1", "func_fields_import_result",
  "func_fields_body", "func_fields_body1", "func_result_body", "func_body",
  "func_body1", "offset", "elem", "table", "table_fields", "data",
  "memory", "memory_fields", "global", "global_fields", "import_desc",
  "import", "inline_import", "export_desc", "export", "inline_export",
  "type_def", "start", "module_field", "module_fields_opt",
  "module_fields", "module", "inline_module", "script_var_opt",
  "script_module", "action", "assertion", "cmd", "cmd_list", "const",
  "const_list", "script", "script_start", YY_NULLPTR
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336
};
# endif

#define YYPACT_NINF -375

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-375)))

#define YYTABLE_NINF -31

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      14,  1021,  -375,  -375,  -375,  -375,  -375,  -375,  -375,  -375,
    -375,  -375,  -375,  -375,  -375,    37,  -375,  -375,  -375,  -375,
    -375,  -375,    47,  -375,    32,    74,   216,    18,    74,    74,
     124,    74,   124,    97,    97,    74,    74,    97,   105,   105,
     118,   118,   118,   139,   139,   139,   151,   139,   150,  -375,
     214,  -375,  -375,  -375,   536,  -375,  -375,  -375,  -375,   140,
     108,   158,   174,    34,   156,   407,   226,  -375,  -375,    54,
     226,   164,  -375,    97,   239,  -375,    57,   105,  -375,    97,
      97,   146,    97,    97,    97,   147,  -375,   241,   244,   167,
      97,    97,    97,   361,  -375,  -375,    74,    74,    74,   216,
     216,  -375,  -375,  -375,  -375,   216,   216,  -375,   216,   216,
     216,   216,   216,   274,   274,   260,  -375,  -375,  -375,  -375,
    -375,  -375,  -375,  -375,   575,   614,  -375,  -375,  -375,   216,
     216,    74,  -375,   297,  -375,  -375,  -375,  -375,  -375,   317,
     536,  -375,   320,  -375,   321,     7,  -375,   614,   322,    36,
      34,   210,  -375,   324,  -375,   313,   323,   325,   323,   156,
      74,    74,    74,   614,   328,   330,    74,  -375,   223,   120,
    -375,  -375,   331,   323,    54,   164,  -375,   247,   332,   329,
      91,   333,   130,   164,   164,   334,    37,   335,  -375,   337,
     338,   352,   354,   279,  -375,  -375,   356,   357,   358,   216,
      74,  -375,    74,    97,    97,  -375,   653,   653,   653,  -375,
    -375,   216,  -375,  -375,  -375,  -375,  -375,  -375,  -375,  -375,
     326,   326,  -375,  -375,  -375,  -375,   770,  -375,  1020,  -375,
    -375,  -375,   653,  -375,   142,   364,  -375,  -375,  -375,  -375,
     253,   360,  -375,  -375,   367,  -375,  -375,  -375,   368,  -375,
    -375,   319,  -375,  -375,  -375,  -375,  -375,   653,   383,   653,
     386,   328,  -375,  -375,   458,   231,  -375,  -375,   164,  -375,
    -375,  -375,   387,  -375,  -375,   165,  -375,   398,   216,   216,
     216,   216,   216,  -375,  -375,  -375,   249,   250,  -375,  -375,
     307,  -375,  -375,  -375,  -375,   344,  -375,  -375,  -375,  -375,
    -375,   399,   137,   403,   141,   154,   404,    97,   415,   941,
     653,   416,  -375,    59,   417,   263,  -375,  -375,  -375,   287,
      74,  -375,   261,  -375,    74,  -375,  -375,   419,  -375,  -375,
     901,   383,   433,  -375,  -375,  -375,  -375,  -375,   434,   445,
     497,   458,  -375,  -375,   289,  -375,  -375,   455,  -375,    74,
      74,    74,    74,  -375,   456,   459,   460,   461,   462,  -375,
    -375,  -375,   260,  -375,   575,   463,   692,   731,   464,   465,
    -375,  -375,  -375,    74,    74,    74,    74,   216,   614,  -375,
    -375,   287,   446,   169,   439,   170,   190,   466,   191,  -375,
     242,   614,  -375,   981,   328,   449,   451,  -375,  -375,  -375,
    -375,   480,    36,   323,   323,  -375,  -375,  -375,  -375,  -375,
     495,  -375,   575,   815,  -375,   860,  -375,   731,  -375,   195,
    -375,  -375,   614,  -375,   614,  -375,  -375,    74,   364,   509,
     481,   320,   511,   513,  -375,   518,   614,  -375,   519,   534,
     251,   548,   550,   551,   557,   558,  -375,  -375,  -375,  -375,
     549,  -375,  -375,  -375,   364,   510,  -375,  -375,   320,   521,
    -375,   588,   589,  -375,  -375,  -375,  -375,  -375,  -375,  -375,
      74,  -375,  -375,   581,   591,  -375,   614,   582,   600,   614,
    -375,   601,  -375
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     217,     0,   114,   185,   179,   180,   177,   181,   178,   176,
     183,   184,   175,   182,   188,   191,   210,   219,   190,   208,
     209,   212,   218,   220,     0,    31,     0,     0,    31,    31,
       0,    31,     0,     0,     0,    31,    31,     0,   192,   192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   189,
       0,   213,     1,    33,   108,    32,    23,    28,    27,     0,
       0,     0,     0,     0,     0,     0,     0,   138,    29,     0,
       0,     4,     6,     0,     0,     7,   186,   192,   193,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,     0,     0,     0,    44,    45,    34,    34,    34,     0,
       0,    29,   107,   106,   105,     0,     0,    50,     0,     0,
       0,     0,     0,    36,    36,     0,    61,    62,    63,    64,
      46,    43,    65,    66,   108,   108,    40,    41,    42,     0,
       0,    34,   134,     0,   117,   127,   128,   131,   133,   125,
     108,   174,    16,   172,     0,     0,    10,   108,     0,     0,
       0,     0,     9,     0,   142,     0,    20,     0,     0,     0,
      34,    34,    34,   108,   110,     0,    34,    29,     0,     0,
     149,    19,     0,     0,     0,     4,     2,     5,     0,     0,
       0,     0,     0,     0,     0,     0,   187,     0,   215,     0,
       0,     0,     0,     0,   204,   205,     0,     0,     0,     0,
       7,     7,     7,     0,     0,    35,   108,   108,   108,    47,
      48,     0,    51,    52,    53,    54,    55,    56,    57,    37,
      38,    38,    24,    25,    26,    60,     0,   116,     0,   109,
      68,    67,   108,   115,     0,   125,   119,   121,   122,   120,
       0,     0,    13,   173,     0,   112,   154,   153,     0,   155,
     156,     0,    18,    21,   141,   143,   144,   108,     0,   108,
       0,   110,    84,    83,     0,     0,   140,    30,     4,   148,
     150,   151,     0,     3,   147,     0,   162,     0,     0,     0,
       0,     0,     0,   170,   113,     8,     0,     0,   194,   211,
       0,   198,   199,   200,   201,     0,   203,   216,   202,   206,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,    76,     0,     0,    49,    39,    58,    59,     0,
       7,     7,     0,   118,     7,     7,    12,     0,    29,    85,
       0,     0,     0,    87,    98,    86,   137,   111,     0,     0,
       0,     0,    88,    90,    95,    91,   139,     0,   146,    31,
      31,    31,    31,   163,     0,     0,     0,     0,     0,   195,
     196,   197,     0,    22,   108,     0,   108,   108,     0,     0,
     171,     7,    75,    34,    34,    34,    34,     0,   108,    79,
      80,    81,     0,     0,     0,     0,     0,     0,     0,    11,
       0,   108,    97,     0,   104,     0,     0,    92,    89,    96,
     152,    16,     0,     0,     0,   165,   168,   166,   167,   169,
       0,   129,   108,     0,   132,     0,   135,   108,   164,     0,
      69,    71,   108,    70,   108,    78,    82,    34,   125,     0,
     125,    16,     0,    16,   145,     0,   108,   103,     0,     0,
       0,     0,     0,     0,     0,     0,   214,   130,   136,    74,
       0,    77,    73,   123,   125,     0,   126,    14,    16,     0,
      17,   100,     0,    93,    94,   158,   157,   161,   159,   160,
      34,   124,    15,     0,   102,    72,   108,     0,     0,   108,
      99,     0,   101
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -375,   138,  -159,    40,  -134,   475,  -129,   554,  -293,   199,
    -151,  -163,   -56,  -130,   -58,   271,   -18,   -78,     0,    20,
     -97,   525,   422,  -234,   -60,  -375,  -185,  -104,   245,   248,
    -375,   288,   -28,  -375,   314,   339,  -375,   327,   341,  -375,
    -375,  -375,   -50,  -122,   421,   515,   504,  -375,  -375,   568,
     476,  -374,   264,   586,  -342,   351,  -375,  -341,    29,  -375,
    -375,   559,  -375,  -375,   547,  -375,   572,  -375,  -375,   -11,
    -375,  -375,    33,  -375,  -375,     3,  -375,   657,  -375,  -375,
     -15,   129,   252,  -375,   725,  -375,  -375,   561,  -375,  -375
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   177,   178,    73,   182,   153,   147,    61,   241,   242,
     154,   170,   155,   124,    58,   225,   267,   168,    54,   205,
     206,   220,   317,   125,   126,   127,   310,   311,   379,   380,
     381,   382,   128,   165,   342,   343,   344,   345,   333,   334,
     129,   130,   131,   132,   262,   246,     2,     3,     4,   133,
     236,   237,   238,   134,   135,   136,   137,   138,    68,     5,
       6,   157,     7,     8,   172,     9,   148,   277,    10,   139,
     181,    11,   140,    12,    13,    14,   185,    15,    16,    17,
      79,    18,    19,    20,    21,    22,   297,   193,    23,    24
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     207,   208,    67,   229,    67,   164,   156,   255,    59,   235,
     270,   156,    66,   171,    70,   166,   272,     1,    49,   244,
     249,    60,   411,   211,    80,   245,   416,    53,    63,    64,
     340,    69,    52,   164,   232,    75,    76,   145,    67,   248,
      48,   245,    67,   166,   146,    55,   146,    62,    55,    55,
      50,    55,   149,   158,   453,    55,    55,   169,   173,    56,
      48,    71,   187,   257,   258,   259,   302,   304,   305,   264,
     447,   203,   204,   331,    74,   374,   448,    77,   375,   341,
     471,   209,   210,    53,   312,   312,   312,   212,   213,   265,
     214,   215,   216,   217,   218,   167,   150,   159,   253,   175,
     156,   156,   174,   313,   314,    72,   340,   340,   441,   347,
     312,   230,   231,   179,    78,   156,   156,   171,   171,   188,
     189,    81,   190,   191,   192,   183,   184,    65,   319,    56,
     196,   197,   198,    57,   284,   312,   261,   312,   457,   149,
     285,   364,    85,   278,   141,   366,   331,   285,   158,   279,
     280,   285,   281,   329,    89,   335,   341,   282,   367,   151,
     142,    56,   143,   173,   285,   472,   164,   152,   164,    82,
      83,    84,   176,   428,   430,    90,   166,    60,   166,   285,
     285,   301,   268,   150,   203,   204,   383,   385,   312,    49,
     386,   388,   159,   315,   431,   433,   199,   320,   321,   449,
     285,   285,    25,    26,    27,   285,   372,   174,    28,    29,
      30,    31,    32,    36,    33,    34,    35,   349,    38,    39,
     303,    56,   306,   350,   351,    57,   352,   266,    56,    65,
     332,    35,    57,   261,    36,   346,    56,   419,    38,    39,
      57,   445,   180,   307,   308,   194,   434,    56,   195,   164,
     390,    57,   444,   359,   360,   273,   425,   273,   273,   166,
     354,   355,   356,   357,   358,   222,   223,   224,   -30,   435,
     164,   442,   -30,   443,   203,   204,   420,   421,   422,   423,
     166,    36,   295,   296,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    86,    87,    88,    91,    92,
     450,   233,   451,   332,   394,   199,   324,   325,   324,   325,
     295,   361,   377,   378,   462,   219,   320,   321,   338,   339,
     234,   286,   287,   240,   152,   243,   247,   251,    56,   254,
     452,   228,   275,   164,   263,   269,   274,   283,   288,   289,
     384,   291,   292,   166,   387,   156,   156,   369,   171,   401,
     402,   403,   404,   164,   478,   164,   293,   481,   294,   424,
     298,   299,   300,   166,   326,   166,   437,   322,   316,    55,
      55,    55,    55,   475,    94,    95,   160,   327,   161,   328,
     244,   162,    99,   100,   101,   102,   330,   362,   103,   104,
     336,   348,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   353,   363,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   365,   368,   199,   200,   201,   202,   370,
      94,    95,   160,   389,   161,   203,   204,   162,    99,   100,
     101,   102,   373,   376,   103,   104,   393,   395,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   396,   429,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   400,
     405,   309,   427,   406,   407,   408,   409,   412,   417,   418,
     163,    94,    95,    96,   377,    97,   432,   378,    98,    99,
     100,   101,   102,   440,   455,   103,   104,   338,   339,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   446,
     228,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      94,    95,    96,   454,    97,   458,   459,    98,    99,   100,
     101,   102,   461,   463,   103,   104,   338,   339,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   464,    93,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    94,
      95,    96,   465,    97,   466,   467,    98,    99,   100,   101,
     102,   468,   469,   103,   104,   470,   321,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   325,   226,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    94,    95,
      96,   473,    97,   474,   477,    98,    99,   100,   101,   102,
     476,   479,   103,   104,   480,   482,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   144,   228,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    94,    95,    96,
     252,    97,   460,   410,    98,    99,   100,   101,   102,   221,
     438,   103,   104,   318,   439,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   398,   309,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    94,    95,    96,   426,
      97,   399,   392,    98,    99,   100,   101,   102,   260,   397,
     103,   104,   337,   276,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   456,   413,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    94,    95,    96,   239,    97,
     227,   323,    98,    99,   100,   101,   102,   414,   256,   103,
     104,   271,   250,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   186,   415,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    94,    95,    96,    51,    97,   290,
       0,    98,    99,   100,   101,   102,     0,     0,   103,   104,
       0,     0,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    94,    95,   160,     0,   161,     0,     0,
     162,    99,   100,   101,   102,     0,     0,   103,   104,     0,
       0,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,     0,   115,   116,   117,   118,   119,   120,   121,
     122,   123,     0,     0,     0,   200,   201,   202,    94,    95,
     160,     0,   161,     0,     0,   162,    99,   100,   101,   102,
       0,     0,   103,   104,     0,     0,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,     0,   115,   116,
     117,   118,   119,   120,   121,   122,   123,     0,     0,     0,
       0,   201,   202,    94,    95,   160,     0,   161,     0,     0,
     162,    99,   100,   101,   102,     0,     0,   103,   104,     0,
       0,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,     0,   115,   116,   117,   118,   119,   120,   121,
     122,   123,     0,     0,    94,    95,   160,   202,   161,   391,
       0,   162,    99,   100,   101,   102,     0,     0,   103,   104,
       0,     0,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   115,   116,   117,   118,   119,   120,
     121,   122,   123,     0,    94,    95,   160,   371,   161,     0,
       0,   162,    99,   100,   101,   102,     0,     0,   103,   104,
       0,     0,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   115,   116,   117,   118,   119,   120,
     121,   122,   123,     0,    94,    95,   160,   371,   161,   436,
       0,   162,    99,   100,   101,   102,     0,     0,   103,   104,
       0,     0,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    94,    95,   160,     0,   161,     0,     0,
     162,    99,   100,   101,   102,     0,     0,   103,   104,     0,
       0,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,     0,   115,   116,   117,   118,   119,   120,   121,
     122,   123,     0,    25,    26,    27,     0,     0,     0,    28,
      29,    30,    31,    32,     0,    33,    34,    35,    36,     0,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47
};

static const yytype_int16 yycheck[] =
{
      97,    98,    30,   125,    32,    65,    64,   158,    26,   139,
     173,    69,    30,    69,    32,    65,   175,     3,    15,    12,
     149,     3,   364,   101,    39,   147,   367,     9,    28,    29,
     264,    31,     0,    93,   131,    35,    36,     3,    66,     3,
       3,   163,    70,    93,    10,    25,    10,    27,    28,    29,
       3,    31,    63,    64,   428,    35,    36,     3,    69,     5,
       3,    32,    77,   160,   161,   162,   200,   201,   202,   166,
     412,    64,    65,   258,    34,    16,   417,    37,    19,   264,
     454,    99,   100,     9,   206,   207,   208,   105,   106,   167,
     108,   109,   110,   111,   112,    66,    63,    64,   156,    70,
     158,   159,    69,   207,   208,     8,   340,   341,   401,   268,
     232,   129,   130,    73,     9,   173,   174,   173,   174,    79,
      80,     3,    82,    83,    84,    68,    69,     3,   232,     5,
      90,    91,    92,     9,     4,   257,   164,   259,   431,   150,
      10,     4,     3,    52,     4,     4,   331,    10,   159,    58,
      59,    10,    61,   257,     3,   259,   341,    66,     4,     3,
      52,     5,     4,   174,    10,   458,   226,    11,   228,    40,
      41,    42,     8,     4,     4,    46,   226,     3,   228,    10,
      10,   199,    62,   150,    64,    65,   320,   321,   310,   186,
     324,   325,   159,   211,     4,     4,    54,    55,    56,     4,
      10,    10,    52,    53,    54,    10,   310,   174,    58,    59,
      60,    61,    62,    67,    64,    65,    66,    52,    71,    72,
     200,     5,   202,    58,    59,     9,    61,     4,     5,     3,
     258,    66,     9,   261,    67,     4,     5,   371,    71,    72,
       9,   404,     3,   203,   204,     4,     4,     5,     4,   309,
     328,     9,   403,     4,     4,     8,   378,     8,     8,   309,
     278,   279,   280,   281,   282,     5,     6,     7,     5,   391,
     330,   401,     9,   402,    64,    65,   373,   374,   375,   376,
     330,    67,     3,     4,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    43,    44,    45,    46,    47,
     422,     4,   424,   331,   332,    54,    55,    56,    55,    56,
       3,     4,    25,    26,   436,    41,    55,    56,    29,    30,
       3,   183,   184,     3,    11,     4,     4,     3,     5,     4,
     427,     3,     3,   393,     4,     4,     4,     4,     4,     4,
     320,     4,     4,   393,   324,   403,   404,   307,   404,   349,
     350,   351,   352,   413,   476,   415,     4,   479,     4,   377,
       4,     4,     4,   413,     4,   415,   394,     3,    42,   349,
     350,   351,   352,   470,    13,    14,    15,    10,    17,    60,
      12,    20,    21,    22,    23,    24,     3,    43,    27,    28,
       4,     4,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,     4,     4,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    10,    10,    54,    55,    56,    57,     4,
      13,    14,    15,     4,    17,    64,    65,    20,    21,    22,
      23,    24,    16,    16,    27,    28,     3,     3,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     3,    10,
      43,    44,    45,    46,    47,    48,    49,    50,    51,     4,
       4,     3,    16,     4,     4,     4,     4,     4,     4,     4,
      63,    13,    14,    15,    25,    17,    10,    26,    20,    21,
      22,    23,    24,     3,     3,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     4,
       3,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      13,    14,    15,     4,    17,     4,     3,    20,    21,    22,
      23,    24,     4,     4,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     4,     3,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    13,
      14,    15,     4,    17,     4,     4,    20,    21,    22,    23,
      24,     4,     4,    27,    28,    16,    56,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    56,     3,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    13,    14,
      15,     3,    17,     4,     3,    20,    21,    22,    23,    24,
      19,    19,    27,    28,     4,     4,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    62,     3,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    13,    14,    15,
     155,    17,   433,   362,    20,    21,    22,    23,    24,   114,
     395,    27,    28,   221,   396,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,   341,     3,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    13,    14,    15,   381,
      17,   344,   331,    20,    21,    22,    23,    24,   163,   340,
      27,    28,   261,   179,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,   430,     3,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    13,    14,    15,   140,    17,
     124,   235,    20,    21,    22,    23,    24,   366,   159,    27,
      28,   174,   150,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    76,     3,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    13,    14,    15,    22,    17,   188,
      -1,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    13,    14,
      15,    -1,    17,    -1,    -1,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    -1,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    57,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    13,    14,    15,    57,    17,    18,
      -1,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    13,    14,    15,    56,    17,    -1,
      -1,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    13,    14,    15,    56,    17,    18,
      -1,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    52,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    -1,
      -1,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   128,   129,   130,   141,   142,   144,   145,   147,
     150,   153,   155,   156,   157,   159,   160,   161,   163,   164,
     165,   166,   167,   170,   171,    52,    53,    54,    58,    59,
      60,    61,    62,    64,    65,    66,    67,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     3,   157,
       3,   166,     0,     9,   100,   101,     5,     9,    96,    98,
       3,    89,   101,   100,   100,     3,    98,   114,   140,   100,
      98,   140,     8,    85,    85,   100,   100,    85,     9,   162,
     162,     3,   163,   163,   163,     3,   164,   164,   164,     3,
     163,   164,   164,     3,    13,    14,    15,    17,    20,    21,
      22,    23,    24,    27,    28,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    95,   105,   106,   107,   114,   122,
     123,   124,   125,   131,   135,   136,   137,   138,   139,   151,
     154,     4,    52,     4,    89,     3,    10,    88,   148,   151,
     154,     3,    11,    87,    92,    94,    96,   143,   151,   154,
      15,    17,    20,    63,   106,   115,   124,   140,    99,     3,
      93,    94,   146,   151,   154,   140,     8,    83,    84,    85,
       3,   152,    86,    68,    69,   158,   159,   162,    85,    85,
      85,    85,    85,   169,     4,     4,    85,    85,    85,    54,
      55,    56,    57,    64,    65,   101,   102,   102,   102,    98,
      98,    99,    98,    98,    98,    98,    98,    98,    98,    41,
     103,   103,     5,     6,     7,    97,     3,   135,     3,   125,
      98,    98,   102,     4,     3,    95,   132,   133,   134,   131,
       3,    90,    91,     4,    12,   125,   127,     4,     3,    88,
     148,     3,    87,    96,     4,    92,   143,   102,   102,   102,
     127,   114,   126,     4,   102,    99,     4,    98,    62,     4,
      93,   146,    84,     8,     4,     3,   128,   149,    52,    58,
      59,    61,    66,     4,     4,    10,    83,    83,     4,     4,
     169,     4,     4,     4,     4,     3,     4,   168,     4,     4,
       4,    98,    86,   101,    86,    86,   101,    85,    85,     3,
     108,   109,   125,   109,   109,    98,    42,   104,   104,   109,
      55,    56,     3,   132,    55,    56,     4,    10,    60,   109,
       3,   108,   114,   120,   121,   109,     4,   126,    29,    30,
     105,   108,   116,   117,   118,   119,     4,    84,     4,    52,
      58,    59,    61,     4,    98,    98,    98,    98,    98,     4,
       4,     4,    43,     4,     4,    10,     4,     4,    10,    85,
       4,    56,   109,    16,    16,    19,    16,    25,    26,   110,
     111,   112,   113,    86,   101,    86,    86,   101,    86,     4,
      99,    18,   120,     3,   114,     3,     3,   117,   116,   119,
       4,   100,   100,   100,   100,     4,     4,     4,     4,     4,
      97,   136,     4,     3,   137,     3,   139,     4,     4,    86,
     102,   102,   102,   102,    98,   125,   113,    16,     4,    10,
       4,     4,    10,     4,     4,   125,    18,   114,   110,   111,
       3,    90,    95,    88,    92,    93,     4,   136,   139,     4,
     125,   125,   102,   133,     4,     3,   134,    90,     4,     3,
      91,     4,   125,     4,     4,     4,     4,     4,     4,     4,
      16,   133,    90,     3,     4,   102,    19,     3,   125,    19,
       4,   125,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    82,    83,    83,    84,    84,    85,    86,    86,    87,
      88,    88,    89,    90,    90,    90,    91,    91,    92,    93,
      94,    94,    95,    96,    97,    97,    97,    98,    98,    99,
      99,   100,   100,   101,   102,   102,   103,   103,   104,   104,
     105,   105,   105,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   107,
     107,   107,   107,   107,   108,   109,   109,   110,   111,   112,
     112,   113,   113,   114,   115,   115,   115,   115,   115,   116,
     116,   117,   117,   118,   118,   119,   119,   120,   120,   121,
     121,   121,   121,   121,   121,   122,   123,   124,   125,   125,
     126,   126,   127,   128,   129,   130,   131,   131,   131,   131,
     131,   132,   133,   133,   133,   134,   134,   135,   136,   136,
     136,   137,   137,   138,   139,   139,   139,   140,   140,   141,
     141,   142,   143,   143,   143,   143,   144,   144,   145,   146,
     146,   146,   146,   147,   148,   148,   148,   149,   149,   149,
     149,   149,   149,   150,   151,   152,   152,   152,   152,   152,
     153,   154,   155,   155,   156,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   158,   158,   159,   159,
     160,   161,   162,   162,   163,   163,   163,   164,   164,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   166,   166,
     166,   166,   167,   167,   168,   169,   169,   170,   170,   170,
     171
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     0,     2,     1,
       1,     4,     4,     1,     5,     6,     0,     5,     2,     1,
       1,     2,     4,     1,     1,     1,     1,     1,     1,     0,
       2,     0,     1,     1,     0,     1,     0,     1,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       1,     2,     2,     2,     2,     2,     2,     2,     3,     3,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     5,
       5,     5,     8,     6,     4,     2,     1,     3,     2,     1,
       1,     1,     2,     3,     2,     3,     3,     3,     3,     2,
       1,     1,     2,     4,     4,     1,     2,     2,     1,     8,
       4,     9,     5,     3,     2,     1,     1,     1,     0,     2,
       0,     2,     1,     5,     1,     5,     2,     1,     3,     2,
       2,     1,     1,     5,     6,     0,     5,     1,     1,     5,
       6,     1,     5,     1,     1,     5,     6,     4,     1,     6,
       5,     5,     1,     2,     2,     5,     6,     5,     5,     1,
       2,     2,     4,     5,     2,     2,     2,     5,     5,     5,
       5,     5,     1,     6,     5,     4,     4,     4,     4,     4,
       5,     4,     4,     5,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     0,     1,     5,     6,     6,     6,     5,     5,
       5,     5,     5,     5,     4,     4,     5,     5,     1,     1,
       1,     5,     1,     2,     4,     0,     2,     0,     1,     1,
       1
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
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1749 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1755 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1761 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1767 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1773 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 41: /* OFFSET_EQ_NAT  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1779 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 42: /* ALIGN_EQ_NAT  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1785 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 83: /* text_list  */
#line 248 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1791 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* text_list_opt  */
#line 248 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1797 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 85: /* quoted_text  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1803 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* value_type_list  */
#line 249 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1809 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* global_type  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1815 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* func_type  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1821 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 90: /* func_sig  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1827 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* func_sig_result  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1833 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* memory_sig  */
#line 244 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1839 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* type_use  */
#line 250 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1845 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* literal  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1851 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* var  */
#line 250 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1857 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* var_list  */
#line 251 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1863 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* bind_var_opt  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1869 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* bind_var  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1875 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* labeling_opt  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1881 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* instr  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1887 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* plain_instr  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1893 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* block_instr  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* block_sig  */
#line 249 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1905 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* block  */
#line 232 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1911 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* plain_catch  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1917 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* plain_catch_all  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1923 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* catch_instr  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1929 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* catch_instr_list  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1935 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* expr  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1941 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 115: /* expr1  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1947 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 116: /* try_  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 117: /* try_instr_list  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1959 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* catch_sexp  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1965 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* catch_sexp_list  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1971 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* if_block  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 121: /* if_  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1983 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* instr_list  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1989 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* expr_list  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* const_expr  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 2001 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* func  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2007 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* func_fields  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2013 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* func_fields_import  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* func_fields_import1  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2025 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* func_fields_import_result  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2031 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* func_fields_body  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2037 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* func_fields_body1  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2043 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 137: /* func_result_body  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2049 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* func_body  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2055 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* func_body1  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2061 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 140: /* offset  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 2067 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* table  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2073 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 143: /* table_fields  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2079 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* memory  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2085 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 146: /* memory_fields  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2091 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 147: /* global  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2097 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 148: /* global_fields  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2103 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 149: /* import_desc  */
#line 243 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2109 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 151: /* inline_import  */
#line 243 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2115 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 152: /* export_desc  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2121 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 154: /* inline_export  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2127 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 157: /* module_field  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2133 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 158: /* module_fields_opt  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2139 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 159: /* module_fields  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2145 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 160: /* module  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2151 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 161: /* inline_module  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2157 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 162: /* script_var_opt  */
#line 250 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 2163 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 163: /* script_module  */
#line 246 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script_module); }
#line 2169 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 164: /* action  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 2175 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 165: /* assertion  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2181 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 166: /* cmd  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2187 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 167: /* cmd_list  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2193 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 169: /* const_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2199 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 170: /* script  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2205 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
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
#line 264 "src/wast-parser.y" /* yacc.c:1646  */
    {
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 2504 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 270 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 2517 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 280 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2523 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 285 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2541 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 303 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2547 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 304 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].type));
    }
#line 2556 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 310 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2562 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 313 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].type);
      (yyval.global)->mutable_ = false;
    }
#line 2572 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 318 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].type);
      (yyval.global)->mutable_ = true;
    }
#line 2582 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 326 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2588 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 331 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2598 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 336 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].type));
      // Ignore bind_var.
      destroy_string_slice(&(yyvsp[-3].text));
    }
#line 2609 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 345 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2615 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 346 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->result_types.insert((yyval.func_sig)->result_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2625 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 354 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
#line 2634 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 360 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
#line 2643 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 366 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2653 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 371 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2663 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 378 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2669 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 384 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid int " PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2682 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 395 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2691 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 399 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2700 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 403 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2709 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 410 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var((yyvsp[0].u64));
      (yyval.var)->loc = (yylsp[0]);
    }
#line 2718 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 414 "src/wast-parser.y" /* yacc.c:1646  */
    {
      StringSlice name;
      DUPTEXT(name, (yyvsp[0].text));
      (yyval.var) = new Var(name);
      (yyval.var)->loc = (yylsp[0]);
    }
#line 2729 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 422 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2735 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 423 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->emplace_back(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2745 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 430 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2751 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 434 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2757 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 34:
#line 438 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2763 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 443 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2769 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 444 "src/wast-parser.y" /* yacc.c:1646  */
    {
      uint64_t offset64;
      if (WABT_FAILED(parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &offset64,
                                  ParseIntType::SignedAndUnsigned))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid offset \"" PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
      if (offset64 > UINT32_MAX) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "offset must be less than or equal to 0xffffffff");
      }
      (yyval.u64) = static_cast<uint32_t>(offset64);
    }
#line 2788 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 460 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 461 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                  ParseIntType::UnsignedOnly))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid alignment \"" PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }

      if ((yyval.u32) != WABT_USE_NATURAL_ALIGNMENT && !is_power_of_two((yyval.u32))) {
        wast_parser_error(&(yylsp[0]), lexer, parser, "alignment must be power-of-two");
      }
    }
#line 2811 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 476 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2817 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 477 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2823 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 482 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2831 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 485 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2839 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 488 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2847 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 491 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2855 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 494 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2864 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 498 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2873 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 502 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2882 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 506 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2890 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 509 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 513 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2908 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 517 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2917 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 521 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2926 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 525 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2935 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 529 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2944 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 533 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 537 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2961 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 540 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2969 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 543 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2987 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 556 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 559 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 3003 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 562 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 3011 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 565 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 3019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 568 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 3027 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 571 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 3035 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 574 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateThrow(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 3044 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 578 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateRethrow(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 3053 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 585 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBlock((yyvsp[-2].block));
      (yyval.expr)->block->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block->label, (yyvsp[0].text));
    }
#line 3063 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 590 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoop((yyvsp[-2].block));
      (yyval.expr)->loop->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->loop->label, (yyvsp[0].text));
    }
#line 3073 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 595 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-2].block), nullptr);
      (yyval.expr)->if_.true_->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3083 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 600 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-5].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->if_.true_->label = (yyvsp[-6].text);
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->if_.true_->label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 606 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-3].block)->label = (yyvsp[-4].text);
      (yyval.expr) = Expr::CreateTry((yyvsp[-3].block), (yyvsp[-2].expr_list).first);
      CHECK_END_LABEL((yylsp[0]), (yyvsp[-3].block)->label, (yyvsp[0].text));
    }
#line 3104 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 614 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 3110 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 617 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = (yyvsp[0].block);
      (yyval.block)->sig.insert((yyval.block)->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3120 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 622 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = new Block();
      (yyval.block)->first = (yyvsp[0].expr_list).first;
    }
#line 3129 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 629 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatch(std::move(*(yyvsp[-1].var)), (yyvsp[0].expr_list).first);
      delete (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3139 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 636 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatchAll((yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 3148 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 649 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-1].expr_list), &(yyvsp[0].expr_list));
    }
#line 3156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 655 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 3162 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 659 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 3170 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 662 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateBlock((yyvsp[0].block));
      expr->block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3180 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 667 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateLoop((yyvsp[0].block));
      expr->loop->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3190 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 672 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      if_->if_.true_->label = (yyvsp[-1].text);
    }
#line 3201 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 678 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Block* block = (yyvsp[0].expr)->try_block.block;
      block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), (yyvsp[0].expr));
    }
#line 3211 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 686 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = (yyvsp[0].expr);
      Block* block = (yyval.expr)->try_block.block;
      block->sig.insert(block->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3222 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 696 "src/wast-parser.y" /* yacc.c:1646  */
    {
        Block* block = new Block();
        (yyval.expr) = Expr::CreateTry(block, (yyvsp[0].expr_list).first);
      }
#line 3231 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 700 "src/wast-parser.y" /* yacc.c:1646  */
    {
        (yyval.expr) = (yyvsp[0].expr);
        Block* block = (yyval.expr)->try_block.block;
        if ((yyvsp[-1].expr_list).last) {
          (yyvsp[-1].expr_list).last->next = block->first;
        } else {
          (yyvsp[-1].expr_list).first->next = block->first;
        }
        block->first = (yyvsp[-1].expr_list).first;
      }
#line 3246 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 713 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3254 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 716 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3262 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 723 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-1].expr_list), &(yyvsp[0].expr_list));
    }
#line 3270 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 729 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->if_.true_;
      true_->sig.insert(true_->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3283 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 740 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
#line 3292 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 744 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 3301 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 748 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
#line 3310 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 752 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
#line 3319 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 756 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
#line 3328 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 104:
#line 760 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[0].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
#line 3337 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 767 "src/wast-parser.y" /* yacc.c:1646  */
    {
     CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "rethrow");
    }
#line 3345 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 772 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "throw");
    }
#line 3353 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 778 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "try");      
    }
#line 3361 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 784 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3367 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 785 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3378 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 793 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3384 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 794 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3395 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 807 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exception) = new Exception();
      (yyval.exception)->name = (yyvsp[-2].text);
      (yyval.exception)->sig = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 3406 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 815 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Except);
      (yyval.module_field)->loc = (yylsp[0]);
      (yyval.module_field)->except = (yyvsp[0].exception);
    }
#line 3416 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 824 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3432 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 838 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3445 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 846 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3455 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 851 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-2]);
      field->import = (yyvsp[-2].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      field->import->func->decl.has_func_type = true;
      field->import->func->decl.type_var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3471 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 862 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3484 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 120:
#line 870 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3497 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 881 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3506 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 889 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3517 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 895 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3529 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 905 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3535 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 906 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3546 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 915 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3555 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 923 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3566 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 130:
#line 929 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3578 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 940 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3589 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 949 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
#line 3598 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 956 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->first_expr = (yyvsp[0].expr_list).first;
    }
#line 3607 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 960 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3617 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 965 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].type));
    }
#line 3629 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 977 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3637 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 984 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::ElemSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->elem_segment = new ElemSegment();
      (yyval.module_field)->elem_segment->table_var = std::move(*(yyvsp[-3].var));
      delete (yyvsp[-3].var);
      (yyval.module_field)->elem_segment->offset = (yyvsp[-2].expr_list).first;
      (yyval.module_field)->elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
    }
#line 3652 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 994 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3668 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1008 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3684 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1022 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Table);
      field->loc = (yylsp[0]);
      field->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3695 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1028 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3708 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1036 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3721 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1044 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* table_field = new ModuleField(ModuleFieldType::Table);
      Table* table = table_field->table = new Table();
      table->elem_limits.initial = (yyvsp[-1].vars)->size();
      table->elem_limits.max = (yyvsp[-1].vars)->size();
      table->elem_limits.has_max = true;
      ModuleField* elem_field = new ModuleField(ModuleFieldType::ElemSegment);
      elem_field->loc = (yylsp[-2]);
      ElemSegment* elem_segment = elem_field->elem_segment = new ElemSegment();
      elem_segment->table_var = Var(kInvalidIndex);
      elem_segment->offset = Expr::CreateConst(Const(Const::I32(), 0));
      elem_segment->offset->loc = (yylsp[-2]);
      elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
      (yyval.module_fields).first = table_field;
      (yyval.module_fields).last = table_field->next = elem_field;
    }
#line 3743 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1064 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::DataSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->data_segment = new DataSegment();
      (yyval.module_field)->data_segment->memory_var = std::move(*(yyvsp[-3].var));
      delete (yyvsp[-3].var);
      (yyval.module_field)->data_segment->offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.module_field)->data_segment->data, &(yyval.module_field)->data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3758 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1074 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3774 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1088 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3790 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1102 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Memory);
      field->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3800 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1107 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3813 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1115 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3826 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1123 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* data_field = new ModuleField(ModuleFieldType::DataSegment);
      data_field->loc = (yylsp[-2]);
      DataSegment* data_segment = data_field->data_segment = new DataSegment();
      data_segment->memory_var = Var(kInvalidIndex);
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
#line 3852 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1147 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3868 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1161 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Global);
      field->global = (yyvsp[-1].global);
      field->global->init_expr = (yyvsp[0].expr_list).first;
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3879 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1167 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1175 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3905 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1188 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3919 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1197 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3932 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1205 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = (yyvsp[-2].text);
    }
#line 3943 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1211 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = (yyvsp[-2].text);
    }
#line 3954 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1217 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = (yyvsp[-2].text);
    }
#line 3965 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1223 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Except;
      (yyval.import)->except = (yyvsp[0].exception);
    }
#line 3975 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1231 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Import);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->import = (yyvsp[-1].import);
      (yyval.module_field)->import->module_name = (yyvsp[-3].text);
      (yyval.module_field)->import->field_name = (yyvsp[-2].text);
    }
#line 3987 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1241 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
#line 3997 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1249 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4008 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1255 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1261 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4030 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1267 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4041 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1273 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Except;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4052 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1281 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Export);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->export_ = (yyvsp[-1].export_);
      (yyval.module_field)->export_->name = (yyvsp[-2].text);
    }
#line 4063 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 171:
#line 1290 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = (yyvsp[-1].text);
    }
#line 4072 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1300 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 4084 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1307 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->name = (yyvsp[-2].text);
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 4097 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1318 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Start);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->start = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4108 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1327 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4114 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1332 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4120 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1333 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4126 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1334 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4132 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 183:
#line 1335 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4138 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1336 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4144 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1337 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4150 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1341 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 4156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1346 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new Module();
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4166 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1351 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4176 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1359 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].script_module)->type == ScriptModule::Type::Text) {
        (yyval.module) = (yyvsp[0].script_module)->text;
        (yyvsp[0].script_module)->text = nullptr;
      } else {
        assert((yyvsp[0].script_module)->type == ScriptModule::Type::Binary);
        (yyval.module) = new Module();
        ReadBinaryOptions options = WABT_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorHandlerModule error_handler(&(yyvsp[0].script_module)->binary.loc, lexer, parser);
        read_binary_ir((yyvsp[0].script_module)->binary.data, (yyvsp[0].script_module)->binary.size, &options,
                       &error_handler, (yyval.module));
        (yyval.module)->name = (yyvsp[0].script_module)->binary.name;
        (yyval.module)->loc = (yyvsp[0].script_module)->binary.loc;
        WABT_ZERO_MEMORY((yyvsp[0].script_module)->binary.name);
      }
      delete (yyvsp[0].script_module);
    }
#line 4198 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 192:
#line 1386 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var(kInvalidIndex);
    }
#line 4206 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 193:
#line 1389 "src/wast-parser.y" /* yacc.c:1646  */
    {
      StringSlice name;
      DUPTEXT(name, (yyvsp[0].text));
      (yyval.var) = new Var(name);
    }
#line 4216 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 194:
#line 1397 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Text;
      (yyval.script_module)->text = (yyvsp[-1].module);
      (yyval.script_module)->text->name = (yyvsp[-2].text);
      (yyval.script_module)->text->loc = (yylsp[-3]);

      // Resolve func type variables where the signature was not specified
      // explicitly.
      for (Func* func: (yyvsp[-1].module)->funcs) {
        if (func->decl.has_func_type && is_empty_signature(&func->decl.sig)) {
          FuncType* func_type = (yyvsp[-1].module)->GetFuncType(func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
          }
        }
      }
    }
#line 4239 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 195:
#line 1415 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Binary;
      (yyval.script_module)->binary.name = (yyvsp[-3].text);
      (yyval.script_module)->binary.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->binary.data, &(yyval.script_module)->binary.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4252 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 196:
#line 1423 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Quoted;
      (yyval.script_module)->quoted.name = (yyvsp[-3].text);
      (yyval.script_module)->quoted.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->quoted.data, &(yyval.script_module)->quoted.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4265 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 197:
#line 1434 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-4]);
      (yyval.action)->module_var = std::move(*(yyvsp[-3].var));
      delete (yyvsp[-3].var);
      (yyval.action)->type = ActionType::Invoke;
      (yyval.action)->name = (yyvsp[-2].text);
      (yyval.action)->invoke = new ActionInvoke();
      (yyval.action)->invoke->args = std::move(*(yyvsp[-1].consts));
      delete (yyvsp[-1].consts);
    }
#line 4281 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 198:
#line 1445 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = std::move(*(yyvsp[-2].var));
      delete (yyvsp[-2].var);
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = (yyvsp[-1].text);
    }
#line 4294 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 199:
#line 1456 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertMalformed;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
#line 4305 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 200:
#line 1462 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertInvalid;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 4316 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 201:
#line 1468 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUnlinkable;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
#line 4327 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 202:
#line 1474 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUninstantiable;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
#line 4338 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 203:
#line 1480 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturn;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
#line 4349 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 204:
#line 1486 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnCanonicalNan;
      (yyval.command)->assert_return_canonical_nan.action = (yyvsp[-1].action);
    }
#line 4359 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 205:
#line 1491 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnArithmeticNan;
      (yyval.command)->assert_return_arithmetic_nan.action = (yyvsp[-1].action);
    }
#line 4369 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 206:
#line 1496 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertTrap;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4380 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 207:
#line 1502 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertExhaustion;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4391 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 208:
#line 1511 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Action;
      (yyval.command)->action = (yyvsp[0].action);
    }
#line 4401 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 210:
#line 1517 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Module;
      (yyval.command)->module = (yyvsp[0].module);
    }
#line 4411 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 211:
#line 1522 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Register;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
#line 4424 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 212:
#line 1532 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4433 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 213:
#line 1536 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4442 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 214:
#line 1543 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4457 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 215:
#line 1555 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4463 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 216:
#line 1556 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
#line 4472 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 217:
#line 1563 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4480 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 218:
#line 1566 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4545 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 219:
#line 1626 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      Command* command = new Command();
      command->type = CommandType::Module;
      command->module = (yyvsp[0].module);
      (yyval.script)->commands.emplace_back(command);
    }
#line 4557 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 220:
#line 1638 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4563 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4567 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1641 "src/wast-parser.y" /* yacc.c:1906  */


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

ExprList join_expr_lists(ExprList* expr1, ExprList* expr2) {
  ExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  return result;
}

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
  if (decl->has_func_type)
    return;

  int sig_index = module->GetFuncTypeIndex(*decl);
  if (sig_index == -1) {
    module->AppendImplicitFuncType(*loc, decl->sig);
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
          module->globals.size() != module->num_global_imports ||
          module->excepts.size() != module->num_except_imports) {
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
          case ExternalKind::Except:
            name = &field->import->except->name;
            bindings = &module->except_bindings;
            index = module->excepts.size();
            module->excepts.push_back(field->except);
            ++module->num_except_imports;
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

      case ModuleFieldType::Except:
        name = &field->except->name;        
        bindings = &module->except_bindings;
        index = module->excepts.size();
        module->excepts.push_back(field->except);
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
  if (out_script) {
    *out_script = parser.script;
  } else {
    delete parser.script;
  }
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
