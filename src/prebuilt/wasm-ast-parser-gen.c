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

/* the default value for YYMAXDEPTH is 10000, which can be easily hit since our
   grammar is right-recursive.

   we can increase YYMAXDEPTH, but the generated parser says that "results are
   undefined" if YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES(YYMAXDEPTH) with
   infinite-precision arithmetic. That's tricky to write a static assertion
   for, so let's "just" limit YYSTACK_BYTES(YYMAXDEPTH) to UINT32_MAX and use
   64-bit arithmetic. this static assert is done at the end of the file, so all
   defines are available. */
#define YYMAXDEPTH 10000000

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

static WasmExprList join_exprs1(WasmLocation* loc, WasmExpr* expr1);
static WasmExprList join_exprs2(WasmLocation* loc, WasmExprList* expr1,
                                WasmExpr* expr2);
static WasmExprList join_exprs3(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExpr* expr3);
static WasmExprList join_exprs4(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExprList* expr3,
                                WasmExpr* expr4);

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

static WasmBool is_empty_signature(WasmFuncSignature* sig);

static void append_implicit_func_declaration(WasmAllocator*, WasmLocation*,
                                             WasmModule*, WasmFuncDeclaration*);

typedef struct BinaryErrorCallbackData {
  WasmLocation* loc;
  WasmAstLexer* lexer;
  WasmAstParser* parser;
} BinaryErrorCallbackData;

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data);

#define wasm_ast_parser_lex wasm_ast_lexer_lex


#line 179 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:339  */

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
    WASM_TOKEN_TYPE_NAT = 260,
    WASM_TOKEN_TYPE_INT = 261,
    WASM_TOKEN_TYPE_FLOAT = 262,
    WASM_TOKEN_TYPE_TEXT = 263,
    WASM_TOKEN_TYPE_VAR = 264,
    WASM_TOKEN_TYPE_VALUE_TYPE = 265,
    WASM_TOKEN_TYPE_NOP = 266,
    WASM_TOKEN_TYPE_DROP = 267,
    WASM_TOKEN_TYPE_BLOCK = 268,
    WASM_TOKEN_TYPE_END = 269,
    WASM_TOKEN_TYPE_IF = 270,
    WASM_TOKEN_TYPE_THEN = 271,
    WASM_TOKEN_TYPE_ELSE = 272,
    WASM_TOKEN_TYPE_LOOP = 273,
    WASM_TOKEN_TYPE_BR = 274,
    WASM_TOKEN_TYPE_BR_IF = 275,
    WASM_TOKEN_TYPE_BR_TABLE = 276,
    WASM_TOKEN_TYPE_CALL = 277,
    WASM_TOKEN_TYPE_CALL_IMPORT = 278,
    WASM_TOKEN_TYPE_CALL_INDIRECT = 279,
    WASM_TOKEN_TYPE_RETURN = 280,
    WASM_TOKEN_TYPE_GET_LOCAL = 281,
    WASM_TOKEN_TYPE_SET_LOCAL = 282,
    WASM_TOKEN_TYPE_TEE_LOCAL = 283,
    WASM_TOKEN_TYPE_GET_GLOBAL = 284,
    WASM_TOKEN_TYPE_SET_GLOBAL = 285,
    WASM_TOKEN_TYPE_LOAD = 286,
    WASM_TOKEN_TYPE_STORE = 287,
    WASM_TOKEN_TYPE_OFFSET = 288,
    WASM_TOKEN_TYPE_ALIGN = 289,
    WASM_TOKEN_TYPE_CONST = 290,
    WASM_TOKEN_TYPE_UNARY = 291,
    WASM_TOKEN_TYPE_BINARY = 292,
    WASM_TOKEN_TYPE_COMPARE = 293,
    WASM_TOKEN_TYPE_CONVERT = 294,
    WASM_TOKEN_TYPE_SELECT = 295,
    WASM_TOKEN_TYPE_UNREACHABLE = 296,
    WASM_TOKEN_TYPE_CURRENT_MEMORY = 297,
    WASM_TOKEN_TYPE_GROW_MEMORY = 298,
    WASM_TOKEN_TYPE_FUNC = 299,
    WASM_TOKEN_TYPE_START = 300,
    WASM_TOKEN_TYPE_TYPE = 301,
    WASM_TOKEN_TYPE_PARAM = 302,
    WASM_TOKEN_TYPE_RESULT = 303,
    WASM_TOKEN_TYPE_LOCAL = 304,
    WASM_TOKEN_TYPE_GLOBAL = 305,
    WASM_TOKEN_TYPE_MODULE = 306,
    WASM_TOKEN_TYPE_MEMORY = 307,
    WASM_TOKEN_TYPE_SEGMENT = 308,
    WASM_TOKEN_TYPE_IMPORT = 309,
    WASM_TOKEN_TYPE_EXPORT = 310,
    WASM_TOKEN_TYPE_TABLE = 311,
    WASM_TOKEN_TYPE_ASSERT_INVALID = 312,
    WASM_TOKEN_TYPE_ASSERT_RETURN = 313,
    WASM_TOKEN_TYPE_ASSERT_RETURN_NAN = 314,
    WASM_TOKEN_TYPE_ASSERT_TRAP = 315,
    WASM_TOKEN_TYPE_INVOKE = 316,
    WASM_TOKEN_TYPE_LOW = 317
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

#line 315 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:358  */

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
#define YYLAST   736

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  161
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  362

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

#if WASM_AST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   214,   214,   220,   233,   234,   240,   241,   245,   249,
     256,   267,   271,   275,   282,   294,   301,   302,   308,   312,
     328,   335,   336,   338,   341,   342,   352,   353,   364,   365,
     368,   371,   374,   377,   382,   387,   396,   401,   406,   412,
     415,   420,   427,   430,   434,   438,   442,   446,   450,   454,
     458,   462,   468,   474,   486,   490,   494,   498,   502,   505,
     510,   513,   516,   519,   525,   531,   541,   547,   553,   559,
     565,   572,   579,   583,   587,   592,   598,   604,   612,   618,
     622,   627,   632,   637,   642,   647,   652,   657,   662,   669,
     676,   689,   694,   699,   704,   709,   713,   719,   720,   730,
     731,   737,   743,   753,   759,   765,   775,   778,   833,   842,
     852,   859,   870,   871,   878,   882,   887,   896,   909,   917,
     918,   925,   937,   948,   954,   963,   967,   974,   978,   985,
     993,   999,  1009,  1016,  1022,  1025,  1064,  1081,  1102,  1119,
    1127,  1135,  1153,  1161,  1172,  1206,  1215,  1239,  1245,  1252,
    1258,  1266,  1273,  1283,  1284,  1292,  1304,  1305,  1308,  1309,
    1316,  1325
};
#endif

#if WASM_AST_PARSER_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "NAT", "INT",
  "FLOAT", "TEXT", "VAR", "VALUE_TYPE", "NOP", "DROP", "BLOCK", "END",
  "IF", "THEN", "ELSE", "LOOP", "BR", "BR_IF", "BR_TABLE", "CALL",
  "CALL_IMPORT", "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL",
  "TEE_LOCAL", "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE", "OFFSET",
  "ALIGN", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT", "SELECT",
  "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY", "FUNC", "START", "TYPE",
  "PARAM", "RESULT", "LOCAL", "GLOBAL", "MODULE", "MEMORY", "SEGMENT",
  "IMPORT", "EXPORT", "TABLE", "ASSERT_INVALID", "ASSERT_RETURN",
  "ASSERT_RETURN_NAN", "ASSERT_TRAP", "INVOKE", "LOW", "$accept",
  "text_list", "value_type_list", "func_type", "nat", "literal", "var",
  "var_list", "bind_var", "quoted_text", "segment_contents", "labeling",
  "labeling1", "offset", "align", "expr", "op", "expr1", "expr_list",
  "func_fields", "func_body", "type_use", "func_info", "func",
  "export_opt", "start", "global", "segment_address", "segment",
  "segment_list", "initial_pages", "max_pages", "memory", "type_def",
  "table", "import", "export", "export_memory", "module_fields",
  "raw_module", "module", "cmd", "cmd_list", "const", "const_opt",
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
     315,   316,   317
};
# endif

#define YYPACT_NINF -289

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-289)))

#define YYTABLE_NINF -98

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -289,    10,  -289,    18,    -9,  -289,  -289,  -289,  -289,    30,
      32,    43,    74,    84,    52,  -289,    37,    59,    28,    52,
      29,    40,    42,  -289,  -289,  -289,  -289,    65,  -289,  -289,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,   101,    52,
      52,    52,    95,    52,    64,    25,   123,   102,   135,    52,
    -289,  -289,  -289,  -289,  -289,    90,  -289,  -289,  -289,   268,
    -289,  -289,   114,    92,  -289,   125,   350,   128,  -289,   143,
      52,    52,    31,    66,   154,   160,   163,   107,   547,  -289,
    -289,   146,   146,   146,   165,   165,   165,    64,    64,    64,
    -289,    64,    64,    64,    64,    64,   126,   126,   107,  -289,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,   391,   350,  -289,
    -289,  -289,  -289,   432,   174,  -289,   181,   141,   693,   182,
     350,  -289,   169,  -289,    52,   186,   187,   188,  -289,  -289,
     190,   191,    52,  -289,  -289,  -289,   193,  -289,   350,   146,
     350,   146,    64,    64,  -289,    64,    64,    64,   350,    64,
      64,    64,    64,    64,   126,   126,   107,   350,   350,   350,
     350,   350,  -289,  -289,   350,    64,   146,   184,   146,   194,
    -289,   350,  -289,   350,   350,   146,  -289,    64,    64,  -289,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,   168,
     168,  -289,   432,   195,  -289,   586,   202,  -289,   127,   208,
     181,  -289,   209,   161,  -289,  -289,   177,   186,    76,   211,
     213,  -289,  -289,  -289,   214,  -289,   215,  -289,  -289,   350,
     473,   203,   350,   146,   350,   350,    64,   350,   350,   350,
    -289,  -289,   350,   350,  -289,   350,   168,   168,  -289,  -289,
     350,   350,  -289,   350,  -289,   220,    27,   216,   223,    54,
     221,  -289,   218,    12,   225,   350,  -289,  -289,    64,  -289,
    -289,  -289,   230,  -289,  -289,  -289,   227,   236,   237,  -289,
     239,  -289,   238,   241,  -289,  -289,  -289,  -289,  -289,   660,
     309,   350,  -289,   350,  -289,   350,   350,  -289,  -289,   350,
    -289,  -289,  -289,   350,   350,  -289,  -289,   350,  -289,   432,
    -289,   242,   514,   514,   243,  -289,  -289,   146,  -289,   234,
      83,  -289,    57,   245,  -289,   247,  -289,    30,  -289,  -289,
     146,   309,  -289,  -289,  -289,   350,  -289,  -289,   350,  -289,
    -289,   432,   625,  -289,  -289,   514,   350,  -289,   249,  -289,
    -289,   246,   251,   350,  -289,  -289,  -289,  -289,   244,   205,
    -289,   252,  -289,   250,   254,   255,   248,  -289,   146,   350,
     258,  -289
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     153,   160,   161,     0,     0,   146,   147,   154,     1,   134,
       0,     0,     0,     0,     0,     2,     0,     0,     0,     0,
       0,     0,     0,    19,   158,   145,     3,     0,   144,   135,
     143,   136,   142,   141,   140,   137,   138,   139,     0,     0,
       0,     0,     0,   112,     0,     0,     0,     0,     0,     0,
      16,   149,   158,   158,   158,     0,   148,   159,   113,    97,
      14,    15,     0,     0,    18,     0,     0,     0,   121,   119,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      32,    21,    21,    21,     0,     0,     0,     0,     0,     0,
      39,     0,     0,     0,     0,     0,    24,    24,     0,    54,
      55,    56,    57,    42,    30,    58,    59,    97,    97,    28,
     103,   107,    99,    97,     0,   114,     6,     0,     0,     0,
       0,   122,     0,   119,     0,     6,     0,     0,   127,    17,
     156,     0,     0,    11,    12,    13,     0,    61,     0,    21,
      97,    21,     0,     0,    16,     0,     0,     0,    72,     0,
       0,     0,     0,     0,    24,    24,     0,     0,     0,     0,
       0,     0,    60,    95,     0,     0,     4,     0,     4,     0,
      23,    97,    22,    97,    97,    22,    10,     0,     0,    16,
      43,    44,    45,    46,    47,    48,    49,    50,    25,    26,
      26,    53,    97,     0,    98,     0,     0,   110,     0,     0,
       6,   115,     0,     0,   124,   120,     0,     6,     0,     0,
       0,   133,   132,   157,     0,   151,     0,   155,    62,    97,
      97,     0,    97,    22,    66,     0,     0,    97,    97,     0,
      73,    83,     0,     0,    86,     0,    26,    26,    90,    91,
       0,     0,    94,     0,    96,     0,     0,     0,     0,     0,
       0,    29,     0,     0,     0,    97,    36,    37,     0,    27,
      51,    52,     0,   111,   108,     4,     0,     0,     0,   116,
       0,   123,     0,     0,   130,   128,   150,   152,    63,     0,
      74,    97,    64,    97,    67,    68,    17,    80,    81,    97,
      84,    85,    87,     0,     0,    92,    93,     0,   106,    97,
       5,     0,    97,    97,     0,    33,    40,    21,    34,     0,
      38,   109,     0,     0,   125,     0,   117,     0,   131,   129,
      21,    75,    78,    65,    69,    70,    82,    88,     0,    79,
     101,    97,     0,   100,   104,    97,    97,    35,     7,     9,
     126,    20,     0,    97,    71,    89,   102,   105,     0,     0,
     118,     0,    41,     0,    76,     0,     0,     8,    21,    97,
       0,    77
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -289,   -54,  -163,  -118,    97,   -88,    58,  -135,   -29,   -16,
    -289,   -82,   -69,   -89,  -151,   -64,  -289,  -289,   -93,  -288,
    -282,  -103,  -101,  -289,  -289,  -289,  -289,  -289,  -289,   144,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,   259,
    -289,  -289,  -289,   138,  -289,    88,  -289,  -289
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    16,   246,   199,   177,   136,    62,    73,   170,    24,
     342,   171,   172,   189,   260,   108,   109,   169,   110,   111,
     112,   113,   114,    29,    59,    30,    31,   317,   205,   122,
      69,   123,    32,    33,    34,    35,    36,    37,    17,     5,
       6,     7,     1,    57,   214,    42,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     173,   174,   119,    38,   192,   249,   193,   209,   190,   226,
     191,   330,   196,     4,   175,   194,    65,    67,     8,    70,
     333,   334,   210,    52,    53,    54,   306,    58,    63,   307,
     107,   299,    71,    72,    64,    18,    60,   300,    15,   261,
      61,    25,     9,   346,   258,    26,    20,   221,    10,    11,
      12,    13,    14,   347,   124,   125,   202,   219,   303,   222,
      23,   338,    27,    28,   300,   236,   237,   300,   238,    60,
     128,    60,   223,    61,   218,    61,   220,    21,   252,     9,
     253,   254,   268,   126,   230,   293,   294,    22,   -17,   272,
      39,   262,   -17,   239,   240,   241,   242,   243,    55,    56,
     244,    40,   312,    41,   273,    51,   255,    68,   207,    43,
      44,    45,   133,   134,   135,    46,   216,    47,   115,    48,
      49,    50,   165,   265,   266,    77,   278,   194,   117,   282,
     127,   129,    64,    66,   287,   288,   116,   247,   120,   250,
      74,    75,    76,    23,    64,   180,   181,   182,   121,   183,
     184,   185,   186,   187,   283,    64,   280,    55,   130,   188,
     284,   285,   309,    55,   131,   289,    55,   132,   290,   291,
     176,   292,   203,   204,   265,   266,   295,   296,   197,   297,
     203,   271,   178,   179,   198,   200,   201,   194,   322,   208,
     323,   211,   212,    55,   248,   215,   326,   217,   251,   263,
     224,   225,   259,   227,   228,   229,   264,   231,   232,   233,
     234,   235,   267,   269,   270,   274,   321,   275,   276,   277,
     281,   324,   325,   245,   298,   336,   301,   302,   194,   327,
     328,   304,   305,   329,   311,   256,   257,   313,   343,   308,
     314,   315,   318,   348,   316,   319,   331,   335,   337,   339,
     351,   340,   349,   353,    26,   350,   354,   356,   352,   357,
     355,   344,   361,   341,   345,   358,   360,   206,   213,    19,
       0,    78,     0,     0,     0,     0,   359,    64,     0,    79,
      80,    81,     0,    82,   286,     0,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   118,     0,     0,     0,   310,     0,     0,     0,
      79,    80,    81,     0,    82,     0,   -97,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   118,     0,     0,     0,     0,     0,     0,
       0,    79,    80,    81,     0,    82,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    78,     0,     0,     0,     0,     0,
       0,     0,    79,    80,    81,     0,    82,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   195,     0,     0,     0,     0,
       0,     0,     0,    79,    80,    81,     0,    82,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   279,     0,     0,     0,
       0,     0,     0,     0,    79,    80,    81,     0,    82,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   332,     0,     0,
       0,     0,     0,     0,     0,    79,    80,    81,     0,    82,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   137,   138,
     139,     0,   140,     0,     0,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,   156,   157,   158,   159,   160,   161,   162,   163,
     164,     0,     0,   165,   166,   167,   168,   137,   138,   139,
       0,   140,     0,     0,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,     0,
       0,   156,   157,   158,   159,   160,   161,   162,   163,   164,
       0,     0,     0,   166,   167,   168,   137,   138,   139,     0,
     140,     0,     0,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,     0,     0,
     156,   157,   158,   159,   160,   161,   162,   163,   164,     0,
       0,   137,   138,   139,   168,   140,   320,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,     0,     0,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   137,   138,   139,     0,   140,     0,
       0,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,     0,     0,   156,   157,
     158,   159,   160,   161,   162,   163,   164
};

static const yytype_int16 yycheck[] =
{
      82,    83,    66,    19,   107,   168,   107,   125,    97,   144,
      98,   299,   113,     3,    83,   108,    45,    46,     0,    48,
     302,   303,   125,    39,    40,    41,    14,    43,     3,    17,
      59,     4,    48,    49,     9,     3,     5,    10,     8,   190,
       9,     4,    51,   331,   179,     8,     3,   140,    57,    58,
      59,    60,    61,   335,    70,    71,   120,   139,     4,   141,
       8,     4,     3,     4,    10,   154,   155,    10,   156,     5,
       4,     5,   141,     9,   138,     9,   140,     3,   171,    51,
     173,   174,   200,    52,   148,   236,   237,     3,     5,   207,
      61,   192,     9,   157,   158,   159,   160,   161,     3,     4,
     164,    61,   265,    61,   207,     4,   175,     5,   124,    44,
      45,    46,     5,     6,     7,    50,   132,    52,     4,    54,
      55,    56,    46,    47,    48,    35,   219,   220,     3,   222,
      72,    73,     9,    10,   227,   228,    44,   166,    10,   168,
      52,    53,    54,     8,     9,    87,    88,    89,     5,    91,
      92,    93,    94,    95,   223,     9,   220,     3,     4,    33,
     224,   225,   255,     3,     4,   229,     3,     4,   232,   233,
       5,   235,     3,     4,    47,    48,   240,   241,     4,   243,
       3,     4,    85,    86,     3,    44,     4,   280,   281,     3,
     283,     4,     4,     3,    10,     4,   289,     4,     4,     4,
     142,   143,    34,   145,   146,   147,     4,   149,   150,   151,
     152,   153,     4,     4,    53,     4,   280,     4,     4,     4,
      17,   285,   286,   165,     4,   307,    10,     4,   321,   293,
     294,    10,    14,   297,     4,   177,   178,    10,   320,    14,
       4,     4,     4,   336,     5,     4,     4,     4,    14,     4,
     343,     4,     3,    48,     8,     4,     4,     3,    14,     4,
      10,   325,     4,   317,   328,    17,   359,   123,   130,    10,
      -1,     3,    -1,    -1,    -1,    -1,   358,     9,    -1,    11,
      12,    13,    -1,    15,   226,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     3,    -1,    -1,    -1,   258,    -1,    -1,    -1,
      11,    12,    13,    -1,    15,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    12,    13,    -1,    15,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     3,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    11,    12,    13,    -1,    15,    -1,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    11,    12,    13,    -1,    15,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    12,    13,    -1,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,     3,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    11,    12,    13,    -1,    15,
      -1,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    11,    12,
      13,    -1,    15,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    -1,    46,    47,    48,    49,    11,    12,    13,
      -1,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    -1,    -1,    47,    48,    49,    11,    12,    13,    -1,
      15,    -1,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      -1,    11,    12,    13,    49,    15,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    11,    12,    13,    -1,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   105,   109,   110,     3,   102,   103,   104,     0,    51,
      57,    58,    59,    60,    61,     8,    64,   101,     3,   102,
       3,     3,     3,     8,    72,     4,     8,     3,     4,    86,
      88,    89,    95,    96,    97,    98,    99,   100,    72,    61,
      61,    61,   108,    44,    45,    46,    50,    52,    54,    55,
      56,     4,    72,    72,    72,     3,     4,   106,    72,    87,
       5,     9,    69,     3,     9,    71,    10,    71,     5,    93,
      71,    72,    72,    70,   108,   108,   108,    35,     3,    11,
      12,    13,    15,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    71,    78,    79,
      81,    82,    83,    84,    85,     4,    44,     3,     3,    78,
      10,     5,    92,    94,    72,    72,    52,    69,     4,    69,
       4,     4,     4,     5,     6,     7,    68,    11,    12,    13,
      15,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    46,    47,    48,    49,    80,
      71,    74,    75,    74,    74,    75,     5,    67,    67,    67,
      69,    69,    69,    69,    69,    69,    69,    69,    33,    76,
      76,    68,    84,    85,    81,     3,    85,     4,     3,    66,
      44,     4,    78,     3,     4,    91,    92,    72,     3,    66,
      84,     4,     4,   106,   107,     4,    72,     4,    78,    74,
      78,    81,    74,    75,    69,    69,    70,    69,    69,    69,
      78,    69,    69,    69,    69,    69,    76,    76,    68,    78,
      78,    78,    78,    78,    78,    69,    65,    71,    10,    65,
      71,     4,    81,    81,    81,    75,    69,    69,    70,    34,
      77,    77,    85,     4,     4,    47,    48,     4,    66,     4,
      53,     4,    66,    84,     4,     4,     4,     4,    81,     3,
      78,    17,    81,    75,    78,    78,    69,    81,    81,    78,
      78,    78,    78,    77,    77,    78,    78,    78,     4,     4,
      10,    10,     4,     4,    10,    14,    14,    17,    14,    81,
      69,     4,    65,    10,     4,     4,     5,    90,     4,     4,
      16,    78,    81,    81,    78,    78,    81,    78,    78,    78,
      82,     4,     3,    83,    83,     4,    74,    14,     4,     4,
       4,    64,    73,    74,    78,    78,    82,    83,    81,     3,
       4,    81,    14,    48,     4,    10,     3,     4,    17,    74,
      81,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    63,    64,    64,    65,    65,    66,    66,    66,    66,
      67,    68,    68,    68,    69,    69,    70,    70,    71,    72,
      73,    74,    74,    75,    76,    76,    77,    77,    78,    78,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    81,    81,    82,
      82,    82,    82,    83,    83,    83,    84,    85,    86,    86,
      86,    86,    87,    87,    88,    89,    89,    90,    91,    92,
      92,    93,    94,    95,    95,    96,    96,    97,    98,    98,
      98,    98,    99,   100,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   102,   102,   103,   104,   104,   104,
     104,   104,   104,   105,   105,   106,   107,   107,   108,   108,
     109,   110
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     2,     0,     4,     8,     4,
       1,     1,     1,     1,     1,     1,     0,     2,     1,     1,
       1,     0,     1,     1,     0,     1,     0,     1,     1,     3,
       1,     1,     1,     4,     4,     5,     3,     3,     4,     1,
       4,     7,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     3,     4,     2,     3,     3,     4,
       4,     5,     1,     2,     3,     4,     7,    12,     4,     4,
       3,     3,     4,     2,     3,     3,     2,     3,     4,     5,
       2,     2,     3,     3,     2,     1,     2,     0,     2,     1,
       5,     5,     6,     1,     5,     6,     4,     1,     6,     7,
       5,     6,     0,     1,     4,     5,     6,     1,     5,     0,
       2,     1,     1,     6,     5,     7,     8,     4,     6,     7,
       6,     7,     5,     5,     0,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     4,     4,     1,     1,     5,     5,
       9,     8,     9,     0,     2,     4,     0,     1,     0,     2,
       1,     1
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
          case 64: /* text_list  */
#line 181 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_text_list(parser->allocator, &((*yyvaluep).text_list)); }
#line 1541 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 65: /* value_type_list  */
#line 184 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_type_vector(parser->allocator, &((*yyvaluep).types)); }
#line 1547 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 66: /* func_type  */
#line 194 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_signature(parser->allocator, &((*yyvaluep).func_sig)); }
#line 1553 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 68: /* literal  */
#line 183 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).literal).text); }
#line 1559 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 69: /* var  */
#line 185 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var(parser->allocator, &((*yyvaluep).var)); }
#line 1565 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 70: /* var_list  */
#line 186 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1571 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 71: /* bind_var  */
#line 182 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1577 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 72: /* quoted_text  */
#line 182 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1583 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 73: /* segment_contents  */
#line 191 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1589 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 74: /* labeling  */
#line 182 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1595 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 75: /* labeling1  */
#line 182 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_string_slice(parser->allocator, &((*yyvaluep).text)); }
#line 1601 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 78: /* expr  */
#line 188 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_list(parser->allocator, ((*yyvaluep).expr_list).first); }
#line 1607 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 79: /* op  */
#line 187 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr(parser->allocator, ((*yyvaluep).expr)); }
#line 1613 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 80: /* expr1  */
#line 188 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_list(parser->allocator, ((*yyvaluep).expr_list).first); }
#line 1619 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 81: /* expr_list  */
#line 188 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_expr_list(parser->allocator, ((*yyvaluep).expr_list).first); }
#line 1625 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 82: /* func_fields  */
#line 189 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_fields(parser->allocator, ((*yyvaluep).func_fields)); }
#line 1631 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 83: /* func_body  */
#line 189 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_fields(parser->allocator, ((*yyvaluep).func_fields)); }
#line 1637 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 85: /* func_info  */
#line 190 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func(parser->allocator, ((*yyvaluep).func)); wasm_free(parser->allocator, ((*yyvaluep).func)); }
#line 1643 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 86: /* func  */
#line 198 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_exported_func(parser->allocator, &((*yyvaluep).exported_func)); }
#line 1649 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 91: /* segment  */
#line 191 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment(parser->allocator, &((*yyvaluep).segment)); }
#line 1655 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 92: /* segment_list  */
#line 192 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_segment_vector_and_elements(parser->allocator, &((*yyvaluep).segments)); }
#line 1661 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 95: /* memory  */
#line 193 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_memory(parser->allocator, &((*yyvaluep).memory)); }
#line 1667 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 96: /* type_def  */
#line 195 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_func_type(parser->allocator, &((*yyvaluep).func_type)); }
#line 1673 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 97: /* table  */
#line 186 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_var_vector_and_elements(parser->allocator, &((*yyvaluep).vars)); }
#line 1679 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 98: /* import  */
#line 196 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_import(parser->allocator, ((*yyvaluep).import)); wasm_free(parser->allocator, ((*yyvaluep).import)); }
#line 1685 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 99: /* export  */
#line 197 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_export(parser->allocator, &((*yyvaluep).export_)); }
#line 1691 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 101: /* module_fields  */
#line 199 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1697 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 102: /* raw_module  */
#line 200 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_raw_module(parser->allocator, &((*yyvaluep).raw_module)); }
#line 1703 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 103: /* module  */
#line 199 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_module(parser->allocator, ((*yyvaluep).module)); wasm_free(parser->allocator, ((*yyvaluep).module)); }
#line 1709 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 104: /* cmd  */
#line 202 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command(parser->allocator, ((*yyvaluep).command)); wasm_free(parser->allocator, ((*yyvaluep).command)); }
#line 1715 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 105: /* cmd_list  */
#line 203 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_command_vector_and_elements(parser->allocator, &((*yyvaluep).commands)); }
#line 1721 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 108: /* const_list  */
#line 201 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_const_vector(parser->allocator, &((*yyvaluep).consts)); }
#line 1727 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
        break;

    case 109: /* script  */
#line 204 "src/wasm-ast-parser.y" /* yacc.c:1257  */
      { wasm_destroy_script(&((*yyvaluep).script)); }
#line 1733 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1257  */
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
#line 214 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 2032 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 3:
#line 220 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 2045 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 4:
#line 233 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.types)); }
#line 2051 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 5:
#line 234 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      wasm_append_type_value(parser->allocator, &(yyval.types), &(yyvsp[0].type));
    }
#line 2060 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 6:
#line 240 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); }
#line 2066 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 7:
#line 241 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = WASM_TYPE_VOID;
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 2075 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 8:
#line 245 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig).result_type = (yyvsp[-1].type);
      (yyval.func_sig).param_types = (yyvsp[-5].types);
    }
#line 2084 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 9:
#line 249 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.func_sig)); (yyval.func_sig).result_type = (yyvsp[-1].type); }
#line 2090 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 10:
#line 256 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser, "invalid int " PRIstringslice,
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2103 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 11:
#line 267 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2112 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 12:
#line 271 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2121 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 13:
#line 275 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2130 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 14:
#line 282 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_INDEX;
      uint64_t index;
      if (WASM_FAILED(wasm_parse_int64((yyvsp[0].literal).text.start,
                                       (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &index,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser, "invalid int " PRIstringslice,
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      (yyval.var).index = index;
    }
#line 2147 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 15:
#line 294 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WASM_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 2157 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 16:
#line 301 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.vars)); }
#line 2163 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 17:
#line 302 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      wasm_append_var_value(parser->allocator, &(yyval.vars), &(yyvsp[0].var));
    }
#line 2172 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 18:
#line 308 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2178 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 19:
#line 312 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 2196 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 20:
#line 328 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      dup_text_list(parser->allocator, &(yyvsp[0].text_list), &(yyval.segment).data, &(yyval.segment).size);
      wasm_destroy_text_list(parser->allocator, &(yyvsp[0].text_list));
    }
#line 2205 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 21:
#line 335 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.text)); }
#line 2211 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 24:
#line 341 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2217 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 25:
#line 342 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
    if (WASM_FAILED(wasm_parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64),
                                     WASM_PARSE_SIGNED_AND_UNSIGNED))) {
      wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                            "invalid offset \"" PRIstringslice "\"",
                            WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2230 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 26:
#line 352 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2236 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 27:
#line 353 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid alignment \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2249 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 28:
#line 364 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2255 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 29:
#line 365 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 2261 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 30:
#line 368 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_unreachable_expr(parser->allocator);
    }
#line 2269 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 31:
#line 371 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_nop_expr(parser->allocator);
    }
#line 2277 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 32:
#line 374 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_drop_expr(parser->allocator);
    }
#line 2285 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 33:
#line 377 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_block_expr(parser->allocator);
      (yyval.expr)->block.label = (yyvsp[-2].text);
      (yyval.expr)->block.first = (yyvsp[-1].expr_list).first;
    }
#line 2295 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 34:
#line 382 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_loop_expr(parser->allocator);
      (yyval.expr)->loop.label = (yyvsp[-2].text);
      (yyval.expr)->loop.first = (yyvsp[-1].expr_list).first;
    }
#line 2305 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 35:
#line 387 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_block_expr(parser->allocator);
      (yyval.expr)->block.label = (yyvsp[-3].text);
      WasmExpr* loop = wasm_new_loop_expr(parser->allocator);
      loop->loc = (yylsp[-4]);
      loop->loop.label = (yyvsp[-2].text);
      loop->loop.first = (yyvsp[-1].expr_list).first;
      (yyval.expr)->block.first = loop;
    }
#line 2319 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 36:
#line 396 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_expr(parser->allocator);
      (yyval.expr)->br.arity = (yyvsp[-1].u32);
      (yyval.expr)->br.var = (yyvsp[0].var);
    }
#line 2329 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 37:
#line 401 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_if_expr(parser->allocator);
      (yyval.expr)->br_if.arity = (yyvsp[-1].u32);
      (yyval.expr)->br_if.var = (yyvsp[0].var);
    }
#line 2339 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 38:
#line 406 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_br_table_expr(parser->allocator);
      (yyval.expr)->br_table.arity = (yyvsp[-2].u32);
      (yyval.expr)->br_table.targets = (yyvsp[-1].vars);
      (yyval.expr)->br_table.default_target = (yyvsp[0].var);
    }
#line 2350 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 39:
#line 412 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_return_expr(parser->allocator);
    }
#line 2358 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 40:
#line 415 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_expr(parser->allocator);
      (yyval.expr)->if_.true_.label = (yyvsp[-2].text);
      (yyval.expr)->if_.true_.first = (yyvsp[-1].expr_list).first;
    }
#line 2368 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 41:
#line 420 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_if_else_expr(parser->allocator);
      (yyval.expr)->if_else.true_.label = (yyvsp[-5].text);
      (yyval.expr)->if_else.true_.first = (yyvsp[-4].expr_list).first;
      (yyval.expr)->if_else.false_.label = (yyvsp[-2].text);
      (yyval.expr)->if_else.false_.first = (yyvsp[-1].expr_list).first;
    }
#line 2380 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 42:
#line 427 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_select_expr(parser->allocator);
    }
#line 2388 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 43:
#line 430 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_expr(parser->allocator);
      (yyval.expr)->call.var = (yyvsp[0].var);
    }
#line 2397 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 44:
#line 434 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_import_expr(parser->allocator);
      (yyval.expr)->call_import.var = (yyvsp[0].var);
    }
#line 2406 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 45:
#line 438 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_call_indirect_expr(parser->allocator);
      (yyval.expr)->call_indirect.var = (yyvsp[0].var);
    }
#line 2415 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 46:
#line 442 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_get_local_expr(parser->allocator);
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2424 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 47:
#line 446 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_set_local_expr(parser->allocator);
      (yyval.expr)->set_local.var = (yyvsp[0].var);
    }
#line 2433 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 48:
#line 450 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_tee_local_expr(parser->allocator);
      (yyval.expr)->tee_local.var = (yyvsp[0].var);
    }
#line 2442 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 49:
#line 454 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_get_global_expr(parser->allocator);
      (yyval.expr)->get_global.var = (yyvsp[0].var);
    }
#line 2451 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 50:
#line 458 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_set_global_expr(parser->allocator);
      (yyval.expr)->set_global.var = (yyvsp[0].var);
    }
#line 2460 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 51:
#line 462 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_load_expr(parser->allocator);
      (yyval.expr)->load.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->load.offset = (yyvsp[-1].u64);
      (yyval.expr)->load.align = (yyvsp[0].u32);
    }
#line 2471 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 52:
#line 468 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_store_expr(parser->allocator);
      (yyval.expr)->store.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->store.offset = (yyvsp[-1].u64);
      (yyval.expr)->store.align = (yyvsp[0].u32);
    }
#line 2482 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 53:
#line 474 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 2499 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 54:
#line 486 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_unary_expr(parser->allocator);
      (yyval.expr)->unary.opcode = (yyvsp[0].opcode);
    }
#line 2508 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 55:
#line 490 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_binary_expr(parser->allocator);
      (yyval.expr)->binary.opcode = (yyvsp[0].opcode);
    }
#line 2517 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 56:
#line 494 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_compare_expr(parser->allocator);
      (yyval.expr)->compare.opcode = (yyvsp[0].opcode);
    }
#line 2526 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 57:
#line 498 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_convert_expr(parser->allocator);
      (yyval.expr)->convert.opcode = (yyvsp[0].opcode);
    }
#line 2535 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 58:
#line 502 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_current_memory_expr(parser->allocator);
    }
#line 2543 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 59:
#line 505 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wasm_new_grow_memory_expr(parser->allocator);
    }
#line 2551 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 60:
#line 510 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs1(&(yylsp[0]), wasm_new_unreachable_expr(parser->allocator));
    }
#line 2559 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 61:
#line 513 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs1(&(yylsp[0]), wasm_new_nop_expr(parser->allocator));
    }
#line 2567 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 62:
#line 516 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), wasm_new_drop_expr(parser->allocator));
    }
#line 2575 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 63:
#line 519 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_block_expr(parser->allocator);
      expr->block.label = (yyvsp[-1].text);
      expr->block.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2586 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 64:
#line 525 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_loop_expr(parser->allocator);
      expr->loop.label = (yyvsp[-1].text);
      expr->loop.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2597 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 65:
#line 531 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* block = wasm_new_block_expr(parser->allocator);
      block->block.label = (yyvsp[-2].text);
      WasmExpr* loop = wasm_new_loop_expr(parser->allocator);
      loop->loc = (yylsp[-3]);
      loop->loop.label = (yyvsp[-1].text);
      loop->loop.first = (yyvsp[0].expr_list).first;
      block->block.first = loop;
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), block);
    }
#line 2612 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 66:
#line 541 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_expr(parser->allocator);
      expr->br.arity = 0;
      expr->br.var = (yyvsp[0].var);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 2623 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 67:
#line 547 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_expr(parser->allocator);
      expr->br.arity = 1;
      expr->br.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2634 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 68:
#line 553 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_if_expr(parser->allocator);
      expr->br_if.arity = 0;
      expr->br_if.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2645 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 69:
#line 559 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_if_expr(parser->allocator);
      expr->br_if.arity = 1;
      expr->br_if.var = (yyvsp[-2].var);
      (yyval.expr_list) = join_exprs3(&(yylsp[-3]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2656 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 70:
#line 565 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_table_expr(parser->allocator);
      expr->br_table.arity = 0;
      expr->br_table.targets = (yyvsp[-2].vars);
      expr->br_table.default_target = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-3]), &(yyvsp[0].expr_list), expr);
    }
#line 2668 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 71:
#line 572 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_br_table_expr(parser->allocator);
      expr->br_table.arity = 1;
      expr->br_table.targets = (yyvsp[-3].vars);
      expr->br_table.default_target = (yyvsp[-2].var);
      (yyval.expr_list) = join_exprs3(&(yylsp[-4]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2680 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 72:
#line 579 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_return_expr(parser->allocator);
      (yyval.expr_list) = join_exprs1(&(yylsp[0]), expr);
    }
#line 2689 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 73:
#line 583 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_return_expr(parser->allocator);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), expr);
    }
#line 2698 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 74:
#line 587 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-1].expr_list), expr);
    }
#line 2708 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 75:
#line 592 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.first = (yyvsp[-1].expr_list).first;
      expr->if_else.false_.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-3]), &(yyvsp[-2].expr_list), expr);
    }
#line 2719 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 76:
#line 598 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.label = (yyvsp[-2].text);
      expr->if_.true_.first = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-6]), &(yyvsp[-5].expr_list), expr);
    }
#line 2730 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 77:
#line 604 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.label = (yyvsp[-7].text);
      expr->if_else.true_.first = (yyvsp[-6].expr_list).first;
      expr->if_else.false_.label = (yyvsp[-2].text);
      expr->if_else.false_.first = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-11]), &(yyvsp[-10].expr_list), expr);
    }
#line 2743 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 78:
#line 612 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.first = (yyvsp[-2].expr_list).first;
      expr->if_else.false_.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 2754 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 79:
#line 618 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs4(&(yylsp[-3]), &(yyvsp[-2].expr_list), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list),
                       wasm_new_select_expr(parser->allocator));
    }
#line 2763 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 80:
#line 622 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_call_expr(parser->allocator);
      expr->call.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2773 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 81:
#line 627 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_call_import_expr(parser->allocator);
      expr->call_import.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2783 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 82:
#line 632 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_call_indirect_expr(parser->allocator);
      expr->call_indirect.var = (yyvsp[-2].var);
      (yyval.expr_list) = join_exprs3(&(yylsp[-3]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2793 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 83:
#line 637 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_get_local_expr(parser->allocator);
      expr->get_local.var = (yyvsp[0].var);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 2803 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 84:
#line 642 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_set_local_expr(parser->allocator);
      expr->set_local.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2813 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 85:
#line 647 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_tee_local_expr(parser->allocator);
      expr->tee_local.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2823 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 86:
#line 652 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_get_global_expr(parser->allocator);
      expr->get_global.var = (yyvsp[0].var);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 2833 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 87:
#line 657 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_set_global_expr(parser->allocator);
      expr->set_global.var = (yyvsp[-1].var);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[0].expr_list), expr);
    }
#line 2843 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 88:
#line 662 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_load_expr(parser->allocator);
      expr->load.opcode = (yyvsp[-3].opcode);
      expr->load.offset = (yyvsp[-2].u64);
      expr->load.align = (yyvsp[-1].u32);
      (yyval.expr_list) = join_exprs2(&(yylsp[-3]), &(yyvsp[0].expr_list), expr);
    }
#line 2855 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 89:
#line 669 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_store_expr(parser->allocator);
      expr->store.opcode = (yyvsp[-4].opcode);
      expr->store.offset = (yyvsp[-3].u64);
      expr->store.align = (yyvsp[-2].u32);
      (yyval.expr_list) = join_exprs3(&(yylsp[-4]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2867 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 90:
#line 676 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->const_.loc = (yylsp[-1]);
      if (WASM_FAILED(parse_const((yyvsp[-1].type), (yyvsp[0].literal).type, (yyvsp[0].literal).text.start,
                                  (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length,
                                  &expr->const_))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      wasm_free(parser->allocator, (char*)(yyvsp[0].literal).text.start);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 2885 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 91:
#line 689 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_unary_expr(parser->allocator);
      expr->unary.opcode = (yyvsp[-1].opcode);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), expr);
    }
#line 2895 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 92:
#line 694 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_binary_expr(parser->allocator);
      expr->binary.opcode = (yyvsp[-2].opcode);
      (yyval.expr_list) = join_exprs3(&(yylsp[-2]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2905 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 93:
#line 699 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_compare_expr(parser->allocator);
      expr->compare.opcode = (yyvsp[-2].opcode);
      (yyval.expr_list) = join_exprs3(&(yylsp[-2]), &(yyvsp[-1].expr_list), &(yyvsp[0].expr_list), expr);
    }
#line 2915 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 94:
#line 704 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_convert_expr(parser->allocator);
      expr->convert.opcode = (yyvsp[-1].opcode);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), expr);
    }
#line 2925 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 95:
#line 709 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_current_memory_expr(parser->allocator);
      (yyval.expr_list) = join_exprs1(&(yylsp[0]), expr);
    }
#line 2934 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 96:
#line 713 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WasmExpr* expr = wasm_new_grow_memory_expr(parser->allocator);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), expr);
    }
#line 2943 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 97:
#line 719 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.expr_list)); }
#line 2949 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 98:
#line 720 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 2960 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 100:
#line 731 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_RESULT_TYPE;
      (yyval.func_fields)->result_type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2971 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 101:
#line 737 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_PARAM_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2982 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 102:
#line 743 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_PARAM;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 2995 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 103:
#line 753 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_EXPRS;
      (yyval.func_fields)->first_expr = (yyvsp[0].expr_list).first;
      (yyval.func_fields)->next = NULL;
    }
#line 3006 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 104:
#line 759 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_LOCAL_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3017 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 105:
#line 765 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field(parser->allocator);
      (yyval.func_fields)->type = WASM_FUNC_FIELD_TYPE_BOUND_LOCAL;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3030 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 106:
#line 775 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 3036 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 107:
#line 778 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func(parser->allocator);
      WasmFuncField* field = (yyvsp[0].func_fields);

      while (field) {
        WasmFuncField* next = field->next;
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
#line 3094 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 108:
#line 833 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-4]);
      (yyval.exported_func).func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.exported_func).func->decl.type_var = (yyvsp[-2].var);
      (yyval.exported_func).export_.name = (yyvsp[-3].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 3108 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 109:
#line 842 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3123 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 110:
#line 852 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-3]);
      (yyval.exported_func).export_.name = (yyvsp[-2].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 3135 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 111:
#line 859 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->loc = (yylsp[-4]);
      (yyval.exported_func).func->name = (yyvsp[-2].text);
      (yyval.exported_func).export_.name = (yyvsp[-3].text);
      (yyval.exported_func).export_.var.type = WASM_VAR_TYPE_INDEX;
      (yyval.exported_func).export_.var.index = -1;
    }
#line 3148 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 112:
#line 870 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.text)); }
#line 3154 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 114:
#line 878 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 3160 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 115:
#line 882 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global).loc = (yylsp[-3]);
      (yyval.global).type = (yyvsp[-2].type);
      (yyval.global).init_expr = (yyvsp[-1].expr_list).first;
    }
#line 3170 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 116:
#line 887 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global).loc = (yylsp[-4]);
      (yyval.global).name = (yyvsp[-3].text);
      (yyval.global).type = (yyvsp[-2].type);
      (yyval.global).init_expr = (yyvsp[-1].expr_list).first;
    }
#line 3181 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 117:
#line 896 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3196 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 118:
#line 909 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.segment).loc = (yylsp[-3]);
      (yyval.segment).data = (yyvsp[-1].segment).data;
      (yyval.segment).size = (yyvsp[-1].segment).size;
      (yyval.segment).addr = (yyvsp[-2].u32);
    }
#line 3207 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 119:
#line 917 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.segments)); }
#line 3213 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 120:
#line 918 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.segments) = (yyvsp[-1].segments);
      wasm_append_segment_value(parser->allocator, &(yyval.segments), &(yyvsp[0].segment));
    }
#line 3222 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 121:
#line 925 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid initial memory pages \"" PRIstringslice
                              "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 3236 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 122:
#line 937 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      if (WASM_FAILED(wasm_parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wasm_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid max memory pages \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 3249 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 123:
#line 948 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-4]);
      (yyval.memory).initial_pages = (yyvsp[-3].u64);
      (yyval.memory).max_pages = (yyvsp[-2].u64);
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3260 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 124:
#line 954 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory).loc = (yylsp[-3]);
      (yyval.memory).initial_pages = (yyvsp[-2].u64);
      (yyval.memory).max_pages = (yyval.memory).initial_pages;
      (yyval.memory).segments = (yyvsp[-1].segments);
    }
#line 3271 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 125:
#line 963 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      WASM_ZERO_MEMORY((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3280 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 126:
#line 967 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-5].text);
      (yyval.func_type).sig = (yyvsp[-2].func_sig);
    }
#line 3289 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 127:
#line 974 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = (yyvsp[-1].vars); }
#line 3295 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 128:
#line 978 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 3307 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 129:
#line 985 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->decl.type_var = (yyvsp[-1].var);
    }
#line 3320 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 130:
#line 993 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 3331 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 131:
#line 999 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import(parser->allocator);
      (yyval.import)->name = (yyvsp[-4].text);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->func_name = (yyvsp[-2].text);
      (yyval.import)->decl.sig = (yyvsp[-1].func_sig);
    }
#line 3343 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1009 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_).name = (yyvsp[-2].text);
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3352 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1016 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_memory).name = (yyvsp[-2].text);
    }
#line 3360 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1022 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new_module(parser->allocator);
    }
#line 3368 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1025 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *(yyvsp[0].exported_func).func;
      wasm_free(parser->allocator, (yyvsp[0].exported_func).func);

      append_implicit_func_declaration(parser->allocator, &(yylsp[0]), (yyval.module),
                                       &field->func.decl);

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
#line 3412 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1064 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;
      field->global = (yyvsp[0].global);

      WasmGlobal* global_ptr = &field->global;
      wasm_append_global_ptr_value(parser->allocator, &(yyval.module)->globals,
                                   &global_ptr);
      if (field->global.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &(yyval.module)->global_bindings, &field->global.name);
        binding->loc = field->loc;
        binding->index = (yyval.module)->globals.size - 1;
      }
    }
#line 3434 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1081 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *(yyvsp[0].import);
      wasm_free(parser->allocator, (yyvsp[0].import));

      append_implicit_func_declaration(parser->allocator, &(yylsp[0]), (yyval.module),
                                       &field->import.decl);

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
#line 3460 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1102 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3482 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1119 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = (yyvsp[0].export_memory);
      (yyval.module)->export_memory = &field->export_memory;
    }
#line 3495 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1127 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = (yyvsp[0].vars);
      (yyval.module)->table = &field->table;
    }
#line 3508 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1135 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3531 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1153 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = (yyvsp[0].memory);
      (yyval.module)->memory = &field->memory;
    }
#line 3544 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1161 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WasmModuleField* field = wasm_append_module_field(parser->allocator, (yyval.module));
      field->loc = (yylsp[0]);
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = (yyvsp[0].var);
      (yyval.module)->start = &field->start;
    }
#line 3557 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1172 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WASM_RAW_MODULE_TYPE_TEXT;
      (yyval.raw_module).text = (yyvsp[-1].module);
      (yyval.raw_module).loc = (yylsp[-2]);

      /* resolve func type variables where the signature was not specified
       * explicitly */
      size_t i;
      for (i = 0; i < (yyvsp[-1].module)->funcs.size; ++i) {
        WasmFunc* func = (yyvsp[-1].module)->funcs.data[i];
        if (wasm_decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          WasmFuncType* func_type =
              wasm_get_func_type_by_var((yyvsp[-1].module), &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
            func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }

      for (i = 0; i < (yyvsp[-1].module)->imports.size; ++i) {
        WasmImport* import = (yyvsp[-1].module)->imports.data[i];
        if (wasm_decl_has_func_type(&import->decl) &&
            is_empty_signature(&import->decl.sig)) {
          WasmFuncType* func_type =
              wasm_get_func_type_by_var((yyvsp[-1].module), &import->decl.type_var);
          if (func_type) {
            import->decl.sig = func_type->sig;
            import->decl.flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }
    }
#line 3596 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1206 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WASM_RAW_MODULE_TYPE_BINARY;
      (yyval.raw_module).loc = (yylsp[-2]);
      dup_text_list(parser->allocator, &(yyvsp[-1].text_list), &(yyval.raw_module).binary.data, &(yyval.raw_module).binary.size);
      wasm_destroy_text_list(parser->allocator, &(yyvsp[-1].text_list));
    }
#line 3607 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1215 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3631 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1239 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_MODULE;
      (yyval.command)->module = *(yyvsp[0].module);
      wasm_free(parser->allocator, (yyvsp[0].module));
    }
#line 3642 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1245 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_INVOKE;
      (yyval.command)->invoke.loc = (yylsp[-3]);
      (yyval.command)->invoke.name = (yyvsp[-2].text);
      (yyval.command)->invoke.args = (yyvsp[-1].consts);
    }
#line 3654 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1252 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 3665 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1258 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command)->assert_return.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_return.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_return.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_return.expected = (yyvsp[-1].const_);
    }
#line 3678 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1266 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command)->assert_return_nan.invoke.loc = (yylsp[-4]);
      (yyval.command)->assert_return_nan.invoke.name = (yyvsp[-3].text);
      (yyval.command)->assert_return_nan.invoke.args = (yyvsp[-2].consts);
    }
#line 3690 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1273 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command(parser->allocator);
      (yyval.command)->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command)->assert_trap.invoke.loc = (yylsp[-5]);
      (yyval.command)->assert_trap.invoke.name = (yyvsp[-4].text);
      (yyval.command)->assert_trap.invoke.args = (yyvsp[-3].consts);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3703 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1283 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.commands)); }
#line 3709 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1284 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      wasm_append_command_value(parser->allocator, &(yyval.commands), (yyvsp[0].command));
      wasm_free(parser->allocator, (yyvsp[0].command));
    }
#line 3719 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1292 "src/wasm-ast-parser.y" /* yacc.c:1646  */
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
#line 3734 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1304 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { (yyval.const_).type = WASM_TYPE_VOID; }
#line 3740 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1308 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    { WASM_ZERO_MEMORY((yyval.consts)); }
#line 3746 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1309 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      wasm_append_const_value(parser->allocator, &(yyval.consts), &(yyvsp[0].const_));
    }
#line 3755 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1316 "src/wasm-ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script).commands = (yyvsp[0].commands);
      parser->script = (yyval.script);
    }
#line 3764 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
    break;


#line 3768 "src/prebuilt/wasm-ast-parser-gen.c" /* yacc.c:1646  */
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
#line 1328 "src/wasm-ast-parser.y" /* yacc.c:1906  */


static void append_expr_list(WasmExprList* expr_list, WasmExprList* expr) {
  if (!expr->first)
    return;
  if (expr_list->last)
    expr_list->last->next = expr->first;
  else
    expr_list->first = expr->first;
  expr_list->last = expr->last;
  expr_list->size += expr->size;
}

static void append_expr(WasmExprList* expr_list, WasmExpr* expr) {
  if (expr_list->last)
    expr_list->last->next = expr;
  else
    expr_list->first = expr;
  expr_list->last = expr;
  expr_list->size++;
}

static WasmExprList join_exprs1(WasmLocation* loc, WasmExpr* expr1) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr(&result, expr1);
  expr1->loc = *loc;
  return result;
}

static WasmExprList join_exprs2(WasmLocation* loc, WasmExprList* expr1,
                                WasmExpr* expr2) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr(&result, expr2);
  expr2->loc = *loc;
  return result;
}

static WasmExprList join_exprs3(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExpr* expr3) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  append_expr(&result, expr3);
  expr3->loc = *loc;
  return result;
}

static WasmExprList join_exprs4(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExprList* expr3,
                                WasmExpr* expr4) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  append_expr_list(&result, expr3);
  append_expr(&result, expr4);
  expr4->loc = *loc;
  return result;
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
      return wasm_parse_int64(s, end, &out->u64,
                              WASM_PARSE_SIGNED_AND_UNSIGNED);
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
    size_t size = (end > src) ? (end - src) : 0;
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

static WasmBool is_empty_signature(WasmFuncSignature* sig) {
  return sig->result_type == WASM_TYPE_VOID && sig->param_types.size == 0;
}

static void append_implicit_func_declaration(WasmAllocator* allocator,
                                             WasmLocation* loc,
                                             WasmModule* module,
                                             WasmFuncDeclaration* decl) {
  if (wasm_decl_has_func_type(decl))
    return;

  int sig_index = wasm_get_func_type_index_by_decl(module, decl);
  if (sig_index == -1) {
    wasm_append_implicit_func_type(allocator, loc, module, &decl->sig);
  } else {
    /* signature already exists, share that one and destroy this one */
    wasm_destroy_func_signature(allocator, &decl->sig);
    WasmFuncSignature* sig = &module->func_types.data[sig_index]->sig;
    decl->sig = *sig;
  }

  decl->flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
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

/* see comment above definition of YYMAXDEPTH at the top of this file */
WASM_STATIC_ASSERT(YYSTACK_ALLOC_MAXIMUM >= UINT32_MAX);
WASM_STATIC_ASSERT(YYSTACK_BYTES((uint64_t)YYMAXDEPTH) <= UINT32_MAX);
