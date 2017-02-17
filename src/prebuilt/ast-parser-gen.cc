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
#define YYSTYPE         WABT_AST_PARSER_STYPE
#define YYLTYPE         WABT_AST_PARSER_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         wabt_ast_parser_parse
#define yylex           wabt_ast_parser_lex
#define yyerror         wabt_ast_parser_error
#define yydebug         wabt_ast_parser_debug
#define yynerrs         wabt_ast_parser_nerrs


/* Copy the first part of user declarations.  */
#line 17 "src/ast-parser.y" /* yacc.c:339  */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast-parser.h"
#include "ast-parser-lexer-shared.h"
#include "binary-reader-ast.h"
#include "binary-reader.h"
#include "literal.h"

#define INVALID_VAR_INDEX (-1)

#define RELOCATE_STACK(type, array, stack_base, old_byte_size, new_size)      \
  do {                                                                        \
    if ((stack_base) == (array)) {                                            \
      (stack_base) = (type*)wabt_alloc((new_size) * sizeof(*(stack_base)));   \
      memcpy((stack_base), (array), old_byte_size);                           \
    } else {                                                                  \
      (stack_base) = (type*)wabt_realloc((stack_base),                        \
                                         (new_size) * sizeof(*(stack_base))); \
    }                                                                         \
    /* Cache the pointer in the parser struct to be free'd later. */          \
    parser->array = (stack_base);                                             \
  } while (0)

#define yyoverflow(message, ss, ss_size, vs, vs_size, ls, ls_size, new_size) \
  do {                                                                       \
    *(new_size) *= 2;                                                        \
    RELOCATE_STACK(yytype_int16, yyssa, *(ss), (ss_size), *(new_size));      \
    RELOCATE_STACK(YYSTYPE, yyvsa, *(vs), (vs_size), *(new_size));           \
    RELOCATE_STACK(YYLTYPE, yylsa, *(ls), (ls_size), *(new_size));           \
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
      (Current).last_column = YYRHSLOC(Rhs, N).last_column;   \
    } else {                                                  \
      (Current).filename = NULL;                              \
      (Current).line = YYRHSLOC(Rhs, 0).line;                 \
      (Current).first_column = (Current).last_column =        \
          YYRHSLOC(Rhs, 0).last_column;                       \
    }                                                         \
  while (0)

#define APPEND_FIELD_TO_LIST(module, field, KIND, kind, loc_, item) \
  do {                                                              \
    field = wabt_append_module_field(module);                       \
    field->loc = loc_;                                              \
    field->type = WABT_MODULE_FIELD_TYPE_##KIND;                    \
    field->kind = item;                                             \
  } while (0)

#define APPEND_ITEM_TO_VECTOR(module, Kind, kind, kinds, item_ptr) \
  do {                                                             \
    Wabt##Kind* dummy = item_ptr;                                  \
    wabt_append_##kind##_ptr_value(&(module)->kinds, &dummy);      \
  } while (0)

#define INSERT_BINDING(module, kind, kinds, loc_, name)             \
  do                                                                \
    if ((name).start) {                                             \
      WabtBinding* binding =                                        \
          wabt_insert_binding(&(module)->kind##_bindings, &(name)); \
      binding->loc = loc_;                                          \
      binding->index = (module)->kinds.size - 1;                    \
    }                                                               \
  while (0)

#define APPEND_INLINE_EXPORT(module, KIND, loc_, value, index_)         \
  do                                                                    \
    if ((value).export_.has_export) {                                   \
      WabtModuleField* export_field;                                    \
      APPEND_FIELD_TO_LIST(module, export_field, EXPORT, export_, loc_, \
                           (value).export_.export_);                    \
      export_field->export_.kind = WABT_EXTERNAL_KIND_##KIND;           \
      export_field->export_.var.loc = loc_;                             \
      export_field->export_.var.index = index_;                         \
      APPEND_ITEM_TO_VECTOR(module, Export, export, exports,            \
                            &export_field->export_);                    \
      INSERT_BINDING(module, export, exports, export_field->loc,        \
                     export_field->export_.name);                       \
    }                                                                   \
  while (0)

#define CHECK_IMPORT_ORDERING(module, kind, kinds, loc_)           \
  do {                                                             \
    if ((module)->kinds.size != (module)->num_##kind##_imports) {  \
      wabt_ast_parser_error(                                       \
          &loc_, lexer, parser,                                    \
          "imports must occur before all non-import definitions"); \
    }                                                              \
  } while (0)

#define CHECK_END_LABEL(loc, begin_label, end_label)                     \
  do {                                                                   \
    if (!wabt_string_slice_is_empty(&(end_label))) {                     \
      if (wabt_string_slice_is_empty(&(begin_label))) {                  \
        wabt_ast_parser_error(&loc, lexer, parser,                       \
                              "unexpected label \"" PRIstringslice "\"", \
                              WABT_PRINTF_STRING_SLICE_ARG(end_label));  \
      } else if (!wabt_string_slices_are_equal(&(begin_label),           \
                                               &(end_label))) {          \
        wabt_ast_parser_error(&loc, lexer, parser,                       \
                              "mismatching label \"" PRIstringslice      \
                              "\" != \"" PRIstringslice "\"",            \
                              WABT_PRINTF_STRING_SLICE_ARG(begin_label), \
                              WABT_PRINTF_STRING_SLICE_ARG(end_label));  \
      }                                                                  \
      wabt_destroy_string_slice(&(end_label));                           \
    }                                                                    \
  } while (0)

#define YYMALLOC(size) wabt_alloc(size)
#define YYFREE(p) wabt_free(p)

#define USE_NATURAL_ALIGNMENT (~0)

static WabtExprList join_exprs1(WabtLocation* loc, WabtExpr* expr1);
static WabtExprList join_exprs2(WabtLocation* loc, WabtExprList* expr1,
                                WabtExpr* expr2);

static WabtFuncField* new_func_field(void) {
  return (WabtFuncField*)wabt_alloc_zero(sizeof(WabtFuncField));
}

static WabtFunc* new_func(void) {
  return (WabtFunc*)wabt_alloc_zero(sizeof(WabtFunc));
}

static WabtCommand* new_command(void) {
  return (WabtCommand*)wabt_alloc_zero(sizeof(WabtCommand));
}

static WabtModule* new_module(void) {
  return (WabtModule*)wabt_alloc_zero(sizeof(WabtModule));
}

static WabtImport* new_import(void) {
  return (WabtImport*)wabt_alloc_zero(sizeof(WabtImport));
}

static WabtTextListNode* new_text_list_node(void) {
  return (WabtTextListNode*)wabt_alloc_zero(sizeof(WabtTextListNode));
}

static WabtResult parse_const(WabtType type, WabtLiteralType literal_type,
                              const char* s, const char* end, WabtConst* out);
static void dup_text_list(WabtTextList * text_list, void** out_data,
                          size_t* out_size);

static bool is_empty_signature(WabtFuncSignature* sig);

static void append_implicit_func_declaration(WabtLocation*, WabtModule*,
                                             WabtFuncDeclaration*);

typedef struct BinaryErrorCallbackData {
  WabtLocation* loc;
  WabtAstLexer* lexer;
  WabtAstParser* parser;
} BinaryErrorCallbackData;

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data);

#define wabt_ast_parser_lex wabt_ast_lexer_lex


#line 252 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:339  */

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
   by #include "ast-parser-gen.h".  */
#ifndef YY_WABT_AST_PARSER_SRC_PREBUILT_AST_PARSER_GEN_H_INCLUDED
# define YY_WABT_AST_PARSER_SRC_PREBUILT_AST_PARSER_GEN_H_INCLUDED
/* Debug traces.  */
#ifndef WABT_AST_PARSER_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define WABT_AST_PARSER_DEBUG 1
#  else
#   define WABT_AST_PARSER_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define WABT_AST_PARSER_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined WABT_AST_PARSER_DEBUG */
#if WABT_AST_PARSER_DEBUG
extern int wabt_ast_parser_debug;
#endif

/* Token type.  */
#ifndef WABT_AST_PARSER_TOKENTYPE
# define WABT_AST_PARSER_TOKENTYPE
  enum wabt_ast_parser_tokentype
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
    WABT_TOKEN_TYPE_CALL = 279,
    WABT_TOKEN_TYPE_CALL_IMPORT = 280,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 281,
    WABT_TOKEN_TYPE_RETURN = 282,
    WABT_TOKEN_TYPE_GET_LOCAL = 283,
    WABT_TOKEN_TYPE_SET_LOCAL = 284,
    WABT_TOKEN_TYPE_TEE_LOCAL = 285,
    WABT_TOKEN_TYPE_GET_GLOBAL = 286,
    WABT_TOKEN_TYPE_SET_GLOBAL = 287,
    WABT_TOKEN_TYPE_LOAD = 288,
    WABT_TOKEN_TYPE_STORE = 289,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 290,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 291,
    WABT_TOKEN_TYPE_CONST = 292,
    WABT_TOKEN_TYPE_UNARY = 293,
    WABT_TOKEN_TYPE_BINARY = 294,
    WABT_TOKEN_TYPE_COMPARE = 295,
    WABT_TOKEN_TYPE_CONVERT = 296,
    WABT_TOKEN_TYPE_SELECT = 297,
    WABT_TOKEN_TYPE_UNREACHABLE = 298,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 299,
    WABT_TOKEN_TYPE_GROW_MEMORY = 300,
    WABT_TOKEN_TYPE_FUNC = 301,
    WABT_TOKEN_TYPE_START = 302,
    WABT_TOKEN_TYPE_TYPE = 303,
    WABT_TOKEN_TYPE_PARAM = 304,
    WABT_TOKEN_TYPE_RESULT = 305,
    WABT_TOKEN_TYPE_LOCAL = 306,
    WABT_TOKEN_TYPE_GLOBAL = 307,
    WABT_TOKEN_TYPE_MODULE = 308,
    WABT_TOKEN_TYPE_TABLE = 309,
    WABT_TOKEN_TYPE_ELEM = 310,
    WABT_TOKEN_TYPE_MEMORY = 311,
    WABT_TOKEN_TYPE_DATA = 312,
    WABT_TOKEN_TYPE_OFFSET = 313,
    WABT_TOKEN_TYPE_IMPORT = 314,
    WABT_TOKEN_TYPE_EXPORT = 315,
    WABT_TOKEN_TYPE_REGISTER = 316,
    WABT_TOKEN_TYPE_INVOKE = 317,
    WABT_TOKEN_TYPE_GET = 318,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 319,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 320,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 321,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 322,
    WABT_TOKEN_TYPE_ASSERT_RETURN_NAN = 323,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 324,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 325,
    WABT_TOKEN_TYPE_INPUT = 326,
    WABT_TOKEN_TYPE_OUTPUT = 327,
    WABT_TOKEN_TYPE_LOW = 328
  };
#endif

/* Value type.  */
#if ! defined WABT_AST_PARSER_STYPE && ! defined WABT_AST_PARSER_STYPE_IS_DECLARED
typedef WabtToken WABT_AST_PARSER_STYPE;
# define WABT_AST_PARSER_STYPE_IS_TRIVIAL 1
# define WABT_AST_PARSER_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined WABT_AST_PARSER_LTYPE && ! defined WABT_AST_PARSER_LTYPE_IS_DECLARED
typedef struct WABT_AST_PARSER_LTYPE WABT_AST_PARSER_LTYPE;
struct WABT_AST_PARSER_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define WABT_AST_PARSER_LTYPE_IS_DECLARED 1
# define WABT_AST_PARSER_LTYPE_IS_TRIVIAL 1
#endif



int wabt_ast_parser_parse (WabtAstLexer* lexer, WabtAstParser* parser);

#endif /* !YY_WABT_AST_PARSER_SRC_PREBUILT_AST_PARSER_GEN_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 399 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:358  */

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
         || (defined WABT_AST_PARSER_LTYPE_IS_TRIVIAL && WABT_AST_PARSER_LTYPE_IS_TRIVIAL \
             && defined WABT_AST_PARSER_STYPE_IS_TRIVIAL && WABT_AST_PARSER_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   794

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  63
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  402

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   328

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
      65,    66,    67,    68,    69,    70,    71,    72,    73
};

#if WABT_AST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   305,   305,   311,   321,   322,   326,   344,   345,   351,
     354,   359,   366,   369,   370,   374,   379,   386,   389,   392,
     397,   404,   410,   421,   425,   429,   436,   441,   448,   449,
     455,   456,   459,   463,   464,   468,   469,   479,   480,   491,
     492,   493,   496,   499,   502,   505,   508,   512,   516,   521,
     524,   528,   532,   536,   540,   544,   548,   552,   558,   564,
     576,   580,   584,   588,   592,   595,   600,   606,   612,   618,
     628,   636,   640,   643,   649,   655,   664,   670,   675,   681,
     686,   692,   700,   701,   709,   710,   718,   723,   724,   730,
     736,   746,   752,   758,   768,   822,   831,   838,   845,   855,
     858,   862,   868,   879,   885,   905,   912,   924,   931,   951,
     973,   980,   993,  1000,  1006,  1012,  1018,  1026,  1031,  1038,
    1044,  1050,  1056,  1065,  1073,  1078,  1083,  1088,  1095,  1102,
    1106,  1109,  1120,  1124,  1131,  1135,  1138,  1146,  1154,  1171,
    1187,  1197,  1204,  1211,  1217,  1256,  1266,  1288,  1298,  1324,
    1329,  1337,  1345,  1355,  1361,  1367,  1373,  1379,  1385,  1390,
    1396,  1405,  1410,  1411,  1417,  1426,  1427,  1435,  1447,  1448,
    1455,  1519
};
#endif

#if WABT_AST_PARSER_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"EOF\"", "error", "$undefined", "\"(\"", "\")\"", "NAT", "INT",
  "FLOAT", "TEXT", "VAR", "VALUE_TYPE", "ANYFUNC", "MUT", "NOP", "DROP",
  "BLOCK", "END", "IF", "THEN", "ELSE", "LOOP", "BR", "BR_IF", "BR_TABLE",
  "CALL", "CALL_IMPORT", "CALL_INDIRECT", "RETURN", "GET_LOCAL",
  "SET_LOCAL", "TEE_LOCAL", "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE",
  "OFFSET_EQ_NAT", "ALIGN_EQ_NAT", "CONST", "UNARY", "BINARY", "COMPARE",
  "CONVERT", "SELECT", "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY",
  "FUNC", "START", "TYPE", "PARAM", "RESULT", "LOCAL", "GLOBAL", "MODULE",
  "TABLE", "ELEM", "MEMORY", "DATA", "OFFSET", "IMPORT", "EXPORT",
  "REGISTER", "INVOKE", "GET", "ASSERT_MALFORMED", "ASSERT_INVALID",
  "ASSERT_UNLINKABLE", "ASSERT_RETURN", "ASSERT_RETURN_NAN", "ASSERT_TRAP",
  "ASSERT_EXHAUSTION", "INPUT", "OUTPUT", "LOW", "$accept",
  "non_empty_text_list", "text_list", "quoted_text", "value_type_list",
  "elem_type", "global_type", "func_type", "func_sig", "table_sig",
  "memory_sig", "limits", "type_use", "nat", "literal", "var", "var_list",
  "bind_var_opt", "bind_var", "labeling_opt", "offset_opt", "align_opt",
  "instr", "plain_instr", "block_instr", "block", "expr", "expr1", "if_",
  "instr_list", "expr_list", "const_expr", "func_fields", "func_body",
  "func_info", "func", "offset", "elem", "table", "data", "memory",
  "global", "import_kind", "import", "inline_import", "export_kind",
  "export", "inline_export_opt", "inline_export", "type_def", "start",
  "module_fields", "raw_module", "module", "script_var_opt", "action",
  "assertion", "cmd", "cmd_list", "const", "const_list", "script",
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
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328
};
# endif

#define YYPACT_NINF -271

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-271)))

#define YYTABLE_NINF -30

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -271,    26,  -271,    77,    84,  -271,  -271,  -271,  -271,  -271,
    -271,    82,    94,   107,   107,   124,   124,   124,   157,   157,
     159,   157,  -271,   106,  -271,  -271,   107,  -271,    94,    94,
     123,    94,    94,    94,   -12,  -271,   160,    18,    94,    94,
      94,  -271,    39,   140,   179,  -271,   186,   194,   198,   215,
     213,  -271,   230,   238,   245,  -271,  -271,   111,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
     248,  -271,  -271,  -271,  -271,   216,  -271,  -271,  -271,  -271,
    -271,    82,    70,    85,    82,    82,   117,    82,   117,    94,
      94,  -271,   200,   405,  -271,  -271,  -271,   266,   142,   267,
     270,    58,   271,   326,   272,  -271,  -271,   273,   272,   106,
      94,   274,  -271,  -271,  -271,   275,   285,  -271,  -271,    82,
      82,    82,    70,    70,  -271,    70,    70,  -271,    70,    70,
      70,    70,    70,   246,   246,   200,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,   438,   471,  -271,  -271,  -271,  -271,
    -271,  -271,   276,   279,   504,  -271,   280,  -271,   281,    25,
    -271,   471,    87,    87,   196,   282,   118,  -271,    82,    82,
      82,   471,   283,   284,  -271,   220,   184,   282,   282,   286,
     106,   293,   287,   289,    53,   299,  -271,    70,    82,  -271,
      82,    94,    94,  -271,  -271,  -271,  -271,  -271,  -271,    70,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,   254,   254,
    -271,   609,   300,   749,  -271,  -271,   197,   306,   316,   570,
     438,   317,   209,   327,  -271,   322,  -271,   333,   330,   334,
     471,   347,   350,   282,  -271,   290,   358,  -271,  -271,  -271,
     368,   283,  -271,  -271,   223,  -271,  -271,   106,   369,  -271,
     370,   227,   373,  -271,    65,   374,    70,    70,    70,    70,
    -271,   375,   121,   366,   151,   177,   371,    94,   376,   372,
     367,    48,   381,   125,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,   384,  -271,  -271,   386,  -271,  -271,   387,  -271,
    -271,  -271,   352,  -271,  -271,   105,  -271,  -271,  -271,  -271,
     417,  -271,  -271,   106,  -271,    82,    82,    82,    82,  -271,
     419,   420,   426,   436,  -271,   438,  -271,   450,   537,   537,
     452,   453,  -271,  -271,    82,    82,    82,    82,   182,   189,
    -271,  -271,  -271,  -271,   683,   460,  -271,   469,   483,   279,
      87,   282,   282,  -271,  -271,  -271,  -271,  -271,   438,   648,
    -271,  -271,   537,  -271,  -271,  -271,   471,  -271,   486,  -271,
     226,   471,   716,   283,  -271,   492,   502,   516,   518,   519,
     525,  -271,  -271,   474,   489,   549,   551,   471,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,    82,  -271,  -271,   553,   558,
    -271,   191,   554,   569,  -271,   471,   567,   584,   471,  -271,
     585,  -271
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     165,   170,   171,     0,     0,   148,   163,   161,   162,   166,
       1,    30,     0,   149,   149,     0,     0,     0,     0,     0,
       0,     0,    32,   135,    31,     6,   149,   150,     0,     0,
       0,     0,     0,     0,     0,   168,     0,     0,     0,     0,
       0,     2,     0,     0,     0,   168,     0,     0,     0,     0,
       0,   158,     0,     0,     0,   147,     3,     0,   146,   140,
     141,   138,   142,   139,   137,   144,   145,   136,   143,   164,
       0,   152,   153,   154,   155,     0,   157,   169,   156,   159,
     160,    30,     0,     0,    30,    30,     0,    30,     0,     0,
       0,   151,     0,    82,    22,    27,    26,     0,     0,     0,
       0,     0,   129,     0,     0,   100,    28,   129,     0,     4,
       0,     0,    23,    24,    25,     0,     0,    43,    44,    33,
      33,    33,     0,     0,    28,     0,     0,    49,     0,     0,
       0,     0,     0,    35,    35,     0,    60,    61,    62,    63,
      45,    42,    64,    65,    82,    82,    39,    40,    41,    91,
      94,    87,     0,    13,    82,   134,    13,   132,     0,     0,
      10,    82,     0,     0,     0,     0,     0,   130,    33,    33,
      33,    82,    84,     0,    28,     0,     0,     0,     0,   130,
       4,     5,     0,     0,     0,     0,   167,     0,     7,     7,
       7,     0,     0,    34,     7,     7,     7,    46,    47,     0,
      50,    51,    52,    53,    54,    55,    56,    36,    37,    37,
      59,     0,     0,     0,    83,    98,     0,     0,     0,     0,
      82,     0,     0,     0,   133,     0,    86,     0,     0,     0,
      82,     0,     0,    19,     9,     0,     0,     7,     7,     7,
       0,    84,    72,    71,     0,   102,    29,     4,     0,    18,
       0,     0,     0,   106,     0,     0,     0,     0,     0,     0,
     128,     0,     0,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,    48,    38,    57,    58,    96,     7,     7,
     119,   118,     0,    97,    12,     0,   111,   122,     0,   120,
      17,    20,     0,   103,    73,     0,    74,    99,    85,   101,
       0,   121,   107,     4,   105,    30,    30,    30,    30,   117,
       0,     0,     0,     0,    21,    82,     8,     0,    82,    82,
       0,     0,   131,    70,    33,    33,    33,    33,     0,     0,
      95,    11,   110,    28,     0,     0,    75,     0,     0,    13,
       0,     0,     0,   124,   127,   125,   126,    89,    82,     0,
      88,    92,    82,   123,    66,    68,    82,    67,    14,    16,
       0,    82,     0,    81,   109,     0,     0,     0,     0,     0,
       0,    90,    93,     0,     0,     0,     0,    82,    80,   108,
     113,   112,   116,   114,   115,    33,     7,   104,    77,     0,
      69,     0,     0,    79,    15,    82,     0,     0,    82,    76,
       0,    78
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -271,   572,  -134,    -7,  -178,   385,  -146,   505,  -143,  -152,
    -160,  -142,  -148,  -138,   481,    10,  -115,   -43,   -11,  -113,
     472,   416,  -271,  -101,  -271,  -133,   -85,  -271,  -271,  -141,
     393,  -137,  -270,  -266,   -96,  -271,   -39,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,    71,  -271,  -271,   520,    73,  -271,
    -271,  -271,   206,  -271,    40,   219,  -271,  -271,  -271,  -271,
     583,  -271,  -271
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   181,   182,    26,   269,   235,   161,    99,   217,   231,
     248,   232,   144,    96,   115,   246,   175,    23,   193,   194,
     208,   275,   145,   146,   147,   270,   148,   173,   336,   149,
     242,   227,   150,   151,   152,    59,   106,    60,    61,    62,
      63,    64,   255,    65,   153,   185,    66,   166,   154,    67,
      68,    43,     5,     6,    28,     7,     8,     9,     1,    77,
      50,     2,     3
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      24,   105,   172,   105,   214,   218,   220,   195,   196,   199,
     262,   264,   265,   223,   236,   172,   229,   230,   250,   105,
     226,    45,    46,   105,    47,    48,    49,   233,   233,     4,
     226,    52,    53,    54,   240,   249,   249,   225,    93,   233,
     233,   101,   102,    55,   107,   347,   252,    56,   212,   109,
      13,    14,   350,   351,    29,   237,   238,   239,   221,   244,
     295,   159,   271,   272,   325,   174,    44,   326,   160,   180,
      24,    11,   100,    24,    24,    94,    24,    10,   371,    95,
      13,    14,   110,   111,   191,   192,   372,   241,    98,   226,
     228,    22,    97,   288,    22,   291,   104,   160,   108,   256,
     328,   329,    25,   183,   294,   257,   296,   258,   334,   259,
     172,   305,   172,   300,    41,   316,    27,   306,   172,   307,
     103,   308,    94,    94,   282,   315,    95,    30,   323,   234,
     -29,   316,   197,   198,   -29,   200,   201,    11,   202,   203,
     204,   205,   206,    57,    58,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,   318,   241,    81,    82,    83,
      34,   316,    37,    84,    51,    85,    86,    87,    88,   338,
      89,    90,   162,   165,   163,   167,    11,   263,   177,   266,
     179,   319,   370,    69,   267,   268,   358,   316,   156,   369,
      71,   367,   316,   359,   368,   394,   366,   261,    72,   316,
     249,   316,    73,   233,   233,   112,   113,   114,   391,   273,
     335,   354,   355,   356,   357,   373,    75,    76,   360,    74,
     376,    31,    32,    33,   245,    94,    38,   299,    94,    95,
     375,    94,    95,   172,    78,    95,   389,    35,    36,    39,
      40,   247,    79,   191,   192,   187,   278,   279,   172,    80,
     363,    75,    91,    92,   397,   191,   192,   400,   278,   279,
     321,   172,   339,   340,   341,   342,   310,   311,   312,   313,
     155,   157,   390,    98,   164,   103,   176,   184,   378,   186,
     215,   207,   216,   222,   303,   224,   213,    94,   243,   251,
     274,   253,   254,   292,    24,    24,    24,    24,   117,   118,
     168,    56,   169,   260,   277,   170,   122,   123,   124,   125,
     280,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     281,   283,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   284,   285,   187,   188,   189,   190,   286,   287,   117,
     118,   168,   225,   169,   191,   192,   170,   122,   123,   124,
     125,   289,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   234,   293,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   297,   301,   302,   213,   317,   304,   309,   314,
     322,   320,   316,   324,   171,   117,   118,   119,   330,   120,
     331,   332,   121,   122,   123,   124,   125,   327,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   333,   116,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   117,   118,
     119,   337,   120,   343,   344,   121,   122,   123,   124,   125,
     345,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     346,   211,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   117,   118,   119,   348,   120,   352,   353,   121,   122,
     123,   124,   125,   362,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   364,   213,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   117,   118,   119,   365,   120,   374,
     385,   121,   122,   123,   124,   125,   379,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   380,   219,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   117,   118,   119,
     381,   120,   382,   383,   121,   122,   123,   124,   125,   384,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   386,
     349,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     117,   118,   119,   387,   120,   388,   392,   121,   122,   123,
     124,   125,   393,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   396,   395,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   117,   118,   168,   398,   169,   399,   401,
     170,   122,   123,   124,   125,    42,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   158,   209,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   210,   290,   187,   188,
     189,   190,   117,   118,   168,   276,   169,   178,    70,   170,
     122,   123,   124,   125,   298,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,     0,   135,   136,   137,   138,
     139,   140,   141,   142,   143,     0,     0,     0,   188,   189,
     190,   117,   118,   168,     0,   169,     0,     0,   170,   122,
     123,   124,   125,     0,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,     0,   135,   136,   137,   138,   139,
     140,   141,   142,   143,     0,     0,   117,   118,   168,   190,
     169,   361,     0,   170,   122,   123,   124,   125,     0,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,     0,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   117,
     118,   168,     0,   169,   377,     0,   170,   122,   123,   124,
     125,     0,   126,   127,   128,   129,   130,   131,   132,   133,
     134,     0,     0,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   117,   118,   168,     0,   169,     0,     0,   170,
     122,   123,   124,   125,     0,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,     0,   135,   136,   137,   138,
     139,   140,   141,   142,   143
};

static const yytype_int16 yycheck[] =
{
      11,    86,   103,    88,   145,   153,   154,   120,   121,   124,
     188,   189,   190,   156,   166,   116,   162,   163,   178,   104,
     161,    28,    29,   108,    31,    32,    33,   165,   166,     3,
     171,    38,    39,    40,   171,   177,   178,    12,    81,   177,
     178,    84,    85,     4,    87,   315,   180,     8,   144,    88,
      62,    63,   318,   319,    14,   168,   169,   170,   154,   174,
     238,     3,   195,   196,    16,   104,    26,    19,    10,   108,
      81,    53,    83,    84,    85,     5,    87,     0,   348,     9,
      62,    63,    89,    90,    59,    60,   352,   172,     3,   230,
       3,     9,    82,   230,     9,   233,    86,    10,    88,    46,
     278,   279,     8,   110,   237,    52,   239,    54,     3,    56,
     211,    46,   213,   247,     8,    10,     9,    52,   219,    54,
       3,    56,     5,     5,   220,     4,     9,     3,   269,    11,
       5,    10,   122,   123,     9,   125,   126,    53,   128,   129,
     130,   131,   132,     3,     4,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,     4,   241,    46,    47,    48,
       3,    10,     3,    52,     4,    54,    55,    56,    57,   303,
      59,    60,   101,   102,   101,   102,    53,   188,   107,   190,
     107,     4,   342,     4,   191,   192,     4,    10,    46,   341,
       4,   339,    10,     4,   340,     4,   339,   187,     4,    10,
     342,    10,     4,   341,   342,     5,     6,     7,   386,   199,
     295,   324,   325,   326,   327,   356,     3,     4,   333,     4,
     361,    15,    16,    17,     4,     5,    20,     4,     5,     9,
       4,     5,     9,   334,     4,     9,   377,    18,    19,    20,
      21,    57,     4,    59,    60,    48,    49,    50,   349,     4,
     335,     3,     4,    37,   395,    59,    60,   398,    49,    50,
     267,   362,   305,   306,   307,   308,   256,   257,   258,   259,
       4,     4,   385,     3,     3,     3,     3,     3,   363,     4,
       4,    35,     3,     3,    57,     4,     3,     5,     4,     3,
      36,     4,     3,     3,   305,   306,   307,   308,    13,    14,
      15,     8,    17,     4,     4,    20,    21,    22,    23,    24,
       4,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       4,     4,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     4,    10,    48,    49,    50,    51,     4,     4,    13,
      14,    15,    12,    17,    59,    60,    20,    21,    22,    23,
      24,     4,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    11,     4,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     4,     4,     4,     3,    10,     4,     4,     4,
       4,    10,    10,    16,    58,    13,    14,    15,     4,    17,
       4,     4,    20,    21,    22,    23,    24,    16,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    55,     3,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    13,    14,
      15,     4,    17,     4,     4,    20,    21,    22,    23,    24,
       4,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       4,     3,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    13,    14,    15,     4,    17,     4,     4,    20,    21,
      22,    23,    24,     3,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     4,     3,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    13,    14,    15,     4,    17,     3,
      16,    20,    21,    22,    23,    24,     4,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     4,     3,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    13,    14,    15,
       4,    17,     4,     4,    20,    21,    22,    23,    24,     4,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    50,
       3,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      13,    14,    15,     4,    17,     4,     3,    20,    21,    22,
      23,    24,     4,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     3,    19,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    13,    14,    15,    19,    17,     4,     4,
      20,    21,    22,    23,    24,    23,    26,    27,    28,    29,
      30,    31,    32,    33,    34,   100,   134,    37,    38,    39,
      40,    41,    42,    43,    44,    45,   135,   232,    48,    49,
      50,    51,    13,    14,    15,   209,    17,   107,    45,    20,
      21,    22,    23,    24,   241,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    49,    50,
      51,    13,    14,    15,    -1,    17,    -1,    -1,    20,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    13,    14,    15,    51,
      17,    18,    -1,    20,    21,    22,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    13,
      14,    15,    -1,    17,    18,    -1,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    13,    14,    15,    -1,    17,    -1,    -1,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   132,   135,   136,     3,   126,   127,   129,   130,   131,
       0,    53,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,     9,    91,    92,     8,    77,     9,   128,   128,
       3,   126,   126,   126,     3,   129,   129,     3,   126,   129,
     129,     8,    75,   125,   128,    77,    77,    77,    77,    77,
     134,     4,    77,    77,    77,     4,     8,     3,     4,   109,
     111,   112,   113,   114,   115,   117,   120,   123,   124,     4,
     134,     4,     4,     4,     4,     3,     4,   133,     4,     4,
       4,    46,    47,    48,    52,    54,    55,    56,    57,    59,
      60,     4,    37,    91,     5,     9,    87,    89,     3,    81,
      92,    91,    91,     3,    89,   100,   110,    91,    89,   110,
      77,    77,     5,     6,     7,    88,     3,    13,    14,    15,
      17,    20,    21,    22,    23,    24,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    86,    96,    97,    98,   100,   103,
     106,   107,   108,   118,   122,     4,    46,     4,    81,     3,
      10,    80,   118,   122,     3,   118,   121,   122,    15,    17,
      20,    58,    97,   101,   110,    90,     3,   118,   121,   122,
     110,    75,    76,    77,     3,   119,     4,    48,    49,    50,
      51,    59,    60,    92,    93,    93,    93,    89,    89,    90,
      89,    89,    89,    89,    89,    89,    89,    35,    94,    94,
      88,     3,   108,     3,   103,     4,     3,    82,    86,     3,
      86,   108,     3,    82,     4,    12,   103,   105,     3,    80,
      80,    83,    85,    87,    11,    79,    83,    93,    93,    93,
     105,   100,   104,     4,    90,     4,    89,    57,    84,    85,
      84,     3,    76,     4,     3,   116,    46,    52,    54,    56,
       4,    89,    78,    92,    78,    78,    92,    77,    77,    78,
      99,    99,    99,    89,    36,    95,    95,     4,    49,    50,
       4,     4,   108,     4,     4,    10,     4,     4,   105,     4,
      79,    87,     3,     4,    99,    78,    99,     4,   104,     4,
      76,     4,     4,    57,     4,    46,    52,    54,    56,     4,
      89,    89,    89,    89,     4,     4,    10,    10,     4,     4,
      10,    77,     4,   103,    16,    16,    19,    16,    78,    78,
       4,     4,     4,    55,     3,   100,   102,     4,    76,    91,
      91,    91,    91,     4,     4,     4,     4,   106,     4,     3,
     107,   107,     4,     4,    93,    93,    93,    93,     4,     4,
      90,    18,     3,   100,     4,     4,    82,    86,    80,    83,
      84,   106,   107,   103,     3,     4,   103,    18,   100,     4,
       4,     4,     4,     4,     4,    16,    50,     4,     4,   103,
      93,    78,     3,     4,     4,    19,     3,   103,    19,     4,
     103,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    76,    76,    77,    78,    78,    79,
      80,    80,    81,    82,    82,    82,    82,    83,    84,    85,
      85,    86,    87,    88,    88,    88,    89,    89,    90,    90,
      91,    91,    92,    93,    93,    94,    94,    95,    95,    96,
      96,    96,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    98,    98,    98,    98,
      99,   100,   101,   101,   101,   101,   102,   102,   102,   102,
     102,   102,   103,   103,   104,   104,   105,   106,   106,   106,
     106,   107,   107,   107,   108,   109,   109,   109,   109,   110,
     110,   111,   111,   112,   112,   113,   113,   114,   114,   114,
     115,   115,   116,   116,   116,   116,   116,   117,   117,   117,
     117,   117,   117,   118,   119,   119,   119,   119,   120,   121,
     121,   122,   123,   123,   124,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   126,   126,   127,   128,
     128,   129,   129,   130,   130,   130,   130,   130,   130,   130,
     130,   131,   131,   131,   131,   132,   132,   133,   134,   134,
     135,   136
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
       1,     1,     1,     1,     1,     1,     5,     5,     5,     8,
       2,     3,     2,     3,     3,     4,     8,     4,     9,     5,
       3,     2,     0,     2,     0,     2,     1,     1,     5,     5,
       6,     1,     5,     6,     1,     7,     6,     6,     5,     4,
       1,     6,     5,     6,    10,     6,     5,     6,     9,     8,
       7,     6,     5,     5,     5,     5,     5,     6,     6,     6,
       6,     6,     6,     5,     4,     4,     4,     4,     5,     0,
       1,     4,     4,     5,     4,     0,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     5,     5,     1,     0,
       1,     6,     5,     5,     5,     5,     5,     5,     4,     5,
       5,     1,     1,     1,     5,     0,     2,     4,     0,     2,
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
#if WABT_AST_PARSER_DEBUG

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
# if defined WABT_AST_PARSER_LTYPE_IS_TRIVIAL && WABT_AST_PARSER_LTYPE_IS_TRIVIAL

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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WabtAstLexer* lexer, WabtAstParser* parser)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, WabtAstLexer* lexer, WabtAstParser* parser)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, WabtAstLexer* lexer, WabtAstParser* parser)
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
#else /* !WABT_AST_PARSER_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !WABT_AST_PARSER_DEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, WabtAstLexer* lexer, WabtAstParser* parser)
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
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1663 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1669 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1675 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1681 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1687 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 35: /* OFFSET_EQ_NAT  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1693 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 36: /* ALIGN_EQ_NAT  */
#line 266 "src/ast-parser.y" /* yacc.c:1257  */
      {}
#line 1699 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 75: /* non_empty_text_list  */
#line 289 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_text_list(&((*yyvaluep).text_list)); }
#line 1705 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 76: /* text_list  */
#line 289 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_text_list(&((*yyvaluep).text_list)); }
#line 1711 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 77: /* quoted_text  */
#line 288 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_string_slice(&((*yyvaluep).text)); }
#line 1717 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 78: /* value_type_list  */
#line 290 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_type_vector(&((*yyvaluep).types)); }
#line 1723 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 81: /* func_type  */
#line 280 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func_signature(&((*yyvaluep).func_sig)); }
#line 1729 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 82: /* func_sig  */
#line 280 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func_signature(&((*yyvaluep).func_sig)); }
#line 1735 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* type_use  */
#line 292 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_var(&((*yyvaluep).var)); }
#line 1741 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* literal  */
#line 286 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1747 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* var  */
#line 292 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_var(&((*yyvaluep).var)); }
#line 1753 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 90: /* var_list  */
#line 291 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_var_vector_and_elements(&((*yyvaluep).vars)); }
#line 1759 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* bind_var_opt  */
#line 288 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_string_slice(&((*yyvaluep).text)); }
#line 1765 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 92: /* bind_var  */
#line 288 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_string_slice(&((*yyvaluep).text)); }
#line 1771 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* labeling_opt  */
#line 288 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_string_slice(&((*yyvaluep).text)); }
#line 1777 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* instr  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1783 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* plain_instr  */
#line 276 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr(((*yyvaluep).expr)); }
#line 1789 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* block_instr  */
#line 276 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr(((*yyvaluep).expr)); }
#line 1795 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* block  */
#line 267 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_block(&((*yyvaluep).block)); }
#line 1801 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* expr  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1807 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* expr1  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1813 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* if_  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1819 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* instr_list  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1825 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* expr_list  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1831 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* const_expr  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1837 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* func_fields  */
#line 278 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func_fields(((*yyvaluep).func_fields)); }
#line 1843 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* func_body  */
#line 278 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func_fields(((*yyvaluep).func_fields)); }
#line 1849 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* func_info  */
#line 279 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func(((*yyvaluep).func)); wabt_free(((*yyvaluep).func)); }
#line 1855 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* func  */
#line 273 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_exported_func(&((*yyvaluep).exported_func)); }
#line 1861 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* offset  */
#line 277 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1867 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* elem  */
#line 271 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_elem_segment(&((*yyvaluep).elem_segment)); }
#line 1873 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* table  */
#line 275 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_exported_table(&((*yyvaluep).exported_table)); }
#line 1879 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* data  */
#line 283 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_data_segment(&((*yyvaluep).data_segment)); }
#line 1885 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* memory  */
#line 274 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_exported_memory(&((*yyvaluep).exported_memory)); }
#line 1891 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 116: /* import_kind  */
#line 282 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_import(((*yyvaluep).import)); wabt_free(((*yyvaluep).import)); }
#line 1897 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 117: /* import  */
#line 282 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_import(((*yyvaluep).import)); wabt_free(((*yyvaluep).import)); }
#line 1903 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* inline_import  */
#line 282 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_import(((*yyvaluep).import)); wabt_free(((*yyvaluep).import)); }
#line 1909 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* export_kind  */
#line 272 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_export(&((*yyvaluep).export_)); }
#line 1915 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* export  */
#line 272 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_export(&((*yyvaluep).export_)); }
#line 1921 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* type_def  */
#line 281 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_func_type(&((*yyvaluep).func_type)); }
#line 1927 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* start  */
#line 292 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_var(&((*yyvaluep).var)); }
#line 1933 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* module_fields  */
#line 284 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_module(((*yyvaluep).module)); wabt_free(((*yyvaluep).module)); }
#line 1939 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* raw_module  */
#line 285 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_raw_module(&((*yyvaluep).raw_module)); }
#line 1945 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* module  */
#line 284 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_module(((*yyvaluep).module)); wabt_free(((*yyvaluep).module)); }
#line 1951 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* script_var_opt  */
#line 292 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_var(&((*yyvaluep).var)); }
#line 1957 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* assertion  */
#line 268 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_command(((*yyvaluep).command)); wabt_free(((*yyvaluep).command)); }
#line 1963 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* cmd  */
#line 268 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_command(((*yyvaluep).command)); wabt_free(((*yyvaluep).command)); }
#line 1969 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* cmd_list  */
#line 269 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_command_vector_and_elements(&((*yyvaluep).commands)); }
#line 1975 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* const_list  */
#line 270 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_const_vector(&((*yyvaluep).consts)); }
#line 1981 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* script  */
#line 287 "src/ast-parser.y" /* yacc.c:1257  */
      { wabt_destroy_script(&((*yyvaluep).script)); }
#line 1987 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1257  */
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
yyparse (WabtAstLexer* lexer, WabtAstParser* parser)
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
# if defined WABT_AST_PARSER_LTYPE_IS_TRIVIAL && WABT_AST_PARSER_LTYPE_IS_TRIVIAL
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
#line 305 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtTextListNode* node = new_text_list_node();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 2286 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 311 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      WabtTextListNode* node = new_text_list_node();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = NULL;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 2299 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 321 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = NULL; }
#line 2305 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 326 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtTextListNode node;
      node.text = (yyvsp[0].text);
      node.next = NULL;
      WabtTextList text_list;
      text_list.first = &node;
      text_list.last = &node;
      void* data;
      size_t size;
      dup_text_list(&text_list, &data, &size);
      (yyval.text).start = (const char*)data;
      (yyval.text).length = size;
    }
#line 2323 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 344 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.types)); }
#line 2329 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 345 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      wabt_append_type_value(&(yyval.types), &(yyvsp[0].type));
    }
#line 2338 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 351 "src/ast-parser.y" /* yacc.c:1646  */
    {}
#line 2344 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 354 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.global));
      (yyval.global).type = (yyvsp[0].type);
      (yyval.global).mutable_ = false;
    }
#line 2354 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 359 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.global));
      (yyval.global).type = (yyvsp[-1].type);
      (yyval.global).mutable_ = true;
    }
#line 2364 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 366 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2370 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 369 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.func_sig)); }
#line 2376 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 370 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.func_sig));
      (yyval.func_sig).param_types = (yyvsp[-1].types);
    }
#line 2385 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 374 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.func_sig));
      (yyval.func_sig).param_types = (yyvsp[-5].types);
      (yyval.func_sig).result_types = (yyvsp[-1].types);
    }
#line 2395 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 379 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.func_sig));
      (yyval.func_sig).result_types = (yyvsp[-1].types);
    }
#line 2404 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 386 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.table).elem_limits = (yyvsp[-1].limits); }
#line 2410 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 389 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.memory).page_limits = (yyvsp[0].limits); }
#line 2416 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 392 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2426 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 397 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2436 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 404 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2442 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 410 "src/ast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(wabt_parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wabt_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid int " PRIstringslice "\"",
                              WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2455 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 421 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2464 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 425 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2473 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 429 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2482 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 436 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WABT_VAR_TYPE_INDEX;
      (yyval.var).index = (yyvsp[0].u64);
    }
#line 2492 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 441 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = WABT_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 2502 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 448 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.vars)); }
#line 2508 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 449 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      wabt_append_var_value(&(yyval.vars), &(yyvsp[0].var));
    }
#line 2517 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 455 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2523 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 459 "src/ast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2529 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 463 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2535 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 35:
#line 468 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2541 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 469 "src/ast-parser.y" /* yacc.c:1646  */
    {
    if (WABT_FAILED(wabt_parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64),
                                     WABT_PARSE_SIGNED_AND_UNSIGNED))) {
      wabt_ast_parser_error(&(yylsp[0]), lexer, parser,
                            "invalid offset \"" PRIstringslice "\"",
                            WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2554 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 479 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2560 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 480 "src/ast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(wabt_parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                       WABT_PARSE_UNSIGNED_ONLY))) {
        wabt_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid alignment \"" PRIstringslice "\"",
                              WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2573 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 491 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2579 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 492 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2585 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 493 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 2591 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 496 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_unreachable_expr();
    }
#line 2599 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 499 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_nop_expr();
    }
#line 2607 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 502 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_drop_expr();
    }
#line 2615 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 505 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_select_expr();
    }
#line 2623 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 508 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_br_expr();
      (yyval.expr)->br.var = (yyvsp[0].var);
    }
#line 2632 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 512 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_br_if_expr();
      (yyval.expr)->br_if.var = (yyvsp[0].var);
    }
#line 2641 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 516 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_br_table_expr();
      (yyval.expr)->br_table.targets = (yyvsp[-1].vars);
      (yyval.expr)->br_table.default_target = (yyvsp[0].var);
    }
#line 2651 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 521 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_return_expr();
    }
#line 2659 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 524 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_call_expr();
      (yyval.expr)->call.var = (yyvsp[0].var);
    }
#line 2668 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 528 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_call_indirect_expr();
      (yyval.expr)->call_indirect.var = (yyvsp[0].var);
    }
#line 2677 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 532 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_get_local_expr();
      (yyval.expr)->get_local.var = (yyvsp[0].var);
    }
#line 2686 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 536 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_set_local_expr();
      (yyval.expr)->set_local.var = (yyvsp[0].var);
    }
#line 2695 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 540 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_tee_local_expr();
      (yyval.expr)->tee_local.var = (yyvsp[0].var);
    }
#line 2704 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 544 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_get_global_expr();
      (yyval.expr)->get_global.var = (yyvsp[0].var);
    }
#line 2713 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 548 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_set_global_expr();
      (yyval.expr)->set_global.var = (yyvsp[0].var);
    }
#line 2722 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 552 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_load_expr();
      (yyval.expr)->load.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->load.offset = (yyvsp[-1].u64);
      (yyval.expr)->load.align = (yyvsp[0].u32);
    }
#line 2733 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 558 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_store_expr();
      (yyval.expr)->store.opcode = (yyvsp[-2].opcode);
      (yyval.expr)->store.offset = (yyvsp[-1].u64);
      (yyval.expr)->store.align = (yyvsp[0].u32);
    }
#line 2744 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 564 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_const_expr();
      (yyval.expr)->const_.loc = (yylsp[-1]);
      if (WABT_FAILED(parse_const((yyvsp[-1].type), (yyvsp[0].literal).type, (yyvsp[0].literal).text.start,
                                  (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length,
                                  &(yyval.expr)->const_))) {
        wabt_ast_parser_error(&(yylsp[0]), lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
      wabt_free((char*)(yyvsp[0].literal).text.start);
    }
#line 2761 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 576 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_unary_expr();
      (yyval.expr)->unary.opcode = (yyvsp[0].opcode);
    }
#line 2770 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 580 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_binary_expr();
      (yyval.expr)->binary.opcode = (yyvsp[0].opcode);
    }
#line 2779 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 584 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_compare_expr();
      (yyval.expr)->compare.opcode = (yyvsp[0].opcode);
    }
#line 2788 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 588 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_convert_expr();
      (yyval.expr)->convert.opcode = (yyvsp[0].opcode);
    }
#line 2797 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 592 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_current_memory_expr();
    }
#line 2805 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 595 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_grow_memory_expr();
    }
#line 2813 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 600 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_block_expr();
      (yyval.expr)->block = (yyvsp[-2].block);
      (yyval.expr)->block.label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block.label, (yyvsp[0].text));
    }
#line 2824 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 606 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_loop_expr();
      (yyval.expr)->loop = (yyvsp[-2].block);
      (yyval.expr)->loop.label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block.label, (yyvsp[0].text));
    }
#line 2835 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 612 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_if_expr();
      (yyval.expr)->if_.true_ = (yyvsp[-2].block);
      (yyval.expr)->if_.true_.label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block.label, (yyvsp[0].text));
    }
#line 2846 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 618 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = wabt_new_if_expr();
      (yyval.expr)->if_.true_ = (yyvsp[-5].block);
      (yyval.expr)->if_.true_.label = (yyvsp[-6].text);
      (yyval.expr)->if_.false_ = (yyvsp[-2].expr_list).first;
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->block.label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block.label, (yyvsp[0].text));
    }
#line 2859 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 628 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.block));
      (yyval.block).sig = (yyvsp[-1].types);
      (yyval.block).first = (yyvsp[0].expr_list).first;
    }
#line 2869 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 636 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 2875 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 640 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 2883 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 643 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_block_expr();
      expr->block = (yyvsp[0].block);
      expr->block.label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2894 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 649 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_loop_expr();
      expr->loop = (yyvsp[0].block);
      expr->loop.label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2905 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 655 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      WabtExpr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == WABT_EXPR_TYPE_IF);
      if_->if_.true_.label = (yyvsp[-2].text);
      if_->if_.true_.sig = (yyvsp[-1].types);
    }
#line 2917 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 664 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[-5].expr_list).first;
      expr->if_.false_ = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
#line 2928 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 670 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 2938 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 675 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[-5].expr_list).first;
      expr->if_.false_ = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
#line 2949 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 79:
#line 681 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[-1].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
#line 2959 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 686 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[-1].expr_list).first;
      expr->if_.false_ = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
#line 2970 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 692 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_if_expr();
      expr->if_.true_.first = (yyvsp[0].expr_list).first;
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
#line 2980 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 700 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 2986 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 701 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 2997 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 709 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3003 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 710 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3014 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 724 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_RESULT_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3025 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 730 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_PARAM_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3036 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 90:
#line 736 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_BOUND_PARAM;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3049 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 746 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_EXPRS;
      (yyval.func_fields)->first_expr = (yyvsp[0].expr_list).first;
      (yyval.func_fields)->next = NULL;
    }
#line 3060 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 752 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_LOCAL_TYPES;
      (yyval.func_fields)->types = (yyvsp[-2].types);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3071 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 758 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_fields) = new_func_field();
      (yyval.func_fields)->type = WABT_FUNC_FIELD_TYPE_BOUND_LOCAL;
      (yyval.func_fields)->bound_type.loc = (yylsp[-4]);
      (yyval.func_fields)->bound_type.name = (yyvsp[-3].text);
      (yyval.func_fields)->bound_type.type = (yyvsp[-2].type);
      (yyval.func_fields)->next = (yyvsp[0].func_fields);
    }
#line 3084 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 768 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new_func();
      WabtFuncField* field = (yyvsp[0].func_fields);

      while (field) {
        WabtFuncField* next = field->next;
        switch (field->type) {
          case WABT_FUNC_FIELD_TYPE_EXPRS:
            (yyval.func)->first_expr = field->first_expr;
            break;

          case WABT_FUNC_FIELD_TYPE_PARAM_TYPES:
          case WABT_FUNC_FIELD_TYPE_LOCAL_TYPES: {
            WabtTypeVector* types =
                field->type == WABT_FUNC_FIELD_TYPE_PARAM_TYPES
                    ? &(yyval.func)->decl.sig.param_types
                    : &(yyval.func)->local_types;
            wabt_extend_types(types, &field->types);
            wabt_destroy_type_vector(&field->types);
            break;
          }

          case WABT_FUNC_FIELD_TYPE_BOUND_PARAM:
          case WABT_FUNC_FIELD_TYPE_BOUND_LOCAL: {
            WabtTypeVector* types;
            WabtBindingHash* bindings;
            if (field->type == WABT_FUNC_FIELD_TYPE_BOUND_PARAM) {
              types = &(yyval.func)->decl.sig.param_types;
              bindings = &(yyval.func)->param_bindings;
            } else {
              types = &(yyval.func)->local_types;
              bindings = &(yyval.func)->local_bindings;
            }

            wabt_append_type_value(types, &field->bound_type.type);
            WabtBinding* binding =
                wabt_insert_binding(bindings, &field->bound_type.name);
            binding->loc = field->bound_type.loc;
            binding->index = types->size - 1;
            break;
          }

          case WABT_FUNC_FIELD_TYPE_RESULT_TYPES:
            (yyval.func)->decl.sig.result_types = field->types;
            break;
        }

        /* we steal memory from the func field, but not the linked list nodes */
        wabt_free(field);
        field = next;
      }
    }
#line 3141 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 822 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_func));
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->decl.flags |= WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.exported_func).func->decl.type_var = (yyvsp[-2].var);
      (yyval.exported_func).func->name = (yyvsp[-4].text);
      (yyval.exported_func).export_ = (yyvsp[-3].optional_export);
    }
#line 3154 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 831 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_func));
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->decl.flags |= WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.exported_func).func->decl.type_var = (yyvsp[-2].var);
      (yyval.exported_func).func->name = (yyvsp[-3].text);
    }
#line 3166 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 838 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_func));
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->name = (yyvsp[-3].text);
      (yyval.exported_func).export_ = (yyvsp[-2].optional_export);
    }
#line 3177 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 845 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_func));
      (yyval.exported_func).func = (yyvsp[-1].func);
      (yyval.exported_func).func->name = (yyvsp[-2].text);
    }
#line 3187 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 855 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3195 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 862 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.elem_segment));
      (yyval.elem_segment).table_var = (yyvsp[-3].var);
      (yyval.elem_segment).offset = (yyvsp[-2].expr_list).first;
      (yyval.elem_segment).vars = (yyvsp[-1].vars);
    }
#line 3206 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 868 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.elem_segment));
      (yyval.elem_segment).table_var.loc = (yylsp[-3]);
      (yyval.elem_segment).table_var.type = WABT_VAR_TYPE_INDEX;
      (yyval.elem_segment).table_var.index = 0;
      (yyval.elem_segment).offset = (yyvsp[-2].expr_list).first;
      (yyval.elem_segment).vars = (yyvsp[-1].vars);
    }
#line 3219 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 879 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exported_table).table = (yyvsp[-1].table);
      (yyval.exported_table).table.name = (yyvsp[-3].text);
      (yyval.exported_table).has_elem_segment = false;
      (yyval.exported_table).export_ = (yyvsp[-2].optional_export);
    }
#line 3230 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 104:
#line 886 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_const_expr();
      expr->loc = (yylsp[-8]);
      expr->const_.type = WABT_TYPE_I32;
      expr->const_.u32 = 0;

      WABT_ZERO_MEMORY((yyval.exported_table));
      (yyval.exported_table).table.name = (yyvsp[-7].text);
      (yyval.exported_table).table.elem_limits.initial = (yyvsp[-2].vars).size;
      (yyval.exported_table).table.elem_limits.max = (yyvsp[-2].vars).size;
      (yyval.exported_table).table.elem_limits.has_max = true;
      (yyval.exported_table).has_elem_segment = true;
      (yyval.exported_table).elem_segment.offset = expr;
      (yyval.exported_table).elem_segment.vars = (yyvsp[-2].vars);
      (yyval.exported_table).export_ = (yyvsp[-6].optional_export);
    }
#line 3251 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 905 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.data_segment));
      (yyval.data_segment).memory_var = (yyvsp[-3].var);
      (yyval.data_segment).offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.data_segment).data, &(yyval.data_segment).size);
      wabt_destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3263 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 912 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.data_segment));
      (yyval.data_segment).memory_var.loc = (yylsp[-3]);
      (yyval.data_segment).memory_var.type = WABT_VAR_TYPE_INDEX;
      (yyval.data_segment).memory_var.index = 0;
      (yyval.data_segment).offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.data_segment).data, &(yyval.data_segment).size);
      wabt_destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3277 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 924 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_memory));
      (yyval.exported_memory).memory = (yyvsp[-1].memory);
      (yyval.exported_memory).memory.name = (yyvsp[-3].text);
      (yyval.exported_memory).has_data_segment = false;
      (yyval.exported_memory).export_ = (yyvsp[-2].optional_export);
    }
#line 3289 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 931 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_const_expr();
      expr->loc = (yylsp[-7]);
      expr->const_.type = WABT_TYPE_I32;
      expr->const_.u32 = 0;

      WABT_ZERO_MEMORY((yyval.exported_memory));
      (yyval.exported_memory).has_data_segment = true;
      (yyval.exported_memory).data_segment.offset = expr;
      dup_text_list(&(yyvsp[-2].text_list), &(yyval.exported_memory).data_segment.data, &(yyval.exported_memory).data_segment.size);
      wabt_destroy_text_list(&(yyvsp[-2].text_list));
      uint32_t byte_size = WABT_ALIGN_UP_TO_PAGE((yyval.exported_memory).data_segment.size);
      uint32_t page_size = WABT_BYTES_TO_PAGES(byte_size);
      (yyval.exported_memory).memory.name = (yyvsp[-6].text);
      (yyval.exported_memory).memory.page_limits.initial = page_size;
      (yyval.exported_memory).memory.page_limits.max = page_size;
      (yyval.exported_memory).memory.page_limits.has_max = true;
      (yyval.exported_memory).export_ = (yyvsp[-5].optional_export);
    }
#line 3313 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 951 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WabtExpr* expr = wabt_new_const_expr();
      expr->loc = (yylsp[-6]);
      expr->const_.type = WABT_TYPE_I32;
      expr->const_.u32 = 0;

      WABT_ZERO_MEMORY((yyval.exported_memory));
      (yyval.exported_memory).has_data_segment = true;
      (yyval.exported_memory).data_segment.offset = expr;
      dup_text_list(&(yyvsp[-2].text_list), &(yyval.exported_memory).data_segment.data, &(yyval.exported_memory).data_segment.size);
      wabt_destroy_text_list(&(yyvsp[-2].text_list));
      uint32_t byte_size = WABT_ALIGN_UP_TO_PAGE((yyval.exported_memory).data_segment.size);
      uint32_t page_size = WABT_BYTES_TO_PAGES(byte_size);
      (yyval.exported_memory).memory.name = (yyvsp[-5].text);
      (yyval.exported_memory).memory.page_limits.initial = page_size;
      (yyval.exported_memory).memory.page_limits.max = page_size;
      (yyval.exported_memory).memory.page_limits.has_max = true;
      (yyval.exported_memory).export_.has_export = false;
    }
#line 3337 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 973 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_global));
      (yyval.exported_global).global = (yyvsp[-2].global);
      (yyval.exported_global).global.name = (yyvsp[-4].text);
      (yyval.exported_global).global.init_expr = (yyvsp[-1].expr_list).first;
      (yyval.exported_global).export_ = (yyvsp[-3].optional_export);
    }
#line 3349 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 980 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.exported_global));
      (yyval.exported_global).global = (yyvsp[-2].global);
      (yyval.exported_global).global.name = (yyvsp[-3].text);
      (yyval.exported_global).global.init_expr = (yyvsp[-1].expr_list).first;
      (yyval.exported_global).export_.has_export = false;
    }
#line 3361 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 993 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->kind = WABT_EXTERNAL_KIND_FUNC;
      (yyval.import)->func.name = (yyvsp[-2].text);
      (yyval.import)->func.decl.flags = WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->func.decl.type_var = (yyvsp[-1].var);
    }
#line 3373 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 1000 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->kind = WABT_EXTERNAL_KIND_FUNC;
      (yyval.import)->func.name = (yyvsp[-2].text);
      (yyval.import)->func.decl.sig = (yyvsp[-1].func_sig);
    }
#line 3384 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 1006 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->kind = WABT_EXTERNAL_KIND_TABLE;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table.name = (yyvsp[-2].text);
    }
#line 3395 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 1012 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->kind = WABT_EXTERNAL_KIND_MEMORY;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory.name = (yyvsp[-2].text);
    }
#line 3406 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 1018 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->kind = WABT_EXTERNAL_KIND_GLOBAL;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global.name = (yyvsp[-2].text);
    }
#line 3417 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 1026 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-1].import);
      (yyval.import)->module_name = (yyvsp[-3].text);
      (yyval.import)->field_name = (yyvsp[-2].text);
    }
#line 3427 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 1031 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-2].import);
      (yyval.import)->kind = WABT_EXTERNAL_KIND_FUNC;
      (yyval.import)->func.name = (yyvsp[-3].text);
      (yyval.import)->func.decl.flags = WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      (yyval.import)->func.decl.type_var = (yyvsp[-1].var);
    }
#line 3439 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 1038 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-2].import);
      (yyval.import)->kind = WABT_EXTERNAL_KIND_FUNC;
      (yyval.import)->func.name = (yyvsp[-3].text);
      (yyval.import)->func.decl.sig = (yyvsp[-1].func_sig);
    }
#line 3450 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 120:
#line 1044 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-2].import);
      (yyval.import)->kind = WABT_EXTERNAL_KIND_TABLE;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table.name = (yyvsp[-3].text);
    }
#line 3461 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 1050 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-2].import);
      (yyval.import)->kind = WABT_EXTERNAL_KIND_MEMORY;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory.name = (yyvsp[-3].text);
    }
#line 3472 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 1056 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = (yyvsp[-2].import);
      (yyval.import)->kind = WABT_EXTERNAL_KIND_GLOBAL;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global.name = (yyvsp[-3].text);
    }
#line 3483 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 1065 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new_import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
#line 3493 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 1073 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.export_));
      (yyval.export_).kind = WABT_EXTERNAL_KIND_FUNC;
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3503 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 1078 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.export_));
      (yyval.export_).kind = WABT_EXTERNAL_KIND_TABLE;
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3513 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 1083 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.export_));
      (yyval.export_).kind = WABT_EXTERNAL_KIND_MEMORY;
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3523 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 1088 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.export_));
      (yyval.export_).kind = WABT_EXTERNAL_KIND_GLOBAL;
      (yyval.export_).var = (yyvsp[-1].var);
    }
#line 3533 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 1095 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = (yyvsp[-1].export_);
      (yyval.export_).name = (yyvsp[-2].text);
    }
#line 3542 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 1102 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.optional_export));
      (yyval.optional_export).has_export = false;
    }
#line 3551 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 1109 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.optional_export));
      (yyval.optional_export).has_export = true;
      (yyval.optional_export).export_.name = (yyvsp[-1].text);
    }
#line 3561 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 1120 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.func_type));
      (yyval.func_type).sig = (yyvsp[-1].func_sig);
    }
#line 3570 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 1124 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_type).name = (yyvsp[-2].text);
      (yyval.func_type).sig = (yyvsp[-1].func_sig);
    }
#line 3579 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 1131 "src/ast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 3585 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 1135 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new_module();
    }
#line 3593 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 1138 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, FUNC_TYPE, func_type, (yylsp[0]), (yyvsp[0].func_type));
      APPEND_ITEM_TO_VECTOR((yyval.module), FuncType, func_type, func_types,
                            &field->func_type);
      INSERT_BINDING((yyval.module), func_type, func_types, (yylsp[0]), (yyvsp[0].func_type).name);
    }
#line 3606 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 1146 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, GLOBAL, global, (yylsp[0]), (yyvsp[0].exported_global).global);
      APPEND_ITEM_TO_VECTOR((yyval.module), Global, global, globals, &field->global);
      INSERT_BINDING((yyval.module), global, globals, (yylsp[0]), (yyvsp[0].exported_global).global.name);
      APPEND_INLINE_EXPORT((yyval.module), GLOBAL, (yylsp[0]), (yyvsp[0].exported_global), (yyval.module)->globals.size - 1);
    }
#line 3619 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1154 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, TABLE, table, (yylsp[0]), (yyvsp[0].exported_table).table);
      APPEND_ITEM_TO_VECTOR((yyval.module), Table, table, tables, &field->table);
      INSERT_BINDING((yyval.module), table, tables, (yylsp[0]), (yyvsp[0].exported_table).table.name);
      APPEND_INLINE_EXPORT((yyval.module), TABLE, (yylsp[0]), (yyvsp[0].exported_table), (yyval.module)->tables.size - 1);

      if ((yyvsp[0].exported_table).has_elem_segment) {
        WabtModuleField* elem_segment_field;
        APPEND_FIELD_TO_LIST((yyval.module), elem_segment_field, ELEM_SEGMENT, elem_segment,
                             (yylsp[0]), (yyvsp[0].exported_table).elem_segment);
        APPEND_ITEM_TO_VECTOR((yyval.module), ElemSegment, elem_segment, elem_segments,
                              &elem_segment_field->elem_segment);
      }

    }
#line 3641 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1171 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, MEMORY, memory, (yylsp[0]), (yyvsp[0].exported_memory).memory);
      APPEND_ITEM_TO_VECTOR((yyval.module), Memory, memory, memories, &field->memory);
      INSERT_BINDING((yyval.module), memory, memories, (yylsp[0]), (yyvsp[0].exported_memory).memory.name);
      APPEND_INLINE_EXPORT((yyval.module), MEMORY, (yylsp[0]), (yyvsp[0].exported_memory), (yyval.module)->memories.size - 1);

      if ((yyvsp[0].exported_memory).has_data_segment) {
        WabtModuleField* data_segment_field;
        APPEND_FIELD_TO_LIST((yyval.module), data_segment_field, DATA_SEGMENT, data_segment,
                             (yylsp[0]), (yyvsp[0].exported_memory).data_segment);
        APPEND_ITEM_TO_VECTOR((yyval.module), DataSegment, data_segment, data_segments,
                              &data_segment_field->data_segment);
      }
    }
#line 3662 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1187 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, FUNC, func, (yylsp[0]), *(yyvsp[0].exported_func).func);
      append_implicit_func_declaration(&(yylsp[0]), (yyval.module), &field->func.decl);
      APPEND_ITEM_TO_VECTOR((yyval.module), Func, func, funcs, &field->func);
      INSERT_BINDING((yyval.module), func, funcs, (yylsp[0]), (yyvsp[0].exported_func).func->name);
      APPEND_INLINE_EXPORT((yyval.module), FUNC, (yylsp[0]), (yyvsp[0].exported_func), (yyval.module)->funcs.size - 1);
      wabt_free((yyvsp[0].exported_func).func);
    }
#line 3677 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1197 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, ELEM_SEGMENT, elem_segment, (yylsp[0]), (yyvsp[0].elem_segment));
      APPEND_ITEM_TO_VECTOR((yyval.module), ElemSegment, elem_segment, elem_segments,
                            &field->elem_segment);
    }
#line 3689 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1204 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, DATA_SEGMENT, data_segment, (yylsp[0]), (yyvsp[0].data_segment));
      APPEND_ITEM_TO_VECTOR((yyval.module), DataSegment, data_segment, data_segments,
                            &field->data_segment);
    }
#line 3701 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1211 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, START, start, (yylsp[0]), (yyvsp[0].var));
      (yyval.module)->start = &field->start;
    }
#line 3712 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1217 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field;
      APPEND_FIELD_TO_LIST((yyval.module), field, IMPORT, import, (yylsp[0]), *(yyvsp[0].import));
      CHECK_IMPORT_ORDERING((yyval.module), func, funcs, (yylsp[0]));
      CHECK_IMPORT_ORDERING((yyval.module), table, tables, (yylsp[0]));
      CHECK_IMPORT_ORDERING((yyval.module), memory, memories, (yylsp[0]));
      CHECK_IMPORT_ORDERING((yyval.module), global, globals, (yylsp[0]));
      switch ((yyvsp[0].import)->kind) {
        case WABT_EXTERNAL_KIND_FUNC:
          append_implicit_func_declaration(&(yylsp[0]), (yyval.module), &field->import.func.decl);
          APPEND_ITEM_TO_VECTOR((yyval.module), Func, func, funcs, &field->import.func);
          INSERT_BINDING((yyval.module), func, funcs, (yylsp[0]), field->import.func.name);
          (yyval.module)->num_func_imports++;
          break;
        case WABT_EXTERNAL_KIND_TABLE:
          APPEND_ITEM_TO_VECTOR((yyval.module), Table, table, tables, &field->import.table);
          INSERT_BINDING((yyval.module), table, tables, (yylsp[0]), field->import.table.name);
          (yyval.module)->num_table_imports++;
          break;
        case WABT_EXTERNAL_KIND_MEMORY:
          APPEND_ITEM_TO_VECTOR((yyval.module), Memory, memory, memories,
                                &field->import.memory);
          INSERT_BINDING((yyval.module), memory, memories, (yylsp[0]), field->import.memory.name);
          (yyval.module)->num_memory_imports++;
          break;
        case WABT_EXTERNAL_KIND_GLOBAL:
          APPEND_ITEM_TO_VECTOR((yyval.module), Global, global, globals,
                                &field->import.global);
          INSERT_BINDING((yyval.module), global, globals, (yylsp[0]), field->import.global.name);
          (yyval.module)->num_global_imports++;
          break;
        case WABT_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
      wabt_free((yyvsp[0].import));
      APPEND_ITEM_TO_VECTOR((yyval.module), Import, import, imports, &field->import);
    }
#line 3756 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1256 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      WabtModuleField* field = wabt_append_module_field((yyval.module));
      APPEND_FIELD_TO_LIST((yyval.module), field, EXPORT, export_, (yylsp[0]), (yyvsp[0].export_));
      APPEND_ITEM_TO_VECTOR((yyval.module), Export, export, exports, &field->export_);
      INSERT_BINDING((yyval.module), export, exports, (yylsp[0]), (yyvsp[0].export_).name);
    }
#line 3768 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1266 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WABT_RAW_MODULE_TYPE_TEXT;
      (yyval.raw_module).text = (yyvsp[-1].module);
      (yyval.raw_module).text->name = (yyvsp[-2].text);
      (yyval.raw_module).text->loc = (yylsp[-3]);

      /* resolve func type variables where the signature was not specified
       * explicitly */
      size_t i;
      for (i = 0; i < (yyvsp[-1].module)->funcs.size; ++i) {
        WabtFunc* func = (yyvsp[-1].module)->funcs.data[i];
        if (wabt_decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          WabtFuncType* func_type =
              wabt_get_func_type_by_var((yyvsp[-1].module), &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
            func->decl.flags |= WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }
    }
#line 3795 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1288 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module).type = WABT_RAW_MODULE_TYPE_BINARY;
      (yyval.raw_module).binary.name = (yyvsp[-2].text);
      (yyval.raw_module).binary.loc = (yylsp[-3]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.raw_module).binary.data, &(yyval.raw_module).binary.size);
      wabt_destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3807 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1298 "src/ast-parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].raw_module).type == WABT_RAW_MODULE_TYPE_TEXT) {
        (yyval.module) = (yyvsp[0].raw_module).text;
      } else {
        assert((yyvsp[0].raw_module).type == WABT_RAW_MODULE_TYPE_BINARY);
        (yyval.module) = new_module();
        WabtReadBinaryOptions options = WABT_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorCallbackData user_data;
        user_data.loc = &(yyvsp[0].raw_module).binary.loc;
        user_data.lexer = lexer;
        user_data.parser = parser;
        WabtBinaryErrorHandler error_handler;
        error_handler.on_error = on_read_binary_error;
        error_handler.user_data = &user_data;
        wabt_read_binary_ast((yyvsp[0].raw_module).binary.data, (yyvsp[0].raw_module).binary.size, &options,
                             &error_handler, (yyval.module));
        wabt_free((yyvsp[0].raw_module).binary.data);
        (yyval.module)->name = (yyvsp[0].raw_module).binary.name;
        (yyval.module)->loc = (yyvsp[0].raw_module).binary.loc;
      }
    }
#line 3833 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1324 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = WABT_VAR_TYPE_INDEX;
      (yyval.var).index = INVALID_VAR_INDEX;
    }
#line 3843 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1329 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = WABT_VAR_TYPE_NAME;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 3853 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1337 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.action));
      (yyval.action).loc = (yylsp[-4]);
      (yyval.action).module_var = (yyvsp[-3].var);
      (yyval.action).type = WABT_ACTION_TYPE_INVOKE;
      (yyval.action).invoke.name = (yyvsp[-2].text);
      (yyval.action).invoke.args = (yyvsp[-1].consts);
    }
#line 3866 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1345 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.action));
      (yyval.action).loc = (yylsp[-3]);
      (yyval.action).module_var = (yyvsp[-2].var);
      (yyval.action).type = WABT_ACTION_TYPE_GET;
      (yyval.action).invoke.name = (yyvsp[-1].text);
    }
#line 3878 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1355 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_MALFORMED;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
#line 3889 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1361 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_INVALID;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 3900 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1367 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_UNLINKABLE;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
#line 3911 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1373 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
#line 3922 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1379 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_RETURN;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
#line 3933 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1385 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_RETURN_NAN;
      (yyval.command)->assert_return_nan.action = (yyvsp[-1].action);
    }
#line 3943 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1390 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_TRAP;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3954 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1396 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ASSERT_EXHAUSTION;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3965 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1405 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_ACTION;
      (yyval.command)->action = (yyvsp[0].action);
    }
#line 3975 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1411 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_MODULE;
      (yyval.command)->module = *(yyvsp[0].module);
      wabt_free((yyvsp[0].module));
    }
#line 3986 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1417 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new_command();
      (yyval.command)->type = WABT_COMMAND_TYPE_REGISTER;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
#line 3998 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1426 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.commands)); }
#line 4004 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1427 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      wabt_append_command_value(&(yyval.commands), (yyvsp[0].command));
      wabt_free((yyvsp[0].command));
    }
#line 4014 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1435 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      if (WABT_FAILED(parse_const((yyvsp[-2].type), (yyvsp[-1].literal).type, (yyvsp[-1].literal).text.start,
                                  (yyvsp[-1].literal).text.start + (yyvsp[-1].literal).text.length, &(yyval.const_)))) {
        wabt_ast_parser_error(&(yylsp[-1]), lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WABT_PRINTF_STRING_SLICE_ARG((yyvsp[-1].literal).text));
      }
      wabt_free((char*)(yyvsp[-1].literal).text.start);
    }
#line 4029 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1447 "src/ast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.consts)); }
#line 4035 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1448 "src/ast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      wabt_append_const_value(&(yyval.consts), &(yyvsp[0].const_));
    }
#line 4044 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1455 "src/ast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.script));
      (yyval.script).commands = (yyvsp[0].commands);

      int last_module_index = -1;
      size_t i;
      for (i = 0; i < (yyval.script).commands.size; ++i) {
        WabtCommand* command = &(yyval.script).commands.data[i];
        WabtVar* module_var = NULL;
        switch (command->type) {
          case WABT_COMMAND_TYPE_MODULE: {
            last_module_index = i;

            /* Wire up module name bindings. */
            WabtModule* module = &command->module;
            if (module->name.length == 0)
              continue;

            WabtStringSlice module_name = wabt_dup_string_slice(module->name);
            WabtBinding* binding =
                wabt_insert_binding(&(yyval.script).module_bindings, &module_name);
            binding->loc = module->loc;
            binding->index = i;
            break;
          }

          case WABT_COMMAND_TYPE_ASSERT_RETURN:
            module_var = &command->assert_return.action.module_var;
            goto has_module_var;
          case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN:
            module_var = &command->assert_return_nan.action.module_var;
            goto has_module_var;
          case WABT_COMMAND_TYPE_ASSERT_TRAP:
          case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
            module_var = &command->assert_trap.action.module_var;
            goto has_module_var;
          case WABT_COMMAND_TYPE_ACTION:
            module_var = &command->action.module_var;
            goto has_module_var;
          case WABT_COMMAND_TYPE_REGISTER:
            module_var = &command->register_.var;
            goto has_module_var;

          has_module_var: {
            /* Resolve actions with an invalid index to use the preceding
             * module. */
            if (module_var->type == WABT_VAR_TYPE_INDEX &&
                module_var->index == INVALID_VAR_INDEX) {
              module_var->index = last_module_index;
            }
            break;
          }

          default:
            break;
        }
      }
      parser->script = (yyval.script);
    }
#line 4108 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4112 "src/prebuilt/ast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1522 "src/ast-parser.y" /* yacc.c:1906  */


static void append_expr_list(WabtExprList* expr_list, WabtExprList* expr) {
  if (!expr->first)
    return;
  if (expr_list->last)
    expr_list->last->next = expr->first;
  else
    expr_list->first = expr->first;
  expr_list->last = expr->last;
  expr_list->size += expr->size;
}

static void append_expr(WabtExprList* expr_list, WabtExpr* expr) {
  if (expr_list->last)
    expr_list->last->next = expr;
  else
    expr_list->first = expr;
  expr_list->last = expr;
  expr_list->size++;
}

static WabtExprList join_exprs1(WabtLocation* loc, WabtExpr* expr1) {
  WabtExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr(&result, expr1);
  expr1->loc = *loc;
  return result;
}

static WabtExprList join_exprs2(WabtLocation* loc, WabtExprList* expr1,
                                WabtExpr* expr2) {
  WabtExprList result;
  WABT_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr(&result, expr2);
  expr2->loc = *loc;
  return result;
}

static WabtResult parse_const(WabtType type,
                              WabtLiteralType literal_type,
                              const char* s,
                              const char* end,
                              WabtConst* out) {
  out->type = type;
  switch (type) {
    case WABT_TYPE_I32:
      return wabt_parse_int32(s, end, &out->u32,
                              WABT_PARSE_SIGNED_AND_UNSIGNED);
    case WABT_TYPE_I64:
      return wabt_parse_int64(s, end, &out->u64,
                              WABT_PARSE_SIGNED_AND_UNSIGNED);
    case WABT_TYPE_F32:
      return wabt_parse_float(literal_type, s, end, &out->f32_bits);
    case WABT_TYPE_F64:
      return wabt_parse_double(literal_type, s, end, &out->f64_bits);
    default:
      assert(0);
      break;
  }
  return WABT_ERROR;
}

static size_t copy_string_contents(WabtStringSlice* text, char* dest) {
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
          if (WABT_SUCCEEDED(wabt_parse_hexdigit(src[0], &hi)) &&
              WABT_SUCCEEDED(wabt_parse_hexdigit(src[1], &lo))) {
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

static void dup_text_list(WabtTextList* text_list,
                          void** out_data,
                          size_t* out_size) {
  /* walk the linked list to see how much total space is needed */
  size_t total_size = 0;
  WabtTextListNode* node;
  for (node = text_list->first; node; node = node->next) {
    /* Always allocate enough space for the entire string including the escape
     * characters. It will only get shorter, and this way we only have to
     * iterate through the string once. */
    const char* src = node->text.start + 1;
    const char* end = node->text.start + node->text.length - 1;
    size_t size = (end > src) ? (end - src) : 0;
    total_size += size;
  }
  char* result = (char*)wabt_alloc(total_size);
  char* dest = result;
  for (node = text_list->first; node; node = node->next) {
    size_t actual_size = copy_string_contents(&node->text, dest);
    dest += actual_size;
  }
  *out_data = result;
  *out_size = dest - result;
}

static bool is_empty_signature(WabtFuncSignature* sig) {
  return sig->result_types.size == 0 && sig->param_types.size == 0;
}

static void append_implicit_func_declaration(WabtLocation* loc,
                                             WabtModule* module,
                                             WabtFuncDeclaration* decl) {
  if (wabt_decl_has_func_type(decl))
    return;

  int sig_index = wabt_get_func_type_index_by_decl(module, decl);
  if (sig_index == -1) {
    wabt_append_implicit_func_type(loc, module, &decl->sig);
  } else {
    /* signature already exists, share that one and destroy this one */
    wabt_destroy_func_signature(&decl->sig);
    WabtFuncSignature* sig = &module->func_types.data[sig_index]->sig;
    decl->sig = *sig;
  }

  decl->flags |= WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
}

WabtResult wabt_parse_ast(WabtAstLexer* lexer,
                          struct WabtScript* out_script,
                          WabtSourceErrorHandler* error_handler) {
  WabtAstParser parser;
  WABT_ZERO_MEMORY(parser);
  parser.error_handler = error_handler;
  int result = wabt_ast_parser_parse(lexer, &parser);
  wabt_free(parser.yyssa);
  wabt_free(parser.yyvsa);
  wabt_free(parser.yylsa);
  *out_script = parser.script;
  return result == 0 && parser.errors == 0 ? WABT_OK : WABT_ERROR;
}

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data) {
  BinaryErrorCallbackData* data = (BinaryErrorCallbackData*)user_data;
  if (offset == WABT_UNKNOWN_OFFSET) {
    wabt_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: %s", error);
  } else {
    wabt_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: @0x%08x: %s", offset, error);
  }
}
