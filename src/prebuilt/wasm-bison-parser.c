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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-literal.h"
#include "wasm-parser.h"
#include "wasm-parser-lexer-shared.h"

#define DUPTEXT(dst, src)                                                   \
  (dst).start = wasm_strndup(parser->allocator, (src).start, (src).length); \
  (dst).length = (src).length

#define CHECK_ALLOC_(cond)                                       \
  if (!(cond)) {                                                 \
    WasmLocation loc;                                            \
    loc.filename = __FILE__;                                     \
    loc.line = __LINE__;                                         \
    loc.first_column = loc.last_column = 0;                      \
    wasm_parser_error(&loc, lexer, parser, "allocation failed"); \
    YYERROR;                                                     \
  }

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v) != NULL)
#define CHECK_ALLOC_STR(s) CHECK_ALLOC_((s).start != NULL)

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

#define USE_NATURAL_ALIGNMENT (~0)

static WasmFuncField* new_func_field(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmFuncField), WASM_DEFAULT_ALIGN);
}

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

static WasmResult parse_const(WasmType type, WasmLiteralType literal_type,
                              const char* s, const char* end, WasmConst* out);
static WasmResult dup_string_contents(WasmAllocator*, WasmStringSlice* text,
                                      void** out_data, size_t* out_size);

static WasmExpr* new_block_expr_with_one(WasmAllocator* allocator,
                                         WasmExpr* expr);
static WasmExpr* new_block_expr_with_list(WasmAllocator* allocator,
                                          WasmLabel* label,
                                          WasmExprPtrVector* exprs);

WasmResult copy_signature_from_func_type(WasmAllocator* allocator,
                                         WasmModule* module,
                                         WasmFuncDeclaration* decl);

#define wasm_parser_lex wasm_lexer_lex


#line 162 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:339  */

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
#ifndef YY_WASM_PARSER_SRC_PREBUILT_WASM_BISON_PARSER_H_INCLUDED
# define YY_WASM_PARSER_SRC_PREBUILT_WASM_BISON_PARSER_H_INCLUDED
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
    WASM_TOKEN_TYPE_THEN = 268,
    WASM_TOKEN_TYPE_ELSE = 269,
    WASM_TOKEN_TYPE_LOOP = 270,
    WASM_TOKEN_TYPE_BR = 271,
    WASM_TOKEN_TYPE_BR_IF = 272,
    WASM_TOKEN_TYPE_BR_TABLE = 273,
    WASM_TOKEN_TYPE_CASE = 274,
    WASM_TOKEN_TYPE_CALL = 275,
    WASM_TOKEN_TYPE_CALL_IMPORT = 276,
    WASM_TOKEN_TYPE_CALL_INDIRECT = 277,
    WASM_TOKEN_TYPE_RETURN = 278,
    WASM_TOKEN_TYPE_GET_LOCAL = 279,
    WASM_TOKEN_TYPE_SET_LOCAL = 280,
    WASM_TOKEN_TYPE_LOAD = 281,
    WASM_TOKEN_TYPE_STORE = 282,
    WASM_TOKEN_TYPE_OFFSET = 283,
    WASM_TOKEN_TYPE_ALIGN = 284,
    WASM_TOKEN_TYPE_CONST = 285,
    WASM_TOKEN_TYPE_UNARY = 286,
    WASM_TOKEN_TYPE_BINARY = 287,
    WASM_TOKEN_TYPE_COMPARE = 288,
    WASM_TOKEN_TYPE_CONVERT = 289,
    WASM_TOKEN_TYPE_SELECT = 290,
    WASM_TOKEN_TYPE_FUNC = 291,
    WASM_TOKEN_TYPE_START = 292,
    WASM_TOKEN_TYPE_TYPE = 293,
    WASM_TOKEN_TYPE_PARAM = 294,
    WASM_TOKEN_TYPE_RESULT = 295,
    WASM_TOKEN_TYPE_LOCAL = 296,
    WASM_TOKEN_TYPE_MODULE = 297,
    WASM_TOKEN_TYPE_MEMORY = 298,
    WASM_TOKEN_TYPE_SEGMENT = 299,
    WASM_TOKEN_TYPE_IMPORT = 300,
    WASM_TOKEN_TYPE_EXPORT = 301,
    WASM_TOKEN_TYPE_TABLE = 302,
    WASM_TOKEN_TYPE_UNREACHABLE = 303,
    WASM_TOKEN_TYPE_MEMORY_SIZE = 304,
    WASM_TOKEN_TYPE_GROW_MEMORY = 305,
    WASM_TOKEN_TYPE_ASSERT_INVALID = 306,
    WASM_TOKEN_TYPE_ASSERT_RETURN = 307,
    WASM_TOKEN_TYPE_ASSERT_RETURN_NAN = 308,
    WASM_TOKEN_TYPE_ASSERT_TRAP = 309,
    WASM_TOKEN_TYPE_INVOKE = 310,
    WASM_TOKEN_TYPE_LOW = 311
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

#endif /* !YY_WASM_PARSER_SRC_PREBUILT_WASM_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 292 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:358  */

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
#define YYLAST   321

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  114
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  264

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   311

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
      55,    56
};

#if WASM_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   191,   191,   192,   198,   199,   203,   207,   214,   219,
     227,   239,   247,   248,   254,   258,   268,   275,   276,   280,
     281,   290,   291,   302,   305,   309,   313,   321,   330,   341,
     354,   360,   367,   374,   381,   389,   395,   400,   407,   413,
     419,   426,   431,   437,   445,   454,   467,   473,   480,   487,
     494,   500,   504,   508,   515,   516,   519,   523,   529,   530,
     535,   542,   548,   556,   562,   568,   578,   581,   645,   651,
     658,   663,   674,   678,   691,   699,   700,   707,   719,   731,
     737,   746,   750,   757,   761,   768,   776,   783,   794,   801,
     807,   810,   819,   828,   836,   844,   852,   860,   868,   878,
     974,   980,   987,   994,  1002,  1009,  1019,  1020,  1028,  1040,
    1041,  1044,  1045,  1052,  1061
};
#endif

#if WASM_PARSER_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "INT", "FLOAT",
  "TEXT", "VAR", "VALUE_TYPE", "NOP", "BLOCK", "IF", "THEN", "ELSE",
  "LOOP", "BR", "BR_IF", "BR_TABLE", "CASE", "CALL", "CALL_IMPORT",
  "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "LOAD", "STORE",
  "OFFSET", "ALIGN", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT",
  "SELECT", "FUNC", "START", "TYPE", "PARAM", "RESULT", "LOCAL", "MODULE",
  "MEMORY", "SEGMENT", "IMPORT", "EXPORT", "TABLE", "UNREACHABLE",
  "MEMORY_SIZE", "GROW_MEMORY", "ASSERT_INVALID", "ASSERT_RETURN",
  "ASSERT_RETURN_NAN", "ASSERT_TRAP", "INVOKE", "LOW", "$accept",
  "value_type_list", "func_type", "literal", "var", "var_list", "bind_var",
  "quoted_text", "string_contents", "labeling", "offset", "align", "expr",
  "expr1", "expr_opt", "non_empty_expr_list", "expr_list", "func_fields",
  "type_use", "func_info", "func", "start", "segment_address", "segment",
  "segment_list", "initial_pages", "max_pages", "memory", "type_def",
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
     305,   306,   307,   308,   309,   310,   311
};
# endif

#define YYPACT_NINF -130

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-130)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -130,     4,  -130,     9,    42,  -130,  -130,  -130,  -130,    12,
      31,    73,    80,    39,    28,    49,    39,    33,    45,    50,
    -130,  -130,   270,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,   104,    39,    39,    39,   115,     5,    46,    20,
     108,   116,    39,  -130,  -130,  -130,  -130,  -130,    86,  -130,
    -130,   139,  -130,   119,  -130,   124,  -130,  -130,   128,   132,
    -130,  -130,   154,   146,   164,  -130,   163,    39,    39,    13,
      82,   122,   126,   131,   134,  -130,   167,   124,   167,    21,
      46,  -130,    46,    46,    46,   124,    46,    46,   155,   155,
     134,   124,   124,   124,   124,   124,    46,   167,   175,   167,
    -130,  -130,   124,   181,   128,   182,   262,  -130,   180,   189,
    -130,  -130,   191,   172,  -130,   138,  -130,    39,   196,   205,
     212,  -130,  -130,   214,   218,    39,  -130,  -130,   219,  -130,
     124,   215,   167,   124,   124,  -130,  -130,   124,    46,   124,
     124,   124,  -130,  -130,   124,  -130,   195,   195,  -130,  -130,
     124,   124,  -130,   124,   222,    26,   216,   223,    43,   226,
    -130,  -130,   236,  -130,  -130,   107,   245,   191,   206,  -130,
    -130,   149,   196,    64,   253,   254,  -130,  -130,  -130,   255,
    -130,   256,  -130,  -130,   221,   124,   124,  -130,  -130,   124,
     124,  -130,  -130,   124,  -130,  -130,   124,   124,  -130,  -130,
     124,  -130,   128,  -130,   257,   128,   128,   258,  -130,  -130,
     259,   260,   261,   271,  -130,   263,   277,  -130,  -130,  -130,
    -130,   167,  -130,  -130,  -130,  -130,  -130,  -130,   124,  -130,
    -130,   128,  -130,  -130,   128,    69,   286,  -130,   287,  -130,
     268,  -130,  -130,   124,  -130,  -130,  -130,   295,  -130,  -130,
    -130,   296,   297,   264,  -130,   299,   290,   249,   301,   167,
    -130,   124,   305,  -130
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     106,   113,   114,     0,     0,   100,   107,     1,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,   111,     0,    99,    91,    98,    97,    96,    95,    92,
      93,    94,     0,     0,     0,     0,     0,    58,     0,     0,
       0,     0,     0,    12,   102,   111,   111,   111,     0,   101,
     112,     0,    14,    58,    56,    59,    60,    67,    58,     0,
      10,    11,     0,     0,     0,    77,    75,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    17,     0,    17,    54,
       0,    12,     0,     0,     0,    54,     0,     0,    19,    19,
       0,     0,     0,     0,     0,     0,     0,     2,     0,     2,
      51,    52,     0,     0,    58,     0,     0,    57,     0,     0,
      70,    72,     4,     0,    78,     0,    75,     0,     4,     0,
       0,    83,    13,   109,     0,     0,     8,     9,     0,    18,
      58,     0,    18,    58,    54,    55,    34,     0,     0,    58,
      58,     0,    36,    41,     0,    20,    21,    21,    45,    46,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      53,    23,     0,    71,    68,     0,     0,     4,     0,    80,
      76,     0,     4,     0,     0,     0,    89,    88,   110,     0,
     104,     0,   108,    25,     0,    26,    58,    32,    35,    30,
      13,    38,    39,    58,    42,    22,     0,     0,    47,    49,
       0,    66,    58,     3,     0,    58,    58,     0,    69,     2,
       0,     0,     0,     0,    79,     0,     0,    86,    84,   103,
     105,    17,    28,    33,    31,    37,    40,    43,     0,    48,
      61,    58,    63,    64,    58,     0,     0,    81,     0,    73,
       0,    87,    85,    58,    44,    62,    65,     5,     7,    82,
      16,     0,     0,     0,    74,    27,     0,     0,     0,    17,
       6,    58,     0,    29
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -130,   -94,  -112,   176,   -21,   233,   -25,     3,  -130,   -78,
     229,   156,   -52,  -130,   -65,  -130,  -129,  -125,   -51,   -36,
    -130,  -130,  -130,  -130,   203,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,   311,  -130,  -130,   198,  -130,    65,
    -130,  -130
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   155,   166,   128,    62,    70,   129,    21,   251,   130,
     146,   196,    54,   103,   136,    55,    56,    57,    58,    59,
      24,    25,   240,   170,   115,    66,   116,    26,    27,    28,
      29,    30,    31,    14,     5,     6,     1,    50,   179,    36,
       2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     133,   183,   104,   107,   187,   158,   174,     4,    51,     7,
     191,   192,    53,    52,    64,    15,    67,   105,    60,    32,
     142,    61,   109,    63,   106,   131,    60,   135,    52,    61,
     202,    22,    23,   135,    17,   203,    45,    46,    47,   149,
     150,   151,   152,   153,    68,    69,    20,   206,   120,   122,
     160,    60,   203,   132,    61,   212,   119,   223,   134,   137,
     215,   139,   140,   141,   226,   143,   144,   175,   162,   188,
     117,   118,   156,   247,   159,   154,    18,   230,   203,   185,
     232,   233,   135,    19,     8,   189,   121,    60,    33,   193,
      61,     8,   194,     9,    10,    11,    12,    13,   198,   199,
      34,   200,    96,   209,   210,    35,   245,   186,    44,   246,
      71,    72,    73,    65,   252,   235,    74,   190,    48,    49,
     172,   216,    51,    20,    52,    48,   123,   106,   181,    48,
     124,   108,   262,   222,    48,   125,   110,   224,   225,   126,
     127,   168,   169,   243,   227,   228,   209,   210,   229,    75,
      76,    77,   168,   214,    78,    79,    80,    81,   111,    82,
      83,    84,    85,    86,    87,    88,    89,   113,   114,    90,
      91,    92,    93,    94,    95,    52,   244,    96,    97,    98,
      99,   261,   112,   145,   157,   161,   163,   100,   101,   102,
      75,    76,    77,   164,   165,    78,    79,    80,    81,   173,
      82,    83,    84,    85,    86,    87,    88,    89,   167,   176,
      90,    91,    92,    93,    94,    95,   177,    48,   184,    97,
      98,    99,   180,   182,   195,   204,   201,   205,   100,   101,
     102,    75,    76,    77,   221,   207,    78,    79,    80,    81,
     208,    82,    83,    84,    85,    86,    87,    88,    89,   211,
     213,    90,    91,    92,    93,    94,    95,   217,   218,   219,
     220,   231,   234,   259,   237,   238,   148,   241,   236,   100,
     101,   102,    75,    76,    77,   250,   239,    78,    79,    80,
      81,   242,    82,    83,    84,    85,    86,    87,    88,    89,
     248,   249,    90,    91,    92,    93,    94,    95,   253,   258,
     254,   255,   257,   197,   256,   260,    37,    38,    39,   263,
     100,   101,   102,    40,   138,    41,    42,    43,   147,   171,
      16,   178
};

static const yytype_uint16 yycheck[] =
{
      78,   130,    53,    55,   133,    99,   118,     3,     3,     0,
     139,   140,    37,     8,    39,     3,    41,    53,     5,    16,
      85,     8,    58,     3,     3,    77,     5,    79,     8,     8,
       4,     3,     4,    85,     3,     9,    33,    34,    35,    91,
      92,    93,    94,    95,    41,    42,     7,     4,    69,    70,
     102,     5,     9,    78,     8,   167,    43,   186,    79,    80,
     172,    82,    83,    84,   193,    86,    87,   118,   104,   134,
      67,    68,    97,     4,    99,    96,     3,   202,     9,   131,
     205,   206,   134,     3,    42,   137,     4,     5,    55,   141,
       8,    42,   144,    51,    52,    53,    54,    55,   150,   151,
      55,   153,    38,    39,    40,    55,   231,   132,     4,   234,
      45,    46,    47,     5,   243,   209,    30,   138,     3,     4,
     117,   172,     3,     7,     8,     3,     4,     3,   125,     3,
       4,     3,   261,   185,     3,     4,     4,   189,   190,     5,
       6,     3,     4,   221,   196,   197,    39,    40,   200,    10,
      11,    12,     3,     4,    15,    16,    17,    18,     4,    20,
      21,    22,    23,    24,    25,    26,    27,     3,     5,    30,
      31,    32,    33,    34,    35,     8,   228,    38,    39,    40,
      41,   259,    36,    28,     9,     4,     4,    48,    49,    50,
      10,    11,    12,     4,     3,    15,    16,    17,    18,     3,
      20,    21,    22,    23,    24,    25,    26,    27,    36,     4,
      30,    31,    32,    33,    34,    35,     4,     3,     3,    39,
      40,    41,     4,     4,    29,     9,     4,     4,    48,    49,
      50,    10,    11,    12,    13,     9,    15,    16,    17,    18,
       4,    20,    21,    22,    23,    24,    25,    26,    27,     4,
      44,    30,    31,    32,    33,    34,    35,     4,     4,     4,
       4,     4,     4,    14,     4,     4,    90,     4,     9,    48,
      49,    50,    10,    11,    12,     7,     5,    15,    16,    17,
      18,     4,    20,    21,    22,    23,    24,    25,    26,    27,
       4,     4,    30,    31,    32,    33,    34,    35,     3,     9,
       4,     4,     3,   147,    40,     4,    36,    37,    38,     4,
      48,    49,    50,    43,    81,    45,    46,    47,    89,   116,
       9,   123
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    93,    97,    98,     3,    91,    92,     0,    42,    51,
      52,    53,    54,    55,    90,     3,    91,     3,     3,     3,
       7,    64,     3,     4,    77,    78,    84,    85,    86,    87,
      88,    89,    64,    55,    55,    55,    96,    36,    37,    38,
      43,    45,    46,    47,     4,    64,    64,    64,     3,     4,
      94,     3,     8,    63,    69,    72,    73,    74,    75,    76,
       5,     8,    61,     3,    63,     5,    82,    63,    64,    64,
      62,    96,    96,    96,    30,    10,    11,    12,    15,    16,
      17,    18,    20,    21,    22,    23,    24,    25,    26,    27,
      30,    31,    32,    33,    34,    35,    38,    39,    40,    41,
      48,    49,    50,    70,    75,    76,     3,    69,     3,    76,
       4,     4,    36,     3,     5,    81,    83,    64,    64,    43,
      61,     4,    61,     4,     4,     4,     5,     6,    60,    63,
      66,    69,    63,    66,    61,    69,    71,    61,    62,    61,
      61,    61,    71,    61,    61,    28,    67,    67,    60,    69,
      69,    69,    69,    69,    61,    58,    63,     9,    58,    63,
      69,     4,    76,     4,     4,     3,    59,    36,     3,     4,
      80,    81,    64,     3,    59,    75,     4,     4,    94,    95,
       4,    64,     4,    73,     3,    69,    63,    73,    71,    69,
      61,    73,    73,    69,    69,    29,    68,    68,    69,    69,
      69,     4,     4,     9,     9,     4,     4,     9,     4,    39,
      40,     4,    59,    44,     4,    59,    75,     4,     4,     4,
       4,    13,    69,    73,    69,    69,    73,    69,    69,    69,
      74,     4,    74,    74,     4,    58,     9,     4,     4,     5,
      79,     4,     4,    66,    69,    74,    74,     4,     4,     4,
       7,    65,    73,     3,     4,     4,    40,     3,     9,    14,
       4,    66,    73,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    58,    59,    59,    59,    59,    60,    60,
      61,    61,    62,    62,    63,    64,    65,    66,    66,    67,
      67,    68,    68,    69,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    71,    71,    72,    72,    73,    73,
      74,    74,    74,    74,    74,    74,    75,    76,    77,    77,
      77,    77,    78,    79,    80,    81,    81,    82,    83,    84,
      84,    85,    85,    86,    87,    87,    87,    87,    88,    89,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    91,
      92,    92,    92,    92,    92,    92,    93,    93,    94,    95,
      95,    96,    96,    97,    98
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     4,     8,     4,     1,     1,
       1,     1,     0,     2,     1,     1,     1,     0,     1,     0,
       1,     0,     1,     3,     1,     3,     3,     7,     4,    12,
       3,     4,     3,     4,     2,     3,     2,     4,     3,     3,
       4,     2,     3,     4,     5,     2,     2,     3,     4,     3,
       2,     1,     1,     2,     0,     1,     1,     2,     0,     1,
       1,     5,     6,     5,     5,     6,     4,     1,     5,     6,
       4,     5,     4,     1,     5,     0,     2,     1,     1,     6,
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
          case 58: /* value_type_list  */
#line 161 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(parser->allocator, &((*yyvaluep).types)); }
#line 1388 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 59: /* func_type  */
#line 171 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(parser->allocator, &((*yyvaluep).func_sig)); }
#line 1394 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 60: /* literal  */
#line 160 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).literal).text); }
#line 1400 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 61: /* var  */
#line 162 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(parser->allocator, &((*yyvaluep).var)); }
#line 1406 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 62: /* var_list  */
#line 163 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1412 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 63: /* bind_var  */
#line 159 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1418 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 64: /* quoted_text  */
#line 159 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1424 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 65: /* string_contents  */
#line 168 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1430 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 66: /* labeling  */
#line 159 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1436 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 69: /* expr  */
#line 164 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1442 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 70: /* expr1  */
#line 164 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1448 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 71: /* expr_opt  */
#line 164 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr(parser->allocator, &((*yyvaluep).expr)); }
#line 1454 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 72: /* non_empty_expr_list  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(parser->allocator, &((*yyvaluep).exprs)); }
#line 1460 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 73: /* expr_list  */
#line 165 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_ptr_vector_and_elements(parser->allocator, &((*yyvaluep).exprs)); }
#line 1466 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 74: /* func_fields  */
#line 166 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_fields(parser->allocator, ((*yyvaluep).func_fields)); }
#line 1472 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 76: /* func_info  */
#line 167 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1478 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 77: /* func  */
#line 167 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1484 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 80: /* segment  */
#line 168 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1490 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 81: /* segment_list  */
#line 169 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(parser->allocator, &((*yyvaluep).segments)); }
#line 1496 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 84: /* memory  */
#line 170 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(parser->allocator, &((*yyvaluep).memory)); }
#line 1502 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 85: /* type_def  */
#line 172 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(parser->allocator, &((*yyvaluep).func_type)); }
#line 1508 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 86: /* table  */
#line 163 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1514 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 87: /* import  */
#line 173 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(parser->allocator, ((*yyvaluep).import)); wasm_free(parser->allocator, ((*yyvaluep).import)); }
#line 1520 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 88: /* export  */
#line 174 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(parser->allocator, &((*yyvaluep).export_)); }
#line 1526 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 90: /* module_fields  */
#line 175 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1532 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 91: /* module  */
#line 175 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1538 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 92: /* cmd  */
#line 177 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(parser->allocator, ((*yyvaluep).command)); wasm_free(parser->allocator, ((*yyvaluep).command)); }
#line 1544 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 93: /* cmd_list  */
#line 178 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(parser->allocator, &((*yyvaluep).commands)); }
#line 1550 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 96: /* const_list  */
#line 176 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(parser->allocator, &((*yyvaluep).consts)); }
#line 1556 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
        break;

    case 97: /* script  */
#line 179 "src/wasm-bison-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1562 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1257  */
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
#line 191 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.types)); }
#line 1856 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 192 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &(yyval.types), &(yyvsp[0].type)));
    }
#line 1865 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 198 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); }
#line 1871 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 199 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 1880 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 203 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 1889 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 207 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 1895 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 214 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
      CHECK_ALLOC_STR((yyval.literal).text);
    }
#line 1905 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 219 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
      CHECK_ALLOC_STR((yyval.literal).text);
    }
#line 1915 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 227 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &index,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser, "invalid int " PRIstringslice,
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      (yyval.var).index = index;
    }
#line 1932 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 239 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
      CHECK_ALLOC_STR((yyval.var).name);
    }
#line 1943 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 247 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.vars)); }
#line 1949 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 248 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      CHECK_ALLOC(wasm_append_var_value(parser->allocator, &(yyval.vars), &(yyvsp[0].var)));
    }
#line 1958 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 254 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); CHECK_ALLOC_STR((yyval.text)); }
#line 1964 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 258 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      void* data;
      size_t size;
      CHECK_ALLOC(dup_string_contents(parser->allocator, &(yyvsp[0].text), &data, &size));
      (yyval.text).start = data;
      (yyval.text).length = size;
    }
#line 1976 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 268 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOC(dup_string_contents(parser->allocator, &(yyvsp[0].text), &(yyval.segment).data,
                                      &(yyval.segment).size));
    }
#line 1985 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 275 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.text)); }
#line 1991 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 276 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.text) = (yyvsp[0].text); }
#line 1997 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 280 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2003 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 281 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64)))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid offset \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2015 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 290 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2021 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 291 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid alignment \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2034 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 302 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 2040 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 305 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_NOP);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2049 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 309 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new_block_expr_with_list(parser->allocator, &(yyvsp[-1].text), &(yyvsp[0].exprs));
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2058 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 313 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_.cond = (yyvsp[-1].expr);
      WasmExpr* true_block = new_block_expr_with_one(parser->allocator, (yyvsp[0].expr));
      CHECK_ALLOC_NULL(true_block);
      (yyval.expr)->if_.true_ = true_block;
    }
#line 2071 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 321 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_.cond = (yyvsp[-5].expr);
      WasmExpr* true_block =
          new_block_expr_with_list(parser->allocator, &(yyvsp[-2].text), &(yyvsp[-1].exprs));
      CHECK_ALLOC_NULL(true_block);
      (yyval.expr)->if_.true_ = true_block;
    }
#line 2085 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 330 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      WasmExpr* true_block = new_block_expr_with_one(parser->allocator, (yyvsp[-1].expr));
      CHECK_ALLOC_NULL(true_block);
      (yyval.expr)->if_else.true_ = true_block;
      WasmExpr* false_block = new_block_expr_with_one(parser->allocator, (yyvsp[0].expr));
      CHECK_ALLOC_NULL(false_block);
      (yyval.expr)->if_else.false_ = false_block;
    }
#line 2101 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 341 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->if_else.cond = (yyvsp[-10].expr);
      WasmExpr* true_block =
          new_block_expr_with_list(parser->allocator, &(yyvsp[-7].text), &(yyvsp[-6].exprs));
      CHECK_ALLOC_NULL(true_block);
      (yyval.expr)->if_else.true_ = true_block;
      WasmExpr* false_block =
          new_block_expr_with_list(parser->allocator, &(yyvsp[-2].text), &(yyvsp[-1].exprs));
      CHECK_ALLOC_NULL(false_block);
      (yyval.expr)->if_else.false_ = false_block;
    }
#line 2119 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 354 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2130 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 360 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_if.var = (yyvsp[-2].var);
      (yyval.expr)->br_if.expr = (yyvsp[-1].expr);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2142 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 367 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      WASM_ZERO_MEMORY((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2154 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 374 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.exprs = (yyvsp[0].exprs);
    }
#line 2166 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 381 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var.loc = (yylsp[-1]);
      (yyval.expr)->br.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.expr)->br.var.index = 0;
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2179 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 389 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2190 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 395 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_return_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2200 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 400 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_table_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->br_table.expr = (yyvsp[0].expr);
      (yyval.expr)->br_table.targets = (yyvsp[-2].vars);
      (yyval.expr)->br_table.default_target = (yyvsp[-1].var);
    }
#line 2212 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 407 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2223 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 413 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_import_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.args = (yyvsp[0].exprs);
    }
#line 2234 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 419 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_indirect_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.args = (yyvsp[0].exprs);
    }
#line 2246 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 426 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_get_local_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2256 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 431 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_set_local_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2267 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 437 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_load_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->load.opcode = (yyvsp[-3].opcode);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2280 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 445 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_store_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->store.opcode = (yyvsp[-4].opcode);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2294 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 454 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_const_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (WASM_FAILED(parse_const((yyvsp[-1].type), (yyvsp[0].literal).type, (yyvsp[0].literal).text.start,
                                  (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length,
                                  &(yyval.expr)->const_))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      wasm_free(parser->allocator, (char*)(yyvsp[0].literal).text.start);
    }
#line 2312 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 467 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_unary_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->unary.opcode = (yyvsp[-1].opcode);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2323 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 473 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_binary_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->binary.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2335 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 480 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_select_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->select.true_ = (yyvsp[-2].expr);
      (yyval.expr)->select.false_ = (yyvsp[-1].expr);
      (yyval.expr)->select.cond = (yyvsp[0].expr);
    }
#line 2347 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 487 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_compare_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->compare.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2359 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 494 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_convert_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->convert.opcode = (yyvsp[-1].opcode);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2370 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 500 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_UNREACHABLE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2379 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 504 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_MEMORY_SIZE);
      CHECK_ALLOC_NULL((yyval.expr));
    }
#line 2388 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 508 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_grow_memory_expr(parser->allocator);
      CHECK_ALLOC_NULL((yyval.expr));
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2398 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 515 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2404 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 519 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      WASM_ZERO_MEMORY((yyval.exprs));
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2413 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 523 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.exprs) = (yyvsp[-1].exprs);
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &(yyval.exprs), &(yyvsp[0].expr)));
    }
#line 2422 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 529 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.exprs)); }
#line 2428 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 535 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      CHECK_ALLOC_NULL((yyval.func_fields));
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_EXPRS;
      (yyval.func_fields)->exprs = (yyvsp[0].exprs);
      (yyval.func_fields)->next = NULL;
    }
#line 2440 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 542 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_PARAM_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2451 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 548 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_PARAM;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2464 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 556 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_RESULT_TYPE;
      (yyval.func_fields)->result_type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2475 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 562 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_LOCAL_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2486 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 568 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_LOCAL;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2499 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 578 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2505 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 581 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      CHECK_ALLOC_NULL((yyval.func));
      WasmFuncField* field = (yyvsp[0].func_fields);
      while (field) {
        WasmFuncField* next = field->next;

        if (field->type == WASM_FUNC_FIELD_TYPE_PARAM_TYPES ||
            field->type == WASM_FUNC_FIELD_TYPE_BOUND_PARAM ||
            field->type == WASM_FUNC_FIELD_TYPE_RESULT_TYPE) {
          (yyval.func)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
        }

        switch (field->type) {
          case WASM_FUNC_FIELD_TYPE_EXPRS:
            (yyval.func)->exprs = field->exprs;
            break;

          case WASM_FUNC_FIELD_TYPE_PARAM_TYPES:
          case WASM_FUNC_FIELD_TYPE_LOCAL_TYPES: {
            WasmTypeVector* types =
                field->type == WASM_FUNC_FIELD_TYPE_PARAM_TYPES
                    ? &(yyval.func)->decl.sig.param_types
                    : &(yyval.func)->local_types;
            CHECK_ALLOC(
                wasm_extend_types(parser->allocator, types, &field->types));
            wasm_destroy_type_vector(parser->allocator, &field->types);
            break;
          }

          case WASM_FUNC_FIELD_TYPE_BOUND_PARAM:
          case WASM_FUNC_FIELD_TYPE_BOUND_LOCAL: {
            WasmTypeVector* types;
            WasmBindingHash* bindings;
            if (field->type == WASM_FUNC_FIELD_TYPE_BOUND_PARAM) {
              types = &(yyval.func)->decl.sig.param_types;
              bindings = &(yyval.func)->param_bindings;
            } else {
              types = &(yyval.func)->local_types;
              bindings = &(yyval.func)->local_bindings;
            }

            CHECK_ALLOC(wasm_append_type_value(parser->allocator, types,
                                               &field->bound_type.type));
            WasmBinding* binding = wasm_insert_binding(
                parser->allocator, bindings, &field->bound_type.name);
            CHECK_ALLOC_NULL(binding);
            binding->loc = field->bound_type.loc;
            binding->index = types->size - 1;
            break;
          }

          case WASM_FUNC_FIELD_TYPE_RESULT_TYPE:
            (yyval.func)->decl.sig.result_type = field->result_type;
            break;
        }

        /* we steal memory from the func field, but not the linked list nodes */
        wasm_free(parser->allocator, field);
        field = next;
      }
    }
#line 2572 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 645 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[-1].func);
      (yyval.func)->loc = (yylsp[-3]);
      (yyval.func)->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->decl.type_var = (yyvsp[-2].var);
    }
#line 2583 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 651 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[-1].func);
      (yyval.func)->loc = (yylsp[-4]);
      (yyval.func)->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.func)->decl.type_var = (yyvsp[-2].var);
      (yyval.func)->name = (yyvsp[-3].text);
    }
#line 2595 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 658 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[-1].func);
      (yyval.func)->loc = (yylsp[-2]);
      (yyval.func)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
    }
#line 2605 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 663 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[-1].func);
      (yyval.func)->loc = (yylsp[-3]);
      (yyval.func)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.func)->name = (yyvsp[-2].text);
    }
#line 2616 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 674 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2622 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 678 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid memory segment address \"" PRIstringslice
                          "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2637 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 691 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      (yyval.segment).addr = (yyvsp[-2].u32);
    }
#line 2648 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 699 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.segments)); }
#line 2654 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 700 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      CHECK_ALLOC(wasm_append_segment_value(parser->allocator, &(yyval.segments), &(yyvsp[0].segment)));
    }
#line 2663 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 707 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid initial memory pages \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2677 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 719 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid max memory pages \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2691 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 731 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      (yyval.memory).initial_pages = (yyvsp[-3].u32);
      (yyval.memory).max_pages = (yyvsp[-2].u32);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 2702 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 737 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      (yyval.memory).initial_pages = (yyvsp[-2].u32);
      (yyval.memory).max_pages = (yyval.memory).initial_pages;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 2713 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 746 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      WASM_ZERO_MEMORY((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 2722 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 750 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 2731 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 757 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 2737 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 761 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 2749 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 768 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 2762 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 776 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 2774 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 783 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 2787 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 794 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_).name = (yyvsp[-2].text);
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 2796 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 801 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_memory).name = (yyvsp[-2].text);
    }
#line 2804 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 807 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new_module(parser->allocator);
    }
#line 2812 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 810 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *(yyvsp[0].func);
      wasm_free(parser->allocator, (yyvsp[0].func));
    }
#line 2826 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 819 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *(yyvsp[0].import);
      wasm_free(parser->allocator, (yyvsp[0].import));
    }
#line 2840 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 828 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = (yyvsp[0].export_);
    }
#line 2853 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 836 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = (yyvsp[0].export_memory);
    }
#line 2866 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 844 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
    }
#line 2879 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 852 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);
    }
#line 2892 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 860 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
    }
#line 2905 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 868 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      CHECK_ALLOC_NULL(field);
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = (yyvsp[0].var);
    }
#line 2918 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 878 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      (yyval.module)->loc = (yylsp[-2]);

      /* cache values */
      WasmModuleField* field;
      for (field = (yyval.module)->first_field; field; field = field->next) {
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
            (yyval.module)->start = &field->start;
            break;
        }
      }

      size_t i;
      for (i = 0; i < (yyval.module)->funcs.size; ++i) {
        WasmFunc* func = (yyval.module)->funcs.data[i];
        CHECK_ALLOC(
            copy_signature_from_func_type(parser->allocator, (yyval.module), &func->decl));
      }

      for (i = 0; i < (yyval.module)->imports.size; ++i) {
        WasmImport* import = (yyval.module)->imports.data[i];
        CHECK_ALLOC(copy_signature_from_func_type(parser->allocator, (yyval.module),
                                                  &import->decl));
      }
    }
#line 3013 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 974 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_MODULE;
      (yyval.command)->module = *(yyvsp[0].module);
      wasm_free(parser->allocator, (yyvsp[0].module));
    }
#line 3024 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 980 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command)->invoke.loc = (yylsp[-3]);
      (yyval.command)->invoke.name = (yyvsp[-2].text);
      (yyval.command)->invoke.args = (yyvsp[-1].consts);
    }
#line 3036 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 987 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command)->assert_invalid.module = *(yyvsp[-2].module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
      wasm_free(parser->allocator, (yyvsp[-2].module));
    }
#line 3048 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 994 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command)->assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_return.expected = (yyvsp[-1].const_);
    }
#line 3061 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1002 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command)->assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command)->assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command)->assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3073 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1009 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command)->assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3086 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1019 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.commands)); }
#line 3092 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1020 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      CHECK_ALLOC(wasm_append_command_value(parser->allocator, &(yyval.commands), (yyvsp[0].command)));
      wasm_free(parser->allocator, (yyvsp[0].command));
    }
#line 3102 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1028 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (WASM_FAILED(parse_const((yyvsp[-2].type), (yyvsp[-1].literal).type, (yyvsp[-1].literal).text.start,
                                  (yyvsp[-1].literal).text.start + (yyvsp[-1].literal).text.length, &(yyval.const_)))) {
        wasm_parser_error(&(yylsp[-1]), lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG((yyvsp[-1].literal).text));
      }
      wasm_free(parser->allocator, (char*)(yyvsp[-1].literal).text.start);
    }
#line 3117 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1040 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3123 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1044 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.consts)); }
#line 3129 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1045 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      CHECK_ALLOC(wasm_append_const_value(parser->allocator, &(yyval.consts), &(yyvsp[0].const_)));
    }
#line 3138 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1052 "src/wasm-bison-parser.y" /* yacc.c:1646  */
    {
      (yyval.script).commands = (yyvsp[0].commands);
      parser->script = (yyval.script);
    }
#line 3147 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
    break;


#line 3151 "src/prebuilt/wasm-bison-parser.c" /* yacc.c:1646  */
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
#line 1064 "src/wasm-bison-parser.y" /* yacc.c:1906  */


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

static WasmResult parse_const(WasmType type,
                              WasmLiteralType literal_type,
                              const char* s,
                              const char* end,
                              WasmConst* out) {
  out->type = type;
  switch (type) {
    case WASM_TYPE_I32:
      return wasm_parse_int32(s, end, &out->u32,
                              WASM_PARSE_SIGNED_AND_UNSIGNED);
    case WASM_TYPE_I64:
      return wasm_parse_int64(s, end, &out->u64);
    case WASM_TYPE_F32:
      return wasm_parse_float(literal_type, s, end, &out->f32_bits);
    case WASM_TYPE_F64:
      return wasm_parse_double(literal_type, s, end, &out->f64_bits);
    default:
      assert(0);
      break;
  }
  return WASM_ERROR;
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
          if (WASM_SUCCEEDED(wasm_parse_hexdigit(src[0], &hi)) &&
              WASM_SUCCEEDED(wasm_parse_hexdigit(src[1], &lo))) {
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
  WasmParser parser;
  WASM_ZERO_MEMORY(parser);
  WasmAllocator* allocator = wasm_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  out_script->allocator = allocator;
  int result = wasm_parser_parse(lexer, &parser);
  out_script->commands = parser.script.commands;
  return result == 0 && parser.errors == 0 ? WASM_OK : WASM_ERROR;
}

WasmExpr* new_block_expr_with_one(WasmAllocator* allocator, WasmExpr* expr) {
  WasmExpr* block = wasm_new_block_expr(allocator);
  if (block) {
    WasmExprPtr* expr_ptr =
        wasm_append_expr_ptr(allocator, &block->block.exprs);
    if (expr_ptr) {
      *expr_ptr = expr;
      return block;
    }
    wasm_destroy_expr_ptr(allocator, &block);
  }
  return NULL;
}

WasmExpr* new_block_expr_with_list(WasmAllocator* allocator,
                                   WasmLabel* label,
                                   WasmExprPtrVector* exprs) {
  WasmExpr* block = wasm_new_block_expr(allocator);
  if (!block)
    return NULL;
  block->block.label = *label;
  block->block.exprs = *exprs;
  return block;
}

WasmResult copy_signature_from_func_type(WasmAllocator* allocator,
                                         WasmModule* module,
                                         WasmFuncDeclaration* decl) {
  /* if a function or import only defines a func type (and no explicit
   * signature), copy the signature over for convenience */
  if (wasm_decl_has_func_type(decl) && !wasm_decl_has_signature(decl)) {
    int index = wasm_get_func_type_index_by_var(module, &decl->type_var);
    if (index >= 0 && (size_t)index < module->func_types.size) {
      WasmFuncType* func_type = module->func_types.data[index];
      decl->sig.result_type = func_type->sig.result_type;
      return wasm_extend_types(allocator, &decl->sig.param_types,
                               &func_type->sig.param_types);
    } else {
      /* technically not OK, but we'll catch this error later in the AST
       * checker */
      return WASM_OK;
    }
  }
  return WASM_OK;
}
