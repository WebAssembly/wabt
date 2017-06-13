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
    WABT_TOKEN_TYPE_TABLE = 312,
    WABT_TOKEN_TYPE_ELEM = 313,
    WABT_TOKEN_TYPE_MEMORY = 314,
    WABT_TOKEN_TYPE_DATA = 315,
    WABT_TOKEN_TYPE_OFFSET = 316,
    WABT_TOKEN_TYPE_IMPORT = 317,
    WABT_TOKEN_TYPE_EXPORT = 318,
    WABT_TOKEN_TYPE_EXCEPT = 319,
    WABT_TOKEN_TYPE_MODULE = 320,
    WABT_TOKEN_TYPE_BIN = 321,
    WABT_TOKEN_TYPE_QUOTE = 322,
    WABT_TOKEN_TYPE_REGISTER = 323,
    WABT_TOKEN_TYPE_INVOKE = 324,
    WABT_TOKEN_TYPE_GET = 325,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 326,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 327,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 328,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 329,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 330,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 331,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 332,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 333,
    WABT_TOKEN_TYPE_LOW = 334
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

#line 367 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */

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
#define YYLAST   987

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  212
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  473

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   334

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
      75,    76,    77,    78,    79
};

#if WABT_WAST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   261,   261,   267,   277,   278,   282,   300,   301,   307,
     310,   315,   323,   327,   328,   333,   342,   343,   351,   357,
     363,   368,   375,   381,   392,   396,   400,   407,   411,   419,
     420,   427,   428,   431,   435,   436,   440,   441,   457,   458,
     473,   474,   475,   479,   482,   485,   488,   491,   495,   499,
     503,   506,   510,   514,   518,   522,   526,   530,   534,   537,
     540,   553,   556,   559,   562,   565,   568,   571,   575,   582,
     587,   592,   597,   603,   612,   615,   620,   627,   632,   639,
     640,   646,   650,   653,   658,   663,   669,   677,   680,   686,
     694,   697,   701,   705,   709,   713,   717,   724,   729,   735,
     741,   742,   750,   751,   759,   764,   772,   781,   795,   803,
     808,   819,   827,   838,   845,   846,   852,   862,   863,   872,
     879,   880,   886,   896,   897,   906,   913,   917,   922,   934,
     937,   941,   951,   965,   979,   985,   993,  1001,  1021,  1031,
    1045,  1059,  1064,  1072,  1080,  1104,  1118,  1124,  1132,  1145,
    1154,  1162,  1168,  1174,  1180,  1188,  1198,  1206,  1212,  1218,
    1224,  1230,  1238,  1247,  1257,  1264,  1275,  1284,  1285,  1286,
    1287,  1288,  1289,  1290,  1291,  1292,  1293,  1294,  1298,  1299,
    1303,  1308,  1316,  1336,  1343,  1346,  1354,  1372,  1380,  1391,
    1402,  1413,  1419,  1425,  1431,  1437,  1443,  1448,  1453,  1459,
    1468,  1473,  1474,  1479,  1489,  1493,  1500,  1512,  1513,  1520,
    1523,  1583,  1595
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
  "TRY", "CATCH", "CATCH_ALL", "THROW", "RETHROW", "CALL", "CALL_INDIRECT",
  "RETURN", "GET_LOCAL", "SET_LOCAL", "TEE_LOCAL", "GET_GLOBAL",
  "SET_GLOBAL", "LOAD", "STORE", "OFFSET_EQ_NAT", "ALIGN_EQ_NAT", "CONST",
  "UNARY", "BINARY", "COMPARE", "CONVERT", "SELECT", "UNREACHABLE",
  "CURRENT_MEMORY", "GROW_MEMORY", "FUNC", "START", "TYPE", "PARAM",
  "RESULT", "LOCAL", "GLOBAL", "TABLE", "ELEM", "MEMORY", "DATA", "OFFSET",
  "IMPORT", "EXPORT", "EXCEPT", "MODULE", "BIN", "QUOTE", "REGISTER",
  "INVOKE", "GET", "ASSERT_MALFORMED", "ASSERT_INVALID",
  "ASSERT_UNLINKABLE", "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
  "ASSERT_RETURN_ARITHMETIC_NAN", "ASSERT_TRAP", "ASSERT_EXHAUSTION",
  "LOW", "$accept", "text_list", "text_list_opt", "quoted_text",
  "value_type_list", "elem_type", "global_type", "func_type", "func_sig",
  "func_sig_result", "table_sig", "memory_sig", "limits", "type_use",
  "nat", "literal", "var", "var_list", "bind_var_opt", "bind_var",
  "labeling_opt", "offset_opt", "align_opt", "instr", "plain_instr",
  "block_instr", "block_sig", "block", "catch_instr", "catch_instr_list",
  "expr", "expr1", "catch_list", "if_block", "if_", "rethrow_check",
  "throw_check", "try_check", "instr_list", "expr_list", "const_expr",
  "exception", "exception_field", "func", "func_fields",
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334
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
      57,   909,  -375,  -375,  -375,  -375,  -375,  -375,  -375,  -375,
    -375,  -375,  -375,  -375,  -375,    71,  -375,  -375,  -375,  -375,
    -375,  -375,    90,  -375,   123,   126,   116,   113,   126,   126,
     175,   126,   175,   134,   134,   126,   126,   134,   145,   145,
     160,   160,   160,   179,   179,   179,   194,   179,   153,  -375,
     215,  -375,  -375,  -375,   448,  -375,  -375,  -375,  -375,   225,
     184,   235,   251,    30,    43,   399,   265,  -375,  -375,    34,
     265,   267,  -375,   134,   291,  -375,    29,   145,  -375,   134,
     134,   247,   134,   134,   134,    39,  -375,   275,   302,    87,
     134,   134,   134,   355,  -375,  -375,   126,   126,   126,   116,
     116,  -375,  -375,  -375,  -375,   116,   116,  -375,   116,   116,
     116,   116,   116,   274,   274,   242,  -375,  -375,  -375,  -375,
    -375,  -375,  -375,  -375,   485,   522,  -375,  -375,  -375,   116,
     116,   126,  -375,   310,  -375,  -375,  -375,  -375,  -375,   312,
     448,  -375,   313,  -375,   314,     8,  -375,   522,   316,    95,
      30,    70,  -375,   318,  -375,   311,   319,   321,   319,    43,
     126,   126,   126,   522,   320,   322,   126,  -375,   140,   214,
    -375,  -375,   325,   319,    34,   267,  -375,   326,   329,   327,
      84,   336,   171,   267,   267,   337,    71,   340,  -375,   344,
     345,   346,   348,   170,  -375,  -375,   351,   352,   356,   116,
     126,  -375,   126,   134,   134,  -375,   559,   559,   559,  -375,
    -375,   116,  -375,  -375,  -375,  -375,  -375,  -375,  -375,  -375,
     331,   331,  -375,  -375,  -375,  -375,   670,  -375,   908,  -375,
    -375,  -375,   559,  -375,   243,   358,  -375,  -375,  -375,  -375,
     165,   369,  -375,  -375,   364,  -375,  -375,  -375,   350,  -375,
    -375,   323,  -375,  -375,  -375,  -375,  -375,   559,   377,   559,
     390,   320,  -375,  -375,   392,   222,  -375,  -375,   267,  -375,
    -375,  -375,   401,  -375,  -375,   108,  -375,   402,   116,   116,
     116,   116,   116,  -375,  -375,  -375,   217,   224,  -375,  -375,
     241,  -375,  -375,  -375,  -375,   370,  -375,  -375,  -375,  -375,
    -375,   411,   181,   414,   183,   185,   415,   134,   434,   833,
     559,   423,  -375,    36,   433,   231,  -375,  -375,  -375,   276,
     126,  -375,   255,  -375,   126,  -375,  -375,   446,  -375,  -375,
     795,   377,   449,  -375,  -375,  -375,  -375,  -375,   438,  -375,
     450,  -375,   126,   126,   126,   126,  -375,   451,   452,   453,
     454,   455,  -375,  -375,  -375,   242,  -375,   485,   460,   596,
     633,   462,   463,  -375,  -375,  -375,   126,   126,   126,   126,
     116,   522,   276,   457,   188,   464,   190,   192,   477,   197,
    -375,   233,   522,  -375,   871,   320,   559,  -375,   498,    95,
     319,   319,  -375,  -375,  -375,  -375,  -375,   499,  -375,   485,
     713,  -375,   756,  -375,   633,  -375,   210,  -375,  -375,   522,
    -375,   522,  -375,  -375,   126,   358,   500,   507,   313,   520,
     508,  -375,   534,   522,  -375,   536,   246,   537,   543,   544,
     557,   571,  -375,  -375,  -375,  -375,   561,  -375,  -375,  -375,
     358,   524,  -375,  -375,   313,   530,  -375,   582,   594,   609,
    -375,  -375,  -375,  -375,  -375,   126,  -375,  -375,   595,   612,
     276,  -375,  -375,   522,   602,   618,   631,   522,   609,  -375,
     645,  -375,  -375
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     209,     0,   106,   177,   171,   172,   169,   173,   170,   168,
     175,   176,   167,   174,   180,   183,   202,   211,   182,   200,
     201,   204,   210,   212,     0,    31,     0,     0,    31,    31,
       0,    31,     0,     0,     0,    31,    31,     0,   184,   184,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   181,
       0,   205,     1,    33,   100,    32,    23,    28,    27,     0,
       0,     0,     0,     0,     0,     0,     0,   130,    29,     0,
       0,     4,     6,     0,     0,     7,   178,   184,   185,     0,
       0,     0,     0,     0,     0,     0,   207,     0,     0,     0,
       0,     0,     0,     0,    44,    45,    34,    34,    34,     0,
       0,    29,    99,    98,    97,     0,     0,    50,     0,     0,
       0,     0,     0,    36,    36,     0,    61,    62,    63,    64,
      46,    43,    65,    66,   100,   100,    40,    41,    42,     0,
       0,    34,   126,     0,   109,   119,   120,   123,   125,   117,
     100,   166,    16,   164,     0,     0,    10,   100,     0,     0,
       0,     0,     9,     0,   134,     0,    20,     0,     0,     0,
      34,    34,    34,   100,   102,     0,    34,    29,     0,     0,
     141,    19,     0,     0,     0,     4,     2,     5,     0,     0,
       0,     0,     0,     0,     0,     0,   179,     0,   207,     0,
       0,     0,     0,     0,   196,   197,     0,     0,     0,     0,
       7,     7,     7,     0,     0,    35,   100,   100,   100,    47,
      48,     0,    51,    52,    53,    54,    55,    56,    57,    37,
      38,    38,    24,    25,    26,    60,     0,   108,     0,   101,
      68,    67,   100,   107,     0,   117,   111,   113,   114,   112,
       0,     0,    13,   165,     0,   104,   146,   145,     0,   147,
     148,     0,    18,    21,   133,   135,   136,   100,     0,   100,
       0,   102,    82,    81,     0,     0,   132,    30,     4,   140,
     142,   143,     0,     3,   139,     0,   154,     0,     0,     0,
       0,     0,     0,   162,   105,     8,     0,     0,   186,   203,
       0,   190,   191,   192,   193,     0,   195,   208,   194,   198,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     100,     0,    76,     0,     0,    49,    39,    58,    59,     0,
       7,     7,     0,   110,     7,     7,    12,     0,    29,    83,
       0,     0,     0,    85,    90,    84,   129,   103,     0,   131,
       0,   138,    31,    31,    31,    31,   155,     0,     0,     0,
       0,     0,   187,   188,   189,     0,    22,   100,     0,   100,
     100,     0,     0,   163,     7,    75,    34,    34,    34,    34,
       0,   100,    79,     0,     0,     0,     0,     0,     0,     0,
      11,     0,   100,    89,     0,    96,   100,   144,    16,     0,
       0,     0,   157,   160,   158,   159,   161,     0,   121,   100,
       0,   124,     0,   127,   100,   156,     0,    69,    71,   100,
      70,   100,    78,    80,    34,   117,     0,   117,    16,     0,
      16,   137,     0,   100,    95,     0,     0,     0,     0,     0,
       0,     0,   206,   122,   128,    74,     0,    77,    73,   115,
     117,     0,   118,    14,    16,     0,    17,    92,     0,     0,
     150,   149,   153,   151,   152,    34,   116,    15,     0,    94,
       0,    86,    72,   100,     0,     0,     0,   100,    87,    91,
       0,    88,    93
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -375,   127,  -144,    47,  -174,   496,  -136,   590,  -343,   238,
    -147,  -156,   -63,  -127,   -59,   304,   -23,   -87,    -6,    22,
     -97,   558,   465,  -375,   -57,  -375,  -240,  -140,   213,   317,
     -28,  -375,   220,   365,  -375,  -375,  -375,   -49,  -104,   459,
     532,   531,  -375,  -375,   569,   486,  -374,   305,   605,  -338,
     372,  -375,  -325,    24,  -375,  -375,   573,  -375,  -375,   564,
    -375,   589,  -375,  -375,     9,  -375,  -375,    49,  -375,  -375,
       0,  -375,   676,  -375,  -375,   -15,   120,   219,  -375,   731,
    -375,  -375,   575,  -375,  -375
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   177,   178,    73,   182,   153,   147,    61,   241,   242,
     154,   170,   155,   124,    58,   225,   267,   168,    54,   205,
     206,   220,   317,   125,   126,   127,   310,   311,   372,   373,
     128,   165,   461,   333,   334,   129,   130,   131,   132,   262,
     246,     2,     3,     4,   133,   236,   237,   238,   134,   135,
     136,   137,   138,    68,     5,     6,   157,     7,     8,   172,
       9,   148,   277,    10,   139,   181,    11,   140,    12,    13,
      14,   185,    15,    16,    17,    79,    18,    19,    20,    21,
      22,   297,   193,    23,    24
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     207,   208,    67,    59,    67,   156,   171,    66,   164,    70,
     156,   255,   235,   249,   211,    49,   166,   270,   331,   398,
     244,   229,    63,    64,    80,    69,   302,   304,   305,    75,
      76,   272,    48,   145,   232,   403,   164,   169,    67,    56,
     146,   439,    67,   245,   166,   427,   151,    55,    56,    62,
      55,    55,   367,    55,   152,   368,    71,    55,    55,   245,
       1,   433,   187,   257,   258,   259,   456,   313,   314,   264,
     203,   204,   149,   158,    48,   443,   209,   210,   173,   434,
     265,    74,   212,   213,    77,   214,   215,   216,   217,   218,
     167,   331,   319,    50,   175,   183,   184,   253,   248,   156,
     156,   457,   312,   312,   312,   146,   230,   231,    38,    39,
     171,   171,   150,   159,   156,   156,    60,   329,   174,   335,
     179,    56,    53,    52,   340,    57,   188,   189,   312,   190,
     191,   192,   203,   204,   278,    53,   261,   196,   197,   198,
     279,   280,    72,   281,   266,    56,   374,   376,   282,    57,
     377,   379,    36,   312,    78,   312,    38,    39,   342,   149,
      82,    83,    84,    81,   343,   344,    90,   345,   158,   164,
     365,   164,    35,   295,   296,   284,   301,   166,    65,   166,
      56,   285,    85,   173,    57,   357,    49,   359,   315,   360,
     406,   285,   415,   285,   417,   285,   418,    89,   285,   150,
     285,   420,   285,    25,    26,    27,   312,   285,   159,    28,
      29,    30,    31,    32,   435,    33,    34,    35,   324,   325,
     285,   352,   303,   174,   306,   273,   339,    56,   353,   141,
     332,    57,   273,   261,   142,   431,   -30,   421,    56,   143,
     -30,   381,    57,   430,   295,   354,   425,   222,   223,   224,
     307,   308,   164,   429,    60,   347,   348,   349,   350,   351,
     166,   428,    86,    87,    88,    91,    92,   412,    65,   407,
     408,   409,   410,   164,   268,   176,   203,   204,   422,   194,
      36,   166,   312,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   180,   199,   320,   321,   199,   324,
     325,   370,   371,   332,   385,   436,   195,   437,   320,   321,
     286,   287,    36,   219,   233,   234,   240,   438,   243,   448,
     247,   251,   152,   228,    56,   254,   263,   164,   171,   269,
     275,   156,   156,   274,   273,   166,   388,   389,   390,   391,
     283,   288,   375,   164,   289,   164,   378,   411,   291,   292,
     293,   166,   294,   166,   362,   298,   299,   424,   462,   466,
     300,   322,   244,   470,    55,    55,    55,    55,    94,    95,
     160,   316,   161,   326,   327,   162,    99,   100,   101,   102,
     330,   328,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   336,   338,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   341,   346,   199,   200,   201,
     202,   355,    94,    95,   160,   356,   161,   203,   204,   162,
      99,   100,   101,   102,   358,   361,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   363,   366,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   369,
     380,    93,   384,   386,   387,   392,   393,   394,   395,   396,
     163,    94,    95,    96,   399,    97,   404,   405,    98,    99,
     100,   101,   102,   414,   416,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   419,   226,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    94,    95,
      96,   426,    97,   432,   440,    98,    99,   100,   101,   102,
     441,   445,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   444,   228,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    94,    95,    96,   447,    97,
     449,   450,    98,    99,   100,   101,   102,   451,   452,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   453,   309,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    94,    95,    96,   454,    97,   455,   321,    98,
      99,   100,   101,   102,   325,   458,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   459,   400,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    94,
      95,    96,   460,    97,   463,   464,    98,    99,   100,   101,
     102,   467,   468,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   469,   402,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    94,    95,    96,   472,
      97,   252,   144,    98,    99,   100,   101,   102,   446,   397,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   221,   465,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    94,    95,   160,   318,   161,   471,   413,
     162,    99,   100,   101,   102,   260,   383,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   239,
     276,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     337,   323,   442,   200,   201,   202,    94,    95,   160,   227,
     161,   401,   256,   162,    99,   100,   101,   102,   271,   250,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   186,    51,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   290,     0,     0,     0,   201,   202,    94,
      95,   160,     0,   161,     0,     0,   162,    99,   100,   101,
     102,     0,     0,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,     0,   115,   116,   117,
     118,   119,   120,   121,   122,   123,     0,     0,    94,    95,
     160,   202,   161,   382,     0,   162,    99,   100,   101,   102,
       0,     0,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,     0,   115,   116,   117,   118,
     119,   120,   121,   122,   123,     0,    94,    95,   160,   364,
     161,     0,     0,   162,    99,   100,   101,   102,     0,     0,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   115,   116,   117,   118,   119,   120,
     121,   122,   123,     0,    94,    95,   160,   364,   161,   423,
       0,   162,    99,   100,   101,   102,     0,     0,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
       0,     0,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    94,    95,   160,     0,   161,     0,     0,   162,    99,
     100,   101,   102,     0,     0,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,     0,   115,
     116,   117,   118,   119,   120,   121,   122,   123,     0,    25,
      26,    27,     0,     0,     0,    28,    29,    30,    31,    32,
       0,    33,    34,    35,    36,     0,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47
};

static const yytype_int16 yycheck[] =
{
      97,    98,    30,    26,    32,    64,    69,    30,    65,    32,
      69,   158,   139,   149,   101,    15,    65,   173,   258,   357,
      12,   125,    28,    29,    39,    31,   200,   201,   202,    35,
      36,   175,     3,     3,   131,   360,    93,     3,    66,     5,
      10,   415,    70,   147,    93,   388,     3,    25,     5,    27,
      28,    29,    16,    31,    11,    19,    32,    35,    36,   163,
       3,   399,    77,   160,   161,   162,   440,   207,   208,   166,
      62,    63,    63,    64,     3,   418,    99,   100,    69,   404,
     167,    34,   105,   106,    37,   108,   109,   110,   111,   112,
      66,   331,   232,     3,    70,    66,    67,   156,     3,   158,
     159,   444,   206,   207,   208,    10,   129,   130,    69,    70,
     173,   174,    63,    64,   173,   174,     3,   257,    69,   259,
      73,     5,     9,     0,   268,     9,    79,    80,   232,    82,
      83,    84,    62,    63,    50,     9,   164,    90,    91,    92,
      56,    57,     8,    59,     4,     5,   320,   321,    64,     9,
     324,   325,    65,   257,     9,   259,    69,    70,    50,   150,
      40,    41,    42,     3,    56,    57,    46,    59,   159,   226,
     310,   228,    64,     3,     4,     4,   199,   226,     3,   228,
       5,    10,     3,   174,     9,     4,   186,     4,   211,     4,
     364,    10,     4,    10,     4,    10,     4,     3,    10,   150,
      10,     4,    10,    50,    51,    52,   310,    10,   159,    56,
      57,    58,    59,    60,     4,    62,    63,    64,    53,    54,
      10,     4,   200,   174,   202,     8,     4,     5,     4,     4,
     258,     9,     8,   261,    50,   391,     5,     4,     5,     4,
       9,   328,     9,   390,     3,     4,   386,     5,     6,     7,
     203,   204,   309,   389,     3,   278,   279,   280,   281,   282,
     309,   388,    43,    44,    45,    46,    47,   371,     3,   366,
     367,   368,   369,   330,    60,     8,    62,    63,   382,     4,
      65,   330,   386,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     3,    52,    53,    54,    52,    53,
      54,    25,    26,   331,   332,   409,     4,   411,    53,    54,
     183,   184,    65,    39,     4,     3,     3,   414,     4,   423,
       4,     3,    11,     3,     5,     4,     4,   384,   391,     4,
       3,   390,   391,     4,     8,   384,   342,   343,   344,   345,
       4,     4,   320,   400,     4,   402,   324,   370,     4,     4,
       4,   400,     4,   402,   307,     4,     4,   385,   455,   463,
       4,     3,    12,   467,   342,   343,   344,   345,    13,    14,
      15,    40,    17,     4,    10,    20,    21,    22,    23,    24,
       3,    58,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,     4,     3,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     4,     4,    52,    53,    54,
      55,    41,    13,    14,    15,     4,    17,    62,    63,    20,
      21,    22,    23,    24,    10,    10,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,     4,    16,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    16,
       4,     3,     3,    15,     4,     4,     4,     4,     4,     4,
      61,    13,    14,    15,     4,    17,     4,     4,    20,    21,
      22,    23,    24,    16,    10,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    10,     3,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    13,    14,
      15,     3,    17,     4,     4,    20,    21,    22,    23,    24,
       3,     3,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,     4,     3,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    13,    14,    15,     4,    17,
       4,     4,    20,    21,    22,    23,    24,     4,     4,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,     4,     3,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    13,    14,    15,     4,    17,    16,    54,    20,
      21,    22,    23,    24,    54,     3,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,     4,     3,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    13,
      14,    15,     3,    17,    19,     3,    20,    21,    22,    23,
      24,    19,     4,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,     4,     3,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    13,    14,    15,     4,
      17,   155,    62,    20,    21,    22,    23,    24,   420,   355,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,   114,   460,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    13,    14,    15,   221,    17,   468,   372,
      20,    21,    22,    23,    24,   163,   331,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   140,
     179,    41,    42,    43,    44,    45,    46,    47,    48,    49,
     261,   235,   417,    53,    54,    55,    13,    14,    15,   124,
      17,   359,   159,    20,    21,    22,    23,    24,   174,   150,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    76,    22,    41,    42,    43,    44,    45,    46,
      47,    48,    49,   188,    -1,    -1,    -1,    54,    55,    13,
      14,    15,    -1,    17,    -1,    -1,    20,    21,    22,    23,
      24,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    -1,    13,    14,
      15,    55,    17,    18,    -1,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    13,    14,    15,    54,
      17,    -1,    -1,    20,    21,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    13,    14,    15,    54,    17,    18,
      -1,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    13,    14,    15,    -1,    17,    -1,    -1,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    50,
      51,    52,    -1,    -1,    -1,    56,    57,    58,    59,    60,
      -1,    62,    63,    64,    65,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   121,   122,   123,   134,   135,   137,   138,   140,
     143,   146,   148,   149,   150,   152,   153,   154,   156,   157,
     158,   159,   160,   163,   164,    50,    51,    52,    56,    57,
      58,    59,    60,    62,    63,    64,    65,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     3,   150,
       3,   159,     0,     9,    98,    99,     5,     9,    94,    96,
       3,    87,    99,    98,    98,     3,    96,   110,   133,    98,
      96,   133,     8,    83,    83,    98,    98,    83,     9,   155,
     155,     3,   156,   156,   156,     3,   157,   157,   157,     3,
     156,   157,   157,     3,    13,    14,    15,    17,    20,    21,
      22,    23,    24,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    93,   103,   104,   105,   110,   115,
     116,   117,   118,   124,   128,   129,   130,   131,   132,   144,
     147,     4,    50,     4,    87,     3,    10,    86,   141,   144,
     147,     3,    11,    85,    90,    92,    94,   136,   144,   147,
      15,    17,    20,    61,   104,   111,   117,   133,    97,     3,
      91,    92,   139,   144,   147,   133,     8,    81,    82,    83,
       3,   145,    84,    66,    67,   151,   152,   155,    83,    83,
      83,    83,    83,   162,     4,     4,    83,    83,    83,    52,
      53,    54,    55,    62,    63,    99,   100,   100,   100,    96,
      96,    97,    96,    96,    96,    96,    96,    96,    96,    39,
     101,   101,     5,     6,     7,    95,     3,   128,     3,   118,
      96,    96,   100,     4,     3,    93,   125,   126,   127,   124,
       3,    88,    89,     4,    12,   118,   120,     4,     3,    86,
     141,     3,    85,    94,     4,    90,   136,   100,   100,   100,
     120,   110,   119,     4,   100,    97,     4,    96,    60,     4,
      91,   139,    82,     8,     4,     3,   121,   142,    50,    56,
      57,    59,    64,     4,     4,    10,    81,    81,     4,     4,
     162,     4,     4,     4,     4,     3,     4,   161,     4,     4,
       4,    96,    84,    99,    84,    84,    99,    83,    83,     3,
     106,   107,   118,   107,   107,    96,    40,   102,   102,   107,
      53,    54,     3,   125,    53,    54,     4,    10,    58,   107,
       3,   106,   110,   113,   114,   107,     4,   119,     3,     4,
      82,     4,    50,    56,    57,    59,     4,    96,    96,    96,
      96,    96,     4,     4,     4,    41,     4,     4,    10,     4,
       4,    10,    83,     4,    54,   107,    16,    16,    19,    16,
      25,    26,   108,   109,    84,    99,    84,    84,    99,    84,
       4,    97,    18,   113,     3,   110,    15,     4,    98,    98,
      98,    98,     4,     4,     4,     4,     4,    95,   129,     4,
       3,   130,     3,   132,     4,     4,    84,   100,   100,   100,
     100,    96,   118,   109,    16,     4,    10,     4,     4,    10,
       4,     4,   118,    18,   110,   107,     3,    88,    93,    86,
      90,    91,     4,   129,   132,     4,   118,   118,   100,   126,
       4,     3,   127,    88,     4,     3,    89,     4,   118,     4,
       4,     4,     4,     4,     4,    16,   126,    88,     3,     4,
       3,   112,   100,    19,     3,   108,   118,    19,     4,     4,
     118,   112,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    81,    82,    82,    83,    84,    84,    85,
      86,    86,    87,    88,    88,    88,    89,    89,    90,    91,
      92,    92,    93,    94,    95,    95,    95,    96,    96,    97,
      97,    98,    98,    99,   100,   100,   101,   101,   102,   102,
     103,   103,   103,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   105,
     105,   105,   105,   105,   106,   107,   107,   108,   108,   109,
     109,   110,   111,   111,   111,   111,   111,   112,   112,   113,
     113,   114,   114,   114,   114,   114,   114,   115,   116,   117,
     118,   118,   119,   119,   120,   121,   122,   123,   124,   124,
     124,   124,   124,   125,   126,   126,   126,   127,   127,   128,
     129,   129,   129,   130,   130,   131,   132,   132,   132,   133,
     133,   134,   134,   135,   136,   136,   136,   136,   137,   137,
     138,   139,   139,   139,   139,   140,   141,   141,   141,   142,
     142,   142,   142,   142,   142,   143,   144,   145,   145,   145,
     145,   145,   146,   147,   148,   148,   149,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   151,   151,
     152,   152,   153,   154,   155,   155,   156,   156,   156,   157,
     157,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     159,   159,   159,   159,   160,   160,   161,   162,   162,   163,
     163,   163,   164
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
       2,     3,     2,     3,     3,     3,     7,     3,     4,     2,
       1,     8,     4,     9,     5,     3,     2,     1,     1,     1,
       0,     2,     0,     2,     1,     5,     1,     5,     2,     1,
       3,     2,     2,     1,     1,     5,     6,     0,     5,     1,
       1,     5,     6,     1,     5,     1,     1,     5,     6,     4,
       1,     6,     5,     5,     1,     2,     2,     5,     6,     5,
       5,     1,     2,     2,     4,     5,     2,     2,     2,     5,
       5,     5,     5,     5,     1,     6,     5,     4,     4,     4,
       4,     4,     5,     4,     4,     5,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     2,     1,     1,     0,     1,     5,     6,     6,     6,
       5,     5,     5,     5,     5,     5,     4,     4,     5,     5,
       1,     1,     1,     5,     1,     2,     4,     0,     2,     0,
       1,     1,     1
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
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1714 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1720 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1726 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1732 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1738 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 39: /* OFFSET_EQ_NAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1744 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 40: /* ALIGN_EQ_NAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1750 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 81: /* text_list  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1756 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 82: /* text_list_opt  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1762 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 83: /* quoted_text  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1768 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* value_type_list  */
#line 246 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1774 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* global_type  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1780 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 87: /* func_type  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1786 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* func_sig  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1792 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* func_sig_result  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1798 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* memory_sig  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1804 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* type_use  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1810 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* literal  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1816 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* var  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1822 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* var_list  */
#line 248 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1828 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* bind_var_opt  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1834 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* bind_var  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1840 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* labeling_opt  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1846 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* instr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1852 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* plain_instr  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1858 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* block_instr  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1864 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* block_sig  */
#line 246 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1870 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* block  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1876 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* catch_instr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1882 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* catch_instr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1888 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* expr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1894 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* expr1  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1900 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* catch_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1906 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* if_block  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1912 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* if_  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1918 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* instr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1924 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* expr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1930 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* const_expr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 1936 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* func  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1942 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* func_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1948 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* func_fields_import  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1954 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* func_fields_import1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1960 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* func_fields_import_result  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1966 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* func_fields_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1972 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 129: /* func_fields_body1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1978 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* func_result_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1984 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* func_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1990 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* func_body1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1996 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* offset  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { DestroyExprList(((*yyvaluep).expr_list).first); }
#line 2002 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* table  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2008 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* table_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2014 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* memory  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2020 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* memory_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2026 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 140: /* global  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2032 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 141: /* global_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2038 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* import_desc  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2044 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 144: /* inline_import  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2050 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* export_desc  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2056 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 147: /* inline_export  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2062 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 150: /* module_field  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2068 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 151: /* module_fields_opt  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2074 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 152: /* module_fields  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2080 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 153: /* module  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2086 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 154: /* inline_module  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2092 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 155: /* script_var_opt  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 2098 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 156: /* script_module  */
#line 243 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script_module); }
#line 2104 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 157: /* action  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 2110 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 158: /* assertion  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2116 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 159: /* cmd  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2122 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 160: /* cmd_list  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2128 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 162: /* const_list  */
#line 232 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2134 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 163: /* script  */
#line 244 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2140 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
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
#line 261 "src/wast-parser.y" /* yacc.c:1646  */
    {
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 2439 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 267 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 2452 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 277 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2458 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 282 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2476 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 300 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2482 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 301 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].type));
    }
#line 2491 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 307 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2497 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 310 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].type);
      (yyval.global)->mutable_ = false;
    }
#line 2507 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 315 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].type);
      (yyval.global)->mutable_ = true;
    }
#line 2517 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 323 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2523 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 328 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2533 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 333 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].type));
      // Ignore bind_var.
      destroy_string_slice(&(yyvsp[-3].text));
    }
#line 2544 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 342 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2550 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 343 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->result_types.insert((yyval.func_sig)->result_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2560 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 351 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
#line 2569 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 357 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
#line 2578 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 363 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2588 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 368 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2598 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 375 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2604 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 381 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid int " PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2617 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 392 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2626 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 396 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2635 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 400 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2644 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 407 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var((yyvsp[0].u64));
      (yyval.var)->loc = (yylsp[0]);
    }
#line 2653 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 411 "src/wast-parser.y" /* yacc.c:1646  */
    {
      StringSlice name;
      DUPTEXT(name, (yyvsp[0].text));
      (yyval.var) = new Var(name);
      (yyval.var)->loc = (yylsp[0]);
    }
#line 2664 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 419 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2670 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 420 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->emplace_back(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2680 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 427 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2686 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 431 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2692 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 34:
#line 435 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2698 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 440 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2704 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 441 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2723 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 457 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2729 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 458 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2746 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 473 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2752 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 474 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2758 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 479 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2766 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 482 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2774 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 485 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2782 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 488 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2790 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 491 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2799 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 495 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2808 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 499 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2817 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 503 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2825 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 506 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2834 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 510 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2843 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 514 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2852 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 518 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2861 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 522 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2870 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 526 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2879 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 530 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2888 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 534 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2896 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 537 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2904 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 540 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2922 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 553 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2930 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 556 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 2938 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 559 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 2946 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 562 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 2954 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 565 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 2962 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 568 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 2970 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 571 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateThrow(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2979 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 575 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateRethrow(std::move(*(yyvsp[0].var)));
      delete (yyvsp[0].var);
    }
#line 2988 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 582 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBlock((yyvsp[-2].block));
      (yyval.expr)->block->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block->label, (yyvsp[0].text));
    }
#line 2998 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 587 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoop((yyvsp[-2].block));
      (yyval.expr)->loop->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->loop->label, (yyvsp[0].text));
    }
#line 3008 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 592 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-2].block), nullptr);
      (yyval.expr)->if_.true_->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3018 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 597 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-5].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->if_.true_->label = (yyvsp[-6].text);
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->if_.true_->label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3029 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 603 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-3].block)->label = (yyvsp[-4].text);
      (yyval.expr) = Expr::CreateTry((yyvsp[-3].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->try_block.label = (yyvsp[-4].text);
      CHECK_END_LABEL((yylsp[0]), (yyvsp[-3].block)->label, (yyvsp[0].text));
    }
#line 3040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 612 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 3046 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 615 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = (yyvsp[0].block);
      (yyval.block)->sig.insert((yyval.block)->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3056 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 620 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = new Block();
      (yyval.block)->first = (yyvsp[0].expr_list).first;
    }
#line 3065 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 627 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatch(std::move(*(yyvsp[-1].var)), (yyvsp[0].expr_list).first);
      delete (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3075 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 632 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatchAll((yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 3084 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 640 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-1].expr_list), &(yyvsp[0].expr_list));
    }
#line 3092 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 646 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 3098 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 650 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 3106 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 653 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateBlock((yyvsp[0].block));
      expr->block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3116 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 658 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateLoop((yyvsp[0].block));
      expr->loop->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3126 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 663 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      if_->if_.true_->label = (yyvsp[-1].text);
    }
#line 3137 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 669 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* try_ = Expr::CreateTry((yyvsp[-2].block), (yyvsp[0].expr_list).first);
      try_->try_block.label = (yyvsp[-5].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-6]), try_);
    }
#line 3147 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 677 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3155 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 680 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-2].expr_list), &(yyvsp[0].expr_list));
    }
#line 3163 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 686 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->if_.true_;
      true_->sig.insert(true_->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3176 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 697 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
#line 3185 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 701 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 3194 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 705 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
#line 3203 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 709 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
#line 3212 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 713 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
#line 3221 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 717 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[0].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
#line 3230 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 724 "src/wast-parser.y" /* yacc.c:1646  */
    {
     CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "rethrow");
    }
#line 3238 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 729 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "throw");
    }
#line 3246 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 735 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "try");      
    }
#line 3254 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 741 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3260 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 742 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3271 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 750 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3277 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 751 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3288 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 764 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exception) = new Exception();
      (yyval.exception)->name = (yyvsp[-2].text);
      (yyval.exception)->sig = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 3299 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 772 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Except);
      (yyval.module_field)->loc = (yylsp[0]);
      (yyval.module_field)->except = (yyvsp[0].exception);
    }
#line 3309 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 781 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3325 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 795 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3338 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 803 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3348 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 808 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3364 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 819 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3377 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 827 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3390 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 838 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3399 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 846 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3410 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 852 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3422 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 862 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3428 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 863 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3439 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 872 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3448 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 880 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3459 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 886 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3471 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 897 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3482 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 906 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
#line 3491 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 913 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->first_expr = (yyvsp[0].expr_list).first;
    }
#line 3500 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 917 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3510 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 922 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].type));
    }
#line 3522 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 934 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3530 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 941 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3545 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 951 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3561 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 965 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3577 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 979 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Table);
      field->loc = (yylsp[0]);
      field->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3588 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 985 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3601 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 993 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3614 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 1001 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3636 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1021 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3651 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1031 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3667 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1045 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3683 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1059 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Memory);
      field->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3693 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1064 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3706 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1072 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3719 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1080 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3745 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1104 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3761 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1118 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Global);
      field->global = (yyvsp[-1].global);
      field->global->init_expr = (yyvsp[0].expr_list).first;
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3772 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1124 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3785 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1132 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3798 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1145 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3812 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1154 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3825 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1162 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = (yyvsp[-2].text);
    }
#line 3836 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1168 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = (yyvsp[-2].text);
    }
#line 3847 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1174 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = (yyvsp[-2].text);
    }
#line 3858 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1180 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Except;
      (yyval.import)->except = (yyvsp[0].exception);
    }
#line 3868 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1188 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Import);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->import = (yyvsp[-1].import);
      (yyval.module_field)->import->module_name = (yyvsp[-3].text);
      (yyval.module_field)->import->field_name = (yyvsp[-2].text);
    }
#line 3880 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1198 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
#line 3890 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1206 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3901 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1212 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3912 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1218 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3923 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1224 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3934 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1230 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Except;
      (yyval.export_)->var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 3945 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1238 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Export);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->export_ = (yyvsp[-1].export_);
      (yyval.module_field)->export_->name = (yyvsp[-2].text);
    }
#line 3956 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1247 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = (yyvsp[-1].text);
    }
#line 3965 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1257 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1264 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->name = (yyvsp[-2].text);
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3990 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1275 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Start);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->start = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
    }
#line 4001 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1284 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4007 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1289 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4013 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1290 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1291 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4025 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1292 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4031 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 176:
#line 1293 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4037 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 177:
#line 1294 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4043 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1298 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 4049 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1303 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new Module();
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4059 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1308 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4069 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1316 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4091 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1343 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var(kInvalidIndex);
    }
#line 4099 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1346 "src/wast-parser.y" /* yacc.c:1646  */
    {
      StringSlice name;
      DUPTEXT(name, (yyvsp[0].text));
      (yyval.var) = new Var(name);
    }
#line 4109 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1354 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4132 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1372 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Binary;
      (yyval.script_module)->binary.name = (yyvsp[-3].text);
      (yyval.script_module)->binary.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->binary.data, &(yyval.script_module)->binary.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4145 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1380 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Quoted;
      (yyval.script_module)->quoted.name = (yyvsp[-3].text);
      (yyval.script_module)->quoted.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->quoted.data, &(yyval.script_module)->quoted.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4158 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1391 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4174 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1402 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = std::move(*(yyvsp[-2].var));
      delete (yyvsp[-2].var);
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = (yyvsp[-1].text);
    }
#line 4187 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 191:
#line 1413 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertMalformed;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
#line 4198 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 192:
#line 1419 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertInvalid;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 4209 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 193:
#line 1425 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUnlinkable;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
#line 4220 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 194:
#line 1431 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUninstantiable;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
#line 4231 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 195:
#line 1437 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturn;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
#line 4242 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 196:
#line 1443 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnCanonicalNan;
      (yyval.command)->assert_return_canonical_nan.action = (yyvsp[-1].action);
    }
#line 4252 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 197:
#line 1448 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnArithmeticNan;
      (yyval.command)->assert_return_arithmetic_nan.action = (yyvsp[-1].action);
    }
#line 4262 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 198:
#line 1453 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertTrap;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4273 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 199:
#line 1459 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertExhaustion;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4284 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 200:
#line 1468 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Action;
      (yyval.command)->action = (yyvsp[0].action);
    }
#line 4294 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 202:
#line 1474 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Module;
      (yyval.command)->module = (yyvsp[0].module);
    }
#line 4304 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 203:
#line 1479 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Register;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = std::move(*(yyvsp[-1].var));
      delete (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
#line 4317 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 204:
#line 1489 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4326 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 205:
#line 1493 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4335 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 206:
#line 1500 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4350 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 207:
#line 1512 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4356 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 208:
#line 1513 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
#line 4365 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 209:
#line 1520 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4373 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 210:
#line 1523 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4438 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 211:
#line 1583 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      Command* command = new Command();
      command->type = CommandType::Module;
      command->module = (yyvsp[0].module);
      (yyval.script)->commands.emplace_back(command);
    }
#line 4450 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 212:
#line 1595 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4456 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4460 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1598 "src/wast-parser.y" /* yacc.c:1906  */


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
