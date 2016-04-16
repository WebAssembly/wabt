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

%top {
#include <stdint.h>
}

%{
#include <assert.h>

#include "wasm-config.h"

#include "wasm-allocator.h"
#include "wasm-parser.h"
#include "wasm-parser-lexer-shared.h"
#include "wasm-vector.h"

/* must be included after so some typedefs will be defined */
#include "wasm-bison-parser.h"

#if !HAVE_UNISTD_H
#define YY_NO_UNISTD_H
#if COMPILER_IS_MSVC
/* unistd.h replacement for Windows */
#include <stdlib.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#error "no unistd.h, but not Windows!"
#endif /* COMPILER_IS_MSVC */
#endif /* !HAVE_UNISTD_H */

#define WASM_INITIAL_LINE_OFFSETS 10000

typedef size_t WasmLineOffset;
WASM_DEFINE_VECTOR(line_offset, WasmLineOffset);

typedef enum WasmLexerType {
  WASM_LEXER_TYPE_FILE,
  WASM_LEXER_TYPE_BUFFER,
} WasmLexerType;

typedef struct WasmLexerExtra {
  WasmAllocator* allocator;
  WasmLexerType type;
  union {
    FILE* file;
    struct {
      const void* data;
      size_t size;
      size_t read_offset;
    } buffer;
  };
  const char* filename;
  size_t offset;
  WasmLineOffsetVector line_offsets;
  int column;
  int comment_nesting;
} WasmLexerExtra;

static void read_input(WasmLexer, char* buffer, size_t* result,
                       size_t max_size);

#define YY_DECL                                                           \
  int wasm_lexer_lex(WASM_PARSER_STYPE* yylval_param,                     \
                     WASM_PARSER_LTYPE* yylloc_param, yyscan_t yyscanner, \
                     WasmParser* parser)

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)               \
  do {                                                \
    size_t my_result = 0;                             \
    read_input(yyscanner, buf, &my_result, max_size); \
    result = my_result;                               \
  } while (0)

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

#define TEXT                   \
  yylval->text.start = yytext; \
  yylval->text.length = yyleng

#define TEXT_AT(offset)                 \
  yylval->text.start = yytext + offset; \
  yylval->text.length = yyleng - offset

#define TYPE(type_) yylval->type = WASM_TYPE_##type_

#define OPCODE(name) yylval->opcode = WASM_OPCODE_##name

#define LITERAL(type_)                      \
  yylval->literal.type = WASM_LITERAL_TYPE_##type_; \
  yylval->literal.text.start = yytext;              \
  yylval->literal.text.length = yyleng

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
FLOAT     {FLOAT0}|{FLOAT1}
TEXT      \"{CHARACTER}*\"
ATOM      ({LETTER}|{DIGIT}|_|{TICK}|{SYMBOL})+
NAME      \${ATOM}

%%

"("                   { return TOK(LPAR); }
")"                   { return TOK(RPAR); }
{INT}                 { LITERAL(INT); return TOK(INT); }
{FLOAT}               { LITERAL(FLOAT); return TOK(FLOAT); }
{HEXFLOAT}            { LITERAL(HEXFLOAT); return TOK(FLOAT); }
{INFINITY}            { LITERAL(INFINITY); return TOK(FLOAT); }
{NAN}                 { LITERAL(NAN); return TOK(FLOAT); }
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
"i32.load"            { OPCODE(I32_LOAD); return TOK(LOAD); }
"i64.load"            { OPCODE(I64_LOAD); return TOK(LOAD); }
"f32.load"            { OPCODE(F32_LOAD); return TOK(LOAD); }
"f64.load"            { OPCODE(F64_LOAD); return TOK(LOAD); }
"i32.store"           { OPCODE(I32_STORE); return TOK(STORE); }
"i64.store"           { OPCODE(I64_STORE); return TOK(STORE); }
"f32.store"           { OPCODE(F32_STORE); return TOK(STORE); }
"f64.store"           { OPCODE(F64_STORE); return TOK(STORE); }
"i32.load8_s"         { OPCODE(I32_LOAD8_S); return TOK(LOAD); }
"i64.load8_s"         { OPCODE(I64_LOAD8_S); return TOK(LOAD); }
"i32.load8_u"         { OPCODE(I32_LOAD8_U); return TOK(LOAD); }
"i64.load8_u"         { OPCODE(I64_LOAD8_U); return TOK(LOAD); }
"i32.load16_s"        { OPCODE(I32_LOAD16_S); return TOK(LOAD); }
"i64.load16_s"        { OPCODE(I64_LOAD16_S); return TOK(LOAD); }
"i32.load16_u"        { OPCODE(I32_LOAD16_U); return TOK(LOAD); }
"i64.load16_u"        { OPCODE(I64_LOAD16_U); return TOK(LOAD); }
"i64.load32_s"        { OPCODE(I64_LOAD32_S); return TOK(LOAD); }
"i64.load32_u"        { OPCODE(I64_LOAD32_U); return TOK(LOAD); }
"i32.store8"          { OPCODE(I32_STORE8); return TOK(STORE); }
"i64.store8"          { OPCODE(I64_STORE8); return TOK(STORE); }
"i32.store16"         { OPCODE(I32_STORE16); return TOK(STORE); }
"i64.store16"         { OPCODE(I64_STORE16); return TOK(STORE); }
"i64.store32"         { OPCODE(I64_STORE32); return TOK(STORE); }
"offset="{DIGITS}     { TEXT_AT(7); return TOK(OFFSET); }
"align="{DIGITS}      { TEXT_AT(6); return TOK(ALIGN); }
"i32.const"           { TYPE(I32); return TOK(CONST); }
"i64.const"           { TYPE(I64); return TOK(CONST); }
"f32.const"           { TYPE(F32); return TOK(CONST); }
"f64.const"           { TYPE(F64); return TOK(CONST); }
"i32.eqz"             { OPCODE(I32_EQZ); return TOK(CONVERT); }
"i64.eqz"             { OPCODE(I64_EQZ); return TOK(CONVERT); }
"i32.clz"             { OPCODE(I32_CLZ); return TOK(UNARY); }
"i64.clz"             { OPCODE(I64_CLZ); return TOK(UNARY); }
"i32.ctz"             { OPCODE(I32_CTZ); return TOK(UNARY); }
"i64.ctz"             { OPCODE(I64_CTZ); return TOK(UNARY); }
"i32.popcnt"          { OPCODE(I32_POPCNT); return TOK(UNARY); }
"i64.popcnt"          { OPCODE(I64_POPCNT); return TOK(UNARY); }
"f32.neg"             { OPCODE(F32_NEG); return TOK(UNARY); }
"f64.neg"             { OPCODE(F64_NEG); return TOK(UNARY); }
"f32.abs"             { OPCODE(F32_ABS); return TOK(UNARY); }
"f64.abs"             { OPCODE(F64_ABS); return TOK(UNARY); }
"f32.sqrt"            { OPCODE(F32_SQRT); return TOK(UNARY); }
"f64.sqrt"            { OPCODE(F64_SQRT); return TOK(UNARY); }
"f32.ceil"            { OPCODE(F32_CEIL); return TOK(UNARY); }
"f64.ceil"            { OPCODE(F64_CEIL); return TOK(UNARY); }
"f32.floor"           { OPCODE(F32_FLOOR); return TOK(UNARY); }
"f64.floor"           { OPCODE(F64_FLOOR); return TOK(UNARY); }
"f32.trunc"           { OPCODE(F32_TRUNC); return TOK(UNARY); }
"f64.trunc"           { OPCODE(F64_TRUNC); return TOK(UNARY); }
"f32.nearest"         { OPCODE(F32_NEAREST); return TOK(UNARY); }
"f64.nearest"         { OPCODE(F64_NEAREST); return TOK(UNARY); }
"i32.add"             { OPCODE(I32_ADD); return TOK(BINARY); }
"i64.add"             { OPCODE(I64_ADD); return TOK(BINARY); }
"i32.sub"             { OPCODE(I32_SUB); return TOK(BINARY); }
"i64.sub"             { OPCODE(I64_SUB); return TOK(BINARY); }
"i32.mul"             { OPCODE(I32_MUL); return TOK(BINARY); }
"i64.mul"             { OPCODE(I64_MUL); return TOK(BINARY); }
"i32.div_s"           { OPCODE(I32_DIV_S); return TOK(BINARY); }
"i64.div_s"           { OPCODE(I64_DIV_S); return TOK(BINARY); }
"i32.div_u"           { OPCODE(I32_DIV_U); return TOK(BINARY); }
"i64.div_u"           { OPCODE(I64_DIV_U); return TOK(BINARY); }
"i32.rem_s"           { OPCODE(I32_REM_S); return TOK(BINARY); }
"i64.rem_s"           { OPCODE(I64_REM_S); return TOK(BINARY); }
"i32.rem_u"           { OPCODE(I32_REM_U); return TOK(BINARY); }
"i64.rem_u"           { OPCODE(I64_REM_U); return TOK(BINARY); }
"i32.and"             { OPCODE(I32_AND); return TOK(BINARY); }
"i64.and"             { OPCODE(I64_AND); return TOK(BINARY); }
"i32.or"              { OPCODE(I32_OR); return TOK(BINARY); }
"i64.or"              { OPCODE(I64_OR); return TOK(BINARY); }
"i32.xor"             { OPCODE(I32_XOR); return TOK(BINARY); }
"i64.xor"             { OPCODE(I64_XOR); return TOK(BINARY); }
"i32.shl"             { OPCODE(I32_SHL); return TOK(BINARY); }
"i64.shl"             { OPCODE(I64_SHL); return TOK(BINARY); }
"i32.shr_s"           { OPCODE(I32_SHR_S); return TOK(BINARY); }
"i64.shr_s"           { OPCODE(I64_SHR_S); return TOK(BINARY); }
"i32.shr_u"           { OPCODE(I32_SHR_U); return TOK(BINARY); }
"i64.shr_u"           { OPCODE(I64_SHR_U); return TOK(BINARY); }
"i32.rotl"            { OPCODE(I32_ROTL); return TOK(BINARY); }
"i64.rotl"            { OPCODE(I64_ROTL); return TOK(BINARY); }
"i32.rotr"            { OPCODE(I32_ROTR); return TOK(BINARY); }
"i64.rotr"            { OPCODE(I64_ROTR); return TOK(BINARY); }
"f32.add"             { OPCODE(F32_ADD); return TOK(BINARY); }
"f64.add"             { OPCODE(F64_ADD); return TOK(BINARY); }
"f32.sub"             { OPCODE(F32_SUB); return TOK(BINARY); }
"f64.sub"             { OPCODE(F64_SUB); return TOK(BINARY); }
"f32.mul"             { OPCODE(F32_MUL); return TOK(BINARY); }
"f64.mul"             { OPCODE(F64_MUL); return TOK(BINARY); }
"f32.div"             { OPCODE(F32_DIV); return TOK(BINARY); }
"f64.div"             { OPCODE(F64_DIV); return TOK(BINARY); }
"f32.min"             { OPCODE(F32_MIN); return TOK(BINARY); }
"f64.min"             { OPCODE(F64_MIN); return TOK(BINARY); }
"f32.max"             { OPCODE(F32_MAX); return TOK(BINARY); }
"f64.max"             { OPCODE(F64_MAX); return TOK(BINARY); }
"f32.copysign"        { OPCODE(F32_COPYSIGN); return TOK(BINARY); }
"f64.copysign"        { OPCODE(F64_COPYSIGN); return TOK(BINARY); }
"i32.eq"              { OPCODE(I32_EQ); return TOK(COMPARE); }
"i64.eq"              { OPCODE(I64_EQ); return TOK(COMPARE); }
"i32.ne"              { OPCODE(I32_NE); return TOK(COMPARE); }
"i64.ne"              { OPCODE(I64_NE); return TOK(COMPARE); }
"i32.lt_s"            { OPCODE(I32_LT_S); return TOK(COMPARE); }
"i64.lt_s"            { OPCODE(I64_LT_S); return TOK(COMPARE); }
"i32.lt_u"            { OPCODE(I32_LT_U); return TOK(COMPARE); }
"i64.lt_u"            { OPCODE(I64_LT_U); return TOK(COMPARE); }
"i32.le_s"            { OPCODE(I32_LE_S); return TOK(COMPARE); }
"i64.le_s"            { OPCODE(I64_LE_S); return TOK(COMPARE); }
"i32.le_u"            { OPCODE(I32_LE_U); return TOK(COMPARE); }
"i64.le_u"            { OPCODE(I64_LE_U); return TOK(COMPARE); }
"i32.gt_s"            { OPCODE(I32_GT_S); return TOK(COMPARE); }
"i64.gt_s"            { OPCODE(I64_GT_S); return TOK(COMPARE); }
"i32.gt_u"            { OPCODE(I32_GT_U); return TOK(COMPARE); }
"i64.gt_u"            { OPCODE(I64_GT_U); return TOK(COMPARE); }
"i32.ge_s"            { OPCODE(I32_GE_S); return TOK(COMPARE); }
"i64.ge_s"            { OPCODE(I64_GE_S); return TOK(COMPARE); }
"i32.ge_u"            { OPCODE(I32_GE_U); return TOK(COMPARE); }
"i64.ge_u"            { OPCODE(I64_GE_U); return TOK(COMPARE); }
"f32.eq"              { OPCODE(F32_EQ); return TOK(COMPARE); }
"f64.eq"              { OPCODE(F64_EQ); return TOK(COMPARE); }
"f32.ne"              { OPCODE(F32_NE); return TOK(COMPARE); }
"f64.ne"              { OPCODE(F64_NE); return TOK(COMPARE); }
"f32.lt"              { OPCODE(F32_LT); return TOK(COMPARE); }
"f64.lt"              { OPCODE(F64_LT); return TOK(COMPARE); }
"f32.le"              { OPCODE(F32_LE); return TOK(COMPARE); }
"f64.le"              { OPCODE(F64_LE); return TOK(COMPARE); }
"f32.gt"              { OPCODE(F32_GT); return TOK(COMPARE); }
"f64.gt"              { OPCODE(F64_GT); return TOK(COMPARE); }
"f32.ge"              { OPCODE(F32_GE); return TOK(COMPARE); }
"f64.ge"              { OPCODE(F64_GE); return TOK(COMPARE); }
"i64.extend_s/i32"    { OPCODE(I64_EXTEND_S_I32); return TOK(CONVERT); }
"i64.extend_u/i32"    { OPCODE(I64_EXTEND_U_I32); return TOK(CONVERT); }
"i32.wrap/i64"        { OPCODE(I32_WRAP_I64); return TOK(CONVERT); }
"i32.trunc_s/f32"     { OPCODE(I32_TRUNC_S_F32); return TOK(CONVERT); }
"i64.trunc_s/f32"     { OPCODE(I64_TRUNC_S_F32); return TOK(CONVERT); }
"i32.trunc_s/f64"     { OPCODE(I32_TRUNC_S_F64); return TOK(CONVERT); }
"i64.trunc_s/f64"     { OPCODE(I64_TRUNC_S_F64); return TOK(CONVERT); }
"i32.trunc_u/f32"     { OPCODE(I32_TRUNC_U_F32); return TOK(CONVERT); }
"i64.trunc_u/f32"     { OPCODE(I64_TRUNC_U_F32); return TOK(CONVERT); }
"i32.trunc_u/f64"     { OPCODE(I32_TRUNC_U_F64); return TOK(CONVERT); }
"i64.trunc_u/f64"     { OPCODE(I64_TRUNC_U_F64); return TOK(CONVERT); }
"f32.convert_s/i32"   { OPCODE(F32_CONVERT_S_I32); return TOK(CONVERT); }
"f64.convert_s/i32"   { OPCODE(F64_CONVERT_S_I32); return TOK(CONVERT); }
"f32.convert_s/i64"   { OPCODE(F32_CONVERT_S_I64); return TOK(CONVERT); }
"f64.convert_s/i64"   { OPCODE(F64_CONVERT_S_I64); return TOK(CONVERT); }
"f32.convert_u/i32"   { OPCODE(F32_CONVERT_U_I32); return TOK(CONVERT); }
"f64.convert_u/i32"   { OPCODE(F64_CONVERT_U_I32); return TOK(CONVERT); }
"f32.convert_u/i64"   { OPCODE(F32_CONVERT_U_I64); return TOK(CONVERT); }
"f64.convert_u/i64"   { OPCODE(F64_CONVERT_U_I64); return TOK(CONVERT); }
"f64.promote/f32"     { OPCODE(F64_PROMOTE_F32); return TOK(CONVERT); }
"f32.demote/f64"      { OPCODE(F32_DEMOTE_F64); return TOK(CONVERT); }
"f32.reinterpret/i32" { OPCODE(F32_REINTERPRET_I32); return TOK(CONVERT); }
"i32.reinterpret/f32" { OPCODE(I32_REINTERPRET_F32); return TOK(CONVERT); }
"f64.reinterpret/i64" { OPCODE(F64_REINTERPRET_I64); return TOK(CONVERT); }
"i64.reinterpret/f64" { OPCODE(I64_REINTERPRET_F64); return TOK(CONVERT); }
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
[ \t\r]+
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

static WasmLexer wasm_new_lexer(WasmAllocator* allocator,
                                WasmLexerType type,
                                const char* filename) {
  WasmLexerExtra* extra =
      wasm_alloc_zero(allocator, sizeof(WasmLexerExtra), WASM_DEFAULT_ALIGN);
  if (!extra)
    return NULL;
  extra->allocator = allocator;
  extra->column = 1;
  extra->filename = filename;
  extra->type = type;

  if (WASM_FAILED(wasm_reserve_line_offsets(allocator, &extra->line_offsets,
                                            WASM_INITIAL_LINE_OFFSETS))) {
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
  yyset_extra(extra, lexer);
  s_lexer_allocator = NULL;
  return lexer;
}

WasmLexer wasm_new_file_lexer(WasmAllocator* allocator, const char* filename) {
  WasmLexer lexer = wasm_new_lexer(allocator, WASM_LEXER_TYPE_FILE, filename);
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  extra->file = fopen(filename, "r");
  if (!extra->file) {
    wasm_destroy_lexer(lexer);
    wasm_free(allocator, extra);
    return NULL;
  }
  yyset_in(extra->file, lexer);
  return lexer;
}

WasmLexer wasm_new_buffer_lexer(WasmAllocator* allocator, const char* filename,
                                const void* data, size_t size) {
  WasmLexer lexer = wasm_new_lexer(allocator, WASM_LEXER_TYPE_BUFFER, filename);
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  extra->buffer.data = data;
  extra->buffer.size = size;
  extra->buffer.read_offset = 0;
  return lexer;
}

static void read_input(WasmLexer lexer, char* buffer, size_t* result,
                       size_t max_size) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  if (extra->type == WASM_LEXER_TYPE_FILE) {
    *result = fread(buffer, 1, max_size, extra->file);
  } else {
    assert(extra->type == WASM_LEXER_TYPE_BUFFER);
    size_t read_size = max_size;
    size_t offset = extra->buffer.read_offset;
    size_t bytes_left = extra->buffer.size - offset;
    if (read_size > bytes_left)
      read_size = bytes_left;
    memcpy(buffer, (char*)extra->buffer.data + offset, read_size);
    extra->buffer.read_offset += read_size;
    *result = read_size;
  }
}

void wasm_destroy_lexer(WasmLexer lexer) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  if (extra->type == WASM_LEXER_TYPE_FILE && extra->file)
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

static size_t lexer_get_offset_from_line(WasmLexer lexer, int line) {
  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  if (line < 0 || (size_t)line >= extra->line_offsets.size)
    return WASM_INVALID_LINE_OFFSET;

  return extra->line_offsets.data[line];
}

static WasmResult lexer_get_offsets_from_line(WasmLexer lexer,
                                              int line,
                                              size_t* out_line_start,
                                              size_t* out_line_end) {
  size_t line_start = lexer_get_offset_from_line(lexer, line);
  size_t line_end = lexer_get_offset_from_line(lexer, line + 1);
  if (line_end == WASM_INVALID_LINE_OFFSET) {
    /* we haven't gotten to the next line yet. scan ahead to find it. */
    WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
    if (extra->type == WASM_LEXER_TYPE_BUFFER) {
      const char* buffer_start = extra->buffer.data;
      const char* buffer_line_start = buffer_start + line_start;
      const char* buffer_end = buffer_start + extra->buffer.size;

      const char* newline =
          memchr(buffer_line_start, '\n', buffer_end - buffer_line_start);
      line_end = (newline ? newline : buffer_end) - buffer_start;
    } else {
      assert(extra->type == WASM_LEXER_TYPE_FILE);
      FILE* lexer_file = extra->file;
      long old_offset = ftell(lexer_file);
      if (old_offset == -1)
        return WASM_ERROR;

      /* we haven't gotten to the next line yet. read the file to find it. */
      size_t offset = line_start;
      line_end = offset;
      if (fseek(lexer_file, offset, SEEK_SET) == -1)
        return WASM_ERROR;

      char buffer[256];
      while (1) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), lexer_file);
        if (bytes_read == 0)
          break;

        char* newline = memchr(buffer, '\n', bytes_read);
        if (newline) {
          line_end += (newline - buffer);
          break;
        } else {
          line_end += bytes_read;
        }
      }
      if (fseek(lexer_file, old_offset, SEEK_SET) == -1)
        return WASM_ERROR;
    }
  } else {
    /* don't include the newline */
    line_end--;
  }

  *out_line_start = line_start;
  *out_line_end = line_end;
  return WASM_OK;
}

static void clamp_source_line_offsets_to_location(size_t line_start,
                                                  size_t line_end,
                                                  int first_column,
                                                  int last_column,
                                                  size_t max_line_length,
                                                  size_t* out_new_line_start,
                                                  size_t* out_new_line_end) {
  size_t line_length = line_end - line_start;
  if (line_length > max_line_length) {
    size_t column_range = last_column - first_column;
    size_t center_on;
    if (column_range > max_line_length) {
      /* the column range doesn't fit, just center on first_column */
      center_on = first_column - 1;
    } else {
      /* the entire range fits, display it all in the center */
      center_on = (first_column + last_column) / 2 - 1;
    }
    if (center_on > max_line_length / 2)
      line_start += center_on - max_line_length / 2;
    if (line_start > line_end - max_line_length)
      line_start = line_end - max_line_length;
    line_end = line_start + max_line_length;
  }

  *out_new_line_start = line_start;
  *out_new_line_end = line_end;
}

WasmResult wasm_lexer_get_source_line(WasmLexer lexer,
                                      const WasmLocation* loc,
                                      size_t line_max_length,
                                      char* line,
                                      size_t* out_line_length,
                                      int* out_column_offset) {
  WasmResult result;
  size_t line_start;
  size_t line_end;
  result =
      lexer_get_offsets_from_line(lexer, loc->line, &line_start, &line_end);
  if (WASM_FAILED(result))
    return result;

  size_t new_line_start;
  size_t new_line_end;
  clamp_source_line_offsets_to_location(
      line_start, line_end, loc->first_column, loc->last_column,
      line_max_length, &new_line_start, &new_line_end);
  WasmBool has_start_ellipsis = line_start != new_line_start;
  WasmBool has_end_ellipsis = line_end != new_line_end;

  char* write_start = line;
  size_t line_length = new_line_end - new_line_start;
  size_t read_start = new_line_start;
  size_t read_length = line_length;
  if (has_start_ellipsis) {
    memcpy(line, "...", 3);
    read_start += 3;
    write_start += 3;
    read_length -= 3;
  }
  if (has_end_ellipsis) {
    memcpy(line + line_length - 3, "...", 3);
    read_length -= 3;
  }

  WasmLexerExtra* extra = wasm_lexer_get_extra(lexer);
  if (extra->type == WASM_LEXER_TYPE_BUFFER) {
    char* buffer_read_start = (char*)extra->buffer.data + read_start;
    memcpy(write_start, buffer_read_start, read_length);
  } else {
    assert(extra->type == WASM_LEXER_TYPE_FILE);
    FILE* lexer_file = extra->file;
    long old_offset = ftell(lexer_file);
    if (old_offset == -1)
      return WASM_ERROR;
    if (fseek(lexer_file, read_start, SEEK_SET) == -1)
      return WASM_ERROR;
    if (fread(write_start, 1, read_length, lexer_file) < read_length)
      return WASM_ERROR;
    if (fseek(lexer_file, old_offset, SEEK_SET) == -1)
      return WASM_ERROR;
  }

  line[line_length] = '\0';

  *out_line_length = line_length;
  *out_column_offset = new_line_start - line_start;
  return WASM_OK;
}
