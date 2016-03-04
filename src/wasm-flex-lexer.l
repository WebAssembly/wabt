/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

%{
#include <assert.h>

#include "wasm-config.h"

#include "wasm-allocator.h"
#include "wasm-internal.h"
#include "wasm-parser.h"
#include "wasm-vector.h"

/* must be included after so some typedefs will be defined */
#include "wasm-bison-parser.h"

#if !HAVE_UNISTD_H
#define YY_NO_UNISTD_H
#if COMPILER_IS_MSVC
/* unistd.h replacement for Windows */
#include <stdlib.h>
#define isatty _isatty
#define fileno _fileno
#else
#error "no unistd.h, but not Windows!"
#endif /* COMPILER_IS_MSVC */
#endif /* !HAVE_UNISTD_H */

#define WASM_INITIAL_LINE_OFFSETS 10000

typedef size_t WasmLineOffset;
DECLARE_VECTOR(line_offset, WasmLineOffset);
DEFINE_VECTOR(line_offset, WasmLineOffset);

typedef struct WasmLexerExtra {
  WasmAllocator* allocator;
  FILE* file;
  const char* filename;
  size_t offset;
  WasmLineOffsetVector line_offsets;
  int column;
  int comment_nesting;
} WasmLexerExtra;

#define YY_DECL                                                           \
  int wasm_lexer_lex(WASM_PARSER_STYPE* yylval_param,                     \
                     WASM_PARSER_LTYPE* yylloc_param, yyscan_t yyscanner, \
                     WasmParser* parser)

#define COMMENT_NESTING (yyextra->comment_nesting)
#define RESET_COLUMN                                                       \
  do {                                                                     \
    yyextra->column = 1;                                                   \
    *wasm_append_line_offset(yyextra->allocator, &yyextra->line_offsets) = \
        yyextra->offset;                                                   \
  } while (0)

#define YY_USER_ACTION                      \
  {                                         \
    yylloc->filename = yyextra->filename;   \
    yylloc->line = yylineno;                \
    yylloc->first_column = yyextra->column; \
    yyextra->column += yyleng;              \
    yylloc->last_column = yyextra->column;  \
    yyextra->offset += yyleng;              \
  }

#define WASM_SIGN_S WASM_SIGNED
#define WASM_SIGN_U WASM_UNSIGNED

/* aliases for non-wrapping/extending loads/stores */
#define WASM_MEM_OP_TYPE_F32_LOAD32 WASM_MEM_OP_TYPE_F32_LOAD
#define WASM_MEM_OP_TYPE_F32_STORE32 WASM_MEM_OP_TYPE_F32_STORE
#define WASM_MEM_OP_TYPE_F64_LOAD64 WASM_MEM_OP_TYPE_F64_LOAD
#define WASM_MEM_OP_TYPE_F64_STORE64 WASM_MEM_OP_TYPE_F64_STORE
#define WASM_MEM_OP_TYPE_I32_LOAD32 WASM_MEM_OP_TYPE_I32_LOAD
#define WASM_MEM_OP_TYPE_I32_STORE32 WASM_MEM_OP_TYPE_I32_STORE
#define WASM_MEM_OP_TYPE_I64_LOAD64 WASM_MEM_OP_TYPE_I64_LOAD
#define WASM_MEM_OP_TYPE_I64_STORE64 WASM_MEM_OP_TYPE_I64_STORE

#define TEXT                   \
  yylval->text.start = yytext; \
  yylval->text.length = yyleng

#define TEXT_AT(offset)                 \
  yylval->text.start = yytext + offset; \
  yylval->text.length = yyleng - offset

#define TYPE(type_) yylval->type = WASM_TYPE_##type_

#define TYPE_STRUCT(struct_, type_) yylval->struct_.type = WASM_TYPE_##type_

#define MEMOP(type_, name, size_)                                 \
  TYPE_STRUCT(mem, type_);                                        \
  yylval->mem.op_type = WASM_MEM_OP_TYPE_##type_##_##name##size_; \
  yylval->mem.size = WASM_MEM_SIZE_##size_

#define MEMOPSIGN(type_, name, size_, sign_)                                \
  TYPE_STRUCT(mem, type_);                                                  \
  yylval->mem.op_type = WASM_MEM_OP_TYPE_##type_##_##name##size_##_##sign_; \
  yylval->mem.size = WASM_MEM_SIZE_##size_

#define UNOP(type_, name)    \
  TYPE_STRUCT(unary, type_); \
  yylval->unary.op_type = WASM_UNARY_OP_TYPE_##type_##_##name

#define BINOP(type_, name)    \
  TYPE_STRUCT(binary, type_); \
  yylval->binary.op_type = WASM_BINARY_OP_TYPE_##type_##_##name

#define CMPOP(type_, name)     \
  TYPE_STRUCT(compare, type_); \
  yylval->compare.op_type = WASM_COMPARE_OP_TYPE_##type_##_##name

#define CONVTYPE(type_, name, type2_)                                         \
  TYPE_STRUCT(convert, type_);                                                \
  yylval->convert.op_type = WASM_CONVERT_OP_TYPE_##type_##_##name##_##type2_; \
  yylval->convert.type2 = WASM_TYPE_##type2_

#define TOK(name) WASM_TOKEN_TYPE_##name

%}

%option prefix="wasm_lexer_"
%option bison-bridge
%option bison-locations
%option yylineno
%option reentrant
%option noyyalloc
%option noyyrealloc
%option noyyfree
%option extra-type="WasmLexerExtra*"

%x LINE_COMMENT
%x BLOCK_COMMENT
%x BAD_TEXT

SPACE     [ \t]
DIGIT     [0-9]
DIGITS    [0-9]+
HEXDIGIT  [0-9a-fA-F]
LETTER    [a-zA-Z]
SYMBOL    [+\-*/\\\^~=<>!?@#$%&|:`.]
TICK      `
ESCAPE    [nt\\'"]
CHARACTER [^"\\\x00-\x1f\x7f]|\\{ESCAPE}|\\{HEXDIGIT}{HEXDIGIT}
SIGN      [+-]?
NUM       {SIGN}{DIGIT}+
HEXNUM    {SIGN}"0x"{HEXDIGIT}+
INT       {NUM}|{HEXNUM}
FLOAT0    {NUM}\.{DIGIT}*
FLOAT1    {NUM}(\.{DIGIT}*)?[eE]{NUM}
HEXFLOAT  {SIGN}"0x"{HEXDIGIT}+\.?{HEXDIGIT}*p{SIGN}{DIGIT}+
INFINITY  {SIGN}"infinity"
NAN       {SIGN}"nan"|{SIGN}"nan:0x"{HEXDIGIT}+
FLOAT     {FLOAT0}|{FLOAT1}|{HEXFLOAT}|{INFINITY}|{NAN}
TEXT      \"{CHARACTER}*\"
ATOM      ({LETTER}|{DIGIT}|_|{TICK}|{SYMBOL})+
NAME      \${ATOM}

%%

"("                   { return TOK(LPAR); }
")"                   { return TOK(RPAR); }
{INT}                 { TEXT; return TOK(INT); }
{FLOAT}               { TEXT; return TOK(FLOAT); }
{TEXT}                { TEXT; return TOK(TEXT); }
\"                    { BEGIN(BAD_TEXT); }
<BAD_TEXT>{CHARACTER} {}
<BAD_TEXT>\n          { BEGIN(INITIAL);
                        RESET_COLUMN;
                        wasm_parser_error(yylloc, yyscanner, parser,
                                          "newline in string"); }
<BAD_TEXT><<EOF>>     { wasm_parser_error(yylloc, yyscanner, parser,
                                          "unexpected EOF");
                        return TOK(EOF); }
<BAD_TEXT>\\.         { wasm_parser_error(yylloc, yyscanner, parser,
                                          "bad escape \"%.*s\"", yyleng,
                                          yytext); }
<BAD_TEXT>\"          { BEGIN(INITIAL); TEXT; return TOK(TEXT); }
<BAD_TEXT>[^\\]       { wasm_parser_error(yylloc, yyscanner, parser,
                                          "illegal character in string"); }
"i32"                 { TYPE(I32); return TOK(VALUE_TYPE); }
"i64"                 { TYPE(I64); return TOK(VALUE_TYPE); }
"f32"                 { TYPE(F32); return TOK(VALUE_TYPE); }
"f64"                 { TYPE(F64); return TOK(VALUE_TYPE); }
"nop"                 { return TOK(NOP); }
"block"               { return TOK(BLOCK); }
"if"                  { return TOK(IF); }
"if_else"             { return TOK(IF); /* TODO(binji): remove */ }
"then"                { return TOK(THEN); }
"else"                { return TOK(ELSE); }
"loop"                { return TOK(LOOP); }
"br"                  { return TOK(BR); }
"br_if"               { return TOK(BR_IF); }
"br_table"            { return TOK(BR_TABLE); }
"case"                { return TOK(CASE); }
"call"                { return TOK(CALL); }
"call_import"         { return TOK(CALL_IMPORT); }
"call_indirect"       { return TOK(CALL_INDIRECT); }
"return"              { return TOK(RETURN); }
"get_local"           { return TOK(GET_LOCAL); }
"set_local"           { return TOK(SET_LOCAL); }
"i32.load"            { MEMOP(I32, LOAD, 32); return TOK(LOAD); }
"i64.load"            { MEMOP(I64, LOAD, 64); return TOK(LOAD); }
"f32.load"            { MEMOP(F32, LOAD, 32); return TOK(LOAD); }
"f64.load"            { MEMOP(F64, LOAD, 64); return TOK(LOAD); }
"i32.store"           { MEMOP(I32, STORE, 32); return TOK(STORE); }
"i64.store"           { MEMOP(I64, STORE, 64); return TOK(STORE); }
"f32.store"           { MEMOP(F32, STORE, 32); return TOK(STORE); }
"f64.store"           { MEMOP(F64, STORE, 64); return TOK(STORE); }
"i32.load8_s"         { MEMOPSIGN(I32, LOAD, 8, S); return TOK(LOAD); }
"i64.load8_s"         { MEMOPSIGN(I64, LOAD, 8, S); return TOK(LOAD); }
"i32.load8_u"         { MEMOPSIGN(I32, LOAD, 8, U); return TOK(LOAD); }
"i64.load8_u"         { MEMOPSIGN(I64, LOAD, 8, U); return TOK(LOAD); }
"i32.load16_s"        { MEMOPSIGN(I32, LOAD, 16, S); return TOK(LOAD); }
"i64.load16_s"        { MEMOPSIGN(I64, LOAD, 16, S); return TOK(LOAD); }
"i32.load16_u"        { MEMOPSIGN(I32, LOAD, 16, U); return TOK(LOAD); }
"i64.load16_u"        { MEMOPSIGN(I64, LOAD, 16, U); return TOK(LOAD); }
"i64.load32_s"        { MEMOPSIGN(I64, LOAD, 32, S); return TOK(LOAD); }
"i64.load32_u"        { MEMOPSIGN(I64, LOAD, 32, U); return TOK(LOAD); }
"i32.store8"          { MEMOP(I32, STORE, 8); return TOK(STORE); }
"i64.store8"          { MEMOP(I64, STORE, 8); return TOK(STORE); }
"i32.store16"         { MEMOP(I32, STORE, 16); return TOK(STORE); }
"i64.store16"         { MEMOP(I64, STORE, 16); return TOK(STORE); }
"i64.store32"         { MEMOP(I64, STORE, 32); return TOK(STORE); }
"offset="{DIGITS}     { TEXT_AT(7); return TOK(OFFSET); }
"align="{DIGITS}      { TEXT_AT(6); return TOK(ALIGN); }
"i32.const"           { TYPE(I32); return TOK(CONST); }
"i64.const"           { TYPE(I64); return TOK(CONST); }
"f32.const"           { TYPE(F32); return TOK(CONST); }
"f64.const"           { TYPE(F64); return TOK(CONST); }
"i32.not"             { UNOP(I32, NOT); return TOK(UNARY); }
"i32.clz"             { UNOP(I32, CLZ); return TOK(UNARY); }
"i64.clz"             { UNOP(I64, CLZ); return TOK(UNARY); }
"i32.ctz"             { UNOP(I32, CTZ); return TOK(UNARY); }
"i64.ctz"             { UNOP(I64, CTZ); return TOK(UNARY); }
"i32.popcnt"          { UNOP(I32, POPCNT); return TOK(UNARY); }
"i64.popcnt"          { UNOP(I64, POPCNT); return TOK(UNARY); }
"f32.neg"             { UNOP(F32, NEG); return TOK(UNARY); }
"f64.neg"             { UNOP(F64, NEG); return TOK(UNARY); }
"f32.abs"             { UNOP(F32, ABS); return TOK(UNARY); }
"f64.abs"             { UNOP(F64, ABS); return TOK(UNARY); }
"f32.sqrt"            { UNOP(F32, SQRT); return TOK(UNARY); }
"f64.sqrt"            { UNOP(F64, SQRT); return TOK(UNARY); }
"f32.ceil"            { UNOP(F32, CEIL); return TOK(UNARY); }
"f64.ceil"            { UNOP(F64, CEIL); return TOK(UNARY); }
"f32.floor"           { UNOP(F32, FLOOR); return TOK(UNARY); }
"f64.floor"           { UNOP(F64, FLOOR); return TOK(UNARY); }
"f32.trunc"           { UNOP(F32, TRUNC); return TOK(UNARY); }
"f64.trunc"           { UNOP(F64, TRUNC); return TOK(UNARY); }
"f32.nearest"         { UNOP(F32, NEAREST); return TOK(UNARY); }
"f64.nearest"         { UNOP(F64, NEAREST); return TOK(UNARY); }
"i32.add"             { BINOP(I32, ADD); return TOK(BINARY); }
"i64.add"             { BINOP(I64, ADD); return TOK(BINARY); }
"i32.sub"             { BINOP(I32, SUB); return TOK(BINARY); }
"i64.sub"             { BINOP(I64, SUB); return TOK(BINARY); }
"i32.mul"             { BINOP(I32, MUL); return TOK(BINARY); }
"i64.mul"             { BINOP(I64, MUL); return TOK(BINARY); }
"i32.div_s"           { BINOP(I32, DIV_S); return TOK(BINARY); }
"i64.div_s"           { BINOP(I64, DIV_S); return TOK(BINARY); }
"i32.div_u"           { BINOP(I32, DIV_U); return TOK(BINARY); }
"i64.div_u"           { BINOP(I64, DIV_U); return TOK(BINARY); }
"i32.rem_s"           { BINOP(I32, REM_S); return TOK(BINARY); }
"i64.rem_s"           { BINOP(I64, REM_S); return TOK(BINARY); }
"i32.rem_u"           { BINOP(I32, REM_U); return TOK(BINARY); }
"i64.rem_u"           { BINOP(I64, REM_U); return TOK(BINARY); }
"i32.and"             { BINOP(I32, AND); return TOK(BINARY); }
"i64.and"             { BINOP(I64, AND); return TOK(BINARY); }
"i32.or"              { BINOP(I32, OR); return TOK(BINARY); }
"i64.or"              { BINOP(I64, OR); return TOK(BINARY); }
"i32.xor"             { BINOP(I32, XOR); return TOK(BINARY); }
"i64.xor"             { BINOP(I64, XOR); return TOK(BINARY); }
"i32.shl"             { BINOP(I32, SHL); return TOK(BINARY); }
"i64.shl"             { BINOP(I64, SHL); return TOK(BINARY); }
"i32.shr_s"           { BINOP(I32, SHR_S); return TOK(BINARY); }
"i64.shr_s"           { BINOP(I64, SHR_S); return TOK(BINARY); }
"i32.shr_u"           { BINOP(I32, SHR_U); return TOK(BINARY); }
"i64.shr_u"           { BINOP(I64, SHR_U); return TOK(BINARY); }
"i32.rotl"            { BINOP(I32, ROTL); return TOK(BINARY); }
"i64.rotl"            { BINOP(I64, ROTL); return TOK(BINARY); }
"i32.rotr"            { BINOP(I32, ROTR); return TOK(BINARY); }
"i64.rotr"            { BINOP(I64, ROTR); return TOK(BINARY); }
"f32.add"             { BINOP(F32, ADD); return TOK(BINARY); }
"f64.add"             { BINOP(F64, ADD); return TOK(BINARY); }
"f32.sub"             { BINOP(F32, SUB); return TOK(BINARY); }
"f64.sub"             { BINOP(F64, SUB); return TOK(BINARY); }
"f32.mul"             { BINOP(F32, MUL); return TOK(BINARY); }
"f64.mul"             { BINOP(F64, MUL); return TOK(BINARY); }
"f32.div"             { BINOP(F32, DIV); return TOK(BINARY); }
"f64.div"             { BINOP(F64, DIV); return TOK(BINARY); }
"f32.min"             { BINOP(F32, MIN); return TOK(BINARY); }
"f64.min"             { BINOP(F64, MIN); return TOK(BINARY); }
"f32.max"             { BINOP(F32, MAX); return TOK(BINARY); }
"f64.max"             { BINOP(F64, MAX); return TOK(BINARY); }
"f32.copysign"        { BINOP(F32, COPYSIGN); return TOK(BINARY); }
"f64.copysign"        { BINOP(F64, COPYSIGN); return TOK(BINARY); }
"i32.eq"              { CMPOP(I32, EQ); return TOK(COMPARE); }
"i64.eq"              { CMPOP(I64, EQ); return TOK(COMPARE); }
"i32.ne"              { CMPOP(I32, NE); return TOK(COMPARE); }
"i64.ne"              { CMPOP(I64, NE); return TOK(COMPARE); }
"i32.lt_s"            { CMPOP(I32, LT_S); return TOK(COMPARE); }
"i64.lt_s"            { CMPOP(I64, LT_S); return TOK(COMPARE); }
"i32.lt_u"            { CMPOP(I32, LT_U); return TOK(COMPARE); }
"i64.lt_u"            { CMPOP(I64, LT_U); return TOK(COMPARE); }
"i32.le_s"            { CMPOP(I32, LE_S); return TOK(COMPARE); }
"i64.le_s"            { CMPOP(I64, LE_S); return TOK(COMPARE); }
"i32.le_u"            { CMPOP(I32, LE_U); return TOK(COMPARE); }
"i64.le_u"            { CMPOP(I64, LE_U); return TOK(COMPARE); }
"i32.gt_s"            { CMPOP(I32, GT_S); return TOK(COMPARE); }
"i64.gt_s"            { CMPOP(I64, GT_S); return TOK(COMPARE); }
"i32.gt_u"            { CMPOP(I32, GT_U); return TOK(COMPARE); }
"i64.gt_u"            { CMPOP(I64, GT_U); return TOK(COMPARE); }
"i32.ge_s"            { CMPOP(I32, GE_S); return TOK(COMPARE); }
"i64.ge_s"            { CMPOP(I64, GE_S); return TOK(COMPARE); }
"i32.ge_u"            { CMPOP(I32, GE_U); return TOK(COMPARE); }
"i64.ge_u"            { CMPOP(I64, GE_U); return TOK(COMPARE); }
"f32.eq"              { CMPOP(F32, EQ); return TOK(COMPARE); }
"f64.eq"              { CMPOP(F64, EQ); return TOK(COMPARE); }
"f32.ne"              { CMPOP(F32, NE); return TOK(COMPARE); }
"f64.ne"              { CMPOP(F64, NE); return TOK(COMPARE); }
"f32.lt"              { CMPOP(F32, LT); return TOK(COMPARE); }
"f64.lt"              { CMPOP(F64, LT); return TOK(COMPARE); }
"f32.le"              { CMPOP(F32, LE); return TOK(COMPARE); }
"f64.le"              { CMPOP(F64, LE); return TOK(COMPARE); }
"f32.gt"              { CMPOP(F32, GT); return TOK(COMPARE); }
"f64.gt"              { CMPOP(F64, GT); return TOK(COMPARE); }
"f32.ge"              { CMPOP(F32, GE); return TOK(COMPARE); }
"f64.ge"              { CMPOP(F64, GE); return TOK(COMPARE); }
"i64.extend_s/i32"    { CONVTYPE(I64, EXTEND_S, I32); return TOK(CONVERT); }
"i64.extend_u/i32"    { CONVTYPE(I64, EXTEND_U, I32); return TOK(CONVERT); }
"i32.wrap/i64"        { CONVTYPE(I32, WRAP, I64); return TOK(CONVERT); }
"i32.trunc_s/f32"     { CONVTYPE(I32, TRUNC_S, F32); return TOK(CONVERT); }
"i64.trunc_s/f32"     { CONVTYPE(I64, TRUNC_S, F32); return TOK(CONVERT); }
"i32.trunc_s/f64"     { CONVTYPE(I32, TRUNC_S, F64); return TOK(CONVERT); }
"i64.trunc_s/f64"     { CONVTYPE(I64, TRUNC_S, F64); return TOK(CONVERT); }
"i32.trunc_u/f32"     { CONVTYPE(I32, TRUNC_U, F32); return TOK(CONVERT); }
"i64.trunc_u/f32"     { CONVTYPE(I64, TRUNC_U, F32); return TOK(CONVERT); }
"i32.trunc_u/f64"     { CONVTYPE(I32, TRUNC_U, F64); return TOK(CONVERT); }
"i64.trunc_u/f64"     { CONVTYPE(I64, TRUNC_U, F64); return TOK(CONVERT); }
"f32.convert_s/i32"   { CONVTYPE(F32, CONVERT_S, I32); return TOK(CONVERT); }
"f64.convert_s/i32"   { CONVTYPE(F64, CONVERT_S, I32); return TOK(CONVERT); }
"f32.convert_s/i64"   { CONVTYPE(F32, CONVERT_S, I64); return TOK(CONVERT); }
"f64.convert_s/i64"   { CONVTYPE(F64, CONVERT_S, I64); return TOK(CONVERT); }
"f32.convert_u/i32"   { CONVTYPE(F32, CONVERT_U, I32); return TOK(CONVERT); }
"f64.convert_u/i32"   { CONVTYPE(F64, CONVERT_U, I32); return TOK(CONVERT); }
"f32.convert_u/i64"   { CONVTYPE(F32, CONVERT_U, I64); return TOK(CONVERT); }
"f64.convert_u/i64"   { CONVTYPE(F64, CONVERT_U, I64); return TOK(CONVERT); }
"f64.promote/f32"     { CONVTYPE(F64, PROMOTE, F32); return TOK(CONVERT); }
"f32.demote/f64"      { CONVTYPE(F32, DEMOTE, F64); return TOK(CONVERT); }
"f32.reinterpret/i32" { CONVTYPE(F32, REINTERPRET, I32); return TOK(CONVERT); }
"i32.reinterpret/f32" { CONVTYPE(I32, REINTERPRET, F32); return TOK(CONVERT); }
"f64.reinterpret/i64" { CONVTYPE(F64, REINTERPRET, I64); return TOK(CONVERT); }
"i64.reinterpret/f64" { CONVTYPE(I64, REINTERPRET, F64); return TOK(CONVERT); }
"select"              { return TOK(SELECT); }
"unreachable"         { return TOK(UNREACHABLE); }
"memory_size"         { return TOK(MEMORY_SIZE); }
"grow_memory"         { return TOK(GROW_MEMORY); }
"type"                { return TOK(TYPE); }
"func"                { return TOK(FUNC); }
"param"               { return TOK(PARAM); }
"result"              { return TOK(RESULT); }
"local"               { return TOK(LOCAL); }
"module"              { return TOK(MODULE); }
"memory"              { return TOK(MEMORY); }
"segment"             { return TOK(SEGMENT); }
"start"               { return TOK(START); }
"import"              { return TOK(IMPORT); }
"export"              { return TOK(EXPORT); }
"table"               { return TOK(TABLE); }
"assert_invalid"      { return TOK(ASSERT_INVALID); }
"assert_return"       { return TOK(ASSERT_RETURN); }
"assert_return_nan"   { return TOK(ASSERT_RETURN_NAN); }
"assert_trap"         { return TOK(ASSERT_TRAP); }
"invoke"              { return TOK(INVOKE); }
{NAME}                { TEXT; return TOK(VAR); }

";;"                  { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\n      { RESET_COLUMN; BEGIN(INITIAL); }
<LINE_COMMENT><<EOF>> { return TOK(EOF); }
<LINE_COMMENT>.
"(;"                  { BEGIN(BLOCK_COMMENT); COMMENT_NESTING = 1; }
<BLOCK_COMMENT>"(;"   { COMMENT_NESTING++; }
<BLOCK_COMMENT>";)"   { if (--COMMENT_NESTING == 0) BEGIN(INITIAL); }
<BLOCK_COMMENT>\n     { RESET_COLUMN; }
<BLOCK_COMMENT><<EOF>> { wasm_parser_error(yylloc, yyscanner, parser,
                                           "unexpected EOF");
                         return TOK(EOF); }
<BLOCK_COMMENT>.
\n                    { RESET_COLUMN; }
[ \t\r]
<<EOF>>               { return TOK(EOF); }
{ATOM}                { wasm_parser_error(yylloc, yyscanner, parser,
                                          "unexpected token \"%.*s\"", yyleng,
                                          yytext); }
.                     { wasm_parser_error(yylloc, yyscanner, parser,
                                          "unexpected char"); }

%%

int wasm_lexer_wrap(yyscan_t yyscanner) {
  return 1;
}

static WasmAllocator* s_lexer_allocator;

WasmLexer wasm_new_lexer(WasmAllocator* allocator, const char* filename) {
  WasmLexerExtra* extra =
      wasm_alloc_zero(allocator, sizeof(WasmLexerExtra), WASM_DEFAULT_ALIGN);
  if (!extra)
    return NULL;
  extra->allocator = allocator;
  extra->column = 1;
  extra->filename = filename;
  extra->file = fopen(filename, "r");

  if (!extra->file) {
    wasm_free(allocator, extra);
    return NULL;
  }

  if (wasm_reserve_line_offsets(allocator, &extra->line_offsets,
                                WASM_INITIAL_LINE_OFFSETS) != WASM_OK) {
    fclose(extra->file);
    wasm_free(allocator, extra);
    return NULL;
  }

  /* Skip the zeroeth line, and make the first line offset 0 */
  assert(extra->line_offsets.capacity >= 2);
  *wasm_append_line_offset(allocator, &extra->line_offsets) = 0;
  *wasm_append_line_offset(allocator, &extra->line_offsets) = 0;

  /* when the lexer is created in yylex_init, it uses wasm_lexer_alloc before
   extra has been set, so we have to use a static variable here. */
  s_lexer_allocator = allocator;
  yyscan_t lexer;
  yylex_init(&lexer);
  yyset_in(extra->file, lexer);
  yyset_extra(extra, lexer);
  s_lexer_allocator = NULL;
  return lexer;
}

void wasm_destroy_lexer(WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  fclose(extra->file);
  wasm_destroy_line_offset_vector(extra->allocator, &extra->line_offsets);
  wasm_lexer_lex_destroy((yyscan_t)lexer);
  wasm_free(extra->allocator, extra);
}

void* wasm_lexer_alloc(size_t bytes, WasmLexer lexer) {
  WasmAllocator* allocator =
      lexer ? (wasm_lexer_get_extra(lexer))->allocator : s_lexer_allocator;
  return wasm_alloc(allocator, bytes, WASM_DEFAULT_ALIGN);
}

void* wasm_lexer_realloc(void* ptr, size_t bytes, WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  return wasm_realloc(extra->allocator, ptr, bytes, WASM_DEFAULT_ALIGN);
}

void wasm_lexer_free(void* ptr, WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  wasm_free(extra->allocator, ptr);
}

WasmAllocator* wasm_lexer_get_allocator(WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  return extra->allocator;
}

FILE* wasm_lexer_get_file(WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  return extra->file;
}

size_t wasm_lexer_get_file_offset_from_line(WasmLexer lexer, int line) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  if (line < 0 || line >= extra->line_offsets.size)
    return INVALID_LINE_OFFSET;

  return extra->line_offsets.data[line];
}
