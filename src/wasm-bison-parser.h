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
    WASM_TOKEN_TYPE_GLOBAL = 310,
    WASM_TOKEN_TYPE_LOAD_GLOBAL = 311,
    WASM_TOKEN_TYPE_STORE_GLOBAL = 312,
    WASM_TOKEN_TYPE_LOW = 313
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
