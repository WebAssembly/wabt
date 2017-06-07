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

#define YYMALLOC(size) new char [size]
#define YYFREE(p) delete [] (p)

#define USE_NATURAL_ALIGNMENT (~0)

namespace wabt {

static ExprList join_exprs1(Location* loc, Expr* expr1);
static ExprList join_exprs2(Location* loc, ExprList* expr1, Expr* expr2);

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


#line 199 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:339  */

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
    WABT_TOKEN_TYPE_CALL = 279,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 280,
    WABT_TOKEN_TYPE_RETURN = 281,
    WABT_TOKEN_TYPE_GET_LOCAL = 282,
    WABT_TOKEN_TYPE_SET_LOCAL = 283,
    WABT_TOKEN_TYPE_TEE_LOCAL = 284,
    WABT_TOKEN_TYPE_GET_GLOBAL = 285,
    WABT_TOKEN_TYPE_SET_GLOBAL = 286,
    WABT_TOKEN_TYPE_LOAD = 287,
    WABT_TOKEN_TYPE_STORE = 288,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 289,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 290,
    WABT_TOKEN_TYPE_CONST = 291,
    WABT_TOKEN_TYPE_UNARY = 292,
    WABT_TOKEN_TYPE_BINARY = 293,
    WABT_TOKEN_TYPE_COMPARE = 294,
    WABT_TOKEN_TYPE_CONVERT = 295,
    WABT_TOKEN_TYPE_SELECT = 296,
    WABT_TOKEN_TYPE_UNREACHABLE = 297,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 298,
    WABT_TOKEN_TYPE_GROW_MEMORY = 299,
    WABT_TOKEN_TYPE_FUNC = 300,
    WABT_TOKEN_TYPE_START = 301,
    WABT_TOKEN_TYPE_TYPE = 302,
    WABT_TOKEN_TYPE_PARAM = 303,
    WABT_TOKEN_TYPE_RESULT = 304,
    WABT_TOKEN_TYPE_LOCAL = 305,
    WABT_TOKEN_TYPE_GLOBAL = 306,
    WABT_TOKEN_TYPE_MODULE = 307,
    WABT_TOKEN_TYPE_TABLE = 308,
    WABT_TOKEN_TYPE_ELEM = 309,
    WABT_TOKEN_TYPE_MEMORY = 310,
    WABT_TOKEN_TYPE_DATA = 311,
    WABT_TOKEN_TYPE_OFFSET = 312,
    WABT_TOKEN_TYPE_IMPORT = 313,
    WABT_TOKEN_TYPE_EXPORT = 314,
    WABT_TOKEN_TYPE_REGISTER = 315,
    WABT_TOKEN_TYPE_INVOKE = 316,
    WABT_TOKEN_TYPE_GET = 317,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 318,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 319,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 320,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 321,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 322,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 323,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 324,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 325,
    WABT_TOKEN_TYPE_LOW = 326
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

#line 344 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */

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
#define YYFINAL  49
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   823

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  74
/* YYNRULES -- Number of rules.  */
#define YYNRULES  190
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  419

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

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
      65,    66,    67,    68,    69,    70,    71
};

#if WABT_WAST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   241,   241,   247,   257,   258,   262,   280,   281,   287,
     290,   295,   302,   305,   306,   311,   318,   326,   332,   338,
     343,   350,   356,   367,   371,   375,   382,   387,   394,   395,
     401,   402,   405,   409,   410,   414,   415,   425,   426,   437,
     438,   439,   442,   445,   448,   451,   454,   457,   460,   463,
     466,   469,   472,   475,   478,   481,   484,   487,   490,   493,
     506,   509,   512,   515,   518,   521,   526,   531,   536,   541,
     549,   552,   557,   564,   568,   571,   576,   581,   589,   597,
     600,   604,   608,   612,   616,   620,   627,   628,   636,   637,
     645,   650,   664,   671,   676,   686,   694,   705,   712,   713,
     718,   724,   734,   741,   742,   747,   753,   763,   770,   774,
     779,   791,   794,   798,   807,   821,   835,   841,   849,   857,
     876,   885,   899,   913,   918,   926,   934,   957,   971,   977,
     985,   998,  1006,  1014,  1020,  1026,  1035,  1045,  1053,  1058,
    1063,  1068,  1075,  1084,  1094,  1101,  1112,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,  1133,  1134,  1138,
    1143,  1151,  1171,  1182,  1202,  1209,  1214,  1222,  1232,  1242,
    1248,  1254,  1260,  1266,  1272,  1277,  1282,  1288,  1297,  1302,
    1303,  1308,  1317,  1321,  1328,  1340,  1341,  1348,  1351,  1411,
    1423
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
  "CALL", "CALL_INDIRECT", "RETURN", "GET_LOCAL", "SET_LOCAL", "TEE_LOCAL",
  "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE", "OFFSET_EQ_NAT",
  "ALIGN_EQ_NAT", "CONST", "UNARY", "BINARY", "COMPARE", "CONVERT",
  "SELECT", "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY", "FUNC",
  "START", "TYPE", "PARAM", "RESULT", "LOCAL", "GLOBAL", "MODULE", "TABLE",
  "ELEM", "MEMORY", "DATA", "OFFSET", "IMPORT", "EXPORT", "REGISTER",
  "INVOKE", "GET", "ASSERT_MALFORMED", "ASSERT_INVALID",
  "ASSERT_UNLINKABLE", "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
  "ASSERT_RETURN_ARITHMETIC_NAN", "ASSERT_TRAP", "ASSERT_EXHAUSTION",
  "LOW", "$accept", "text_list", "text_list_opt", "quoted_text",
  "value_type_list", "elem_type", "global_type", "func_type", "func_sig",
  "table_sig", "memory_sig", "limits", "type_use", "nat", "literal", "var",
  "var_list", "bind_var_opt", "bind_var", "labeling_opt", "offset_opt",
  "align_opt", "instr", "plain_instr", "block_instr", "block_sig", "block",
  "expr", "expr1", "if_block", "if_", "instr_list", "expr_list",
  "const_expr", "func", "func_fields", "func_fields_import",
  "func_fields_import1", "func_fields_body", "func_fields_body1",
  "func_body", "func_body1", "offset", "elem", "table", "table_fields",
  "data", "memory", "memory_fields", "global", "global_fields",
  "import_desc", "import", "inline_import", "export_desc", "export",
  "inline_export", "type_def", "start", "module_field",
  "module_fields_opt", "module_fields", "raw_module", "module",
  "inline_module", "script_var_opt", "action", "assertion", "cmd",
  "cmd_list", "const", "const_list", "script", "script_start", YY_NULLPTR
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
     325,   326
};
# endif

#define YYPACT_NINF -339

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-339)))

#define YYTABLE_NINF -30

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      43,   753,  -339,  -339,  -339,  -339,  -339,  -339,  -339,  -339,
    -339,  -339,  -339,   122,  -339,  -339,  -339,  -339,  -339,  -339,
     140,  -339,   153,   152,   232,    67,   152,   152,   152,   157,
     152,   157,   159,   159,   159,   160,   160,   184,   184,   184,
     226,   226,   226,   237,   226,   148,  -339,    34,  -339,  -339,
    -339,   422,  -339,  -339,  -339,  -339,   215,   193,   249,   242,
      56,   197,    66,   377,   265,  -339,  -339,   241,   265,   262,
    -339,   159,   268,   160,  -339,   159,   159,   221,   159,   159,
     159,    17,  -339,   270,   271,     6,   159,   159,   159,   337,
    -339,  -339,   152,   152,   152,   232,   232,  -339,   232,   232,
    -339,   232,   232,   232,   232,   232,   244,   244,    84,  -339,
    -339,  -339,  -339,  -339,  -339,  -339,  -339,   454,   486,  -339,
    -339,  -339,  -339,   275,  -339,  -339,  -339,  -339,   277,   422,
    -339,   278,  -339,   281,    25,  -339,   486,   282,   120,    56,
    -339,   235,   283,   122,    55,  -339,   279,  -339,   280,   284,
     286,   284,    66,   152,   152,   152,   486,   289,   290,  -339,
     130,    68,  -339,  -339,   291,   284,   241,   262,   285,   292,
     294,   103,   297,   304,  -339,   305,   306,   309,   311,   206,
    -339,  -339,   312,   317,   319,   232,   152,  -339,   152,   159,
     159,  -339,   518,   518,   518,  -339,  -339,   232,  -339,  -339,
    -339,  -339,  -339,  -339,  -339,  -339,   263,   263,  -339,  -339,
    -339,  -339,   582,  -339,   752,  -339,  -339,   149,   321,  -339,
    -339,  -339,   210,   322,  -339,   315,  -339,  -339,  -339,   316,
    -339,  -339,  -339,  -339,  -339,   273,  -339,  -339,  -339,  -339,
    -339,   518,   326,   518,   327,   289,  -339,  -339,   175,  -339,
    -339,   262,  -339,  -339,  -339,   328,  -339,   104,   329,   232,
     232,   232,   232,  -339,  -339,   261,  -339,  -339,  -339,  -339,
     298,  -339,  -339,  -339,  -339,  -339,   331,   102,   320,   161,
     164,   330,   159,   332,   687,   518,   323,  -339,   238,   325,
     246,  -339,  -339,  -339,   152,  -339,   218,  -339,  -339,  -339,
    -339,   333,  -339,  -339,   654,   326,   335,  -339,  -339,  -339,
    -339,  -339,  -339,   338,  -339,   152,   152,   152,   152,  -339,
     339,   340,   341,   349,  -339,    84,  -339,   454,  -339,   351,
     550,   550,   352,   367,  -339,  -339,  -339,   152,   152,   152,
     152,   166,   362,   168,   204,   208,  -339,   219,   486,  -339,
     720,   289,  -339,   379,   120,   284,   284,  -339,  -339,  -339,
    -339,   384,  -339,   454,   620,  -339,  -339,   550,  -339,   216,
    -339,  -339,   486,  -339,   321,   385,  -339,   380,  -339,  -339,
     389,   486,  -339,   214,   407,   408,   418,   419,   420,  -339,
    -339,  -339,  -339,   410,  -339,   321,   378,   425,   426,  -339,
    -339,  -339,  -339,  -339,   152,  -339,  -339,   412,   429,  -339,
     217,   486,   414,  -339,   434,   486,  -339,   436,  -339
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     187,     0,   151,   152,   149,   153,   150,   148,   155,   156,
     147,   154,   159,   164,   163,   180,   189,   178,   179,   182,
     188,   190,     0,    30,     0,     0,    30,    30,    30,     0,
      30,     0,     0,     0,     0,   165,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,   183,     1,
      32,    86,    31,    22,    27,    26,     0,     0,     0,     0,
       0,   157,     0,     0,     0,   112,    28,     0,     0,     4,
       6,     0,     0,   165,   166,     0,     0,     0,     0,     0,
       0,     0,   185,     0,     0,     0,     0,     0,     0,     0,
      43,    44,    33,    33,    33,     0,     0,    28,     0,     0,
      49,     0,     0,     0,     0,     0,    35,    35,     0,    60,
      61,    62,    63,    45,    42,    64,    65,    86,    86,    39,
      40,    41,   108,     0,    93,   102,   103,   107,    98,    86,
     146,    13,   144,     0,     0,    10,    86,     0,     0,     0,
       2,     0,     0,   158,     0,     9,     0,   116,     0,    19,
       0,     0,     0,    33,    33,    33,    86,    88,     0,    28,
       0,     0,   123,    18,     0,     0,     0,     4,     5,     0,
       0,     0,     0,     0,   185,     0,     0,     0,     0,     0,
     174,   175,     0,     0,     0,     0,     7,     7,     7,     0,
       0,    34,    86,    86,    86,    46,    47,     0,    50,    51,
      52,    53,    54,    55,    56,    36,    37,    37,    23,    24,
      25,    59,     0,    92,     0,    87,    91,     0,    98,    95,
      97,    96,     0,     0,   145,     0,    90,   128,   127,     0,
     129,   130,   162,     3,   161,     0,    17,    20,   115,   117,
     118,    86,     0,    86,     0,    88,    74,    73,     0,   114,
      29,     4,   122,   124,   125,     0,   121,     0,     0,     0,
       0,     0,     0,   142,   181,     0,   168,   169,   170,   171,
       0,   173,   186,   172,   176,   177,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,     0,    72,     0,     0,
      48,    38,    57,    58,     7,     7,     0,    94,     7,     7,
      12,     0,    28,    75,     0,     0,     0,    77,    79,    76,
     111,    89,   113,     0,   120,    30,    30,    30,    30,   136,
       0,     0,     0,     0,   167,     0,    21,    86,     8,     0,
      86,    86,     0,     0,   143,     7,    71,    33,    33,    33,
      33,     0,     0,     0,     0,     0,    11,     0,    86,    78,
       0,    85,   126,    13,     0,     0,     0,   138,   141,   139,
     140,     0,   105,    86,     0,   104,   109,    86,   137,     0,
      66,    68,    86,    67,    98,     0,    99,    14,    16,   119,
       0,    86,    84,     0,     0,     0,     0,     0,     0,   184,
     106,   110,    70,     0,   100,    98,     0,    81,     0,   132,
     131,   135,   133,   134,    33,   101,     7,     0,    83,    69,
       0,    86,     0,    15,     0,    86,    80,     0,    82
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -339,   368,  -136,   -26,  -154,   293,  -129,   397,   117,  -138,
    -145,   -56,  -123,   -44,   147,    87,   -87,     2,   -11,   -90,
     366,   295,  -339,   -62,  -339,  -220,  -168,   -29,  -339,   183,
    -339,  -112,   259,   364,  -339,   376,   318,  -338,   417,  -289,
     207,  -280,    24,  -339,  -339,   400,  -339,  -339,   402,  -339,
     427,  -339,  -339,   -20,  -339,  -339,   -19,  -339,  -339,     8,
    -339,   508,    99,  -339,  -339,    20,   191,  -339,   564,  -339,
    -339,   411,  -339,  -339
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   168,   169,    71,   277,   146,   136,    58,   223,   147,
     162,   148,   117,    55,   211,   250,   160,    51,   191,   192,
     206,   292,   118,   119,   120,   285,   286,   121,   158,   307,
     308,   122,   246,   227,     2,   123,   219,   220,   124,   125,
     126,   127,    66,     3,     4,   150,     5,     6,   164,     7,
     137,   258,     8,   128,   172,     9,   129,    10,    11,    12,
     142,    13,    14,    15,    16,    75,    17,    18,    19,    20,
     272,   179,    21,    22
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,   157,    65,   193,   194,   218,   215,    72,    73,   230,
     197,   163,    52,   239,    59,    52,    52,    52,   149,    52,
     253,    46,   305,   149,   226,   288,   289,   157,    60,    61,
      62,   255,    67,   279,   280,    65,   394,   225,   362,    65,
     138,   139,   151,   152,   226,   170,     1,   165,   166,   174,
     175,   366,   176,   177,   178,    69,    76,   405,    27,   134,
     182,   183,   184,   241,   242,   243,   135,    35,    36,   144,
      57,    53,   248,   303,   390,   309,    50,   145,    35,    36,
     287,   287,   287,   189,   190,   305,    27,   391,   159,   208,
     209,   210,   167,   173,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,   237,   327,   149,   149,   163,
     163,    56,   328,   189,   190,   313,    64,   336,    68,   138,
     139,   149,   149,   229,   251,    45,   189,   190,   245,   287,
     135,   287,   151,   152,   249,    53,    78,    79,    80,    54,
     341,   343,    86,    47,   344,   345,   165,   166,   259,   315,
     157,    46,   157,    49,   260,   316,   261,   317,   262,   318,
      63,    50,    53,   282,   283,   330,    54,    70,   331,    74,
     374,   328,   376,   287,   328,   278,   328,   281,   328,   312,
      53,   369,   195,   196,    54,   198,   199,    77,   200,   201,
     202,   203,   204,    23,    24,    25,   185,   294,   295,    26,
      45,    28,    29,    30,    31,   140,    32,    33,   377,   270,
     271,   388,   378,   306,   328,   347,   245,   387,   328,   130,
     392,   413,   157,   379,    53,   386,   328,   328,    54,    81,
     385,    82,    83,    84,    87,    88,   380,    53,   131,   232,
      85,    54,   157,   233,   161,    57,    53,   370,   371,   372,
     373,   -29,   410,   132,   338,   -29,   333,   339,   298,   299,
     393,   185,   298,   299,   270,   324,   294,   295,    63,   398,
     140,   171,   276,    27,   180,   181,   306,   351,   205,   216,
     217,   222,   235,   342,   290,   224,   228,   234,   157,    53,
     238,   145,   214,   233,   247,   252,   256,   257,   291,   414,
     163,   263,   157,   417,    52,    52,    52,    52,   264,   266,
     267,   149,   149,   268,   409,   269,   273,   353,   354,   355,
     356,   274,   382,   275,   296,   301,   300,   302,   225,   304,
     329,   310,   314,   319,   325,   326,   334,   346,   350,   337,
     332,   340,   352,   357,   358,   359,   320,   321,   322,   323,
      90,    91,   153,   360,   154,   363,   367,   155,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   368,   375,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   383,   396,   185,   186,   187,   188,   389,   395,
      90,    91,   153,   397,   154,   189,   190,   155,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   399,   400,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   401,   402,   403,    89,   404,   406,   407,   141,
     408,   411,   412,   415,   156,    90,    91,    92,   416,    93,
     418,   236,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   133,   212,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    90,    91,    92,
     384,    93,   361,   207,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   349,   214,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    90,
      91,    92,   293,    93,   311,   221,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     244,   284,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    90,    91,    92,   213,    93,   297,   365,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   240,   364,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    90,    91,    92,   231,    93,   254,   143,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    48,   265,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    90,    91,   153,     0,   154,
       0,     0,   155,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,     0,     0,   108,   109,
     110,   111,   112,   113,   114,   115,   116,     0,     0,     0,
     186,   187,   188,    90,    91,   153,     0,   154,     0,     0,
     155,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,     0,     0,   108,   109,   110,   111,
     112,   113,   114,   115,   116,     0,     0,    90,    91,   153,
     188,   154,   348,     0,   155,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,     0,     0,
     108,   109,   110,   111,   112,   113,   114,   115,   116,     0,
      90,    91,   153,   335,   154,     0,     0,   155,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,     0,    90,    91,   153,   335,   154,   381,     0,
     155,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,     0,     0,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    90,    91,   153,     0,   154,
       0,     0,   155,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,     0,     0,   108,   109,
     110,   111,   112,   113,   114,   115,   116,     0,    23,    24,
      25,     0,     0,     0,    26,    27,    28,    29,    30,    31,
       0,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44
};

static const yytype_int16 yycheck[] =
{
      29,    63,    31,    93,    94,   128,   118,    33,    34,   138,
      97,    67,    23,   151,    25,    26,    27,    28,    62,    30,
     165,    13,   242,    67,   136,   193,   194,    89,    26,    27,
      28,   167,    30,   187,   188,    64,   374,    12,   327,    68,
      60,    60,    62,    62,   156,    71,     3,    67,    67,    75,
      76,   331,    78,    79,    80,    31,    36,   395,    52,     3,
      86,    87,    88,   153,   154,   155,    10,    61,    62,     3,
       3,     5,   159,   241,   363,   243,     9,    11,    61,    62,
     192,   193,   194,    58,    59,   305,    52,   367,    64,     5,
       6,     7,    68,    73,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,   149,     4,   151,   152,   165,
     166,    24,    10,    58,    59,   251,    29,   285,    31,   139,
     139,   165,   166,     3,    56,     3,    58,    59,   157,   241,
      10,   243,   152,   152,     4,     5,    37,    38,    39,     9,
     294,   295,    43,     3,   298,   299,   166,   166,    45,    45,
     212,   143,   214,     0,    51,    51,    53,    53,    55,    55,
       3,     9,     5,   189,   190,     4,     9,     8,     4,     9,
       4,    10,     4,   285,    10,   186,    10,   188,    10,     4,
       5,   335,    95,    96,     9,    98,    99,     3,   101,   102,
     103,   104,   105,    45,    46,    47,    47,    48,    49,    51,
       3,    53,    54,    55,    56,     8,    58,    59,     4,     3,
       4,   356,     4,   242,    10,   302,   245,   355,    10,     4,
       4,     4,   284,     4,     5,   354,    10,    10,     9,     3,
     353,    40,    41,    42,    43,    44,   348,     5,    45,     4,
       3,     9,   304,     8,     3,     3,     5,   337,   338,   339,
     340,     5,   406,     4,    16,     9,   282,    19,    48,    49,
     372,    47,    48,    49,     3,     4,    48,    49,     3,   381,
       8,     3,   185,    52,     4,     4,   305,   306,    34,     4,
       3,     3,     3,   294,   197,     4,     4,     4,   350,     5,
       4,    11,     3,     8,     4,     4,     4,     3,    35,   411,
     356,     4,   364,   415,   315,   316,   317,   318,     4,     4,
       4,   355,   356,     4,   404,     4,     4,   315,   316,   317,
     318,     4,   351,     4,     3,    10,     4,    54,    12,     3,
      10,     4,     4,     4,    36,     4,     4,     4,     3,    16,
      10,    16,     4,     4,     4,     4,   259,   260,   261,   262,
      13,    14,    15,     4,    17,     4,     4,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,     4,    10,    36,    37,    38,    39,    40,    41,    42,
      43,    44,     3,     3,    47,    48,    49,    50,     4,     4,
      13,    14,    15,     4,    17,    58,    59,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,     4,     4,    36,    37,    38,    39,    40,    41,    42,
      43,    44,     4,     4,     4,     3,    16,    49,     3,    61,
       4,    19,     3,    19,    57,    13,    14,    15,     4,    17,
       4,   148,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,     3,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    13,    14,    15,
     353,    17,   325,   107,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,   305,     3,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    13,
      14,    15,   207,    17,   245,   129,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     156,     3,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    13,    14,    15,   117,    17,   218,   330,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,   152,     3,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    13,    14,    15,   139,    17,   166,    61,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    20,   174,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    13,    14,    15,    -1,    17,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      48,    49,    50,    13,    14,    15,    -1,    17,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    -1,    13,    14,    15,
      50,    17,    18,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      13,    14,    15,    49,    17,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    -1,    13,    14,    15,    49,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    13,    14,    15,    -1,    17,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    45,    46,
      47,    -1,    -1,    -1,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   106,   115,   116,   118,   119,   121,   124,   127,
     129,   130,   131,   133,   134,   135,   136,   138,   139,   140,
     141,   144,   145,    45,    46,    47,    51,    52,    53,    54,
      55,    56,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,     3,   131,     3,   140,     0,
       9,    89,    90,     5,     9,    85,    87,     3,    79,    90,
      89,    89,    89,     3,    87,    99,   114,    89,    87,   114,
       8,    75,    75,    75,     9,   137,   137,     3,   134,   134,
     134,     3,   138,   138,   138,     3,   134,   138,   138,     3,
      13,    14,    15,    17,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    84,    94,    95,
      96,    99,   103,   107,   110,   111,   112,   113,   125,   128,
       4,    45,     4,    79,     3,    10,    78,   122,   125,   128,
       8,    73,   132,   133,     3,    11,    77,    81,    83,    85,
     117,   125,   128,    15,    17,    20,    57,    95,   100,   114,
      88,     3,    82,    83,   120,   125,   128,   114,    73,    74,
      75,     3,   126,   137,    75,    75,    75,    75,    75,   143,
       4,     4,    75,    75,    75,    47,    48,    49,    50,    58,
      59,    90,    91,    91,    91,    87,    87,    88,    87,    87,
      87,    87,    87,    87,    87,    34,    92,    92,     5,     6,
       7,    86,     3,   110,     3,   103,     4,     3,    84,   108,
     109,   107,     3,    80,     4,    12,   103,   105,     4,     3,
      78,   122,     4,     8,     4,     3,    77,    85,     4,    81,
     117,    91,    91,    91,   105,    99,   104,     4,    88,     4,
      87,    56,     4,    82,   120,    74,     4,     3,   123,    45,
      51,    53,    55,     4,     4,   143,     4,     4,     4,     4,
       3,     4,   142,     4,     4,     4,    87,    76,    90,    76,
      76,    90,    75,    75,     3,    97,    98,   103,    98,    98,
      87,    35,    93,    93,    48,    49,     3,   108,    48,    49,
       4,    10,    54,    98,     3,    97,    99,   101,   102,    98,
       4,   104,     4,    74,     4,    45,    51,    53,    55,     4,
      87,    87,    87,    87,     4,    36,     4,     4,    10,    10,
       4,     4,    10,    75,     4,    49,    98,    16,    16,    19,
      16,    76,    90,    76,    76,    76,     4,    88,    18,   101,
       3,    99,     4,    89,    89,    89,    89,     4,     4,     4,
       4,    86,   111,     4,     3,   112,   113,     4,     4,    76,
      91,    91,    91,    91,     4,    10,     4,     4,     4,     4,
     103,    18,    99,     3,    80,    84,    78,    81,    82,     4,
     111,   113,     4,   103,   109,     4,     3,     4,   103,     4,
       4,     4,     4,     4,    16,   109,    49,     3,     4,    91,
      76,    19,     3,     4,   103,    19,     4,   103,     4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    72,    73,    73,    74,    74,    75,    76,    76,    77,
      78,    78,    79,    80,    80,    80,    80,    81,    82,    83,
      83,    84,    85,    86,    86,    86,    87,    87,    88,    88,
      89,    89,    90,    91,    91,    92,    92,    93,    93,    94,
      94,    94,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      97,    98,    98,    99,   100,   100,   100,   100,   101,   101,
     102,   102,   102,   102,   102,   102,   103,   103,   104,   104,
     105,   106,   107,   107,   107,   107,   107,   108,   109,   109,
     109,   109,   110,   111,   111,   111,   111,   112,   113,   113,
     113,   114,   114,   115,   115,   116,   117,   117,   117,   117,
     118,   118,   119,   120,   120,   120,   120,   121,   122,   122,
     122,   123,   123,   123,   123,   123,   124,   125,   126,   126,
     126,   126,   127,   128,   129,   129,   130,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   132,   132,   133,
     133,   134,   134,   135,   136,   137,   137,   138,   138,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   140,   140,
     140,   140,   141,   141,   142,   143,   143,   144,   144,   144,
     145
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
       4,     2,     1,     3,     2,     3,     3,     3,     2,     1,
       8,     4,     9,     5,     3,     2,     0,     2,     0,     2,
       1,     5,     2,     1,     3,     2,     2,     1,     0,     4,
       5,     6,     1,     1,     5,     5,     6,     1,     1,     5,
       6,     4,     1,     6,     5,     5,     1,     2,     2,     5,
       6,     5,     5,     1,     2,     2,     4,     5,     2,     2,
       2,     5,     5,     5,     5,     5,     6,     5,     4,     4,
       4,     4,     5,     4,     4,     5,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       2,     5,     5,     1,     1,     0,     1,     6,     5,     5,
       5,     5,     5,     5,     4,     4,     5,     5,     1,     1,
       1,     5,     1,     2,     4,     0,     2,     0,     1,     1,
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
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1628 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 6: /* INT  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1634 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 7: /* FLOAT  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1640 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 8: /* TEXT  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1646 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 9: /* VAR  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1652 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 34: /* OFFSET_EQ_NAT  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1658 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 35: /* ALIGN_EQ_NAT  */
#line 205 "src/wast-parser.y" /* yacc.c:1257  */
      {}
#line 1664 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 73: /* text_list  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1670 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 74: /* text_list_opt  */
#line 225 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_text_list(&((*yyvaluep).text_list)); }
#line 1676 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 75: /* quoted_text  */
#line 206 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1682 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 76: /* value_type_list  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1688 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 78: /* global_type  */
#line 219 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1694 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 79: /* func_type  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1700 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 80: /* func_sig  */
#line 218 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1706 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 82: /* memory_sig  */
#line 221 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1712 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* type_use  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1718 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* literal  */
#line 207 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).literal).text); }
#line 1724 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 87: /* var  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1730 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* var_list  */
#line 228 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1736 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* bind_var_opt  */
#line 206 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1742 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 90: /* bind_var  */
#line 206 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1748 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* labeling_opt  */
#line 206 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_string_slice(&((*yyvaluep).text)); }
#line 1754 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 94: /* instr  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1760 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* plain_instr  */
#line 214 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1766 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 96: /* block_instr  */
#line 214 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1772 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 97: /* block_sig  */
#line 226 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1778 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* block  */
#line 209 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1784 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* expr  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1790 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* expr1  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1796 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* if_block  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1802 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* if_  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1808 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 103: /* instr_list  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1814 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 104: /* expr_list  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1820 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* const_expr  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1826 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* func  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1832 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* func_fields  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1838 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* func_fields_import  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1844 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* func_fields_import1  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1850 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 110: /* func_fields_body  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1856 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 111: /* func_fields_body1  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1862 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 112: /* func_body  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1868 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 113: /* func_body1  */
#line 217 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1874 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* offset  */
#line 215 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_expr_list(((*yyvaluep).expr_list).first); }
#line 1880 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 116: /* table  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1886 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 117: /* table_fields  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* memory  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1898 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* memory_fields  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1904 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 121: /* global  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1910 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 122: /* global_fields  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1916 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 123: /* import_desc  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1922 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* inline_import  */
#line 220 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 1928 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* export_desc  */
#line 213 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 1934 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 128: /* inline_export  */
#line 213 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 1940 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* module_field  */
#line 216 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_module_field_list(&((*yyvaluep).module_fields)); }
#line 1946 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* module_fields_opt  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1952 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* module_fields  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1958 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* raw_module  */
#line 223 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).raw_module); }
#line 1964 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* module  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1970 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* inline_module  */
#line 222 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 1976 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 137: /* script_var_opt  */
#line 227 "src/wast-parser.y" /* yacc.c:1257  */
      { destroy_var(&((*yyvaluep).var)); }
#line 1982 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* action  */
#line 208 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 1988 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* assertion  */
#line 210 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 1994 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 140: /* cmd  */
#line 210 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2000 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 141: /* cmd_list  */
#line 211 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2006 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 143: /* const_list  */
#line 212 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2012 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 144: /* script  */
#line 224 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2018 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
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
#line 241 "src/wast-parser.y" /* yacc.c:1646  */
    {
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).first = (yyval.text_list).last = node;
    }
#line 2317 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 247 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.text_list) = (yyvsp[-1].text_list);
      TextListNode* node = new TextListNode();
      DUPTEXT(node->text, (yyvsp[0].text));
      node->next = nullptr;
      (yyval.text_list).last->next = node;
      (yyval.text_list).last = node;
    }
#line 2330 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 257 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.text_list).first = (yyval.text_list).last = nullptr; }
#line 2336 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 262 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2354 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 280 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2360 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 281 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].type));
    }
#line 2369 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 287 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2375 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 290 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].type);
      (yyval.global)->mutable_ = false;
    }
#line 2385 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 295 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].type);
      (yyval.global)->mutable_ = true;
    }
#line 2395 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 302 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2401 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 305 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2407 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 306 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->param_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 2417 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 311 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->param_types = std::move(*(yyvsp[-5].types));
      delete (yyvsp[-5].types);
      (yyval.func_sig)->result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 2429 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 318 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = new FuncSignature();
      (yyval.func_sig)->result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 2439 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 326 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
#line 2448 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 332 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
#line 2457 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 338 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2467 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 343 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2477 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 350 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2483 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 356 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if (WABT_FAILED(parse_uint64((yyvsp[0].literal).text.start,
                                        (yyvsp[0].literal).text.start + (yyvsp[0].literal).text.length, &(yyval.u64)))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid int " PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].literal).text));
      }
    }
#line 2496 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 367 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2505 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 371 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2514 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 375 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal).type = (yyvsp[0].literal).type;
      DUPTEXT((yyval.literal).text, (yyvsp[0].literal).text);
    }
#line 2523 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 382 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Index;
      (yyval.var).index = (yyvsp[0].u64);
    }
#line 2533 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 387 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var).loc = (yylsp[0]);
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 2543 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 394 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2549 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 395 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->push_back((yyvsp[0].var));
    }
#line 2558 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 401 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2564 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 405 "src/wast-parser.y" /* yacc.c:1646  */
    { DUPTEXT((yyval.text), (yyvsp[0].text)); }
#line 2570 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 409 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.text)); }
#line 2576 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 35:
#line 414 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2582 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 415 "src/wast-parser.y" /* yacc.c:1646  */
    {
    if (WABT_FAILED(parse_int64((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u64),
                                ParseIntType::SignedAndUnsigned))) {
      wast_parser_error(&(yylsp[0]), lexer, parser,
                        "invalid offset \"" PRIstringslice "\"",
                        WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2595 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 425 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2601 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 426 "src/wast-parser.y" /* yacc.c:1646  */
    {
    if (WABT_FAILED(parse_int32((yyvsp[0].text).start, (yyvsp[0].text).start + (yyvsp[0].text).length, &(yyval.u32),
                                ParseIntType::UnsignedOnly))) {
      wast_parser_error(&(yylsp[0]), lexer, parser,
                        "invalid alignment \"" PRIstringslice "\"",
                        WABT_PRINTF_STRING_SLICE_ARG((yyvsp[0].text)));
      }
    }
#line 2614 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 437 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2620 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 438 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = join_exprs1(&(yylsp[0]), (yyvsp[0].expr)); }
#line 2626 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 439 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 2632 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 442 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnreachable();
    }
#line 2640 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 445 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateNop();
    }
#line 2648 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 448 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateDrop();
    }
#line 2656 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 451 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSelect();
    }
#line 2664 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 454 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBr((yyvsp[0].var));
    }
#line 2672 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 457 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrIf((yyvsp[0].var));
    }
#line 2680 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 460 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBrTable((yyvsp[-1].vars), (yyvsp[0].var));
    }
#line 2688 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 463 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateReturn();
    }
#line 2696 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 466 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCall((yyvsp[0].var));
    }
#line 2704 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 469 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCallIndirect((yyvsp[0].var));
    }
#line 2712 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 472 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetLocal((yyvsp[0].var));
    }
#line 2720 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 475 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetLocal((yyvsp[0].var));
    }
#line 2728 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 478 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateTeeLocal((yyvsp[0].var));
    }
#line 2736 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 481 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGetGlobal((yyvsp[0].var));
    }
#line 2744 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 484 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateSetGlobal((yyvsp[0].var));
    }
#line 2752 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 487 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoad((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2760 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 490 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateStore((yyvsp[-2].opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2768 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 493 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 2786 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 506 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateUnary((yyvsp[0].opcode));
    }
#line 2794 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 509 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBinary((yyvsp[0].opcode));
    }
#line 2802 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 512 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCompare((yyvsp[0].opcode));
    }
#line 2810 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 515 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateConvert((yyvsp[0].opcode));
    }
#line 2818 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 518 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateCurrentMemory();
    }
#line 2826 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 521 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateGrowMemory();
    }
#line 2834 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 526 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateBlock((yyvsp[-2].block));
      (yyval.expr)->block->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->block->label, (yyvsp[0].text));
    }
#line 2844 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 531 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateLoop((yyvsp[-2].block));
      (yyval.expr)->loop->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->loop->label, (yyvsp[0].text));
    }
#line 2854 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 536 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-2].block), nullptr);
      (yyval.expr)->if_.true_->label = (yyvsp[-3].text);
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 2864 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 541 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = Expr::CreateIf((yyvsp[-5].block), (yyvsp[-2].expr_list).first);
      (yyval.expr)->if_.true_->label = (yyvsp[-6].text);
      CHECK_END_LABEL((yylsp[-3]), (yyval.expr)->if_.true_->label, (yyvsp[-3].text));
      CHECK_END_LABEL((yylsp[0]), (yyval.expr)->if_.true_->label, (yyvsp[0].text));
    }
#line 2875 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 549 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 2881 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 552 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = (yyvsp[0].block);
      (yyval.block)->sig.insert((yyval.block)->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 2891 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 557 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = new Block();
      (yyval.block)->first = (yyvsp[0].expr_list).first;
    }
#line 2900 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 564 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 2906 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 568 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[0].expr_list), (yyvsp[-1].expr));
    }
#line 2914 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 571 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateBlock((yyvsp[0].block));
      expr->block->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2924 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 576 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateLoop((yyvsp[0].block));
      expr->loop->label = (yyvsp[-1].text);
      (yyval.expr_list) = join_exprs1(&(yylsp[-2]), expr);
    }
#line 2934 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 581 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      if_->if_.true_->label = (yyvsp[-1].text);
    }
#line 2945 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 589 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* if_ = (yyvsp[0].expr_list).last;
      assert(if_->type == ExprType::If);
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->if_.true_;
      true_->sig.insert(true_->sig.end(), (yyvsp[-1].types)->begin(), (yyvsp[-1].types)->end());
      delete (yyvsp[-1].types);
    }
#line 2958 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 80:
#line 600 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs1(&(yylsp[-7]), expr);
    }
#line 2967 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 604 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs1(&(yylsp[-3]), expr);
    }
#line 2976 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 608 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-5].expr_list).first), (yyvsp[-1].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-8]), &(yyvsp[-8].expr_list), expr);
    }
#line 2985 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 612 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-4]), &(yyvsp[-4].expr_list), expr);
    }
#line 2994 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 616 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[-1].expr_list).first), (yyvsp[0].expr_list).first);
      (yyval.expr_list) = join_exprs2(&(yylsp[-2]), &(yyvsp[-2].expr_list), expr);
    }
#line 3003 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 620 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = Expr::CreateIf(new Block((yyvsp[0].expr_list).first), nullptr);
      (yyval.expr_list) = join_exprs2(&(yylsp[-1]), &(yyvsp[-1].expr_list), expr);
    }
#line 3012 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 627 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3018 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 628 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3029 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 636 "src/wast-parser.y" /* yacc.c:1646  */
    { WABT_ZERO_MEMORY((yyval.expr_list)); }
#line 3035 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 637 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list).first = (yyvsp[-1].expr_list).first;
      (yyvsp[-1].expr_list).last->next = (yyvsp[0].expr_list).first;
      (yyval.expr_list).last = (yyvsp[0].expr_list).last ? (yyvsp[0].expr_list).last : (yyvsp[-1].expr_list).last;
      (yyval.expr_list).size = (yyvsp[-1].expr_list).size + (yyvsp[0].expr_list).size;
    }
#line 3046 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 650 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3062 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 664 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = (yyvsp[-1].var);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3074 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 671 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Func);
      field->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3084 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 676 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3099 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 686 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3112 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 96:
#line 694 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3125 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 705 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3134 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 712 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3140 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 713 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->decl.sig.result_types = std::move(*(yyvsp[-1].types));
      delete (yyvsp[-1].types);
    }
#line 3150 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 718 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3161 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 724 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3173 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 734 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3182 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 104:
#line 742 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.result_types = std::move(*(yyvsp[-2].types));
      delete (yyvsp[-2].types);
    }
#line 3192 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 747 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(),
                                      (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3203 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 753 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].type));
    }
#line 3215 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 763 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      reverse_bindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
#line 3224 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 770 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->first_expr = (yyvsp[0].expr_list).first;
    }
#line 3233 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 774 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].types)->begin(), (yyvsp[-2].types)->end());
      delete (yyvsp[-2].types);
    }
#line 3243 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 110:
#line 779 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(string_slice_to_string((yyvsp[-3].text)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      destroy_string_slice(&(yyvsp[-3].text));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].type));
    }
#line 3255 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 791 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3263 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 798 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::ElemSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->elem_segment = new ElemSegment();
      (yyval.module_field)->elem_segment->table_var = (yyvsp[-3].var);
      (yyval.module_field)->elem_segment->offset = (yyvsp[-2].expr_list).first;
      (yyval.module_field)->elem_segment->vars = std::move(*(yyvsp[-1].vars));
      delete (yyvsp[-1].vars);
    }
#line 3277 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 807 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3293 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 821 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3309 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 835 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Table);
      field->loc = (yylsp[0]);
      field->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3320 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 841 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3333 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 849 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3346 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 857 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3367 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 120:
#line 876 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::DataSegment);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->data_segment = new DataSegment();
      (yyval.module_field)->data_segment->memory_var = (yyvsp[-3].var);
      (yyval.module_field)->data_segment->offset = (yyvsp[-2].expr_list).first;
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.module_field)->data_segment->data, &(yyval.module_field)->data_segment->size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3381 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 885 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3397 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 899 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3413 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 913 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Memory);
      field->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3423 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 918 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3436 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 926 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3449 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 126:
#line 934 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3474 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 957 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3490 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 971 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Global);
      field->global = (yyvsp[-1].global);
      field->global->init_expr = (yyvsp[0].expr_list).first;
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3501 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 129:
#line 977 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Import);
      field->loc = (yylsp[-1]);
      field->import = (yyvsp[-1].import);
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields).first = (yyval.module_fields).last = field;
    }
#line 3514 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "src/wast-parser.y" /* yacc.c:1646  */
    {
      ModuleField* field = new ModuleField(ModuleFieldType::Export);
      field->loc = (yylsp[-1]);
      field->export_ = (yyvsp[-1].export_);
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields).first = (yyvsp[0].module_fields).first;
      (yyval.module_fields).last = (yyvsp[0].module_fields).last->next = field;
    }
#line 3527 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 998 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = (yyvsp[-1].var);
    }
#line 3540 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 1006 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = (yyvsp[-2].text);
      (yyval.import)->func->decl.sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3553 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 1014 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = (yyvsp[-2].text);
    }
#line 3564 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 1020 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = (yyvsp[-2].text);
    }
#line 3575 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 1026 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = (yyvsp[-2].text);
    }
#line 3586 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 136:
#line 1035 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Import);
      (yyval.module_field)->loc = (yylsp[-4]);
      (yyval.module_field)->import = (yyvsp[-1].import);
      (yyval.module_field)->import->module_name = (yyvsp[-3].text);
      (yyval.module_field)->import->field_name = (yyvsp[-2].text);
    }
#line 3598 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 1045 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = (yyvsp[-2].text);
      (yyval.import)->field_name = (yyvsp[-1].text);
    }
#line 3608 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1053 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3618 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1058 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3628 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1063 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3638 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1068 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = (yyvsp[-1].var);
    }
#line 3648 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1075 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Export);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->export_ = (yyvsp[-1].export_);
      (yyval.module_field)->export_->name = (yyvsp[-2].text);
    }
#line 3659 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1084 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = (yyvsp[-1].text);
    }
#line 3668 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1094 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3680 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1101 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::FuncType);
      (yyval.module_field)->loc = (yylsp[-3]);
      (yyval.module_field)->func_type = new FuncType();
      (yyval.module_field)->func_type->name = (yyvsp[-2].text);
      (yyval.module_field)->func_type->sig = std::move(*(yyvsp[-1].func_sig));
      delete (yyvsp[-1].func_sig);
    }
#line 3693 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1112 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ModuleField(ModuleFieldType::Start);
      (yyval.module_field)->loc = (yylsp[-2]);
      (yyval.module_field)->start = (yyvsp[-1].var);
    }
#line 3703 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1120 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3709 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1125 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3715 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1126 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3721 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1127 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3727 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1128 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3733 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1129 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields).first = (yyval.module_fields).last = (yyvsp[0].module_field); }
#line 3739 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1133 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 3745 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1138 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new Module();
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 3755 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1143 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      check_import_ordering(&(yylsp[0]), lexer, parser, (yyval.module), (yyvsp[0].module_fields).first);
      append_module_fields((yyval.module), (yyvsp[0].module_fields).first);
    }
#line 3765 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1151 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module) = new RawModule();
      (yyval.raw_module)->type = RawModuleType::Text;
      (yyval.raw_module)->text = (yyvsp[-1].module);
      (yyval.raw_module)->text->name = (yyvsp[-2].text);
      (yyval.raw_module)->text->loc = (yylsp[-3]);

      /* resolve func type variables where the signature was not specified
       * explicitly */
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
#line 3790 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1171 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.raw_module) = new RawModule();
      (yyval.raw_module)->type = RawModuleType::Binary;
      (yyval.raw_module)->binary.name = (yyvsp[-2].text);
      (yyval.raw_module)->binary.loc = (yylsp[-3]);
      dup_text_list(&(yyvsp[-1].text_list), &(yyval.raw_module)->binary.data, &(yyval.raw_module)->binary.size);
      destroy_text_list(&(yyvsp[-1].text_list));
    }
#line 3803 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1182 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].raw_module)->type == RawModuleType::Text) {
        (yyval.module) = (yyvsp[0].raw_module)->text;
        (yyvsp[0].raw_module)->text = nullptr;
      } else {
        assert((yyvsp[0].raw_module)->type == RawModuleType::Binary);
        (yyval.module) = new Module();
        ReadBinaryOptions options = WABT_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorHandlerModule error_handler(&(yyvsp[0].raw_module)->binary.loc, lexer, parser);
        read_binary_ir((yyvsp[0].raw_module)->binary.data, (yyvsp[0].raw_module)->binary.size, &options,
                       &error_handler, (yyval.module));
        (yyval.module)->name = (yyvsp[0].raw_module)->binary.name;
        (yyval.module)->loc = (yyvsp[0].raw_module)->binary.loc;
        WABT_ZERO_MEMORY((yyvsp[0].raw_module)->binary.name);
      }
      delete (yyvsp[0].raw_module);
    }
#line 3825 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1209 "src/wast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Index;
      (yyval.var).index = kInvalidIndex;
    }
#line 3835 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1214 "src/wast-parser.y" /* yacc.c:1646  */
    {
      WABT_ZERO_MEMORY((yyval.var));
      (yyval.var).type = VarType::Name;
      DUPTEXT((yyval.var).name, (yyvsp[0].text));
    }
#line 3845 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1222 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 3860 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1232 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = (yyvsp[-2].var);
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = (yyvsp[-1].text);
    }
#line 3872 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1242 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertMalformed;
      (yyval.command)->assert_malformed.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_malformed.text = (yyvsp[-1].text);
    }
#line 3883 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1248 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertInvalid;
      (yyval.command)->assert_invalid.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_invalid.text = (yyvsp[-1].text);
    }
#line 3894 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 171:
#line 1254 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUnlinkable;
      (yyval.command)->assert_unlinkable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_unlinkable.text = (yyvsp[-1].text);
    }
#line 3905 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1260 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertUninstantiable;
      (yyval.command)->assert_uninstantiable.module = (yyvsp[-2].raw_module);
      (yyval.command)->assert_uninstantiable.text = (yyvsp[-1].text);
    }
#line 3916 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1266 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturn;
      (yyval.command)->assert_return.action = (yyvsp[-2].action);
      (yyval.command)->assert_return.expected = (yyvsp[-1].consts);
    }
#line 3927 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 174:
#line 1272 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnCanonicalNan;
      (yyval.command)->assert_return_canonical_nan.action = (yyvsp[-1].action);
    }
#line 3937 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 175:
#line 1277 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertReturnArithmeticNan;
      (yyval.command)->assert_return_arithmetic_nan.action = (yyvsp[-1].action);
    }
#line 3947 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 176:
#line 1282 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertTrap;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3958 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 177:
#line 1288 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::AssertExhaustion;
      (yyval.command)->assert_trap.action = (yyvsp[-2].action);
      (yyval.command)->assert_trap.text = (yyvsp[-1].text);
    }
#line 3969 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1297 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Action;
      (yyval.command)->action = (yyvsp[0].action);
    }
#line 3979 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1303 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Module;
      (yyval.command)->module = (yyvsp[0].module);
    }
#line 3989 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1308 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new Command();
      (yyval.command)->type = CommandType::Register;
      (yyval.command)->register_.module_name = (yyvsp[-2].text);
      (yyval.command)->register_.var = (yyvsp[-1].var);
      (yyval.command)->register_.var.loc = (yylsp[-1]);
    }
#line 4001 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1317 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4010 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 183:
#line 1321 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4019 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1328 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4034 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 185:
#line 1340 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1341 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
#line 4049 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1348 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4057 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1351 "src/wast-parser.y" /* yacc.c:1646  */
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
#line 4122 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 189:
#line 1411 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      Command* command = new Command();
      command->type = CommandType::Module;
      command->module = (yyvsp[0].module);
      (yyval.script)->commands.emplace_back(command);
    }
#line 4134 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1423 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4140 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4144 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1426 "src/wast-parser.y" /* yacc.c:1906  */


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
          module->globals.size() != module->num_global_imports) {
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
                 SourceErrorHandler* error_handler) {
  WastParser parser;
  WABT_ZERO_MEMORY(parser);
  parser.error_handler = error_handler;
  int result = wabt_wast_parser_parse(lexer, &parser);
  delete [] parser.yyssa;
  delete [] parser.yyvsa;
  delete [] parser.yylsa;
  *out_script = parser.script;
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
