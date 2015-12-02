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




/* Copy the first part of user declarations.  */
#line 1 "src/wasm-parser.y" /* yacc.c:339  */

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "wasm.h"
#include "wasm-internal.h"

#define ZEROMEM(var) memset(&(var), 0, sizeof(var));
#define DUPTEXT(dst, src)                           \
  (dst).start = strndup((src).start, (src).length); \
  (dst).length = (src).length
#define DUPQUOTEDTEXT(dst, src)                             \
  (dst).start = strndup((src).start + 1, (src).length - 2); \
  (dst).length = (src).length - 2

#define YYLLOC_DEFAULT(Current, Rhs, N)                                        \
  do                                                                           \
    if (N) {                                                                   \
      (Current).filename = YYRHSLOC(Rhs, 1).filename;                          \
      (Current).first_line = YYRHSLOC(Rhs, 1).first_line;                      \
      (Current).first_column = YYRHSLOC(Rhs, 1).first_column;                  \
      (Current).last_line = YYRHSLOC(Rhs, N).last_line;                        \
      (Current).last_column = YYRHSLOC(Rhs, N).last_column;                    \
    } else {                                                                   \
      (Current).filename = NULL;                                               \
      (Current).first_line = (Current).last_line = YYRHSLOC(Rhs, 0).last_line; \
      (Current).first_column = (Current).last_column =                         \
          YYRHSLOC(Rhs, 0).last_column;                                        \
    }                                                                          \
  while (0)

static WasmExprPtr wasm_new_expr(WasmExprType type) {
  WasmExprPtr result = calloc(1, sizeof(WasmExpr));
  result->type = type;
  return result;
}

static int read_int32(const char* s, const char* end, uint32_t* out,
                      int allow_signed);
static int read_int64(const char* s, const char* end, uint64_t* out);
static int read_uint64(const char* s, const char* end, uint64_t* out);
static int read_float(const char* s, const char* end, float* out);
static int read_double(const char* s, const char* end, double* out);
static int read_const(WasmType type, const char* s, const char* end,
                      WasmConst* out);
static void dup_string_contents(WasmStringSlice * text, void** out_data,
                                size_t* out_size);


#line 121 "src/wasm-parser.c" /* yacc.c:339  */

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
   by #include "wasm-parser.h".  */
#ifndef YY_YY_SRC_WASM_PARSER_H_INCLUDED
# define YY_YY_SRC_WASM_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    WASM_TOKEN_TYPE_EOF = 0,
    WASM_TOKEN_TYPE_LPAR = 258,
    WASM_TOKEN_TYPE_RPAR = 259,
    WASM_TOKEN_TYPE_INT = 260,
    WASM_TOKEN_TYPE_FLOAT = 261,
    WASM_TOKEN_TYPE_TEXT = 262,
    WASM_TOKEN_TYPE_VAR = 263,
    WASM_TOKEN_TYPE_VALUE_TYPE = 264,
    WASM_TOKEN_TYPE_NOP = 265,
    WASM_TOKEN_TYPE_BLOCK = 266,
    WASM_TOKEN_TYPE_IF = 267,
    WASM_TOKEN_TYPE_IF_ELSE = 268,
    WASM_TOKEN_TYPE_LOOP = 269,
    WASM_TOKEN_TYPE_LABEL = 270,
    WASM_TOKEN_TYPE_BR = 271,
    WASM_TOKEN_TYPE_BR_IF = 272,
    WASM_TOKEN_TYPE_TABLESWITCH = 273,
    WASM_TOKEN_TYPE_CASE = 274,
    WASM_TOKEN_TYPE_CALL = 275,
    WASM_TOKEN_TYPE_CALL_IMPORT = 276,
    WASM_TOKEN_TYPE_CALL_INDIRECT = 277,
    WASM_TOKEN_TYPE_RETURN = 278,
    WASM_TOKEN_TYPE_GET_LOCAL = 279,
    WASM_TOKEN_TYPE_SET_LOCAL = 280,
    WASM_TOKEN_TYPE_LOAD = 281,
    WASM_TOKEN_TYPE_STORE = 282,
    WASM_TOKEN_TYPE_LOAD_EXTEND = 283,
    WASM_TOKEN_TYPE_STORE_WRAP = 284,
    WASM_TOKEN_TYPE_OFFSET = 285,
    WASM_TOKEN_TYPE_ALIGN = 286,
    WASM_TOKEN_TYPE_CONST = 287,
    WASM_TOKEN_TYPE_UNARY = 288,
    WASM_TOKEN_TYPE_BINARY = 289,
    WASM_TOKEN_TYPE_COMPARE = 290,
    WASM_TOKEN_TYPE_CONVERT = 291,
    WASM_TOKEN_TYPE_CAST = 292,
    WASM_TOKEN_TYPE_SELECT = 293,
    WASM_TOKEN_TYPE_FUNC = 294,
    WASM_TOKEN_TYPE_TYPE = 295,
    WASM_TOKEN_TYPE_PARAM = 296,
    WASM_TOKEN_TYPE_RESULT = 297,
    WASM_TOKEN_TYPE_LOCAL = 298,
    WASM_TOKEN_TYPE_MODULE = 299,
    WASM_TOKEN_TYPE_MEMORY = 300,
    WASM_TOKEN_TYPE_SEGMENT = 301,
    WASM_TOKEN_TYPE_IMPORT = 302,
    WASM_TOKEN_TYPE_EXPORT = 303,
    WASM_TOKEN_TYPE_TABLE = 304,
    WASM_TOKEN_TYPE_UNREACHABLE = 305,
    WASM_TOKEN_TYPE_MEMORY_SIZE = 306,
    WASM_TOKEN_TYPE_GROW_MEMORY = 307,
    WASM_TOKEN_TYPE_HAS_FEATURE = 308,
    WASM_TOKEN_TYPE_ASSERT_INVALID = 309,
    WASM_TOKEN_TYPE_ASSERT_RETURN = 310,
    WASM_TOKEN_TYPE_ASSERT_RETURN_NAN = 311,
    WASM_TOKEN_TYPE_ASSERT_TRAP = 312,
    WASM_TOKEN_TYPE_INVOKE = 313,
    WASM_TOKEN_TYPE_GLOBAL = 314,
    WASM_TOKEN_TYPE_LOAD_GLOBAL = 315,
    WASM_TOKEN_TYPE_STORE_GLOBAL = 316,
    WASM_TOKEN_TYPE_PAGE_SIZE = 317,
    WASM_TOKEN_TYPE_LOW = 318
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef WasmToken YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (WasmScanner scanner, WasmParser* parser);

#endif /* !YY_YY_SRC_WASM_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 250 "src/wasm-parser.c" /* yacc.c:358  */

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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   744

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  188
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  359

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

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
      55,    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   131,   131,   132,   135,   136,   140,   144,   151,   152,
     156,   164,   171,   172,   175,   179,   183,   187,   191,   192,
     196,   197,   204,   205,   213,   216,   217,   221,   226,   232,
     237,   242,   248,   254,   259,   266,   271,   275,   295,   300,
     305,   311,   315,   320,   327,   335,   342,   350,   358,   363,
     369,   376,   382,   387,   392,   393,   394,   398,   402,   406,
     411,   414,   415,   418,   419,   422,   423,   427,   431,   437,
     438,   441,   442,   445,   446,   453,   458,   466,   471,   481,
     484,   489,   497,   502,   512,   515,   518,   522,   527,   533,
     540,   548,   557,   565,   572,   579,   586,   592,   599,   607,
     614,   620,   627,   633,   638,   644,   649,   655,   662,   670,
     677,   683,   690,   696,   701,   707,   714,   720,   725,   729,
     734,   740,   747,   755,   762,   768,   775,   781,   786,   792,
     799,   805,   810,   816,   821,   825,   830,   836,   843,   849,
     854,   860,   865,   869,   874,   880,   885,   889,   894,   900,
     900,   906,   916,   917,   921,   931,   942,   946,   953,   957,
     964,   972,   979,   990,   997,  1001,  1012,  1013,  1020,  1027,
    1034,  1041,  1048,  1055,  1064,  1128,  1129,  1135,  1140,  1147,
    1153,  1162,  1163,  1167,  1176,  1177,  1180,  1181,  1185
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "INT", "FLOAT",
  "TEXT", "VAR", "VALUE_TYPE", "NOP", "BLOCK", "IF", "IF_ELSE", "LOOP",
  "LABEL", "BR", "BR_IF", "TABLESWITCH", "CASE", "CALL", "CALL_IMPORT",
  "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "LOAD", "STORE",
  "LOAD_EXTEND", "STORE_WRAP", "OFFSET", "ALIGN", "CONST", "UNARY",
  "BINARY", "COMPARE", "CONVERT", "CAST", "SELECT", "FUNC", "TYPE",
  "PARAM", "RESULT", "LOCAL", "MODULE", "MEMORY", "SEGMENT", "IMPORT",
  "EXPORT", "TABLE", "UNREACHABLE", "MEMORY_SIZE", "GROW_MEMORY",
  "HAS_FEATURE", "ASSERT_INVALID", "ASSERT_RETURN", "ASSERT_RETURN_NAN",
  "ASSERT_TRAP", "INVOKE", "GLOBAL", "LOAD_GLOBAL", "STORE_GLOBAL",
  "PAGE_SIZE", "LOW", "$accept", "value_type_list", "func_type", "literal",
  "var", "var_list", "bind_var", "text", "quoted_text", "string_contents",
  "labeling", "offset", "align", "expr", "expr1", "expr_opt",
  "non_empty_expr_list", "expr_list", "target", "target_list", "case",
  "case_list", "param_list", "result", "local_list", "type_use",
  "func_info", "func", "@1", "segment", "segment_list", "memory",
  "type_def", "table", "import", "export", "global", "module_fields",
  "module", "cmd", "cmd_list", "const", "const_opt", "const_list",
  "script", YY_NULLPTR
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
     315,   316,   317,   318
};
# endif

#define YYPACT_NINF -157

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-157)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -157,     3,    32,    10,  -157,  -157,  -157,  -157,     7,    35,
      42,    57,    63,    77,    29,    63,    39,    53,    59,  -157,
    -157,    81,  -157,  -157,  -157,  -157,  -157,  -157,  -157,  -157,
     141,    63,    63,    63,    97,  -157,    28,   156,   117,    63,
    -157,   163,  -157,  -157,  -157,  -157,   142,  -157,  -157,    50,
     150,  -157,   190,   194,    63,    63,    54,    14,    65,   193,
     139,   152,   162,   173,   417,   198,  -157,   200,   202,   203,
     204,   211,   212,   214,   176,  -157,   164,    63,   215,  -157,
    -157,   217,  -157,  -157,  -157,  -157,   218,   216,   219,    63,
    -157,  -157,   221,  -157,   163,   200,   200,   163,   163,    34,
      54,   163,    54,    54,    54,   200,    54,    54,   197,   197,
     197,   197,   173,   200,   200,   200,   200,   200,   200,    54,
     163,   220,   163,  -157,  -157,   200,   223,    54,    54,  -157,
     224,   200,   202,   203,   204,   211,   682,  -157,   470,   200,
     203,   204,   576,   200,   204,   629,   200,   523,   200,   202,
     203,   204,  -157,   140,   227,   214,   180,   187,  -157,  -157,
     215,   106,   231,   236,  -157,  -157,  -157,   238,  -157,   240,
    -157,  -157,   200,   200,   200,   163,   200,   200,   200,  -157,
    -157,   200,   200,   200,   200,   200,  -157,  -157,   200,  -157,
     222,   222,   222,   222,  -157,  -157,   200,   200,  -157,  -157,
     200,   241,    73,   237,   244,   104,   242,  -157,  -157,  -157,
    -157,   200,  -157,   200,   203,   204,   200,   204,   200,   200,
     202,   203,   204,   163,   200,   204,   200,   200,   163,   200,
     203,   204,   200,   204,   200,  -157,   243,   245,   246,  -157,
     249,   251,   252,  -157,  -157,  -157,  -157,   200,  -157,   200,
     200,   200,  -157,  -157,  -157,  -157,   254,  -157,  -157,   200,
    -157,  -157,   200,   200,   200,   200,  -157,  -157,   200,  -157,
    -157,   255,  -157,  -157,   256,  -157,   200,   204,   200,   200,
     200,   203,   204,   200,   204,   200,   105,   253,   200,   114,
     257,   200,   204,   200,   200,   127,   259,  -157,   260,   258,
    -157,  -157,  -157,  -157,   209,  -157,  -157,   200,  -157,   200,
    -157,  -157,  -157,   200,   200,   204,   200,   200,  -157,   263,
    -157,   267,   200,   271,  -157,  -157,  -157,   274,  -157,  -157,
    -157,   200,  -157,  -157,   233,  -157,   192,   270,   125,   278,
    -157,   280,    54,    54,  -157,  -157,   281,   284,   279,  -157,
    -157,   275,  -157,   129,   200,   285,   286,  -157,  -157
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     181,   188,     0,     0,   175,   182,     1,   166,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    16,
     186,     0,   174,   167,   172,   171,   170,   168,   169,   173,
       0,     0,     0,     0,     0,   149,     0,     0,     0,     0,
      12,     2,   177,   186,   186,   186,     0,   176,   187,    85,
       0,    14,     0,   152,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,    63,   148,   134,   142,
     146,   118,     0,     4,     0,   152,     0,     0,     4,    10,
      11,     0,   158,    13,   164,     3,     0,   184,     0,     0,
       8,     9,     0,    25,    18,     0,     0,    18,    18,    61,
       0,    18,     0,     0,     0,    61,     0,     0,    20,    20,
      20,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     2,    54,    55,     0,     0,     0,     0,    60,
       0,   117,   105,   113,   103,    87,     0,    64,     0,   141,
     135,   139,     0,   145,   143,     0,   147,     0,   133,   119,
     127,   131,   150,     0,     0,     4,     0,     0,   155,   153,
       4,     0,     0,     0,   163,   165,   185,     0,   179,     0,
     183,    19,    26,     0,     0,    19,    65,     0,    61,    62,
      34,     0,     0,    65,    65,     0,    36,    41,     0,    21,
      22,    22,    22,    22,    47,    48,     0,     0,    52,    53,
       0,     0,     0,     0,     0,     0,     0,    56,    15,    57,
      58,     0,    24,   112,   106,   110,   116,   114,   104,   102,
      88,    96,   100,     2,   138,   136,   140,   144,     2,   126,
     120,   124,   130,   128,   132,     2,     0,     0,     0,   154,
       0,     0,     0,   161,   159,   178,   180,    27,    29,     0,
      65,    66,    31,    33,    35,    30,     0,    38,    39,    65,
      42,    23,     0,     0,     0,     0,    49,    51,     0,    84,
      75,     0,    79,    80,     0,    59,   109,   107,   111,   115,
      95,    89,    93,    99,    97,   101,     0,     0,   137,     0,
       0,   123,   121,   125,   129,     0,     0,   156,     0,     0,
     162,   160,    28,    32,     0,    40,    43,     0,    45,     0,
      50,    76,    81,   108,    92,    90,    94,    98,    77,     0,
      82,     0,   122,     5,     7,   157,    17,     0,    69,    44,
      46,    91,    78,    83,     0,   151,     0,     0,     0,     0,
      70,     0,     0,     0,    73,     6,     0,     0,    37,    68,
      67,     0,    74,    65,    65,     0,     0,    71,    72
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -157,  -113,   -48,   149,   -56,  -157,   -36,  -157,     2,  -157,
      37,    40,   -39,    95,  -157,   -76,   -45,  -156,   -46,  -157,
    -157,  -157,   -50,   -16,   -57,   -62,  -157,  -157,  -157,  -157,
     225,  -157,  -157,  -157,  -157,  -157,  -157,  -157,   288,  -157,
    -157,   210,  -157,   115,  -157
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    58,   154,    92,    81,    57,   171,   209,    20,   327,
     172,   190,   262,    66,   130,   180,   251,   252,   340,   336,
     352,   348,    68,    69,    70,    71,    72,    23,    49,   159,
      76,    24,    25,    26,    27,    28,    29,    13,     4,     5,
       1,    48,   167,    34,     2
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      52,    83,    54,   135,    67,    59,     3,   202,   134,   205,
      14,   141,   144,    65,   151,   132,   163,    30,    82,    79,
     131,   149,    80,   139,   143,   146,   148,   257,   258,   186,
     162,    50,     6,    43,    44,    45,    51,   136,    16,    79,
      55,    56,    80,   178,   181,    17,   183,   184,   185,   133,
     187,   188,   140,    64,     7,   150,    77,    78,    51,    79,
      18,   175,    80,   201,     8,     9,    10,    11,    12,    84,
      19,   210,   211,     7,    85,   215,   217,   270,   222,   160,
      21,    22,    85,   225,   203,   220,   206,   213,   216,   218,
     219,   169,   231,   233,   303,   224,   226,    31,   242,   227,
      46,    47,   254,   305,   229,   232,   234,   238,   273,   318,
     286,    32,   241,    85,    85,   289,   214,    33,   320,   221,
      35,    36,   295,    85,    19,    51,    37,   247,    38,    39,
      40,   323,   136,   230,   176,   177,    85,    51,   182,   250,
      41,   342,    46,    87,   343,    42,   119,   235,   236,   191,
     192,   193,   263,   264,   265,    46,    88,   277,    60,    61,
      62,    53,   137,   282,   284,    46,    89,   157,   158,   276,
     278,    51,   279,   292,    63,   280,   283,   285,    90,    91,
     288,   235,   236,   157,   239,   291,   293,   287,   294,    73,
     173,   174,   290,    74,   179,   338,   339,   355,   356,    75,
     179,    64,    86,   136,   281,   138,   142,   145,   195,   196,
     197,   198,   199,   200,   147,   155,   152,   153,   161,    46,
     207,   164,   165,   168,   315,   170,   137,   189,   212,   204,
     208,   237,   313,   240,   137,   243,   314,   316,   137,   317,
     244,   137,   245,   137,   246,   269,   271,   322,   272,   297,
     298,   274,   296,   261,   299,   300,   301,   304,   328,   311,
     312,   194,   319,   324,   325,   326,   321,   332,   248,   249,
     331,   333,   253,   179,   334,   337,   255,   256,   335,   341,
     259,   338,   351,   260,   345,   349,   346,   347,   350,   357,
     358,   266,   267,   344,   353,   268,    15,   166,     0,     0,
     156,     0,     0,     0,     0,     0,   275,     0,   137,     0,
       0,   137,     0,   137,   137,     0,     0,   354,     0,   137,
       0,   137,   137,     0,   137,     0,     0,   137,     0,   137,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   137,     0,   302,     0,   137,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   306,   307,   308,
     309,     0,     0,   310,     0,     0,     0,     0,     0,     0,
       0,   137,     0,   137,   137,   137,     0,     0,   137,     0,
     137,     0,     0,   137,     0,     0,   137,     0,   137,   137,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   329,     0,   330,     0,     0,     0,   137,   137,
       0,   137,   137,     0,     0,     0,     0,   137,     0,     0,
       0,     0,     0,     0,     0,     0,   137,    93,    94,    95,
      96,    97,    98,    99,   100,   101,     0,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
     113,   114,   115,   116,   117,   118,     0,   119,   120,   121,
     122,     0,     0,     0,     0,     0,     0,   123,   124,   125,
     126,     0,     0,     0,     0,     0,     0,   127,   128,   129,
      93,    94,    95,    96,    97,    98,    99,   100,   101,     0,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,   113,   114,   115,   116,   117,   118,     0,
       0,   223,   121,   122,     0,     0,     0,     0,     0,     0,
     123,   124,   125,   126,     0,     0,     0,     0,     0,     0,
     127,   128,   129,    93,    94,    95,    96,    97,    98,    99,
     100,   101,     0,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,   113,   114,   115,   116,
     117,   118,     0,     0,   120,   121,   122,     0,     0,     0,
       0,     0,     0,   123,   124,   125,   126,     0,     0,     0,
       0,     0,     0,   127,   128,   129,    93,    94,    95,    96,
      97,    98,    99,   100,   101,     0,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,   113,
     114,   115,   116,   117,   118,     0,     0,     0,     0,   122,
       0,     0,     0,     0,     0,     0,   123,   124,   125,   126,
       0,     0,     0,     0,     0,     0,   127,   128,   129,    93,
      94,    95,    96,    97,    98,    99,   100,   101,     0,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,   113,   114,   115,   116,   117,   118,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,     0,   123,
     124,   125,   126,     0,     0,     0,     0,     0,     0,   127,
     128,   129,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     0,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,   113,   114,   115,   116,   117,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   123,   124,   125,   126,     0,     0,     0,     0,
       0,     0,   127,   128,   129
};

static const yytype_int16 yycheck[] =
{
      36,    57,    38,    65,    49,    41,     3,   120,    65,   122,
       3,    68,    69,    49,    71,    65,    78,    15,     4,     5,
      65,    71,     8,    68,    69,    70,    71,   183,   184,   105,
      78,     3,     0,    31,    32,    33,     8,     3,     3,     5,
      38,    39,     8,    99,   100,     3,   102,   103,   104,    65,
     106,   107,    68,     3,    44,    71,    54,    55,     8,     5,
       3,    97,     8,   119,    54,    55,    56,    57,    58,     4,
       7,   127,   128,    44,     9,   132,   133,     4,   135,    77,
       3,     4,     9,   140,   120,   135,   122,   132,   133,   134,
     135,    89,   149,   150,   250,   140,   141,    58,   160,   144,
       3,     4,   178,   259,   149,   150,   151,   155,     4,     4,
     223,    58,   160,     9,     9,   228,   132,    58,     4,   135,
      39,    40,   235,     9,     7,     8,    45,   172,    47,    48,
      49,     4,     3,   149,    97,    98,     9,     8,   101,   175,
      59,    16,     3,     4,    19,     4,    40,    41,    42,   109,
     110,   111,   191,   192,   193,     3,     4,   214,    43,    44,
      45,     5,    67,   220,   221,     3,     4,     3,     4,   214,
     215,     8,   217,   230,    32,   220,   221,   222,     5,     6,
     225,    41,    42,     3,     4,   230,   231,   223,   233,    39,
      95,    96,   228,     3,    99,     3,     4,   353,   354,     5,
     105,     3,     9,     3,   220,     3,     3,     3,   113,   114,
     115,   116,   117,   118,     3,    39,     4,     3,     3,     3,
     125,     4,     4,     4,   281,     4,   131,    30,     4,     9,
       7,     4,   277,    46,   139,     4,   281,   282,   143,   284,
       4,   146,     4,   148,     4,     4,     9,   292,     4,     4,
       4,     9,     9,    31,     5,     4,     4,     3,    49,     4,
       4,   112,     9,     4,     4,     7,     9,     4,   173,   174,
     315,     4,   177,   178,     3,    42,   181,   182,     4,     9,
     185,     3,     3,   188,     4,     4,   342,   343,     4,     4,
       4,   196,   197,   339,    19,   200,     8,    87,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    -1,   211,    -1,   213,    -1,
      -1,   216,    -1,   218,   219,    -1,    -1,   353,    -1,   224,
      -1,   226,   227,    -1,   229,    -1,    -1,   232,    -1,   234,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   247,    -1,   249,    -1,   251,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   262,   263,   264,
     265,    -1,    -1,   268,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   276,    -1,   278,   279,   280,    -1,    -1,   283,    -1,
     285,    -1,    -1,   288,    -1,    -1,   291,    -1,   293,   294,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   307,    -1,   309,    -1,    -1,    -1,   313,   314,
      -1,   316,   317,    -1,    -1,    -1,    -1,   322,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   331,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    -1,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   104,   108,     3,   102,   103,     0,    44,    54,    55,
      56,    57,    58,   101,     3,   102,     3,     3,     3,     7,
      72,     3,     4,    91,    95,    96,    97,    98,    99,   100,
      72,    58,    58,    58,   107,    39,    40,    45,    47,    48,
      49,    59,     4,    72,    72,    72,     3,     4,   105,    92,
       3,     8,    70,     5,    70,    72,    72,    69,    65,    70,
     107,   107,   107,    32,     3,    70,    77,    80,    86,    87,
      88,    89,    90,    39,     3,     5,    94,    72,    72,     5,
       8,    68,     4,    68,     4,     9,     9,     4,     4,     4,
       5,     6,    67,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    32,    33,    34,    35,    36,    37,    38,    40,
      41,    42,    43,    50,    51,    52,    53,    60,    61,    62,
      78,    80,    86,    87,    88,    89,     3,    77,     3,    80,
      87,    88,     3,    80,    88,     3,    80,     3,    80,    86,
      87,    88,     4,     3,    66,    39,    94,     3,     4,    93,
      72,     3,    66,    89,     4,     4,   105,   106,     4,    72,
       4,    70,    74,    77,    77,    70,    74,    74,    68,    77,
      79,    68,    74,    68,    68,    68,    79,    68,    68,    30,
      75,    75,    75,    75,    67,    77,    77,    77,    77,    77,
      77,    68,    65,    70,     9,    65,    70,    77,     7,    71,
      68,    68,     4,    80,    87,    88,    80,    88,    80,    80,
      86,    87,    88,    41,    80,    88,    80,    80,    43,    80,
      87,    88,    80,    88,    80,    41,    42,     4,    66,     4,
      46,    66,    89,     4,     4,     4,     4,    80,    77,    77,
      70,    80,    81,    77,    79,    77,    77,    81,    81,    77,
      77,    31,    76,    76,    76,    76,    77,    77,    77,     4,
       4,     9,     4,     4,     9,    77,    80,    88,    80,    80,
      80,    87,    88,    80,    88,    80,    65,    70,    80,    65,
      70,    80,    88,    80,    80,    65,     9,     4,     4,     5,
       4,     4,    77,    81,     3,    81,    77,    77,    77,    77,
      77,     4,     4,    80,    80,    88,    80,    80,     4,     9,
       4,     9,    80,     4,     4,     4,     7,    73,    49,    77,
      77,    80,     4,     4,     3,     4,    83,    42,     3,     4,
      82,     9,    16,    19,    82,     4,    68,    68,    85,     4,
       4,     3,    84,    19,    70,    81,    81,     4,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    64,    65,    65,    66,    66,    66,    66,    67,    67,
      68,    68,    69,    69,    70,    71,    72,    73,    74,    74,
      75,    75,    76,    76,    77,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    79,    79,    80,    80,    81,    81,    82,    82,    83,
      83,    84,    84,    85,    85,    86,    86,    86,    86,    87,
      88,    88,    88,    88,    89,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    92,
      91,    93,    94,    94,    95,    95,    96,    96,    97,    98,
      98,    98,    98,    99,   100,   100,   101,   101,   101,   101,
     101,   101,   101,   101,   102,   103,   103,   103,   103,   103,
     103,   104,   104,   105,   106,   106,   107,   107,   108
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     4,     8,     4,     1,     1,
       1,     1,     0,     2,     1,     1,     1,     1,     0,     1,
       0,     1,     0,     1,     3,     1,     2,     3,     4,     3,
       3,     3,     4,     3,     2,     3,     2,     9,     3,     3,
       4,     2,     3,     4,     5,     4,     5,     2,     2,     3,
       4,     3,     2,     2,     1,     1,     2,     2,     2,     3,
       1,     0,     1,     1,     2,     0,     1,     4,     4,     0,
       2,     4,     5,     0,     2,     4,     5,     5,     6,     4,
       4,     5,     5,     6,     4,     0,     1,     2,     3,     4,
       5,     6,     5,     4,     5,     4,     3,     4,     5,     4,
       3,     4,     3,     2,     3,     2,     3,     4,     5,     4,
       3,     4,     3,     2,     3,     4,     3,     2,     1,     2,
       3,     4,     5,     4,     3,     4,     3,     2,     3,     4,
       3,     2,     3,     2,     1,     2,     3,     4,     3,     2,
       3,     2,     1,     2,     3,     2,     1,     2,     1,     0,
       5,     5,     0,     2,     6,     5,     7,     8,     4,     6,
       7,     6,     7,     5,     4,     5,     0,     2,     2,     2,
       2,     2,     2,     2,     4,     1,     5,     5,     9,     8,
       9,     0,     2,     4,     0,     1,     0,     2,     1
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
      yyerror (&yylloc, scanner, parser, YY_("syntax error: cannot back up")); \
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
#if YYDEBUG

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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

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
                  Type, Value, Location, scanner, parser); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmScanner scanner, WasmParser* parser)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmScanner scanner, WasmParser* parser)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, parser);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, WasmScanner scanner, WasmParser* parser)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, parser);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, parser); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, WasmScanner scanner, WasmParser* parser)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (parser);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (WasmScanner scanner, WasmParser* parser)
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
      yychar = yylex (&yylval, &yylloc, scanner, parser);
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
#line 131 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.types)); }
#line 1764 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 132 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); *wasm_append_type(&(yyval.types)) = (yyvsp[0].type); }
#line 1770 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 135 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); }
#line 1776 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 136 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 1785 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 140 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 1794 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 144 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 1800 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 151 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 1806 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 152 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 1812 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 156 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &index, 0))
        yyerror(&(yylsp[0]), scanner, parser, "invalid int %.*s", (yyvsp[0].text).length, (yyvsp[0].text).start);
      (yyval.var).index = index;
    }
#line 1825 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 164 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 1835 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 171 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.vars)); }
#line 1841 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 172 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); *wasm_append_var(&(yyval.vars)) = (yyvsp[0].var); }
#line 1847 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 175 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 1853 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 179 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 1859 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 183 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPQUOTEDTEXT((yyval.text), (yyvsp[0].text)); }
#line 1865 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 187 "src/wasm-parser.y" /* yacc.c:1646  */
    { dup_string_contents(&(yyvsp[0].text), &(yyval.segment).data, &(yyval.segment).size); }
#line 1871 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 191 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.text)); }
#line 1877 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 192 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 1883 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 196 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = 0; }
#line 1889 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 197 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        yyerror(&(yylsp[0]), scanner, parser, "invalid offset \"%.*s\"", (yyvsp[0].text).length,
                (yyvsp[0].text).start);
    }
#line 1899 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 204 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = WASM_USE_NATURAL_ALIGNMENT; }
#line 1905 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 205 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        yyerror(&(yylsp[0]), scanner, parser, "invalid alignment \"%.*s\"", (yyvsp[0].text).length,
                (yyvsp[0].text).start);
    }
#line 1915 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 213 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 1921 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 216 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_NOP); }
#line 1927 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 217 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      (yyval.expr)->block.label = (yyvsp[0].text);
    }
#line 1936 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 221 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      (yyval.expr)->block.label = (yyvsp[-1].text);
      (yyval.expr)->block.exprs = (yyvsp[0].exprs);
    }
#line 1946 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 226 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF_ELSE);
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[-1].expr);
      (yyval.expr)->if_else.false_ = (yyvsp[0].expr);
    }
#line 1957 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 232 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF);
      (yyval.expr)->if_else.cond = (yyvsp[-1].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[0].expr);
    }
#line 1967 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 237 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR_IF);
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 1977 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 242 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      ZEROMEM((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 1988 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 248 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 1999 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 254 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LABEL);
      (yyval.expr)->label.label = (yyvsp[-1].text);
      (yyval.expr)->label.expr = (yyvsp[0].expr);
    }
#line 2009 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 259 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      (yyval.expr)->br.var.loc = (yylsp[-1]);
      (yyval.expr)->br.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.expr)->br.var.index = 0;
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2021 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 266 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2031 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 271 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_RETURN);
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2040 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 275 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_TABLESWITCH);
      (yyval.expr)->tableswitch.label = (yyvsp[-7].text);
      (yyval.expr)->tableswitch.expr = (yyvsp[-6].expr);
      (yyval.expr)->tableswitch.targets = (yyvsp[-3].targets);
      (yyval.expr)->tableswitch.default_target = (yyvsp[-1].target);
      (yyval.expr)->tableswitch.cases = (yyvsp[0].cases);

      int i;
      for (i = 0; i < (yyval.expr)->tableswitch.cases.size; ++i) {
        WasmCase* case_ = &(yyval.expr)->tableswitch.cases.data[i];
        if (case_->label.start) {
          WasmBinding* binding =
              wasm_append_binding(&(yyval.expr)->tableswitch.case_bindings);
          binding->loc = case_->loc;
          binding->name = case_->label;
          binding->index = i;
        }
      }
    }
#line 2065 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 295 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL);
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2075 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 300 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_IMPORT);
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2085 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 305 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_INDIRECT);
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.args = (yyvsp[0].exprs);
    }
#line 2096 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 311 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GET_LOCAL);
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2105 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 315 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SET_LOCAL);
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2115 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 320 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD);
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u32);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2127 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 327 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE);
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u32);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2140 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 335 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD_EXTEND);
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u32);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2152 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 342 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE_WRAP);
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u32);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2165 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 350 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONST);
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (!read_const((yyvsp[-1].type), (yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.expr)->const_))
        yyerror(&(yylsp[0]), scanner, parser, "invalid literal \"%.*s\"", (yyvsp[0].text).length,
                (yyvsp[0].text).start);
      free((char*)(yyvsp[0].text).start);
    }
#line 2178 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 358 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNARY);
      (yyval.expr)->unary.op = (yyvsp[-1].unary);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2188 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 363 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BINARY);
      (yyval.expr)->binary.op = (yyvsp[-2].binary);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2199 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 369 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SELECT);
      (yyval.expr)->select.type = (yyvsp[-3].type);
      (yyval.expr)->select.cond = (yyvsp[-2].expr);
      (yyval.expr)->select.true_ = (yyvsp[-1].expr);
      (yyval.expr)->select.false_ = (yyvsp[0].expr);
    }
#line 2211 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 376 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_COMPARE);
      (yyval.expr)->compare.op = (yyvsp[-2].compare);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2222 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 382 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONVERT);
      (yyval.expr)->convert.op = (yyvsp[-1].convert);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2232 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 387 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CAST);
      (yyval.expr)->cast.op = (yyvsp[-1].cast);
      (yyval.expr)->cast.expr = (yyvsp[0].expr);
    }
#line 2242 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 392 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNREACHABLE); }
#line 2248 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 393 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_MEMORY_SIZE); }
#line 2254 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 394 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GROW_MEMORY);
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2263 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 398 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_HAS_FEATURE);
      (yyval.expr)->has_feature.text = (yyvsp[0].text);
    }
#line 2272 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 402 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD_GLOBAL);
      (yyval.expr)->load_global.var = (yyvsp[0].var);
    }
#line 2281 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 406 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE_GLOBAL);
      (yyval.expr)->store_global.var = (yyvsp[-1].var);
      (yyval.expr)->store_global.expr = (yyvsp[0].expr);
    }
#line 2291 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 411 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_PAGE_SIZE); }
#line 2297 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 414 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2303 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 418 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.exprs)); *wasm_append_expr_ptr(&(yyval.exprs)) = (yyvsp[0].expr); }
#line 2309 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 419 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.exprs) = (yyvsp[-1].exprs); *wasm_append_expr_ptr(&(yyval.exprs)) = (yyvsp[0].expr); }
#line 2315 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 422 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.exprs)); }
#line 2321 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 427 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_CASE;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2330 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 431 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_BR;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2339 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 437 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.targets)); }
#line 2345 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 438 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.targets) = (yyvsp[-1].targets); *wasm_append_target(&(yyval.targets)) = (yyvsp[0].target); }
#line 2351 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 441 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.case_).label); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2357 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 442 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.case_).label = (yyvsp[-2].text); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2363 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 445 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.cases)); }
#line 2369 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 446 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.cases) = (yyvsp[-1].cases); *wasm_append_case(&(yyval.cases)) = (yyvsp[0].case_); }
#line 2375 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 453 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2385 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 458 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      *wasm_append_type(&(yyval.type_bindings).types) = (yyvsp[-1].type);
    }
#line 2398 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 466 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2408 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 471 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      *wasm_append_type(&(yyval.type_bindings).types) = (yyvsp[-1].type);
    }
#line 2421 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 481 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 2427 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 484 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2437 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 489 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      *wasm_append_type(&(yyval.type_bindings).types) = (yyvsp[-1].type);
    }
#line 2450 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 497 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2460 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 502 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      *wasm_append_type(&(yyval.type_bindings).types) = (yyvsp[-1].type);
    }
#line 2473 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 512 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2479 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 515 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
    }
#line 2487 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 518 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[0].text);
    }
#line 2496 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 522 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 2506 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 527 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 2517 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 533 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2529 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 540 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2542 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 548 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-5].text);
      (yyval.func).type_var = (yyvsp[-4].var);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2556 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 557 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2569 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 565 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2581 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 572 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
    }
#line 2593 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 579 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2605 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 586 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2616 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 592 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2628 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 599 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2641 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 607 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2653 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 614 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2664 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 620 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2676 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 627 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2687 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 633 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2697 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 638 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2708 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 644 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 2718 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 649 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2729 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 655 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2741 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 662 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2754 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 670 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2766 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 677 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2777 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 683 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2789 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 690 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2800 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 696 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2810 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 701 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2821 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 707 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2833 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 714 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2844 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 720 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2854 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 725 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 2863 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 729 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 2873 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 734 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2884 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 740 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2896 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 747 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-4].var);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2909 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 755 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2921 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 762 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2932 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 768 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2944 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 775 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2955 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 781 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2965 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 786 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2976 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 792 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2988 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 799 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2999 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 805 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3009 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 810 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3020 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 816 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3030 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 821 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3039 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 825 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3049 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 830 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3060 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 836 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3072 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 843 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3083 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 849 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3093 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 854 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3104 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 860 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3114 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 865 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3123 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 869 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3133 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 874 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3144 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 880 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3154 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 885 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3163 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 889 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3173 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 894 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3182 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 900 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func)); }
#line 3188 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 900 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.func) = (yyvsp[-1].func); (yyval.func).loc = (yylsp[-3]); }
#line 3194 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 906 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.segment).addr, 0))
        yyerror(&(yylsp[-2]), scanner, parser, "invalid memory segment address \"%.*s\"",
                (yyvsp[-2].text).length, (yyvsp[-2].text).start);
    }
#line 3207 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 916 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.segments)); }
#line 3213 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 917 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.segments) = (yyvsp[-1].segments); *wasm_append_segment(&(yyval.segments)) = (yyvsp[0].segment); }
#line 3219 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 921 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      if (!read_int32((yyvsp[-3].text).start, (yyvsp[-3].text).start + (yyvsp[-3].text).length, &(yyval.memory).initial_size, 0))
        yyerror(&(yylsp[-3]), scanner, parser, "invalid initial memory size \"%.*s\"",
                (yyvsp[-3].text).length, (yyvsp[-3].text).start);
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.memory).max_size, 0))
        yyerror(&(yylsp[-2]), scanner, parser, "invalid max memory size \"%.*s\"",
                (yyvsp[-2].text).length, (yyvsp[-2].text).start);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3234 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 931 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.memory).initial_size, 0))
        yyerror(&(yylsp[-2]), scanner, parser, "invalid initial memory size \"%.*s\"",
                (yyvsp[-2].text).length, (yyvsp[-2].text).start);
      (yyval.memory).max_size = (yyval.memory).initial_size;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3247 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 942 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3256 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 946 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3265 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 953 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 3271 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 957 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3283 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 964 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3296 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 972 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3308 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 979 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3321 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 990 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.export).name = (yyvsp[-2].text);
      (yyval.export).var = (yyvsp[-1].var);
    }
#line 3330 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 997 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      (yyval.type_bindings).types = (yyvsp[-1].types);
    }
#line 3339 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1001 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = 0;
      *wasm_append_type(&(yyval.type_bindings).types) = (yyvsp[-1].type);
    }
#line 3352 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1012 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.module_fields)); }
#line 3358 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1013 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = (yyvsp[0].func);
    }
#line 3370 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1020 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = (yyvsp[0].import);
    }
#line 3382 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1027 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export = (yyvsp[0].export);
    }
#line 3394 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1034 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
    }
#line 3406 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1041 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);
    }
#line 3418 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1048 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
    }
#line 3430 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1055 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;
      field->global = (yyvsp[0].type_bindings);
    }
#line 3442 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1064 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module).loc = (yylsp[-2]);
      (yyval.module).fields = (yyvsp[-1].module_fields);

      /* cache values */
      int i;
      for (i = 0; i < (yyval.module).fields.size; ++i) {
        WasmModuleField* field = &(yyval.module).fields.data[i];
        switch (field->type) {
          case WASM_MODULE_FIELD_TYPE_FUNC:
            *wasm_append_func_ptr(&(yyval.module).funcs) = &field->func;
            if (field->func.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).func_bindings);
              binding->loc = field->loc;
              binding->name = field->func.name;
              binding->index = (yyval.module).funcs.size - 1;
            }
            break;
          case WASM_MODULE_FIELD_TYPE_IMPORT:
            *wasm_append_import_ptr(&(yyval.module).imports) = &field->import;
            if (field->import.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).import_bindings);
              binding->loc = field->loc;
              binding->name = field->import.name;
              binding->index = (yyval.module).imports.size - 1;
            }
            break;
          case WASM_MODULE_FIELD_TYPE_EXPORT:
            *wasm_append_export_ptr(&(yyval.module).exports) = &field->export;
            if (field->export.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).export_bindings);
              binding->loc = field->loc;
              binding->name = field->export.name;
              binding->index = (yyval.module).exports.size - 1;
            }
            break;
          case WASM_MODULE_FIELD_TYPE_TABLE:
            (yyval.module).table = &field->table;
            break;
          case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
            *wasm_append_func_type_ptr(&(yyval.module).func_types) = &field->func_type;
            if (field->func_type.name.start) {
              WasmBinding* binding =
                  wasm_append_binding(&(yyval.module).func_type_bindings);
              binding->loc = field->loc;
              binding->name = field->func_type.name;
              binding->index = (yyval.module).func_types.size - 1;
            }
            break;
          case WASM_MODULE_FIELD_TYPE_MEMORY:
            (yyval.module).memory = &field->memory;
            break;
          case WASM_MODULE_FIELD_TYPE_GLOBAL:
            wasm_extend_type_bindings(&(yyval.module).globals, &field->global);
            break;
        }
      }
    }
#line 3505 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1128 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.command).type = WASM_COMMAND_TYPE_MODULE; (yyval.command).module = (yyvsp[0].module); }
#line 3511 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1129 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command).invoke.loc = (yylsp[-3]);
      (yyval.command).invoke.name = (yyvsp[-2].text);
      (yyval.command).invoke.args = (yyvsp[-1].consts);
    }
#line 3522 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1135 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command).assert_invalid.module = (yyvsp[-2].module);
      (yyval.command).assert_invalid.text = (yyvsp[-1].text);
    }
#line 3532 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1140 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command).assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_return.expected = (yyvsp[-1].const_);
    }
#line 3544 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1147 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command).assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command).assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command).assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3555 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1153 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command).assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_trap.text = (yyvsp[-1].text);
    }
#line 3567 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1162 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.commands)); }
#line 3573 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1163 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.commands) = (yyvsp[-1].commands); *wasm_append_command(&(yyval.commands)) = (yyvsp[0].command); }
#line 3579 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1167 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (!read_const((yyvsp[-2].type), (yyvsp[-1].text).start, (yyvsp[-1].text).start + (yyvsp[-1].text).length, &(yyval.const_)))
        yyerror(&(yylsp[-1]), scanner, parser, "invalid literal \"%.*s\"",
                (yyvsp[-1].text).length, (yyvsp[-1].text).start);
      free((char*)(yyvsp[-1].text).start);
    }
#line 3591 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1176 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3597 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1180 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.consts)); }
#line 3603 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1181 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = (yyvsp[-1].consts); *wasm_append_const(&(yyval.consts)) = (yyvsp[0].const_); }
#line 3609 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1185 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.script).commands = (yyvsp[0].commands); parser->script = (yyval.script); }
#line 3615 "src/wasm-parser.c" /* yacc.c:1646  */
    break;


#line 3619 "src/wasm-parser.c" /* yacc.c:1646  */
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
      yyerror (&yylloc, scanner, parser, YY_("syntax error"));
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
        yyerror (&yylloc, scanner, parser, yymsgp);
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
                      yytoken, &yylval, &yylloc, scanner, parser);
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
                  yystos[yystate], yyvsp, yylsp, scanner, parser);
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
  yyerror (&yylloc, scanner, parser, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, scanner, parser);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, parser);
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
#line 1188 "src/wasm-parser.y" /* yacc.c:1906  */


DEFINE_VECTOR(type, WasmType)
DEFINE_VECTOR(var, WasmVar);
DEFINE_VECTOR(expr_ptr, WasmExprPtr);
DEFINE_VECTOR(target, WasmTarget);
DEFINE_VECTOR(case, WasmCase);
DEFINE_VECTOR(binding, WasmBinding);
DEFINE_VECTOR(func_ptr, WasmFuncPtr);
DEFINE_VECTOR(segment, WasmSegment);
DEFINE_VECTOR(func_type_ptr, WasmFuncTypePtr);
DEFINE_VECTOR(import_ptr, WasmImportPtr);
DEFINE_VECTOR(export_ptr, WasmExportPtr);
DEFINE_VECTOR(module_field, WasmModuleField);
DEFINE_VECTOR(const, WasmConst);
DEFINE_VECTOR(command, WasmCommand);

void yyerror(WasmLocation* loc,
             WasmScanner scanner,
             WasmParser* parser,
             const char* fmt,
             ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s:%d:%d: ", loc->filename, loc->first_line,
          loc->first_column);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  parser->errors++;
}

static int hexdigit(char c, uint32_t* out) {
  if ((unsigned int)(c - '0') <= 9) {
    *out = c - '0';
    return 1;
  } else if ((unsigned int)(c - 'a') <= 6) {
    *out = 10 + (c - 'a');
    return 1;
  } else if ((unsigned int)(c - 'A') <= 6) {
    *out = 10 + (c - 'A');
    return 1;
  }
  return 0;
}

/* return 1 if the non-NULL-terminated string starting with |start| and ending
 with |end| starts with the NULL-terminated string |prefix|. */
static int string_starts_with(const char* start,
                              const char* end,
                              const char* prefix) {
  while (start < end && *prefix) {
    if (*start != *prefix)
      return 0;
    start++;
    prefix++;
  }
  return *prefix == 0;
}

static int read_uint64(const char* s, const char* end, uint64_t* out) {
  if (s == end)
    return 0;
  uint64_t value = 0;
  if (*s == '0' && s + 1 < end && s[1] == 'x') {
    s += 2;
    if (s == end)
      return 0;
    for (; s < end; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      uint64_t old_value = value;
      value = value * 16 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  } else {
    for (; s < end; ++s) {
      uint32_t digit = (*s - '0');
      if (digit > 9)
        return 0;
      uint64_t old_value = value;
      value = value * 10 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  }
  if (s != end)
    return 0;
  *out = value;
  return 1;
}

static int read_int64(const char* s, const char* end, uint64_t* out) {
  int has_sign = 0;
  if (*s == '-') {
    has_sign = 1;
    s++;
  }
  uint64_t value;
  int result = read_uint64(s, end, &value);
  if (has_sign) {
    if (value > (uint64_t)INT64_MAX + 1) /* abs(INT64_MIN) == INT64_MAX + 1 */
      return 0;
    value = UINT64_MAX - value + 1;
  }
  *out = value;
  return result;
}

static int read_int32(const char* s,
                      const char* end,
                      uint32_t* out,
                      int allow_signed) {
  uint64_t value;
  int has_sign = 0;
  if (*s == '-') {
    if (!allow_signed)
      return 0;
    has_sign = 1;
    s++;
  }
  if (!read_uint64(s, end, &value))
    return 0;

  if (has_sign) {
    if (value > (uint64_t)INT32_MAX + 1) /* abs(INT32_MIN) == INT32_MAX + 1 */
      return 0;
    value = UINT32_MAX - value + 1;
  } else {
    if (value > (uint64_t)UINT32_MAX)
      return 0;
  }
  *out = (uint32_t)value;
  return 1;
}

static int read_float_nan(const char* s, const char* end, float* out) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  if (!string_starts_with(s, end, "nan"))
    return 0;
  s += 3;

  uint32_t tag;
  if (s != end) {
    tag = 0;
    if (!string_starts_with(s, end, ":0x"))
      return 0;
    s += 3;

    for (; s < end; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      const uint32_t max_tag = 0x7fffff;
      if (tag > max_tag)
        return 0;
    }

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x400000;
  }

  uint32_t bits = 0x7f800000 | tag;
  if (is_neg)
    bits |= 0x80000000U;
  memcpy(out, &bits, sizeof(*out));
  return 1;
}

static int read_float(const char* s, const char* end, float* out) {
  if (read_float_nan(s, end, out))
    return 1;

  errno = 0;
  char* endptr;
  float value;
  value = strtof(s, &endptr);
  if (endptr != end ||
      ((value == 0 || value == HUGE_VALF || value == -HUGE_VALF) && errno != 0))
    return 0;

  *out = value;
  return 1;
}

static int read_double_nan(const char* s, const char* end, double* out) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  if (!string_starts_with(s, end, "nan"))
    return 0;
  s += 3;

  uint64_t tag;
  if (s != end) {
    tag = 0;
    if (!string_starts_with(s, end, ":0x"))
      return 0;
    s += 3;

    for (; s < end; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      const uint64_t max_tag = 0xfffffffffffffULL;
      if (tag > max_tag)
        return 0;
    }

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x8000000000000ULL;
  }

  uint64_t bits = 0x7ff0000000000000ULL | tag;
  if (is_neg)
    bits |= 0x8000000000000000ULL;
  memcpy(out, &bits, sizeof(*out));
  return 1;
}

static int read_double(const char* s, const char* end, double* out) {
  if (read_double_nan(s, end, out))
    return 1;

  errno = 0;
  char* endptr;
  double value;
  value = strtod(s, &endptr);
  if (endptr != end ||
      ((value == 0 || value == HUGE_VAL || value == -HUGE_VAL) && errno != 0))
    return 0;

  *out = value;
  return 1;
}

static int read_const(WasmType type,
                      const char* s,
                      const char* end,
                      WasmConst* out) {
  out->type = type;
  switch (type) {
    case WASM_TYPE_I32:
      return read_int32(s, end, &out->u32, 1);
    case WASM_TYPE_I64:
      return read_int64(s, end, &out->u64);
    case WASM_TYPE_F32:
      return read_float(s, end, &out->f32);
    case WASM_TYPE_F64:
      return read_double(s, end, &out->f64);
    default:
      assert(0);
      break;
  }
  return 0;
}

static size_t copy_string_contents(WasmStringSlice* text,
                                   char* dest,
                                   size_t size) {
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
          /* The string should be validated already, so we know this is a hex
           * sequence */
          uint32_t hi;
          uint32_t lo;
          if (!hexdigit(src[0], &hi) || !hexdigit(src[1], &lo))
            assert(0);
          *dest++ = (hi << 4) | lo;
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

static void dup_string_contents(WasmStringSlice* text,
                                void** out_data,
                                size_t* out_size) {
  const char* src = text->start + 1;
  const char* end = text->start + text->length - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src;
  char* result = malloc(size);
  size_t actual_size = copy_string_contents(text, result, size);
  *out_data = result;
  *out_size = actual_size;
}
