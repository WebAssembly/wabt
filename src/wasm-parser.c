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
#define YYSTYPE         WASM_STYPE
#define YYLTYPE         WASM_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         wasm_parse
#define yylex           wasm_lex
#define yyerror         wasm_error
#define yydebug         wasm_debug
#define yynerrs         wasm_nerrs


/* Copy the first part of user declarations.  */
#line 17 "src/wasm-parser.y" /* yacc.c:339  */

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

#define CHECK_ALLOC_(cond)                                  \
  if ((cond)) {                                             \
    WasmLocation loc;                                       \
    loc.filename = __FILE__;                                \
    loc.first_line = loc.last_line = __LINE__;              \
    loc.first_column = loc.last_column = 0;                 \
    wasm_error(&loc, scanner, parser, "allocation failed"); \
    YYERROR;                                                \
  }

#define CHECK_ALLOC(e) CHECK_ALLOC_((e) != WASM_OK)
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_(!(v))
#define CHECK_ALLOC_STR(s) CHECK_ALLOC_(!(s).start)

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
  if (!result)
    return NULL;
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
static WasmResult dup_string_contents(WasmStringSlice * text, void** out_data,
                                      size_t* out_size);


#line 147 "src/wasm-parser.c" /* yacc.c:339  */

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
#ifndef YY_WASM_SRC_WASM_PARSER_H_INCLUDED
# define YY_WASM_SRC_WASM_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef WASM_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define WASM_DEBUG 1
#  else
#   define WASM_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define WASM_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined WASM_DEBUG */
#if WASM_DEBUG
extern int wasm_debug;
#endif

/* Token type.  */
#ifndef WASM_TOKENTYPE
# define WASM_TOKENTYPE
  enum wasm_tokentype
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
    WASM_TOKEN_TYPE_BR = 270,
    WASM_TOKEN_TYPE_BR_IF = 271,
    WASM_TOKEN_TYPE_TABLESWITCH = 272,
    WASM_TOKEN_TYPE_CASE = 273,
    WASM_TOKEN_TYPE_CALL = 274,
    WASM_TOKEN_TYPE_CALL_IMPORT = 275,
    WASM_TOKEN_TYPE_CALL_INDIRECT = 276,
    WASM_TOKEN_TYPE_RETURN = 277,
    WASM_TOKEN_TYPE_GET_LOCAL = 278,
    WASM_TOKEN_TYPE_SET_LOCAL = 279,
    WASM_TOKEN_TYPE_LOAD = 280,
    WASM_TOKEN_TYPE_STORE = 281,
    WASM_TOKEN_TYPE_OFFSET = 282,
    WASM_TOKEN_TYPE_ALIGN = 283,
    WASM_TOKEN_TYPE_CONST = 284,
    WASM_TOKEN_TYPE_UNARY = 285,
    WASM_TOKEN_TYPE_BINARY = 286,
    WASM_TOKEN_TYPE_COMPARE = 287,
    WASM_TOKEN_TYPE_CONVERT = 288,
    WASM_TOKEN_TYPE_CAST = 289,
    WASM_TOKEN_TYPE_SELECT = 290,
    WASM_TOKEN_TYPE_FUNC = 291,
    WASM_TOKEN_TYPE_TYPE = 292,
    WASM_TOKEN_TYPE_PARAM = 293,
    WASM_TOKEN_TYPE_RESULT = 294,
    WASM_TOKEN_TYPE_LOCAL = 295,
    WASM_TOKEN_TYPE_MODULE = 296,
    WASM_TOKEN_TYPE_MEMORY = 297,
    WASM_TOKEN_TYPE_SEGMENT = 298,
    WASM_TOKEN_TYPE_IMPORT = 299,
    WASM_TOKEN_TYPE_EXPORT = 300,
    WASM_TOKEN_TYPE_TABLE = 301,
    WASM_TOKEN_TYPE_UNREACHABLE = 302,
    WASM_TOKEN_TYPE_MEMORY_SIZE = 303,
    WASM_TOKEN_TYPE_GROW_MEMORY = 304,
    WASM_TOKEN_TYPE_HAS_FEATURE = 305,
    WASM_TOKEN_TYPE_ASSERT_INVALID = 306,
    WASM_TOKEN_TYPE_ASSERT_RETURN = 307,
    WASM_TOKEN_TYPE_ASSERT_RETURN_NAN = 308,
    WASM_TOKEN_TYPE_ASSERT_TRAP = 309,
    WASM_TOKEN_TYPE_INVOKE = 310,
    WASM_TOKEN_TYPE_GLOBAL = 311,
    WASM_TOKEN_TYPE_LOAD_GLOBAL = 312,
    WASM_TOKEN_TYPE_STORE_GLOBAL = 313,
    WASM_TOKEN_TYPE_LOW = 314
  };
#endif

/* Value type.  */
#if ! defined WASM_STYPE && ! defined WASM_STYPE_IS_DECLARED
typedef WasmToken WASM_STYPE;
# define WASM_STYPE_IS_TRIVIAL 1
# define WASM_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined WASM_LTYPE && ! defined WASM_LTYPE_IS_DECLARED
typedef struct WASM_LTYPE WASM_LTYPE;
struct WASM_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define WASM_LTYPE_IS_DECLARED 1
# define WASM_LTYPE_IS_TRIVIAL 1
#endif



int wasm_parse (WasmScanner scanner, WasmParser* parser);

#endif /* !YY_WASM_SRC_WASM_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 280 "src/wasm-parser.c" /* yacc.c:358  */

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
         || (defined WASM_LTYPE_IS_TRIVIAL && WASM_LTYPE_IS_TRIVIAL \
             && defined WASM_STYPE_IS_TRIVIAL && WASM_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   673

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  60
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  188
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  350

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   314

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
      55,    56,    57,    58,    59
};

#if WASM_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   189,   189,   190,   196,   197,   201,   205,   212,   213,
     217,   226,   234,   235,   238,   242,   246,   250,   254,   255,
     259,   260,   267,   268,   276,   279,   283,   288,   294,   301,
     307,   313,   320,   327,   334,   342,   348,   353,   375,   381,
     387,   394,   399,   405,   413,   422,   431,   437,   444,   452,
     459,   465,   471,   475,   479,   484,   489,   494,   502,   503,
     506,   510,   516,   517,   521,   525,   531,   532,   538,   539,
     542,   543,   553,   558,   567,   572,   583,   586,   591,   600,
     605,   616,   619,   623,   628,   634,   641,   649,   658,   668,
     677,   685,   694,   702,   709,   717,   726,   734,   741,   749,
     756,   762,   769,   775,   782,   790,   799,   807,   814,   822,
     829,   835,   842,   850,   857,   863,   868,   874,   881,   889,
     898,   906,   913,   921,   928,   934,   941,   949,   956,   962,
     969,   975,   980,   986,   993,  1001,  1008,  1014,  1021,  1027,
    1032,  1038,  1045,  1051,  1056,  1062,  1069,  1075,  1084,  1092,
    1093,  1100,  1108,  1116,  1122,  1131,  1135,  1142,  1146,  1153,
    1161,  1168,  1179,  1186,  1190,  1202,  1203,  1211,  1219,  1227,
    1235,  1243,  1251,  1261,  1339,  1340,  1346,  1351,  1358,  1364,
    1373,  1374,  1381,  1390,  1391,  1394,  1395,  1402,  1408
};
#endif

#if WASM_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "INT", "FLOAT",
  "TEXT", "VAR", "VALUE_TYPE", "NOP", "BLOCK", "IF", "IF_ELSE", "LOOP",
  "BR", "BR_IF", "TABLESWITCH", "CASE", "CALL", "CALL_IMPORT",
  "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "LOAD", "STORE",
  "OFFSET", "ALIGN", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT",
  "CAST", "SELECT", "FUNC", "TYPE", "PARAM", "RESULT", "LOCAL", "MODULE",
  "MEMORY", "SEGMENT", "IMPORT", "EXPORT", "TABLE", "UNREACHABLE",
  "MEMORY_SIZE", "GROW_MEMORY", "HAS_FEATURE", "ASSERT_INVALID",
  "ASSERT_RETURN", "ASSERT_RETURN_NAN", "ASSERT_TRAP", "INVOKE", "GLOBAL",
  "LOAD_GLOBAL", "STORE_GLOBAL", "LOW", "$accept", "value_type_list",
  "func_type", "literal", "var", "var_list", "bind_var", "text",
  "quoted_text", "string_contents", "labeling", "offset", "align", "expr",
  "expr1", "expr_opt", "non_empty_expr_list", "expr_list", "target",
  "target_list", "case", "case_list", "param_list", "result", "local_list",
  "type_use", "func_info", "func", "segment_address", "segment",
  "segment_list", "initial_size", "max_size", "memory", "type_def",
  "table", "import", "export", "global", "module_fields", "module", "cmd",
  "cmd_list", "const", "const_opt", "const_list", "script", "start", YY_NULLPTR
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
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314
};
# endif

#define YYPACT_NINF -153

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-153)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -153,     5,  -153,    16,    77,  -153,  -153,  -153,  -153,    32,
      48,    81,    83,    37,    23,    39,    37,    66,    72,    93,
    -153,  -153,    80,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,   134,    37,    37,    37,    98,    20,    21,   140,    61,
      37,  -153,   163,  -153,  -153,  -153,  -153,   138,  -153,  -153,
     329,  -153,   171,  -153,   174,   177,   179,   182,   200,   201,
     168,   203,  -153,   202,    37,    37,    62,   106,    34,   199,
     109,   152,   159,   160,  -153,   163,   174,   174,   163,     4,
     174,   163,    62,    62,    62,   174,    62,    62,   184,   184,
     160,   174,   174,   174,   174,   174,   174,    62,   163,   204,
     163,  -153,  -153,   174,   208,    62,    62,   212,   174,   177,
     179,   182,   200,   615,  -153,   406,   174,   179,   182,   517,
     174,   182,   566,   174,   468,   174,   177,   179,   182,  -153,
     206,   183,  -153,   192,  -153,    37,   215,  -153,  -153,   216,
    -153,  -153,  -153,  -153,   217,   219,   222,    37,  -153,  -153,
     223,  -153,   174,   174,   174,   163,   174,   174,  -153,  -153,
      62,   174,   174,   174,   174,  -153,  -153,   174,  -153,   195,
     195,  -153,  -153,   174,   174,  -153,  -153,   174,   225,    41,
     221,   227,    54,   224,  -153,  -153,  -153,  -153,   174,  -153,
     174,   179,   182,   174,   182,   174,   174,   177,   179,   182,
     163,   174,   182,   174,   174,   163,   174,   179,   182,   174,
     182,   174,   161,   228,   206,   191,  -153,  -153,   194,   215,
     113,   231,   233,  -153,  -153,  -153,   240,  -153,   242,  -153,
     174,  -153,   174,   174,   174,  -153,  -153,   174,   244,  -153,
    -153,   174,  -153,  -153,   174,   174,  -153,  -153,   174,  -153,
    -153,   245,  -153,  -153,   246,  -153,   174,   182,   174,   174,
     174,   179,   182,   174,   182,   174,    57,   243,   174,    88,
     247,   174,   182,   174,   174,  -153,   249,   251,   255,   259,
    -153,   261,   262,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
     226,  -153,  -153,   174,  -153,  -153,  -153,   174,   174,   182,
     174,   174,  -153,   265,  -153,   266,   174,    90,   267,  -153,
     270,  -153,   268,  -153,  -153,  -153,  -153,   174,  -153,  -153,
     273,  -153,  -153,  -153,   275,   198,   248,  -153,   124,   279,
    -153,   274,    62,    62,  -153,   280,   281,   284,   286,  -153,
    -153,  -153,   272,  -153,   132,   174,   287,   288,  -153,  -153
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     180,   187,   188,     0,     0,   174,   181,     1,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      16,   185,     0,   173,   166,   171,   170,   169,   167,   168,
     172,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    12,     2,   176,   185,   185,   185,     0,   175,   186,
       0,    14,    83,    60,   145,   131,   139,   143,   115,     0,
       0,     0,   151,   149,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    18,     0,     0,    18,    58,
       0,    18,     0,     0,     0,    58,     0,     0,    20,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     2,     0,
       2,    52,    53,     0,     0,     0,     0,     0,   114,   102,
     110,   100,    84,     0,    61,     0,   138,   132,   136,     0,
     142,   140,     0,   144,     0,   130,   116,   124,   128,   146,
       4,     0,   152,     0,   149,     0,     4,    10,    11,     0,
     157,    13,   163,     3,     0,   183,     0,     0,     8,     9,
       0,    19,    26,     0,     0,    19,    62,    58,    59,    34,
       0,     0,    62,    62,     0,    36,    41,     0,    21,    22,
      22,    45,    46,     0,     0,    50,    51,     0,     0,     0,
       0,     0,     0,     0,    54,    15,    55,    56,     0,    24,
     109,   103,   107,   113,   111,   101,    99,    85,    93,    97,
       2,   135,   133,   137,   141,     2,   123,   117,   121,   127,
     125,   129,     0,     0,     4,     0,   154,   150,     0,     4,
       0,     0,     0,   162,   164,   184,     0,   178,     0,   182,
      27,    29,     0,    62,    63,    32,    35,    30,     0,    38,
      39,    62,    42,    23,     0,     0,    47,    49,     0,    81,
      72,     0,    76,    77,     0,    57,   106,   104,   108,   112,
      92,    86,    90,    96,    94,    98,     0,     0,   134,     0,
       0,   120,   118,   122,   126,     2,     0,     0,     0,     0,
     153,     0,     0,   160,   158,   177,   179,    28,    33,    31,
       0,    40,    43,     0,    48,    73,    78,   105,    89,    87,
      91,    95,    74,     0,    79,     0,   119,     0,     0,   155,
       0,   147,     0,   161,   159,    66,    44,    88,    75,    80,
       5,     7,   156,    17,     0,     0,     0,   148,     0,     0,
      67,     0,     0,     0,    70,     0,     0,     0,    37,     6,
      65,    64,     0,    71,    62,    62,     0,     0,    68,    69
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -153,   -85,  -132,   209,   -65,  -153,   -36,  -153,    14,  -153,
      65,   207,   123,   -20,  -153,   -80,   205,  -152,   -35,  -153,
    -153,  -153,   -27,    -3,   -19,   -32,  -153,  -153,  -153,  -153,
     164,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
     291,  -153,  -153,   150,  -153,   114,  -153,  -153
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    68,   213,   150,   139,    67,   151,   186,    21,   324,
     152,   169,   244,    53,   107,   159,   234,   235,   330,   325,
     343,   338,    55,    56,    57,    58,    59,    24,   312,   217,
     133,    63,   134,    25,    26,    27,    28,    29,    30,    14,
       5,     6,     1,    49,   226,    35,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      52,    61,   141,    64,   221,   165,    69,   113,     4,   137,
     239,   240,   138,   179,   157,   182,     7,   162,   163,   164,
     112,   166,   167,    50,    60,   109,    22,    23,    51,    51,
      31,   126,   178,   111,   114,    15,   118,   121,   142,   128,
     187,   188,   155,   143,    20,   250,    44,    45,    46,   110,
     143,    17,   117,    65,    66,   127,   153,   154,   253,   158,
     160,   302,   180,   143,   183,   158,   143,   137,    20,    51,
     138,   172,   173,   174,   175,   176,   177,   236,   135,   136,
       8,   288,   278,   184,    18,   197,    19,   281,   114,   291,
     192,   194,   304,   199,   320,   237,   114,   143,   202,   143,
     114,    47,    48,   114,   222,   114,   191,   208,   210,   198,
     140,   137,    47,   145,   138,   266,    36,    37,     8,   233,
     269,    32,    38,   207,    39,    40,    41,    33,     9,    10,
      11,    12,    13,   231,   232,   113,    42,   158,    43,   332,
      51,   238,   333,   156,   241,    62,   161,   242,    34,   219,
      97,   275,   276,   246,   247,    47,   146,   248,    70,    71,
      72,   228,    47,   147,   267,   148,   149,    73,   255,   270,
     114,    51,   257,   114,    50,   114,   114,   113,   262,   264,
     115,   114,   119,   114,   114,   122,   114,   282,   272,   114,
     307,   114,   346,   347,   261,   215,   216,   215,   280,   275,
     276,   328,   329,   124,   130,   129,   131,   132,   144,   212,
     114,   168,   287,   181,   114,   185,   189,   289,   220,   214,
     223,   224,    47,   243,   292,   293,   227,   229,   294,   249,
     251,   252,   277,   254,   279,   283,   114,   284,   114,   114,
     114,    54,   299,   114,   285,   114,   286,   290,   114,   295,
     296,   114,   303,   114,   114,   309,   305,   108,   308,   310,
     116,   120,   123,   125,   311,   313,   314,   336,   337,   318,
     319,   321,   315,   316,   322,   323,   326,   114,   114,   327,
     114,   114,   328,   335,   339,   340,   114,   331,   341,   342,
     344,   348,   349,   245,   334,   225,   170,   114,   218,   171,
      16,     0,     0,     0,     0,     0,     0,     0,   345,     0,
       0,     0,     0,     0,   190,   193,   195,   196,     0,     0,
       0,     0,   201,   203,     0,     0,   204,     0,     0,     0,
       0,   206,   209,   211,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,     0,    82,    83,
      84,    85,    86,    87,    88,    89,     0,   230,    90,    91,
      92,    93,    94,    95,    96,     0,    97,    98,    99,   100,
       0,     0,     0,     0,     0,     0,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,   105,   106,     0,     0,
       0,     0,     0,     0,     0,     0,   256,   258,     0,   259,
       0,     0,   260,   263,   265,     0,     0,   268,     0,     0,
       0,     0,   271,   273,     0,   274,    74,    75,    76,    77,
      78,    79,    80,    81,     0,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,    90,    91,    92,    93,    94,
      95,    96,     0,     0,   200,    99,   100,     0,     0,     0,
       0,     0,     0,   101,   102,   103,   104,     0,     0,     0,
       0,     0,   297,   105,   106,     0,   298,   300,     0,   301,
       0,     0,     0,     0,     0,     0,     0,   306,    74,    75,
      76,    77,    78,    79,    80,    81,     0,    82,    83,    84,
      85,    86,    87,    88,    89,     0,     0,    90,    91,    92,
      93,    94,    95,    96,   317,     0,    98,    99,   100,     0,
       0,     0,     0,     0,     0,   101,   102,   103,   104,     0,
       0,     0,     0,     0,     0,   105,   106,    74,    75,    76,
      77,    78,    79,    80,    81,     0,    82,    83,    84,    85,
      86,    87,    88,    89,     0,     0,    90,    91,    92,    93,
      94,    95,    96,     0,     0,     0,     0,   100,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,     0,     0,
       0,     0,     0,     0,   105,   106,    74,    75,    76,    77,
      78,    79,    80,    81,     0,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,    90,    91,    92,    93,    94,
      95,    96,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,   101,   102,   103,   104,     0,     0,     0,
       0,     0,     0,   105,   106,    74,    75,    76,    77,    78,
      79,    80,    81,     0,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,    90,    91,    92,    93,    94,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,     0,   105,   106
};

static const yytype_int16 yycheck[] =
{
      36,    37,    67,    39,   136,    85,    42,     3,     3,     5,
     162,   163,     8,    98,    79,   100,     0,    82,    83,    84,
      52,    86,    87,     3,     3,    52,     3,     4,     8,     8,
      16,    58,    97,    52,    54,     3,    55,    56,     4,    58,
     105,   106,    78,     9,     7,     4,    32,    33,    34,    52,
       9,     3,    55,    39,    40,    58,    76,    77,     4,    79,
      80,     4,    98,     9,   100,    85,     9,     5,     7,     8,
       8,    91,    92,    93,    94,    95,    96,   157,    64,    65,
      41,   233,   214,   103,     3,   112,     3,   219,   108,   241,
     109,   110,     4,   112,     4,   160,   116,     9,   117,     9,
     120,     3,     4,   123,   136,   125,   109,   126,   127,   112,
       4,     5,     3,     4,     8,   200,    36,    37,    41,   155,
     205,    55,    42,   126,    44,    45,    46,    55,    51,    52,
      53,    54,    55,   153,   154,     3,    56,   157,     4,    15,
       8,   161,    18,    78,   164,     5,    81,   167,    55,   135,
      37,    38,    39,   173,   174,     3,     4,   177,    44,    45,
      46,   147,     3,     4,   200,     5,     6,    29,   188,   205,
     190,     8,   191,   193,     3,   195,   196,     3,   197,   198,
       3,   201,     3,   203,   204,     3,   206,   219,   207,   209,
     275,   211,   344,   345,   197,     3,     4,     3,     4,    38,
      39,     3,     4,     3,    36,     4,     3,     5,     9,     3,
     230,    27,   232,     9,   234,     7,     4,   237,     3,    36,
       4,     4,     3,    28,   244,   245,     4,     4,   248,     4,
       9,     4,     4,     9,    43,     4,   256,     4,   258,   259,
     260,    36,   261,   263,     4,   265,     4,     3,   268,     4,
       4,   271,     9,   273,   274,     4,     9,    52,     9,     4,
      55,    56,    57,    58,     5,     4,     4,   332,   333,     4,
       4,     4,    46,   293,     4,     7,     3,   297,   298,     4,
     300,   301,     3,     9,     4,     4,   306,    39,     4,     3,
      18,     4,     4,   170,   329,   145,    89,   317,   134,    90,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,
      -1,    -1,    -1,    -1,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,   117,   118,    -1,    -1,   121,    -1,    -1,    -1,
      -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   152,    29,    30,
      31,    32,    33,    34,    35,    -1,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,
      -1,    -1,   197,   198,   199,    -1,    -1,   202,    -1,    -1,
      -1,    -1,   207,   208,    -1,   210,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,   257,    57,    58,    -1,   261,   262,    -1,   264,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   272,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,   299,    -1,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    29,    30,    31,    32,
      33,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    29,    30,    31,    32,    33,    34,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    58
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   102,   106,   107,     3,   100,   101,     0,    41,    51,
      52,    53,    54,    55,    99,     3,   100,     3,     3,     3,
       7,    68,     3,     4,    87,    93,    94,    95,    96,    97,
      98,    68,    55,    55,    55,   105,    36,    37,    42,    44,
      45,    46,    56,     4,    68,    68,    68,     3,     4,   103,
       3,     8,    66,    73,    76,    82,    83,    84,    85,    86,
       3,    66,     5,    91,    66,    68,    68,    65,    61,    66,
     105,   105,   105,    29,    10,    11,    12,    13,    14,    15,
      16,    17,    19,    20,    21,    22,    23,    24,    25,    26,
      29,    30,    31,    32,    33,    34,    35,    37,    38,    39,
      40,    47,    48,    49,    50,    57,    58,    74,    76,    82,
      83,    84,    85,     3,    73,     3,    76,    83,    84,     3,
      76,    84,     3,    76,     3,    76,    82,    83,    84,     4,
      36,     3,     5,    90,    92,    68,    68,     5,     8,    64,
       4,    64,     4,     9,     9,     4,     4,     4,     5,     6,
      63,    66,    70,    73,    73,    66,    70,    64,    73,    75,
      73,    70,    64,    64,    64,    75,    64,    64,    27,    71,
      71,    63,    73,    73,    73,    73,    73,    73,    64,    61,
      66,     9,    61,    66,    73,     7,    67,    64,    64,     4,
      76,    83,    84,    76,    84,    76,    76,    82,    83,    84,
      38,    76,    84,    76,    76,    40,    76,    83,    84,    76,
      84,    76,     3,    62,    36,     3,     4,    89,    90,    68,
       3,    62,    85,     4,     4,   103,   104,     4,    68,     4,
      76,    73,    73,    66,    76,    77,    75,    64,    73,    77,
      77,    73,    73,    28,    72,    72,    73,    73,    73,     4,
       4,     9,     4,     4,     9,    73,    76,    84,    76,    76,
      76,    83,    84,    76,    84,    76,    61,    66,    76,    61,
      66,    76,    84,    76,    76,    38,    39,     4,    62,    43,
       4,    62,    85,     4,     4,     4,     4,    73,    77,    73,
       3,    77,    73,    73,    73,     4,     4,    76,    76,    84,
      76,    76,     4,     9,     4,     9,    76,    61,     9,     4,
       4,     5,    88,     4,     4,    46,    73,    76,     4,     4,
       4,     4,     4,     7,    69,    79,     3,     4,     3,     4,
      78,    39,    15,    18,    78,     9,    64,    64,    81,     4,
       4,     4,     3,    80,    18,    66,    77,    77,     4,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    60,    61,    61,    62,    62,    62,    62,    63,    63,
      64,    64,    65,    65,    66,    67,    68,    69,    70,    70,
      71,    71,    72,    72,    73,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    75,    75,
      76,    76,    77,    77,    78,    78,    79,    79,    80,    80,
      81,    81,    82,    82,    82,    82,    83,    84,    84,    84,
      84,    85,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    87,    88,    89,    90,
      90,    91,    92,    93,    93,    94,    94,    95,    96,    96,
      96,    96,    97,    98,    98,    99,    99,    99,    99,    99,
      99,    99,    99,   100,   101,   101,   101,   101,   101,   101,
     102,   102,   103,   104,   104,   105,   105,   106,   107
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     4,     8,     4,     1,     1,
       1,     1,     0,     2,     1,     1,     1,     1,     0,     1,
       0,     1,     0,     1,     3,     1,     2,     3,     4,     3,
       3,     4,     3,     4,     2,     3,     2,     9,     3,     3,
       4,     2,     3,     4,     5,     2,     2,     3,     4,     3,
       2,     2,     1,     1,     2,     2,     2,     3,     0,     1,
       1,     2,     0,     1,     4,     4,     0,     2,     4,     5,
       0,     2,     4,     5,     5,     6,     4,     4,     5,     5,
       6,     4,     0,     1,     2,     3,     4,     5,     6,     5,
       4,     5,     4,     3,     4,     5,     4,     3,     4,     3,
       2,     3,     2,     3,     4,     5,     4,     3,     4,     3,
       2,     3,     4,     3,     2,     1,     2,     3,     4,     5,
       4,     3,     4,     3,     2,     3,     4,     3,     2,     3,
       2,     1,     2,     3,     4,     3,     2,     3,     2,     1,
       2,     3,     2,     1,     2,     1,     4,     1,     5,     0,
       2,     1,     1,     6,     5,     7,     8,     4,     6,     7,
       6,     7,     5,     4,     5,     0,     2,     2,     2,     2,
       2,     2,     2,     4,     1,     5,     5,     9,     8,     9,
       0,     2,     4,     0,     1,     0,     2,     1,     1
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
#if WASM_DEBUG

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
# if defined WASM_LTYPE_IS_TRIVIAL && WASM_LTYPE_IS_TRIVIAL

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
#else /* !WASM_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !WASM_DEBUG */


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
  switch (yytype)
    {
          case 61: /* value_type_list  */
#line 154 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(&((*yyvaluep).types)); }
#line 1492 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 62: /* func_type  */
#line 168 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(&((*yyvaluep).func_sig)); }
#line 1498 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 63: /* literal  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1504 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 64: /* var  */
#line 155 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(&((*yyvaluep).var)); }
#line 1510 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 65: /* var_list  */
#line 156 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(&((*yyvaluep).vars)); }
#line 1516 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 66: /* bind_var  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1522 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 67: /* text  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1528 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 68: /* quoted_text  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1534 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 69: /* string_contents  */
#line 165 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(&((*yyvaluep).segment)); }
#line 1540 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 70: /* labeling  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1546 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 73: /* expr  */
#line 157 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1552 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 74: /* expr1  */
#line 157 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1558 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 75: /* expr_opt  */
#line 157 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1564 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 76: /* non_empty_expr_list  */
#line 158 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(&((*yyvaluep).exprs)); }
#line 1570 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 77: /* expr_list  */
#line 158 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(&((*yyvaluep).exprs)); }
#line 1576 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 78: /* target  */
#line 159 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target(&((*yyvaluep).target)); }
#line 1582 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 79: /* target_list  */
#line 160 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target_vector_and_elements(&((*yyvaluep).targets)); }
#line 1588 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 80: /* case  */
#line 161 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case(&((*yyvaluep).case_)); }
#line 1594 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 81: /* case_list  */
#line 162 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case_vector_and_elements(&((*yyvaluep).cases)); }
#line 1600 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 82: /* param_list  */
#line 163 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1606 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 84: /* local_list  */
#line 163 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1612 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 86: /* func_info  */
#line 164 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(&((*yyvaluep).func)); }
#line 1618 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 87: /* func  */
#line 164 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(&((*yyvaluep).func)); }
#line 1624 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 89: /* segment  */
#line 165 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(&((*yyvaluep).segment)); }
#line 1630 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 90: /* segment_list  */
#line 166 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(&((*yyvaluep).segments)); }
#line 1636 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 93: /* memory  */
#line 167 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(&((*yyvaluep).memory)); }
#line 1642 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 94: /* type_def  */
#line 169 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(&((*yyvaluep).func_type)); }
#line 1648 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 95: /* table  */
#line 156 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(&((*yyvaluep).vars)); }
#line 1654 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 96: /* import  */
#line 170 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(&((*yyvaluep).import)); }
#line 1660 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 97: /* export  */
#line 171 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(&((*yyvaluep).export)); }
#line 1666 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 98: /* global  */
#line 163 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1672 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 99: /* module_fields  */
#line 172 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module_field_vector_and_elements(&((*yyvaluep).module_fields)); }
#line 1678 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 100: /* module  */
#line 173 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(&((*yyvaluep).module)); }
#line 1684 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 101: /* cmd  */
#line 175 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(&((*yyvaluep).command)); }
#line 1690 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 102: /* cmd_list  */
#line 176 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(&((*yyvaluep).commands)); }
#line 1696 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 105: /* const_list  */
#line 174 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(&((*yyvaluep).consts)); }
#line 1702 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 106: /* script  */
#line 177 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1708 "src/wasm-parser.c" /* yacc.c:1257  */
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
# if defined WASM_LTYPE_IS_TRIVIAL && WASM_LTYPE_IS_TRIVIAL
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
#line 189 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.types)); }
#line 2002 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 190 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      CHECK_ALLOC(wasm_append_type_value(&(yyval.types), &(yyvsp[0].type)));
    }
#line 2011 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 196 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); }
#line 2017 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 197 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 2026 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 201 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 2035 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 205 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 2041 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 212 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2047 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 213 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2053 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 217 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &index, 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid int %.*s", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
      (yyval.var).index = index;
    }
#line 2067 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 226 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
      CHECK_ALLOC_STR((yyval.var).name);
    }
#line 2078 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 234 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.vars)); }
#line 2084 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 235 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); CHECK_ALLOC(wasm_append_var_value(&(yyval.vars), &(yyvsp[0].var))); }
#line 2090 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 238 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2096 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 242 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2102 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 246 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPQUOTEDTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2108 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 250 "src/wasm-parser.y" /* yacc.c:1646  */
    { CHECK_ALLOC(dup_string_contents(&(yyvsp[0].text), &(yyval.segment).data, &(yyval.segment).size)); }
#line 2114 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 254 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.text)); }
#line 2120 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 255 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.text) = (yyvsp[0].text); }
#line 2126 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 259 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2132 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 260 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64)))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid offset \"%.*s\"", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
    }
#line 2142 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 267 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = WASM_USE_NATURAL_ALIGNMENT; }
#line 2148 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 268 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid alignment \"%.*s\"",
                   (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 2158 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 276 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 2164 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 279 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_NOP);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2173 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 283 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[0].text);
    }
#line 2183 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 288 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[-1].text);
      (yyval.expr)->block.exprs = (yyvsp[0].exprs);
    }
#line 2194 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 294 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF_ELSE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[-1].expr);
      (yyval.expr)->if_else.false_ = (yyvsp[0].expr);
    }
#line 2206 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 301 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-1].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[0].expr);
    }
#line 2217 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 307 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.cond = (yyvsp[-1].expr);
      (yyval.expr)->br_if.var = (yyvsp[0].var);
    }
#line 2228 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 313 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.cond = (yyvsp[-2].expr);
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.expr = (yyvsp[0].expr);
    }
#line 2240 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 320 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      CHECK_ALLOC_NULL((yyval.expr));
      ZEROMEM((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2252 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 327 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2264 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 334 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var.loc = (yylsp[-1]);
      (yyval.expr)->br.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.expr)->br.var.index = 0;
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2277 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 342 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2288 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 348 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_RETURN);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2298 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 353 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_TABLESWITCH);
      CHECK_ALLOC_NULL((yyval.expr));
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
          CHECK_ALLOC_NULL(binding);
          binding->loc = case_->loc;
          binding->name = case_->label;
          binding->index = i;
        }
      }
    }
#line 2325 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 375 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2336 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 381 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_IMPORT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2347 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 387 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_INDIRECT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.args = (yyvsp[0].exprs);
    }
#line 2359 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 394 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GET_LOCAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2369 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 399 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SET_LOCAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2380 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 405 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2393 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 413 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2407 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 422 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONST);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (!read_const((yyvsp[-1].type), (yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.expr)->const_))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid literal \"%.*s\"", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
      free((char*)(yyvsp[0].text).start);
    }
#line 2421 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 431 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNARY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->unary.op = (yyvsp[-1].unary);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2432 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 437 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BINARY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->binary.op = (yyvsp[-2].binary);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2444 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 444 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SELECT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->select.type = (yyvsp[-3].type);
      (yyval.expr)->select.cond = (yyvsp[-2].expr);
      (yyval.expr)->select.true_ = (yyvsp[-1].expr);
      (yyval.expr)->select.false_ = (yyvsp[0].expr);
    }
#line 2457 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 452 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_COMPARE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->compare.op = (yyvsp[-2].compare);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2469 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 459 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONVERT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->convert.op = (yyvsp[-1].convert);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2480 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 465 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CAST);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->cast.op = (yyvsp[-1].cast);
      (yyval.expr)->cast.expr = (yyvsp[0].expr);
    }
#line 2491 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 471 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNREACHABLE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2500 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 475 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_MEMORY_SIZE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2509 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 479 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GROW_MEMORY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2519 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 484 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_HAS_FEATURE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->has_feature.text = (yyvsp[0].text);
    }
#line 2529 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 489 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD_GLOBAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load_global.var = (yyvsp[0].var);
    }
#line 2539 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 494 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE_GLOBAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store_global.var = (yyvsp[-1].var);
      (yyval.expr)->store_global.expr = (yyvsp[0].expr);
    }
#line 2550 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 502 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2556 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 506 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.exprs));
      CHECK_ALLOC(wasm_append_expr_ptr_value(&(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2565 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 510 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.exprs) = (yyvsp[-1].exprs);
      CHECK_ALLOC(wasm_append_expr_ptr_value(&(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2574 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 516 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.exprs)); }
#line 2580 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 521 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_CASE;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2589 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 525 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_BR;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2598 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 531 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.targets)); }
#line 2604 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 532 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.targets) = (yyvsp[-1].targets);
      CHECK_ALLOC(wasm_append_target_value(&(yyval.targets), &(yyvsp[0].target)));
    }
#line 2613 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 538 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.case_).label); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2619 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 539 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.case_).label = (yyvsp[-2].text); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2625 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 542 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.cases)); }
#line 2631 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 543 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.cases) = (yyvsp[-1].cases);
      CHECK_ALLOC(wasm_append_case_value(&(yyval.cases), &(yyvsp[0].case_)));
    }
#line 2640 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 553 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2650 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 558 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2664 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 567 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2674 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 572 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2688 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 583 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 2694 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 586 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2704 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 591 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2718 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 600 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2728 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 605 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2742 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 616 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2748 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 619 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
    }
#line 2757 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 623 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[0].text);
    }
#line 2767 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 628 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 2778 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 634 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 2790 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 641 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2803 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 649 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2817 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 658 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-5].text);
      (yyval.func).type_var = (yyvsp[-4].var);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2832 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 668 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2846 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 677 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2859 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 685 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2873 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 694 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2886 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 702 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2898 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 709 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2911 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 717 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2925 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 726 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2938 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 734 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2950 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 741 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2963 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 749 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2975 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 756 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2986 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 762 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2998 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 769 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3009 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 775 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3021 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 782 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3034 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 790 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3048 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 799 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3061 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 807 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3073 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 814 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3086 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 822 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3098 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 829 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3109 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 835 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3121 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 842 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3134 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 850 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3146 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 857 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3157 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 863 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 3167 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 868 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3178 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 874 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3190 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 881 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3203 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 889 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-4].var);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3217 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 898 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3230 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 906 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3242 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 913 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3255 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 921 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3267 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 928 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3278 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 934 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3290 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 941 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3303 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 949 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3315 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 956 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3326 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 962 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3338 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 969 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3349 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 975 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3359 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 980 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3370 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 986 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3382 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 993 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3395 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1001 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3407 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1008 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3418 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1014 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3430 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1021 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3441 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1027 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3451 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1032 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3462 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1038 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3474 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1045 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3485 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1051 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3495 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1056 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3506 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1062 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3516 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1069 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.func) = (yyvsp[-1].func); (yyval.func).loc = (yylsp[-2]); }
#line 3522 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1075 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_error(&(yylsp[0]), scanner, parser,
                   "invalid memory segment address \"%.*s\"", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
    }
#line 3533 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1084 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      (yyval.segment).addr = (yyvsp[-2].u32);
    }
#line 3544 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1092 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.segments)); }
#line 3550 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1093 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      CHECK_ALLOC(wasm_append_segment_value(&(yyval.segments), &(yyvsp[0].segment)));
    }
#line 3559 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1100 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid initial memory size \"%.*s\"",
                   (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 3569 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1108 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid max memory size \"%.*s\"",
                   (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 3579 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1116 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      (yyval.memory).initial_size = (yyvsp[-3].u32);
      (yyval.memory).max_size = (yyvsp[-2].u32);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3590 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1122 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      (yyval.memory).initial_size = (yyvsp[-2].u32);
      (yyval.memory).max_size = (yyval.memory).initial_size;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3601 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1131 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3610 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1135 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3619 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1142 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 3625 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1146 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3637 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1153 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3650 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1161 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3662 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1168 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3675 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1179 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.export).name = (yyvsp[-2].text);
      (yyval.export).var = (yyvsp[-1].var);
    }
#line 3684 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1186 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      (yyval.type_bindings).types = (yyvsp[-1].types);
    }
#line 3693 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1190 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = 0;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 3707 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1202 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.module_fields)); }
#line 3713 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1203 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = (yyvsp[0].func);
    }
#line 3726 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1211 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = (yyvsp[0].import);
    }
#line 3739 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1219 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = (yyvsp[0].export);
    }
#line 3752 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1227 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
    }
#line 3765 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1235 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);
    }
#line 3778 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1243 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
    }
#line 3791 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1251 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;
      field->global = (yyvsp[0].type_bindings);
    }
#line 3804 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1261 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.module));
      (yyval.module).loc = (yylsp[-2]);
      (yyval.module).fields = (yyvsp[-1].module_fields);

      /* cache values */
      int i;
      for (i = 0; i < (yyval.module).fields.size; ++i) {
        WasmModuleField* field = &(yyval.module).fields.data[i];
        switch (field->type) {
          case WASM_MODULE_FIELD_TYPE_FUNC: {
            WasmFuncPtr func_ptr = &field->func;
            CHECK_ALLOC(wasm_append_func_ptr_value(&(yyval.module).funcs, &func_ptr));
            if (field->func.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).func_bindings);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->name = field->func.name;
              binding->index = (yyval.module).funcs.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_IMPORT: {
            WasmImportPtr import_ptr = &field->import;
            CHECK_ALLOC(wasm_append_import_ptr_value(&(yyval.module).imports, &import_ptr));
            if (field->import.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).import_bindings);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->name = field->import.name;
              binding->index = (yyval.module).imports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_EXPORT: {
            WasmExportPtr export_ptr = &field->export_;
            CHECK_ALLOC(wasm_append_export_ptr_value(&(yyval.module).exports, &export_ptr));
            if (field->export_.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).export_bindings);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->name = field->export_.name;
              binding->index = (yyval.module).exports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_TABLE:
            (yyval.module).table = &field->table;
            break;
          case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
            WasmFuncTypePtr func_type_ptr = &field->func_type;
            CHECK_ALLOC(wasm_append_func_type_ptr_value(&(yyval.module).func_types,
                                                        &func_type_ptr));
            if (field->func_type.name.start) {
              WasmBinding* binding =
                  wasm_append_binding(&(yyval.module).func_type_bindings);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->name = field->func_type.name;
              binding->index = (yyval.module).func_types.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_MEMORY:
            (yyval.module).memory = &field->memory;
            break;
          case WASM_MODULE_FIELD_TYPE_GLOBAL:
            CHECK_ALLOC(wasm_extend_type_bindings(&(yyval.module).globals, &field->global));
            break;
        }
      }
    }
#line 3881 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1339 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.command).type = WASM_COMMAND_TYPE_MODULE; (yyval.command).module = (yyvsp[0].module); }
#line 3887 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1340 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command).invoke.loc = (yylsp[-3]);
      (yyval.command).invoke.name = (yyvsp[-2].text);
      (yyval.command).invoke.args = (yyvsp[-1].consts);
    }
#line 3898 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1346 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command).assert_invalid.module = (yyvsp[-2].module);
      (yyval.command).assert_invalid.text = (yyvsp[-1].text);
    }
#line 3908 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1351 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command).assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_return.expected = (yyvsp[-1].const_);
    }
#line 3920 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1358 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command).assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command).assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command).assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3931 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1364 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command).assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_trap.text = (yyvsp[-1].text);
    }
#line 3943 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1373 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.commands)); }
#line 3949 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1374 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      CHECK_ALLOC(wasm_append_command_value(&(yyval.commands), &(yyvsp[0].command)));
    }
#line 3958 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1381 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (!read_const((yyvsp[-2].type), (yyvsp[-1].text).start, (yyvsp[-1].text).start + (yyvsp[-1].text).length, &(yyval.const_)))
        wasm_error(&(yylsp[-1]), scanner, parser, "invalid literal \"%.*s\"", (yyvsp[-1].text).length,
                   (yyvsp[-1].text).start);
      free((char*)(yyvsp[-1].text).start);
    }
#line 3970 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1390 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3976 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1394 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.consts)); }
#line 3982 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1395 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      CHECK_ALLOC(wasm_append_const_value(&(yyval.consts), &(yyvsp[0].const_)));
    }
#line 3991 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1402 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.script).commands = (yyvsp[0].commands); parser->script = (yyval.script); }
#line 3997 "src/wasm-parser.c" /* yacc.c:1646  */
    break;


#line 4001 "src/wasm-parser.c" /* yacc.c:1646  */
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
#line 1411 "src/wasm-parser.y" /* yacc.c:1906  */


void wasm_error(WasmLocation* loc,
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

static WasmResult dup_string_contents(WasmStringSlice* text,
                                      void** out_data,
                                      size_t* out_size) {
  const char* src = text->start + 1;
  const char* end = text->start + text->length - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src;
  char* result = malloc(size);
  if (!result)
    return WASM_ERROR;
  size_t actual_size = copy_string_contents(text, result, size);
  *out_data = result;
  *out_size = actual_size;
  return WASM_OK;
}
