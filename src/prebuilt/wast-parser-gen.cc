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
#include <iterator>
#include <utility>

#include "binary-reader.h"
#include "binary-reader-ir.h"
#include "cast.h"
#include "error-handler.h"
#include "literal.h"
#include "string-view.h"
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

#define CHECK_END_LABEL(loc, begin_label, end_label)                      \
  do {                                                                    \
    if (!end_label->empty()) {                                            \
      if (begin_label.empty()) {                                          \
        wast_parser_error(&loc, lexer, parser, "unexpected label \"%s\"", \
                          end_label->c_str());                            \
      } else if (begin_label != *end_label) {                             \
        wast_parser_error(&loc, lexer, parser,                            \
                          "mismatching label \"%s\" != \"%s\"",           \
                          begin_label.c_str(), end_label->c_str());       \
      }                                                                   \
    }                                                                     \
    delete (end_label);                                                   \
  } while (0)

#define CHECK_ALLOW_EXCEPTIONS(loc, opcode_name)                      \
  do {                                                                \
    if (!parser->options->allow_future_exceptions) {                   \
      wast_parser_error(loc, lexer, parser, "opcode not allowed: %s", \
                        opcode_name);                                 \
    }                                                                 \
 } while (0)

#define YYMALLOC(size) new char [size]
#define YYFREE(p) delete [] (p)

#define USE_NATURAL_ALIGNMENT (~0)

namespace wabt {

static bool IsPowerOfTwo(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

Result ParseConst(Type type, const Literal& literal, Const* out);

static void ReverseBindings(TypeVector*, BindingHash*);

static bool IsEmptySignature(const FuncSignature* sig);

static void CheckImportOrdering(Location* loc,
                                WastLexer* lexer,
                                WastParser* parser,
                                Module* module,
                                const ModuleFieldList&);
static void AppendModuleFields(Module*, ModuleFieldList&&);

class BinaryErrorHandlerModule : public ErrorHandler {
 public:
  BinaryErrorHandlerModule(Location* loc, WastLexer* lexer, WastParser* parser);
  bool OnError(const Location&,
               const std::string& error,
               const std::string& source_line,
               size_t source_line_column_offset) override;

  // Unused.
  size_t source_line_max_length() const override { return 0; }

 private:
  Location* loc_;
  WastLexer* lexer_;
  WastParser* parser_;
};

template <typename OutputIter>
void RemoveEscapes(string_view text, OutputIter dest) {
  // Remove surrounding quotes; if any. This may be empty if the string was
  // invalid (e.g. if it contained a bad escape sequence).
  if (text.size() <= 2)
    return;

  text = text.substr(1, text.size() - 2);

  const char* src = text.data();
  const char* end = text.data() + text.size();

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
          if (Succeeded(parse_hexdigit(src[0], &hi)) &&
              Succeeded(parse_hexdigit(src[1], &lo))) {
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
}

template <typename OutputIter>
void RemoveEscapes(const TextVector& texts, OutputIter out) {
  for (const std::string& text: texts)
    RemoveEscapes(text, out);
}

template <typename T>
T MoveAndDelete(T* ptr) {
  T result = std::move(*ptr);
  delete ptr;
  return result;
}

template <typename T, typename U>
void PrependAndDelete(T& dest, U* source) {
  dest.insert(dest.begin(), std::begin(*source), std::end(*source));
  delete source;
}

template <typename T, typename U>
void AppendAndDelete(T& dest, U* source) {
  dest.insert(dest.end(), std::begin(*source), std::end(*source));
  delete source;
}

#define wabt_wast_parser_lex(...) lexer->GetToken(__VA_ARGS__, parser)
#define wabt_wast_parser_error wast_parser_error


#line 286 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:339  */

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
    WABT_TOKEN_TYPE_LPAR_CATCH = 284,
    WABT_TOKEN_TYPE_LPAR_CATCH_ALL = 285,
    WABT_TOKEN_TYPE_CALL = 286,
    WABT_TOKEN_TYPE_CALL_INDIRECT = 287,
    WABT_TOKEN_TYPE_RETURN = 288,
    WABT_TOKEN_TYPE_GET_LOCAL = 289,
    WABT_TOKEN_TYPE_SET_LOCAL = 290,
    WABT_TOKEN_TYPE_TEE_LOCAL = 291,
    WABT_TOKEN_TYPE_GET_GLOBAL = 292,
    WABT_TOKEN_TYPE_SET_GLOBAL = 293,
    WABT_TOKEN_TYPE_LOAD = 294,
    WABT_TOKEN_TYPE_STORE = 295,
    WABT_TOKEN_TYPE_OFFSET_EQ_NAT = 296,
    WABT_TOKEN_TYPE_ALIGN_EQ_NAT = 297,
    WABT_TOKEN_TYPE_CONST = 298,
    WABT_TOKEN_TYPE_UNARY = 299,
    WABT_TOKEN_TYPE_BINARY = 300,
    WABT_TOKEN_TYPE_COMPARE = 301,
    WABT_TOKEN_TYPE_CONVERT = 302,
    WABT_TOKEN_TYPE_SELECT = 303,
    WABT_TOKEN_TYPE_UNREACHABLE = 304,
    WABT_TOKEN_TYPE_CURRENT_MEMORY = 305,
    WABT_TOKEN_TYPE_GROW_MEMORY = 306,
    WABT_TOKEN_TYPE_FUNC = 307,
    WABT_TOKEN_TYPE_START = 308,
    WABT_TOKEN_TYPE_TYPE = 309,
    WABT_TOKEN_TYPE_PARAM = 310,
    WABT_TOKEN_TYPE_RESULT = 311,
    WABT_TOKEN_TYPE_LOCAL = 312,
    WABT_TOKEN_TYPE_GLOBAL = 313,
    WABT_TOKEN_TYPE_TABLE = 314,
    WABT_TOKEN_TYPE_ELEM = 315,
    WABT_TOKEN_TYPE_MEMORY = 316,
    WABT_TOKEN_TYPE_DATA = 317,
    WABT_TOKEN_TYPE_OFFSET = 318,
    WABT_TOKEN_TYPE_IMPORT = 319,
    WABT_TOKEN_TYPE_EXPORT = 320,
    WABT_TOKEN_TYPE_EXCEPT = 321,
    WABT_TOKEN_TYPE_MODULE = 322,
    WABT_TOKEN_TYPE_BIN = 323,
    WABT_TOKEN_TYPE_QUOTE = 324,
    WABT_TOKEN_TYPE_REGISTER = 325,
    WABT_TOKEN_TYPE_INVOKE = 326,
    WABT_TOKEN_TYPE_GET = 327,
    WABT_TOKEN_TYPE_ASSERT_MALFORMED = 328,
    WABT_TOKEN_TYPE_ASSERT_INVALID = 329,
    WABT_TOKEN_TYPE_ASSERT_UNLINKABLE = 330,
    WABT_TOKEN_TYPE_ASSERT_RETURN = 331,
    WABT_TOKEN_TYPE_ASSERT_RETURN_CANONICAL_NAN = 332,
    WABT_TOKEN_TYPE_ASSERT_RETURN_ARITHMETIC_NAN = 333,
    WABT_TOKEN_TYPE_ASSERT_TRAP = 334,
    WABT_TOKEN_TYPE_ASSERT_EXHAUSTION = 335,
    WABT_TOKEN_TYPE_LOW = 336
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

#line 441 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:358  */

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
#define YYLAST   1028

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  89
/* YYNRULES -- Number of rules.  */
#define YYNRULES  218
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  481

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   336

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
      75,    76,    77,    78,    79,    80,    81
};

#if WABT_WAST_PARSER_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   331,   331,   335,   341,   342,   346,   355,   356,   362,
     365,   370,   378,   382,   383,   387,   396,   397,   404,   410,
     416,   421,   428,   434,   445,   448,   451,   457,   460,   465,
     466,   472,   473,   476,   480,   481,   485,   486,   503,   504,
     520,   524,   528,   532,   535,   538,   541,   544,   547,   550,
     553,   556,   559,   562,   565,   568,   571,   574,   577,   580,
     583,   593,   596,   599,   602,   605,   608,   611,   614,   620,
     626,   632,   638,   645,   654,   657,   661,   667,   673,   680,
     681,   685,   690,   697,   701,   706,   712,   718,   723,   732,
     737,   746,   749,   755,   760,   768,   774,   777,   782,   787,
     793,   799,   805,   814,   819,   825,   831,   832,   838,   839,
     845,   850,   855,   862,   876,   882,   885,   893,   899,   908,
     915,   916,   920,   929,   930,   937,   944,   945,   949,   958,
     959,   966,   973,   977,   981,   992,   995,   999,  1006,  1016,
    1030,  1033,  1039,  1045,  1064,  1071,  1081,  1095,  1098,  1104,
    1110,  1132,  1146,  1151,  1157,  1168,  1176,  1183,  1189,  1195,
    1201,  1209,  1218,  1226,  1231,  1236,  1241,  1246,  1253,  1261,
    1271,  1276,  1285,  1291,  1292,  1293,  1294,  1295,  1296,  1297,
    1298,  1299,  1300,  1301,  1305,  1306,  1310,  1315,  1323,  1343,
    1350,  1353,  1359,  1376,  1382,  1391,  1400,  1410,  1413,  1416,
    1419,  1422,  1425,  1428,  1431,  1434,  1440,  1443,  1444,  1447,
    1454,  1458,  1465,  1475,  1476,  1483,  1486,  1548,  1557
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
  "TRY", "CATCH", "CATCH_ALL", "THROW", "RETHROW", "LPAR_CATCH",
  "LPAR_CATCH_ALL", "CALL", "CALL_INDIRECT", "RETURN", "GET_LOCAL",
  "SET_LOCAL", "TEE_LOCAL", "GET_GLOBAL", "SET_GLOBAL", "LOAD", "STORE",
  "OFFSET_EQ_NAT", "ALIGN_EQ_NAT", "CONST", "UNARY", "BINARY", "COMPARE",
  "CONVERT", "SELECT", "UNREACHABLE", "CURRENT_MEMORY", "GROW_MEMORY",
  "FUNC", "START", "TYPE", "PARAM", "RESULT", "LOCAL", "GLOBAL", "TABLE",
  "ELEM", "MEMORY", "DATA", "OFFSET", "IMPORT", "EXPORT", "EXCEPT",
  "MODULE", "BIN", "QUOTE", "REGISTER", "INVOKE", "GET",
  "ASSERT_MALFORMED", "ASSERT_INVALID", "ASSERT_UNLINKABLE",
  "ASSERT_RETURN", "ASSERT_RETURN_CANONICAL_NAN",
  "ASSERT_RETURN_ARITHMETIC_NAN", "ASSERT_TRAP", "ASSERT_EXHAUSTION",
  "LOW", "$accept", "text_list", "text_list_opt", "quoted_text",
  "value_type_list", "elem_type", "global_type", "func_type", "func_sig",
  "func_sig_result", "table_sig", "memory_sig", "limits", "type_use",
  "nat", "literal", "var", "var_list", "bind_var_opt", "bind_var",
  "labeling_opt", "offset_opt", "align_opt", "instr", "plain_instr",
  "block_instr", "block_sig", "block", "plain_catch", "plain_catch_all",
  "catch_instr", "catch_instr_list", "expr", "expr1", "try_", "catch_sexp",
  "catch_sexp_list", "if_block", "if_", "rethrow_check", "throw_check",
  "try_check", "instr_list", "expr_list", "const_expr", "exception",
  "exception_field", "func", "func_fields", "func_fields_import",
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336
};
# endif

#define YYPACT_NINF -400

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-400)))

#define YYTABLE_NINF -31

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      20,   948,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,    55,  -400,  -400,  -400,  -400,
    -400,  -400,    77,  -400,   105,   106,   238,   129,   106,   106,
     168,   106,   168,   141,   141,   106,   106,   141,   147,   147,
     167,   167,   167,   193,   193,   193,   205,   193,   100,  -400,
     240,  -400,  -400,  -400,   463,  -400,  -400,  -400,  -400,   211,
     165,   220,   226,    34,   152,   412,   241,  -400,  -400,    50,
     241,   253,  -400,   141,   246,  -400,    51,   147,  -400,   141,
     141,   198,   141,   141,   141,   -40,  -400,   271,   278,   160,
     141,   141,   141,   366,  -400,  -400,   106,   106,   106,   238,
     238,  -400,  -400,  -400,  -400,   238,   238,  -400,   238,   238,
     238,   238,   238,   249,   249,   282,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,   502,   541,  -400,  -400,  -400,   238,
     238,   106,  -400,   290,  -400,  -400,  -400,  -400,  -400,   293,
     463,  -400,   302,  -400,   304,    49,  -400,   541,   319,   121,
      34,   103,  -400,   324,  -400,   320,   325,   328,   325,   152,
     106,   106,   106,   541,   330,   331,   106,  -400,   233,   221,
    -400,  -400,   332,   325,    50,   253,  -400,   335,   334,   336,
     117,   340,   135,   253,   253,   344,    55,   345,  -400,   346,
     347,   349,   353,   190,  -400,  -400,   354,   355,   359,   238,
     106,  -400,   106,   141,   141,  -400,   580,   580,   580,  -400,
    -400,   238,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
     298,   298,  -400,  -400,  -400,  -400,   697,  -400,   947,  -400,
    -400,  -400,   580,  -400,   237,   367,  -400,  -400,  -400,  -400,
     179,   368,  -400,  -400,   361,  -400,  -400,  -400,   362,  -400,
    -400,   313,  -400,  -400,  -400,  -400,  -400,   580,   379,   580,
     380,   330,  -400,  -400,   580,   236,  -400,  -400,   253,  -400,
    -400,  -400,   381,  -400,  -400,   148,  -400,   387,   238,   238,
     238,   238,   238,  -400,  -400,  -400,   119,   242,  -400,  -400,
     255,  -400,  -400,  -400,  -400,   352,  -400,  -400,  -400,  -400,
    -400,   388,   187,   386,   191,   200,   397,   141,   404,   868,
     580,   402,  -400,    24,   403,   251,  -400,  -400,  -400,   276,
     106,  -400,   266,  -400,   106,  -400,  -400,   420,  -400,  -400,
     828,   379,   425,  -400,  -400,  -400,  -400,  -400,   580,  -400,
     296,  -400,   433,  -400,   106,   106,   106,   106,  -400,   434,
     437,   438,   449,   450,  -400,  -400,  -400,   282,  -400,   502,
     460,   619,   658,   461,   464,  -400,  -400,  -400,   106,   106,
     106,   106,   238,   541,  -400,  -400,  -400,   118,   201,   457,
     208,   215,   459,   216,  -400,   248,   541,  -400,   908,   330,
    -400,   467,   468,  -400,   296,  -400,   469,   121,   325,   325,
    -400,  -400,  -400,  -400,  -400,   470,  -400,   502,   742,  -400,
     787,  -400,   658,  -400,   218,  -400,  -400,   541,  -400,   541,
    -400,   106,  -400,   367,   475,   478,   302,   484,   479,  -400,
     485,   541,  -400,   448,   466,  -400,   244,   489,   500,   514,
     516,   517,  -400,  -400,  -400,  -400,   511,  -400,  -400,  -400,
     367,   472,  -400,  -400,   302,   476,  -400,   528,   539,   553,
     555,  -400,  -400,  -400,  -400,  -400,   106,  -400,  -400,   547,
     557,  -400,  -400,  -400,   541,   548,   566,   541,  -400,   567,
    -400
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     215,     0,   112,   183,   177,   178,   175,   179,   176,   174,
     181,   182,   173,   180,   186,   189,   208,   217,   188,   206,
     207,   210,   216,   218,     0,    31,     0,     0,    31,    31,
       0,    31,     0,     0,     0,    31,    31,     0,   190,   190,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   187,
       0,   211,     1,    33,   106,    32,    23,    28,    27,     0,
       0,     0,     0,     0,     0,     0,     0,   136,    29,     0,
       0,     4,     6,     0,     0,     7,   184,   190,   191,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,    44,    45,    34,    34,    34,     0,
       0,    29,   105,   104,   103,     0,     0,    50,     0,     0,
       0,     0,     0,    36,    36,     0,    61,    62,    63,    64,
      46,    43,    65,    66,   106,   106,    40,    41,    42,     0,
       0,    34,   132,     0,   115,   125,   126,   129,   131,   123,
     106,   172,    16,   170,     0,     0,    10,   106,     0,     0,
       0,     0,     9,     0,   140,     0,    20,     0,     0,     0,
      34,    34,    34,   106,   108,     0,    34,    29,     0,     0,
     147,    19,     0,     0,     0,     4,     2,     5,     0,     0,
       0,     0,     0,     0,     0,     0,   185,     0,   213,     0,
       0,     0,     0,     0,   202,   203,     0,     0,     0,     0,
       7,     7,     7,     0,     0,    35,   106,   106,   106,    47,
      48,     0,    51,    52,    53,    54,    55,    56,    57,    37,
      38,    38,    24,    25,    26,    60,     0,   114,     0,   107,
      68,    67,   106,   113,     0,   123,   117,   119,   120,   118,
       0,     0,    13,   171,     0,   110,   152,   151,     0,   153,
     154,     0,    18,    21,   139,   141,   142,   106,     0,   106,
       0,   108,    84,    83,   106,     0,   138,    30,     4,   146,
     148,   149,     0,     3,   145,     0,   160,     0,     0,     0,
       0,     0,     0,   168,   111,     8,     0,     0,   192,   209,
       0,   196,   197,   198,   199,     0,   201,   214,   200,   204,
     205,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     106,     0,    76,     0,     0,    49,    39,    58,    59,     0,
       7,     7,     0,   116,     7,     7,    12,     0,    29,    85,
       0,     0,     0,    87,    96,    86,   135,   109,   106,    88,
       0,   137,     0,   144,    31,    31,    31,    31,   161,     0,
       0,     0,     0,     0,   193,   194,   195,     0,    22,   106,
       0,   106,   106,     0,     0,   169,     7,    75,    34,    34,
      34,    34,     0,   106,    79,    80,    81,     0,     0,     0,
       0,     0,     0,     0,    11,     0,   106,    95,     0,   102,
      89,     0,     0,    93,    90,   150,    16,     0,     0,     0,
     163,   166,   164,   165,   167,     0,   127,   106,     0,   130,
       0,   133,   106,   162,     0,    69,    71,   106,    70,   106,
      78,    34,    82,   123,     0,   123,    16,     0,    16,   143,
       0,   106,   101,     0,     0,    94,     0,     0,     0,     0,
       0,     0,   212,   128,   134,    74,     0,    77,    73,   121,
     123,     0,   124,    14,    16,     0,    17,    98,     0,     0,
       0,   156,   155,   159,   157,   158,    34,   122,    15,     0,
     100,    91,    92,    72,   106,     0,     0,   106,    97,     0,
      99
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -400,   145,  -159,    -1,  -174,   427,  -143,   534,  -381,   170,
    -150,  -160,   -62,  -134,   -52,   252,   -12,   -92,    31,    21,
     -97,   491,   378,  -400,   -54,  -400,  -190,  -131,   173,   176,
     258,  -400,   -28,  -400,   283,   243,  -400,   307,  -400,  -400,
    -400,   -46,  -122,   383,   482,   481,  -400,  -400,   508,   414,
    -399,   259,   550,  -337,   315,  -400,  -341,    63,  -400,  -400,
     518,  -400,  -400,   509,  -400,   537,  -400,  -400,   -34,  -400,
    -400,    39,  -400,  -400,    -5,  -400,   612,  -400,  -400,    53,
     144,   234,  -400,   677,  -400,  -400,   512,  -400,  -400
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   177,   178,    73,   182,   153,   147,    61,   241,   242,
     154,   170,   155,   124,    58,   225,   267,   168,    54,   205,
     206,   220,   317,   125,   126,   127,   310,   311,   374,   375,
     376,   377,   128,   165,   339,   393,   394,   333,   334,   129,
     130,   131,   132,   262,   246,     2,     3,     4,   133,   236,
     237,   238,   134,   135,   136,   137,   138,    68,     5,     6,
     157,     7,     8,   172,     9,   148,   277,    10,   139,   181,
      11,   140,    12,    13,    14,   185,    15,    16,    17,    79,
      18,    19,    20,    21,    22,   297,   193,    23,    24
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     207,   208,    67,   229,    67,   235,   249,   171,   255,   211,
      49,   164,   156,   270,    59,   437,   272,   156,    66,   166,
      70,   411,   406,     1,   449,   245,   302,   304,   305,   149,
     158,    38,    39,    74,   232,   173,    77,   145,    67,   164,
     369,   245,    67,   370,   146,   453,    55,   166,    62,    55,
      55,   467,    55,   169,    48,    56,    55,    55,    48,    63,
      64,   244,    69,   257,   258,   259,    75,    76,   331,   264,
     443,   444,   179,   468,   338,   265,   313,   314,   188,   189,
      50,   190,   191,   192,   312,   312,   312,   209,   210,   196,
     197,   198,    80,   212,   213,    71,   214,   215,   216,   217,
     218,   319,   150,   159,   253,    52,   156,   156,   174,   342,
     312,   171,   171,   203,   204,    53,   149,   230,   231,   183,
     184,   156,   156,   354,   248,   158,   329,   273,   335,   167,
     187,   146,    60,   175,   421,   312,   261,   312,    53,   284,
     173,   331,   340,   372,   373,   285,   378,   380,   338,    72,
     381,   383,    25,    26,    27,   151,    78,    56,    28,    29,
      30,    31,    32,   152,    33,    34,    35,   203,   204,   278,
      81,    65,   164,    56,   164,   279,   280,    57,   281,   367,
     166,    49,   166,   282,    82,    83,    84,   301,   312,   150,
      90,   359,   414,   295,   296,   361,    85,   285,   159,   315,
     344,   285,   307,   308,   362,   423,   345,   346,    89,   347,
     285,   285,   425,   174,    35,   141,   340,   142,   285,   426,
     428,   303,   445,   306,   143,   285,   285,    36,   285,    60,
     332,    38,    39,   261,   324,   325,   385,   266,    56,   441,
     341,    56,    57,    56,    65,    57,   355,    57,   440,   180,
     273,   420,   429,    56,   439,   164,   -30,    57,   295,   356,
     -30,   176,   438,   166,   430,    36,   349,   350,   351,   352,
     353,   415,   416,   417,   418,   194,   164,    86,    87,    88,
      91,    92,   195,   268,   166,   203,   204,   222,   223,   224,
     219,   199,   320,   321,   233,   446,   234,   447,   199,   324,
     325,   372,   373,   332,   389,   240,   364,    36,   243,   458,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   320,   321,   247,   448,   391,   392,   251,   286,   287,
      56,   152,   254,   228,   164,   263,   269,   171,   274,   275,
     316,   379,   166,   273,   283,   382,   156,   156,   288,   289,
     291,   292,   476,   293,   164,   479,   164,   294,   298,   299,
     419,   432,   166,   300,   166,    55,    55,    55,    55,   473,
     322,   327,   326,   328,   244,   396,   397,   398,   399,    94,
      95,   160,   330,   161,   336,   343,   162,    99,   100,   101,
     102,   348,   358,   103,   104,   357,   360,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   363,   365,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   368,   371,
     199,   200,   201,   202,   384,    94,    95,   160,   388,   161,
     203,   204,   162,    99,   100,   101,   102,   395,   400,   103,
     104,   401,   402,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   403,   404,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   407,   412,    93,   424,   413,   427,
     433,   434,   436,   372,   442,   163,    94,    95,    96,   450,
      97,   451,   455,    98,    99,   100,   101,   102,   454,   457,
     103,   104,   373,   461,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   462,   226,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    94,    95,    96,   463,    97,
     464,   465,    98,    99,   100,   101,   102,   466,   321,   103,
     104,   469,   325,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   470,   228,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    94,    95,    96,   471,    97,   472,
     475,    98,    99,   100,   101,   102,   474,   477,   103,   104,
     478,   480,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   252,   309,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    94,    95,    96,   144,    97,   456,   318,
      98,    99,   100,   101,   102,   221,   459,   103,   104,   405,
     460,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   390,   408,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    94,    95,    96,   422,    97,   435,   387,    98,
      99,   100,   101,   102,   337,   260,   103,   104,   239,   323,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     276,   410,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    94,    95,    96,   227,    97,   409,   256,    98,    99,
     100,   101,   102,   271,   452,   103,   104,   250,   186,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    51,
     290,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      94,    95,   160,     0,   161,     0,     0,   162,    99,   100,
     101,   102,     0,     0,   103,   104,     0,     0,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
     115,   116,   117,   118,   119,   120,   121,   122,   123,     0,
       0,     0,   200,   201,   202,    94,    95,   160,     0,   161,
       0,     0,   162,    99,   100,   101,   102,     0,     0,   103,
     104,     0,     0,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,     0,     0,   115,   116,   117,   118,   119,
     120,   121,   122,   123,     0,     0,     0,     0,   201,   202,
      94,    95,   160,     0,   161,     0,     0,   162,    99,   100,
     101,   102,     0,     0,   103,   104,     0,     0,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
     115,   116,   117,   118,   119,   120,   121,   122,   123,     0,
       0,    94,    95,   160,   202,   161,   386,     0,   162,    99,
     100,   101,   102,     0,     0,   103,   104,     0,     0,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
       0,   115,   116,   117,   118,   119,   120,   121,   122,   123,
       0,    94,    95,   160,   366,   161,     0,     0,   162,    99,
     100,   101,   102,     0,     0,   103,   104,     0,     0,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
       0,   115,   116,   117,   118,   119,   120,   121,   122,   123,
       0,    94,    95,   160,   366,   161,   431,     0,   162,    99,
     100,   101,   102,     0,     0,   103,   104,     0,     0,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
       0,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      94,    95,   160,     0,   161,     0,     0,   162,    99,   100,
     101,   102,     0,     0,   103,   104,     0,     0,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
     115,   116,   117,   118,   119,   120,   121,   122,   123,     0,
      25,    26,    27,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,    34,    35,    36,     0,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47
};

static const yytype_int16 yycheck[] =
{
      97,    98,    30,   125,    32,   139,   149,    69,   158,   101,
      15,    65,    64,   173,    26,   396,   175,    69,    30,    65,
      32,   362,   359,     3,   423,   147,   200,   201,   202,    63,
      64,    71,    72,    34,   131,    69,    37,     3,    66,    93,
      16,   163,    70,    19,    10,   426,    25,    93,    27,    28,
      29,   450,    31,     3,     3,     5,    35,    36,     3,    28,
      29,    12,    31,   160,   161,   162,    35,    36,   258,   166,
     407,   412,    73,   454,   264,   167,   207,   208,    79,    80,
       3,    82,    83,    84,   206,   207,   208,    99,   100,    90,
      91,    92,    39,   105,   106,    32,   108,   109,   110,   111,
     112,   232,    63,    64,   156,     0,   158,   159,    69,   268,
     232,   173,   174,    64,    65,     9,   150,   129,   130,    68,
      69,   173,   174,     4,     3,   159,   257,     8,   259,    66,
      77,    10,     3,    70,    16,   257,   164,   259,     9,     4,
     174,   331,   264,    25,    26,    10,   320,   321,   338,     8,
     324,   325,    52,    53,    54,     3,     9,     5,    58,    59,
      60,    61,    62,    11,    64,    65,    66,    64,    65,    52,
       3,     3,   226,     5,   228,    58,    59,     9,    61,   310,
     226,   186,   228,    66,    40,    41,    42,   199,   310,   150,
      46,     4,   366,     3,     4,     4,     3,    10,   159,   211,
      52,    10,   203,   204,     4,     4,    58,    59,     3,    61,
      10,    10,     4,   174,    66,     4,   338,    52,    10,     4,
       4,   200,     4,   202,     4,    10,    10,    67,    10,     3,
     258,    71,    72,   261,    55,    56,   328,     4,     5,   399,
       4,     5,     9,     5,     3,     9,     4,     9,   398,     3,
       8,   373,     4,     5,   397,   309,     5,     9,     3,     4,
       9,     8,   396,   309,   386,    67,   278,   279,   280,   281,
     282,   368,   369,   370,   371,     4,   330,    43,    44,    45,
      46,    47,     4,    62,   330,    64,    65,     5,     6,     7,
      41,    54,    55,    56,     4,   417,     3,   419,    54,    55,
      56,    25,    26,   331,   332,     3,   307,    67,     4,   431,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    55,    56,     4,   421,    29,    30,     3,   183,   184,
       5,    11,     4,     3,   388,     4,     4,   399,     4,     3,
      42,   320,   388,     8,     4,   324,   398,   399,     4,     4,
       4,     4,   474,     4,   408,   477,   410,     4,     4,     4,
     372,   389,   408,     4,   410,   344,   345,   346,   347,   466,
       3,    10,     4,    60,    12,   344,   345,   346,   347,    13,
      14,    15,     3,    17,     4,     4,    20,    21,    22,    23,
      24,     4,     4,    27,    28,    43,    10,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    10,     4,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    16,    16,
      54,    55,    56,    57,     4,    13,    14,    15,     3,    17,
      64,    65,    20,    21,    22,    23,    24,     4,     4,    27,
      28,     4,     4,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,     4,     4,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     4,     4,     3,    10,     4,    10,
       3,     3,     3,    25,     4,    63,    13,    14,    15,     4,
      17,     3,     3,    20,    21,    22,    23,    24,     4,     4,
      27,    28,    26,     4,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,     4,     3,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    13,    14,    15,     4,    17,
       4,     4,    20,    21,    22,    23,    24,    16,    56,    27,
      28,     3,    56,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,     4,     3,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    13,    14,    15,     4,    17,     4,
       3,    20,    21,    22,    23,    24,    19,    19,    27,    28,
       4,     4,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   155,     3,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    13,    14,    15,    62,    17,   428,   221,
      20,    21,    22,    23,    24,   114,   433,    27,    28,   357,
     434,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,   338,     3,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    13,    14,    15,   377,    17,   394,   331,    20,
      21,    22,    23,    24,   261,   163,    27,    28,   140,   235,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
     179,     3,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    13,    14,    15,   124,    17,   361,   159,    20,    21,
      22,    23,    24,   174,   425,    27,    28,   150,    76,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    22,
     188,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      13,    14,    15,    -1,    17,    -1,    -1,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    55,    56,    57,    13,    14,    15,    -1,    17,
      -1,    -1,    20,    21,    22,    23,    24,    -1,    -1,    27,
      28,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    57,
      13,    14,    15,    -1,    17,    -1,    -1,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    13,    14,    15,    57,    17,    18,    -1,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    13,    14,    15,    56,    17,    -1,    -1,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    13,    14,    15,    56,    17,    18,    -1,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      13,    14,    15,    -1,    17,    -1,    -1,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      52,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   127,   128,   129,   140,   141,   143,   144,   146,
     149,   152,   154,   155,   156,   158,   159,   160,   162,   163,
     164,   165,   166,   169,   170,    52,    53,    54,    58,    59,
      60,    61,    62,    64,    65,    66,    67,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     3,   156,
       3,   165,     0,     9,   100,   101,     5,     9,    96,    98,
       3,    89,   101,   100,   100,     3,    98,   114,   139,   100,
      98,   139,     8,    85,    85,   100,   100,    85,     9,   161,
     161,     3,   162,   162,   162,     3,   163,   163,   163,     3,
     162,   163,   163,     3,    13,    14,    15,    17,    20,    21,
      22,    23,    24,    27,    28,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    95,   105,   106,   107,   114,   121,
     122,   123,   124,   130,   134,   135,   136,   137,   138,   150,
     153,     4,    52,     4,    89,     3,    10,    88,   147,   150,
     153,     3,    11,    87,    92,    94,    96,   142,   150,   153,
      15,    17,    20,    63,   106,   115,   123,   139,    99,     3,
      93,    94,   145,   150,   153,   139,     8,    83,    84,    85,
       3,   151,    86,    68,    69,   157,   158,   161,    85,    85,
      85,    85,    85,   168,     4,     4,    85,    85,    85,    54,
      55,    56,    57,    64,    65,   101,   102,   102,   102,    98,
      98,    99,    98,    98,    98,    98,    98,    98,    98,    41,
     103,   103,     5,     6,     7,    97,     3,   134,     3,   124,
      98,    98,   102,     4,     3,    95,   131,   132,   133,   130,
       3,    90,    91,     4,    12,   124,   126,     4,     3,    88,
     147,     3,    87,    96,     4,    92,   142,   102,   102,   102,
     126,   114,   125,     4,   102,    99,     4,    98,    62,     4,
      93,   145,    84,     8,     4,     3,   127,   148,    52,    58,
      59,    61,    66,     4,     4,    10,    83,    83,     4,     4,
     168,     4,     4,     4,     4,     3,     4,   167,     4,     4,
       4,    98,    86,   101,    86,    86,   101,    85,    85,     3,
     108,   109,   124,   109,   109,    98,    42,   104,   104,   109,
      55,    56,     3,   131,    55,    56,     4,    10,    60,   109,
       3,   108,   114,   119,   120,   109,     4,   125,   108,   116,
     124,     4,    84,     4,    52,    58,    59,    61,     4,    98,
      98,    98,    98,    98,     4,     4,     4,    43,     4,     4,
      10,     4,     4,    10,    85,     4,    56,   109,    16,    16,
      19,    16,    25,    26,   110,   111,   112,   113,    86,   101,
      86,    86,   101,    86,     4,    99,    18,   119,     3,   114,
     116,    29,    30,   117,   118,     4,   100,   100,   100,   100,
       4,     4,     4,     4,     4,    97,   135,     4,     3,   136,
       3,   138,     4,     4,    86,   102,   102,   102,   102,    98,
     124,    16,   112,     4,    10,     4,     4,    10,     4,     4,
     124,    18,   114,     3,     3,   117,     3,    90,    95,    88,
      92,    93,     4,   135,   138,     4,   124,   124,   102,   132,
       4,     3,   133,    90,     4,     3,    91,     4,   124,   110,
     111,     4,     4,     4,     4,     4,    16,   132,    90,     3,
       4,     4,     4,   102,    19,     3,   124,    19,     4,   124,
       4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    82,    83,    83,    84,    84,    85,    86,    86,    87,
      88,    88,    89,    90,    90,    90,    91,    91,    92,    93,
      94,    94,    95,    96,    97,    97,    97,    98,    98,    99,
      99,   100,   100,   101,   102,   102,   103,   103,   104,   104,
     105,   105,   105,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   107,
     107,   107,   107,   107,   108,   109,   109,   110,   111,   112,
     112,   113,   113,   114,   115,   115,   115,   115,   115,   116,
     116,   117,   117,   118,   118,   119,   119,   120,   120,   120,
     120,   120,   120,   121,   122,   123,   124,   124,   125,   125,
     126,   127,   128,   129,   130,   130,   130,   130,   130,   131,
     132,   132,   132,   133,   133,   134,   135,   135,   135,   136,
     136,   137,   138,   138,   138,   139,   139,   140,   140,   141,
     142,   142,   142,   142,   143,   143,   144,   145,   145,   145,
     145,   146,   147,   147,   147,   148,   148,   148,   148,   148,
     148,   149,   150,   151,   151,   151,   151,   151,   152,   153,
     154,   154,   155,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   157,   157,   158,   158,   159,   160,
     161,   161,   162,   162,   162,   163,   163,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   165,   165,   165,   165,
     166,   166,   167,   168,   168,   169,   169,   169,   170
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
       1,     1,     2,     3,     2,     3,     3,     3,     3,     2,
       2,     4,     4,     1,     2,     2,     1,     8,     4,     9,
       5,     3,     2,     1,     1,     1,     0,     2,     0,     2,
       1,     5,     1,     5,     2,     1,     3,     2,     2,     1,
       1,     5,     6,     0,     5,     1,     1,     5,     6,     1,
       5,     1,     1,     5,     6,     4,     1,     6,     5,     5,
       1,     2,     2,     5,     6,     5,     5,     1,     2,     2,
       4,     5,     2,     2,     2,     5,     5,     5,     5,     5,
       1,     6,     5,     4,     4,     4,     4,     4,     5,     4,
       4,     5,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     2,     1,     1,
       0,     1,     5,     6,     6,     6,     5,     5,     5,     5,
       5,     5,     4,     4,     5,     5,     1,     1,     1,     5,
       1,     2,     4,     0,     2,     0,     1,     1,     1
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
          case 83: /* text_list  */
#line 315 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).texts); }
#line 1802 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 84: /* text_list_opt  */
#line 315 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).texts); }
#line 1808 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 85: /* quoted_text  */
#line 314 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).string); }
#line 1814 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 86: /* value_type_list  */
#line 316 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1820 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 88: /* global_type  */
#line 308 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).global); }
#line 1826 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 89: /* func_type  */
#line 307 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1832 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 90: /* func_sig  */
#line 307 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1838 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 91: /* func_sig_result  */
#line 307 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func_sig); }
#line 1844 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 93: /* memory_sig  */
#line 310 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).memory); }
#line 1850 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 95: /* type_use  */
#line 317 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1856 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 98: /* var  */
#line 317 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 1862 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 99: /* var_list  */
#line 318 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).vars); }
#line 1868 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 100: /* bind_var_opt  */
#line 314 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).string); }
#line 1874 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 101: /* bind_var  */
#line 314 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).string); }
#line 1880 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 102: /* labeling_opt  */
#line 314 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).string); }
#line 1886 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 105: /* instr  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1892 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 106: /* plain_instr  */
#line 303 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1898 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 107: /* block_instr  */
#line 303 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr); }
#line 1904 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 108: /* block_sig  */
#line 316 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).types); }
#line 1910 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 109: /* block  */
#line 298 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).block); }
#line 1916 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 114: /* expr  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1922 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 115: /* expr1  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1928 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 119: /* if_block  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1934 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 120: /* if_  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1940 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 124: /* instr_list  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1946 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 125: /* expr_list  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1952 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 126: /* const_expr  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 1958 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 129: /* func  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 1964 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 130: /* func_fields  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 1970 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 131: /* func_fields_import  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1976 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 132: /* func_fields_import1  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1982 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 133: /* func_fields_import_result  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1988 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 134: /* func_fields_body  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 1994 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 135: /* func_fields_body1  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2000 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 136: /* func_result_body  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2006 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 137: /* func_body  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2012 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 138: /* func_body1  */
#line 306 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).func); }
#line 2018 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 139: /* offset  */
#line 304 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).expr_list); }
#line 2024 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 141: /* table  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2030 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 142: /* table_fields  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2036 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 144: /* memory  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2042 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 145: /* memory_fields  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2048 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 146: /* global  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2054 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 147: /* global_fields  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2060 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 148: /* import_desc  */
#line 309 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2066 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 150: /* inline_import  */
#line 309 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).import); }
#line 2072 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 151: /* export_desc  */
#line 302 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2078 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 153: /* inline_export  */
#line 302 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).export_); }
#line 2084 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 156: /* module_field  */
#line 305 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module_fields); }
#line 2090 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 157: /* module_fields_opt  */
#line 311 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2096 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 158: /* module_fields  */
#line 311 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2102 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 159: /* module  */
#line 311 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2108 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 160: /* inline_module  */
#line 311 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).module); }
#line 2114 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 161: /* script_var_opt  */
#line 317 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).var); }
#line 2120 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 162: /* script_module  */
#line 312 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script_module); }
#line 2126 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 163: /* action  */
#line 297 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).action); }
#line 2132 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 164: /* assertion  */
#line 299 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2138 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 165: /* cmd  */
#line 299 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).command); }
#line 2144 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 166: /* cmd_list  */
#line 300 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).commands); }
#line 2150 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 168: /* const_list  */
#line 301 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).consts); }
#line 2156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
        break;

    case 169: /* script  */
#line 313 "src/wast-parser.y" /* yacc.c:1257  */
      { delete ((*yyvaluep).script); }
#line 2162 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1257  */
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
#line 331 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.texts) = new TextVector();
      (yyval.texts)->emplace_back((yyvsp[0].t_text).to_string());
    }
#line 2459 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 3:
#line 335 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.texts) = (yyvsp[-1].texts);
      (yyval.texts)->emplace_back((yyvsp[0].t_text).to_string());
    }
#line 2468 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 4:
#line 341 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.texts) = new TextVector(); }
#line 2474 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 346 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.string) = new std::string();
      RemoveEscapes((yyvsp[0].t_text).to_string_view(), std::back_inserter(*(yyval.string)));
    }
#line 2483 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 355 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = new TypeVector(); }
#line 2489 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 356 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.types) = (yyvsp[-1].types);
      (yyval.types)->push_back((yyvsp[0].t_type));
    }
#line 2498 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 362 "src/wast-parser.y" /* yacc.c:1646  */
    {}
#line 2504 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 365 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[0].t_type);
      (yyval.global)->mutable_ = false;
    }
#line 2514 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 370 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.global) = new Global();
      (yyval.global)->type = (yyvsp[-1].t_type);
      (yyval.global)->mutable_ = true;
    }
#line 2524 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 378 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = (yyvsp[-1].func_sig); }
#line 2530 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 383 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      PrependAndDelete((yyval.func_sig)->param_types, (yyvsp[-2].types));
    }
#line 2539 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 387 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      (yyval.func_sig)->param_types.insert((yyval.func_sig)->param_types.begin(), (yyvsp[-2].t_type));
      // Ignore bind_var.
      delete (yyvsp[-3].string);
    }
#line 2550 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 396 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func_sig) = new FuncSignature(); }
#line 2556 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 397 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func_sig) = (yyvsp[0].func_sig);
      PrependAndDelete((yyval.func_sig)->result_types, (yyvsp[-2].types));
    }
#line 2565 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 404 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.table) = new Table();
      (yyval.table)->elem_limits = (yyvsp[-1].limits);
    }
#line 2574 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 410 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.memory) = new Memory();
      (yyval.memory)->page_limits = (yyvsp[0].limits);
    }
#line 2583 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 416 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = false;
      (yyval.limits).initial = (yyvsp[0].u64);
      (yyval.limits).max = 0;
    }
#line 2593 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 421 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.limits).has_max = true;
      (yyval.limits).initial = (yyvsp[-1].u64);
      (yyval.limits).max = (yyvsp[0].u64);
    }
#line 2603 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 428 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 2609 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 434 "src/wast-parser.y" /* yacc.c:1646  */
    {
      string_view sv = (yyvsp[0].t_literal).text.to_string_view();
      if (Failed(parse_uint64(sv.begin(), sv.end(), &(yyval.u64)))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid int \"" PRIstringview "\"",
                          WABT_PRINTF_STRING_VIEW_ARG(sv));
      }
    }
#line 2622 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 445 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal) = new Literal((yyvsp[0].t_literal));
    }
#line 2630 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 448 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal) = new Literal((yyvsp[0].t_literal));
    }
#line 2638 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 451 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.literal) = new Literal((yyvsp[0].t_literal));
    }
#line 2646 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 457 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var((yyvsp[0].u64), (yylsp[0]));
    }
#line 2654 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 460 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var((yyvsp[0].t_text).to_string_view(), (yylsp[0]));
    }
#line 2662 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 465 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.vars) = new VarVector(); }
#line 2668 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 466 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.vars) = (yyvsp[-1].vars);
      (yyval.vars)->emplace_back(MoveAndDelete((yyvsp[0].var)));
    }
#line 2677 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 472 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.string) = new std::string(); }
#line 2683 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 476 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.string) = new std::string((yyvsp[0].t_text).to_string()); }
#line 2689 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 34:
#line 480 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.string) = new std::string(); }
#line 2695 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 485 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u64) = 0; }
#line 2701 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 486 "src/wast-parser.y" /* yacc.c:1646  */
    {
      uint64_t offset64;
      string_view sv = (yyvsp[0].t_text).to_string_view();
      if (Failed(parse_int64(sv.begin(), sv.end(), &offset64,
                             ParseIntType::SignedAndUnsigned))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid offset \"" PRIstringview "\"",
                          WABT_PRINTF_STRING_VIEW_ARG(sv));
      }
      if (offset64 > UINT32_MAX) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "offset must be less than or equal to 0xffffffff");
      }
      (yyval.u64) = static_cast<uint32_t>(offset64);
    }
#line 2721 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 503 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.u32) = USE_NATURAL_ALIGNMENT; }
#line 2727 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 504 "src/wast-parser.y" /* yacc.c:1646  */
    {
      string_view sv = (yyvsp[0].t_text).to_string_view();
      if (Failed(parse_int32(sv.begin(), sv.end(), &(yyval.u32),
                             ParseIntType::UnsignedOnly))) {
        wast_parser_error(&(yylsp[0]), lexer, parser,
                          "invalid alignment \"" PRIstringview "\"",
                          WABT_PRINTF_STRING_VIEW_ARG(sv));
      }

      if ((yyval.u32) != WABT_USE_NATURAL_ALIGNMENT && !IsPowerOfTwo((yyval.u32))) {
        wast_parser_error(&(yylsp[0]), lexer, parser, "alignment must be power-of-two");
      }
    }
#line 2745 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 520 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = new ExprList((yyvsp[0].expr));
      (yyval.expr_list)->back().loc = (yylsp[0]);
    }
#line 2754 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 524 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = new ExprList((yyvsp[0].expr));
      (yyval.expr_list)->back().loc = (yylsp[0]);
    }
#line 2763 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 532 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new UnreachableExpr();
    }
#line 2771 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 535 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new NopExpr();
    }
#line 2779 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 538 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new DropExpr();
    }
#line 2787 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 541 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new SelectExpr();
    }
#line 2795 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 544 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new BrExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2803 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 547 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new BrIfExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2811 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 550 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new BrTableExpr((yyvsp[-1].vars), MoveAndDelete((yyvsp[0].var)));
    }
#line 2819 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 553 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new ReturnExpr();
    }
#line 2827 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 556 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new CallExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2835 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 559 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new CallIndirectExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2843 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 562 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new GetLocalExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2851 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 565 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new SetLocalExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2859 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 568 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new TeeLocalExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2867 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 571 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new GetGlobalExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2875 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 574 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new SetGlobalExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2883 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 577 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new LoadExpr((yyvsp[-2].t_opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2891 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 580 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new StoreExpr((yyvsp[-2].t_opcode), (yyvsp[0].u32), (yyvsp[-1].u64));
    }
#line 2899 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 583 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Const const_;
      const_.loc = (yylsp[-1]);
      auto literal = MoveAndDelete((yyvsp[0].literal));
      if (Failed(ParseConst((yyvsp[-1].t_type), literal, &const_))) {
        wast_parser_error(&(yylsp[0]), lexer, parser, "invalid literal \"%s\"",
                          literal.text.c_str());
      }
      (yyval.expr) = new ConstExpr(const_);
    }
#line 2914 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 593 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new UnaryExpr((yyvsp[0].t_opcode));
    }
#line 2922 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 596 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new BinaryExpr((yyvsp[0].t_opcode));
    }
#line 2930 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 599 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new CompareExpr((yyvsp[0].t_opcode));
    }
#line 2938 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 602 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new ConvertExpr((yyvsp[0].t_opcode));
    }
#line 2946 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 605 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new CurrentMemoryExpr();
    }
#line 2954 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 608 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new GrowMemoryExpr();
    }
#line 2962 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 611 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new ThrowExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2970 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 614 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr) = new RethrowExpr(MoveAndDelete((yyvsp[0].var)));
    }
#line 2978 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 620 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new BlockExpr((yyvsp[-2].block));
      expr->block->label = MoveAndDelete((yyvsp[-3].string));
      CHECK_END_LABEL((yylsp[0]), expr->block->label, (yyvsp[0].string));
      (yyval.expr) = expr;
    }
#line 2989 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 626 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new LoopExpr((yyvsp[-2].block));
      expr->block->label = MoveAndDelete((yyvsp[-3].string));
      CHECK_END_LABEL((yylsp[0]), expr->block->label, (yyvsp[0].string));
      (yyval.expr) = expr;
    }
#line 3000 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 71:
#line 632 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new IfExpr((yyvsp[-2].block));
      expr->true_->label = MoveAndDelete((yyvsp[-3].string));
      CHECK_END_LABEL((yylsp[0]), expr->true_->label, (yyvsp[0].string));
      (yyval.expr) = expr;
    }
#line 3011 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 72:
#line 638 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new IfExpr((yyvsp[-5].block), MoveAndDelete((yyvsp[-2].expr_list)));
      expr->true_->label = MoveAndDelete((yyvsp[-6].string));
      CHECK_END_LABEL((yylsp[-3]), expr->true_->label, (yyvsp[-3].string));
      CHECK_END_LABEL((yylsp[0]), expr->true_->label, (yyvsp[0].string));
      (yyval.expr) = expr;
    }
#line 3023 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 73:
#line 645 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyvsp[-3].block)->label = MoveAndDelete((yyvsp[-4].string));
      (yyval.expr) = (yyvsp[-2].try_expr);
      cast<TryExpr>((yyval.expr))->block = (yyvsp[-3].block);
      CHECK_END_LABEL((yylsp[0]), (yyvsp[-3].block)->label, (yyvsp[0].string));
    }
#line 3034 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 74:
#line 654 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.types) = (yyvsp[-1].types); }
#line 3040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 75:
#line 657 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = (yyvsp[0].block);
      AppendAndDelete((yyval.block)->sig, (yyvsp[-1].types));
    }
#line 3049 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 76:
#line 661 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.block) = new Block(MoveAndDelete((yyvsp[0].expr_list)));
    }
#line 3057 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 77:
#line 667 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.catch_) = new Catch(MoveAndDelete((yyvsp[-1].var)), MoveAndDelete((yyvsp[0].expr_list)));
      (yyval.catch_)->loc = (yylsp[-2]);
    }
#line 3066 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 78:
#line 673 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.catch_) = new Catch(MoveAndDelete((yyvsp[0].expr_list)));
      (yyval.catch_)->loc = (yylsp[-1]);
    }
#line 3075 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 81:
#line 685 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new TryExpr();
      expr->catches.push_back((yyvsp[0].catch_));
      (yyval.try_expr) = expr;
    }
#line 3085 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 82:
#line 690 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.try_expr) = (yyvsp[-1].try_expr);
      cast<TryExpr>((yyval.try_expr))->catches.push_back((yyvsp[0].catch_));
    }
#line 3094 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 83:
#line 697 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[-1].expr_list); }
#line 3100 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 84:
#line 701 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      (yyval.expr_list)->push_back((yyvsp[-1].expr));
      (yyvsp[-1].expr)->loc = (yylsp[-1]);
    }
#line 3110 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 85:
#line 706 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new BlockExpr((yyvsp[0].block));
      expr->block->label = MoveAndDelete((yyvsp[-1].string));
      expr->loc = (yylsp[-2]);
      (yyval.expr_list) = new ExprList(expr);
    }
#line 3121 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 86:
#line 712 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new LoopExpr((yyvsp[0].block));
      expr->block->label = MoveAndDelete((yyvsp[-1].string));
      expr->loc = (yylsp[-2]);
      (yyval.expr_list) = new ExprList(expr);
    }
#line 3132 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 87:
#line 718 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      IfExpr* if_ = cast<IfExpr>(&(yyvsp[0].expr_list)->back());
      if_->true_->label = MoveAndDelete((yyvsp[-1].string));
    }
#line 3142 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 88:
#line 723 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Block* block = (yyvsp[0].try_expr)->block;
      block->label = MoveAndDelete((yyvsp[-1].string));
      (yyvsp[0].try_expr)->loc = (yylsp[-2]);
      (yyval.expr_list) = new ExprList((yyvsp[0].try_expr));
    }
#line 3153 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 89:
#line 732 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.try_expr) = (yyvsp[0].try_expr);
      Block* block = (yyval.try_expr)->block;
      AppendAndDelete(block->sig, (yyvsp[-1].types));
    }
#line 3163 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 90:
#line 737 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Block* block = new Block();
      block->exprs = MoveAndDelete((yyvsp[-1].expr_list));
      (yyval.try_expr) = (yyvsp[0].try_expr);
      (yyval.try_expr)->block = block;
    }
#line 3174 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 91:
#line 746 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.catch_) = (yyvsp[-1].catch_);
    }
#line 3182 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 92:
#line 749 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.catch_) = (yyvsp[-1].catch_);
    }
#line 3190 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 93:
#line 755 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto expr = new TryExpr();
      expr->catches.push_back((yyvsp[0].catch_));
      (yyval.try_expr) = expr;
    }
#line 3200 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 94:
#line 760 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.try_expr) = (yyvsp[-1].try_expr);
      cast<TryExpr>((yyval.try_expr))->catches.push_back((yyvsp[0].catch_));
    }
#line 3209 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 95:
#line 768 "src/wast-parser.y" /* yacc.c:1646  */
    {
      IfExpr* if_ = cast<IfExpr>(&(yyvsp[0].expr_list)->back());
      (yyval.expr_list) = (yyvsp[0].expr_list);
      Block* true_ = if_->true_;
      AppendAndDelete(true_->sig, (yyvsp[-1].types));
    }
#line 3220 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 97:
#line 777 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[-5].expr_list))), MoveAndDelete((yyvsp[-1].expr_list)));
      expr->loc = (yylsp[-7]);
      (yyval.expr_list) = new ExprList(expr);
    }
#line 3230 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 98:
#line 782 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[-1].expr_list))));
      expr->loc = (yylsp[-3]);
      (yyval.expr_list) = new ExprList(expr);
    }
#line 3240 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 99:
#line 787 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[-5].expr_list))), MoveAndDelete((yyvsp[-1].expr_list)));
      expr->loc = (yylsp[-8]);
      (yyval.expr_list) = (yyvsp[-8].expr_list);
      (yyval.expr_list)->push_back(expr);
    }
#line 3251 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 100:
#line 793 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[-1].expr_list))));
      expr->loc = (yylsp[-4]);
      (yyval.expr_list) = (yyvsp[-4].expr_list);
      (yyval.expr_list)->push_back(expr);
    }
#line 3262 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 101:
#line 799 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[-1].expr_list))), MoveAndDelete((yyvsp[0].expr_list)));
      expr->loc = (yylsp[-2]);
      (yyval.expr_list) = (yyvsp[-2].expr_list);
      (yyval.expr_list)->push_back(expr);
    }
#line 3273 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 102:
#line 805 "src/wast-parser.y" /* yacc.c:1646  */
    {
      Expr* expr = new IfExpr(new Block(MoveAndDelete((yyvsp[0].expr_list))));
      expr->loc = (yylsp[-1]);
      (yyval.expr_list) = (yyvsp[-1].expr_list);
      (yyval.expr_list)->push_back(expr);
    }
#line 3284 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 103:
#line 814 "src/wast-parser.y" /* yacc.c:1646  */
    {
     CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "rethrow");
    }
#line 3292 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 104:
#line 819 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "throw");
    }
#line 3300 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 105:
#line 825 "src/wast-parser.y" /* yacc.c:1646  */
    {
      CHECK_ALLOW_EXCEPTIONS(&(yylsp[0]), "try");
    }
#line 3308 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 106:
#line 831 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new ExprList(); }
#line 3314 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 107:
#line 832 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      (yyval.expr_list)->splice((yyval.expr_list)->begin(), MoveAndDelete((yyvsp[-1].expr_list)));
    }
#line 3323 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 108:
#line 838 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new ExprList(); }
#line 3329 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 109:
#line 839 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[0].expr_list);
      (yyval.expr_list)->splice((yyval.expr_list)->begin(), MoveAndDelete((yyvsp[-1].expr_list)));
    }
#line 3338 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 111:
#line 850 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.exception) = new Exception(MoveAndDelete((yyvsp[-2].string)), MoveAndDelete((yyvsp[-1].types)));
    }
#line 3346 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 112:
#line 855 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new ExceptionModuleField((yyvsp[0].exception));
    }
#line 3354 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 113:
#line 862 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main_field = &(yyval.module_fields)->front();
      main_field->loc = (yylsp[-3]);
      if (auto func_field = dyn_cast<FuncModuleField>(main_field)) {
        func_field->func->name = MoveAndDelete((yyvsp[-2].string));
      } else {
        cast<ImportModuleField>(main_field)->import->func->name =
            MoveAndDelete((yyvsp[-2].string));
      }
    }
#line 3370 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 114:
#line 876 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new FuncModuleField((yyvsp[0].func));
      field->func->decl.has_func_type = true;
      field->func->decl.type_var = MoveAndDelete((yyvsp[-1].var));
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3381 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 115:
#line 882 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = new ModuleFieldList(new FuncModuleField((yyvsp[0].func)));
    }
#line 3389 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 116:
#line 885 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-2].import), (yylsp[-2]));
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      field->import->func->decl.has_func_type = true;
      field->import->func->decl.type_var = MoveAndDelete((yyvsp[-1].var));
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3402 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 117:
#line 893 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-1].import), (yylsp[-1]));
      field->import->kind = ExternalKind::Func;
      field->import->func = (yyvsp[0].func);
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3413 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 118:
#line 899 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ExportModuleField((yyvsp[-1].export_), (yylsp[-1]));
      field->export_->kind = ExternalKind::Func;
      (yyval.module_fields) = (yyvsp[0].module_fields);
      (yyval.module_fields)->push_back(field);
    }
#line 3424 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 119:
#line 908 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      ReverseBindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3433 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 121:
#line 916 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      PrependAndDelete((yyval.func)->decl.sig.param_types, (yyvsp[-2].types));
    }
#line 3442 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 122:
#line 920 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(MoveAndDelete((yyvsp[-3].string)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].t_type));
    }
#line 3453 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 123:
#line 929 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.func) = new Func(); }
#line 3459 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 124:
#line 930 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      PrependAndDelete((yyval.func)->decl.sig.result_types, (yyvsp[-2].types));
    }
#line 3468 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 125:
#line 937 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      ReverseBindings(&(yyval.func)->decl.sig.param_types, &(yyval.func)->param_bindings);
    }
#line 3477 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 127:
#line 945 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      PrependAndDelete((yyval.func)->decl.sig.param_types, (yyvsp[-2].types));
    }
#line 3486 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 128:
#line 949 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->param_bindings.emplace(MoveAndDelete((yyvsp[-3].string)),
                                 Binding((yylsp[-3]), (yyval.func)->decl.sig.param_types.size()));
      (yyval.func)->decl.sig.param_types.insert((yyval.func)->decl.sig.param_types.begin(), (yyvsp[-2].t_type));
    }
#line 3497 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 130:
#line 959 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      PrependAndDelete((yyval.func)->decl.sig.result_types, (yyvsp[-2].types));
    }
#line 3506 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 131:
#line 966 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      ReverseBindings(&(yyval.func)->local_types, &(yyval.func)->local_bindings);
    }
#line 3515 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 132:
#line 973 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = new Func();
      (yyval.func)->exprs = MoveAndDelete((yyvsp[0].expr_list));
    }
#line 3524 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 133:
#line 977 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      PrependAndDelete((yyval.func)->local_types, (yyvsp[-2].types));
    }
#line 3533 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 134:
#line 981 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.func) = (yyvsp[0].func);
      (yyval.func)->local_bindings.emplace(MoveAndDelete((yyvsp[-3].string)),
                                 Binding((yylsp[-3]), (yyval.func)->local_types.size()));
      (yyval.func)->local_types.insert((yyval.func)->local_types.begin(), (yyvsp[-2].t_type));
    }
#line 3544 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 135:
#line 992 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.expr_list) = (yyvsp[-1].expr_list);
    }
#line 3552 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 137:
#line 999 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto elem_segment = new ElemSegment();
      elem_segment->table_var = MoveAndDelete((yyvsp[-3].var));
      elem_segment->offset = MoveAndDelete((yyvsp[-2].expr_list));
      elem_segment->vars = MoveAndDelete((yyvsp[-1].vars));
      (yyval.module_field) = new ElemSegmentModuleField(elem_segment, (yylsp[-4]));
    }
#line 3564 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 138:
#line 1006 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto elem_segment = new ElemSegment();
      elem_segment->table_var = Var(0, (yylsp[-3]));
      elem_segment->offset = MoveAndDelete((yyvsp[-2].expr_list));
      elem_segment->vars = MoveAndDelete((yyvsp[-1].vars));
      (yyval.module_field) = new ElemSegmentModuleField(elem_segment, (yylsp[-3]));
    }
#line 3576 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 139:
#line 1016 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main_field = &(yyval.module_fields)->front();
      main_field->loc = (yylsp[-3]);
      if (auto table_field = dyn_cast<TableModuleField>(main_field)) {
        table_field->table->name = MoveAndDelete((yyvsp[-2].string));
      } else {
        cast<ImportModuleField>(main_field)->import->table->name =
            MoveAndDelete((yyvsp[-2].string));
      }
    }
#line 3592 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 140:
#line 1030 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = new ModuleFieldList(new TableModuleField((yyvsp[0].table)));
    }
#line 3600 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 141:
#line 1033 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-1].import));
      field->import->kind = ExternalKind::Table;
      field->import->table = (yyvsp[0].table);
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3611 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 142:
#line 1039 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ExportModuleField((yyvsp[-1].export_), (yylsp[-1]));
      field->export_->kind = ExternalKind::Table;
      (yyval.module_fields) = (yyvsp[0].module_fields);
      (yyval.module_fields)->push_back(field);
    }
#line 3622 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 143:
#line 1045 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto table = new Table();
      table->elem_limits.initial = (yyvsp[-1].vars)->size();
      table->elem_limits.max = (yyvsp[-1].vars)->size();
      table->elem_limits.has_max = true;

      auto elem_segment = new ElemSegment();
      elem_segment->table_var = Var(kInvalidIndex);
      elem_segment->offset.push_back(new ConstExpr(Const(Const::I32(), 0)));
      elem_segment->offset.back().loc = (yylsp[-2]);
      elem_segment->vars = MoveAndDelete((yyvsp[-1].vars));

      (yyval.module_fields) = new ModuleFieldList();
      (yyval.module_fields)->push_back(new TableModuleField(table));
      (yyval.module_fields)->push_back(new ElemSegmentModuleField(elem_segment, (yylsp[-2])));
    }
#line 3643 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 144:
#line 1064 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto data_segment = new DataSegment();
      data_segment->memory_var = MoveAndDelete((yyvsp[-3].var));
      data_segment->offset = MoveAndDelete((yyvsp[-2].expr_list));
      RemoveEscapes(MoveAndDelete((yyvsp[-1].texts)), std::back_inserter(data_segment->data));
      (yyval.module_field) = new DataSegmentModuleField(data_segment, (yylsp[-4]));
    }
#line 3655 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 145:
#line 1071 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto data_segment = new DataSegment();
      data_segment->memory_var = Var(0, (yylsp[-3]));
      data_segment->offset = MoveAndDelete((yyvsp[-2].expr_list));
      RemoveEscapes(MoveAndDelete((yyvsp[-1].texts)), std::back_inserter(data_segment->data));
      (yyval.module_field) = new DataSegmentModuleField(data_segment, (yylsp[-3]));
    }
#line 3667 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 146:
#line 1081 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main_field = &(yyval.module_fields)->front();
      main_field->loc = (yylsp[-3]);
      if (auto memory_field = dyn_cast<MemoryModuleField>(main_field)) {
        memory_field->memory->name = MoveAndDelete((yyvsp[-2].string));
      } else {
        cast<ImportModuleField>(main_field)->import->memory->name =
            MoveAndDelete((yyvsp[-2].string));
      }
    }
#line 3683 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 147:
#line 1095 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = new ModuleFieldList(new MemoryModuleField((yyvsp[0].memory)));
    }
#line 3691 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 148:
#line 1098 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-1].import));
      field->import->kind = ExternalKind::Memory;
      field->import->memory = (yyvsp[0].memory);
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3702 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 149:
#line 1104 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ExportModuleField((yyvsp[-1].export_), (yylsp[-1]));
      field->export_->kind = ExternalKind::Memory;
      (yyval.module_fields) = (yyvsp[0].module_fields);
      (yyval.module_fields)->push_back(field);
    }
#line 3713 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 150:
#line 1110 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto data_segment = new DataSegment();
      data_segment->memory_var = Var(kInvalidIndex);
      data_segment->offset.push_back(new ConstExpr(Const(Const::I32(), 0)));
      data_segment->offset.back().loc = (yylsp[-2]);
      RemoveEscapes(MoveAndDelete((yyvsp[-1].texts)), std::back_inserter(data_segment->data));

      uint32_t byte_size = WABT_ALIGN_UP_TO_PAGE(data_segment->data.size());
      uint32_t page_size = WABT_BYTES_TO_PAGES(byte_size);

      auto memory = new Memory();
      memory->page_limits.initial = page_size;
      memory->page_limits.max = page_size;
      memory->page_limits.has_max = true;

      (yyval.module_fields) = new ModuleFieldList();
      (yyval.module_fields)->push_back(new MemoryModuleField(memory));
      (yyval.module_fields)->push_back(new DataSegmentModuleField(data_segment, (yylsp[-2])));
    }
#line 3737 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 151:
#line 1132 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_fields) = (yyvsp[-1].module_fields);
      ModuleField* main_field = &(yyval.module_fields)->front();
      main_field->loc = (yylsp[-3]);
      if (auto global_field = dyn_cast<GlobalModuleField>(main_field)) {
        global_field->global->name = MoveAndDelete((yyvsp[-2].string));
      } else {
        cast<ImportModuleField>(main_field)->import->global->name =
            MoveAndDelete((yyvsp[-2].string));
      }
    }
#line 3753 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 152:
#line 1146 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new GlobalModuleField((yyvsp[-1].global));
      field->global->init_expr = MoveAndDelete((yyvsp[0].expr_list));
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3763 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 153:
#line 1151 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-1].import));
      field->import->kind = ExternalKind::Global;
      field->import->global = (yyvsp[0].global);
      (yyval.module_fields) = new ModuleFieldList(field);
    }
#line 3774 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 154:
#line 1157 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ExportModuleField((yyvsp[-1].export_), (yylsp[-1]));
      field->export_->kind = ExternalKind::Global;
      (yyval.module_fields) = (yyvsp[0].module_fields);
      (yyval.module_fields)->push_back(field);
    }
#line 3785 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 155:
#line 1168 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = MoveAndDelete((yyvsp[-2].string));
      (yyval.import)->func->decl.has_func_type = true;
      (yyval.import)->func->decl.type_var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3798 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 156:
#line 1176 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Func;
      (yyval.import)->func = new Func();
      (yyval.import)->func->name = MoveAndDelete((yyvsp[-2].string));
      (yyval.import)->func->decl.sig = MoveAndDelete((yyvsp[-1].func_sig));
    }
#line 3810 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 157:
#line 1183 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Table;
      (yyval.import)->table = (yyvsp[-1].table);
      (yyval.import)->table->name = MoveAndDelete((yyvsp[-2].string));
    }
#line 3821 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 158:
#line 1189 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Memory;
      (yyval.import)->memory = (yyvsp[-1].memory);
      (yyval.import)->memory->name = MoveAndDelete((yyvsp[-2].string));
    }
#line 3832 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 159:
#line 1195 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Global;
      (yyval.import)->global = (yyvsp[-1].global);
      (yyval.import)->global->name = MoveAndDelete((yyvsp[-2].string));
    }
#line 3843 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 160:
#line 1201 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->kind = ExternalKind::Except;
      (yyval.import)->except = (yyvsp[0].exception);
    }
#line 3853 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 161:
#line 1209 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ImportModuleField((yyvsp[-1].import), (yylsp[-4]));
      field->import->module_name = MoveAndDelete((yyvsp[-3].string));
      field->import->field_name = MoveAndDelete((yyvsp[-2].string));
      (yyval.module_field) = field;
    }
#line 3864 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 162:
#line 1218 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.import) = new Import();
      (yyval.import)->module_name = MoveAndDelete((yyvsp[-2].string));
      (yyval.import)->field_name = MoveAndDelete((yyvsp[-1].string));
    }
#line 3874 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 163:
#line 1226 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Func;
      (yyval.export_)->var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3884 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 164:
#line 1231 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Table;
      (yyval.export_)->var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3894 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 165:
#line 1236 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Memory;
      (yyval.export_)->var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3904 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 166:
#line 1241 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Global;
      (yyval.export_)->var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3914 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 167:
#line 1246 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->kind = ExternalKind::Except;
      (yyval.export_)->var = MoveAndDelete((yyvsp[-1].var));
    }
#line 3924 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 168:
#line 1253 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto field = new ExportModuleField((yyvsp[-1].export_), (yylsp[-3]));
      field->export_->name = MoveAndDelete((yyvsp[-2].string));
      (yyval.module_field) = field;
    }
#line 3934 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 169:
#line 1261 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.export_) = new Export();
      (yyval.export_)->name = MoveAndDelete((yyvsp[-1].string));
    }
#line 3943 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 170:
#line 1271 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto func_type = new FuncType();
      func_type->sig = MoveAndDelete((yyvsp[-1].func_sig));
      (yyval.module_field) = new FuncTypeModuleField(func_type, (yylsp[-2]));
    }
#line 3953 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 171:
#line 1276 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto func_type = new FuncType();
      func_type->name = MoveAndDelete((yyvsp[-2].string));
      func_type->sig = MoveAndDelete((yyvsp[-1].func_sig));
      (yyval.module_field) = new FuncTypeModuleField(func_type, (yylsp[-3]));
    }
#line 3964 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 172:
#line 1285 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module_field) = new StartModuleField(MoveAndDelete((yyvsp[-1].var)), (yylsp[-2]));
    }
#line 3972 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 173:
#line 1291 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 3978 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 178:
#line 1296 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 3984 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 179:
#line 1297 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 3990 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 180:
#line 1298 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 3996 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 181:
#line 1299 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 4002 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 182:
#line 1300 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 4008 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 183:
#line 1301 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module_fields) = new ModuleFieldList((yyvsp[0].module_field)); }
#line 4014 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 184:
#line 1305 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.module) = new Module(); }
#line 4020 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 186:
#line 1310 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = new Module();
      CheckImportOrdering(&(yylsp[0]), lexer, parser, (yyval.module), *(yyvsp[0].module_fields));
      AppendModuleFields((yyval.module), MoveAndDelete((yyvsp[0].module_fields)));
    }
#line 4030 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 187:
#line 1315 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.module) = (yyvsp[-1].module);
      CheckImportOrdering(&(yylsp[0]), lexer, parser, (yyval.module), *(yyvsp[0].module_fields));
      AppendModuleFields((yyval.module), MoveAndDelete((yyvsp[0].module_fields)));
    }
#line 4040 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 188:
#line 1323 "src/wast-parser.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].script_module)->type == ScriptModule::Type::Text) {
        (yyval.module) = (yyvsp[0].script_module)->text;
        (yyvsp[0].script_module)->text = nullptr;
      } else {
        assert((yyvsp[0].script_module)->type == ScriptModule::Type::Binary);
        (yyval.module) = new Module();
        ReadBinaryOptions options;
        BinaryErrorHandlerModule error_handler(&(yyvsp[0].script_module)->binary.loc, lexer, parser);
        const char* filename = "<text>";
        read_binary_ir(filename, (yyvsp[0].script_module)->binary.data.data(), (yyvsp[0].script_module)->binary.data.size(),
                       &options, &error_handler, (yyval.module));
        (yyval.module)->name = (yyvsp[0].script_module)->binary.name;
        (yyval.module)->loc = (yyvsp[0].script_module)->binary.loc;
      }
      delete (yyvsp[0].script_module);
    }
#line 4062 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 190:
#line 1350 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var(kInvalidIndex);
    }
#line 4070 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 191:
#line 1353 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.var) = new Var((yyvsp[0].t_text).to_string_view(), (yylsp[0]));
    }
#line 4078 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 192:
#line 1359 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule(ScriptModule::Type::Text);
      (yyval.script_module)->text = (yyvsp[-1].module);
      (yyval.script_module)->text->name = MoveAndDelete((yyvsp[-2].string));
      (yyval.script_module)->text->loc = (yylsp[-3]);

      // Resolve func type variables where the signature was not specified
      // explicitly.
      for (Func* func: (yyvsp[-1].module)->funcs) {
        if (func->decl.has_func_type && IsEmptySignature(&func->decl.sig)) {
          FuncType* func_type = (yyvsp[-1].module)->GetFuncType(func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
          }
        }
      }
    }
#line 4100 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 193:
#line 1376 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule(ScriptModule::Type::Binary);
      (yyval.script_module)->binary.name = MoveAndDelete((yyvsp[-3].string));
      (yyval.script_module)->binary.loc = (yylsp[-4]);
      RemoveEscapes(MoveAndDelete((yyvsp[-1].texts)), std::back_inserter((yyval.script_module)->binary.data));
    }
#line 4111 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 194:
#line 1382 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script_module) = new ScriptModule(ScriptModule::Type::Quoted);
      (yyval.script_module)->quoted.name = MoveAndDelete((yyvsp[-3].string));
      (yyval.script_module)->quoted.loc = (yylsp[-4]);
      RemoveEscapes(MoveAndDelete((yyvsp[-1].texts)), std::back_inserter((yyval.script_module)->quoted.data));
    }
#line 4122 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 195:
#line 1391 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-4]);
      (yyval.action)->module_var = MoveAndDelete((yyvsp[-3].var));
      (yyval.action)->type = ActionType::Invoke;
      (yyval.action)->name = MoveAndDelete((yyvsp[-2].string));
      (yyval.action)->invoke = new ActionInvoke();
      (yyval.action)->invoke->args = MoveAndDelete((yyvsp[-1].consts));
    }
#line 4136 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 196:
#line 1400 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.action) = new Action();
      (yyval.action)->loc = (yylsp[-3]);
      (yyval.action)->module_var = MoveAndDelete((yyvsp[-2].var));
      (yyval.action)->type = ActionType::Get;
      (yyval.action)->name = MoveAndDelete((yyvsp[-1].string));
    }
#line 4148 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 197:
#line 1410 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertMalformedCommand((yyvsp[-2].script_module), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4156 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 198:
#line 1413 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertInvalidCommand((yyvsp[-2].script_module), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4164 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 199:
#line 1416 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertUnlinkableCommand((yyvsp[-2].script_module), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4172 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 200:
#line 1419 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertUninstantiableCommand((yyvsp[-2].script_module), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4180 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 201:
#line 1422 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertReturnCommand((yyvsp[-2].action), (yyvsp[-1].consts));
    }
#line 4188 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 202:
#line 1425 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertReturnCanonicalNanCommand((yyvsp[-1].action));
    }
#line 4196 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 203:
#line 1428 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertReturnArithmeticNanCommand((yyvsp[-1].action));
    }
#line 4204 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 204:
#line 1431 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertTrapCommand((yyvsp[-2].action), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4212 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 205:
#line 1434 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new AssertExhaustionCommand((yyvsp[-2].action), MoveAndDelete((yyvsp[-1].string)));
    }
#line 4220 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 206:
#line 1440 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new ActionCommand((yyvsp[0].action));
    }
#line 4228 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 208:
#line 1444 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.command) = new ModuleCommand((yyvsp[0].module));
    }
#line 4236 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 209:
#line 1447 "src/wast-parser.y" /* yacc.c:1646  */
    {
      auto* command = new RegisterCommand(MoveAndDelete((yyvsp[-2].string)), MoveAndDelete((yyvsp[-1].var)));
      command->var.loc = (yylsp[-1]);
      (yyval.command) = command;
    }
#line 4246 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 210:
#line 1454 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = new CommandPtrVector();
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4255 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 211:
#line 1458 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.commands) = (yyvsp[-1].commands);
      (yyval.commands)->emplace_back((yyvsp[0].command));
    }
#line 4264 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 212:
#line 1465 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.const_).loc = (yylsp[-2]);
      auto literal = MoveAndDelete((yyvsp[-1].literal));
      if (Failed(ParseConst((yyvsp[-2].t_type), literal, &(yyval.const_)))) {
        wast_parser_error(&(yylsp[-1]), lexer, parser, "invalid literal \"%s\"",
                          literal.text.c_str());
      }
    }
#line 4277 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 213:
#line 1475 "src/wast-parser.y" /* yacc.c:1646  */
    { (yyval.consts) = new ConstVector(); }
#line 4283 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 214:
#line 1476 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.consts) = (yyvsp[-1].consts);
      (yyval.consts)->push_back((yyvsp[0].const_));
    }
#line 4292 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 215:
#line 1483 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
    }
#line 4300 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 216:
#line 1486 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      (yyval.script)->commands = MoveAndDelete((yyvsp[0].commands));

      int last_module_index = -1;
      for (size_t i = 0; i < (yyval.script)->commands.size(); ++i) {
        Command* command = (yyval.script)->commands[i].get();
        Var* module_var = nullptr;
        switch (command->type) {
          case CommandType::Module: {
            last_module_index = i;

            // Wire up module name bindings.
            Module* module = cast<ModuleCommand>(command)->module;
            if (module->name.empty())
              continue;

            (yyval.script)->module_bindings.emplace(module->name, Binding(module->loc, i));
            break;
          }

          case CommandType::AssertReturn:
            module_var =
                &cast<AssertReturnCommand>(command)->action->module_var;
            goto has_module_var;
          case CommandType::AssertReturnCanonicalNan:
            module_var = &cast<AssertReturnCanonicalNanCommand>(command)
                              ->action->module_var;
            goto has_module_var;
          case CommandType::AssertReturnArithmeticNan:
            module_var = &cast<AssertReturnArithmeticNanCommand>(command)
                              ->action->module_var;
            goto has_module_var;
          case CommandType::AssertTrap:
            module_var = &cast<AssertTrapCommand>(command)->action->module_var;
            goto has_module_var;
          case CommandType::AssertExhaustion:
            module_var =
                &cast<AssertExhaustionCommand>(command)->action->module_var;
            goto has_module_var;
          case CommandType::Action:
            module_var = &cast<ActionCommand>(command)->action->module_var;
            goto has_module_var;
          case CommandType::Register:
            module_var = &cast<RegisterCommand>(command)->var;
            goto has_module_var;

          has_module_var: {
            // Resolve actions with an invalid index to use the preceding
            // module.
            if (module_var->is_index() &&
                module_var->index() == kInvalidIndex) {
              module_var->set_index(last_module_index);
            }
            break;
          }

          default:
            break;
        }
      }
    }
#line 4367 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 217:
#line 1548 "src/wast-parser.y" /* yacc.c:1646  */
    {
      (yyval.script) = new Script();
      (yyval.script)->commands.emplace_back(new ModuleCommand((yyvsp[0].module)));
    }
#line 4376 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;

  case 218:
#line 1557 "src/wast-parser.y" /* yacc.c:1646  */
    { parser->script = (yyvsp[0].script); }
#line 4382 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
    break;


#line 4386 "src/prebuilt/wast-parser-gen.cc" /* yacc.c:1646  */
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
#line 1560 "src/wast-parser.y" /* yacc.c:1906  */


Result ParseConst(Type type, const Literal& literal, Const* out) {
  string_view sv = literal.text;
  const char* s = sv.begin();
  const char* end = sv.end();

  out->type = type;
  switch (type) {
    case Type::I32:
      return parse_int32(s, end, &out->u32, ParseIntType::SignedAndUnsigned);
    case Type::I64:
      return parse_int64(s, end, &out->u64, ParseIntType::SignedAndUnsigned);
    case Type::F32:
      return parse_float(literal.type, s, end, &out->f32_bits);
    case Type::F64:
      return parse_double(literal.type, s, end, &out->f64_bits);
    default:
      assert(0);
      break;
  }
  return Result::Error;
}

void ReverseBindings(TypeVector* types, BindingHash* bindings) {
  for (auto& pair : *bindings) {
    pair.second.index = types->size() - pair.second.index - 1;
  }
}

bool IsEmptySignature(const FuncSignature* sig) {
  return sig->result_types.empty() && sig->param_types.empty();
}

void append_implicit_func_declaration(Location* loc,
                                      Module* module,
                                      FuncDeclaration* decl) {
  if (decl->has_func_type)
    return;

  int sig_index = module->GetFuncTypeIndex(*decl);
  if (sig_index == -1) {
    module->AppendImplicitFuncType(*loc, decl->sig);
  } else {
    decl->sig = module->func_types[sig_index]->sig;
  }
}

void CheckImportOrdering(Location* loc, WastLexer* lexer, WastParser* parser,
                         Module* module, const ModuleFieldList& fields) {
  for (const ModuleField& field: fields) {
    if (field.type == ModuleFieldType::Import) {
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

void AppendModuleFields(Module* module, ModuleFieldList&& fields) {
  ModuleField* main_field = &fields.front();
  Index main_index = kInvalidIndex;

  for (ModuleField& field : fields) {
    std::string* name = nullptr;
    BindingHash* bindings = nullptr;
    Index index = kInvalidIndex;

    switch (field.type) {
      case ModuleFieldType::Func: {
        Func* func = cast<FuncModuleField>(&field)->func;
        append_implicit_func_declaration(&field.loc, module, &func->decl);
        name = &func->name;
        bindings = &module->func_bindings;
        index = module->funcs.size();
        module->funcs.push_back(func);
        break;
      }

      case ModuleFieldType::Global: {
        Global* global = cast<GlobalModuleField>(&field)->global;
        name = &global->name;
        bindings = &module->global_bindings;
        index = module->globals.size();
        module->globals.push_back(global);
        break;
      }

      case ModuleFieldType::Import: {
        Import* import = cast<ImportModuleField>(&field)->import;

        switch (import->kind) {
          case ExternalKind::Func:
            append_implicit_func_declaration(&field.loc, module,
                                             &import->func->decl);
            name = &import->func->name;
            bindings = &module->func_bindings;
            index = module->funcs.size();
            module->funcs.push_back(import->func);
            ++module->num_func_imports;
            break;
          case ExternalKind::Table:
            name = &import->table->name;
            bindings = &module->table_bindings;
            index = module->tables.size();
            module->tables.push_back(import->table);
            ++module->num_table_imports;
            break;
          case ExternalKind::Memory:
            name = &import->memory->name;
            bindings = &module->memory_bindings;
            index = module->memories.size();
            module->memories.push_back(import->memory);
            ++module->num_memory_imports;
            break;
          case ExternalKind::Global:
            name = &import->global->name;
            bindings = &module->global_bindings;
            index = module->globals.size();
            module->globals.push_back(import->global);
            ++module->num_global_imports;
            break;
          case ExternalKind::Except:
            name = &import->except->name;
            bindings = &module->except_bindings;
            index = module->excepts.size();
            module->excepts.push_back(import->except);
            ++module->num_except_imports;
            break;
        }
        module->imports.push_back(import);
        break;
      }

      case ModuleFieldType::Export: {
        Export* export_ = cast<ExportModuleField>(&field)->export_;
        if (&field != main_field) {
          // If this is not the main field, it must be an inline export.
          export_->var.set_index(main_index);
        }
        name = &export_->name;
        bindings = &module->export_bindings;
        index = module->exports.size();
        module->exports.push_back(export_);
        break;
      }

      case ModuleFieldType::FuncType: {
        FuncType* func_type = cast<FuncTypeModuleField>(&field)->func_type;
        name = &func_type->name;
        bindings = &module->func_type_bindings;
        index = module->func_types.size();
        module->func_types.push_back(func_type);
        break;
      }

      case ModuleFieldType::Table: {
        Table* table = cast<TableModuleField>(&field)->table;
        name = &table->name;
        bindings = &module->table_bindings;
        index = module->tables.size();
        module->tables.push_back(table);
        break;
      }

      case ModuleFieldType::ElemSegment: {
        ElemSegment* elem_segment =
            cast<ElemSegmentModuleField>(&field)->elem_segment;
        if (&field != main_field) {
          // If this is not the main field, it must be an inline elem segment.
          elem_segment->table_var.set_index(main_index);
        }
        module->elem_segments.push_back(elem_segment);
        break;
      }

      case ModuleFieldType::Memory: {
        Memory* memory = cast<MemoryModuleField>(&field)->memory;
        name = &memory->name;
        bindings = &module->memory_bindings;
        index = module->memories.size();
        module->memories.push_back(memory);
        break;
      }

      case ModuleFieldType::DataSegment: {
        DataSegment* data_segment =
            cast<DataSegmentModuleField>(&field)->data_segment;
        if (&field != main_field) {
          // If this is not the main field, it must be an inline data segment.
          data_segment->memory_var.set_index(main_index);
        }
        module->data_segments.push_back(data_segment);
        break;
      }

      case ModuleFieldType::Except: {
        Exception* except = cast<ExceptionModuleField>(&field)->except;
        name = &except->name;
        bindings = &module->except_bindings;
        index = module->excepts.size();
        module->excepts.push_back(except);
        break;
      }

      case ModuleFieldType::Start:
        module->start = &cast<StartModuleField>(&field)->start;
        break;
    }

    if (&field == main_field)
      main_index = index;

    if (name && bindings) {
      // Exported names are allowed to be empty; other names aren't.
      if (bindings == &module->export_bindings || !name->empty()) {
        bindings->emplace(*name, Binding(field.loc, index));
      }
    }
  }

  module->fields.splice(module->fields.end(), fields);
}

Result parse_wast(WastLexer* lexer, Script** out_script,
                  ErrorHandler* error_handler,
                  WastParseOptions* options) {
  WastParser parser;
  ZeroMemory(parser);
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
    : ErrorHandler(Location::Type::Binary),
      loc_(loc),
      lexer_(lexer),
      parser_(parser) {}

bool BinaryErrorHandlerModule::OnError(
    const Location& binary_loc, const std::string& error,
    const std::string& source_line, size_t source_line_column_offset) {
  if (binary_loc.offset == kInvalidOffset) {
    wast_parser_error(loc_, lexer_, parser_, "error in binary module: %s",
                      error.c_str());
  } else {
    wast_parser_error(loc_, lexer_, parser_,
                      "error in binary module: @0x%08" PRIzx ": %s",
                      binary_loc.offset, error.c_str());
  }
  return true;
}

}  // namespace wabt
