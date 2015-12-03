/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
    WASM_TOKEN_TYPE_PAGE_SIZE = 317,
    WASM_TOKEN_TYPE_LOW = 318
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
