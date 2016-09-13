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
#include "wasm-ast-parser.h"
#include "wasm-ast-parser-lexer-shared.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-binary-reader.h"
#include "wasm-literal.h"

/* the default value for YYMAXDEPTH is 10000, which can be easily hit since our
   grammar is right-recursive.

   we can increase YYMAXDEPTH, but the generated parser says that "results are
   undefined" if YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES(YYMAXDEPTH) with
   infinite-precision arithmetic. That's tricky to write a static assertion
   for, so let's "just" limit YYSTACK_BYTES(YYMAXDEPTH) to UINT32_MAX and use
   64-bit arithmetic. this static assert is done at the end of the file, so all
   defines are available. */
#define YYMAXDEPTH 10000000

#define DUPTEXT(dst, src)                                                   \
  (dst).start = wasm_strndup(parser->allocator, (src).start, (src).length); \
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

#define YYMALLOC(size) wasm_alloc(parser->allocator, size, WASM_DEFAULT_ALIGN)
#define YYFREE(p) wasm_free(parser->allocator, p)

#define USE_NATURAL_ALIGNMENT (~0)

#define PARSE_NAT(dst, s, s_loc, error_msg)                                   \
  do {                                                                        \
    if (WASM_FAILED(wasm_parse_uint64(s.text.start,                           \
                                      s.text.start + s.text.length, &dst))) { \
      wasm_ast_parser_error(&s_loc, lexer, parser,                            \
                            error_msg PRIstringslice "\"",                    \
                            WASM_PRINTF_STRING_SLICE_ARG(s.text));            \
    }                                                                         \
  } while (0)

static WasmExprList join_exprs1(WasmLocation* loc, WasmExpr* expr1);
static WasmExprList join_exprs2(WasmLocation* loc, WasmExprList* expr1,
                                WasmExpr* expr2);
static WasmExprList join_exprs3(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExpr* expr3);
static WasmExprList join_exprs4(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExprList* expr3,
                                WasmExpr* expr4);

static WasmFuncField* new_func_field(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmFuncField), WASM_DEFAULT_ALIGN);
}

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

static WasmTextListNode* new_text_list_node(WasmAllocator* allocator) {
  return wasm_alloc_zero(allocator, sizeof(WasmTextListNode),
                         WASM_DEFAULT_ALIGN);
}

static WasmResult parse_const(WasmType type, WasmLiteralType literal_type,
                              const char* s, const char* end, WasmConst* out);
static void dup_text_list(WasmAllocator*, WasmTextList* text_list,
                          void** out_data, size_t* out_size);

static WasmBool is_empty_signature(WasmFuncSignature* sig);

static void append_implicit_func_declaration(WasmAllocator*, WasmLocation*,
                                             WasmModule*, WasmFuncDeclaration*);

static WasmType get_result_type_from_type_vector(WasmTypeVector* types);

typedef struct BinaryErrorCallbackData {
  WasmLocation* loc;
  WasmAstLexer* lexer;
  WasmAstParser* parser;
} BinaryErrorCallbackData;

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data);

#define wasm_ast_parser_lex wasm_ast_lexer_lex

%}

%define api.prefix {wasm_ast_parser_}
%define api.pure true
%define api.value.type {WasmToken}
%define api.token.prefix {WASM_TOKEN_TYPE_}
%define parse.error verbose
%lex-param {WasmAstLexer* lexer} {WasmAstParser* parser}
%parse-param {WasmAstLexer* lexer} {WasmAstParser* parser}
%locations

%token LPAR "("
%token RPAR ")"
%token NAT INT FLOAT TEXT VAR VALUE_TYPE ANYFUNC MUT
%token NOP DROP BLOCK END IF THEN ELSE LOOP BR BR_IF BR_TABLE
%token CALL CALL_IMPORT CALL_INDIRECT RETURN
%token GET_LOCAL SET_LOCAL TEE_LOCAL GET_GLOBAL SET_GLOBAL
%token LOAD STORE OFFSET_EQ_NAT ALIGN_EQ_NAT
%token CONST UNARY BINARY COMPARE CONVERT SELECT
%token UNREACHABLE CURRENT_MEMORY GROW_MEMORY
%token FUNC START TYPE PARAM RESULT LOCAL GLOBAL
%token MODULE TABLE ELEM MEMORY DATA OFFSET IMPORT EXPORT
%token REGISTER INVOKE GET
%token ASSERT_INVALID ASSERT_UNLINKABLE
%token ASSERT_RETURN ASSERT_RETURN_NAN ASSERT_TRAP
%token INPUT OUTPUT
%token EOF 0 "EOF"

%type<opcode> BINARY COMPARE CONVERT LOAD STORE UNARY
%type<text> ALIGN_EQ_NAT OFFSET_EQ_NAT TEXT VAR
%type<type> SELECT
%type<type> CONST VALUE_TYPE
%type<literal> NAT INT FLOAT

%type<command> cmd
%type<commands> cmd_list
%type<const_> const const_opt
%type<consts> const_list
%type<elem_segment> elem
%type<export_> export
%type<export_memory> export_memory
%type<exported_func> func
%type<expr> op
%type<expr_list> expr expr1 expr_list const_expr offset
%type<func_fields> func_fields func_body
%type<func> func_info
%type<func_sig> func_type
%type<func_type> type_def
%type<global> global
%type<import> import
%type<limits> memory_limits table_limits
%type<memory> memory
%type<data_segment> data
%type<module> module module_fields
%type<raw_module> raw_module
%type<literal> literal
%type<script> script
%type<table> table
%type<text> bind_var labeling labeling1 quoted_text export_opt
%type<text_list> non_empty_text_list text_list
%type<types> value_type_list
%type<u32> nat align_opt
%type<u64> offset_opt initial_elems max_elems initial_pages max_pages
%type<vars> var_list
%type<var> start type_use var

%destructor { wasm_destroy_command(parser->allocator, $$); wasm_free(parser->allocator, $$); } cmd
%destructor { wasm_destroy_command_vector_and_elements(parser->allocator, &$$); } cmd_list
%destructor { wasm_destroy_const_vector(parser->allocator, &$$); } const_list
%destructor { wasm_destroy_elem_segment(parser->allocator, &$$); } elem
%destructor { wasm_destroy_export(parser->allocator, &$$); } export
%destructor { wasm_destroy_exported_func(parser->allocator, &$$); } func
%destructor { wasm_destroy_expr(parser->allocator, $$); } op
%destructor { wasm_destroy_expr_list(parser->allocator, $$.first); } expr expr1 expr_list const_expr offset
%destructor { wasm_destroy_func_fields(parser->allocator, $$); } func_fields func_body
%destructor { wasm_destroy_func(parser->allocator, $$); wasm_free(parser->allocator, $$); } func_info
%destructor { wasm_destroy_func_signature(parser->allocator, &$$); } func_type
%destructor { wasm_destroy_func_type(parser->allocator, &$$); } type_def
%destructor { wasm_destroy_import(parser->allocator, $$); wasm_free(parser->allocator, $$); } import
%destructor { wasm_destroy_memory_data_segment_pair(parser->allocator, &$$); } memory
%destructor { wasm_destroy_data_segment(parser->allocator, &$$); } data
%destructor { wasm_destroy_module(parser->allocator, $$); wasm_free(parser->allocator, $$); } module module_fields
%destructor { wasm_destroy_raw_module(parser->allocator, &$$); } raw_module
%destructor { wasm_destroy_string_slice(parser->allocator, &$$.text); } literal
%destructor { wasm_destroy_script(&$$); } script
%destructor { wasm_destroy_table_elem_segment_pair(parser->allocator, &$$); } table
%destructor { wasm_destroy_string_slice(parser->allocator, &$$); } bind_var labeling labeling1 quoted_text export_opt
%destructor { wasm_destroy_text_list(parser->allocator, &$$); } non_empty_text_list text_list
%destructor { wasm_destroy_type_vector(parser->allocator, &$$); } value_type_list
%destructor { wasm_destroy_var_vector_and_elements(parser->allocator, &$$); } var_list
%destructor { wasm_destroy_var(parser->allocator, &$$); } var


%nonassoc LOW
%nonassoc VAR

%start script_start

%%

non_empty_text_list :
    TEXT {
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, $1);
      node->next = NULL;
      $$.first = $$.last = node;
    }
  | non_empty_text_list TEXT {
      $$ = $1;
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, $2);
      node->next = NULL;
      $$.last->next = node;
      $$.last = node;
    }
;
text_list :
    /* empty */ { $$.first = $$.last = NULL; }
  | non_empty_text_list
;

/* Types */

value_type_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | value_type_list VALUE_TYPE {
      $$ = $1;
      wasm_append_type_value(parser->allocator, &$$, &$2);
    }
;
elem_type :
    ANYFUNC {}
;
func_type :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | LPAR PARAM value_type_list RPAR {
      $$.result_type = WASM_TYPE_VOID;
      $$.param_types = $3;
    }
  | LPAR PARAM value_type_list RPAR LPAR RESULT value_type_list RPAR {
      $$.param_types = $3;
      $$.result_type = get_result_type_from_type_vector(&$7);
    }
  | LPAR RESULT value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.result_type = get_result_type_from_type_vector(&$3);
    }
;


/* Expressions */

nat :
    NAT {
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&@1, lexer, parser, "invalid int " PRIstringslice,
                              WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

literal :
    NAT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
    }
  | INT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
    }
  | FLOAT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
    }
;

var :
    NAT {
      $$.loc = @1;
      $$.type = WASM_VAR_TYPE_INDEX;
      uint64_t index;
      if (WASM_FAILED(wasm_parse_int64($1.text.start,
                                       $1.text.start + $1.text.length, &index,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&@1, lexer, parser, "invalid int " PRIstringslice,
                              WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
      $$.index = index;
    }
  | VAR {
      $$.loc = @1;
      $$.type = WASM_VAR_TYPE_NAME;
      DUPTEXT($$.name, $1);
    }
;
var_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | var_list var {
      $$ = $1;
      wasm_append_var_value(parser->allocator, &$$, &$2);
    }
;
bind_var :
    VAR { DUPTEXT($$, $1); }
;

quoted_text :
    TEXT {
      WasmTextListNode node;
      node.text = $1;
      node.next = NULL;
      WasmTextList text_list;
      text_list.first = &node;
      text_list.last = &node;
      void* data;
      size_t size;
      dup_text_list(parser->allocator, &text_list, &data, &size);
      $$.start = data;
      $$.length = size;
    }
;

labeling :
    /* empty */ %prec LOW { WASM_ZERO_MEMORY($$); }
  | labeling1
;
labeling1 : bind_var ;

offset_opt :
    /* empty */ { $$ = 0; }
  | OFFSET_EQ_NAT {
    if (WASM_FAILED(wasm_parse_int64($1.start, $1.start + $1.length, &$$,
                                     WASM_PARSE_SIGNED_AND_UNSIGNED))) {
      wasm_ast_parser_error(&@1, lexer, parser,
                            "invalid offset \"" PRIstringslice "\"",
                            WASM_PRINTF_STRING_SLICE_ARG($1));
      }
    }
;
align_opt :
    /* empty */ { $$ = USE_NATURAL_ALIGNMENT; }
  | ALIGN_EQ_NAT {
      if (WASM_FAILED(wasm_parse_int32($1.start, $1.start + $1.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&@1, lexer, parser,
                              "invalid alignment \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG($1));
      }
    }
;

expr :
    op { $$ = join_exprs1(&@1, $1); }
  | LPAR expr1 RPAR { $$ = $2; }
;
op :
    UNREACHABLE {
      $$ = wasm_new_unreachable_expr(parser->allocator);
    }
  | NOP {
      $$ = wasm_new_nop_expr(parser->allocator);
    }
  | DROP {
      $$ = wasm_new_drop_expr(parser->allocator);
    }
  | BLOCK labeling expr_list END {
      $$ = wasm_new_block_expr(parser->allocator);
      $$->block.label = $2;
      $$->block.first = $3.first;
    }
  | LOOP labeling expr_list END {
      $$ = wasm_new_loop_expr(parser->allocator);
      $$->loop.label = $2;
      $$->loop.first = $3.first;
    }
  | LOOP labeling1 labeling1 expr_list END {
      $$ = wasm_new_block_expr(parser->allocator);
      $$->block.label = $2;
      WasmExpr* loop = wasm_new_loop_expr(parser->allocator);
      loop->loc = @1;
      loop->loop.label = $3;
      loop->loop.first = $4.first;
      $$->block.first = loop;
    }
  | BR nat var {
      $$ = wasm_new_br_expr(parser->allocator);
      $$->br.arity = $2;
      $$->br.var = $3;
    }
  | BR_IF nat var {
      $$ = wasm_new_br_if_expr(parser->allocator);
      $$->br_if.arity = $2;
      $$->br_if.var = $3;
    }
  | BR_TABLE nat var_list var {
      $$ = wasm_new_br_table_expr(parser->allocator);
      $$->br_table.arity = $2;
      $$->br_table.targets = $3;
      $$->br_table.default_target = $4;
      // split_var_list_as_br_table_targets(&$$, $3);
    }
  | RETURN {
      $$ = wasm_new_return_expr(parser->allocator);
    }
  | IF labeling expr_list END {
      $$ = wasm_new_if_expr(parser->allocator);
      $$->if_.true_.label = $2;
      $$->if_.true_.first = $3.first;
    }
  | IF labeling expr_list ELSE labeling expr_list END {
      $$ = wasm_new_if_else_expr(parser->allocator);
      $$->if_else.true_.label = $2;
      $$->if_else.true_.first = $3.first;
      $$->if_else.false_.label = $5;
      $$->if_else.false_.first = $6.first;
    }
  | SELECT {
      $$ = wasm_new_select_expr(parser->allocator);
    }
  | CALL var {
      $$ = wasm_new_call_expr(parser->allocator);
      $$->call.var = $2;
    }
  | CALL_IMPORT var {
      $$ = wasm_new_call_import_expr(parser->allocator);
      $$->call_import.var = $2;
    }
  | CALL_INDIRECT var {
      $$ = wasm_new_call_indirect_expr(parser->allocator);
      $$->call_indirect.var = $2;
    }
  | GET_LOCAL var {
      $$ = wasm_new_get_local_expr(parser->allocator);
      $$->get_local.var = $2;
    }
  | SET_LOCAL var {
      $$ = wasm_new_set_local_expr(parser->allocator);
      $$->set_local.var = $2;
    }
  | TEE_LOCAL var {
      $$ = wasm_new_tee_local_expr(parser->allocator);
      $$->tee_local.var = $2;
    }
  | GET_GLOBAL var {
      $$ = wasm_new_get_global_expr(parser->allocator);
      $$->get_global.var = $2;
    }
  | SET_GLOBAL var {
      $$ = wasm_new_set_global_expr(parser->allocator);
      $$->set_global.var = $2;
    }
  | LOAD offset_opt align_opt {
      $$ = wasm_new_load_expr(parser->allocator);
      $$->load.opcode = $1;
      $$->load.offset = $2;
      $$->load.align = $3;
    }
  | STORE offset_opt align_opt {
      $$ = wasm_new_store_expr(parser->allocator);
      $$->store.opcode = $1;
      $$->store.offset = $2;
      $$->store.align = $3;
    }
  | CONST literal {
      $$ = wasm_new_const_expr(parser->allocator);
      $$->const_.loc = @1;
      if (WASM_FAILED(parse_const($1, $2.type, $2.text.start,
                                  $2.text.start + $2.text.length,
                                  &$$->const_))) {
        wasm_ast_parser_error(&@2, lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG($2.text));
      }
      wasm_free(parser->allocator, (char*)$2.text.start);
    }
  | UNARY {
      $$ = wasm_new_unary_expr(parser->allocator);
      $$->unary.opcode = $1;
    }
  | BINARY {
      $$ = wasm_new_binary_expr(parser->allocator);
      $$->binary.opcode = $1;
    }
  | COMPARE {
      $$ = wasm_new_compare_expr(parser->allocator);
      $$->compare.opcode = $1;
    }
  | CONVERT {
      $$ = wasm_new_convert_expr(parser->allocator);
      $$->convert.opcode = $1;
    }
  | CURRENT_MEMORY {
      $$ = wasm_new_current_memory_expr(parser->allocator);
    }
  | GROW_MEMORY {
      $$ = wasm_new_grow_memory_expr(parser->allocator);
    }
;
expr1 :
    UNREACHABLE {
      $$ = join_exprs1(&@1, wasm_new_unreachable_expr(parser->allocator));
    }
  | NOP {
      $$ = join_exprs1(&@1, wasm_new_nop_expr(parser->allocator));
    }
  | DROP expr {
      $$ = join_exprs2(&@1, &$2, wasm_new_drop_expr(parser->allocator));
    }
  | BLOCK labeling expr_list {
      WasmExpr* expr = wasm_new_block_expr(parser->allocator);
      expr->block.label = $2;
      expr->block.first = $3.first;
      $$ = join_exprs1(&@1, expr);
    }
  | LOOP labeling expr_list {
      WasmExpr* expr = wasm_new_loop_expr(parser->allocator);
      expr->loop.label = $2;
      expr->loop.first = $3.first;
      $$ = join_exprs1(&@1, expr);
    }
  | LOOP labeling1 labeling1 expr_list {
      WasmExpr* block = wasm_new_block_expr(parser->allocator);
      block->block.label = $2;
      WasmExpr* loop = wasm_new_loop_expr(parser->allocator);
      loop->loc = @1;
      loop->loop.label = $3;
      loop->loop.first = $4.first;
      block->block.first = loop;
      $$ = join_exprs1(&@1, block);
    }
  | BR var {
      WasmExpr* expr = wasm_new_br_expr(parser->allocator);
      expr->br.arity = 0;
      expr->br.var = $2;
      $$ = join_exprs1(&@1, expr);
    }
  | BR var expr {
      WasmExpr* expr = wasm_new_br_expr(parser->allocator);
      expr->br.arity = 1;
      expr->br.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | BR_IF var expr {
      WasmExpr* expr = wasm_new_br_if_expr(parser->allocator);
      expr->br_if.arity = 0;
      expr->br_if.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | BR_IF var expr expr {
      WasmExpr* expr = wasm_new_br_if_expr(parser->allocator);
      expr->br_if.arity = 1;
      expr->br_if.var = $2;
      $$ = join_exprs3(&@1, &$3, &$4, expr);
    }
  | BR_TABLE var_list var expr {
      WasmExpr* expr = wasm_new_br_table_expr(parser->allocator);
      expr->br_table.arity = 0;
      expr->br_table.targets = $2;
      expr->br_table.default_target = $3;
      // split_var_list_as_br_table_targets(expr, $2);
      $$ = join_exprs2(&@1, &$4, expr);
    }
  | BR_TABLE var_list var expr expr {
      WasmExpr* expr = wasm_new_br_table_expr(parser->allocator);
      expr->br_table.arity = 1;
      expr->br_table.targets = $2;
      expr->br_table.default_target = $3;
      // split_var_list_as_br_table_targets(expr, $2);
      $$ = join_exprs3(&@1, &$4, &$5, expr);
    }
  | RETURN expr_list {
      WasmExpr* expr = wasm_new_return_expr(parser->allocator);
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | IF expr expr {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $3.first;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | IF expr expr expr {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.first = $3.first;
      expr->if_else.false_.first = $4.first;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | IF expr LPAR THEN labeling expr_list RPAR {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.label = $5;
      expr->if_.true_.first = $6.first;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | IF expr LPAR THEN labeling expr_list RPAR LPAR ELSE labeling expr_list RPAR {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.label = $5;
      expr->if_else.true_.first = $6.first;
      expr->if_else.false_.label = $10;
      expr->if_else.false_.first = $11.first;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | IF expr_list ELSE expr_list {
      WasmExpr* expr = wasm_new_if_else_expr(parser->allocator);
      expr->if_else.true_.first = $2.first;
      expr->if_else.false_.first = $4.first;
      $$ = join_exprs1(&@1, expr);
    }
  | SELECT expr expr expr {
      $$ = join_exprs4(&@1, &$2, &$3, &$4,
                       wasm_new_select_expr(parser->allocator));
    }
  | CALL var expr_list {
      WasmExpr* expr = wasm_new_call_expr(parser->allocator);
      expr->call.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | CALL_IMPORT var expr_list {
      WasmExpr* expr = wasm_new_call_import_expr(parser->allocator);
      expr->call_import.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | CALL_INDIRECT var expr expr_list {
      WasmExpr* expr = wasm_new_call_indirect_expr(parser->allocator);
      expr->call_indirect.var = $2;
      $$ = join_exprs3(&@1, &$3, &$4, expr);
    }
  | GET_LOCAL var {
      WasmExpr* expr = wasm_new_get_local_expr(parser->allocator);
      expr->get_local.var = $2;
      $$ = join_exprs1(&@1, expr);
    }
  | SET_LOCAL var expr {
      WasmExpr* expr = wasm_new_set_local_expr(parser->allocator);
      expr->set_local.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | TEE_LOCAL var expr {
      WasmExpr* expr = wasm_new_tee_local_expr(parser->allocator);
      expr->tee_local.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | GET_GLOBAL var {
      WasmExpr* expr = wasm_new_get_global_expr(parser->allocator);
      expr->get_global.var = $2;
      $$ = join_exprs1(&@1, expr);
    }
  | SET_GLOBAL var expr {
      WasmExpr* expr = wasm_new_set_global_expr(parser->allocator);
      expr->set_global.var = $2;
      $$ = join_exprs2(&@1, &$3, expr);
    }
  | LOAD offset_opt align_opt expr {
      WasmExpr* expr = wasm_new_load_expr(parser->allocator);
      expr->load.opcode = $1;
      expr->load.offset = $2;
      expr->load.align = $3;
      $$ = join_exprs2(&@1, &$4, expr);
    }
  | STORE offset_opt align_opt expr expr {
      WasmExpr* expr = wasm_new_store_expr(parser->allocator);
      expr->store.opcode = $1;
      expr->store.offset = $2;
      expr->store.align = $3;
      $$ = join_exprs3(&@1, &$4, &$5, expr);
    }
  | CONST literal {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->const_.loc = @1;
      if (WASM_FAILED(parse_const($1, $2.type, $2.text.start,
                                  $2.text.start + $2.text.length,
                                  &expr->const_))) {
        wasm_ast_parser_error(&@2, lexer, parser,
                              "invalid literal \"" PRIstringslice "\"",
                              WASM_PRINTF_STRING_SLICE_ARG($2.text));
      }
      wasm_free(parser->allocator, (char*)$2.text.start);
      $$ = join_exprs1(&@1, expr);
    }
  | UNARY expr {
      WasmExpr* expr = wasm_new_unary_expr(parser->allocator);
      expr->unary.opcode = $1;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | BINARY expr expr {
      WasmExpr* expr = wasm_new_binary_expr(parser->allocator);
      expr->binary.opcode = $1;
      $$ = join_exprs3(&@1, &$2, &$3, expr);
    }
  | COMPARE expr expr {
      WasmExpr* expr = wasm_new_compare_expr(parser->allocator);
      expr->compare.opcode = $1;
      $$ = join_exprs3(&@1, &$2, &$3, expr);
    }
  | CONVERT expr {
      WasmExpr* expr = wasm_new_convert_expr(parser->allocator);
      expr->convert.opcode = $1;
      $$ = join_exprs2(&@1, &$2, expr);
    }
  | CURRENT_MEMORY {
      WasmExpr* expr = wasm_new_current_memory_expr(parser->allocator);
      $$ = join_exprs1(&@1, expr);
    }
  | GROW_MEMORY expr {
      WasmExpr* expr = wasm_new_grow_memory_expr(parser->allocator);
      $$ = join_exprs2(&@1, &$2, expr);
    }
;
expr_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | expr expr_list {
      $$.first = $1.first;
      $1.last->next = $2.first;
      $$.last = $2.last ? $2.last : $1.last;
      $$.size = $1.size + $2.size;
    }
;
const_expr :
    expr_list
;

/* Functions */
func_fields :
    func_body
  | LPAR RESULT value_type_list RPAR func_body {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_RESULT_TYPES;
      $$->types = $3;
      $$->next = $5;
    }
  | LPAR PARAM value_type_list RPAR func_fields {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_PARAM_TYPES;
      $$->types = $3;
      $$->next = $5;
    }
  | LPAR PARAM bind_var VALUE_TYPE RPAR func_fields {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_BOUND_PARAM;
      $$->bound_type.loc = @2;
      $$->bound_type.name = $3;
      $$->bound_type.type = $4;
      $$->next = $6;
    }
;
func_body :
    expr_list {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_EXPRS;
      $$->first_expr = $1.first;
      $$->next = NULL;
    }
  | LPAR LOCAL value_type_list RPAR func_body {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_LOCAL_TYPES;
      $$->types = $3;
      $$->next = $5;
    }
  | LPAR LOCAL bind_var VALUE_TYPE RPAR func_body {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_BOUND_LOCAL;
      $$->bound_type.loc = @2;
      $$->bound_type.name = $3;
      $$->bound_type.type = $4;
      $$->next = $6;
    }
;
type_use :
    LPAR TYPE var RPAR { $$ = $3; }
;
func_info :
    func_fields {
      $$ = new_func(parser->allocator);
      WasmFuncField* field = $1;

      while (field) {
        WasmFuncField* next = field->next;
        switch (field->type) {
          case WASM_FUNC_FIELD_TYPE_EXPRS:
            $$->first_expr = field->first_expr;
            break;

          case WASM_FUNC_FIELD_TYPE_PARAM_TYPES:
          case WASM_FUNC_FIELD_TYPE_LOCAL_TYPES: {
            WasmTypeVector* types =
                field->type == WASM_FUNC_FIELD_TYPE_PARAM_TYPES
                    ? &$$->decl.sig.param_types
                    : &$$->local_types;
            wasm_extend_types(parser->allocator, types, &field->types);
            wasm_destroy_type_vector(parser->allocator, &field->types);
            break;
          }

          case WASM_FUNC_FIELD_TYPE_BOUND_PARAM:
          case WASM_FUNC_FIELD_TYPE_BOUND_LOCAL: {
            WasmTypeVector* types;
            WasmBindingHash* bindings;
            if (field->type == WASM_FUNC_FIELD_TYPE_BOUND_PARAM) {
              types = &$$->decl.sig.param_types;
              bindings = &$$->param_bindings;
            } else {
              types = &$$->local_types;
              bindings = &$$->local_bindings;
            }

            wasm_append_type_value(parser->allocator, types,
                                   &field->bound_type.type);
            WasmBinding* binding = wasm_insert_binding(
                parser->allocator, bindings, &field->bound_type.name);
            binding->loc = field->bound_type.loc;
            binding->index = types->size - 1;
            break;
          }

          case WASM_FUNC_FIELD_TYPE_RESULT_TYPES:
            $$->decl.sig.result_type =
                get_result_type_from_type_vector(&field->types);
            break;
        }

        /* we steal memory from the func field, but not the linked list nodes */
        wasm_free(parser->allocator, field);
        field = next;
      }
    }
;
func :
    LPAR FUNC export_opt type_use func_info RPAR {
      $$.func = $5;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt bind_var type_use func_info RPAR {
      $$.func = $6;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $5;
      $$.func->name = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt func_info RPAR {
      $$.func = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt bind_var func_info RPAR {
      $$.func = $5;
      $$.func->name = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
;

export_opt :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | quoted_text
;

/* Tables & Memories */

offset :
    LPAR OFFSET const_expr RPAR {
      $$ = $3;
    }
  | LPAR expr1 RPAR {
      $$ = $2;
    }
;
elem :
    LPAR ELEM offset var_list RPAR {
      $$.offset = $3.first;
      $$.vars = $4;
    }
;
initial_elems :
    NAT { PARSE_NAT($$, $1, @1, "invalid initial table elems"); }
;
max_elems :
    NAT { PARSE_NAT($$, $1, @1, "invalid max table elems"); }
;
table_limits :
    initial_elems {
      $$.initial = $1;
      $$.max = 0;
      $$.has_max = WASM_FALSE;
    }
  | initial_elems max_elems {
      $$.initial = $1;
      $$.max = $2;
      $$.has_max = WASM_TRUE;
    }
;
table :
    LPAR TABLE table_limits elem_type RPAR {
      $$.table.elem_limits = $3;
      $$.has_elem_segment = WASM_FALSE;
    }
  | LPAR TABLE elem_type LPAR ELEM var_list RPAR RPAR {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->loc = @2;
      expr->const_.type = WASM_TYPE_I32;
      expr->const_.u32 = 0;

      WASM_ZERO_MEMORY($$);
      $$.table.elem_limits.initial = $6.size;
      $$.table.elem_limits.max = $6.size;
      $$.table.elem_limits.has_max = WASM_TRUE;
      $$.has_elem_segment = WASM_TRUE;
      $$.elem_segment.offset = expr;
      $$.elem_segment.vars = $6;
    }
;
data :
    LPAR DATA offset text_list RPAR {
      $$.offset = $3.first;
      dup_text_list(parser->allocator, &$4, &$$.data, &$$.size);
    }
;
initial_pages :
    NAT { PARSE_NAT($$, $1, @1, "invalid initial memory pages"); }
;
max_pages :
    NAT { PARSE_NAT($$, $1, @1, "invalid max memory pages"); }
;
memory_limits :
    initial_pages {
      $$.initial = $1;
      $$.max = 0;
      $$.has_max = WASM_FALSE;
    }
  | initial_pages max_pages {
      $$.initial = $1;
      $$.max = $2;
      $$.has_max = WASM_TRUE;
    }
;
memory :
    LPAR MEMORY memory_limits RPAR {
      $$.memory.page_limits = $3;
      $$.has_data_segment = WASM_FALSE;
    }
  | LPAR MEMORY LPAR DATA text_list RPAR RPAR {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->loc = @2;
      expr->const_.type = WASM_TYPE_I32;
      expr->const_.u32 = 0;

      WASM_ZERO_MEMORY($$);
      $$.has_data_segment = WASM_TRUE;
      $$.data_segment.offset = expr;
      dup_text_list(parser->allocator, &$5, &$$.data_segment.data,
                    &$$.data_segment.size);
      uint32_t byte_size = WASM_ALIGN_UP_TO_PAGE($$.data_segment.size);
      uint32_t page_size = WASM_BYTES_TO_PAGES(byte_size);
      $$.memory.page_limits.initial = page_size;
      $$.memory.page_limits.max = page_size;
      $$.memory.page_limits.has_max = WASM_TRUE;
    }
;

/* Modules */

start :
    LPAR START var RPAR { $$ = $3; }
;

global :
    LPAR GLOBAL VALUE_TYPE expr RPAR {
      WASM_ZERO_MEMORY($$);
      $$.type = $3;
      $$.init_expr = $4.first;
    }
  | LPAR GLOBAL bind_var VALUE_TYPE expr RPAR {
      WASM_ZERO_MEMORY($$);
      $$.name = $3;
      $$.type = $4;
      $$.init_expr = $5.first;
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

import :
    LPAR IMPORT quoted_text quoted_text type_use RPAR {
      $$ = new_import(parser->allocator);
      $$->module_name = $3;
      $$->func_name = $4;
      $$->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$->decl.type_var = $5;
    }
  | LPAR IMPORT bind_var quoted_text quoted_text type_use RPAR /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->name = $3;
      $$->module_name = $4;
      $$->func_name = $5;
      $$->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$->decl.type_var = $6;
    }
  | LPAR IMPORT quoted_text quoted_text func_type RPAR  /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->module_name = $3;
      $$->func_name = $4;
      $$->decl.sig = $5;
    }
  | LPAR IMPORT bind_var quoted_text quoted_text func_type RPAR  /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->name = $3;
      $$->module_name = $4;
      $$->func_name = $5;
      $$->decl.sig = $6;
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
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_FUNC;
      field->func = *$2.func;
      wasm_free(parser->allocator, $2.func);

      append_implicit_func_declaration(parser->allocator, &@2, $$,
                                       &field->func.decl);

      WasmFunc* func_ptr = &field->func;
      wasm_append_func_ptr_value(parser->allocator, &$$->funcs, &func_ptr);
      if (field->func.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &$$->func_bindings, &field->func.name);
        binding->loc = field->loc;
        binding->index = $$->funcs.size - 1;
      }

      /* is this function using the export syntactic sugar? */
      if ($2.export_.name.start != NULL) {
        WasmModuleField* export_field =
            wasm_append_module_field(parser->allocator, $$);
        export_field->loc = @2;
        export_field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
        export_field->export_ = $2.export_;
        export_field->export_.var.index = $$->funcs.size - 1;

        WasmExport* export_ptr = &export_field->export_;
        wasm_append_export_ptr_value(parser->allocator, &$$->exports,
                                     &export_ptr);
        WasmBinding* binding =
            wasm_insert_binding(parser->allocator, &$$->export_bindings,
                                &export_field->export_.name);
        binding->loc = export_field->loc;
        binding->index = $$->exports.size - 1;
      }
    }
  | module_fields global {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;
      field->global = $2;

      WasmGlobal* global_ptr = &field->global;
      wasm_append_global_ptr_value(parser->allocator, &$$->globals,
                                   &global_ptr);
      if (field->global.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &$$->global_bindings, &field->global.name);
        binding->loc = field->loc;
        binding->index = $$->globals.size - 1;
      }
    }
  | module_fields import {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *$2;
      wasm_free(parser->allocator, $2);

      append_implicit_func_declaration(parser->allocator, &@2, $$,
                                       &field->import.decl);

      WasmImport* import_ptr = &field->import;
      wasm_append_import_ptr_value(parser->allocator, &$$->imports,
                                   &import_ptr);
      if (field->import.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &$$->import_bindings, &field->import.name);
        binding->loc = field->loc;
        binding->index = $$->imports.size - 1;
      }
    }
  | module_fields export {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT;
      field->export_ = $2;

      WasmExport* export_ptr = &field->export_;
      wasm_append_export_ptr_value(parser->allocator, &$$->exports,
                                   &export_ptr);
      if (field->export_.name.start) {
        WasmBinding* binding = wasm_insert_binding(
            parser->allocator, &$$->export_bindings, &field->export_.name);
        binding->loc = field->loc;
        binding->index = $$->exports.size - 1;
      }
    }
  | module_fields export_memory {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
      field->export_memory = $2;
      $$->export_memory = &field->export_memory;
    }
  | module_fields table {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_TABLE;
      field->table = $2.table;
      $$->table = &field->table;

      /* is this table using the elem_segment syntactic sugar? */
      if ($2.has_elem_segment) {
        WasmModuleField* elem_segment_field =
            wasm_append_module_field(parser->allocator, $$);
        elem_segment_field->loc = @2;
        elem_segment_field->type = WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT;
        elem_segment_field->elem_segment = $2.elem_segment;

        WasmElemSegment* elem_segment_ptr = &elem_segment_field->elem_segment;
        wasm_append_elem_segment_ptr_value(
            parser->allocator, &$$->elem_segments, &elem_segment_ptr);
      }
    }
  | module_fields elem {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT;
      field->elem_segment = $2;

      WasmElemSegment* elem_segment_ptr = &field->elem_segment;
      wasm_append_elem_segment_ptr_value(parser->allocator, &$$->elem_segments,
                                         &elem_segment_ptr);
    }
  | module_fields type_def {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
      field->func_type = $2;

      WasmFuncType* func_type_ptr = &field->func_type;
      wasm_append_func_type_ptr_value(parser->allocator, &$$->func_types,
                                      &func_type_ptr);
      if (field->func_type.name.start) {
        WasmBinding* binding =
            wasm_insert_binding(parser->allocator, &$$->func_type_bindings,
                                &field->func_type.name);
        binding->loc = field->loc;
        binding->index = $$->func_types.size - 1;
      }
    }
  | module_fields memory {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
      field->memory = $2.memory;
      $$->memory = &field->memory;

      /* is this table using the memory data syntactic sugar? */
      if ($2.has_data_segment) {
        WasmModuleField* data_segment_field =
            wasm_append_module_field(parser->allocator, $$);
        data_segment_field->loc = @2;
        data_segment_field->type = WASM_MODULE_FIELD_TYPE_DATA_SEGMENT;
        data_segment_field->data_segment = $2.data_segment;

        WasmDataSegment* data_segment_ptr = &data_segment_field->data_segment;
        wasm_append_data_segment_ptr_value(
            parser->allocator, &$$->data_segments, &data_segment_ptr);
      }
    }
  | module_fields data {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_DATA_SEGMENT;
      field->data_segment = $2;

      WasmDataSegment* data_segment_ptr = &field->data_segment;
      wasm_append_data_segment_ptr_value(parser->allocator, &$$->data_segments,
                                         &data_segment_ptr);
    }
  | module_fields start {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_START;
      field->start = $2;
      $$->start = &field->start;
    }
;

raw_module :
    LPAR MODULE module_fields RPAR {
      $$.type = WASM_RAW_MODULE_TYPE_TEXT;
      $$.text = $3;
      $$.loc = @2;

      /* resolve func type variables where the signature was not specified
       * explicitly */
      size_t i;
      for (i = 0; i < $3->funcs.size; ++i) {
        WasmFunc* func = $3->funcs.data[i];
        if (wasm_decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          WasmFuncType* func_type =
              wasm_get_func_type_by_var($3, &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
            func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }

      for (i = 0; i < $3->imports.size; ++i) {
        WasmImport* import = $3->imports.data[i];
        if (wasm_decl_has_func_type(&import->decl) &&
            is_empty_signature(&import->decl.sig)) {
          WasmFuncType* func_type =
              wasm_get_func_type_by_var($3, &import->decl.type_var);
          if (func_type) {
            import->decl.sig = func_type->sig;
            import->decl.flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }
    }
  | LPAR MODULE non_empty_text_list RPAR {
      $$.type = WASM_RAW_MODULE_TYPE_BINARY;
      $$.loc = @2;
      dup_text_list(parser->allocator, &$3, &$$.binary.data, &$$.binary.size);
      wasm_destroy_text_list(parser->allocator, &$3);
    }
;

module :
    raw_module {
      if ($1.type == WASM_RAW_MODULE_TYPE_TEXT) {
        $$ = $1.text;
      } else {
        assert($1.type == WASM_RAW_MODULE_TYPE_BINARY);
        $$ = new_module(parser->allocator);
        WasmReadBinaryOptions options = WASM_READ_BINARY_OPTIONS_DEFAULT;
        BinaryErrorCallbackData user_data;
        user_data.loc = &$1.loc;
        user_data.lexer = lexer;
        user_data.parser = parser;
        WasmBinaryErrorHandler error_handler;
        error_handler.on_error = on_read_binary_error;
        error_handler.user_data = &user_data;
        wasm_read_binary_ast(parser->allocator, $1.binary.data, $1.binary.size,
                             &options, &error_handler, $$);
        wasm_free(parser->allocator, $1.binary.data);
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
  | LPAR ASSERT_INVALID raw_module quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      $$->assert_invalid.module = $3;
      $$->assert_invalid.text = $4;
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
      wasm_append_command_value(parser->allocator, &$$, $2);
      wasm_free(parser->allocator, $2);
    }
;

const :
    LPAR CONST literal RPAR {
      $$.loc = @2;
      if (WASM_FAILED(parse_const($2, $3.type, $3.text.start,
                                  $3.text.start + $3.text.length, &$$))) {
        wasm_ast_parser_error(&@3, lexer, parser,
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
      wasm_append_const_value(parser->allocator, &$$, &$2);
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

static void append_expr_list(WasmExprList* expr_list, WasmExprList* expr) {
  if (!expr->first)
    return;
  if (expr_list->last)
    expr_list->last->next = expr->first;
  else
    expr_list->first = expr->first;
  expr_list->last = expr->last;
  expr_list->size += expr->size;
}

static void append_expr(WasmExprList* expr_list, WasmExpr* expr) {
  if (expr_list->last)
    expr_list->last->next = expr;
  else
    expr_list->first = expr;
  expr_list->last = expr;
  expr_list->size++;
}

static WasmExprList join_exprs1(WasmLocation* loc, WasmExpr* expr1) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr(&result, expr1);
  expr1->loc = *loc;
  return result;
}

static WasmExprList join_exprs2(WasmLocation* loc, WasmExprList* expr1,
                                WasmExpr* expr2) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr(&result, expr2);
  expr2->loc = *loc;
  return result;
}

static WasmExprList join_exprs3(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExpr* expr3) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  append_expr(&result, expr3);
  expr3->loc = *loc;
  return result;
}

static WasmExprList join_exprs4(WasmLocation* loc, WasmExprList* expr1,
                                WasmExprList* expr2, WasmExprList* expr3,
                                WasmExpr* expr4) {
  WasmExprList result;
  WASM_ZERO_MEMORY(result);
  append_expr_list(&result, expr1);
  append_expr_list(&result, expr2);
  append_expr_list(&result, expr3);
  append_expr(&result, expr4);
  expr4->loc = *loc;
  return result;
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
      return wasm_parse_int64(s, end, &out->u64,
                              WASM_PARSE_SIGNED_AND_UNSIGNED);
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

static size_t copy_string_contents(WasmStringSlice* text, char* dest) {
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

static void dup_text_list(WasmAllocator* allocator,
                          WasmTextList* text_list,
                          void** out_data,
                          size_t* out_size) {
  /* walk the linked list to see how much total space is needed */
  size_t total_size = 0;
  WasmTextListNode* node;
  for (node = text_list->first; node; node = node->next) {
    /* Always allocate enough space for the entire string including the escape
     * characters. It will only get shorter, and this way we only have to
     * iterate through the string once. */
    const char* src = node->text.start + 1;
    const char* end = node->text.start + node->text.length - 1;
    size_t size = (end > src) ? (end - src) : 0;
    total_size += size;
  }
  char* result = wasm_alloc(allocator, total_size, 1);
  char* dest = result;
  for (node = text_list->first; node; node = node->next) {
    size_t actual_size = copy_string_contents(&node->text, dest);
    dest += actual_size;
  }
  *out_data = result;
  *out_size = dest - result;
}

static WasmBool is_empty_signature(WasmFuncSignature* sig) {
  return sig->result_type == WASM_TYPE_VOID && sig->param_types.size == 0;
}

static void append_implicit_func_declaration(WasmAllocator* allocator,
                                             WasmLocation* loc,
                                             WasmModule* module,
                                             WasmFuncDeclaration* decl) {
  if (wasm_decl_has_func_type(decl))
    return;

  int sig_index = wasm_get_func_type_index_by_decl(module, decl);
  if (sig_index == -1) {
    wasm_append_implicit_func_type(allocator, loc, module, &decl->sig);
  } else {
    /* signature already exists, share that one and destroy this one */
    wasm_destroy_func_signature(allocator, &decl->sig);
    WasmFuncSignature* sig = &module->func_types.data[sig_index]->sig;
    decl->sig = *sig;
  }

  decl->flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
}

static WasmType get_result_type_from_type_vector(WasmTypeVector* types) {
  /* TODO(binji): handle multiple result types more cleanly */
  switch (types->size) {
    case 0: return WASM_TYPE_VOID; break;
    case 1: return types->data[0]; break;
    default: return WASM_TYPE_MULTIPLE; break;
  }
}

WasmResult wasm_parse_ast(WasmAstLexer* lexer,
                          struct WasmScript* out_script,
                          WasmSourceErrorHandler* error_handler) {
  WasmAstParser parser;
  WASM_ZERO_MEMORY(parser);
  WasmAllocator* allocator = wasm_ast_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  parser.error_handler = error_handler;
  out_script->allocator = allocator;
  int result = wasm_ast_parser_parse(lexer, &parser);
  out_script->commands = parser.script.commands;
  return result == 0 && parser.errors == 0 ? WASM_OK : WASM_ERROR;
}

static void on_read_binary_error(uint32_t offset, const char* error,
                                 void* user_data) {
  BinaryErrorCallbackData* data = user_data;
  if (offset == WASM_UNKNOWN_OFFSET) {
    wasm_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: %s", error);
  } else {
    wasm_ast_parser_error(data->loc, data->lexer, data->parser,
                          "error in binary module: @0x%08x: %s", offset, error);
  }
}

/* see comment above definition of YYMAXDEPTH at the top of this file */
WASM_STATIC_ASSERT(YYSTACK_ALLOC_MAXIMUM >= UINT32_MAX);
WASM_STATIC_ASSERT(YYSTACK_BYTES((uint64_t)YYMAXDEPTH) <= UINT32_MAX);
