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

#include "wasm-lexer.h"

#include <assert.h>
#include <stdio.h>

#include "wasm-config.h"

#include "wasm-allocator.h"
#include "wasm-parser.h"
#include "wasm-parser-lexer-shared.h"
#include "wasm-vector.h"

/* must be included after so some typedefs will be defined */
#include "wasm-bison-parser.h"

/*!max:re2c */

#define WASM_INITIAL_LEXER_BUFFER_SIZE (64 * 1024)
#define WASM_INITIAL_LINE_OFFSETS 10000

#define YY_USER_ACTION                        \
  {                                           \
    loc->filename = lexer->filename;          \
    loc->line = lexer->line;                  \
    loc->first_column = COLUMN(lexer->token); \
    loc->last_column = COLUMN(lexer->cursor); \
  }

#define RETURN(name) \
  YY_USER_ACTION;    \
  return WASM_TOKEN_TYPE_##name

#define ERROR(...) \
  YY_USER_ACTION;  \
  wasm_parser_error(loc, lexer, parser, __VA_ARGS__)

#define BEGIN(c)       \
  do {                 \
    cond = YYCOND_##c; \
  } while (0)
#define FILL(n)                                     \
  do {                                              \
    if (WASM_FAILED(fill(loc, lexer, parser, n))) { \
      RETURN(EOF);                                  \
      continue;                                     \
    }                                               \
  } while (0)

#define yytext (lexer->token)
#define yyleng (lexer->cursor - lexer->token)

/* p must be a pointer somewhere in the lexer buffer */
#define FILE_OFFSET(p) ((p) - (lexer->buffer) + lexer->buffer_file_offset)
#define COLUMN(p) (FILE_OFFSET(p) - lexer->last_line_file_offset + 1)

#define COMMENT_NESTING (lexer->comment_nesting)
#define NEWLINE                                                        \
  do {                                                                 \
    lexer->line++;                                                     \
    *wasm_append_line_offset(lexer->allocator, &lexer->line_offsets) = \
        lexer->last_line_file_offset = FILE_OFFSET(lexer->cursor);     \
  } while (0)

#define TEXT                 \
  lval->text.start = yytext; \
  lval->text.length = yyleng

#define TEXT_AT(offset)               \
  lval->text.start = yytext + offset; \
  lval->text.length = yyleng - offset

#define TYPE(type_) lval->type = WASM_TYPE_##type_

#define OPCODE(name) lval->opcode = WASM_OPCODE_##name

#define LITERAL(type_)                      \
  lval->literal.type = WASM_LITERAL_TYPE_##type_; \
  lval->literal.text.start = yytext;              \
  lval->literal.text.length = yyleng

static WasmResult fill(WasmLocation* loc,
                       WasmLexer* lexer,
                       WasmParser* parser,
                       size_t need) {
  if (lexer->eof)
    return WASM_ERROR;
  size_t free = lexer->token - lexer->buffer;
  assert((size_t)(lexer->cursor - lexer->buffer) >= free);
  /* our buffer is too small, need to realloc */
  if (free < need) {
    char* old_buffer = lexer->buffer;
    size_t old_buffer_size = lexer->buffer_size;
    size_t new_buffer_size =
        old_buffer_size ? old_buffer_size * 2 : WASM_INITIAL_LEXER_BUFFER_SIZE;
    /* make sure there is enough space for the bytes requested (need) and an
     * additional YYMAXFILL bytes which is needed for the re2c lexer
     * implementation when the eof is reached */
    while ((new_buffer_size - old_buffer_size) + free < need + YYMAXFILL)
      new_buffer_size *= 2;

    /* TODO(binji): could just alloc instead, because we know we'll need to
     * memmove below */
    char* new_buffer = wasm_realloc(lexer->allocator, lexer->buffer,
                                    new_buffer_size, WASM_DEFAULT_ALIGN);
    if (new_buffer == NULL) {
      wasm_parser_error(loc, lexer, parser,
                        "unable to reallocate lexer buffer.");
      return WASM_ERROR;
    }
    memmove(new_buffer, lexer->token, lexer->limit - lexer->token);
    lexer->buffer = new_buffer;
    lexer->buffer_size = new_buffer_size;
    lexer->token = new_buffer + (lexer->token - old_buffer) - free;
    lexer->marker = new_buffer + (lexer->marker - old_buffer) - free;
    lexer->cursor = new_buffer + (lexer->cursor - old_buffer) - free;
    lexer->limit = new_buffer + (lexer->limit - old_buffer) - free;
    lexer->buffer_file_offset += free;
    free += new_buffer_size - old_buffer_size;
  } else {
    /* shift everything down to make more room in the buffer */
    memmove(lexer->buffer, lexer->token, lexer->limit - lexer->token);
    lexer->token -= free;
    lexer->marker -= free;
    lexer->cursor -= free;
    lexer->limit -= free;
    lexer->buffer_file_offset += free;
  }
  /* read the new data into the buffer */
  if (lexer->source.type == WASM_LEXER_SOURCE_TYPE_FILE) {
    lexer->limit += fread(lexer->limit, 1, free, lexer->source.file);
  } else {
    /* TODO(binji): could lex directly from buffer */
    assert(lexer->source.type == WASM_LEXER_SOURCE_TYPE_BUFFER);
    size_t read_size = free;
    size_t offset = lexer->source.buffer.read_offset;
    size_t bytes_left = lexer->source.buffer.size - offset;
    if (read_size > bytes_left)
      read_size = bytes_left;
    memcpy(lexer->buffer, (char*)lexer->source.buffer.data + offset,
           read_size);
    lexer->source.buffer.read_offset += read_size;
    lexer->limit += read_size;
  }
  /* if at the end of file, need to fill YYMAXFILL more characters with "fake
   * characters", that are not a lexeme nor a lexeme suffix. see
   * http://re2c.org/examples/example_03.html */
  if (lexer->limit < lexer->buffer + lexer->buffer_size - YYMAXFILL) {
    lexer->eof = WASM_TRUE;
    memset(lexer->limit, 0, YYMAXFILL);
    lexer->limit += YYMAXFILL;
  }
  return WASM_OK;
}

int wasm_lexer_lex(WASM_PARSER_STYPE* lval,
                   WASM_PARSER_LTYPE* loc,
                   WasmLexer* lexer,
                   WasmParser* parser) {
  enum {
    YYCOND_INIT,
    YYCOND_BAD_TEXT,
    YYCOND_LINE_COMMENT,
    YYCOND_BLOCK_COMMENT,
    YYCOND_i = YYCOND_INIT,
  } cond = YYCOND_INIT;

  for (;;) {
    lexer->token = lexer->cursor;
  /*!re2c
    re2c:condprefix = YYCOND_;
    re2c:condenumprefix = YYCOND_;
    re2c:define:YYCTYPE = char;
    re2c:define:YYCURSOR = lexer->cursor;
    re2c:define:YYMARKER = lexer->marker;
    re2c:define:YYLIMIT = lexer->limit;
    re2c:define:YYFILL = "FILL";
    re2c:define:YYGETCONDITION = "cond";
    re2c:define:YYGETCONDITION:naked = 1;
    re2c:define:YYSETCONDITION = "cond = @@;";
    re2c:define:YYSETCONDITION:naked = 1;

    space =     [ \t];
    digit =     [0-9];
    digits =    [0-9]+;
    hexdigit =  [0-9a-fA-F];
    letter =    [a-zA-Z];
    symbol =    [+\-*\/\\\^~=<>!?@#$%&|:`.];
    tick =      "`";
    escape =    [nt\\'"];
    character = [^"\\\x00-\x1f\x7f] | "\\" escape | "\\" hexdigit hexdigit;
    sign =      [+-]?;
    num =       sign digit+;
    hexnum =    sign "0x" hexdigit+;
    int =       num | hexnum;
    float0 =    num "." digit*;
    float1 =    num ("." digit*)? [eE] num;
    hexfloat =  sign "0x" hexdigit+ "."? hexdigit* "p" sign digit+;
    infinity =  sign "infinity";
    nan =       sign "nan" | sign "nan:0x" hexdigit+;
    float =     float0 | float1;
    text =      '"' character* '"';
    atom =      (letter | digit | "_" | tick | symbol)+;
    name =      "$" atom;
    EOF =       "\x00";

    <i> "("                   { RETURN(LPAR); }
    <i> ")"                   { RETURN(RPAR); }
    <i> int                   { LITERAL(INT); RETURN(INT); }
    <i> float                 { LITERAL(FLOAT); RETURN(FLOAT); }
    <i> hexfloat              { LITERAL(HEXFLOAT); RETURN(FLOAT); }
    <i> infinity              { LITERAL(INFINITY); RETURN(FLOAT); }
    <i> nan                   { LITERAL(NAN); RETURN(FLOAT); }
    <i> text                  { TEXT; RETURN(TEXT); }
    <i> '"' => BAD_TEXT       { continue; }
    <BAD_TEXT> character      { continue; }
    <BAD_TEXT> "\n" => i      { ERROR("newline in string"); NEWLINE; continue; }
    <BAD_TEXT> "\\".          { ERROR("bad escape \"%.*s\"", yyleng, yytext); continue; }
    <BAD_TEXT> '"' => i       { TEXT; RETURN(TEXT); }
    <BAD_TEXT> EOF            { ERROR("unexpected EOF"); RETURN(EOF); }
    <BAD_TEXT> *              { ERROR("illegal character in string"); continue; }
    <i> "i32"                 { TYPE(I32); RETURN(VALUE_TYPE); }
    <i> "i64"                 { TYPE(I64); RETURN(VALUE_TYPE); }
    <i> "f32"                 { TYPE(F32); RETURN(VALUE_TYPE); }
    <i> "f64"                 { TYPE(F64); RETURN(VALUE_TYPE); }
    <i> "nop"                 { RETURN(NOP); }
    <i> "block"               { RETURN(BLOCK); }
    <i> "if"                  { RETURN(IF); }
    <i> "if_else"             { RETURN(IF); }
    <i> "then"                { RETURN(THEN); }
    <i> "else"                { RETURN(ELSE); }
    <i> "loop"                { RETURN(LOOP); }
    <i> "br"                  { RETURN(BR); }
    <i> "br_if"               { RETURN(BR_IF); }
    <i> "br_table"            { RETURN(BR_TABLE); }
    <i> "case"                { RETURN(CASE); }
    <i> "call"                { RETURN(CALL); }
    <i> "call_import"         { RETURN(CALL_IMPORT); }
    <i> "call_indirect"       { RETURN(CALL_INDIRECT); }
    <i> "return"              { RETURN(RETURN); }
    <i> "get_local"           { RETURN(GET_LOCAL); }
    <i> "set_local"           { RETURN(SET_LOCAL); }
    <i> "i32.load"            { OPCODE(I32_LOAD); RETURN(LOAD); }
    <i> "i64.load"            { OPCODE(I64_LOAD); RETURN(LOAD); }
    <i> "f32.load"            { OPCODE(F32_LOAD); RETURN(LOAD); }
    <i> "f64.load"            { OPCODE(F64_LOAD); RETURN(LOAD); }
    <i> "i32.store"           { OPCODE(I32_STORE); RETURN(STORE); }
    <i> "i64.store"           { OPCODE(I64_STORE); RETURN(STORE); }
    <i> "f32.store"           { OPCODE(F32_STORE); RETURN(STORE); }
    <i> "f64.store"           { OPCODE(F64_STORE); RETURN(STORE); }
    <i> "i32.load8_s"         { OPCODE(I32_LOAD8_S); RETURN(LOAD); }
    <i> "i64.load8_s"         { OPCODE(I64_LOAD8_S); RETURN(LOAD); }
    <i> "i32.load8_u"         { OPCODE(I32_LOAD8_U); RETURN(LOAD); }
    <i> "i64.load8_u"         { OPCODE(I64_LOAD8_U); RETURN(LOAD); }
    <i> "i32.load16_s"        { OPCODE(I32_LOAD16_S); RETURN(LOAD); }
    <i> "i64.load16_s"        { OPCODE(I64_LOAD16_S); RETURN(LOAD); }
    <i> "i32.load16_u"        { OPCODE(I32_LOAD16_U); RETURN(LOAD); }
    <i> "i64.load16_u"        { OPCODE(I64_LOAD16_U); RETURN(LOAD); }
    <i> "i64.load32_s"        { OPCODE(I64_LOAD32_S); RETURN(LOAD); }
    <i> "i64.load32_u"        { OPCODE(I64_LOAD32_U); RETURN(LOAD); }
    <i> "i32.store8"          { OPCODE(I32_STORE8); RETURN(STORE); }
    <i> "i64.store8"          { OPCODE(I64_STORE8); RETURN(STORE); }
    <i> "i32.store16"         { OPCODE(I32_STORE16); RETURN(STORE); }
    <i> "i64.store16"         { OPCODE(I64_STORE16); RETURN(STORE); }
    <i> "i64.store32"         { OPCODE(I64_STORE32); RETURN(STORE); }
    <i> "offset="digits       { TEXT_AT(7); RETURN(OFFSET); }
    <i> "align="digits        { TEXT_AT(6); RETURN(ALIGN); }
    <i> "i32.const"           { TYPE(I32); RETURN(CONST); }
    <i> "i64.const"           { TYPE(I64); RETURN(CONST); }
    <i> "f32.const"           { TYPE(F32); RETURN(CONST); }
    <i> "f64.const"           { TYPE(F64); RETURN(CONST); }
    <i> "i32.eqz"             { OPCODE(I32_EQZ); RETURN(CONVERT); }
    <i> "i64.eqz"             { OPCODE(I64_EQZ); RETURN(CONVERT); }
    <i> "i32.clz"             { OPCODE(I32_CLZ); RETURN(UNARY); }
    <i> "i64.clz"             { OPCODE(I64_CLZ); RETURN(UNARY); }
    <i> "i32.ctz"             { OPCODE(I32_CTZ); RETURN(UNARY); }
    <i> "i64.ctz"             { OPCODE(I64_CTZ); RETURN(UNARY); }
    <i> "i32.popcnt"          { OPCODE(I32_POPCNT); RETURN(UNARY); }
    <i> "i64.popcnt"          { OPCODE(I64_POPCNT); RETURN(UNARY); }
    <i> "f32.neg"             { OPCODE(F32_NEG); RETURN(UNARY); }
    <i> "f64.neg"             { OPCODE(F64_NEG); RETURN(UNARY); }
    <i> "f32.abs"             { OPCODE(F32_ABS); RETURN(UNARY); }
    <i> "f64.abs"             { OPCODE(F64_ABS); RETURN(UNARY); }
    <i> "f32.sqrt"            { OPCODE(F32_SQRT); RETURN(UNARY); }
    <i> "f64.sqrt"            { OPCODE(F64_SQRT); RETURN(UNARY); }
    <i> "f32.ceil"            { OPCODE(F32_CEIL); RETURN(UNARY); }
    <i> "f64.ceil"            { OPCODE(F64_CEIL); RETURN(UNARY); }
    <i> "f32.floor"           { OPCODE(F32_FLOOR); RETURN(UNARY); }
    <i> "f64.floor"           { OPCODE(F64_FLOOR); RETURN(UNARY); }
    <i> "f32.trunc"           { OPCODE(F32_TRUNC); RETURN(UNARY); }
    <i> "f64.trunc"           { OPCODE(F64_TRUNC); RETURN(UNARY); }
    <i> "f32.nearest"         { OPCODE(F32_NEAREST); RETURN(UNARY); }
    <i> "f64.nearest"         { OPCODE(F64_NEAREST); RETURN(UNARY); }
    <i> "i32.add"             { OPCODE(I32_ADD); RETURN(BINARY); }
    <i> "i64.add"             { OPCODE(I64_ADD); RETURN(BINARY); }
    <i> "i32.sub"             { OPCODE(I32_SUB); RETURN(BINARY); }
    <i> "i64.sub"             { OPCODE(I64_SUB); RETURN(BINARY); }
    <i> "i32.mul"             { OPCODE(I32_MUL); RETURN(BINARY); }
    <i> "i64.mul"             { OPCODE(I64_MUL); RETURN(BINARY); }
    <i> "i32.div_s"           { OPCODE(I32_DIV_S); RETURN(BINARY); }
    <i> "i64.div_s"           { OPCODE(I64_DIV_S); RETURN(BINARY); }
    <i> "i32.div_u"           { OPCODE(I32_DIV_U); RETURN(BINARY); }
    <i> "i64.div_u"           { OPCODE(I64_DIV_U); RETURN(BINARY); }
    <i> "i32.rem_s"           { OPCODE(I32_REM_S); RETURN(BINARY); }
    <i> "i64.rem_s"           { OPCODE(I64_REM_S); RETURN(BINARY); }
    <i> "i32.rem_u"           { OPCODE(I32_REM_U); RETURN(BINARY); }
    <i> "i64.rem_u"           { OPCODE(I64_REM_U); RETURN(BINARY); }
    <i> "i32.and"             { OPCODE(I32_AND); RETURN(BINARY); }
    <i> "i64.and"             { OPCODE(I64_AND); RETURN(BINARY); }
    <i> "i32.or"              { OPCODE(I32_OR); RETURN(BINARY); }
    <i> "i64.or"              { OPCODE(I64_OR); RETURN(BINARY); }
    <i> "i32.xor"             { OPCODE(I32_XOR); RETURN(BINARY); }
    <i> "i64.xor"             { OPCODE(I64_XOR); RETURN(BINARY); }
    <i> "i32.shl"             { OPCODE(I32_SHL); RETURN(BINARY); }
    <i> "i64.shl"             { OPCODE(I64_SHL); RETURN(BINARY); }
    <i> "i32.shr_s"           { OPCODE(I32_SHR_S); RETURN(BINARY); }
    <i> "i64.shr_s"           { OPCODE(I64_SHR_S); RETURN(BINARY); }
    <i> "i32.shr_u"           { OPCODE(I32_SHR_U); RETURN(BINARY); }
    <i> "i64.shr_u"           { OPCODE(I64_SHR_U); RETURN(BINARY); }
    <i> "i32.rotl"            { OPCODE(I32_ROTL); RETURN(BINARY); }
    <i> "i64.rotl"            { OPCODE(I64_ROTL); RETURN(BINARY); }
    <i> "i32.rotr"            { OPCODE(I32_ROTR); RETURN(BINARY); }
    <i> "i64.rotr"            { OPCODE(I64_ROTR); RETURN(BINARY); }
    <i> "f32.add"             { OPCODE(F32_ADD); RETURN(BINARY); }
    <i> "f64.add"             { OPCODE(F64_ADD); RETURN(BINARY); }
    <i> "f32.sub"             { OPCODE(F32_SUB); RETURN(BINARY); }
    <i> "f64.sub"             { OPCODE(F64_SUB); RETURN(BINARY); }
    <i> "f32.mul"             { OPCODE(F32_MUL); RETURN(BINARY); }
    <i> "f64.mul"             { OPCODE(F64_MUL); RETURN(BINARY); }
    <i> "f32.div"             { OPCODE(F32_DIV); RETURN(BINARY); }
    <i> "f64.div"             { OPCODE(F64_DIV); RETURN(BINARY); }
    <i> "f32.min"             { OPCODE(F32_MIN); RETURN(BINARY); }
    <i> "f64.min"             { OPCODE(F64_MIN); RETURN(BINARY); }
    <i> "f32.max"             { OPCODE(F32_MAX); RETURN(BINARY); }
    <i> "f64.max"             { OPCODE(F64_MAX); RETURN(BINARY); }
    <i> "f32.copysign"        { OPCODE(F32_COPYSIGN); RETURN(BINARY); }
    <i> "f64.copysign"        { OPCODE(F64_COPYSIGN); RETURN(BINARY); }
    <i> "i32.eq"              { OPCODE(I32_EQ); RETURN(COMPARE); }
    <i> "i64.eq"              { OPCODE(I64_EQ); RETURN(COMPARE); }
    <i> "i32.ne"              { OPCODE(I32_NE); RETURN(COMPARE); }
    <i> "i64.ne"              { OPCODE(I64_NE); RETURN(COMPARE); }
    <i> "i32.lt_s"            { OPCODE(I32_LT_S); RETURN(COMPARE); }
    <i> "i64.lt_s"            { OPCODE(I64_LT_S); RETURN(COMPARE); }
    <i> "i32.lt_u"            { OPCODE(I32_LT_U); RETURN(COMPARE); }
    <i> "i64.lt_u"            { OPCODE(I64_LT_U); RETURN(COMPARE); }
    <i> "i32.le_s"            { OPCODE(I32_LE_S); RETURN(COMPARE); }
    <i> "i64.le_s"            { OPCODE(I64_LE_S); RETURN(COMPARE); }
    <i> "i32.le_u"            { OPCODE(I32_LE_U); RETURN(COMPARE); }
    <i> "i64.le_u"            { OPCODE(I64_LE_U); RETURN(COMPARE); }
    <i> "i32.gt_s"            { OPCODE(I32_GT_S); RETURN(COMPARE); }
    <i> "i64.gt_s"            { OPCODE(I64_GT_S); RETURN(COMPARE); }
    <i> "i32.gt_u"            { OPCODE(I32_GT_U); RETURN(COMPARE); }
    <i> "i64.gt_u"            { OPCODE(I64_GT_U); RETURN(COMPARE); }
    <i> "i32.ge_s"            { OPCODE(I32_GE_S); RETURN(COMPARE); }
    <i> "i64.ge_s"            { OPCODE(I64_GE_S); RETURN(COMPARE); }
    <i> "i32.ge_u"            { OPCODE(I32_GE_U); RETURN(COMPARE); }
    <i> "i64.ge_u"            { OPCODE(I64_GE_U); RETURN(COMPARE); }
    <i> "f32.eq"              { OPCODE(F32_EQ); RETURN(COMPARE); }
    <i> "f64.eq"              { OPCODE(F64_EQ); RETURN(COMPARE); }
    <i> "f32.ne"              { OPCODE(F32_NE); RETURN(COMPARE); }
    <i> "f64.ne"              { OPCODE(F64_NE); RETURN(COMPARE); }
    <i> "f32.lt"              { OPCODE(F32_LT); RETURN(COMPARE); }
    <i> "f64.lt"              { OPCODE(F64_LT); RETURN(COMPARE); }
    <i> "f32.le"              { OPCODE(F32_LE); RETURN(COMPARE); }
    <i> "f64.le"              { OPCODE(F64_LE); RETURN(COMPARE); }
    <i> "f32.gt"              { OPCODE(F32_GT); RETURN(COMPARE); }
    <i> "f64.gt"              { OPCODE(F64_GT); RETURN(COMPARE); }
    <i> "f32.ge"              { OPCODE(F32_GE); RETURN(COMPARE); }
    <i> "f64.ge"              { OPCODE(F64_GE); RETURN(COMPARE); }
    <i> "i64.extend_s/i32"    { OPCODE(I64_EXTEND_S_I32); RETURN(CONVERT); }
    <i> "i64.extend_u/i32"    { OPCODE(I64_EXTEND_U_I32); RETURN(CONVERT); }
    <i> "i32.wrap/i64"        { OPCODE(I32_WRAP_I64); RETURN(CONVERT); }
    <i> "i32.trunc_s/f32"     { OPCODE(I32_TRUNC_S_F32); RETURN(CONVERT); }
    <i> "i64.trunc_s/f32"     { OPCODE(I64_TRUNC_S_F32); RETURN(CONVERT); }
    <i> "i32.trunc_s/f64"     { OPCODE(I32_TRUNC_S_F64); RETURN(CONVERT); }
    <i> "i64.trunc_s/f64"     { OPCODE(I64_TRUNC_S_F64); RETURN(CONVERT); }
    <i> "i32.trunc_u/f32"     { OPCODE(I32_TRUNC_U_F32); RETURN(CONVERT); }
    <i> "i64.trunc_u/f32"     { OPCODE(I64_TRUNC_U_F32); RETURN(CONVERT); }
    <i> "i32.trunc_u/f64"     { OPCODE(I32_TRUNC_U_F64); RETURN(CONVERT); }
    <i> "i64.trunc_u/f64"     { OPCODE(I64_TRUNC_U_F64); RETURN(CONVERT); }
    <i> "f32.convert_s/i32"   { OPCODE(F32_CONVERT_S_I32); RETURN(CONVERT); }
    <i> "f64.convert_s/i32"   { OPCODE(F64_CONVERT_S_I32); RETURN(CONVERT); }
    <i> "f32.convert_s/i64"   { OPCODE(F32_CONVERT_S_I64); RETURN(CONVERT); }
    <i> "f64.convert_s/i64"   { OPCODE(F64_CONVERT_S_I64); RETURN(CONVERT); }
    <i> "f32.convert_u/i32"   { OPCODE(F32_CONVERT_U_I32); RETURN(CONVERT); }
    <i> "f64.convert_u/i32"   { OPCODE(F64_CONVERT_U_I32); RETURN(CONVERT); }
    <i> "f32.convert_u/i64"   { OPCODE(F32_CONVERT_U_I64); RETURN(CONVERT); }
    <i> "f64.convert_u/i64"   { OPCODE(F64_CONVERT_U_I64); RETURN(CONVERT); }
    <i> "f64.promote/f32"     { OPCODE(F64_PROMOTE_F32); RETURN(CONVERT); }
    <i> "f32.demote/f64"      { OPCODE(F32_DEMOTE_F64); RETURN(CONVERT); }
    <i> "f32.reinterpret/i32" { OPCODE(F32_REINTERPRET_I32); RETURN(CONVERT); }
    <i> "i32.reinterpret/f32" { OPCODE(I32_REINTERPRET_F32); RETURN(CONVERT); }
    <i> "f64.reinterpret/i64" { OPCODE(F64_REINTERPRET_I64); RETURN(CONVERT); }
    <i> "i64.reinterpret/f64" { OPCODE(I64_REINTERPRET_F64); RETURN(CONVERT); }
    <i> "select"              { RETURN(SELECT); }
    <i> "unreachable"         { RETURN(UNREACHABLE); }
    <i> "memory_size"         { RETURN(MEMORY_SIZE); }
    <i> "grow_memory"         { RETURN(GROW_MEMORY); }
    <i> "type"                { RETURN(TYPE); }
    <i> "func"                { RETURN(FUNC); }
    <i> "param"               { RETURN(PARAM); }
    <i> "result"              { RETURN(RESULT); }
    <i> "local"               { RETURN(LOCAL); }
    <i> "module"              { RETURN(MODULE); }
    <i> "memory"              { RETURN(MEMORY); }
    <i> "segment"             { RETURN(SEGMENT); }
    <i> "start"               { RETURN(START); }
    <i> "import"              { RETURN(IMPORT); }
    <i> "export"              { RETURN(EXPORT); }
    <i> "table"               { RETURN(TABLE); }
    <i> "assert_invalid"      { RETURN(ASSERT_INVALID); }
    <i> "assert_return"       { RETURN(ASSERT_RETURN); }
    <i> "assert_return_nan"   { RETURN(ASSERT_RETURN_NAN); }
    <i> "assert_trap"         { RETURN(ASSERT_TRAP); }
    <i> "invoke"              { RETURN(INVOKE); }
    <i> name                  { TEXT; RETURN(VAR); }

    <i> ";;" => LINE_COMMENT  { continue; }
    <LINE_COMMENT> "\n" => i  { NEWLINE; continue; }
    <LINE_COMMENT> *          { continue; }
    <i> "(;" => BLOCK_COMMENT { COMMENT_NESTING = 1; continue; }
    <BLOCK_COMMENT> "(;"      { COMMENT_NESTING++; continue; }
    <BLOCK_COMMENT> ";)"      { if (--COMMENT_NESTING == 0) BEGIN(i); continue; }
    <BLOCK_COMMENT> "\n"      { NEWLINE; continue; }
    <BLOCK_COMMENT> EOF       { ERROR("unexpected EOF"); RETURN(EOF); }
    <BLOCK_COMMENT> *         { continue; }
    <i> "\n"                  { NEWLINE; continue; }
    <i> [ \t\r]+              { continue; }
    <i> atom                  { ERROR("unexpected token \"%.*s\"",
                                      yyleng, yytext);
                                continue;}
    <*> EOF                   { RETURN(EOF); }
    <*> *                     { ERROR("unexpected char"); continue; }
   */
  }
}

static WasmLexer* wasm_new_lexer(WasmAllocator* allocator,
                                 WasmLexerSourceType type,
                                 const char* filename) {
  WasmLexer* lexer =
      wasm_alloc_zero(allocator, sizeof(WasmLexer), WASM_DEFAULT_ALIGN);
  lexer->allocator = allocator;
  lexer->line = 1;
  lexer->filename = filename;
  lexer->source.type = type;

  if (WASM_FAILED(wasm_reserve_line_offsets(allocator, &lexer->line_offsets,
                                            WASM_INITIAL_LINE_OFFSETS))) {
    wasm_free(allocator, lexer);
    return NULL;
  }

  /* Skip the zeroeth line, and make the first line offset 0 */
  assert(lexer->line_offsets.capacity >= 2);
  *wasm_append_line_offset(allocator, &lexer->line_offsets) = 0;
  *wasm_append_line_offset(allocator, &lexer->line_offsets) = 0;
  return lexer;
}

WasmLexer* wasm_new_file_lexer(WasmAllocator* allocator, const char* filename) {
  WasmLexer* lexer =
      wasm_new_lexer(allocator, WASM_LEXER_SOURCE_TYPE_FILE, filename);
  lexer->source.file = fopen(filename, "r");
  if (!lexer->source.file) {
    wasm_destroy_lexer(lexer);
    wasm_free(allocator, lexer);
    return NULL;
  }
  return lexer;
}

WasmLexer* wasm_new_buffer_lexer(WasmAllocator* allocator, const char* filename,
                                const void* data, size_t size) {
  WasmLexer* lexer =
      wasm_new_lexer(allocator, WASM_LEXER_SOURCE_TYPE_BUFFER, filename);
  lexer->source.buffer.data = data;
  lexer->source.buffer.size = size;
  lexer->source.buffer.read_offset = 0;
  return lexer;
}

void wasm_destroy_lexer(WasmLexer* lexer) {
  if (lexer->source.type == WASM_LEXER_SOURCE_TYPE_FILE && lexer->source.file)
    fclose(lexer->source.file);
  wasm_destroy_line_offset_vector(lexer->allocator, &lexer->line_offsets);
  wasm_free(lexer->allocator, lexer->buffer);
  wasm_free(lexer->allocator, lexer);
}

WasmAllocator* wasm_lexer_get_allocator(WasmLexer* lexer) {
  return lexer->allocator;
}

static size_t lexer_get_offset_from_line(WasmLexer* lexer, int line) {
  if (line < 0 || (size_t)line >= lexer->line_offsets.size)
    return WASM_INVALID_LINE_OFFSET;

  return lexer->line_offsets.data[line];
}

static WasmResult lexer_get_offsets_from_line(WasmLexer* lexer,
                                              int line,
                                              size_t* out_line_start,
                                              size_t* out_line_end) {
  size_t line_start = lexer_get_offset_from_line(lexer, line);
  size_t line_end = lexer_get_offset_from_line(lexer, line + 1);
  if (line_end == WASM_INVALID_LINE_OFFSET) {
    /* we haven't gotten to the next line yet. scan ahead to find it. */
    if (lexer->source.type == WASM_LEXER_SOURCE_TYPE_BUFFER) {
      const char* buffer_start = lexer->source.buffer.data;
      const char* buffer_line_start = buffer_start + line_start;
      const char* buffer_end = buffer_start + lexer->source.buffer.size;

      const char* newline =
          memchr(buffer_line_start, '\n', buffer_end - buffer_line_start);
      line_end = (newline ? newline : buffer_end) - buffer_start;
    } else {
      assert(lexer->source.type == WASM_LEXER_SOURCE_TYPE_FILE);
      FILE* lexer_file = lexer->source.file;
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

WasmResult wasm_lexer_get_source_line(WasmLexer* lexer,
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

  if (lexer->source.type == WASM_LEXER_SOURCE_TYPE_BUFFER) {
    char* buffer_read_start = (char*)lexer->source.buffer.data + read_start;
    memcpy(write_start, buffer_read_start, read_length);
  } else {
    assert(lexer->source.type == WASM_LEXER_SOURCE_TYPE_FILE);
    FILE* lexer_file = lexer->source.file;
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
