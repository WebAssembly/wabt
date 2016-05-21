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
#define YYSTYPE         WASM_AST_PARSER_STYPE
#define YYLTYPE         WASM_AST_PARSER_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         wasm_ast_parser_parse
#define yylex           wasm_ast_parser_lex
#define yyerror         wasm_ast_parser_error
#define yydebug         wasm_ast_parser_debug
#define yynerrs         wasm_ast_parser_nerrs


/* Copy the first part of user declarations.  */
#line 17 "src/wasm-ast-parser.y" /* yacc.c:339  */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-ast-parser.h"
#include "wasm-ast-parser-lexer-shared.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-binary-reader.h"
#include "wasm-literal.h"

#define DUPTEXT(dst, src)                                                   \
  (dst).start = wasm_strndup(parser->allocator, (src).start, (src).length); \
  (dst).length = (src).length

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

static WasmTextListNode* new_text_list_node(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmTextListNode),
                         WASM_DEFAULT_ALIGN);
}

static WasmResult parse_const(WasmType type, WasmLiteralType literal_type,
                              const char* s, const char* end, WasmConst* out);
static void dup_text_list(WasmAllocator*, WasmTextList* text_list,
                          void** out_data, size_t* out_size);

static void copy_signature_from_func_type(
    WasmAllocator* allocator, WasmModule* module, WasmFuncDeclaration* decl);

typedef struct BinaryErrorCallbackData {
  WasmLocation* loc;
  WasmAstLexer* lexer;
  WasmAstParser* parser;
} BinaryErrorCallbackData;

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data);

#define wasm_ast_parser_lex wasm_ast_lexer_lex


#line 157 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:339  */

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
   by #include "wasm-ast-parser-gen.h".  */
#ifndef YY_WASM_AST_PARSER_SRC_PREBUILT_WASM_AST_PARSER_GEN_H_INCLUDED
# define YY_WASM_AST_PARSER_SRC_PREBUILT_WASM_AST_PARSER_GEN_H_INCLUDED
/* Debug traces.  */
#ifndef WASM_AST_PARSER_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define WASM_AST_PARSER_DEBUG 1
#  else
#   define WASM_AST_PARSER_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define WASM_AST_PARSER_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined WASM_AST_PARSER_DEBUG */
#if WASM_AST_PARSER_DEBUG
extern int wasm_ast_parser_debug;
#endif

/* Token type.  */
#ifndef WASM_AST_PARSER_TOKENTYPE
# define WASM_AST_PARSER_TOKENTYPE
  enum wasm_ast_parser_tokentype
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
    WASM_TOKEN_TYPE_CURRENT_MEMORY = 304,
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
#if ! defined WASM_AST_PARSER_STYPE && ! defined WASM_AST_PARSER_STYPE_IS_DECLARED
typedef WasmToken WASM_AST_PARSER_STYPE;
# define WASM_AST_PARSER_STYPE_IS_TRIVIAL 1
# define WASM_AST_PARSER_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined WASM_AST_PARSER_LTYPE && ! defined WASM_AST_PARSER_LTYPE_IS_DECLARED
typedef struct WASM_AST_PARSER_LTYPE WASM_AST_PARSER_LTYPE;
struct WASM_AST_PARSER_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define WASM_AST_PARSER_LTYPE_IS_DECLARED 1
# define WASM_AST_PARSER_LTYPE_IS_TRIVIAL 1
#endif



int wasm_ast_parser_parse (WasmAstLexer* lexer, WasmAstParser* parser);

#endif /* !YY_WASM_AST_PARSER_SRC_PREBUILT_WASM_AST_PARSER_GEN_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 287 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:358  */

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
         || (defined WASM_AST_PARSER_LTYPE_IS_TRIVIAL && WASM_AST_PARSER_LTYPE_IS_TRIVIAL \
             && defined WASM_AST_PARSER_STYPE_IS_TRIVIAL && WASM_AST_PARSER_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   330

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  120
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  271

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

#if WASM_AST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   190,   190,   196,   209,   210,   216,   217,   221,   225,
     232,   236,   243,   255,   262,   263,   269,   273,   289,   296,
     297,   301,   302,   311,   312,   323,   326,   329,   334,   339,
     345,   351,   359,   364,   370,   376,   382,   387,   391,   398,
     405,   411,   417,   424,   428,   433,   440,   448,   460,   465,
     471,   477,   483,   488,   491,   495,   501,   502,   505,   509,
     517,   518,   523,   529,   535,   543,   549,   555,   565,   568,
     629,   638,   648,   656,   668,   669,   676,   680,   693,   701,
     702,   709,   721,   732,   738,   747,   751,   758,   762,   769,
     777,   784,   795,   802,   808,   811,   847,   865,   882,   890,
     898,   916,   924,   935,   952,   961,   985,   991,   998,  1004,
    1012,  1019,  1029,  1030,  1038,  1050,  1051,  1054,  1055,  1062,
    1071
};
#endif

#if WASM_AST_PARSER_DEBUG || YYERROR_VERBOSE || 1
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
  "CURRENT_MEMORY", "GROW_MEMORY", "ASSERT_INVALID", "ASSERT_RETURN",
  "ASSERT_RETURN_NAN", "ASSERT_TRAP", "INVOKE", "LOW", "$accept",
  "text_list", "value_type_list", "func_type", "literal", "var",
  "var_list", "bind_var", "quoted_text", "segment_contents", "labeling",
  "offset", "align", "expr", "expr1", "expr_opt", "non_empty_expr_list",
  "expr_list", "func_fields", "type_use", "func_info", "func",
  "export_opt", "start", "segment_address", "segment", "segment_list",
  "initial_pages", "max_pages", "memory", "type_def", "table", "import",
  "export", "export_memory", "module_fields", "raw_module", "module",
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
    -153,    25,  -153,    34,    67,  -153,  -153,  -153,  -153,    31,
      33,    48,    52,    60,    58,  -153,    40,     1,    30,    58,
      19,    38,    44,  -153,  -153,  -153,  -153,   106,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,    80,    58,    58,
      58,    18,    58,    73,     4,    83,   105,    58,  -153,  -153,
    -153,  -153,  -153,    71,  -153,  -153,  -153,    12,  -153,  -153,
     111,   109,  -153,   145,  -153,   152,    58,    58,    21,    41,
     120,   125,   129,   130,   157,   147,  -153,   153,  -153,  -153,
     155,   156,  -153,   158,   123,  -153,   136,  -153,    58,   160,
     166,   167,  -153,  -153,   161,   172,    58,  -153,  -153,   181,
    -153,   178,   153,   178,    73,    73,  -153,    73,    73,    73,
     153,    73,    73,   165,   165,   130,   153,   153,   153,   153,
     153,    73,   178,   185,   178,  -153,  -153,   153,   195,   155,
     196,   280,  -153,   198,   197,  -153,   107,   199,   158,   168,
    -153,  -153,   151,   160,    65,   200,   207,  -153,  -153,  -153,
     213,  -153,   222,  -153,  -153,   153,   224,   178,   153,   153,
     153,    73,   153,   153,   153,  -153,  -153,  -153,   153,  -153,
     173,   173,  -153,  -153,   153,   153,  -153,   153,   230,    15,
     226,   232,    26,   231,  -153,  -153,   237,  -153,  -153,  -153,
     233,   240,   241,   238,  -153,   249,   254,  -153,  -153,  -153,
    -153,  -153,   239,   153,   153,  -153,  -153,   153,   153,  -153,
    -153,   153,  -153,  -153,   153,   153,  -153,  -153,   153,  -153,
     155,  -153,   263,   155,   155,   264,  -153,    28,   271,  -153,
     272,  -153,    31,  -153,  -153,   178,  -153,  -153,  -153,   153,
    -153,  -153,   153,  -153,  -153,   155,  -153,  -153,   155,   274,
    -153,  -153,   273,   275,   153,  -153,  -153,  -153,  -153,   242,
    -153,   277,   269,   281,   279,   285,  -153,   178,   153,   282,
    -153
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     112,   119,   120,     0,     0,   105,   106,   113,     1,    94,
       0,     0,     0,     0,     0,     2,     0,     0,     0,     0,
       0,     0,     0,    17,   117,   104,     3,     0,   103,    95,
     102,   101,   100,    99,    96,    97,    98,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,     0,    14,   108,
     117,   117,   117,     0,   107,   118,    75,    60,    12,    13,
       0,     0,    16,     0,    81,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    60,    58,    61,    62,    69,
      60,     0,    76,     6,     0,    82,     0,    79,     0,     6,
       0,     0,    87,    15,   115,     0,     0,    10,    11,     0,
      26,    19,     0,    19,     0,     0,    14,     0,     0,     0,
      56,     0,     0,    21,    21,     0,     0,     0,     0,     0,
       0,     0,     4,     0,     4,    53,    54,     0,     0,    60,
       0,     0,    59,     0,     0,    72,     0,     0,     6,     0,
      84,    80,     0,     6,     0,     0,     0,    93,    92,   116,
       0,   110,     0,   114,    20,    60,     0,    20,    60,    56,
       0,     0,    60,    60,     0,    57,    37,    43,     0,    22,
      23,    23,    47,    48,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,    55,    25,     0,    73,    70,     4,
       0,     0,     0,     0,    83,     0,     0,    90,    88,   109,
     111,    27,     0,    28,    60,    34,    36,    32,    15,    40,
      41,    60,    44,    24,     0,     0,    49,    51,     0,    68,
      60,     5,     0,    60,    60,     0,    71,     0,     0,    85,
       0,    77,     0,    91,    89,    19,    30,    35,    33,    38,
      42,    45,     0,    50,    63,    60,    65,    66,    60,     7,
       9,    86,    18,     0,    60,    39,    46,    64,    67,     0,
      78,     0,     0,    29,     0,     0,     8,    19,    60,     0,
      31
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -153,    53,  -123,   -58,   179,   -51,   187,   -30,    29,  -153,
    -101,   194,   138,   -77,  -153,   159,  -153,  -152,  -134,   -66,
     -67,  -153,  -153,  -153,  -153,  -153,   229,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,   307,  -153,  -153,  -153,
     225,  -153,    56,  -153,  -153
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    16,   179,   137,    99,    60,    69,   154,    24,   253,
     155,   170,   214,    76,   128,   166,    77,    78,    79,    80,
      81,    29,    57,    30,   232,   141,    86,    65,    87,    31,
      32,    33,    34,    35,    36,    17,     5,     6,     7,     1,
      55,   150,    41,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     132,   182,   158,   201,    27,    28,   205,    61,   130,   129,
     209,   210,    62,   134,    63,    74,    66,    91,    93,   220,
      62,    53,    54,   146,   221,   156,    58,    75,     4,    59,
     224,   145,   249,   165,     8,   221,    18,   221,    15,   173,
     174,   175,   176,   177,    25,    92,    58,    26,    37,    59,
     184,    20,   237,   159,   160,    21,   162,   163,   164,   240,
     167,   168,   186,    22,    90,    23,   227,    50,    51,    52,
     178,    56,     9,   157,    38,    67,    68,   196,    58,   203,
     192,    59,   165,   207,    49,   195,   244,   211,    64,   246,
     247,   212,   180,    39,   183,    88,    89,   216,   217,    40,
     218,    73,   261,   121,   189,   190,    70,    71,    72,     9,
     208,   257,    23,    62,   258,    82,   269,   143,    10,    11,
      12,    13,    14,    53,    94,   152,   236,   204,    53,    95,
     238,   239,    53,    96,   254,    97,    98,   241,   242,   139,
     140,   243,    42,    43,    44,    83,   189,   190,    84,    45,
      74,    46,    47,    48,   139,   194,   131,    85,   133,   138,
     135,   136,   255,   144,    53,   256,   268,   100,   101,   102,
     147,   148,   103,   104,   105,   106,   151,   107,   108,   109,
     110,   111,   112,   113,   114,   153,    62,   115,   116,   117,
     118,   119,   120,   169,   181,   121,   122,   123,   124,   185,
     187,   188,   213,   191,   197,   125,   126,   127,   100,   101,
     102,   198,   193,   103,   104,   105,   106,   199,   107,   108,
     109,   110,   111,   112,   113,   114,   200,   202,   115,   116,
     117,   118,   119,   120,   219,   222,   223,   122,   123,   124,
     225,   226,   228,   231,   229,   230,   125,   126,   127,   100,
     101,   102,   235,   233,   103,   104,   105,   106,   234,   107,
     108,   109,   110,   111,   112,   113,   114,   245,   248,   115,
     116,   117,   118,   119,   120,   250,   251,   259,   264,   260,
      26,   263,   262,   266,   265,   252,   270,   125,   126,   127,
     100,   101,   102,   161,   172,   103,   104,   105,   106,   267,
     107,   108,   109,   110,   111,   112,   113,   114,   171,   215,
     115,   116,   117,   118,   119,   120,   142,    19,   206,   149,
       0,     0,     0,     0,     0,     0,     0,     0,   125,   126,
     127
};

static const yytype_int16 yycheck[] =
{
      77,   124,   103,   155,     3,     4,   158,     3,    75,    75,
     162,   163,     8,    80,    44,     3,    46,    68,    69,     4,
       8,     3,     4,    89,     9,   102,     5,    57,     3,     8,
       4,    89,     4,   110,     0,     9,     3,     9,     7,   116,
     117,   118,   119,   120,     4,     4,     5,     7,    19,     8,
     127,     3,   204,   104,   105,     3,   107,   108,   109,   211,
     111,   112,   129,     3,    43,     7,   189,    38,    39,    40,
     121,    42,    42,   103,    55,    46,    47,   143,     5,   156,
     138,     8,   159,   160,     4,   143,   220,   164,     5,   223,
     224,   168,   122,    55,   124,    66,    67,   174,   175,    55,
     177,    30,   254,    38,    39,    40,    50,    51,    52,    42,
     161,   245,     7,     8,   248,     4,   268,    88,    51,    52,
      53,    54,    55,     3,     4,    96,   203,   157,     3,     4,
     207,   208,     3,     4,   235,     5,     6,   214,   215,     3,
       4,   218,    36,    37,    38,    36,    39,    40,     3,    43,
       3,    45,    46,    47,     3,     4,     3,     5,     3,    36,
       4,     3,   239,     3,     3,   242,   267,    10,    11,    12,
       4,     4,    15,    16,    17,    18,     4,    20,    21,    22,
      23,    24,    25,    26,    27,     4,     8,    30,    31,    32,
      33,    34,    35,    28,     9,    38,    39,    40,    41,     4,
       4,     4,    29,     4,     4,    48,    49,    50,    10,    11,
      12,     4,    44,    15,    16,    17,    18,     4,    20,    21,
      22,    23,    24,    25,    26,    27,     4,     3,    30,    31,
      32,    33,    34,    35,     4,     9,     4,    39,    40,    41,
       9,     4,     9,     5,     4,     4,    48,    49,    50,    10,
      11,    12,    13,     4,    15,    16,    17,    18,     4,    20,
      21,    22,    23,    24,    25,    26,    27,     4,     4,    30,
      31,    32,    33,    34,    35,     4,     4,     3,     9,     4,
       7,     4,    40,     4,     3,   232,     4,    48,    49,    50,
      10,    11,    12,   106,   115,    15,    16,    17,    18,    14,
      20,    21,    22,    23,    24,    25,    26,    27,   114,   171,
      30,    31,    32,    33,    34,    35,    87,    10,   159,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      50
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    96,   100,   101,     3,    93,    94,    95,     0,    42,
      51,    52,    53,    54,    55,     7,    58,    92,     3,    93,
       3,     3,     3,     7,    65,     4,     7,     3,     4,    78,
      80,    86,    87,    88,    89,    90,    91,    65,    55,    55,
      55,    99,    36,    37,    38,    43,    45,    46,    47,     4,
      65,    65,    65,     3,     4,    97,    65,    79,     5,     8,
      62,     3,     8,    64,     5,    84,    64,    65,    65,    63,
      99,    99,    99,    30,     3,    64,    70,    73,    74,    75,
      76,    77,     4,    36,     3,     5,    83,    85,    65,    65,
      43,    62,     4,    62,     4,     4,     4,     5,     6,    61,
      10,    11,    12,    15,    16,    17,    18,    20,    21,    22,
      23,    24,    25,    26,    27,    30,    31,    32,    33,    34,
      35,    38,    39,    40,    41,    48,    49,    50,    71,    76,
      77,     3,    70,     3,    77,     4,     3,    60,    36,     3,
       4,    82,    83,    65,     3,    60,    76,     4,     4,    97,
      98,     4,    65,     4,    64,    67,    70,    64,    67,    62,
      62,    63,    62,    62,    62,    70,    72,    62,    62,    28,
      68,    68,    61,    70,    70,    70,    70,    70,    62,    59,
      64,     9,    59,    64,    70,     4,    77,     4,     4,    39,
      40,     4,    60,    44,     4,    60,    76,     4,     4,     4,
       4,    74,     3,    70,    64,    74,    72,    70,    62,    74,
      74,    70,    70,    29,    69,    69,    70,    70,    70,     4,
       4,     9,     9,     4,     4,     9,     4,    59,     9,     4,
       4,     5,    81,     4,     4,    13,    70,    74,    70,    70,
      74,    70,    70,    70,    75,     4,    75,    75,     4,     4,
       4,     4,    58,    66,    67,    70,    70,    75,    75,     3,
       4,    74,    40,     4,     9,     3,     4,    14,    67,    74,
       4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    58,    59,    59,    60,    60,    60,    60,
      61,    61,    62,    62,    63,    63,    64,    65,    66,    67,
      67,    68,    68,    69,    69,    70,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    72,    72,    73,    73,
      74,    74,    75,    75,    75,    75,    75,    75,    76,    77,
      78,    78,    78,    78,    79,    79,    80,    81,    82,    83,
      83,    84,    85,    86,    86,    87,    87,    88,    89,    89,
      89,    89,    90,    91,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    93,    93,    94,    95,    95,    95,    95,
      95,    95,    96,    96,    97,    98,    98,    99,    99,   100,
     101
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     2,     0,     4,     8,     4,
       1,     1,     1,     1,     0,     2,     1,     1,     1,     0,
       1,     0,     1,     0,     1,     3,     1,     3,     3,     7,
       4,    12,     3,     4,     3,     4,     3,     2,     4,     5,
       3,     3,     4,     2,     3,     4,     5,     2,     2,     3,
       4,     3,     2,     1,     1,     2,     0,     1,     1,     2,
       0,     1,     1,     5,     6,     5,     5,     6,     4,     1,
       6,     7,     5,     6,     0,     1,     4,     1,     5,     0,
       2,     1,     1,     6,     5,     7,     8,     4,     6,     7,
       6,     7,     5,     5,     0,     2,     2,     2,     2,     2,
       2,     2,     2,     4,     4,     1,     1,     5,     5,     9,
       8,     9,     0,     2,     4,     0,     1,     0,     2,     1,
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
#if WASM_AST_PARSER_DEBUG

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
# if defined WASM_AST_PARSER_LTYPE_IS_TRIVIAL && WASM_AST_PARSER_LTYPE_IS_TRIVIAL

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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmAstLexer* lexer, WasmAstParser* parser)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WasmAstLexer* lexer, WasmAstParser* parser)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, WasmAstLexer* lexer, WasmAstParser* parser)
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
#else /* !WASM_AST_PARSER_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !WASM_AST_PARSER_DEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, WasmAstLexer* lexer, WasmAstParser* parser)
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
          case 58: /* text_list  */
#line 157 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_text_list(parser->allocator, &((*yyvaluep).text_list)); }
#line 1392 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 59: /* value_type_list  */
#line 160 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(parser->allocator, &((*yyvaluep).types)); }
#line 1398 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 60: /* func_type  */
#line 170 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(parser->allocator, &((*yyvaluep).func_sig)); }
#line 1404 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 61: /* literal  */
#line 159 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).literal).text); }
#line 1410 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 62: /* var  */
#line 161 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(parser->allocator, &((*yyvaluep).var)); }
#line 1416 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 63: /* var_list  */
#line 162 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1422 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 64: /* bind_var  */
#line 158 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1428 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 65: /* quoted_text  */
#line 158 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1434 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 66: /* segment_contents  */
#line 167 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1440 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 67: /* labeling  */
#line 158 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1446 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 70: /* expr  */
#line 163 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr(parser->allocator, ((*yyvaluep).expr)); }
#line 1452 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 71: /* expr1  */
#line 163 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr(parser->allocator, ((*yyvaluep).expr)); }
#line 1458 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 72: /* expr_opt  */
#line 163 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr(parser->allocator, ((*yyvaluep).expr)); }
#line 1464 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 73: /* non_empty_expr_list  */
#line 164 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_list(parser->allocator, ((*yyvaluep).expr_list).first); }
#line 1470 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 74: /* expr_list  */
#line 164 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_list(parser->allocator, ((*yyvaluep).expr_list).first); }
#line 1476 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 75: /* func_fields  */
#line 165 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_fields(parser->allocator, ((*yyvaluep).func_fields)); }
#line 1482 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 77: /* func_info  */
#line 166 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1488 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 78: /* func  */
#line 174 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_exported_func(parser->allocator, &((*yyvaluep).exported_func)); }
#line 1494 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 82: /* segment  */
#line 167 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1500 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 83: /* segment_list  */
#line 168 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(parser->allocator, &((*yyvaluep).segments)); }
#line 1506 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 86: /* memory  */
#line 169 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(parser->allocator, &((*yyvaluep).memory)); }
#line 1512 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 87: /* type_def  */
#line 171 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(parser->allocator, &((*yyvaluep).func_type)); }
#line 1518 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 88: /* table  */
#line 162 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1524 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 89: /* import  */
#line 172 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(parser->allocator, ((*yyvaluep).import)); wasm_free(parser->allocator, ((*yyvaluep).import)); }
#line 1530 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 90: /* export  */
#line 173 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(parser->allocator, &((*yyvaluep).export_)); }
#line 1536 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 92: /* module_fields  */
#line 175 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1542 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 93: /* raw_module  */
#line 176 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_raw_module(parser->allocator, &((*yyvaluep).raw_module)); }
#line 1548 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 94: /* module  */
#line 175 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1554 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 95: /* cmd  */
#line 178 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(parser->allocator, ((*yyvaluep).command)); wasm_free(parser->allocator, ((*yyvaluep).command)); }
#line 1560 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 96: /* cmd_list  */
#line 179 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(parser->allocator, &((*yyvaluep).commands)); }
#line 1566 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 99: /* const_list  */
#line 177 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(parser->allocator, &((*yyvaluep).consts)); }
#line 1572 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 100: /* script  */
#line 180 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1578 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
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
yyparse (WasmAstLexer* lexer, WasmAstParser* parser)
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
# if defined WASM_AST_PARSER_LTYPE_IS_TRIVIAL && WASM_AST_PARSER_LTYPE_IS_TRIVIAL
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
#line 190 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 1877 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 3:
#line 196 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 1890 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 4:
#line 209 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.types)); }
#line 1896 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 5:
#line 210 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      wasm_append_type_value(parser->allocator, &(yyval.types), &(yyvsp[0].type));
    }
#line 1905 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 6:
#line 216 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); }
#line 1911 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 7:
#line 217 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 1920 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 8:
#line 221 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 1929 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 9:
#line 225 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 1935 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 10:
#line 232 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 1944 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 11:
#line 236 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 1953 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 12:
#line 243 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &index,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser, "invalid int " PRIstringslice,
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      (yyval.var).index = index;
    }
#line 1970 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 13:
#line 255 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 1980 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 14:
#line 262 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.vars)); }
#line 1986 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 15:
#line 263 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      wasm_append_var_value(parser->allocator, &(yyval.vars), &(yyvsp[0].var));
    }
#line 1995 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 16:
#line 269 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2001 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 17:
#line 273 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmTextListNode node;
      node.text = (yyvsp[0].text);
      node.next = NULL;
      WasmTextList text_list;
      text_list.first = &node;
      text_list.last = &node;
      void* data;
      size_t size;
      dup_text_list(parser->allocator, &text_list, &data, &size);
      (yyval.text).start = data;
      (yyval.text).length = size;
    }
#line 2019 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 18:
#line 289 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      dup_text_list(parser->allocator, &(yyvsp[0].text_list), &(yyval.segment).data, &(yyval.segment).size);
      wasm_destroy_text_list(parser->allocator, &(yyvsp[0].text_list));
    }
#line 2028 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 19:
#line 296 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.text)); }
#line 2034 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 20:
#line 297 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.text) = (yyvsp[0].text); }
#line 2040 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 21:
#line 301 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2046 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 22:
#line 302 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64)))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid offset \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2058 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 23:
#line 311 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2064 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 24:
#line 312 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid alignment \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2077 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 25:
#line 323 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); (yyval.expr)->loc = (yylsp[-2]); }
#line 2083 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 26:
#line 326 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_NOP);
    }
#line 2091 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 27:
#line 329 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_block_expr(parser->allocator);
      (yyval.expr)->block.label = (yyvsp[-1].text);
      (yyval.expr)->block.first = (yyvsp[0].expr_list).first;
    }
#line 2101 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 28:
#line 334 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      (yyval.expr)->if_.cond = (yyvsp[-1].expr);
      (yyval.expr)->if_.true_.first = (yyvsp[0].expr);
    }
#line 2111 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 29:
#line 339 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      (yyval.expr)->if_.cond = (yyvsp[-5].expr);
      (yyval.expr)->if_.true_.label = (yyvsp[-2].text);
      (yyval.expr)->if_.true_.first = (yyvsp[-1].expr_list).first;
    }
#line 2122 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 30:
#line 345 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      (yyval.expr)->if_else.cond = (yyvsp[-2].expr);
      (yyval.expr)->if_else.true_.first = (yyvsp[-1].expr);
      (yyval.expr)->if_else.false_.first = (yyvsp[0].expr);
    }
#line 2133 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 31:
#line 351 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      (yyval.expr)->if_else.cond = (yyvsp[-10].expr);
      (yyval.expr)->if_else.true_.label = (yyvsp[-7].text);
      (yyval.expr)->if_else.true_.first = (yyvsp[-6].expr_list).first;
      (yyval.expr)->if_else.false_.label = (yyvsp[-2].text);
      (yyval.expr)->if_else.false_.first = (yyvsp[-1].expr_list).first;
    }
#line 2146 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 32:
#line 359 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      (yyval.expr)->br_if.var = (yyvsp[-1].var);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2156 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 33:
#line 364 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      (yyval.expr)->br_if.var = (yyvsp[-2].var);
      (yyval.expr)->br_if.expr = (yyvsp[-1].expr);
      (yyval.expr)->br_if.cond = (yyvsp[0].expr);
    }
#line 2167 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 34:
#line 370 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      WASM_ZERO_MEMORY((yyval.expr)->loop.outer);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.first = (yyvsp[0].expr_list).first;
    }
#line 2178 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 35:
#line 376 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      (yyval.expr)->loop.outer = (yyvsp[-2].text);
      (yyval.expr)->loop.inner = (yyvsp[-1].text);
      (yyval.expr)->loop.first = (yyvsp[0].expr_list).first;
    }
#line 2189 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 36:
#line 382 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      (yyval.expr)->br.var = (yyvsp[-1].var);
      (yyval.expr)->br.expr = (yyvsp[0].expr);
    }
#line 2199 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 37:
#line 387 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_return_expr(parser->allocator);
      (yyval.expr)->return_.expr = (yyvsp[0].expr);
    }
#line 2208 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 38:
#line 391 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_table_expr(parser->allocator);
      (yyval.expr)->br_table.key = (yyvsp[0].expr);
      (yyval.expr)->br_table.expr = NULL;
      (yyval.expr)->br_table.targets = (yyvsp[-2].vars);
      (yyval.expr)->br_table.default_target = (yyvsp[-1].var);
    }
#line 2220 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 39:
#line 398 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_table_expr(parser->allocator);
      (yyval.expr)->br_table.key = (yyvsp[0].expr);
      (yyval.expr)->br_table.expr = (yyvsp[-1].expr);
      (yyval.expr)->br_table.targets = (yyvsp[-3].vars);
      (yyval.expr)->br_table.default_target = (yyvsp[-2].var);
    }
#line 2232 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 40:
#line 405 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_expr(parser->allocator);
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.first_arg = (yyvsp[0].expr_list).first;
      (yyval.expr)->call.num_args = (yyvsp[0].expr_list).size;
    }
#line 2243 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 41:
#line 411 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_import_expr(parser->allocator);
      (yyval.expr)->call.var = (yyvsp[-1].var);
      (yyval.expr)->call.first_arg = (yyvsp[0].expr_list).first;
      (yyval.expr)->call.num_args = (yyvsp[0].expr_list).size;
    }
#line 2254 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 42:
#line 417 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_indirect_expr(parser->allocator);
      (yyval.expr)->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr)->call_indirect.expr = (yyvsp[-1].expr);
      (yyval.expr)->call_indirect.first_arg = (yyvsp[0].expr_list).first;
      (yyval.expr)->call_indirect.num_args = (yyvsp[0].expr_list).size;
    }
#line 2266 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 43:
#line 424 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_get_local_expr(parser->allocator);
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2275 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 44:
#line 428 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_set_local_expr(parser->allocator);
      (yyval.expr)->set_local.var = (yyvsp[-1].var);
      (yyval.expr)->set_local.expr = (yyvsp[0].expr);
    }
#line 2285 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 45:
#line 433 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_load_expr(parser->allocator);
      (yyval.expr)->load.opcode = (yyvsp[-3].opcode);
      (yyval.expr)->load.offset = (yyvsp[-2].u64);
      (yyval.expr)->load.align = (yyvsp[-1].u32);
      (yyval.expr)->load.addr = (yyvsp[0].expr);
    }
#line 2297 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 46:
#line 440 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_store_expr(parser->allocator);
      (yyval.expr)->store.opcode = (yyvsp[-4].opcode);
      (yyval.expr)->store.offset = (yyvsp[-3].u64);
      (yyval.expr)->store.align = (yyvsp[-2].u32);
      (yyval.expr)->store.addr = (yyvsp[-1].expr);
      (yyval.expr)->store.value = (yyvsp[0].expr);
    }
#line 2310 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 47:
#line 448 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_const_expr(parser->allocator);
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (WASM_FAILED(parse_const((yyvsp[-1].type), (yyvsp[0].literal).type, (yyvsp[0].literal).text.start,
                                  (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length,
                                  &(yyval.expr)->const_))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      wasm_free(parser->allocator, (char*)(yyvsp[0].literal).text.start);
    }
#line 2327 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 48:
#line 460 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_unary_expr(parser->allocator);
      (yyval.expr)->unary.opcode = (yyvsp[-1].opcode);
      (yyval.expr)->unary.expr = (yyvsp[0].expr);
    }
#line 2337 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 49:
#line 465 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_binary_expr(parser->allocator);
      (yyval.expr)->binary.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->binary.left = (yyvsp[-1].expr);
      (yyval.expr)->binary.right = (yyvsp[0].expr);
    }
#line 2348 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 50:
#line 471 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_select_expr(parser->allocator);
      (yyval.expr)->select.true_ = (yyvsp[-2].expr);
      (yyval.expr)->select.false_ = (yyvsp[-1].expr);
      (yyval.expr)->select.cond = (yyvsp[0].expr);
    }
#line 2359 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 51:
#line 477 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_compare_expr(parser->allocator);
      (yyval.expr)->compare.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->compare.left = (yyvsp[-1].expr);
      (yyval.expr)->compare.right = (yyvsp[0].expr);
    }
#line 2370 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 52:
#line 483 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_convert_expr(parser->allocator);
      (yyval.expr)->convert.opcode = (yyvsp[-1].opcode);
      (yyval.expr)->convert.expr = (yyvsp[0].expr);
    }
#line 2380 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 53:
#line 488 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_UNREACHABLE);
    }
#line 2388 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 54:
#line 491 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_empty_expr(parser->allocator,
                               WASM_EXPR_TYPE_CURRENT_MEMORY);
    }
#line 2397 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 55:
#line 495 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_grow_memory_expr(parser->allocator);
      (yyval.expr)->grow_memory.expr = (yyvsp[0].expr);
    }
#line 2406 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 56:
#line 501 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2412 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 58:
#line 505 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyval.expr_list).last = (yyvsp[0].expr);
      (yyval.expr_list).size = 1;
    }
#line 2421 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 59:
#line 509 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
      (yyval.expr_list).last->next = (yyvsp[0].expr);
      (yyval.expr_list).last = (yyvsp[0].expr);
      (yyval.expr_list).size++;
    }
#line 2432 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 60:
#line 517 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.expr_list)); }
#line 2438 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 62:
#line 523 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_EXPRS;
      (yyval.func_fields)->first_expr = (yyvsp[0].expr_list).first;
      (yyval.func_fields)->next = NULL;
    }
#line 2449 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 63:
#line 529 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_PARAM_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2460 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 64:
#line 535 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_PARAM;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2473 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 65:
#line 543 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_RESULT_TYPE;
      (yyval.func_fields)->result_type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2484 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 66:
#line 549 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_LOCAL_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2495 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 67:
#line 555 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_LOCAL;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2508 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 68:
#line 565 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2514 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 69:
#line 568 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
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
            (yyval.func)->first_expr = field->first_expr;
            break;

          case WASM_FUNC_FIELD_TYPE_PARAM_TYPES:
          case WASM_FUNC_FIELD_TYPE_LOCAL_TYPES: {
            WasmTypeVector* types =
                field->type == WASM_FUNC_FIELD_TYPE_PARAM_TYPES
                    ? &(yyval.func)->decl.sig.param_types
                    : &(yyval.func)->local_types;
            wasm_extend_types(parser->allocator, types, &field->types);
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

            wasm_append_type_value(parser->allocator, types,
                                   &field->bound_type.type);
            WasmBinding* binding = wasm_insert_binding(
                parser->allocator, bindings, &field->bound_type.name);
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
#line 2578 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 70:
#line 629 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-4]);
      (yyval.exported_func).func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.exported_func).func->decl.type_var = (yyvsp[-2].var);
      (yyval.exported_func).export_.name = (yyvsp[-3].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 2592 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 71:
#line 638 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-5]);
      (yyval.exported_func).func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.exported_func).func->decl.type_var = (yyvsp[-2].var);
      (yyval.exported_func).func->name = (yyvsp[-3].text);
      (yyval.exported_func).export_.name = (yyvsp[-4].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 2607 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 72:
#line 648 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-3]);
      (yyval.exported_func).func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.exported_func).export_.name = (yyvsp[-2].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 2620 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 73:
#line 656 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-4]);
      (yyval.exported_func).func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.exported_func).func->name = (yyvsp[-2].text);
      (yyval.exported_func).export_.name = (yyvsp[-3].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 2634 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 74:
#line 668 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.text)); }
#line 2640 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 76:
#line 676 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2646 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 77:
#line 680 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid memory segment address \"" PRIstringslice
                              "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2661 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 78:
#line 693 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      (yyval.segment).addr = (yyvsp[-2].u32);
    }
#line 2672 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 79:
#line 701 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.segments)); }
#line 2678 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 80:
#line 702 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      wasm_append_segment_value(parser->allocator, &(yyval.segments), &(yyvsp[0].segment));
    }
#line 2687 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 81:
#line 709 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid initial memory pages \"" PRIstringslice
                              "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2701 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 82:
#line 721 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid max memory pages \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2714 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 83:
#line 732 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      (yyval.memory).initial_pages = (yyvsp[-3].u64);
      (yyval.memory).max_pages = (yyvsp[-2].u64);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 2725 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 84:
#line 738 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      (yyval.memory).initial_pages = (yyvsp[-2].u64);
      (yyval.memory).max_pages = (yyval.memory).initial_pages;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 2736 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 85:
#line 747 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WASM_ZERO_MEMORY((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 2745 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 86:
#line 751 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 2754 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 87:
#line 758 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 2760 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 88:
#line 762 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 2772 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 89:
#line 769 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 2785 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 90:
#line 777 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 2797 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 91:
#line 784 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 2810 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 92:
#line 795 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_).name = (yyvsp[-2].text);
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 2819 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 93:
#line 802 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_memory).name = (yyvsp[-2].text);
    }
#line 2827 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 94:
#line 808 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new_module(parser->allocator);
    }
#line 2835 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 95:
#line 811 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *(yyvsp[0].exported_func).func;
      wasm_free(parser->allocator, (yyvsp[0].exported_func).func);

      WasmFunc* func_ptr = &field->func;
      wasm_append_func_ptr_value(parser->allocator, &(yyval.module)->funcs, &func_ptr);
      if (field->func.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &(yyval.module)->func_bindings, &field->func.name);
        binding->loc = field->loc;
        binding->index = (yyval.module)->funcs.size - 1;
      }

      /* is this function using the export syntactic sugar? */
      if ((yyvsp[0].exported_func).export_.name.start != NULL) {
        WasmModuleField* export_field =
            wasm_append_module_field(parser->allocator, (yyval.module));
        export_field->loc = (yylsp[0]);
        export_field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
        export_field->export_ = (yyvsp[0].exported_func).export_;
        export_field->export_.var.index = (yyval.module)->funcs.size - 1;

        WasmExport* export_ptr = &export_field->export_;
        wasm_append_export_ptr_value(parser->allocator, &(yyval.module)->exports,
                                     &export_ptr);
        WasmBinding* binding =
            wasm_insert_binding(parser->allocator, &(yyval.module)->export_bindings,
                                &export_field->export_.name);
        binding->loc = export_field->loc;
        binding->index = (yyval.module)->exports.size - 1;
      }
    }
#line 2876 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 96:
#line 847 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *(yyvsp[0].import);
      wasm_free(parser->allocator, (yyvsp[0].import));

      WasmImport* import_ptr = &field->import;
      wasm_append_import_ptr_value(parser->allocator, &(yyval.module)->imports,
                                   &import_ptr);
      if (field->import.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &(yyval.module)->import_bindings, &field->import.name);
        binding->loc = field->loc;
        binding->index = (yyval.module)->imports.size - 1;
      }
    }
#line 2899 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 97:
#line 865 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = (yyvsp[0].export_);

      WasmExport* export_ptr = &field->export_;
      wasm_append_export_ptr_value(parser->allocator, &(yyval.module)->exports,
                                   &export_ptr);
      if (field->export_.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &(yyval.module)->export_bindings, &field->export_.name);
        binding->loc = field->loc;
        binding->index = (yyval.module)->exports.size - 1;
      }
    }
#line 2921 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 98:
#line 882 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = (yyvsp[0].export_memory);
      (yyval.module)->export_memory = &field->export_memory;
    }
#line 2934 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 99:
#line 890 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
      (yyval.module)->table = &field->table;
    }
#line 2947 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 100:
#line 898 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = (yyvsp[0].func_type);

      WasmFuncType* func_type_ptr = &field->func_type;
      wasm_append_func_type_ptr_value(parser->allocator, &(yyval.module)->func_types,
                                      &func_type_ptr);
      if (field->func_type.name.start) {
        WasmBinding* binding =
            wasm_insert_binding(parser->allocator, &(yyval.module)->func_type_bindings,
                                &field->func_type.name);
        binding->loc = field->loc;
        binding->index = (yyval.module)->func_types.size - 1;
      }
    }
#line 2970 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 101:
#line 916 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
      (yyval.module)->memory = &field->memory;
    }
#line 2983 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 102:
#line 924 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = (yyvsp[0].var);
      (yyval.module)->start = &field->start;
    }
#line 2996 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 103:
#line 935 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WASM_RAW_MODULE_TYPE_TEXT;
      (yyval.raw_module).text = (yyvsp[-1].module);
      (yyval.raw_module).loc = (yylsp[-2]);
      WasmModule* module = (yyval.raw_module).text;

      size_t i;
      for (i = 0; i < module->funcs.size; ++i) {
        WasmFunc* func = module->funcs.data[i];
        copy_signature_from_func_type(parser->allocator, module, &func->decl);
      }

      for (i = 0; i < module->imports.size; ++i) {
        WasmImport* import = module->imports.data[i];
        copy_signature_from_func_type(parser->allocator, module, &import->decl);
      }
    }
#line 3018 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 104:
#line 952 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WASM_RAW_MODULE_TYPE_BINARY;
      (yyval.raw_module).loc = (yylsp[-2]);
      dup_text_list(parser->allocator, &(yyvsp[-1].text_list), &(yyval.raw_module).binary.data, &(yyval.raw_module).binary.size);
      wasm_destroy_text_list(parser->allocator, &(yyvsp[-1].text_list));
    }
#line 3029 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 105:
#line 961 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].raw_module).type == WASM_RAW_MODULE_TYPE_TEXT) {
        (yyval.module) = (yyvsp[0].raw_module).text;
      } else {
        assert((yyvsp[0].raw_module).type == WASM_RAW_MODULE_TYPE_BINARY);
        (yyval.module) = new_module(parser->allocator);
        WasmReadBinaryOptions options = WASM_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorCallbackData user_data;
        user_data.loc = &(yyvsp[0].raw_module).loc;
        user_data.lexer = lexer;
        user_data.parser = parser;
        WasmBinaryErrorHandler error_handler;
        error_handler.on_error = on_read_binary_error;
        error_handler.user_data = &user_data;
        wasm_read_binary_ast(parser->allocator, (yyvsp[0].raw_module).binary.data, (yyvsp[0].raw_module).binary.size,
                             &options, &error_handler, (yyval.module));
        wasm_free(parser->allocator, (yyvsp[0].raw_module).binary.data);
      }
    }
#line 3053 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 106:
#line 985 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_MODULE;
      (yyval.command)->module = *(yyvsp[0].module);
      wasm_free(parser->allocator, (yyvsp[0].module));
    }
#line 3064 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 107:
#line 991 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command)->invoke.loc = (yylsp[-3]);
      (yyval.command)->invoke.name = (yyvsp[-2].text);
      (yyval.command)->invoke.args = (yyvsp[-1].consts);
    }
#line 3076 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 108:
#line 998 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 3087 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1004 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command)->assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_return.expected = (yyvsp[-1].const_);
    }
#line 3100 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1012 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command)->assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command)->assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command)->assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3112 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1019 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command)->assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3125 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1029 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.commands)); }
#line 3131 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1030 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      wasm_append_command_value(parser->allocator, &(yyval.commands), (yyvsp[0].command));
      wasm_free(parser->allocator, (yyvsp[0].command));
    }
#line 3141 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1038 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (WASM_FAILED(parse_const((yyvsp[-2].type), (yyvsp[-1].literal).type, (yyvsp[-1].literal).text.start,
                                  (yyvsp[-1].literal).text.start + (yyvsp[-1].literal).text.length, &(yyval.const_)))) {
        wasm_ast_parser_error(&(yylsp[-1]), lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[-1].literal).text));
      }
      wasm_free(parser->allocator, (char*)(yyvsp[-1].literal).text.start);
    }
#line 3156 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1050 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3162 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1054 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.consts)); }
#line 3168 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1055 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      wasm_append_const_value(parser->allocator, &(yyval.consts), &(yyvsp[0].const_));
    }
#line 3177 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1062 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script).commands = (yyvsp[0].commands);
      parser->script = (yyval.script);
    }
#line 3186 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;


#line 3190 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
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
#line 1074 "src/wasm-ast-parser.y" /* yacc.c:1906  */


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

static size_t copy_string_contents(WasmStringSlice* text, char* dest) {
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

static void dup_text_list(WasmAllocator* allocator,
                          WasmTextList* text_list,
                          void** out_data,
                          size_t* out_size) {
  /* walk the linked list to see how much total space is needed */
  size_t total_size = 0;
  WasmTextListNode* node;
  for (node = text_list->first; node; node = node->next) {
    /* Always allocate enough space for the entire string including the escape
     * characters. It will only get shorter, and this way we only have to
     * iterate through the string once. */
    const char* src = node->text.start + 1;
    const char* end = node->text.start + node->text.length - 1;
    size_t size = end - src;
    total_size += size;
  }
  char* result = wasm_alloc(allocator, total_size, 1);
  char* dest = result;
  for (node = text_list->first; node; node = node->next) {
    size_t actual_size = copy_string_contents(&node->text, dest);
    dest += actual_size;
  }
  *out_data = result;
  *out_size = dest - result;
}

WasmResult wasm_parse_ast(WasmAstLexer* lexer,
                          struct WasmScript* out_script,
                          WasmSourceErrorHandler* error_handler) {
  WasmAstParser parser;
  WASM_ZERO_MEMORY(parser);
  WasmAllocator* allocator = wasm_ast_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  parser.error_handler = error_handler;
  out_script->allocator = allocator;
  int result = wasm_ast_parser_parse(lexer, &parser);
  out_script->commands = parser.script.commands;
  return result == 0 && parser.errors == 0 ? WASM_OK : WASM_ERROR;
}

static void copy_signature_from_func_type(WasmAllocator* allocator,
                                          WasmModule* module,
                                          WasmFuncDeclaration* decl) {
  /* if a function or import only defines a func type (and no explicit
   * signature), copy the signature over for convenience */
  if (wasm_decl_has_func_type(decl) && !wasm_decl_has_signature(decl)) {
    int index = wasm_get_func_type_index_by_var(module, &decl->type_var);
    if (index >= 0 && (size_t)index < module->func_types.size) {
      WasmFuncType* func_type = module->func_types.data[index];
      decl->sig.result_type = func_type->sig.result_type;
      wasm_extend_types(allocator, &decl->sig.param_types,
                        &func_type->sig.param_types);
    } else {
      /* technically not OK, but we'll catch this error later in the AST
       * checker */
    }
  }
}

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data) {
  BinaryErrorCallbackData* data = user_data;
  if (offset == WASM_UNKNOWN_OFFSET) {
    wasm_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: %s", error);
  } else {
    wasm_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: @0x%08x: %s", offset, error);
  }
}
