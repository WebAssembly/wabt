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
#define YYSTYPE         WASM_PARSER_STYPE
#define YYLTYPE         WASM_PARSER_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         wasm_parser_parse
#define yylex           wasm_parser_lex
#define yyerror         wasm_parser_error
#define yydebug         wasm_parser_debug
#define yynerrs         wasm_parser_nerrs


/* Copy the first part of user declarations.  */
#line 17 "src/wasm-bison-parser.y" /* yacc.c:339  */

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-internal.h"
#include "wasm-parser.h"

#define ZEROMEM(var) memset(&(var), 0, sizeof(var));

#define DUPTEXT(dst, src)                                                   \
  (dst).start = wasm_strndup(parser->allocator, (src).start, (src).length); \
  (dst).length = (src).length

#define DUPQUOTEDTEXT(dst, src)                                           \
  (dst).start =                                                           \
      wasm_strndup(parser->allocator, (src).start + 1, (src).length - 2); \
  (dst).length = (src).length - 2

#define CHECK_ALLOC_(cond)                                       \
  if ((cond)) {                                                  \
    WasmLocation loc;                                            \
    loc.filename = __FILE__;                                     \
    loc.line = __LINE__;                                         \
    loc.first_column = loc.last_column = 0;                      \
    wasm_parser_error(&loc, lexer, parser, "allocation failed"); \
    YYERROR;                                                     \
  }

#define CHECK_ALLOC(e) CHECK_ALLOC_((e) != WASM_OK)
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_(!(v))
#define CHECK_ALLOC_STR(s) CHECK_ALLOC_(!(s).start)

#define YYLLOC_DEFAULT(Current, Rhs, N)                       \
  do                                                          \
    if (N) {                                                  \
      (Current).filename = YYRHSLOC(Rhs, 1).filename;         \
      (Current).line = YYRHSLOC(Rhs, 1).line;                 \
      (Current).first_column = YYRHSLOC(Rhs, 1).first_column; \
      (Current).last_column = YYRHSLOC(Rhs, N).last_column;   \
    } else {                                                  \
      (Current).filename = NULL;                              \
      (Current).line = YYRHSLOC(Rhs, 0).line;                 \
      (Current).first_column = (Current).last_column =        \
          YYRHSLOC(Rhs, 0).last_column;                       \
    }                                                         \
  while (0)

#define YYMALLOC(size) wasm_alloc(parser->allocator, size, WASM_DEFAULT_ALIGN)
#define YYFREE(p) wasm_free(parser->allocator, p)

static WasmFunc* new_func(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmFunc), WASM_DEFAULT_ALIGN);
}

static WasmCommand* new_command(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmCommand), WASM_DEFAULT_ALIGN);
}

static WasmModule* new_module(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmModule), WASM_DEFAULT_ALIGN);
}

static WasmImport* new_import(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmImport), WASM_DEFAULT_ALIGN);
}

static int read_int32(const char* s, const char* end, uint32_t* out,
                      int allow_signed);
static int read_int64(const char* s, const char* end, uint64_t* out);
static int read_uint64(const char* s, const char* end, uint64_t* out);
static int read_float(const char* s, const char* end, float* out);
static int read_double(const char* s, const char* end, double* out);
static int read_const(WasmType type, const char* s, const char* end,
                      WasmConst* out);
static WasmResult dup_string_contents(WasmAllocator*, WasmStringSlice* text,
                                      void** out_data, size_t* out_size);

#define wasm_parser_lex wasm_lexer_lex


#line 160 "src/wasm-bison-parser.c" /* yacc.c:339  */

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
   by #include "wasm-bison-parser.h".  */
#ifndef YY_WASM_PARSER_SRC_WASM_BISON_PARSER_H_INCLUDED
# define YY_WASM_PARSER_SRC_WASM_BISON_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef WASM_PARSER_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define WASM_PARSER_DEBUG 1
#  else
#   define WASM_PARSER_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define WASM_PARSER_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined WASM_PARSER_DEBUG */
#if WASM_PARSER_DEBUG
extern int wasm_parser_debug;
#endif

/* Token type.  */
#ifndef WASM_PARSER_TOKENTYPE
# define WASM_PARSER_TOKENTYPE
  enum wasm_parser_tokentype
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
    WASM_TOKEN_TYPE_SELECT = 289,
    WASM_TOKEN_TYPE_FUNC = 290,
    WASM_TOKEN_TYPE_START = 291,
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
    WASM_TOKEN_TYPE_ASSERT_INVALID = 305,
    WASM_TOKEN_TYPE_ASSERT_RETURN = 306,
    WASM_TOKEN_TYPE_ASSERT_RETURN_NAN = 307,
    WASM_TOKEN_TYPE_ASSERT_TRAP = 308,
    WASM_TOKEN_TYPE_INVOKE = 309,
    WASM_TOKEN_TYPE_LOW = 310
  };
#endif

/* Value type.  */
#if ! defined WASM_PARSER_STYPE && ! defined WASM_PARSER_STYPE_IS_DECLARED
typedef WasmToken WASM_PARSER_STYPE;
# define WASM_PARSER_STYPE_IS_TRIVIAL 1
# define WASM_PARSER_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined WASM_PARSER_LTYPE && ! defined WASM_PARSER_LTYPE_IS_DECLARED
typedef struct WASM_PARSER_LTYPE WASM_PARSER_LTYPE;
struct WASM_PARSER_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define WASM_PARSER_LTYPE_IS_DECLARED 1
# define WASM_PARSER_LTYPE_IS_TRIVIAL 1
#endif



int wasm_parser_parse (WasmLexer lexer, WasmParser* parser);

#endif /* !YY_WASM_PARSER_SRC_WASM_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 289 "src/wasm-bison-parser.c" /* yacc.c:358  */

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
         || (defined WASM_PARSER_LTYPE_IS_TRIVIAL && WASM_PARSER_LTYPE_IS_TRIVIAL \
             && defined WASM_PARSER_STYPE_IS_TRIVIAL && WASM_PARSER_STYPE_IS_TRIVIAL)))

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
#define YYLAST   629

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  184
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  340

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   310

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
      55
};

#if WASM_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   201,   201,   202,   208,   209,   213,   217,   224,   225,
     229,   238,   246,   247,   253,   257,   261,   268,   269,   273,
     274,   281,   282,   290,   293,   297,   302,   308,   315,   321,
     327,   334,   341,   348,   356,   362,   367,   388,   394,   400,
     407,   412,   418,   426,   435,   444,   450,   457,   465,   472,
     478,   482,   486,   493,   494,   497,   501,   507,   508,   512,
     516,   522,   523,   529,   530,   533,   534,   544,   549,   558,
     563,   574,   577,   582,   591,   596,   607,   610,   614,   619,
     625,   632,   640,   649,   659,   668,   676,   685,   693,   700,
     708,   717,   725,   732,   740,   747,   753,   760,   766,   773,
     781,   790,   798,   805,   813,   820,   826,   833,   841,   848,
     854,   859,   865,   872,   880,   889,   897,   904,   912,   919,
     925,   932,   940,   947,   953,   960,   966,   971,   977,   984,
     992,   999,  1005,  1012,  1018,  1023,  1029,  1036,  1042,  1047,
    1053,  1060,  1066,  1070,  1079,  1087,  1088,  1095,  1104,  1113,
    1119,  1128,  1132,  1139,  1143,  1150,  1158,  1165,  1176,  1183,
    1189,  1190,  1199,  1208,  1216,  1224,  1232,  1240,  1248,  1258,
    1346,  1352,  1359,  1366,  1374,  1381,  1391,  1392,  1400,  1409,
    1410,  1413,  1414,  1421,  1430
};
#endif

#if WASM_PARSER_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "INT", "FLOAT",
  "TEXT", "VAR", "VALUE_TYPE", "NOP", "BLOCK", "IF", "IF_ELSE", "LOOP",
  "BR", "BR_IF", "TABLESWITCH", "CASE", "CALL", "CALL_IMPORT",
  "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "LOAD", "STORE",
  "OFFSET", "ALIGN", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT",
  "SELECT", "FUNC", "START", "TYPE", "PARAM", "RESULT", "LOCAL", "MODULE",
  "MEMORY", "SEGMENT", "IMPORT", "EXPORT", "TABLE", "UNREACHABLE",
  "MEMORY_SIZE", "GROW_MEMORY", "ASSERT_INVALID", "ASSERT_RETURN",
  "ASSERT_RETURN_NAN", "ASSERT_TRAP", "INVOKE", "LOW", "$accept",
  "value_type_list", "func_type", "literal", "var", "var_list", "bind_var",
  "quoted_text", "string_contents", "labeling", "offset", "align", "expr",
  "expr1", "expr_opt", "non_empty_expr_list", "expr_list", "target",
  "target_list", "case", "case_list", "param_list", "result", "local_list",
  "type_use", "func_info", "func", "start", "segment_address", "segment",
  "segment_list", "initial_size", "max_size", "memory", "type_def",
  "table", "import", "export", "export_memory", "module_fields", "module",
  "cmd", "cmd_list", "const", "const_opt", "const_list", "script",
  "script_start", YY_NULLPTR
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
     305,   306,   307,   308,   309,   310
};
# endif

#define YYPACT_NINF -146

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-146)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -146,     4,  -146,     9,    75,  -146,  -146,  -146,  -146,     8,
      25,    51,    58,    62,    41,    53,    62,    44,    49,    69,
    -146,  -146,   210,  -146,  -146,  -146,  -146,  -146,  -146,  -146,
    -146,  -146,   126,    62,    62,    62,   107,    19,    50,    34,
     132,   127,    62,  -146,  -146,  -146,  -146,  -146,   103,  -146,
    -146,   358,  -146,   140,  -146,   142,   145,   165,   173,   175,
     179,  -146,  -146,   180,   150,   183,  -146,   187,    62,    62,
      10,    21,   147,   152,   156,   159,  -146,   185,   142,   142,
     185,    48,    50,   185,    50,    50,    50,   142,    50,    50,
     169,   169,   159,   142,   142,   142,   142,   142,    50,   185,
     186,   185,  -146,  -146,   142,   193,   142,   145,   165,   173,
     175,   580,  -146,   420,   142,   165,   173,   500,   142,   173,
     540,   142,   460,   142,   145,   165,   173,  -146,  -146,   195,
     170,  -146,   158,  -146,    62,   197,   203,   205,  -146,  -146,
     207,   209,    62,  -146,  -146,   211,  -146,   142,   142,   142,
     185,   142,   142,  -146,  -146,   142,   142,   142,   142,   142,
    -146,  -146,   142,  -146,   190,   190,  -146,  -146,   142,   142,
    -146,   142,   215,    61,   212,   218,    64,   219,  -146,  -146,
     142,   165,   173,   142,   173,   142,   142,   145,   165,   173,
     185,   142,   173,   142,   142,   185,   142,   165,   173,   142,
     173,   142,   128,   223,   195,   189,  -146,  -146,   168,   197,
      81,   230,   231,  -146,  -146,  -146,   235,  -146,   238,  -146,
     142,  -146,   142,   142,   142,  -146,  -146,   142,   245,  -146,
    -146,   142,  -146,  -146,   142,   142,  -146,  -146,   142,  -146,
    -146,  -146,   246,  -146,  -146,   247,   142,   173,   142,   142,
     142,   165,   173,   142,   173,   142,    71,   244,   142,    84,
     250,   142,   173,   142,   142,  -146,   252,   258,   259,   261,
    -146,   263,   264,  -146,  -146,  -146,  -146,  -146,  -146,  -146,
     224,  -146,  -146,   142,  -146,  -146,  -146,   142,   142,   173,
     142,   142,  -146,   265,  -146,   267,   142,    87,   268,  -146,
     269,  -146,   271,  -146,  -146,  -146,  -146,   142,  -146,  -146,
     276,  -146,  -146,  -146,   277,   178,   241,  -146,    66,   281,
    -146,   278,    50,    50,  -146,   284,   285,   286,   282,  -146,
    -146,  -146,   280,  -146,    92,   142,   290,   291,  -146,  -146
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     176,   183,   184,     0,     0,   170,   177,     1,   160,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,   181,     0,   169,   161,   168,   167,   166,   165,   162,
     163,   164,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,     0,    12,   172,   181,   181,   181,     0,   171,
     182,     0,    14,    78,    55,   140,   126,   134,   138,   110,
       0,    10,    11,     0,     0,     0,   147,   145,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    17,     0,     0,
      17,    53,     0,    17,     0,     0,     0,    53,     0,     0,
      19,    19,     0,     0,     0,     0,     0,     0,     0,     2,
       0,     2,    50,    51,     0,     0,   109,    97,   105,    95,
      79,     0,    56,     0,   133,   127,   131,     0,   137,   135,
       0,   139,     0,   125,   111,   119,   123,   141,   142,     4,
       0,   148,     0,   145,     0,     4,     0,     0,   153,    13,
     179,     0,     0,     8,     9,     0,    18,    25,     0,     0,
      18,    57,    53,    54,    33,     0,     0,    57,    57,     0,
      35,    40,     0,    20,    21,    21,    44,    45,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    52,    23,
     104,    98,   102,   108,   106,    96,    94,    80,    88,    92,
       2,   130,   128,   132,   136,     2,   118,   112,   116,   122,
     120,   124,     0,     0,     4,     0,   150,   146,     0,     4,
       0,     0,     0,   159,   158,   180,     0,   174,     0,   178,
      26,    28,     0,    57,    58,    31,    34,    29,     0,    37,
      38,    57,    41,    22,     0,     0,    46,    48,     0,    76,
      67,     3,     0,    71,    72,     0,   101,    99,   103,   107,
      87,    81,    85,    91,    89,    93,     0,     0,   129,     0,
       0,   115,   113,   117,   121,     2,     0,     0,     0,     0,
     149,     0,     0,   156,   154,   173,   175,    27,    32,    30,
       0,    39,    42,     0,    47,    68,    73,   100,    84,    82,
      86,    90,    69,     0,    74,     0,   114,     0,     0,   151,
       0,   143,     0,   157,   155,    61,    43,    83,    70,    75,
       5,     7,   152,    16,     0,     0,     0,   144,     0,     0,
      62,     0,     0,     0,    65,     0,     0,     0,    36,     6,
      60,    59,     0,    66,    57,    57,     0,     0,    63,    64
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -146,   -91,  -132,   204,   -65,  -146,   -37,   196,  -146,     2,
     208,   135,   -47,  -146,   -73,   167,  -145,   -17,  -146,  -146,
    -146,   -23,     7,   -18,   -52,  -146,  -146,  -146,  -146,  -146,
     171,  -146,  -146,  -146,  -146,  -146,  -146,  -146,  -146,  -146,
     292,  -146,  -146,   163,  -146,    95,  -146,  -146
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   173,   203,   145,    63,    71,   146,    21,   314,   147,
     164,   234,    54,   105,   154,   224,   225,   320,   315,   333,
     328,    56,    57,    58,    59,    60,    24,    25,   302,   207,
     132,    67,   133,    26,    27,    28,    29,    30,    31,    14,
       5,     6,     1,    50,   216,    36,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      53,   110,    65,   211,    68,   137,   139,     4,   112,     7,
     176,    15,   229,   230,   160,    61,   152,   155,    62,   157,
     158,   159,    51,   161,   162,   138,    61,    52,    17,    62,
     107,   148,   149,   172,   153,   109,   124,    64,   116,   119,
     153,   126,    52,   150,    22,    23,   167,   168,   169,   170,
     171,   111,   136,    61,    18,    61,    62,   178,    62,   112,
     108,    19,   174,   115,   177,   240,   125,   112,   244,    20,
     241,   112,   268,   241,   112,   292,   112,   271,   278,   226,
     241,   322,   151,   212,   323,   156,   281,   187,   294,   182,
     184,   310,   189,   241,     8,   111,   241,   192,    33,   256,
      52,   221,   222,    34,   259,   153,   198,   200,   227,   228,
      48,    49,   231,   223,   181,   232,     8,   188,    98,   265,
     266,   236,   237,    35,   238,     9,    10,    11,    12,    13,
      44,   197,    75,   112,    20,    52,   112,    66,   112,   112,
      72,    73,    74,    51,   112,   111,   112,   112,   113,   112,
      48,   140,   112,   257,   112,    48,   141,   272,   260,    48,
     142,   205,   206,   247,   143,   144,   265,   266,   117,   252,
     254,   205,   270,   112,   297,   277,   120,   112,   122,   262,
     279,   318,   319,   127,   128,   129,   130,   282,   283,   336,
     337,   284,   131,    52,   251,   175,   163,   179,   202,   112,
     210,   112,   112,   112,    55,   204,   112,   213,   112,   214,
      48,   112,    32,   217,   112,   219,   112,   112,   233,   239,
     106,   242,   243,   114,   118,   121,   123,   267,   245,    45,
      46,    47,   269,   289,   273,   274,   306,    69,    70,   275,
     112,   112,   276,   112,   112,    37,    38,    39,   280,   112,
     285,   286,    40,   293,    41,    42,    43,   326,   327,   295,
     112,   298,   299,   300,   134,   135,   301,   303,   304,   308,
     305,   309,   311,   312,   180,   183,   185,   186,   313,   316,
     321,   317,   191,   193,   318,   332,   194,   325,   329,   330,
     331,   196,   199,   201,   338,   339,   166,   335,   334,   165,
     235,    16,   324,   215,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   246,   248,
       0,   249,     0,     0,   250,   253,   255,     0,     0,   258,
       0,     0,     0,     0,   261,   263,     0,   264,    76,    77,
      78,    79,    80,    81,    82,    83,     0,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,    92,    93,    94,
      95,    96,    97,     0,     0,    98,    99,   100,   101,     0,
       0,     0,     0,     0,     0,   102,   103,   104,     0,     0,
       0,     0,     0,     0,   287,     0,     0,     0,   288,   290,
       0,   291,     0,     0,     0,     0,     0,     0,     0,   296,
      76,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,     0,   307,     0,   190,   100,
     101,     0,     0,     0,     0,     0,     0,   102,   103,   104,
      76,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,     0,     0,     0,    99,   100,
     101,     0,     0,     0,     0,     0,     0,   102,   103,   104,
      76,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,     0,     0,   102,   103,   104,
      76,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,     0,     0,     0,     0,     0,
     195,     0,     0,     0,     0,     0,     0,   102,   103,   104,
      76,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,    92,
      93,    94,    95,    96,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   102,   103,   104
};

static const yytype_int16 yycheck[] =
{
      37,    53,    39,   135,    41,    70,    71,     3,    55,     0,
     101,     3,   157,   158,    87,     5,    81,    82,     8,    84,
      85,    86,     3,    88,    89,     4,     5,     8,     3,     8,
      53,    78,    79,    98,    81,    53,    59,     3,    56,    57,
      87,    59,     8,    80,     3,     4,    93,    94,    95,    96,
      97,     3,    42,     5,     3,     5,     8,   104,     8,   106,
      53,     3,    99,    56,   101,     4,    59,   114,     4,     7,
       9,   118,   204,     9,   121,     4,   123,   209,   223,   152,
       9,    15,    80,   135,    18,    83,   231,   110,     4,   107,
     108,     4,   110,     9,    41,     3,     9,   115,    54,   190,
       8,   148,   149,    54,   195,   152,   124,   125,   155,   156,
       3,     4,   159,   150,   107,   162,    41,   110,    37,    38,
      39,   168,   169,    54,   171,    50,    51,    52,    53,    54,
       4,   124,    29,   180,     7,     8,   183,     5,   185,   186,
      45,    46,    47,     3,   191,     3,   193,   194,     3,   196,
       3,     4,   199,   190,   201,     3,     4,   209,   195,     3,
       4,     3,     4,   181,     5,     6,    38,    39,     3,   187,
     188,     3,     4,   220,   265,   222,     3,   224,     3,   197,
     227,     3,     4,     4,     4,    35,     3,   234,   235,   334,
     335,   238,     5,     8,   187,     9,    27,     4,     3,   246,
       3,   248,   249,   250,    37,    35,   253,     4,   255,     4,
       3,   258,    16,     4,   261,     4,   263,   264,    28,     4,
      53,     9,     4,    56,    57,    58,    59,     4,     9,    33,
      34,    35,    43,   251,     4,     4,   283,    41,    42,     4,
     287,   288,     4,   290,   291,    35,    36,    37,     3,   296,
       4,     4,    42,     9,    44,    45,    46,   322,   323,     9,
     307,     9,     4,     4,    68,    69,     5,     4,     4,     4,
      46,     4,     4,     4,   107,   108,   109,   110,     7,     3,
      39,     4,   115,   116,     3,     3,   119,     9,     4,     4,
       4,   124,   125,   126,     4,     4,    92,   334,    18,    91,
     165,     9,   319,   140,   133,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,    -1,    -1,    -1,   197,   198,    -1,   200,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    29,    30,    31,
      32,    33,    34,    -1,    -1,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    -1,    -1,   247,    -1,    -1,    -1,   251,   252,
      -1,   254,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   262,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    -1,   289,    -1,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    98,   102,   103,     3,    96,    97,     0,    41,    50,
      51,    52,    53,    54,    95,     3,    96,     3,     3,     3,
       7,    63,     3,     4,    82,    83,    89,    90,    91,    92,
      93,    94,    63,    54,    54,    54,   101,    35,    36,    37,
      42,    44,    45,    46,     4,    63,    63,    63,     3,     4,
      99,     3,     8,    62,    68,    71,    77,    78,    79,    80,
      81,     5,     8,    60,     3,    62,     5,    87,    62,    63,
      63,    61,   101,   101,   101,    29,    10,    11,    12,    13,
      14,    15,    16,    17,    19,    20,    21,    22,    23,    24,
      25,    26,    29,    30,    31,    32,    33,    34,    37,    38,
      39,    40,    47,    48,    49,    69,    71,    77,    78,    79,
      80,     3,    68,     3,    71,    78,    79,     3,    71,    79,
       3,    71,     3,    71,    77,    78,    79,     4,     4,    35,
       3,     5,    86,    88,    63,    63,    42,    60,     4,    60,
       4,     4,     4,     5,     6,    59,    62,    65,    68,    68,
      62,    65,    60,    68,    70,    60,    65,    60,    60,    60,
      70,    60,    60,    27,    66,    66,    59,    68,    68,    68,
      68,    68,    60,    57,    62,     9,    57,    62,    68,     4,
      71,    78,    79,    71,    79,    71,    71,    77,    78,    79,
      38,    71,    79,    71,    71,    40,    71,    78,    79,    71,
      79,    71,     3,    58,    35,     3,     4,    85,    86,    63,
       3,    58,    80,     4,     4,    99,   100,     4,    63,     4,
      71,    68,    68,    62,    71,    72,    70,    68,    68,    72,
      72,    68,    68,    28,    67,    67,    68,    68,    68,     4,
       4,     9,     9,     4,     4,     9,    71,    79,    71,    71,
      71,    78,    79,    71,    79,    71,    57,    62,    71,    57,
      62,    71,    79,    71,    71,    38,    39,     4,    58,    43,
       4,    58,    80,     4,     4,     4,     4,    68,    72,    68,
       3,    72,    68,    68,    68,     4,     4,    71,    71,    79,
      71,    71,     4,     9,     4,     9,    71,    57,     9,     4,
       4,     5,    84,     4,     4,    46,    68,    71,     4,     4,
       4,     4,     4,     7,    64,    74,     3,     4,     3,     4,
      73,    39,    15,    18,    73,     9,    60,    60,    76,     4,
       4,     4,     3,    75,    18,    62,    72,    72,     4,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    57,    57,    58,    58,    58,    58,    59,    59,
      60,    60,    61,    61,    62,    63,    64,    65,    65,    66,
      66,    67,    67,    68,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    70,    70,    71,    71,    72,    72,    73,
      73,    74,    74,    75,    75,    76,    76,    77,    77,    77,
      77,    78,    79,    79,    79,    79,    80,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    82,    83,    84,    85,    86,    86,    87,    88,    89,
      89,    90,    90,    91,    92,    92,    92,    92,    93,    94,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    96,
      97,    97,    97,    97,    97,    97,    98,    98,    99,   100,
     100,   101,   101,   102,   103
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     4,     8,     4,     1,     1,
       1,     1,     0,     2,     1,     1,     1,     0,     1,     0,
       1,     0,     1,     3,     1,     2,     3,     4,     3,     3,
       4,     3,     4,     2,     3,     2,     9,     3,     3,     4,
       2,     3,     4,     5,     2,     2,     3,     4,     3,     2,
       1,     1,     2,     0,     1,     1,     2,     0,     1,     4,
       4,     0,     2,     4,     5,     0,     2,     4,     5,     5,
       6,     4,     4,     5,     5,     6,     4,     0,     1,     2,
       3,     4,     5,     6,     5,     4,     5,     4,     3,     4,
       5,     4,     3,     4,     3,     2,     3,     2,     3,     4,
       5,     4,     3,     4,     3,     2,     3,     4,     3,     2,
       1,     2,     3,     4,     5,     4,     3,     4,     3,     2,
       3,     4,     3,     2,     3,     2,     1,     2,     3,     4,
       3,     2,     3,     2,     1,     2,     3,     2,     1,     2,
       1,     4,     4,     1,     5,     0,     2,     1,     1,     6,
       5,     7,     8,     4,     6,     7,     6,     7,     5,     5,
       0,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       1,     5,     5,     9,     8,     9,     0,     2,     4,     0,
       1,     0,     2,     1,     1
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
#if WASM_PARSER_DEBUG

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
# if defined WASM_PARSER_LTYPE_IS_TRIVIAL && WASM_PARSER_LTYPE_IS_TRIVIAL

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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmLexer lexer, WasmParser* parser)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmLexer lexer, WasmParser* parser)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, WasmLexer lexer, WasmParser* parser)
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
#else /* !WASM_PARSER_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !WASM_PARSER_DEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, WasmLexer lexer, WasmParser* parser)
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
          case 57: /* value_type_list  */
#line 166 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(parser->allocator, &((*yyvaluep).types)); }
#line 1488 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 58: /* func_type  */
#line 180 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(parser->allocator, &((*yyvaluep).func_sig)); }
#line 1494 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 59: /* literal  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1500 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 60: /* var  */
#line 167 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(parser->allocator, &((*yyvaluep).var)); }
#line 1506 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 61: /* var_list  */
#line 168 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1512 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 62: /* bind_var  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1518 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 63: /* quoted_text  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1524 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 64: /* string_contents  */
#line 177 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1530 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 65: /* labeling  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1536 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 68: /* expr  */
#line 169 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1542 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 69: /* expr1  */
#line 169 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1548 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 70: /* expr_opt  */
#line 169 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1554 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 71: /* non_empty_expr_list  */
#line 170 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(parser->allocator, &((*yyvaluep).exprs)); }
#line 1560 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 72: /* expr_list  */
#line 170 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(parser->allocator, &((*yyvaluep).exprs)); }
#line 1566 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 73: /* target  */
#line 171 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target(parser->allocator, &((*yyvaluep).target)); }
#line 1572 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 74: /* target_list  */
#line 172 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_target_vector_and_elements(parser->allocator, &((*yyvaluep).targets)); }
#line 1578 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 75: /* case  */
#line 173 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case(parser->allocator, &((*yyvaluep).case_)); }
#line 1584 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 76: /* case_list  */
#line 174 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_case_vector_and_elements(parser->allocator, &((*yyvaluep).cases)); }
#line 1590 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 77: /* param_list  */
#line 175 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(parser->allocator, &((*yyvaluep).type_bindings)); }
#line 1596 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 79: /* local_list  */
#line 175 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_bindings(parser->allocator, &((*yyvaluep).type_bindings)); }
#line 1602 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 81: /* func_info  */
#line 176 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1608 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 82: /* func  */
#line 176 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1614 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 85: /* segment  */
#line 177 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1620 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 86: /* segment_list  */
#line 178 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(parser->allocator, &((*yyvaluep).segments)); }
#line 1626 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 89: /* memory  */
#line 179 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(parser->allocator, &((*yyvaluep).memory)); }
#line 1632 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 90: /* type_def  */
#line 181 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(parser->allocator, &((*yyvaluep).func_type)); }
#line 1638 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 91: /* table  */
#line 168 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1644 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 92: /* import  */
#line 182 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(parser->allocator, ((*yyvaluep).import)); wasm_free(parser->allocator, ((*yyvaluep).import)); }
#line 1650 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 93: /* export  */
#line 183 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(parser->allocator, &((*yyvaluep).export_)); }
#line 1656 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 95: /* module_fields  */
#line 184 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module_field_vector_and_elements(parser->allocator, &((*yyvaluep).module_fields)); }
#line 1662 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 96: /* module  */
#line 185 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1668 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 97: /* cmd  */
#line 187 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(parser->allocator, ((*yyvaluep).command)); wasm_free(parser->allocator, ((*yyvaluep).command)); }
#line 1674 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 98: /* cmd_list  */
#line 188 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(parser->allocator, &((*yyvaluep).commands)); }
#line 1680 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 101: /* const_list  */
#line 186 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(parser->allocator, &((*yyvaluep).consts)); }
#line 1686 "src/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 102: /* script  */
#line 189 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1692 "src/wasm-bison-parser.c" /* yacc.c:1257  */
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
yyparse (WasmLexer lexer, WasmParser* parser)
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
# if defined WASM_PARSER_LTYPE_IS_TRIVIAL && WASM_PARSER_LTYPE_IS_TRIVIAL
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
      yychar = yylex (&yylval, &yylloc, lexer, parser);
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
#line 201 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.types)); }
#line 1986 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 202 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.types), &(yyvsp[0].type)));
    }
#line 1995 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 208 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); }
#line 2001 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 209 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 2010 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 213 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 2019 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 217 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 2025 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 224 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2031 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 225 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2037 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 229 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &index, 0))
        wasm_parser_error(&(yylsp[0]), lexer, parser, "invalid int %.*s", (yyvsp[0].text).length,
                          (yyvsp[0].text).start);
      (yyval.var).index = index;
    }
#line 2051 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 238 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
      CHECK_ALLOC_STR((yyval.var).name);
    }
#line 2062 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 246 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.vars)); }
#line 2068 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 247 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      CHECK_ALLOC(wasm_append_var_value(parser->allocator, &(yyval.vars), &(yyvsp[0].var)));
    }
#line 2077 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 253 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2083 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 257 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { DUPQUOTEDTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 2089 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 261 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOC(dup_string_contents(parser->allocator, &(yyvsp[0].text), &(yyval.segment).data,
                                      &(yyval.segment).size));
    }
#line 2098 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 268 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.text)); }
#line 2104 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 269 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.text) = (yyvsp[0].text); }
#line 2110 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 273 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2116 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 274 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (!read_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64)))
        wasm_parser_error(&(yylsp[0]), lexer, parser, "invalid offset \"%.*s\"",
                          (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 2126 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 281 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = WASM_USE_NATURAL_ALIGNMENT; }
#line 2132 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 282 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_parser_error(&(yylsp[0]), lexer, parser, "invalid alignment \"%.*s\"",
                          (yyvsp[0].text).length, (yyvsp[0].text).start);
    }
#line 2142 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 290 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 2148 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 293 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_NOP);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2157 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 297 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_block_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[0].text);
    }
#line 2167 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 302 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_block_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->block.label = (yyvsp[-1].text);
      (yyval.expr)->block.exprs = (yyvsp[0].exprs);
    }
#line 2178 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 308 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      (yyval.expr)->if_else.true_ = (yyvsp[-1].expr);
      (yyval.expr)->if_else.false_ = (yyvsp[0].expr);
    }
#line 2190 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 315 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_.cond = (yyvsp[-1].expr);
      (yyval.expr)->if_.true_ = (yyvsp[0].expr);
    }
#line 2201 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 321 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2212 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 327 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.var = (yyvsp[-2].var);
      (yyval.expr)->br_if.expr = (yyvsp[-1].expr);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2224 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 334 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      ZEROMEM((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2236 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 341 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2248 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 348 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var.loc = (yylsp[-1]);
      (yyval.expr)->br.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.expr)->br.var.index = 0;
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2261 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 356 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2272 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 362 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_return_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2282 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 367 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_tableswitch_expr(parser->allocator);
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
          WasmBinding* binding = wasm_insert_binding(
              parser->allocator, &(yyval.expr)->tableswitch.case_bindings, &case_->label);
          CHECK_ALLOC_NULL(binding);
          binding->loc = case_->loc;
          binding->index = i;
        }
      }
    }
#line 2308 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 388 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2319 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 394 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_import_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2330 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 400 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_indirect_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.args = (yyvsp[0].exprs);
    }
#line 2342 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 407 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_get_local_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2352 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 412 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_set_local_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2363 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 418 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_load_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load.op = (yyvsp[-3].mem);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2376 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 426 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_store_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store.op = (yyvsp[-4].mem);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2390 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 435 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_const_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (!read_const((yyvsp[-1].type), (yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.expr)->const_))
        wasm_parser_error(&(yylsp[0]), lexer, parser, "invalid literal \"%.*s\"",
                          (yyvsp[0].text).length, (yyvsp[0].text).start);
      wasm_free(parser->allocator, (char*)(yyvsp[0].text).start);
    }
#line 2404 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 444 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_unary_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->unary.op = (yyvsp[-1].unary);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2415 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 450 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_binary_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->binary.op = (yyvsp[-2].binary);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2427 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 457 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_select_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->select.type = (yyvsp[-3].type);
      (yyval.expr)->select.true_ = (yyvsp[-2].expr);
      (yyval.expr)->select.false_ = (yyvsp[-1].expr);
      (yyval.expr)->select.cond = (yyvsp[0].expr);
    }
#line 2440 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 465 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_compare_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->compare.op = (yyvsp[-2].compare);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2452 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 472 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_convert_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->convert.op = (yyvsp[-1].convert);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2463 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 478 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_UNREACHABLE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2472 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 482 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_MEMORY_SIZE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2481 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 486 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_grow_memory_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2491 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 493 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2497 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 497 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.exprs));
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2506 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 501 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.exprs) = (yyvsp[-1].exprs);
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2515 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 507 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.exprs)); }
#line 2521 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 512 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_CASE;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2530 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 516 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.target).type = WASM_TARGET_TYPE_BR;
      (yyval.target).var = (yyvsp[-1].var);
    }
#line 2539 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 522 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.targets)); }
#line 2545 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 523 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.targets) = (yyvsp[-1].targets);
      CHECK_ALLOC(wasm_append_target_value(parser->allocator, &(yyval.targets), &(yyvsp[0].target)));
    }
#line 2554 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 529 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.case_).label); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2560 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 530 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.case_).label = (yyvsp[-2].text); (yyval.case_).exprs = (yyvsp[-1].exprs); }
#line 2566 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 533 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.cases)); }
#line 2572 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 534 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.cases) = (yyvsp[-1].cases);
      CHECK_ALLOC(wasm_append_case_value(parser->allocator, &(yyval.cases), &(yyvsp[0].case_)));
    }
#line 2581 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 544 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(parser->allocator, &(yyvsp[-1].types));
    }
#line 2591 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 549 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &(yyval.type_bindings).bindings, &(yyvsp[-2].text));
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2605 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 558 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(parser->allocator, &(yyvsp[-1].types));
    }
#line 2615 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 563 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &(yyval.type_bindings).bindings, &(yyvsp[-2].text));
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2629 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 574 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 2635 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 577 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(parser->allocator, &(yyvsp[-1].types));
    }
#line 2645 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 582 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.type_bindings));
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &(yyval.type_bindings).bindings, &(yyvsp[-2].text));
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2659 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 591 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-4].type_bindings);
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].types)));
      wasm_destroy_type_vector(parser->allocator, &(yyvsp[-1].types));
    }
#line 2669 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 596 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.type_bindings) = (yyvsp[-5].type_bindings);
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &(yyval.type_bindings).bindings, &(yyvsp[-2].text));
      CHECK_ALLOC_NULL(binding);
      binding->loc = (yylsp[-3]);
      binding->index = (yyval.type_bindings).types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.type_bindings).types, &(yyvsp[-1].type)));
    }
#line 2683 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 607 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2689 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 610 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
    }
#line 2698 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 614 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[0].text);
    }
#line 2708 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 619 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->name = (yyvsp[-1].text);
      (yyval.func)->type_var = (yyvsp[0].var);
    }
#line 2719 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 625 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->params = (yyvsp[0].type_bindings);
    }
#line 2731 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 632 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 2744 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 640 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-4].text);
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2758 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 649 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-5].text);
      (yyval.func)->type_var = (yyvsp[-4].var);
      (yyval.func)->params = (yyvsp[-3].type_bindings);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2773 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 659 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-4].text);
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2787 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 668 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2800 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 676 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-4].text);
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2814 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 685 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2827 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 693 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 2839 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 700 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2852 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 708 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-4].text);
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2866 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 717 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2879 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 725 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2891 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 732 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2904 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 740 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2916 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 747 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-1].text);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2927 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 753 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2939 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 760 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-1].text);
      (yyval.func)->params = (yyvsp[0].type_bindings);
    }
#line 2950 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 766 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 2962 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 773 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 2975 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 781 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-4].text);
      (yyval.func)->params = (yyvsp[-3].type_bindings);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 2989 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 790 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3002 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 798 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3014 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 805 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3027 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 813 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3039 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 820 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-1].text);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 3050 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 826 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3062 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 833 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-3].text);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3075 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 841 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3087 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 848 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-1].text);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3098 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 854 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->type_var = (yyvsp[0].var);
    }
#line 3108 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 859 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->params = (yyvsp[0].type_bindings);
    }
#line 3119 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 865 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 3131 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 872 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3144 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 880 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-4].var);
      (yyval.func)->params = (yyvsp[-3].type_bindings);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3158 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 889 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3171 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 897 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3183 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 904 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3196 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 912 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3208 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 919 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 3219 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 925 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3231 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 932 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-3].var);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3244 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 940 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3256 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 947 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3267 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 953 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->type_var = (yyvsp[-2].var);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3279 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 960 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->type_var = (yyvsp[-1].var);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3290 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 966 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[0].type_bindings);
    }
#line 3300 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 971 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 3311 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 977 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3323 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 984 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-3].type_bindings);
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3336 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 992 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3348 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 999 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3359 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1005 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-2].type_bindings);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3371 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1012 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->params = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3382 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1018 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->result_type = (yyvsp[0].type);
    }
#line 3392 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1023 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3403 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1029 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->result_type = (yyvsp[-2].type);
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3415 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1036 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->result_type = (yyvsp[-1].type);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3426 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1042 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->locals = (yyvsp[0].type_bindings);
    }
#line 3436 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1047 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->locals = (yyvsp[-1].type_bindings);
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3447 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1053 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      (yyval.func)->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      (yyval.func)->exprs = (yyvsp[0].exprs);
    }
#line 3457 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1060 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.func) = (yyvsp[-1].func); (yyval.func)->loc = (yylsp[-2]); }
#line 3463 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1066 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 3469 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1070 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid memory segment address \"%.*s\"", (yyvsp[0].text).length,
                          (yyvsp[0].text).start);
    }
#line 3480 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1079 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      (yyval.segment).addr = (yyvsp[-2].u32);
    }
#line 3491 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1087 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.segments)); }
#line 3497 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1088 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      CHECK_ALLOC(wasm_append_segment_value(parser->allocator, &(yyval.segments), &(yyvsp[0].segment)));
    }
#line 3506 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1095 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid initial memory size \"%.*s\"", (yyvsp[0].text).length,
                          (yyvsp[0].text).start);
    }
#line 3517 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1104 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (!read_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32), 0))
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid max memory size \"%.*s\"", (yyvsp[0].text).length,
                          (yyvsp[0].text).start);
    }
#line 3528 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1113 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      (yyval.memory).initial_size = (yyvsp[-3].u32);
      (yyval.memory).max_size = (yyvsp[-2].u32);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3539 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1119 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      (yyval.memory).initial_size = (yyvsp[-2].u32);
      (yyval.memory).max_size = (yyval.memory).initial_size;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3550 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1128 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      ZEROMEM((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3559 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1132 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3568 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1139 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 3574 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1143 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->type_var = (yyvsp[-1].var);
    }
#line 3586 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1150 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->import_type = WASM_IMPORT_HAS_TYPE;
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->type_var = (yyvsp[-1].var);
    }
#line 3599 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1158 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->func_sig = (yyvsp[-1].func_sig);
    }
#line 3611 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1165 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->func_sig = (yyvsp[-1].func_sig);
    }
#line 3624 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1176 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_).name = (yyvsp[-2].text);
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3633 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1183 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_memory).name = (yyvsp[-2].text);
    }
#line 3641 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1189 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.module_fields)); }
#line 3647 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1190 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *(yyvsp[0].func);
      wasm_free(parser->allocator, (yyvsp[0].func));
    }
#line 3661 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1199 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *(yyvsp[0].import);
      wasm_free(parser->allocator, (yyvsp[0].import));
    }
#line 3675 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1208 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = (yyvsp[0].export_);
    }
#line 3688 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1216 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = (yyvsp[0].export_memory);
    }
#line 3701 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1224 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
    }
#line 3714 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1232 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);
    }
#line 3727 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1240 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
    }
#line 3740 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1248 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, &(yyval.module_fields));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = (yyvsp[0].var);
    }
#line 3753 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1258 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new_module(parser->allocator);
      (yyval.module)->loc = (yylsp[-2]);
      (yyval.module)->fields = (yyvsp[-1].module_fields);
      /* clear the start function */
      (yyval.module)->start.type = WASM_VAR_TYPE_INDEX;
      (yyval.module)->start.index = -1;

      /* cache values */
      int i;
      for (i = 0; i < (yyval.module)->fields.size; ++i) {
        WasmModuleField* field = &(yyval.module)->fields.data[i];
        switch (field->type) {
          case WASM_MODULE_FIELD_TYPE_FUNC: {
            WasmFuncPtr func_ptr = &field->func;
            CHECK_ALLOC(wasm_append_func_ptr_value(parser->allocator,
                                                   &(yyval.module)->funcs, &func_ptr));
            if (field->func.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &(yyval.module)->func_bindings, &field->func.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = (yyval.module)->funcs.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_IMPORT: {
            WasmImportPtr import_ptr = &field->import;
            CHECK_ALLOC(wasm_append_import_ptr_value(
                parser->allocator, &(yyval.module)->imports, &import_ptr));
            if (field->import.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &(yyval.module)->import_bindings, &field->import.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = (yyval.module)->imports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_EXPORT: {
            WasmExportPtr export_ptr = &field->export_;
            CHECK_ALLOC(wasm_append_export_ptr_value(
                parser->allocator, &(yyval.module)->exports, &export_ptr));
            if (field->export_.name.start) {
              WasmBinding* binding =
                  wasm_insert_binding(parser->allocator, &(yyval.module)->export_bindings,
                                      &field->export_.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = (yyval.module)->exports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
            (yyval.module)->export_memory = &field->export_memory;
            break;
          case WASM_MODULE_FIELD_TYPE_TABLE:
            (yyval.module)->table = &field->table;
            break;
          case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
            WasmFuncTypePtr func_type_ptr = &field->func_type;
            CHECK_ALLOC(wasm_append_func_type_ptr_value(
                parser->allocator, &(yyval.module)->func_types, &func_type_ptr));
            if (field->func_type.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &(yyval.module)->func_type_bindings,
                  &field->func_type.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = (yyval.module)->func_types.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_MEMORY:
            (yyval.module)->memory = &field->memory;
            break;
          case WASM_MODULE_FIELD_TYPE_START:
            (yyval.module)->start = field->start;
            break;
        }
      }
    }
#line 3840 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1346 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_MODULE;
      (yyval.command)->module = *(yyvsp[0].module);
      wasm_free(parser->allocator, (yyvsp[0].module));
    }
#line 3851 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1352 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command)->invoke.loc = (yylsp[-3]);
      (yyval.command)->invoke.name = (yyvsp[-2].text);
      (yyval.command)->invoke.args = (yyvsp[-1].consts);
    }
#line 3863 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1359 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command)->assert_invalid.module = *(yyvsp[-2].module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
      wasm_free(parser->allocator, (yyvsp[-2].module));
    }
#line 3875 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1366 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command)->assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_return.expected = (yyvsp[-1].const_);
    }
#line 3888 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1374 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command)->assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command)->assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command)->assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3900 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1381 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command)->assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3913 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1391 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.commands)); }
#line 3919 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1392 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      CHECK_ALLOC(wasm_append_command_value(parser->allocator, &(yyval.commands), (yyvsp[0].command)));
      wasm_free(parser->allocator, (yyvsp[0].command));
    }
#line 3929 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1400 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (!read_const((yyvsp[-2].type), (yyvsp[-1].text).start, (yyvsp[-1].text).start + (yyvsp[-1].text).length, &(yyval.const_)))
        wasm_parser_error(&(yylsp[-1]), lexer, parser, "invalid literal \"%.*s\"",
                          (yyvsp[-1].text).length, (yyvsp[-1].text).start);
      wasm_free(parser->allocator, (char*)(yyvsp[-1].text).start);
    }
#line 3941 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1409 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3947 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1413 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { ZEROMEM((yyval.consts)); }
#line 3953 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1414 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      CHECK_ALLOC(wasm_append_const_value(parser->allocator, &(yyval.consts), &(yyvsp[0].const_)));
    }
#line 3962 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1421 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.script).commands = (yyvsp[0].commands);
      parser->script = (yyval.script);
    }
#line 3971 "src/wasm-bison-parser.c" /* yacc.c:1646  */
    break;


#line 3975 "src/wasm-bison-parser.c" /* yacc.c:1646  */
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
#line 1433 "src/wasm-bison-parser.y" /* yacc.c:1906  */


void wasm_parser_error(WasmLocation* loc,
                       WasmLexer lexer,
                       WasmParser* parser,
                       const char* fmt,
                       ...) {
  parser->errors++;
  va_list args;
  va_start(args, fmt);
  wasm_vfprint_error(stderr, loc, lexer, fmt, args);
  va_end(args);
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
  uint64_t value = 0;
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

static WasmResult dup_string_contents(WasmAllocator* allocator,
                                      WasmStringSlice* text,
                                      void** out_data,
                                      size_t* out_size) {
  const char* src = text->start + 1;
  const char* end = text->start + text->length - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src;
  char* result = wasm_alloc(allocator, size, 1);
  if (!result)
    return WASM_ERROR;
  size_t actual_size = copy_string_contents(text, result, size);
  *out_data = result;
  *out_size = actual_size;
  return WASM_OK;
}

WasmResult wasm_parse(WasmLexer lexer, struct WasmScript* out_script) {
  WasmParser parser = {};
  WasmAllocator* allocator = wasm_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  out_script->allocator = allocator;
  int result = wasm_parser_parse(lexer, &parser);
  out_script->commands = parser.script.commands;
  return result == 0 && parser.errors == 0 ? WASM_OK : WASM_ERROR;
}
