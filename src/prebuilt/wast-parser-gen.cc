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
#define YYLAST   991

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  212
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  474

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
     363,   368,   375,   381,   392,   396,   400,   407,   412,   419,
     420,   426,   427,   430,   434,   435,   439,   440,   456,   457,
     472,   473,   474,   478,   481,   484,   487,   490,   493,   496,
     499,   502,   505,   508,   511,   514,   517,   520,   523,   526,
     529,   542,   545,   548,   551,   554,   557,   560,   563,   569,
     574,   579,   584,   590,   599,   602,   607,   614,   618,   625,
     626,   632,   636,   639,   644,   649,   655,   664,   667,   673,
     681,   684,   688,   692,   696,   700,   704,   711,   716,   722,
     728,   729,   737,   738,   746,   751,   758,   768,   782,   789,
     794,   804,   812,   823,   830,   831,   837,   847,   848,   857,
     864,   865,   871,   881,   882,   891,   898,   902,   907,   919,
     922,   926,   935,   949,   963,   969,   977,   985,  1004,  1013,
    1027,  1041,  1046,  1054,  1062,  1085,  1099,  1105,  1113,  1126,
    1134,  1142,  1148,  1154,  1160,  1168,  1178,  1186,  1191,  1196,
    1201,  1206,  1213,  1222,  1232,  1239,  1250,  1258,  1259,  1260,
    1261,  1262,  1263,  1264,  1265,  1266,  1267,  1268,  1272,  1273,
    1277,  1282,  1290,  1310,  1317,  1322,  1330,  1350,  1358,  1369,
    1379,  1389,  1395,  1401,  1407,  1413,  1419,  1424,  1429,  1435,
    1444,  1449,  1450,  1455,  1464,  1468,  1475,  1487,  1488,  1495,
    1498,  1558,  1570
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
  "except", "exception", "func", "func_fields", "func_fields_import",
  "func_fields_import1", "func_fields_import_result", "func_fields_body",
  "func_fields_body1", "func_result_body", "func_body", "func_body1",
  "offset", "elem", "table", "table_fields", "data", "memory",
  "memory_fields", "global", "global_fields", "import_desc", "import",
  "inline_import", "export_desc", "export", "inline_export", "type_def",
  "start", "module_field", "module_fields_opt", "module_fields", "module",
  "inline_module", "script_var_opt", "script_module", "action",
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334
};
# endif

#define YYPACT_NINF -391

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-391)))

#define YYTABLE_NINF -31

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      30,   913,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,    44,  -391,  -391,  -391,  -391,
    -391,  -391,    53,  -391,    81,    86,    93,   119,    86,    86,
     114,    86,   114,   104,   104,    86,    86,   104,   148,   148,
     117,   117,   117,   132,   132,   132,   229,   132,   142,  -391,
     222,  -391,  -391,  -391,   452,  -391,  -391,  -391,  -391,   207,
     193,   247,   251,    25,   113,   403,   264,  -391,  -391,   224,
     264,   297,  -391,   104,   285,  -391,    24,   148,  -391,   104,
     104,   242,   104,   104,   104,    75,  -391,   307,   308,   154,
     104,   104,   104,   359,  -391,  -391,    86,    86,    86,    93,
      93,  -391,  -391,  -391,  -391,    93,    93,  -391,    93,    93,
      93,    93,    93,   274,   274,   190,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,   489,   526,  -391,  -391,  -391,    93,
      93,    86,  -391,   310,  -391,  -391,  -391,  -391,  -391,   312,
     452,  -391,   313,  -391,   314,    46,  -391,   526,   315,    49,
      25,   120,  -391,   318,  -391,   319,   320,   322,   320,   113,
      86,    86,    86,   526,   321,   325,    86,  -391,   216,   -14,
    -391,  -391,   329,   320,   224,   297,  -391,   326,   331,   338,
     111,   340,   121,   297,   297,   344,    44,   345,  -391,   347,
     349,   351,   352,   236,  -391,  -391,   356,   357,   358,    93,
      86,  -391,    86,   104,   104,  -391,   563,   563,   563,  -391,
    -391,    93,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
     283,   283,  -391,  -391,  -391,  -391,   674,  -391,   912,  -391,
    -391,  -391,   563,  -391,   195,   339,  -391,  -391,  -391,  -391,
     227,   361,  -391,  -391,   353,  -391,  -391,  -391,   355,  -391,
    -391,   317,  -391,  -391,  -391,  -391,  -391,   563,   374,   563,
     380,   321,  -391,  -391,   375,   233,  -391,  -391,   297,  -391,
    -391,  -391,   381,  -391,  -391,   158,  -391,   394,    93,    93,
      93,    93,    93,  -391,  -391,  -391,   129,   205,  -391,  -391,
     280,  -391,  -391,  -391,  -391,   368,  -391,  -391,  -391,  -391,
    -391,   395,   130,   400,   162,   169,   405,   104,   415,   837,
     563,   412,  -391,   139,   413,   226,  -391,  -391,  -391,   260,
      86,  -391,   248,  -391,    86,  -391,  -391,   438,  -391,  -391,
     799,   374,   440,  -391,  -391,  -391,  -391,  -391,   439,  -391,
     449,  -391,    86,    86,    86,    86,  -391,   453,   454,   455,
     456,   457,  -391,  -391,  -391,   190,  -391,   489,   458,   600,
     637,   459,   464,  -391,  -391,  -391,    86,    86,    86,    86,
      93,   526,   260,   461,   177,   446,   180,   181,   460,   206,
    -391,   241,   526,  -391,   875,   321,    86,  -391,   468,    49,
     320,   320,  -391,  -391,  -391,  -391,  -391,   474,  -391,   489,
     717,  -391,   760,  -391,   637,  -391,   208,  -391,  -391,   526,
    -391,   526,  -391,  -391,    86,   339,   487,   502,   313,   503,
     505,  -391,   510,   526,  -391,   563,   212,   511,   524,   538,
     540,   541,  -391,  -391,  -391,  -391,   535,  -391,  -391,  -391,
     339,   498,  -391,  -391,   313,   525,  -391,   562,   577,   578,
    -391,  -391,  -391,  -391,  -391,    86,  -391,  -391,   569,   586,
     599,  -391,   526,   597,   260,  -391,   614,   526,   615,  -391,
     621,   599,  -391,  -391
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     209,     0,   177,   105,   171,   172,   169,   173,   170,   168,
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
       0,     0,     0,   162,   106,     8,     0,     0,   186,   203,
       0,   190,   191,   192,   193,     0,   195,   208,   194,   198,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     100,     0,    76,     0,     0,    49,    39,    58,    59,     0,
       7,     7,     0,   110,     7,     7,    12,     0,    29,    83,
       0,     0,     0,    85,    90,    84,   129,   103,     0,   131,
       0,   138,    31,    31,    31,    31,   155,     0,     0,     0,
       0,     0,   187,   188,   189,     0,    22,   100,     0,   100,
     100,     0,     0,   163,     7,    75,    34,    34,    34,    34,
       0,   100,    79,     0,     0,     0,     0,     0,     0,     0,
      11,     0,   100,    89,     0,    96,    34,   144,    16,     0,
       0,     0,   157,   160,   158,   159,   161,     0,   121,   100,
       0,   124,     0,   127,   100,   156,     0,    69,    71,   100,
      70,   100,    78,    80,    34,   117,     0,   117,    16,     0,
      16,   137,     0,   100,    95,   100,     0,     0,     0,     0,
       0,     0,   206,   122,   128,    74,     0,    77,    73,   115,
     117,     0,   118,    14,    16,     0,    17,    92,     0,     0,
     150,   149,   153,   151,   152,    34,   116,    15,     0,    94,
       0,    72,   100,     0,     0,    86,     0,   100,     0,    91,
       0,    87,    93,    88
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -391,   126,  -155,    59,  -161,   471,  -136,   591,  -317,   219,
    -146,  -157,   -63,  -128,   -59,   300,   -23,   -87,    -5,    26,
     -97,   542,   441,  -391,   -57,  -391,  -237,  -189,   199,   304,
     -28,  -391,   221,   346,  -391,  -391,  -391,   -48,  -103,   429,
     530,  -391,   520,  -391,   560,   478,  -390,   309,   590,  -325,
     365,  -391,  -331,    60,  -391,  -391,   566,  -391,  -391,   559,
    -391,   585,  -391,  -391,     3,  -391,  -391,    15,  -391,  -391,
       0,  -391,   660,  -391,  -391,    -2,   106,   231,  -391,   720,
    -391,  -391,   555,  -391,  -391
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   177,   178,    73,   182,   153,   147,    61,   241,   242,
     154,   170,   155,   124,    58,   225,   267,   168,    54,   205,
     206,   220,   317,   125,   126,   127,   310,   311,   372,   373,
     128,   165,   465,   333,   334,   129,   130,   131,   132,   262,
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
     156,   235,   255,   249,   211,    49,   270,   166,   313,   314,
     272,   331,   229,    63,    64,   439,    69,    48,   145,   403,
      75,    76,   398,     1,   232,   146,   164,    80,    67,   302,
     304,   305,    67,   319,   245,   166,   268,    48,   203,   204,
     456,    55,   248,    62,    55,    55,    50,    55,   244,   146,
     245,    55,    55,   257,   258,   259,   149,   158,   329,   264,
     335,   427,   173,   434,   433,   187,   209,   210,   150,   159,
     265,    52,   212,   213,   174,   214,   215,   216,   217,   218,
     183,   184,    71,    74,   331,    53,    77,   253,    56,   156,
     156,   443,    57,   312,   312,   312,   230,   231,   203,   204,
     171,   171,    72,   340,   156,   156,   151,    65,    56,    56,
      81,   365,    60,    57,   152,   284,   167,   457,    53,   312,
     175,   285,   179,   352,   357,    85,   261,   273,   188,   189,
     285,   190,   191,   192,    38,    39,    82,    83,    84,   196,
     197,   198,    90,   149,   312,   367,   312,    78,   368,   374,
     376,   278,   158,   377,   379,   150,   359,   279,   280,   164,
     281,   164,   285,   360,   159,   282,   301,   173,   166,   285,
     166,   415,   203,   204,   417,   418,    49,   285,   315,   174,
     285,   285,    25,    26,    27,   222,   223,   224,    28,    29,
      30,    31,    32,   406,    33,    34,    35,   312,   342,   353,
     420,   141,   435,   273,   343,   344,   285,   345,   285,    36,
     266,    56,    35,    38,    39,    57,   303,   169,   306,    56,
     332,   -30,    89,   261,   431,   -30,   449,   339,    56,   295,
     296,   381,    57,   142,   430,   421,    56,   199,   320,   321,
      57,   143,   164,   429,    60,   347,   348,   349,   350,   351,
     428,   166,   307,   308,   199,   324,   325,    65,   412,   407,
     408,   409,   410,   164,    86,    87,    88,    91,    92,   422,
     324,   325,   166,   295,   354,   370,   371,    36,   180,   425,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   320,   321,   332,   385,   176,   436,    36,   437,   286,
     287,   194,   195,   219,   233,   234,   240,   438,   243,   247,
     448,   251,   312,   316,   228,    56,   254,   164,   171,   263,
     152,   156,   156,   269,   273,   274,   166,   388,   389,   390,
     391,   275,   322,   164,   283,   164,   375,   411,   288,   289,
     378,   291,   166,   292,   166,   293,   294,   424,   461,   466,
     298,   299,   300,   327,   470,   326,   362,   244,    55,    55,
      55,    55,    94,    95,   160,   328,   161,   330,   338,   162,
      99,   100,   101,   102,   336,   341,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   346,   356,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   355,
     358,   199,   200,   201,   202,   361,    94,    95,   160,   363,
     161,   203,   204,   162,    99,   100,   101,   102,   366,   369,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   380,   384,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   387,   386,    93,   416,   392,   393,   394,
     395,   396,   399,   404,   163,    94,    95,    96,   405,    97,
     419,   426,    98,    99,   100,   101,   102,   414,   432,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   440,   226,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    94,    95,    96,   441,    97,   444,   445,    98,
      99,   100,   101,   102,   447,   450,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   451,   228,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    94,
      95,    96,   452,    97,   453,   454,    98,    99,   100,   101,
     102,   455,   321,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   458,   309,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    94,    95,    96,   325,
      97,   459,   460,    98,    99,   100,   101,   102,   462,   463,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   464,   400,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    94,    95,    96,   467,    97,   469,   471,
      98,    99,   100,   101,   102,   472,   252,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   446,
     402,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      94,    95,    96,   144,    97,   397,   221,    98,    99,   100,
     101,   102,   318,   468,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   413,   383,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    94,    95,   160,
     337,   161,   473,   260,   162,    99,   100,   101,   102,   276,
     239,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   323,   227,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   401,   256,   442,   200,   201,   202,
      94,    95,   160,   271,   161,   250,   186,   162,    99,   100,
     101,   102,    51,   290,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,     0,   115,   116,
     117,   118,   119,   120,   121,   122,   123,     0,     0,     0,
       0,   201,   202,    94,    95,   160,     0,   161,     0,     0,
     162,    99,   100,   101,   102,     0,     0,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
       0,   115,   116,   117,   118,   119,   120,   121,   122,   123,
       0,     0,    94,    95,   160,   202,   161,   382,     0,   162,
      99,   100,   101,   102,     0,     0,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
     115,   116,   117,   118,   119,   120,   121,   122,   123,     0,
      94,    95,   160,   364,   161,     0,     0,   162,    99,   100,
     101,   102,     0,     0,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,     0,   115,   116,
     117,   118,   119,   120,   121,   122,   123,     0,    94,    95,
     160,   364,   161,   423,     0,   162,    99,   100,   101,   102,
       0,     0,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,     0,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    94,    95,   160,     0,   161,
       0,     0,   162,    99,   100,   101,   102,     0,     0,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,     0,   115,   116,   117,   118,   119,   120,   121,
     122,   123,     0,    25,    26,    27,     0,     0,     0,    28,
      29,    30,    31,    32,     0,    33,    34,    35,    36,     0,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47
};

static const yytype_int16 yycheck[] =
{
      97,    98,    30,    26,    32,    64,    69,    30,    65,    32,
      69,   139,   158,   149,   101,    15,   173,    65,   207,   208,
     175,   258,   125,    28,    29,   415,    31,     3,     3,   360,
      35,    36,   357,     3,   131,    10,    93,    39,    66,   200,
     201,   202,    70,   232,   147,    93,    60,     3,    62,    63,
     440,    25,     3,    27,    28,    29,     3,    31,    12,    10,
     163,    35,    36,   160,   161,   162,    63,    64,   257,   166,
     259,   388,    69,   404,   399,    77,    99,   100,    63,    64,
     167,     0,   105,   106,    69,   108,   109,   110,   111,   112,
      66,    67,    32,    34,   331,     9,    37,   156,     5,   158,
     159,   418,     9,   206,   207,   208,   129,   130,    62,    63,
     173,   174,     8,   268,   173,   174,     3,     3,     5,     5,
       3,   310,     3,     9,    11,     4,    66,   444,     9,   232,
      70,    10,    73,     4,     4,     3,   164,     8,    79,    80,
      10,    82,    83,    84,    69,    70,    40,    41,    42,    90,
      91,    92,    46,   150,   257,    16,   259,     9,    19,   320,
     321,    50,   159,   324,   325,   150,     4,    56,    57,   226,
      59,   228,    10,     4,   159,    64,   199,   174,   226,    10,
     228,     4,    62,    63,     4,     4,   186,    10,   211,   174,
      10,    10,    50,    51,    52,     5,     6,     7,    56,    57,
      58,    59,    60,   364,    62,    63,    64,   310,    50,     4,
       4,     4,     4,     8,    56,    57,    10,    59,    10,    65,
       4,     5,    64,    69,    70,     9,   200,     3,   202,     5,
     258,     5,     3,   261,   391,     9,   425,     4,     5,     3,
       4,   328,     9,    50,   390,     4,     5,    52,    53,    54,
       9,     4,   309,   389,     3,   278,   279,   280,   281,   282,
     388,   309,   203,   204,    52,    53,    54,     3,   371,   366,
     367,   368,   369,   330,    43,    44,    45,    46,    47,   382,
      53,    54,   330,     3,     4,    25,    26,    65,     3,   386,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    53,    54,   331,   332,     8,   409,    65,   411,   183,
     184,     4,     4,    39,     4,     3,     3,   414,     4,     4,
     423,     3,   425,    40,     3,     5,     4,   384,   391,     4,
      11,   390,   391,     4,     8,     4,   384,   342,   343,   344,
     345,     3,     3,   400,     4,   402,   320,   370,     4,     4,
     324,     4,   400,     4,   402,     4,     4,   385,   455,   462,
       4,     4,     4,    10,   467,     4,   307,    12,   342,   343,
     344,   345,    13,    14,    15,    58,    17,     3,     3,    20,
      21,    22,    23,    24,     4,     4,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,     4,     4,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    41,
      10,    52,    53,    54,    55,    10,    13,    14,    15,     4,
      17,    62,    63,    20,    21,    22,    23,    24,    16,    16,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,     4,     3,    41,    42,    43,    44,    45,    46,
      47,    48,    49,     4,    15,     3,    10,     4,     4,     4,
       4,     4,     4,     4,    61,    13,    14,    15,     4,    17,
      10,     3,    20,    21,    22,    23,    24,    16,     4,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,     4,     3,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    13,    14,    15,     3,    17,     4,     3,    20,
      21,    22,    23,    24,     4,     4,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,     4,     3,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    13,
      14,    15,     4,    17,     4,     4,    20,    21,    22,    23,
      24,    16,    54,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,     3,     3,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    13,    14,    15,    54,
      17,     4,     4,    20,    21,    22,    23,    24,    19,     3,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,     3,     3,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    13,    14,    15,    19,    17,     4,     4,
      20,    21,    22,    23,    24,     4,   155,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   420,
       3,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      13,    14,    15,    62,    17,   355,   114,    20,    21,    22,
      23,    24,   221,   464,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,   372,   331,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    13,    14,    15,
     261,    17,   471,   163,    20,    21,    22,    23,    24,   179,
     140,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,   235,   124,    41,    42,    43,    44,    45,
      46,    47,    48,    49,   359,   159,   417,    53,    54,    55,
      13,    14,    15,   174,    17,   150,    76,    20,    21,    22,
      23,    24,    22,   188,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    55,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    -1,    13,    14,    15,    55,    17,    18,    -1,    20,
      21,    22,    23,    24,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      13,    14,    15,    54,    17,    -1,    -1,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    13,    14,
      15,    54,    17,    18,    -1,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    13,    14,    15,    -1,    17,
      -1,    -1,    20,    21,    22,    23,    24,    -1,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    50,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78
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
      91,   139,    82,     8,     4,     3,   122,   142,    50,    56,
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
       4,     4,   118,    18,   110,   100,     3,    88,    93,    86,
      90,    91,     4,   129,   132,     4,   118,   118,   100,   126,
       4,     3,   127,    88,     4,     3,    89,     4,   118,   107,
       4,     4,     4,     4,     4,    16,   126,    88,     3,     4,
       4,   100,    19,     3,     3,   112,   118,    19,   108,     4,
     118,     4,     4,   112
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
       2,     3,     2,     3,     3,     3,     8,     3,     4,     2,
       1,     8,     4,     9,     5,     3,     2,     1,     1,     1,
       0,     2,     0,     2,     1,     1,     5,     5,     2,     1,
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
#line 1716 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1722 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1728 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1734 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1740 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 39: /* OFFSET_EQ_NAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1746 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 40: /* ALIGN_EQ_NAT  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1752 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 81: /* text_list  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1758 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 82: /* text_list_opt  */
#line 245 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1764 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 83: /* quoted_text  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1770 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* value_type_list  */
#line 246 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1776 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* global_type  */
#line 239 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1782 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 87: /* func_type  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1788 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* func_sig  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* func_sig_result  */
#line 238 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1800 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* memory_sig  */
#line 241 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1806 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* type_use  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1812 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* literal  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1818 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* var  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1824 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* var_list  */
#line 248 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1830 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* bind_var_opt  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1836 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* bind_var  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1842 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* labeling_opt  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1848 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* instr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1854 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* plain_instr  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1860 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* block_instr  */
#line 234 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1866 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* block_sig  */
#line 246 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1872 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* block  */
#line 229 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1878 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* catch_instr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1884 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* catch_instr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1890 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* expr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1896 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* expr1  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1902 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* catch_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1908 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* if_block  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1914 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* if_  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1920 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 118: /* instr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1926 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* expr_list  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1932 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* const_expr  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1938 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* func  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1944 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* func_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1950 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* func_fields_import  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1956 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* func_fields_import1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1962 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 127: /* func_fields_import_result  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1968 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* func_fields_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1974 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 129: /* func_fields_body1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1980 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* func_result_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1986 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* func_body  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1992 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* func_body1  */
#line 237 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1998 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* offset  */
#line 235 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 2004 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* table  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2010 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* table_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2016 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* memory  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2022 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* memory_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2028 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 140: /* global  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2034 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 141: /* global_fields  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* import_desc  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2046 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 144: /* inline_import  */
#line 240 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2052 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* export_desc  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2058 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 147: /* inline_export  */
#line 233 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2064 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 150: /* module_field  */
#line 236 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 2070 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 151: /* module_fields_opt  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2076 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 152: /* module_fields  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2082 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 153: /* module  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2088 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 154: /* inline_module  */
#line 242 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 155: /* script_var_opt  */
#line 247 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 2100 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 156: /* script_module  */
#line 243 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script_module); }
#line 2106 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 157: /* action  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 2112 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 158: /* assertion  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2118 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 159: /* cmd  */
#line 230 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2124 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 160: /* cmd_list  */
#line 231 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2130 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 162: /* const_list  */
#line 232 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2136 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 163: /* script  */
#line 244 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2142 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
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
#line 2441 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 2454 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 277 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2460 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 2478 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 300 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2484 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 301 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].type));
    }
#line 2493 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 307 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2499 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 310 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].type);
      (yyval.global)->mutable_ = false;
    }
#line 2509 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 315 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].type);
      (yyval.global)->mutable_ = true;
    }
#line 2519 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 323 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2525 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 328 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2535 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 333 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].type));
      // Ignore bind_var.
      destroy_string_slice(&(yyvsp[-3].text));
    }
#line 2546 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 342 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2552 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 343 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->result_types.insert((yyval.func_sig)->result_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 2562 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 351 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
#line 2571 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 357 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
#line 2580 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 363 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2590 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 368 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2600 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 375 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2606 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 2619 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 392 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2628 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 396 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2637 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 400 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2646 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 407 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Index;
      (yyval.var).index = (yyvsp[0].u64);
    }
#line 2656 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 412 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 2666 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 419 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2672 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 420 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->push_back((yyvsp[0].var));
    }
#line 2681 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 426 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2687 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 430 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2693 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 34:
#line 434 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2699 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 439 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2705 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 440 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2724 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 456 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2730 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 457 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2747 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 472 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2753 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 473 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2759 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 478 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2767 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 481 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2775 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 484 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2783 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 487 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2791 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 490 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr((yyvsp[0].var));
    }
#line 2799 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 493 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf((yyvsp[0].var));
    }
#line 2807 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 496 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), (yyvsp[0].var));
    }
#line 2815 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 499 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2823 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 502 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall((yyvsp[0].var));
    }
#line 2831 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 505 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect((yyvsp[0].var));
    }
#line 2839 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 508 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal((yyvsp[0].var));
    }
#line 2847 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 511 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal((yyvsp[0].var));
    }
#line 2855 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 514 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal((yyvsp[0].var));
    }
#line 2863 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 517 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal((yyvsp[0].var));
    }
#line 2871 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 520 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal((yyvsp[0].var));
    }
#line 2879 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 523 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2887 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 526 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2895 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 529 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2913 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 542 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2921 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 545 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 2929 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 548 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 2937 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 551 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 2945 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 554 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 2953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 557 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 2961 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 560 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateThrow((yyvsp[0].var));
    }
#line 2969 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 563 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateRethrow((yyvsp[0].var));
    }
#line 2977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 569 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBlock((yyvsp[-2].block));
      (yyval.expr)->block->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block->label, (yyvsp[0].text));
    }
#line 2987 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 574 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoop((yyvsp[-2].block));
      (yyval.expr)->loop->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->loop->label, (yyvsp[0].text));
    }
#line 2997 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 579 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-2].block), nullptr);
      (yyval.expr)->if_.true_->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3007 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 584 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-5].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->if_.true_->label = (yyvsp[-6].text);
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->if_.true_->label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 3018 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 590 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-3].block)->label = (yyvsp[-4].text);
      (yyval.expr) = Expr::CreateTry((yyvsp[-3].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->try_block.label = (yyvsp[-4].text);
      CHECK_END_LABEL((yylsp[0]), (yyvsp[-3].block)->label, (yyvsp[0].text));
    }
#line 3029 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 599 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 3035 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 602 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = (yyvsp[0].block);
      (yyval.block)->sig.insert((yyval.block)->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3045 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 607 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = new Block();
      (yyval.block)->first = (yyvsp[0].expr_list).first;
    }
#line 3054 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 614 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatch((yyvsp[-1].var), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3063 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 618 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateCatchAll((yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-1]), expr);
    }
#line 3072 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 626 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-1].expr_list), &(yyvsp[0].expr_list));
    }
#line 3080 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 632 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 3086 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 636 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 3094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 639 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateBlock((yyvsp[0].block));
      expr->block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3104 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 644 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateLoop((yyvsp[0].block));
      expr->loop->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 3114 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 649 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      if_->if_.true_->label = (yyvsp[-1].text);
    }
#line 3125 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 655 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-2].block)->label = (yyvsp[-3].text);
      Expr* try_ = Expr::CreateTry((yyvsp[-2].block), (yyvsp[0].expr_list).first);
      try_->try_block.label = (yyvsp[-6].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), try_);
    }
#line 3136 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 664 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3144 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 667 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_expr_lists(&(yyvsp[-2].expr_list), &(yyvsp[0].expr_list));
    }
#line 3152 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 673 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->if_.true_;
      true_->sig.insert(true_->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 3165 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 684 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
#line 3174 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 688 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 3183 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 692 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
#line 3192 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 696 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
#line 3201 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 700 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
#line 3210 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 704 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[0].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
#line 3219 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 711 "src/wast-parser.y" /* yacc.c:1646  */
    {
     CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "rethrow");
    }
#line 3227 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 716 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "throw");
    }
#line 3235 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 722 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "try");      
    }
#line 3243 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 728 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3249 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 729 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3260 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 737 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3266 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 738 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3277 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 751 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Except);
      (yyval.module_field)->loc = (yylsp[0]);
      (yyval.module_field)->except = (yyvsp[0].exception);
    }
#line 3287 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 758 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exception) = new Exception();
      (yyval.exception)->name = (yyvsp[-2].text);
      (yyval.exception)->sig = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 3298 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 768 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3314 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 782 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3326 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 789 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3336 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 794 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-2]);
      field->import = (yyvsp[-2].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      field->import->func->decl.has_func_type = true;
      field->import->func->decl.type_var = (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3351 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 804 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3364 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 812 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3377 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 823 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3386 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 831 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3397 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 837 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3409 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 847 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3415 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 848 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3426 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 857 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3435 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 865 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3446 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 871 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3458 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 882 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types.insert((yyval.func)->decl.sig.result_types.begin(),
                                       (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3469 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 891 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
#line 3478 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 898 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->first_expr = (yyvsp[0].expr_list).first;
    }
#line 3487 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 902 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3497 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 907 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].type));
    }
#line 3509 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 919 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3517 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 926 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::ElemSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->elem_segment = new ElemSegment();
      (yyval.module_field)->elem_segment->table_var = (yyvsp[-3].var);
      (yyval.module_field)->elem_segment->offset = (yyvsp[-2].expr_list).first;
      (yyval.module_field)->elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
    }
#line 3531 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 935 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3547 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 949 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3563 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 963 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Table);
      field->loc = (yylsp[0]);
      field->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3574 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 969 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3587 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 977 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3600 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 985 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* table_field = new ModuleField(ModuleFieldType::Table);
      Table* table = table_field->table = new Table();
      table->elem_limits.initial = (yyvsp[-1].vars)->size();
      table->elem_limits.max = (yyvsp[-1].vars)->size();
      table->elem_limits.has_max = true;
      ModuleField* elem_field = new ModuleField(ModuleFieldType::ElemSegment);
      elem_field->loc = (yylsp[-2]);
      ElemSegment* elem_segment = elem_field->elem_segment = new ElemSegment();
      elem_segment->offset = Expr::CreateConst(Const(Const::I32(), 0));
      elem_segment->offset->loc = (yylsp[-2]);
      elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
      (yyval.module_fields).first = table_field;
      (yyval.module_fields).last = table_field->next = elem_field;
    }
#line 3621 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1004 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::DataSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->data_segment = new DataSegment();
      (yyval.module_field)->data_segment->memory_var = (yyvsp[-3].var);
      (yyval.module_field)->data_segment->offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.module_field)->data_segment->data, &(yyval.module_field)->data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3635 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1013 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3651 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1027 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3667 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1041 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Memory);
      field->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3677 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1046 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3690 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1054 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3703 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1062 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* data_field = new ModuleField(ModuleFieldType::DataSegment);
      data_field->loc = (yylsp[-2]);
      DataSegment* data_segment = data_field->data_segment = new DataSegment();
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
#line 3728 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1085 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3744 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1099 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Global);
      field->global = (yyvsp[-1].global);
      field->global->init_expr = (yyvsp[0].expr_list).first;
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3755 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1105 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3768 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1113 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3781 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1126 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = (yyvsp[-1].var);
    }
#line 3794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1134 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3807 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1142 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = (yyvsp[-2].text);
    }
#line 3818 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1148 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = (yyvsp[-2].text);
    }
#line 3829 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1154 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = (yyvsp[-2].text);
    }
#line 3840 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1160 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Except;
      (yyval.import)->except = (yyvsp[0].exception);
    }
#line 3850 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1168 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Import);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->import = (yyvsp[-1].import);
      (yyval.module_field)->import->module_name = (yyvsp[-3].text);
      (yyval.module_field)->import->field_name = (yyvsp[-2].text);
    }
#line 3862 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1178 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
#line 3872 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1186 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3882 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1191 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1196 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3902 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1201 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3912 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1206 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Except;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3922 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1213 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Export);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->export_ = (yyvsp[-1].export_);
      (yyval.module_field)->export_->name = (yyvsp[-2].text);
    }
#line 3933 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1222 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = (yyvsp[-1].text);
    }
#line 3942 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1232 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3954 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1239 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->name = (yyvsp[-2].text);
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3967 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1250 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Start);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->start = (yyvsp[-1].var);
    }
#line 3977 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1258 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3983 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1263 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3989 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1264 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3995 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1265 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4001 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1266 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4007 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 176:
#line 1267 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4013 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 177:
#line 1268 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 4019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1272 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 4025 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1277 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new Module();
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4035 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1282 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 4045 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1290 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4067 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1317 "src/wast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Index;
      (yyval.var).index = kInvalidIndex;
    }
#line 4077 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1322 "src/wast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 4087 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1330 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Text;
      (yyval.script_module)->text = (yyvsp[-1].module);
      (yyval.script_module)->text->name = (yyvsp[-2].text);
      (yyval.script_module)->text->loc = (yylsp[-3]);

      // Resolve func type variables where the signature was not specified
      // explicitly.
      for (Func* func: (yyvsp[-1].module)->funcs) {
        if (decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          FuncType* func_type =
              get_func_type_by_var((yyvsp[-1].module), &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
          }
        }
      }
    }
#line 4112 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1350 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Binary;
      (yyval.script_module)->binary.name = (yyvsp[-3].text);
      (yyval.script_module)->binary.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->binary.data, &(yyval.script_module)->binary.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4125 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1358 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule();
      (yyval.script_module)->type = ScriptModule::Type::Quoted;
      (yyval.script_module)->quoted.name = (yyvsp[-3].text);
      (yyval.script_module)->quoted.loc = (yylsp[-4]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.script_module)->quoted.data, &(yyval.script_module)->quoted.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 4138 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1369 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-4]);
      (yyval.action)->module_var = (yyvsp[-3].var);
      (yyval.action)->type = ActionType::Invoke;
      (yyval.action)->name = (yyvsp[-2].text);
      (yyval.action)->invoke = new ActionInvoke();
      (yyval.action)->invoke->args = std::move(*(yyvsp[-1].consts));
      delete (yyvsp[-1].consts);
    }
#line 4153 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1379 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = (yyvsp[-2].var);
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = (yyvsp[-1].text);
    }
#line 4165 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 191:
#line 1389 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertMalformed;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
#line 4176 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 192:
#line 1395 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertInvalid;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 4187 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 193:
#line 1401 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUnlinkable;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
#line 4198 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 194:
#line 1407 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUninstantiable;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].script_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
#line 4209 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 195:
#line 1413 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturn;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
#line 4220 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 196:
#line 1419 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnCanonicalNan;
      (yyval.command)->assert_return_canonical_nan.action = (yyvsp[-1].action);
    }
#line 4230 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 197:
#line 1424 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnArithmeticNan;
      (yyval.command)->assert_return_arithmetic_nan.action = (yyvsp[-1].action);
    }
#line 4240 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 198:
#line 1429 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertTrap;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4251 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 199:
#line 1435 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertExhaustion;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 4262 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 200:
#line 1444 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Action;
      (yyval.command)->action = (yyvsp[0].action);
    }
#line 4272 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 202:
#line 1450 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Module;
      (yyval.command)->module = (yyvsp[0].module);
    }
#line 4282 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 203:
#line 1455 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Register;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
#line 4294 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 204:
#line 1464 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4303 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 205:
#line 1468 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4312 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 206:
#line 1475 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4327 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 207:
#line 1487 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4333 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 208:
#line 1488 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
#line 4342 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 209:
#line 1495 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4350 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 210:
#line 1498 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4415 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 211:
#line 1558 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      Command* command = new Command();
      command->type = CommandType::Module;
      command->module = (yyvsp[0].module);
      (yyval.script)->commands.emplace_back(command);
    }
#line 4427 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 212:
#line 1570 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4433 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4437 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1573 "src/wast-parser.y" /* yacc.c:1906  */


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
  if (decl_has_func_type(decl))
    return;

  int sig_index = get_func_type_index_by_decl(module, decl);
  if (sig_index == -1) {
    append_implicit_func_type(loc, module, &decl->sig);
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
