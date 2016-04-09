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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-literal.h"
#include "wasm-parser.h"
#include "wasm-parser-lexer-shared.h"

#define DUPTEXT(dst, src)                                                   \
  (dst).start = wasm_strndup(parser->allocator, (src).start, (src).length); \
  (dst).length = (src).length

#define CHECK_ALLOC_(cond)                                       \
  if (!(cond)) {                                                 \
    WasmLocation loc;                                            \
    loc.filename = __FILE__;                                     \
    loc.line = __LINE__;                                         \
    loc.first_column = loc.last_column = 0;                      \
    wasm_parser_error(&loc, lexer, parser, "allocation failed"); \
    YYERROR;                                                     \
  }

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v) != NULL)
#define CHECK_ALLOC_STR(s) CHECK_ALLOC_((s).start != NULL)

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

static WasmResult parse_const(WasmType type, WasmLiteralType literal_type,
                              const char* s, const char* end, WasmConst* out);
static WasmResult dup_string_contents(WasmAllocator*, WasmStringSlice* text,
                                      void** out_data, size_t* out_size);

static WasmExpr* new_block_expr_with_one(WasmAllocator* allocator,
                                         WasmExpr* expr);
static WasmExpr* new_block_expr_with_list(WasmAllocator* allocator,
                                          WasmLabel* label,
                                          WasmExprPtrVector* exprs);

#define wasm_parser_lex wasm_lexer_lex

%}

%define api.prefix {wasm_parser_}
%define api.pure true
%define api.value.type {WasmToken}
%define api.token.prefix {WASM_TOKEN_TYPE_}
%define parse.error verbose
%lex-param {WasmLexer lexer} {WasmParser* parser}
%parse-param {WasmLexer lexer} {WasmParser* parser}
%locations

%token LPAR "("
%token RPAR ")"
%token INT FLOAT TEXT VAR VALUE_TYPE
%token NOP BLOCK IF THEN ELSE LOOP BR BR_IF BR_TABLE CASE
%token CALL CALL_IMPORT CALL_INDIRECT RETURN
%token GET_LOCAL SET_LOCAL LOAD STORE OFFSET ALIGN
%token CONST UNARY BINARY COMPARE CONVERT SELECT
%token FUNC START TYPE PARAM RESULT LOCAL
%token MODULE MEMORY SEGMENT IMPORT EXPORT TABLE
%token UNREACHABLE MEMORY_SIZE GROW_MEMORY
%token ASSERT_INVALID ASSERT_RETURN ASSERT_RETURN_NAN ASSERT_TRAP INVOKE
%token EOF 0 "EOF"

%type<opcode> BINARY COMPARE CONVERT LOAD STORE UNARY
%type<text> ALIGN OFFSET TEXT VAR
%type<type> SELECT
%type<type> CONST VALUE_TYPE
%type<literal> INT FLOAT

%type<command> cmd
%type<commands> cmd_list
%type<const_> const const_opt
%type<consts> const_list
%type<export_> export
%type<export_memory> export_memory
%type<expr> expr expr1 expr_opt
%type<exprs> expr_list non_empty_expr_list
%type<func> func func_info
%type<func_sig> func_type
%type<func_type> type_def
%type<import> import
%type<memory> memory
%type<module> module module_fields
%type<literal> literal
%type<script> script
%type<segment> segment string_contents
%type<segments> segment_list
%type<type_bindings> local_list param_list
%type<text> bind_var labeling quoted_text
%type<type> result
%type<types> value_type_list
%type<u32> align initial_pages max_pages segment_address
%type<u64> offset
%type<vars> table var_list
%type<var> start type_use var

%destructor { wasm_destroy_string_slice(parser->allocator, &$$); } bind_var labeling quoted_text
%destructor { wasm_destroy_string_slice(parser->allocator, &$$.text); } literal
%destructor { wasm_destroy_type_vector(parser->allocator, &$$); } value_type_list
%destructor { wasm_destroy_var(parser->allocator, &$$); } var
%destructor { wasm_destroy_var_vector_and_elements(parser->allocator, &$$); } table var_list
%destructor { wasm_destroy_expr_ptr(parser->allocator, &$$); } expr expr1 expr_opt
%destructor { wasm_destroy_expr_ptr_vector_and_elements(parser->allocator, &$$); } expr_list non_empty_expr_list
%destructor { wasm_destroy_type_bindings(parser->allocator, &$$); } local_list param_list
%destructor { wasm_destroy_func(parser->allocator, $$); wasm_free(parser->allocator, $$); } func func_info
%destructor { wasm_destroy_segment(parser->allocator, &$$); } segment string_contents
%destructor { wasm_destroy_segment_vector_and_elements(parser->allocator, &$$); } segment_list
%destructor { wasm_destroy_memory(parser->allocator, &$$); } memory
%destructor { wasm_destroy_func_signature(parser->allocator, &$$); } func_type
%destructor { wasm_destroy_func_type(parser->allocator, &$$); } type_def
%destructor { wasm_destroy_import(parser->allocator, $$); wasm_free(parser->allocator, $$); } import
%destructor { wasm_destroy_export(parser->allocator, &$$); } export
%destructor { wasm_destroy_module(parser->allocator, $$); wasm_free(parser->allocator, $$); } module module_fields
%destructor { wasm_destroy_const_vector(parser->allocator, &$$); } const_list
%destructor { wasm_destroy_command(parser->allocator, $$); wasm_free(parser->allocator, $$); } cmd
%destructor { wasm_destroy_command_vector_and_elements(parser->allocator, &$$); } cmd_list
%destructor { wasm_destroy_script(&$$); } script

%nonassoc LOW
%nonassoc VAR

%start script_start

%%

/* Types */

value_type_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | value_type_list VALUE_TYPE {
      $$ = $1;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &$$, &$2));
    }
;
func_type :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | LPAR PARAM value_type_list RPAR {
      $$.result_type = WASM_TYPE_VOID;
      $$.param_types = $3;
    }
  | LPAR PARAM value_type_list RPAR LPAR RESULT VALUE_TYPE RPAR {
      $$.result_type = $7;
      $$.param_types = $3;
    }
  | LPAR RESULT VALUE_TYPE RPAR { WASM_ZERO_MEMORY($$); $$.result_type = $3; }
;


/* Expressions */

literal :
    INT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
      CHECK_ALLOC_STR($$.text);
    }
  | FLOAT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
      CHECK_ALLOC_STR($$.text);
    }
;

var :
    INT {
      $$.loc = @1;
      $$.type = WASM_VAR_TYPE_INDEX;
      uint32_t index;
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &index,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&@1, lexer, parser, "invalid int " PRIstringslice,
                          WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
      $$.index = index;
    }
  | VAR {
      $$.loc = @1;
      $$.type = WASM_VAR_TYPE_NAME;
      DUPTEXT($$.name, $1);
      CHECK_ALLOC_STR($$.name);
    }
;
var_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | var_list var {
      $$ = $1;
      CHECK_ALLOC(wasm_append_var_value(parser->allocator, &$$, &$2));
    }
;
bind_var :
    VAR { DUPTEXT($$, $1); CHECK_ALLOC_STR($$); }
;

quoted_text :
    TEXT {
      void* data;
      size_t size;
      CHECK_ALLOC(dup_string_contents(parser->allocator, &$1, &data, &size));
      $$.start = data;
      $$.length = size;
    }
;

string_contents :
    TEXT {
      CHECK_ALLOC(dup_string_contents(parser->allocator, &$1, &$$.data,
                                      &$$.size));
    }
;

labeling :
    /* empty */ %prec LOW { WASM_ZERO_MEMORY($$); }
  | bind_var { $$ = $1; }
;

offset :
    /* empty */ { $$ = 0; }
  | OFFSET {
      if (WASM_FAILED(wasm_parse_int64($1.start, $1.start + $1.length, &$$))) {
        wasm_parser_error(&@1, lexer, parser,
                          "invalid offset \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($1));
      }
    }
;
align :
    /* empty */ { $$ = USE_NATURAL_ALIGNMENT; }
  | ALIGN {
      if (WASM_FAILED(wasm_parse_int32($1.start, $1.start + $1.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&@1, lexer, parser,
                          "invalid alignment \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($1));
      }
    }
;

expr :
    LPAR expr1 RPAR { $$ = $2; $$->loc = @1; }
;
expr1 :
    NOP {
      $$ = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_NOP);
      CHECK_ALLOC_NULL($$);
    }
  | BLOCK labeling expr_list {
      $$ = new_block_expr_with_list(parser->allocator, &$2, &$3);
      CHECK_ALLOC_NULL($$);
    }
  | IF expr expr {
      $$ = wasm_new_if_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->if_.cond = $2;
      WasmExpr* true_block = new_block_expr_with_one(parser->allocator, $3);
      CHECK_ALLOC_NULL(true_block);
      $$->if_.true_ = true_block;
    }
  | IF expr LPAR THEN labeling expr_list RPAR {
      $$ = wasm_new_if_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->if_.cond = $2;
      WasmExpr* true_block =
          new_block_expr_with_list(parser->allocator, &$5, &$6);
      CHECK_ALLOC_NULL(true_block);
      $$->if_.true_ = true_block;
    }
  | IF expr expr expr {
      $$ = wasm_new_if_else_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->if_else.cond = $2;
      WasmExpr* true_block = new_block_expr_with_one(parser->allocator, $3);
      CHECK_ALLOC_NULL(true_block);
      $$->if_else.true_ = true_block;
      WasmExpr* false_block = new_block_expr_with_one(parser->allocator, $4);
      CHECK_ALLOC_NULL(false_block);
      $$->if_else.false_ = false_block;
    }
  | IF expr LPAR THEN labeling expr_list RPAR LPAR ELSE labeling expr_list RPAR {
      $$ = wasm_new_if_else_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->if_else.cond = $2;
      WasmExpr* true_block =
          new_block_expr_with_list(parser->allocator, &$5, &$6);
      CHECK_ALLOC_NULL(true_block);
      $$->if_else.true_ = true_block;
      WasmExpr* false_block =
          new_block_expr_with_list(parser->allocator, &$10, &$11);
      CHECK_ALLOC_NULL(false_block);
      $$->if_else.false_ = false_block;
    }
  | BR_IF var expr {
      $$ = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->br_if.var = $2;
      $$->br_if.cond = $3;
    }
  | BR_IF var expr expr {
      $$ = wasm_new_br_if_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->br_if.var = $2;
      $$->br_if.expr = $3;
      $$->br_if.cond = $4;
    }
  | LOOP labeling expr_list {
      $$ = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      WASM_ZERO_MEMORY($$->loop.outer);
      $$->loop.inner = $2;
      $$->loop.exprs = $3;
    }
  | LOOP bind_var bind_var expr_list {
      $$ = wasm_new_loop_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->loop.outer = $2;
      $$->loop.inner = $3;
      $$->loop.exprs = $4;
    }
  | BR expr_opt {
      $$ = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->br.var.loc = @1;
      $$->br.var.type = WASM_VAR_TYPE_INDEX;
      $$->br.var.index = 0;
      $$->br.expr = $2;
    }
  | BR var expr_opt {
      $$ = wasm_new_br_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->br.var = $2;
      $$->br.expr = $3;
    }
  | RETURN expr_opt {
      $$ = wasm_new_return_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->return_.expr = $2;
    }
  | BR_TABLE var_list var expr {
      $$ = wasm_new_br_table_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->br_table.expr = $4;
      $$->br_table.targets = $2;
      $$->br_table.default_target = $3;
    }
  | CALL var expr_list {
      $$ = wasm_new_call_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->call.var = $2;
      $$->call.args = $3;
    }
  | CALL_IMPORT var expr_list {
      $$ = wasm_new_call_import_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->call.var = $2;
      $$->call.args = $3;
    }
  | CALL_INDIRECT var expr expr_list {
      $$ = wasm_new_call_indirect_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->call_indirect.var = $2;
      $$->call_indirect.expr = $3;
      $$->call_indirect.args = $4;
    }
  | GET_LOCAL var {
      $$ = wasm_new_get_local_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->get_local.var = $2;
    }
  | SET_LOCAL var expr {
      $$ = wasm_new_set_local_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->set_local.var = $2;
      $$->set_local.expr = $3;
    }
  | LOAD offset align expr {
      $$ = wasm_new_load_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->load.opcode = $1;
      $$->load.offset = $2;
      $$->load.align = $3;
      $$->load.addr = $4;
    }
  | STORE offset align expr expr {
      $$ = wasm_new_store_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->store.opcode = $1;
      $$->store.offset = $2;
      $$->store.align = $3;
      $$->store.addr = $4;
      $$->store.value = $5;
    }
  | CONST literal {
      $$ = wasm_new_const_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->const_.loc = @1;
      if (WASM_FAILED(parse_const($1, $2.type, $2.text.start,
                                  $2.text.start + $2.text.length,
                                  &$$->const_))) {
        wasm_parser_error(&@2, lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($2.text));
      }
      wasm_free(parser->allocator, (char*)$2.text.start);
    }
  | UNARY expr {
      $$ = wasm_new_unary_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->unary.opcode = $1;
      $$->unary.expr = $2;
    }
  | BINARY expr expr {
      $$ = wasm_new_binary_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->binary.opcode = $1;
      $$->binary.left = $2;
      $$->binary.right = $3;
    }
  | SELECT expr expr expr {
      $$ = wasm_new_select_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->select.true_ = $2;
      $$->select.false_ = $3;
      $$->select.cond = $4;
    }
  | COMPARE expr expr {
      $$ = wasm_new_compare_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->compare.opcode = $1;
      $$->compare.left = $2;
      $$->compare.right = $3;
    }
  | CONVERT expr {
      $$ = wasm_new_convert_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->convert.opcode = $1;
      $$->convert.expr = $2;
    }
  | UNREACHABLE {
      $$ = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_UNREACHABLE);
      CHECK_ALLOC_NULL($$);
    }
  | MEMORY_SIZE {
      $$ = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_MEMORY_SIZE);
      CHECK_ALLOC_NULL($$);
    }
  | GROW_MEMORY expr {
      $$ = wasm_new_grow_memory_expr(parser->allocator);
      CHECK_ALLOC_NULL($$);
      $$->grow_memory.expr = $2;
    }
;
expr_opt :
    /* empty */ { $$ = NULL; }
  | expr
;
non_empty_expr_list :
    expr {
      WASM_ZERO_MEMORY($$);
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &$$, &$1));
    }
  | non_empty_expr_list expr {
      $$ = $1;
      CHECK_ALLOC(wasm_append_expr_ptr_value(parser->allocator, &$$, &$2));
    }
;
expr_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | non_empty_expr_list
;

/* Functions */

param_list :
    LPAR PARAM value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &$$.types, &$3));
      wasm_destroy_type_vector(parser->allocator, &$3);
    }
  | LPAR PARAM bind_var VALUE_TYPE RPAR {
      WASM_ZERO_MEMORY($$);
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &$$.bindings, &$3);
      CHECK_ALLOC_NULL(binding);
      binding->loc = @2;
      binding->index = $$.types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &$$.types, &$4));
    }
  | param_list LPAR PARAM value_type_list RPAR {
      $$ = $1;
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &$$.types, &$4));
      wasm_destroy_type_vector(parser->allocator, &$4);
    }
  | param_list LPAR PARAM bind_var VALUE_TYPE RPAR {
      $$ = $1;
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &$$.bindings, &$4);
      CHECK_ALLOC_NULL(binding);
      binding->loc = @3;
      binding->index = $$.types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &$$.types, &$5));
    }
;
result :
    LPAR RESULT VALUE_TYPE RPAR { $$ = $3; }
;
local_list :
    LPAR LOCAL value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &$$.types, &$3));
      wasm_destroy_type_vector(parser->allocator, &$3);
    }
  | LPAR LOCAL bind_var VALUE_TYPE RPAR {
      WASM_ZERO_MEMORY($$);
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &$$.bindings, &$3);
      CHECK_ALLOC_NULL(binding);
      binding->loc = @2;
      binding->index = $$.types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &$$.types, &$4));
    }
  | local_list LPAR LOCAL value_type_list RPAR {
      $$ = $1;
      CHECK_ALLOC(wasm_extend_types(parser->allocator, &$$.types, &$4));
      wasm_destroy_type_vector(parser->allocator, &$4);
    }
  | local_list LPAR LOCAL bind_var VALUE_TYPE RPAR {
      $$ = $1;
      WasmBinding* binding =
          wasm_insert_binding(parser->allocator, &$$.bindings, &$4);
      CHECK_ALLOC_NULL(binding);
      binding->loc = @3;
      binding->index = $$.types.size;
      CHECK_ALLOC(wasm_append_type_value(parser->allocator, &$$.types, &$5));
    }
;
type_use :
    LPAR TYPE var RPAR { $$ = $3; }
;
func_info :
    /* empty */ {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
    }
  | bind_var {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
    }
  | bind_var type_use {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->name = $1;
      $$->type_var = $2;
    }
  | bind_var type_use param_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
    }
  | bind_var type_use param_list result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->result_type = $4;
    }
  | bind_var type_use param_list result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->result_type = $4;
      $$->locals = $5;
    }
  | bind_var type_use param_list result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->result_type = $4;
      $$->locals = $5;
      $$->exprs = $6;
    }
  | bind_var type_use param_list result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->result_type = $4;
      $$->exprs = $5;
    }
  | bind_var type_use param_list local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->locals = $4;
    }
  | bind_var type_use param_list local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->locals = $4;
      $$->exprs = $5;
    }
  | bind_var type_use param_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->params = $3;
      $$->exprs = $4;
    }
  | bind_var type_use result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->result_type = $3;
    }
  | bind_var type_use result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->result_type = $3;
      $$->locals = $4;
    }
  | bind_var type_use result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->result_type = $3;
      $$->locals = $4;
      $$->exprs = $5;
    }
  | bind_var type_use result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->type_var = $2;
      $$->result_type = $3;
      $$->exprs = $4;
    }
  | bind_var type_use local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->name = $1;
      $$->type_var = $2;
      $$->locals = $3;
    }
  | bind_var type_use local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->name = $1;
      $$->type_var = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | bind_var type_use non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->name = $1;
      $$->type_var = $2;
      $$->exprs = $3;
    }
  | bind_var local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->locals = $2;
    }
  | bind_var local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->locals = $2;
      $$->exprs = $3;
    }
  | bind_var param_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
    }
  | bind_var param_list result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->result_type = $3;
    }
  | bind_var param_list result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->locals = $4;
    }
  | bind_var param_list result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->locals = $4;
      $$->exprs = $5;
    }
  | bind_var param_list result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->exprs = $4;
    }
  | bind_var param_list local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->locals = $3;
    }
  | bind_var param_list local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | bind_var param_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->params = $2;
      $$->exprs = $3;
    }
  | bind_var result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->result_type = $2;
    }
  | bind_var result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->result_type = $2;
      $$->locals = $3;
    }
  | bind_var result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->result_type = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | bind_var result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->result_type = $2;
      $$->exprs = $3;
    }
  | bind_var non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->name = $1;
      $$->exprs = $2;
    }
  | type_use {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->type_var = $1;
    }
  | type_use param_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
    }
  | type_use param_list result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->result_type = $3;
    }
  | type_use param_list result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->locals = $4;
    }
  | type_use param_list result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->locals = $4;
      $$->exprs = $5;
    }
  | type_use param_list result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->result_type = $3;
      $$->exprs = $4;
    }
  | type_use param_list local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->locals = $3;
    }
  | type_use param_list local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | type_use param_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->params = $2;
      $$->exprs = $3;
    }
  | type_use result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->result_type = $2;
    }
  | type_use result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->result_type = $2;
      $$->locals = $3;
    }
  | type_use result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->result_type = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | type_use result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE | WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->type_var = $1;
      $$->result_type = $2;
      $$->exprs = $3;
    }
  | type_use local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->type_var = $1;
      $$->locals = $2;
    }
  | type_use local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->type_var = $1;
      $$->locals = $2;
      $$->exprs = $3;
    }
  | type_use non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
      $$->type_var = $1;
      $$->exprs = $2;
    }
  | param_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
    }
  | param_list result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->result_type = $2;
    }
  | param_list result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->result_type = $2;
      $$->locals = $3;
    }
  | param_list result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->result_type = $2;
      $$->locals = $3;
      $$->exprs = $4;
    }
  | param_list result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->result_type = $2;
      $$->exprs = $3;
    }
  | param_list local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->locals = $2;
    }
  | param_list local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->locals = $2;
      $$->exprs = $3;
    }
  | param_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->params = $1;
      $$->exprs = $2;
    }
  | result {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->result_type = $1;
    }
  | result local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->result_type = $1;
      $$->locals = $2;
    }
  | result local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->result_type = $1;
      $$->locals = $2;
      $$->exprs = $3;
    }
  | result non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->result_type = $1;
      $$->exprs = $2;
    }
  | local_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->locals = $1;
    }
  | local_list non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->locals = $1;
      $$->exprs = $2;
    }
  | non_empty_expr_list {
      $$ = new_func(parser->allocator);
      $$->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
      $$->exprs = $1;
    }
;
func :
    LPAR FUNC func_info RPAR { $$ = $3; $$->loc = @2; }
;

/* Modules */

start :
    LPAR START var RPAR { $$ = $3; }
;

segment_address :
    INT {
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&@1, lexer, parser,
                          "invalid memory segment address \"" PRIstringslice
                          "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

segment :
    LPAR SEGMENT segment_address string_contents RPAR {
      $$.loc = @2;
      $$.data = $4.data;
      $$.size = $4.size;
      $$.addr = $3;
    }
;
segment_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | segment_list segment {
      $$ = $1;
      CHECK_ALLOC(wasm_append_segment_value(parser->allocator, &$$, &$2));
    }
;

initial_pages :
    INT {
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&@1, lexer, parser,
                          "invalid initial memory pages \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

max_pages :
    INT {
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_parser_error(&@1, lexer, parser,
                          "invalid max memory pages \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

memory :
    LPAR MEMORY initial_pages max_pages segment_list RPAR {
      $$.loc = @2;
      $$.initial_pages = $3;
      $$.max_pages = $4;
      $$.segments = $5;
    }
  | LPAR MEMORY initial_pages segment_list RPAR {
      $$.loc = @2;
      $$.initial_pages = $3;
      $$.max_pages = $$.initial_pages;
      $$.segments = $4;
    }
;

type_def :
    LPAR TYPE LPAR FUNC func_type RPAR RPAR {
      WASM_ZERO_MEMORY($$);
      $$.sig = $5;
    }
  | LPAR TYPE bind_var LPAR FUNC func_type RPAR RPAR {
      $$.name = $3;
      $$.sig = $6;
    }
;

table :
    LPAR TABLE var_list RPAR { $$ = $3; }
;

import :
    LPAR IMPORT quoted_text quoted_text type_use RPAR {
      $$ = new_import(parser->allocator);
      $$->import_type = WASM_IMPORT_HAS_TYPE;
      $$->module_name = $3;
      $$->func_name = $4;
      $$->type_var = $5;
    }
  | LPAR IMPORT bind_var quoted_text quoted_text type_use RPAR /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->import_type = WASM_IMPORT_HAS_TYPE;
      $$->name = $3;
      $$->module_name = $4;
      $$->func_name = $5;
      $$->type_var = $6;
    }
  | LPAR IMPORT quoted_text quoted_text func_type RPAR  /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      $$->module_name = $3;
      $$->func_name = $4;
      $$->func_sig = $5;
    }
  | LPAR IMPORT bind_var quoted_text quoted_text func_type RPAR  /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->import_type = WASM_IMPORT_HAS_FUNC_SIGNATURE;
      $$->name = $3;
      $$->module_name = $4;
      $$->func_name = $5;
      $$->func_sig = $6;
    }
;

export :
    LPAR EXPORT quoted_text var RPAR {
      $$.name = $3;
      $$.var = $4;
    }
;

export_memory :
    LPAR EXPORT quoted_text MEMORY RPAR {
      $$.name = $3;
    }
;

module_fields :
    /* empty */ {
      $$ = new_module(parser->allocator);
    }
  | module_fields func {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *$2;
      wasm_free(parser->allocator, $2);
    }
  | module_fields import {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *$2;
      wasm_free(parser->allocator, $2);
    }
  | module_fields export {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = $2;
    }
  | module_fields export_memory {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = $2;
    }
  | module_fields table {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = $2;
    }
  | module_fields type_def {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = $2;
    }
  | module_fields memory {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = $2;
    }
  | module_fields start {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      CHECK_ALLOC_NULL(field);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = $2;
    }
;
module :
    LPAR MODULE module_fields RPAR {
      $$ = $3;
      $$->loc = @2;

      /* cache values */
      WasmModuleField* field;
      for (field = $$->first_field; field; field = field->next) {
        switch (field->type) {
          case WASM_MODULE_FIELD_TYPE_FUNC: {
            WasmFuncPtr func_ptr = &field->func;
            CHECK_ALLOC(wasm_append_func_ptr_value(parser->allocator,
                                                   &$$->funcs, &func_ptr));
            if (field->func.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &$$->func_bindings, &field->func.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = $$->funcs.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_IMPORT: {
            WasmImportPtr import_ptr = &field->import;
            CHECK_ALLOC(wasm_append_import_ptr_value(
                parser->allocator, &$$->imports, &import_ptr));
            if (field->import.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &$$->import_bindings, &field->import.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = $$->imports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_EXPORT: {
            WasmExportPtr export_ptr = &field->export_;
            CHECK_ALLOC(wasm_append_export_ptr_value(
                parser->allocator, &$$->exports, &export_ptr));
            if (field->export_.name.start) {
              WasmBinding* binding =
                  wasm_insert_binding(parser->allocator, &$$->export_bindings,
                                      &field->export_.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = $$->exports.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
            $$->export_memory = &field->export_memory;
            break;
          case WASM_MODULE_FIELD_TYPE_TABLE:
            $$->table = &field->table;
            break;
          case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
            WasmFuncTypePtr func_type_ptr = &field->func_type;
            CHECK_ALLOC(wasm_append_func_type_ptr_value(
                parser->allocator, &$$->func_types, &func_type_ptr));
            if (field->func_type.name.start) {
              WasmBinding* binding = wasm_insert_binding(
                  parser->allocator, &$$->func_type_bindings,
                  &field->func_type.name);
              CHECK_ALLOC_NULL(binding);
              binding->loc = field->loc;
              binding->index = $$->func_types.size - 1;
            }
            break;
          }
          case WASM_MODULE_FIELD_TYPE_MEMORY:
            $$->memory = &field->memory;
            break;
          case WASM_MODULE_FIELD_TYPE_START:
            $$->start = &field->start;
            break;
        }
      }

      /* if a function only defines a func type (and no explicit signature),
       * copy the signature over for convenience */
      size_t i;
      for (i = 0; i < $$->funcs.size; ++i) {
        WasmFunc* func = $$->funcs.data[i];
        if (func->flags == WASM_FUNC_FLAG_HAS_FUNC_TYPE) {
          int index = wasm_get_func_type_index_by_var($$, &func->type_var);
          if (index >= 0 && (size_t)index < $$->func_types.size) {
            WasmFuncType* func_type = $$->func_types.data[index];
            func->result_type = func_type->sig.result_type;
            CHECK_ALLOC(wasm_extend_types(parser->allocator,
                                          &func->params.types,
                                          &func_type->sig.param_types));
          }
        }

        /* now that func->params is set, we can easily create params_and_locals
         * as well */
        CHECK_ALLOC(wasm_extend_type_bindings(
            parser->allocator, &func->params_and_locals, &func->params));
        CHECK_ALLOC(wasm_extend_type_bindings(
            parser->allocator, &func->params_and_locals, &func->locals));
      }
    }
;


/* Scripts */

cmd :
    module {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_MODULE;
      $$->module = *$1;
      wasm_free(parser->allocator, $1);
    }
  | LPAR INVOKE quoted_text const_list RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_INVOKE;
      $$->invoke.loc = @2;
      $$->invoke.name = $3;
      $$->invoke.args = $4;
    }
  | LPAR ASSERT_INVALID module quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      $$->assert_invalid.module = *$3;
      $$->assert_invalid.text = $4;
      wasm_free(parser->allocator, $3);
    }
  | LPAR ASSERT_RETURN LPAR INVOKE quoted_text const_list RPAR const_opt RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      $$->assert_return.invoke.loc = @4;
      $$->assert_return.invoke.name = $5;
      $$->assert_return.invoke.args = $6;
      $$->assert_return.expected = $8;
    }
  | LPAR ASSERT_RETURN_NAN LPAR INVOKE quoted_text const_list RPAR RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      $$->assert_return_nan.invoke.loc = @4;
      $$->assert_return_nan.invoke.name = $5;
      $$->assert_return_nan.invoke.args = $6;
    }
  | LPAR ASSERT_TRAP LPAR INVOKE quoted_text const_list RPAR quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      $$->assert_trap.invoke.loc = @4;
      $$->assert_trap.invoke.name = $5;
      $$->assert_trap.invoke.args = $6;
      $$->assert_trap.text = $8;
    }
;
cmd_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | cmd_list cmd {
      $$ = $1;
      CHECK_ALLOC(wasm_append_command_value(parser->allocator, &$$, $2));
      wasm_free(parser->allocator, $2);
    }
;

const :
    LPAR CONST literal RPAR {
      $$.loc = @2;
      if (WASM_FAILED(parse_const($2, $3.type, $3.text.start,
                                  $3.text.start + $3.text.length, &$$))) {
        wasm_parser_error(&@3, lexer, parser,
                          "invalid literal \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG($3.text));
      }
      wasm_free(parser->allocator, (char*)$3.text.start);
    }
;
const_opt :
    /* empty */ { $$.type = WASM_TYPE_VOID; }
  | const
;
const_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | const_list const {
      $$ = $1;
      CHECK_ALLOC(wasm_append_const_value(parser->allocator, &$$, &$2));
    }
;

script :
    cmd_list {
      $$.commands = $1;
      parser->script = $$;
    }
;

/* bison destroys the start symbol even on a successful parse. We want to keep
 script from being destroyed, so create a dummy start symbol. */
script_start :
    script
;

%%

void wasm_parser_error(WasmLocation* loc,
                       WasmLexer lexer,
                       WasmParser* parser,
                       const char* fmt,
                       ...) {
  parser->errors++;
  va_list args;
  va_start(args, fmt);
  wasm_vfprint_error(stderr, loc, lexer, fmt, args);
  va_end(args);
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
      return wasm_parse_int64(s, end, &out->u64);
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

static size_t copy_string_contents(WasmStringSlice* text,
                                   char* dest,
                                   size_t size) {
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

static WasmResult dup_string_contents(WasmAllocator* allocator,
                                      WasmStringSlice* text,
                                      void** out_data,
                                      size_t* out_size) {
  const char* src = text->start + 1;
  const char* end = text->start + text->length - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src;
  char* result = wasm_alloc(allocator, size, 1);
  if (!result)
    return WASM_ERROR;
  size_t actual_size = copy_string_contents(text, result, size);
  *out_data = result;
  *out_size = actual_size;
  return WASM_OK;
}

WasmResult wasm_parse(WasmLexer lexer, struct WasmScript* out_script) {
  WasmParser parser;
  WASM_ZERO_MEMORY(parser);
  WasmAllocator* allocator = wasm_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  out_script->allocator = allocator;
  int result = wasm_parser_parse(lexer, &parser);
  out_script->commands = parser.script.commands;
  return result == 0 && parser.errors == 0 ? WASM_OK : WASM_ERROR;
}

WasmExpr* new_block_expr_with_one(WasmAllocator* allocator, WasmExpr* expr) {
  WasmExpr* block = wasm_new_block_expr(allocator);
  if (block) {
    WasmExprPtr* expr_ptr =
        wasm_append_expr_ptr(allocator, &block->block.exprs);
    if (expr_ptr) {
      *expr_ptr = expr;
      return block;
    }
    wasm_destroy_expr_ptr(allocator, &block);
  }
  return NULL;
}

WasmExpr* new_block_expr_with_list(WasmAllocator* allocator,
                                   WasmLabel* label,
                                   WasmExprPtrVector* exprs) {
  WasmExpr* block = wasm_new_block_expr(allocator);
  if (!block)
    return NULL;
  block->block.label = *label;
  block->block.exprs = *exprs;
  return block;
}
