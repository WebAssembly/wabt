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
    WASM_TOKEN_TYPE_LOW = 317
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

#line 283 "src/wasm-parser.c" /* yacc.c:358  */

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
#define YYLAST   708

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  188
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  359

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   317

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
      55,    56,    57,    58,    59,    60,    61,    62
};

#if WASM_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   174,   180,   181,   185,   189,   196,   197,
     201,   210,   218,   219,   222,   226,   230,   234,   238,   239,
     243,   244,   251,   252,   260,   263,   267,   272,   278,   285,
     291,   297,   304,   311,   318,   324,   332,   338,   343,   365,
     371,   377,   384,   389,   395,   403,   412,   420,   429,   438,
     444,   451,   459,   466,   472,   478,   482,   486,   491,   496,
     501,   509,   510,   513,   517,   523,   524,   528,   532,   538,
     539,   545,   546,   549,   550,   560,   565,   574,   579,   590,
     593,   598,   607,   612,   623,   626,   630,   635,   641,   648,
     656,   665,   675,   684,   692,   701,   709,   716,   724,   733,
     741,   748,   756,   763,   769,   776,   782,   789,   797,   806,
     814,   821,   829,   836,   842,   849,   857,   864,   870,   875,
     881,   888,   896,   905,   913,   920,   928,   935,   941,   948,
     956,   963,   969,   976,   982,   987,   993,  1000,  1008,  1015,
    1021,  1028,  1034,  1039,  1045,  1052,  1058,  1063,  1069,  1076,
    1082,  1093,  1094,  1101,  1111,  1122,  1126,  1133,  1137,  1144,
    1152,  1159,  1170,  1177,  1181,  1193,  1194,  1202,  1210,  1218,
    1226,  1234,  1242,  1252,  1330,  1331,  1337,  1342,  1349,  1355,
    1364,  1365,  1372,  1381,  1382,  1385,  1386,  1393,  1399
};
#endif

#if WASM_DEBUG || YYERROR_VERBOSE || 1
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
  "ASSERT_TRAP", "INVOKE", "GLOBAL", "LOAD_GLOBAL", "STORE_GLOBAL", "LOW",
  "$accept", "value_type_list", "func_type", "literal", "var", "var_list",
  "bind_var", "text", "quoted_text", "string_contents", "labeling",
  "offset", "align", "expr", "expr1", "expr_opt", "non_empty_expr_list",
  "expr_list", "target", "target_list", "case", "case_list", "param_list",
  "result", "local_list", "type_use", "func_info", "func", "segment",
  "segment_list", "memory", "type_def", "table", "import", "export",
  "global", "module_fields", "module", "cmd", "cmd_list", "const",
  "const_opt", "const_list", "script", "start", YY_NULLPTR
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
     315,   316,   317
};
# endif

#define YYPACT_NINF -156

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-156)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -156,     2,  -156,    38,    -7,  -156,  -156,  -156,  -156,    39,
      43,    49,    83,    47,    13,    46,    47,    50,    95,   106,
    -156,  -156,   107,  -156,  -156,  -156,  -156,  -156,  -156,  -156,
    -156,    97,    47,    47,    47,   110,     5,    22,   100,   103,
      47,  -156,   157,  -156,  -156,  -156,  -156,   137,  -156,  -156,
     337,  -156,   164,  -156,   167,   168,   169,   174,   175,   176,
     146,   185,   188,    47,    47,    76,    91,    24,   187,   119,
     122,   131,    26,  -156,   157,   167,   167,   157,   157,    21,
     167,   157,    76,    76,    76,   167,    76,    76,   173,   173,
     173,   173,    26,   167,   167,   167,   167,   167,   167,    76,
     157,   195,   157,  -156,  -156,   167,   198,    76,    76,   202,
     167,   168,   169,   174,   175,   647,  -156,   438,   167,   169,
     174,   543,   167,   174,   595,   167,   491,   167,   168,   169,
     174,  -156,   204,   170,  -156,   135,    47,   205,  -156,  -156,
     206,  -156,  -156,  -156,  -156,   207,   209,   210,    47,  -156,
    -156,   211,  -156,   167,   167,   167,   157,   167,   167,   167,
    -156,  -156,    76,   167,   167,   167,   167,  -156,  -156,   167,
    -156,   182,   182,   182,   182,  -156,  -156,   167,   167,  -156,
    -156,   167,   213,    30,   212,   215,    36,   216,  -156,  -156,
    -156,  -156,   167,  -156,   167,   169,   174,   167,   174,   167,
     167,   168,   169,   174,   157,   167,   174,   167,   167,   157,
     167,   169,   174,   167,   174,   167,   108,   218,   204,   154,
     177,  -156,  -156,   205,    31,   222,   223,  -156,  -156,  -156,
     224,  -156,   225,  -156,   167,  -156,   167,   167,   167,  -156,
    -156,  -156,   167,   227,  -156,  -156,   167,  -156,  -156,   167,
     167,   167,   167,  -156,  -156,   167,  -156,  -156,   231,  -156,
    -156,   232,  -156,   167,   174,   167,   167,   167,   169,   174,
     167,   174,   167,    51,   229,   167,    59,   230,   167,   174,
     167,   167,  -156,   233,   236,   237,  -156,   238,   240,   242,
    -156,  -156,  -156,  -156,  -156,  -156,  -156,   219,  -156,  -156,
     167,  -156,   167,  -156,  -156,  -156,   167,   167,   174,   167,
     167,  -156,   247,  -156,   249,   167,    61,   251,  -156,   252,
     254,  -156,  -156,  -156,  -156,  -156,   167,  -156,  -156,   255,
    -156,  -156,  -156,   260,   158,   217,  -156,    78,   262,  -156,
     266,    76,    76,  -156,   272,   275,   276,   278,  -156,  -156,
    -156,   267,  -156,    66,   167,   281,   283,  -156,  -156
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     180,   187,   188,     0,     0,   174,   181,     1,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      16,   185,     0,   173,   166,   171,   170,   169,   167,   168,
     172,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    12,     2,   176,   185,   185,   185,     0,   175,   186,
       0,    14,    86,    63,   148,   134,   142,   146,   118,     0,
       0,     0,   151,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    18,     0,     0,    18,    18,    61,
       0,    18,     0,     0,     0,    61,     0,     0,    20,    20,
      20,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     2,    55,    56,     0,     0,     0,     0,     0,
     117,   105,   113,   103,    87,     0,    64,     0,   141,   135,
     139,     0,   145,   143,     0,   147,     0,   133,   119,   127,
     131,   149,     4,     0,   151,     0,     0,     4,    10,    11,
       0,   157,    13,   163,     3,     0,   183,     0,     0,     8,
       9,     0,    19,    26,     0,     0,    19,    65,     0,    61,
      62,    35,     0,     0,    65,    65,     0,    37,    42,     0,
      21,    22,    22,    22,    22,    48,    49,     0,     0,    53,
      54,     0,     0,     0,     0,     0,     0,     0,    57,    15,
      58,    59,     0,    24,   112,   106,   110,   116,   114,   104,
     102,    88,    96,   100,     2,   138,   136,   140,   144,     2,
     126,   120,   124,   130,   128,   132,     0,     0,     4,     0,
       0,   154,   152,     4,     0,     0,     0,   162,   164,   184,
       0,   178,     0,   182,    27,    29,     0,    65,    66,    32,
      34,    36,    30,     0,    39,    40,    65,    43,    23,     0,
       0,     0,     0,    50,    52,     0,    84,    75,     0,    79,
      80,     0,    60,   109,   107,   111,   115,    95,    89,    93,
      99,    97,   101,     0,     0,   137,     0,     0,   123,   121,
     125,   129,     2,     0,     0,     0,   153,     0,     0,     0,
     160,   158,   177,   179,    28,    33,    31,     0,    41,    44,
       0,    46,     0,    51,    76,    81,   108,    92,    90,    94,
      98,    77,     0,    82,     0,   122,     0,     0,   155,     0,
       0,   161,   159,    69,    45,    47,    91,    78,    83,     5,
       7,   156,    17,     0,     0,     0,   150,     0,     0,    70,
       0,     0,     0,    73,     0,     0,     0,    38,     6,    68,
      67,     0,    74,    65,    65,     0,     0,    71,    72
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -156,   -88,  -130,   201,   -64,  -156,   -36,  -156,   282,  -156,
      25,    28,   -41,   -18,  -156,   -74,   214,  -155,   -44,  -156,
    -156,  -156,   -31,     1,    72,   -48,  -156,  -156,  -156,   156,
    -156,  -156,  -156,  -156,  -156,  -156,  -156,   286,  -156,  -156,
     150,  -156,    98,  -156,  -156
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    67,   217,   151,   140,    66,   152,   190,    21,   333,
     153,   171,   249,    53,   109,   161,   238,   239,   339,   334,
     352,   347,    55,    56,    57,    58,    59,    24,   222,   135,
      25,    26,    27,    28,    29,    30,    14,     5,     6,     1,
      49,   230,    35,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      52,    61,   142,    63,   114,     4,    68,   225,    50,   244,
     245,   167,   183,    51,   186,   159,    22,    23,   164,   165,
     166,   111,   168,   169,   115,    60,   138,   128,   143,   139,
      51,   149,   150,   144,   257,   182,   116,     8,     7,   144,
     260,   156,    15,   191,   192,   144,    17,     9,    10,    11,
      12,    13,    18,   112,    20,   311,   119,   154,   155,   129,
     144,   160,   162,   313,   184,   329,   187,   160,   144,   115,
     144,    99,   282,   283,    51,   176,   177,   178,   179,   180,
     181,   138,   295,   201,   139,   241,    19,   188,   285,   226,
       8,   298,   116,   288,   341,   141,   138,   342,   242,   139,
     116,    43,   157,   158,   116,    62,   163,   116,    32,   116,
      20,    51,   195,    47,    48,   202,   273,   172,   173,   174,
     237,   276,    47,   146,   113,    47,   147,   120,   123,   211,
     130,   250,   251,   252,    47,   148,   235,   236,   220,   221,
     240,   160,    69,    70,    71,   243,    36,    37,   246,   282,
     283,   247,    38,    33,    39,    40,    41,   220,   286,   253,
     254,   337,   338,   255,    34,    51,    42,    50,   274,    72,
     115,   117,   121,   277,   262,   289,   116,   124,   126,   116,
     131,   116,   116,   196,   198,   132,   203,   116,   133,   116,
     116,   206,   116,   134,   316,   116,   145,   116,   355,   356,
     212,   214,   268,   170,   185,   189,   193,   216,   224,   218,
     227,   228,    47,   248,   231,   233,   116,   256,   294,   259,
     116,   258,   284,   287,   296,   261,   290,   291,   292,   293,
     297,   299,   300,   301,   302,   304,   305,   303,   312,   314,
     318,   319,   317,   320,   321,   116,   322,   116,   116,   116,
      54,   327,   116,   328,   116,   330,   331,   116,   335,   340,
     116,   332,   116,   116,   336,   337,   110,   264,   323,   118,
     122,   125,   127,   269,   271,   344,   348,   345,   346,   349,
     350,   351,   324,   279,   325,   357,   353,   358,   116,   116,
     219,   116,   116,   175,   343,    16,   229,   116,    31,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   116,     0,
       0,     0,     0,     0,    44,    45,    46,   354,     0,     0,
       0,    64,    65,     0,     0,   194,   197,   199,   200,     0,
       0,     0,     0,   205,   207,     0,     0,   208,     0,     0,
     308,     0,   210,   213,   215,   136,   137,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     0,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,   234,     0,    92,
      93,    94,    95,    96,    97,    98,     0,    99,   100,   101,
     102,     0,     0,     0,     0,     0,     0,   103,   104,   105,
     106,     0,     0,     0,     0,     0,     0,   107,   108,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   263,
     265,     0,   266,     0,     0,   267,   270,   272,   223,     0,
     275,     0,     0,     0,     0,   278,   280,     0,   281,     0,
     232,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
      92,    93,    94,    95,    96,    97,    98,     0,   306,   204,
     101,   102,   307,   309,     0,   310,     0,     0,   103,   104,
     105,   106,     0,   315,     0,     0,     0,     0,   107,   108,
       0,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       0,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,     0,   326,    92,    93,    94,    95,    96,    97,    98,
       0,     0,   100,   101,   102,     0,     0,     0,     0,     0,
       0,   103,   104,   105,   106,     0,     0,     0,     0,     0,
       0,   107,   108,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     0,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,    92,    93,    94,    95,    96,
      97,    98,     0,     0,     0,     0,   102,     0,     0,     0,
       0,     0,     0,   103,   104,   105,   106,     0,     0,     0,
       0,     0,     0,   107,   108,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     0,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,    92,    93,    94,
      95,    96,    97,    98,     0,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,   103,   104,   105,   106,     0,
       0,     0,     0,     0,     0,   107,   108,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     0,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,    98,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   103,   104,   105,
     106,     0,     0,     0,     0,     0,     0,   107,   108
};

static const yytype_int16 yycheck[] =
{
      36,    37,    66,    39,    52,     3,    42,   137,     3,   164,
     165,    85,   100,     8,   102,    79,     3,     4,    82,    83,
      84,    52,    86,    87,     3,     3,     5,    58,     4,     8,
       8,     5,     6,     9,     4,    99,    54,    44,     0,     9,
       4,    77,     3,   107,   108,     9,     3,    54,    55,    56,
      57,    58,     3,    52,     7,     4,    55,    75,    76,    58,
       9,    79,    80,     4,   100,     4,   102,    85,     9,     3,
       9,    40,    41,    42,     8,    93,    94,    95,    96,    97,
      98,     5,   237,   114,     8,   159,     3,   105,   218,   137,
      44,   246,   110,   223,    16,     4,     5,    19,   162,     8,
     118,     4,    77,    78,   122,     5,    81,   125,    58,   127,
       7,     8,   111,     3,     4,   114,   204,    89,    90,    91,
     156,   209,     3,     4,    52,     3,     4,    55,    56,   128,
      58,   172,   173,   174,     3,     4,   154,   155,     3,     4,
     158,   159,    44,    45,    46,   163,    39,    40,   166,    41,
      42,   169,    45,    58,    47,    48,    49,     3,     4,   177,
     178,     3,     4,   181,    58,     8,    59,     3,   204,    32,
       3,     3,     3,   209,   192,   223,   194,     3,     3,   197,
       4,   199,   200,   111,   112,    39,   114,   205,     3,   207,
     208,   119,   210,     5,   282,   213,     9,   215,   353,   354,
     128,   129,   201,    30,     9,     7,     4,     3,     3,    39,
       4,     4,     3,    31,     4,     4,   234,     4,   236,     4,
     238,     9,     4,    46,   242,     9,     4,     4,     4,     4,
       3,   249,   250,   251,   252,     4,     4,   255,     9,     9,
       4,     4,     9,     5,     4,   263,     4,   265,   266,   267,
      36,     4,   270,     4,   272,     4,     4,   275,     3,    42,
     278,     7,   280,   281,     4,     3,    52,   195,    49,    55,
      56,    57,    58,   201,   202,     9,     4,   341,   342,     4,
       4,     3,   300,   211,   302,     4,    19,     4,   306,   307,
     134,   309,   310,    92,   338,     9,   146,   315,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   326,    -1,
      -1,    -1,    -1,    -1,    32,    33,    34,   353,    -1,    -1,
      -1,    39,    40,    -1,    -1,   111,   112,   113,   114,    -1,
      -1,    -1,    -1,   119,   120,    -1,    -1,   123,    -1,    -1,
     268,    -1,   128,   129,   130,    63,    64,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   153,    -1,    32,
      33,    34,    35,    36,    37,    38,    -1,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,   198,    -1,    -1,   201,   202,   203,   136,    -1,
     206,    -1,    -1,    -1,    -1,   211,   212,    -1,   214,    -1,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,    33,    34,    35,    36,    37,    38,    -1,   264,    41,
      42,    43,   268,   269,    -1,   271,    -1,    -1,    50,    51,
      52,    53,    -1,   279,    -1,    -1,    -1,    -1,    60,    61,
      -1,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,   308,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   102,   106,   107,     3,   100,   101,     0,    44,    54,
      55,    56,    57,    58,    99,     3,   100,     3,     3,     3,
       7,    71,     3,     4,    90,    93,    94,    95,    96,    97,
      98,    71,    58,    58,    58,   105,    39,    40,    45,    47,
      48,    49,    59,     4,    71,    71,    71,     3,     4,   103,
       3,     8,    69,    76,    79,    85,    86,    87,    88,    89,
       3,    69,     5,    69,    71,    71,    68,    64,    69,   105,
     105,   105,    32,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    32,    33,    34,    35,    36,    37,    38,    40,
      41,    42,    43,    50,    51,    52,    53,    60,    61,    77,
      79,    85,    86,    87,    88,     3,    76,     3,    79,    86,
      87,     3,    79,    87,     3,    79,     3,    79,    85,    86,
      87,     4,    39,     3,     5,    92,    71,    71,     5,     8,
      67,     4,    67,     4,     9,     9,     4,     4,     4,     5,
       6,    66,    69,    73,    76,    76,    69,    73,    73,    67,
      76,    78,    76,    73,    67,    67,    67,    78,    67,    67,
      30,    74,    74,    74,    74,    66,    76,    76,    76,    76,
      76,    76,    67,    64,    69,     9,    64,    69,    76,     7,
      70,    67,    67,     4,    79,    86,    87,    79,    87,    79,
      79,    85,    86,    87,    41,    79,    87,    79,    79,    43,
      79,    86,    87,    79,    87,    79,     3,    65,    39,    92,
       3,     4,    91,    71,     3,    65,    88,     4,     4,   103,
     104,     4,    71,     4,    79,    76,    76,    69,    79,    80,
      76,    78,    67,    76,    80,    80,    76,    76,    31,    75,
      75,    75,    75,    76,    76,    76,     4,     4,     9,     4,
       4,     9,    76,    79,    87,    79,    79,    79,    86,    87,
      79,    87,    79,    64,    69,    79,    64,    69,    79,    87,
      79,    79,    41,    42,     4,    65,     4,    46,    65,    88,
       4,     4,     4,     4,    76,    80,    76,     3,    80,    76,
      76,    76,    76,    76,     4,     4,    79,    79,    87,    79,
      79,     4,     9,     4,     9,    79,    64,     9,     4,     4,
       5,     4,     4,    49,    76,    76,    79,     4,     4,     4,
       4,     4,     7,    72,    82,     3,     4,     3,     4,    81,
      42,    16,    19,    81,     9,    67,    67,    84,     4,     4,
       4,     3,    83,    19,    69,    80,    80,     4,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    63,    64,    64,    65,    65,    65,    65,    66,    66,
      67,    67,    68,    68,    69,    70,    71,    72,    73,    73,
      74,    74,    75,    75,    76,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    78,    78,    79,    79,    80,    80,    81,    81,    82,
      82,    83,    83,    84,    84,    85,    85,    85,    85,    86,
      87,    87,    87,    87,    88,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    90,
      91,    92,    92,    93,    93,    94,    94,    95,    96,    96,
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
       3,     4,     3,     4,     3,     2,     3,     2,     9,     3,
       3,     4,     2,     3,     4,     5,     4,     5,     2,     2,
       3,     4,     3,     2,     2,     1,     1,     2,     2,     2,
       3,     0,     1,     1,     2,     0,     1,     4,     4,     0,
       2,     4,     5,     0,     2,     4,     5,     5,     6,     4,
       4,     5,     5,     6,     4,     0,     1,     2,     3,     4,
       5,     6,     5,     4,     5,     4,     3,     4,     5,     4,
       3,     4,     3,     2,     3,     2,     3,     4,     5,     4,
       3,     4,     3,     2,     3,     4,     3,     2,     1,     2,
       3,     4,     5,     4,     3,     4,     3,     2,     3,     4,
       3,     2,     3,     2,     1,     2,     3,     4,     3,     2,
       3,     2,     1,     2,     3,     2,     1,     2,     1,     4,
       5,     0,     2,     6,     5,     7,     8,     4,     6,     7,
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
          case 64: /* value_type_list  */
#line 138 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(&((*yyvaluep).types)); }
#line 1505 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 65: /* func_type  */
#line 152 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(&((*yyvaluep).func_sig)); }
#line 1511 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 66: /* literal  */
#line 137 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1517 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 67: /* var  */
#line 139 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(&((*yyvaluep).var)); }
#line 1523 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 68: /* var_list  */
#line 140 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(&((*yyvaluep).vars)); }
#line 1529 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 69: /* bind_var  */
#line 137 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1535 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 70: /* text  */
#line 137 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1541 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 71: /* quoted_text  */
#line 137 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1547 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 72: /* string_contents  */
#line 149 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(&((*yyvaluep).segment)); }
#line 1553 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 73: /* labeling  */
#line 137 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(&((*yyvaluep).text)); }
#line 1559 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 76: /* expr  */
#line 141 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1565 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 77: /* expr1  */
#line 141 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1571 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 78: /* expr_opt  */
#line 141 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(&((*yyvaluep).expr)); }
#line 1577 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 79: /* non_empty_expr_list  */
#line 142 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(&((*yyvaluep).exprs)); }
#line 1583 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 80: /* expr_list  */
#line 142 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(&((*yyvaluep).exprs)); }
#line 1589 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 81: /* target  */
#line 143 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target(&((*yyvaluep).target)); }
#line 1595 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 82: /* target_list  */
#line 144 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target_vector_and_elements(&((*yyvaluep).targets)); }
#line 1601 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 83: /* case  */
#line 145 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case(&((*yyvaluep).case_)); }
#line 1607 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 84: /* case_list  */
#line 146 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case_vector_and_elements(&((*yyvaluep).cases)); }
#line 1613 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 85: /* param_list  */
#line 147 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1619 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 87: /* local_list  */
#line 147 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1625 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 89: /* func_info  */
#line 148 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(&((*yyvaluep).func)); }
#line 1631 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 90: /* func  */
#line 148 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(&((*yyvaluep).func)); }
#line 1637 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 91: /* segment  */
#line 149 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(&((*yyvaluep).segment)); }
#line 1643 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 92: /* segment_list  */
#line 150 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(&((*yyvaluep).segments)); }
#line 1649 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 93: /* memory  */
#line 151 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(&((*yyvaluep).memory)); }
#line 1655 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 94: /* type_def  */
#line 153 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(&((*yyvaluep).func_type)); }
#line 1661 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 95: /* table  */
#line 140 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(&((*yyvaluep).vars)); }
#line 1667 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 96: /* import  */
#line 154 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(&((*yyvaluep).import)); }
#line 1673 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 97: /* export  */
#line 155 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(&((*yyvaluep).export)); }
#line 1679 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 98: /* global  */
#line 147 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(&((*yyvaluep).type_bindings)); }
#line 1685 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 99: /* module_fields  */
#line 156 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module_field_vector_and_elements(&((*yyvaluep).module_fields)); }
#line 1691 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 100: /* module  */
#line 157 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(&((*yyvaluep).module)); }
#line 1697 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 101: /* cmd  */
#line 159 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(&((*yyvaluep).command)); }
#line 1703 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 102: /* cmd_list  */
#line 160 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(&((*yyvaluep).commands)); }
#line 1709 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 105: /* const_list  */
#line 158 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(&((*yyvaluep).consts)); }
#line 1715 "src/wasm-parser.c" /* yacc.c:1257  */
        break;

    case 106: /* script  */
#line 161 "src/wasm-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1721 "src/wasm-parser.c" /* yacc.c:1257  */
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
#line 173 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.types)); }
#line 2015 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 174 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      CHECK_ALLOC(wasm_append_type_value(&(yyval.types), &(yyvsp[0].type)));
    }
#line 2024 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 180 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); }
#line 2030 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 181 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 2039 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 185 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 2048 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 189 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 2054 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 196 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2060 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 197 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2066 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 201 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &index, 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid int %.*s", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
      (yyval.var).index = index;
    }
#line 2080 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 210 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
      CHECK_ALLOC_STR((yyval.var).name);
    }
#line 2091 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 218 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.vars)); }
#line 2097 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 219 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); CHECK_ALLOC(wasm_append_var_value(&(yyval.vars), &(yyvsp[0].var))); }
#line 2103 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 222 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2109 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 226 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2115 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 230 "src/wasm-parser.y" /* yacc.c:1646  */
    { DUPQUOTEDTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2121 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 234 "src/wasm-parser.y" /* yacc.c:1646  */
    { CHECK_ALLOC(dup_string_contents(&(yyvsp[0].text), &(yyval.segment).data, &(yyval.segment).size)); }
#line 2127 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 238 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.text)); }
#line 2133 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 239 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.text) = (yyvsp[0].text); }
#line 2139 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 243 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2145 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 244 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64)))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid offset \"%.*s\"", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
    }
#line 2155 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 251 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = WASM_USE_NATURAL_ALIGNMENT; }
#line 2161 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 252 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid alignment \"%.*s\"",
                   (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 2171 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 260 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 2177 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 263 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_NOP);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2186 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 267 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[0].text);
    }
#line 2196 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 272 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BLOCK);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[-1].text);
      (yyval.expr)->block.exprs = (yyvsp[0].exprs);
    }
#line 2207 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 278 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF_ELSE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[-1].expr);
      (yyval.expr)->if_else.false_ = (yyvsp[0].expr);
    }
#line 2219 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 285 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-1].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[0].expr);
    }
#line 2230 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 291 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.cond = (yyvsp[-1].expr);
      (yyval.expr)->br_if.var = (yyvsp[0].var);
    }
#line 2241 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 297 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR_IF);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.cond = (yyvsp[-2].expr);
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.expr = (yyvsp[0].expr);
    }
#line 2253 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 304 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      CHECK_ALLOC_NULL((yyval.expr));
      ZEROMEM((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2265 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 311 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOOP);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2277 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 318 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LABEL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->label.label = (yyvsp[-1].text);
      (yyval.expr)->label.expr = (yyvsp[0].expr);
    }
#line 2288 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 324 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var.loc = (yylsp[-1]);
      (yyval.expr)->br.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.expr)->br.var.index = 0;
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2301 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 332 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BR);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2312 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 338 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_RETURN);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2322 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 343 "src/wasm-parser.y" /* yacc.c:1646  */
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
#line 2349 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 365 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2360 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 371 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_IMPORT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2371 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 377 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CALL_INDIRECT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.args = (yyvsp[0].exprs);
    }
#line 2383 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 384 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GET_LOCAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2393 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 389 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SET_LOCAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2404 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 395 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2417 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 403 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2431 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 412 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD_EXTEND);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2444 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 420 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE_WRAP);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2458 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 429 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONST);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (!read_const((yyvsp[-1].type), (yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.expr)->const_))
        wasm_error(&(yylsp[0]), scanner, parser, "invalid literal \"%.*s\"", (yyvsp[0].text).length,
                   (yyvsp[0].text).start);
      free((char*)(yyvsp[0].text).start);
    }
#line 2472 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 438 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNARY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->unary.op = (yyvsp[-1].unary);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2483 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 444 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_BINARY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->binary.op = (yyvsp[-2].binary);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2495 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 451 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_SELECT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->select.type = (yyvsp[-3].type);
      (yyval.expr)->select.cond = (yyvsp[-2].expr);
      (yyval.expr)->select.true_ = (yyvsp[-1].expr);
      (yyval.expr)->select.false_ = (yyvsp[0].expr);
    }
#line 2508 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 459 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_COMPARE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->compare.op = (yyvsp[-2].compare);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2520 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 466 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CONVERT);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->convert.op = (yyvsp[-1].convert);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2531 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 472 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_CAST);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->cast.op = (yyvsp[-1].cast);
      (yyval.expr)->cast.expr = (yyvsp[0].expr);
    }
#line 2542 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 478 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_UNREACHABLE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2551 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 482 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_MEMORY_SIZE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2560 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 486 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_GROW_MEMORY);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2570 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 491 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_HAS_FEATURE);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->has_feature.text = (yyvsp[0].text);
    }
#line 2580 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 496 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_LOAD_GLOBAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load_global.var = (yyvsp[0].var);
    }
#line 2590 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 501 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_expr(WASM_EXPR_TYPE_STORE_GLOBAL);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store_global.var = (yyvsp[-1].var);
      (yyval.expr)->store_global.expr = (yyvsp[0].expr);
    }
#line 2601 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 509 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2607 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 513 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.exprs));
      CHECK_ALLOC(wasm_append_expr_ptr_value(&(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2616 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 517 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.exprs) = (yyvsp[-1].exprs);
      CHECK_ALLOC(wasm_append_expr_ptr_value(&(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2625 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 523 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.exprs)); }
#line 2631 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 528 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_CASE;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2640 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 532 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_BR;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2649 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 538 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.targets)); }
#line 2655 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 539 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.targets) = (yyvsp[-1].targets);
      CHECK_ALLOC(wasm_append_target_value(&(yyval.targets), &(yyvsp[0].target)));
    }
#line 2664 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 545 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.case_).label); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2670 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 546 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.case_).label = (yyvsp[-2].text); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2676 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 549 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.cases)); }
#line 2682 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 550 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.cases) = (yyvsp[-1].cases);
      CHECK_ALLOC(wasm_append_case_value(&(yyval.cases), &(yyvsp[0].case_)));
    }
#line 2691 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 560 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2701 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 565 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2715 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 574 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2725 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 579 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2739 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 590 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 2745 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 593 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2755 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 598 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2769 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 607 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(&(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(&(yyvsp[-1].types));
    }
#line 2779 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 612 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2793 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 623 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2799 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 626 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
    }
#line 2808 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 630 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[0].text);
    }
#line 2818 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 635 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 2829 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 641 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 2841 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 648 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2854 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 656 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2868 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 665 "src/wasm-parser.y" /* yacc.c:1646  */
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
#line 2883 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 675 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2897 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 684 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2910 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 692 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2924 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 701 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2937 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 709 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 2949 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 716 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 2962 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 724 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2976 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 733 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 2989 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 741 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3001 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 748 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3014 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 756 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3026 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 763 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3037 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 769 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3049 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 776 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3060 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 782 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3072 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 789 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3085 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 797 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-4].text);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3099 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 806 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3112 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 814 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3124 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 821 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3137 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 829 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3149 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 836 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3160 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 842 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3172 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 849 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-3].text);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3185 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 857 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-2].text);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3197 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 864 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).name = (yyvsp[-1].text);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3208 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 870 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[0].var);
    }
#line 3218 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 875 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3229 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 881 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3241 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 888 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3254 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 896 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-4].var);
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3268 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 905 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3281 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 913 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3293 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 920 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3306 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 928 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3318 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 935 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3329 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 941 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3341 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 948 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-3].var);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3354 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 956 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3366 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 963 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3377 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 969 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-2].var);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3389 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 976 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func).type_var = (yyvsp[-1].var);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3400 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 982 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[0].type_bindings);
    }
#line 3410 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 987 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3421 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 993 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3433 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1000 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-3].type_bindings);
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3446 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1008 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3458 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1015 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3469 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1021 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-2].type_bindings);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3481 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1028 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).params = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3492 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1034 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[0].type);
    }
#line 3502 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1039 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3513 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1045 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-2].type);
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3525 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1052 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).result_type = (yyvsp[-1].type);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3536 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1058 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[0].type_bindings);
    }
#line 3546 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1063 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).locals = (yyvsp[-1].type_bindings);
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3557 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1069 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func));
      (yyval.func).flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func).exprs = (yyvsp[0].exprs);
    }
#line 3567 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1076 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.func) = (yyvsp[-1].func); (yyval.func).loc = (yylsp[-2]); }
#line 3573 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1082 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.segment).addr, 0))
        wasm_error(&(yylsp[-2]), scanner, parser,
                   "invalid memory segment address \"%.*s\"", (yyvsp[-2].text).length,
                   (yyvsp[-2].text).start);
    }
#line 3587 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1093 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.segments)); }
#line 3593 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1094 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      CHECK_ALLOC(wasm_append_segment_value(&(yyval.segments), &(yyvsp[0].segment)));
    }
#line 3602 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1101 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      if (!read_int32((yyvsp[-3].text).start, (yyvsp[-3].text).start + (yyvsp[-3].text).length, &(yyval.memory).initial_size, 0))
        wasm_error(&(yylsp[-3]), scanner, parser, "invalid initial memory size \"%.*s\"",
                   (yyvsp[-3].text).length, (yyvsp[-3].text).start);
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.memory).max_size, 0))
        wasm_error(&(yylsp[-2]), scanner, parser, "invalid max memory size \"%.*s\"",
                   (yyvsp[-2].text).length, (yyvsp[-2].text).start);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3617 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1111 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      if (!read_int32((yyvsp[-2].text).start, (yyvsp[-2].text).start + (yyvsp[-2].text).length, &(yyval.memory).initial_size, 0))
        wasm_error(&(yylsp[-2]), scanner, parser, "invalid initial memory size \"%.*s\"",
                   (yyvsp[-2].text).length, (yyvsp[-2].text).start);
      (yyval.memory).max_size = (yyval.memory).initial_size;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3630 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1122 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3639 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1126 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3648 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1133 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 3654 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1137 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3666 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1144 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).type_var = (yyvsp[-1].var);
    }
#line 3679 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1152 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3691 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1159 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.import));
      (yyval.import).import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import).name = (yyvsp[-4].text);
      (yyval.import).module_name = (yyvsp[-3].text);
      (yyval.import).func_name = (yyvsp[-2].text);
      (yyval.import).func_sig = (yyvsp[-1].func_sig);
    }
#line 3704 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1170 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.export).name = (yyvsp[-2].text);
      (yyval.export).var = (yyvsp[-1].var);
    }
#line 3713 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1177 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      (yyval.type_bindings).types = (yyvsp[-1].types);
    }
#line 3722 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1181 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding = wasm_append_binding(&(yyval.type_bindings).bindings);
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->name = (yyvsp[-2].text);
      binding->index = 0;
      CHECK_ALLOC(wasm_append_type_value(&(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 3736 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1193 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.module_fields)); }
#line 3742 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1194 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = (yyvsp[0].func);
    }
#line 3755 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1202 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = (yyvsp[0].import);
    }
#line 3768 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1210 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export = (yyvsp[0].export);
    }
#line 3781 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1218 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
    }
#line 3794 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1226 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);
    }
#line 3807 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1234 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
    }
#line 3820 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1242 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(&(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;
      field->global = (yyvsp[0].type_bindings);
    }
#line 3833 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1252 "src/wasm-parser.y" /* yacc.c:1646  */
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
            WasmExportPtr export_ptr = &field->export;
            CHECK_ALLOC(wasm_append_export_ptr_value(&(yyval.module).exports, &export_ptr));
            if (field->export.name.start) {
              WasmBinding* binding = wasm_append_binding(&(yyval.module).export_bindings);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->name = field->export.name;
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
#line 3910 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1330 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.command).type = WASM_COMMAND_TYPE_MODULE; (yyval.command).module = (yyvsp[0].module); }
#line 3916 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1331 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command).invoke.loc = (yylsp[-3]);
      (yyval.command).invoke.name = (yyvsp[-2].text);
      (yyval.command).invoke.args = (yyvsp[-1].consts);
    }
#line 3927 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1337 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command).assert_invalid.module = (yyvsp[-2].module);
      (yyval.command).assert_invalid.text = (yyvsp[-1].text);
    }
#line 3937 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1342 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command).assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_return.expected = (yyvsp[-1].const_);
    }
#line 3949 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1349 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command).assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command).assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command).assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3960 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1355 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.command).type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command).assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command).assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command).assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command).assert_trap.text = (yyvsp[-1].text);
    }
#line 3972 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1364 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.commands)); }
#line 3978 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1365 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      CHECK_ALLOC(wasm_append_command_value(&(yyval.commands), &(yyvsp[0].command)));
    }
#line 3987 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1372 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (!read_const((yyvsp[-2].type), (yyvsp[-1].text).start, (yyvsp[-1].text).start + (yyvsp[-1].text).length, &(yyval.const_)))
        wasm_error(&(yylsp[-1]), scanner, parser, "invalid literal \"%.*s\"", (yyvsp[-1].text).length,
                   (yyvsp[-1].text).start);
      free((char*)(yyvsp[-1].text).start);
    }
#line 3999 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1381 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 4005 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1385 "src/wasm-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.consts)); }
#line 4011 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1386 "src/wasm-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      CHECK_ALLOC(wasm_append_const_value(&(yyval.consts), &(yyvsp[0].const_)));
    }
#line 4020 "src/wasm-parser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1393 "src/wasm-parser.y" /* yacc.c:1646  */
    { (yyval.script).commands = (yyvsp[0].commands); parser->script = (yyval.script); }
#line 4026 "src/wasm-parser.c" /* yacc.c:1646  */
    break;


#line 4030 "src/wasm-parser.c" /* yacc.c:1646  */
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
#line 1402 "src/wasm-parser.y" /* yacc.c:1906  */


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
