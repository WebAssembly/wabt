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

#define INVALID_VAR_INDEX (-1)

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

#define APPEND_FIELD_TO_LIST(module, field, KIND, kind, loc_, item) \
  do {                                                              \
    field = wasm_append_module_field(parser->allocator, module);    \
    field->loc = loc_;                                              \
    field->type = WASM_MODULE_FIELD_TYPE_##KIND;                    \
    field->kind = item;                                             \
  } while (0)

#define APPEND_ITEM_TO_VECTOR(module, Kind, kind, kinds, item_ptr)      \
  do {                                                                  \
    Wasm##Kind* dummy = item_ptr;                                       \
    wasm_append_##kind##_ptr_value(parser->allocator, &(module)->kinds, \
                                   &dummy);                             \
  } while (0)

#define INSERT_BINDING(module, kind, kinds, loc_, name)            \
  do                                                               \
    if ((name).start) {                                            \
      WasmBinding* binding = wasm_insert_binding(                  \
          parser->allocator, &(module)->kind##_bindings, &(name)); \
      binding->loc = loc_;                                         \
      binding->index = (module)->kinds.size - 1;                   \
    }                                                              \
  while (0)

#define APPEND_INLINE_EXPORT(module, KIND, loc_, value, index_)         \
  do                                                                    \
    if ((value).export_.has_export) {                                   \
      WasmModuleField* export_field;                                    \
      APPEND_FIELD_TO_LIST(module, export_field, EXPORT, export_, loc_, \
                           (value).export_.export_);                    \
      export_field->export_.kind = WASM_EXTERNAL_KIND_##KIND;           \
      export_field->export_.var.loc = loc_;                             \
      export_field->export_.var.index = index_;                         \
      APPEND_ITEM_TO_VECTOR(module, Export, export, exports,            \
                            &export_field->export_);                    \
      INSERT_BINDING(module, export, exports, export_field->loc,        \
                     export_field->export_.name);                       \
    }                                                                   \
  while (0)

#define CHECK_IMPORT_ORDERING(module, kind, kinds, loc_)           \
  do {                                                             \
    if ((module)->kinds.size != (module)->num_##kind##_imports) {  \
      wasm_ast_parser_error(                                       \
          &loc_, lexer, parser,                                    \
          "imports must occur before all non-import definitions"); \
    }                                                              \
  } while (0)

#define YYMALLOC(size) wasm_alloc(parser->allocator, size, WASM_DEFAULT_ALIGN)
#define YYFREE(p) wasm_free(parser->allocator, p)

#define USE_NATURAL_ALIGNMENT (~0)

static WasmExprList join_exprs1(WasmLocation* loc, WasmExpr* expr1);
static WasmExprList join_exprs2(WasmLocation* loc, WasmExprList* expr1,
                                WasmExpr* expr2);

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
%token ASSERT_MALFORMED ASSERT_INVALID ASSERT_UNLINKABLE
%token ASSERT_RETURN ASSERT_RETURN_NAN ASSERT_TRAP
%token INPUT OUTPUT
%token EOF 0 "EOF"

%type<opcode> BINARY COMPARE CONVERT LOAD STORE UNARY
%type<text> ALIGN_EQ_NAT OFFSET_EQ_NAT TEXT VAR
%type<type> SELECT
%type<type> CONST VALUE_TYPE
%type<literal> NAT INT FLOAT

%type<action> action
%type<block> block
%type<command> assertion cmd
%type<commands> cmd_list
%type<const_> const
%type<consts> const_list
%type<data_segment> data
%type<elem_segment> elem
%type<export_> export export_kind
%type<exported_func> func
%type<exported_global> global
%type<exported_table> table
%type<exported_memory> memory
%type<expr> plain_instr block_instr
%type<expr_list> instr instr_list expr expr1 expr_list if_ const_expr offset
%type<func_fields> func_fields func_body
%type<func> func_info
%type<func_sig> func_sig func_type
%type<func_type> type_def
%type<global> global_type
%type<import> import import_kind inline_import
%type<limits> limits
%type<memory> memory_sig
%type<module> module module_fields
%type<optional_export> inline_export inline_export_opt
%type<raw_module> raw_module
%type<literal> literal
%type<script> script
%type<table> table_sig
%type<text> bind_var bind_var_opt labeling_opt quoted_text
%type<text_list> non_empty_text_list text_list
%type<types> value_type_list
%type<u32> align_opt
%type<u64> nat offset_opt
%type<vars> var_list
%type<var> start type_use var script_var_opt

/* These non-terminals use the types below that have destructors, but the
 * memory is shared with the lexer, so should not be destroyed. */
%destructor {} ALIGN_EQ_NAT OFFSET_EQ_NAT TEXT VAR NAT INT FLOAT
%destructor { wasm_destroy_block(parser->allocator, &$$); } <block>
%destructor { wasm_destroy_command(parser->allocator, $$); wasm_free(parser->allocator, $$); } <command>
%destructor { wasm_destroy_command_vector_and_elements(parser->allocator, &$$); } <commands>
%destructor { wasm_destroy_const_vector(parser->allocator, &$$); } <consts>
%destructor { wasm_destroy_elem_segment(parser->allocator, &$$); } <elem_segment>
%destructor { wasm_destroy_export(parser->allocator, &$$); } <export_>
%destructor { wasm_destroy_exported_func(parser->allocator, &$$); } <exported_func>
%destructor { wasm_destroy_exported_memory(parser->allocator, &$$); } <exported_memory>
%destructor { wasm_destroy_exported_table(parser->allocator, &$$); } <exported_table>
%destructor { wasm_destroy_expr(parser->allocator, $$); } <expr>
%destructor { wasm_destroy_expr_list(parser->allocator, $$.first); } <expr_list>
%destructor { wasm_destroy_func_fields(parser->allocator, $$); } <func_fields>
%destructor { wasm_destroy_func(parser->allocator, $$); wasm_free(parser->allocator, $$); } <func>
%destructor { wasm_destroy_func_signature(parser->allocator, &$$); } <func_sig>
%destructor { wasm_destroy_func_type(parser->allocator, &$$); } <func_type>
%destructor { wasm_destroy_import(parser->allocator, $$); wasm_free(parser->allocator, $$); } <import>
%destructor { wasm_destroy_data_segment(parser->allocator, &$$); } <data_segment>
%destructor { wasm_destroy_module(parser->allocator, $$); wasm_free(parser->allocator, $$); } <module>
%destructor { wasm_destroy_raw_module(parser->allocator, &$$); } <raw_module>
%destructor { wasm_destroy_string_slice(parser->allocator, &$$.text); } <literal>
%destructor { wasm_destroy_script(&$$); } <script>
%destructor { wasm_destroy_string_slice(parser->allocator, &$$); } <text>
%destructor { wasm_destroy_text_list(parser->allocator, &$$); } <text_list>
%destructor { wasm_destroy_type_vector(parser->allocator, &$$); } <types>
%destructor { wasm_destroy_var_vector_and_elements(parser->allocator, &$$); } <vars>
%destructor { wasm_destroy_var(parser->allocator, &$$); } <var>


%nonassoc LOW
%nonassoc VAR

%start script_start

%%

/* Auxiliaries */

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
global_type :
    VALUE_TYPE {
      WASM_ZERO_MEMORY($$);
      $$.type = $1;
      $$.mutable_ = WASM_FALSE;
    }
  | LPAR MUT VALUE_TYPE RPAR {
      WASM_ZERO_MEMORY($$);
      $$.type = $3;
      $$.mutable_ = WASM_TRUE;
    }
;
func_type :
    LPAR FUNC func_sig RPAR { $$ = $3; }
;
func_sig :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | LPAR PARAM value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.param_types = $3;
    }
  | LPAR PARAM value_type_list RPAR LPAR RESULT value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.param_types = $3;
      $$.result_types = $7;
    }
  | LPAR RESULT value_type_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.result_types = $3;
    }
;

table_sig :
    limits elem_type { $$.elem_limits = $1; }
;
memory_sig :
    limits { $$.page_limits = $1; }
;
limits :
    nat {
      $$.has_max = WASM_FALSE;
      $$.initial = $1;
      $$.max = 0;
    }
  | nat nat {
      $$.has_max = WASM_TRUE;
      $$.initial = $1;
      $$.max = $2;
    }
;
type_use :
    LPAR TYPE var RPAR { $$ = $3; }
;

/* Expressions */

nat :
    NAT {
      if (WASM_FAILED(wasm_parse_uint64($1.text.start,
                                        $1.text.start + $1.text.length, &$$))) {
        wasm_ast_parser_error(&@1, lexer, parser,
                              "invalid int " PRIstringslice "\"",
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
    nat {
      $$.loc = @1;
      $$.type = WASM_VAR_TYPE_INDEX;
      $$.index = $1;
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
bind_var_opt :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | bind_var
;
bind_var :
    VAR { DUPTEXT($$, $1); }
;

labeling_opt :
    /* empty */ %prec LOW { WASM_ZERO_MEMORY($$); }
  | bind_var
;

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

instr :
    plain_instr { $$ = join_exprs1(&@1, $1); }
  | block_instr { $$ = join_exprs1(&@1, $1); }
  | expr { $$ = $1; }
;
plain_instr :
    UNREACHABLE {
      $$ = wasm_new_unreachable_expr(parser->allocator);
    }
  | NOP {
      $$ = wasm_new_nop_expr(parser->allocator);
    }
  | DROP {
      $$ = wasm_new_drop_expr(parser->allocator);
    }
  | SELECT {
      $$ = wasm_new_select_expr(parser->allocator);
    }
  | BR var {
      $$ = wasm_new_br_expr(parser->allocator);
      $$->br.var = $2;
    }
  | BR_IF var {
      $$ = wasm_new_br_if_expr(parser->allocator);
      $$->br_if.var = $2;
    }
  | BR_TABLE var_list var {
      $$ = wasm_new_br_table_expr(parser->allocator);
      $$->br_table.targets = $2;
      $$->br_table.default_target = $3;
    }
  | RETURN {
      $$ = wasm_new_return_expr(parser->allocator);
    }
  | CALL var {
      $$ = wasm_new_call_expr(parser->allocator);
      $$->call.var = $2;
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
block_instr :
    BLOCK labeling_opt block END {
      $$ = wasm_new_block_expr(parser->allocator);
      $$->block = $3;
      $$->block.label = $2;
    }
  | LOOP labeling_opt block END {
      $$ = wasm_new_loop_expr(parser->allocator);
      $$->loop = $3;
      $$->loop.label = $2;
    }
  | IF labeling_opt block END {
      $$ = wasm_new_if_expr(parser->allocator);
      $$->if_.true_ = $3;
      $$->if_.true_.label = $2;
    }
  | IF labeling_opt block ELSE instr_list END {
      $$ = wasm_new_if_expr(parser->allocator);
      $$->if_.true_ = $3;
      $$->if_.true_.label = $2;
      $$->if_.false_ = $5.first;
    }
;
block :
    value_type_list instr_list {
      WASM_ZERO_MEMORY($$);
      $$.sig = $1;
      $$.first = $2.first;
    }
;

expr :
    LPAR expr1 RPAR { $$ = $2; }
;

expr1 :
    plain_instr expr_list {
      $$ = join_exprs2(&@1, &$2, $1);
    }
  | BLOCK labeling_opt block {
      WasmExpr* expr = wasm_new_block_expr(parser->allocator);
      expr->block = $3;
      expr->block.label = $2;
      $$ = join_exprs1(&@1, expr);
    }
  | LOOP labeling_opt block {
      WasmExpr* expr = wasm_new_loop_expr(parser->allocator);
      expr->loop = $3;
      expr->loop.label = $2;
      $$ = join_exprs1(&@1, expr);
    }
  | IF labeling_opt value_type_list if_ {
      $$ = $4;
      WasmExpr* if_ = $4.last;
      assert(if_->type == WASM_EXPR_TYPE_IF);
      if_->if_.true_.label = $2;
      if_->if_.true_.sig = $3;
    }
;
if_ :
    LPAR THEN instr_list RPAR LPAR ELSE instr_list RPAR {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $3.first;
      expr->if_.false_ = $7.first;
      $$ = join_exprs1(&@1, expr);
    }
  | LPAR THEN instr_list RPAR {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $3.first;
      $$ = join_exprs1(&@1, expr);
    }
  | expr LPAR THEN instr_list RPAR LPAR ELSE instr_list RPAR {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $4.first;
      expr->if_.false_ = $8.first;
      $$ = join_exprs2(&@1, &$1, expr);
    }
  | expr LPAR THEN instr_list RPAR {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $4.first;
      $$ = join_exprs2(&@1, &$1, expr);
    }
  | expr expr expr {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $2.first;
      expr->if_.false_ = $3.first;
      $$ = join_exprs2(&@1, &$1, expr);
    }
  | expr expr {
      WasmExpr* expr = wasm_new_if_expr(parser->allocator);
      expr->if_.true_.first = $2.first;
      $$ = join_exprs2(&@1, &$1, expr);
    }
;

instr_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | instr instr_list {
      $$.first = $1.first;
      $1.last->next = $2.first;
      $$.last = $2.last ? $2.last : $1.last;
      $$.size = $1.size + $2.size;
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

const_expr :
    instr_list
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
    instr_list {
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
            $$->decl.sig.result_types = field->types;
            break;
        }

        /* we steal memory from the func field, but not the linked list nodes */
        wasm_free(parser->allocator, field);
        field = next;
      }
    }
;
func :
    LPAR FUNC bind_var_opt inline_export type_use func_info RPAR {
      WASM_ZERO_MEMORY($$);
      $$.func = $6;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $5;
      $$.func->name = $3;
      $$.export_ = $4;
    }
  /* Duplicate above for empty inline_export_opt to avoid LR(1) conflict. */
  | LPAR FUNC bind_var_opt type_use func_info RPAR {
      WASM_ZERO_MEMORY($$);
      $$.func = $5;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $4;
      $$.func->name = $3;
    }
  | LPAR FUNC bind_var_opt inline_export func_info RPAR {
      WASM_ZERO_MEMORY($$);
      $$.func = $5;
      $$.func->name = $3;
      $$.export_ = $4;
    }
  /* Duplicate above for empty inline_export_opt to avoid LR(1) conflict. */
  | LPAR FUNC bind_var_opt func_info RPAR {
      WASM_ZERO_MEMORY($$);
      $$.func = $4;
      $$.func->name = $3;
    }
;

/* Tables & Memories */

offset :
    LPAR OFFSET const_expr RPAR {
      $$ = $3;
    }
  | expr
;

elem :
    LPAR ELEM var offset var_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.table_var = $3;
      $$.offset = $4.first;
      $$.vars = $5;
    }
  | LPAR ELEM offset var_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.table_var.loc = @2;
      $$.table_var.type = WASM_VAR_TYPE_INDEX;
      $$.table_var.index = 0;
      $$.offset = $3.first;
      $$.vars = $4;
    }
;

table :
    LPAR TABLE bind_var_opt inline_export_opt table_sig RPAR {
      $$.table = $5;
      $$.table.name = $3;
      $$.has_elem_segment = WASM_FALSE;
      $$.export_ = $4;
    }
  | LPAR TABLE bind_var_opt inline_export_opt elem_type
         LPAR ELEM var_list RPAR RPAR {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->loc = @2;
      expr->const_.type = WASM_TYPE_I32;
      expr->const_.u32 = 0;

      WASM_ZERO_MEMORY($$);
      $$.table.name = $3;
      $$.table.elem_limits.initial = $8.size;
      $$.table.elem_limits.max = $8.size;
      $$.table.elem_limits.has_max = WASM_TRUE;
      $$.has_elem_segment = WASM_TRUE;
      $$.elem_segment.offset = expr;
      $$.elem_segment.vars = $8;
      $$.export_ = $4;
    }
;

data :
    LPAR DATA var offset text_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.memory_var = $3;
      $$.offset = $4.first;
      dup_text_list(parser->allocator, &$5, &$$.data, &$$.size);
      wasm_destroy_text_list(parser->allocator, &$5);
    }
  | LPAR DATA offset text_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.memory_var.loc = @2;
      $$.memory_var.type = WASM_VAR_TYPE_INDEX;
      $$.memory_var.index = 0;
      $$.offset = $3.first;
      dup_text_list(parser->allocator, &$4, &$$.data, &$$.size);
      wasm_destroy_text_list(parser->allocator, &$4);
    }
;

memory :
    LPAR MEMORY bind_var_opt inline_export_opt memory_sig RPAR {
      WASM_ZERO_MEMORY($$);
      $$.memory = $5;
      $$.memory.name = $3;
      $$.has_data_segment = WASM_FALSE;
      $$.export_ = $4;
    }
  | LPAR MEMORY bind_var_opt inline_export LPAR DATA text_list RPAR RPAR {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->loc = @2;
      expr->const_.type = WASM_TYPE_I32;
      expr->const_.u32 = 0;

      WASM_ZERO_MEMORY($$);
      $$.has_data_segment = WASM_TRUE;
      $$.data_segment.offset = expr;
      dup_text_list(parser->allocator, &$7, &$$.data_segment.data,
                    &$$.data_segment.size);
      wasm_destroy_text_list(parser->allocator, &$7);
      uint32_t byte_size = WASM_ALIGN_UP_TO_PAGE($$.data_segment.size);
      uint32_t page_size = WASM_BYTES_TO_PAGES(byte_size);
      $$.memory.name = $3;
      $$.memory.page_limits.initial = page_size;
      $$.memory.page_limits.max = page_size;
      $$.memory.page_limits.has_max = WASM_TRUE;
      $$.export_ = $4;
    }
  /* Duplicate above for empty inline_export_opt to avoid LR(1) conflict. */
  | LPAR MEMORY bind_var_opt LPAR DATA text_list RPAR RPAR {
      WasmExpr* expr = wasm_new_const_expr(parser->allocator);
      expr->loc = @2;
      expr->const_.type = WASM_TYPE_I32;
      expr->const_.u32 = 0;

      WASM_ZERO_MEMORY($$);
      $$.has_data_segment = WASM_TRUE;
      $$.data_segment.offset = expr;
      dup_text_list(parser->allocator, &$6, &$$.data_segment.data,
                    &$$.data_segment.size);
      wasm_destroy_text_list(parser->allocator, &$6);
      uint32_t byte_size = WASM_ALIGN_UP_TO_PAGE($$.data_segment.size);
      uint32_t page_size = WASM_BYTES_TO_PAGES(byte_size);
      $$.memory.name = $3;
      $$.memory.page_limits.initial = page_size;
      $$.memory.page_limits.max = page_size;
      $$.memory.page_limits.has_max = WASM_TRUE;
      $$.export_.has_export = WASM_FALSE;
    }
;

global :
    LPAR GLOBAL bind_var_opt inline_export global_type const_expr RPAR {
      WASM_ZERO_MEMORY($$);
      $$.global = $5;
      $$.global.name = $3;
      $$.global.init_expr = $6.first;
      $$.export_ = $4;
    }
  | LPAR GLOBAL bind_var_opt global_type const_expr RPAR {
      WASM_ZERO_MEMORY($$);
      $$.global = $4;
      $$.global.name = $3;
      $$.global.init_expr = $5.first;
      $$.export_.has_export = WASM_FALSE;
    }
;


/* Imports & Exports */

import_kind :
    LPAR FUNC bind_var_opt type_use RPAR {
      $$ = new_import(parser->allocator);
      $$->kind = WASM_EXTERNAL_KIND_FUNC;
      $$->func.name = $3;
      $$->func.decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$->func.decl.type_var = $4;
    }
  | LPAR FUNC bind_var_opt func_sig RPAR {
      $$ = new_import(parser->allocator);
      $$->kind = WASM_EXTERNAL_KIND_FUNC;
      $$->func.name = $3;
      $$->func.decl.sig = $4;
    }
  | LPAR TABLE bind_var_opt table_sig RPAR {
      $$ = new_import(parser->allocator);
      $$->kind = WASM_EXTERNAL_KIND_TABLE;
      $$->table = $4;
      $$->table.name = $3;
    }
  | LPAR MEMORY bind_var_opt memory_sig RPAR {
      $$ = new_import(parser->allocator);
      $$->kind = WASM_EXTERNAL_KIND_MEMORY;
      $$->memory = $4;
      $$->memory.name = $3;
    }
  | LPAR GLOBAL bind_var_opt global_type RPAR {
      $$ = new_import(parser->allocator);
      $$->kind = WASM_EXTERNAL_KIND_GLOBAL;
      $$->global = $4;
      $$->global.name = $3;
    }
;
import :
    LPAR IMPORT quoted_text quoted_text import_kind RPAR {
      $$ = $5;
      $$->module_name = $3;
      $$->field_name = $4;
    }
  | LPAR FUNC bind_var_opt inline_import type_use RPAR {
      $$ = $4;
      $$->kind = WASM_EXTERNAL_KIND_FUNC;
      $$->func.name = $3;
      $$->func.decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$->func.decl.type_var = $5;
    }
  | LPAR FUNC bind_var_opt inline_import func_sig RPAR {
      $$ = $4;
      $$->kind = WASM_EXTERNAL_KIND_FUNC;
      $$->func.name = $3;
      $$->func.decl.sig = $5;
    }
  | LPAR TABLE bind_var_opt inline_import table_sig RPAR {
      $$ = $4;
      $$->kind = WASM_EXTERNAL_KIND_TABLE;
      $$->table = $5;
      $$->table.name = $3;
    }
  | LPAR MEMORY bind_var_opt inline_import memory_sig RPAR {
      $$ = $4;
      $$->kind = WASM_EXTERNAL_KIND_MEMORY;
      $$->memory = $5;
      $$->memory.name = $3;
    }
  | LPAR GLOBAL bind_var_opt inline_import global_type RPAR {
      $$ = $4;
      $$->kind = WASM_EXTERNAL_KIND_GLOBAL;
      $$->global = $5;
      $$->global.name = $3;
    }
;

inline_import :
    LPAR IMPORT quoted_text quoted_text RPAR {
      $$ = new_import(parser->allocator);
      $$->module_name = $3;
      $$->field_name = $4;
    }
;

export_kind :
    LPAR FUNC var RPAR {
      WASM_ZERO_MEMORY($$);
      $$.kind = WASM_EXTERNAL_KIND_FUNC;
      $$.var = $3;
    }
  | LPAR TABLE var RPAR {
      WASM_ZERO_MEMORY($$);
      $$.kind = WASM_EXTERNAL_KIND_TABLE;
      $$.var = $3;
    }
  | LPAR MEMORY var RPAR {
      WASM_ZERO_MEMORY($$);
      $$.kind = WASM_EXTERNAL_KIND_MEMORY;
      $$.var = $3;
    }
  | LPAR GLOBAL var RPAR {
      WASM_ZERO_MEMORY($$);
      $$.kind = WASM_EXTERNAL_KIND_GLOBAL;
      $$.var = $3;
    }
;
export :
    LPAR EXPORT quoted_text export_kind RPAR {
      $$ = $4;
      $$.name = $3;
    }
;

inline_export_opt :
    /* empty */ { 
      WASM_ZERO_MEMORY($$);
      $$.has_export = WASM_FALSE;
    }
  | inline_export
;
inline_export :
    LPAR EXPORT quoted_text RPAR {
      WASM_ZERO_MEMORY($$);
      $$.has_export = WASM_TRUE;
      $$.export_.name = $3;
    }
;


/* Modules */

type_def :
    LPAR TYPE func_type RPAR {
      WASM_ZERO_MEMORY($$);
      $$.sig = $3;
    }
  | LPAR TYPE bind_var func_type RPAR {
      $$.name = $3;
      $$.sig = $4;
    }
;

start :
    LPAR START var RPAR { $$ = $3; }
;

module_fields :
    /* empty */ {
      $$ = new_module(parser->allocator);
    }
  | module_fields type_def {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, FUNC_TYPE, func_type, @2, $2);
      APPEND_ITEM_TO_VECTOR($$, FuncType, func_type, func_types,
                            &field->func_type);
      INSERT_BINDING($$, func_type, func_types, @2, $2.name);
    }
  | module_fields global {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, GLOBAL, global, @2, $2.global);
      APPEND_ITEM_TO_VECTOR($$, Global, global, globals, &field->global);
      INSERT_BINDING($$, global, globals, @2, $2.global.name);
      APPEND_INLINE_EXPORT($$, GLOBAL, @2, $2, $$->globals.size - 1);
    }
  | module_fields table {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, TABLE, table, @2, $2.table);
      APPEND_ITEM_TO_VECTOR($$, Table, table, tables, &field->table);
      INSERT_BINDING($$, table, tables, @2, $2.table.name);
      APPEND_INLINE_EXPORT($$, TABLE, @2, $2, $$->tables.size - 1);

      if ($2.has_elem_segment) {
        WasmModuleField* elem_segment_field;
        APPEND_FIELD_TO_LIST($$, elem_segment_field, ELEM_SEGMENT, elem_segment,
                             @2, $2.elem_segment);
        APPEND_ITEM_TO_VECTOR($$, ElemSegment, elem_segment, elem_segments,
                              &elem_segment_field->elem_segment);
      }

    }
  | module_fields memory {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, MEMORY, memory, @2, $2.memory);
      APPEND_ITEM_TO_VECTOR($$, Memory, memory, memories, &field->memory);
      INSERT_BINDING($$, memory, memories, @2, $2.memory.name);
      APPEND_INLINE_EXPORT($$, MEMORY, @2, $2, $$->memories.size - 1);

      if ($2.has_data_segment) {
        WasmModuleField* data_segment_field;
        APPEND_FIELD_TO_LIST($$, data_segment_field, DATA_SEGMENT, data_segment,
                             @2, $2.data_segment);
        APPEND_ITEM_TO_VECTOR($$, DataSegment, data_segment, data_segments,
                              &data_segment_field->data_segment);
      }
    }
  | module_fields func {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, FUNC, func, @2, *$2.func);
      append_implicit_func_declaration(parser->allocator, &@2, $$,
                                       &field->func.decl);
      APPEND_ITEM_TO_VECTOR($$, Func, func, funcs, &field->func);
      INSERT_BINDING($$, func, funcs, @2, $2.func->name);
      APPEND_INLINE_EXPORT($$, FUNC, @2, $2, $$->funcs.size - 1);
      wasm_free(parser->allocator, $2.func);
    }
  | module_fields elem {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, ELEM_SEGMENT, elem_segment, @2, $2);
      APPEND_ITEM_TO_VECTOR($$, ElemSegment, elem_segment, elem_segments,
                            &field->elem_segment);
    }
  | module_fields data {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, DATA_SEGMENT, data_segment, @2, $2);
      APPEND_ITEM_TO_VECTOR($$, DataSegment, data_segment, data_segments,
                            &field->data_segment);
    }
  | module_fields start {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, START, start, @2, $2);
      $$->start = &field->start;
    }
  | module_fields import {
      $$ = $1;
      WasmModuleField* field;
      APPEND_FIELD_TO_LIST($$, field, IMPORT, import, @2, *$2);
      CHECK_IMPORT_ORDERING($$, func, funcs, @2);
      CHECK_IMPORT_ORDERING($$, table, tables, @2);
      CHECK_IMPORT_ORDERING($$, memory, memories, @2);
      CHECK_IMPORT_ORDERING($$, global, globals, @2);
      switch ($2->kind) {
        case WASM_EXTERNAL_KIND_FUNC:
          append_implicit_func_declaration(parser->allocator, &@2, $$,
                                           &field->import.func.decl);
          APPEND_ITEM_TO_VECTOR($$, Func, func, funcs, &field->import.func);
          INSERT_BINDING($$, func, funcs, @2, field->import.func.name);
          $$->num_func_imports++;
          break;
        case WASM_EXTERNAL_KIND_TABLE:
          APPEND_ITEM_TO_VECTOR($$, Table, table, tables, &field->import.table);
          INSERT_BINDING($$, table, tables, @2, field->import.table.name);
          $$->num_table_imports++;
          break;
        case WASM_EXTERNAL_KIND_MEMORY:
          APPEND_ITEM_TO_VECTOR($$, Memory, memory, memories,
                                &field->import.memory);
          INSERT_BINDING($$, memory, memories, @2, field->import.memory.name);
          $$->num_memory_imports++;
          break;
        case WASM_EXTERNAL_KIND_GLOBAL:
          APPEND_ITEM_TO_VECTOR($$, Global, global, globals,
                                &field->import.global);
          INSERT_BINDING($$, global, globals, @2, field->import.global.name);
          $$->num_global_imports++;
          break;
        case WASM_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
      wasm_free(parser->allocator, $2);
      APPEND_ITEM_TO_VECTOR($$, Import, import, imports, &field->import);
    }
  | module_fields export {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      APPEND_FIELD_TO_LIST($$, field, EXPORT, export_, @2, $2);
      APPEND_ITEM_TO_VECTOR($$, Export, export, exports, &field->export_);
      INSERT_BINDING($$, export, exports, @2, $2.name);
    }
;

raw_module :
    LPAR MODULE bind_var_opt module_fields RPAR {
      $$.type = WASM_RAW_MODULE_TYPE_TEXT;
      $$.text = $4;
      $$.text->name = $3;
      $$.text->loc = @2;

      /* resolve func type variables where the signature was not specified
       * explicitly */
      size_t i;
      for (i = 0; i < $4->funcs.size; ++i) {
        WasmFunc* func = $4->funcs.data[i];
        if (wasm_decl_has_func_type(&func->decl) &&
            is_empty_signature(&func->decl.sig)) {
          WasmFuncType* func_type =
              wasm_get_func_type_by_var($4, &func->decl.type_var);
          if (func_type) {
            func->decl.sig = func_type->sig;
            func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
          }
        }
      }
    }
  | LPAR MODULE bind_var_opt non_empty_text_list RPAR {
      $$.type = WASM_RAW_MODULE_TYPE_BINARY;
      $$.binary.name = $3;
      $$.binary.loc = @2;
      dup_text_list(parser->allocator, &$4, &$$.binary.data, &$$.binary.size);
      wasm_destroy_text_list(parser->allocator, &$4);
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
        user_data.loc = &$1.binary.loc;
        user_data.lexer = lexer;
        user_data.parser = parser;
        WasmBinaryErrorHandler error_handler;
        error_handler.on_error = on_read_binary_error;
        error_handler.user_data = &user_data;
        wasm_read_binary_ast(parser->allocator, $1.binary.data, $1.binary.size,
                             &options, &error_handler, $$);
        wasm_free(parser->allocator, $1.binary.data);
        $$->name = $1.binary.name;
        $$->loc = $1.binary.loc;
      }
    }
;

/* Scripts */

script_var_opt :
    /* empty */ {
      WASM_ZERO_MEMORY($$);
      $$.type = WASM_VAR_TYPE_INDEX;
      $$.index = INVALID_VAR_INDEX;
    }
  | VAR {
      WASM_ZERO_MEMORY($$);
      $$.type = WASM_VAR_TYPE_NAME;
      DUPTEXT($$.name, $1);
    }
;

action :
    LPAR INVOKE script_var_opt quoted_text const_list RPAR {
      WASM_ZERO_MEMORY($$);
      $$.loc = @2;
      $$.module_var = $3;
      $$.type = WASM_ACTION_TYPE_INVOKE;
      $$.invoke.name = $4;
      $$.invoke.args = $5;
    }
  | LPAR GET script_var_opt quoted_text RPAR {
      WASM_ZERO_MEMORY($$);
      $$.loc = @2;
      $$.module_var = $3;
      $$.type = WASM_ACTION_TYPE_GET;
      $$.invoke.name = $4;
    }
;

assertion :
    LPAR ASSERT_MALFORMED raw_module quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_MALFORMED;
      $$->assert_malformed.module = $3;
      $$->assert_malformed.text = $4;
    }
  | LPAR ASSERT_INVALID raw_module quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_INVALID;
      $$->assert_invalid.module = $3;
      $$->assert_invalid.text = $4;
    }
  | LPAR ASSERT_UNLINKABLE raw_module quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_UNLINKABLE;
      $$->assert_unlinkable.module = $3;
      $$->assert_unlinkable.text = $4;
    }
  | LPAR ASSERT_RETURN action const_list RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_RETURN;
      $$->assert_return.action = $3;
      $$->assert_return.expected = $4;
    }
  | LPAR ASSERT_RETURN_NAN action RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_RETURN_NAN;
      $$->assert_return_nan.action = $3;
    }
  | LPAR ASSERT_TRAP action quoted_text RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ASSERT_TRAP;
      $$->assert_trap.action = $3;
      $$->assert_trap.text = $4;
    }
;

cmd :
    action {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_ACTION;
      $$->action = $1;
    }
  | assertion
  | module {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_MODULE;
      $$->module = *$1;
      wasm_free(parser->allocator, $1);
    }
  | LPAR REGISTER quoted_text script_var_opt RPAR {
      $$ = new_command(parser->allocator);
      $$->type = WASM_COMMAND_TYPE_REGISTER;
      $$->register_.module_name = $3;
      $$->register_.var = $4;
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
const_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | const_list const {
      $$ = $1;
      wasm_append_const_value(parser->allocator, &$$, &$2);
    }
;

script :
    cmd_list {
      WASM_ZERO_MEMORY($$);
      $$.allocator = parser->allocator;
      $$.commands = $1;

      int last_module_index = -1;
      size_t i;
      for (i = 0; i < $$.commands.size; ++i) {
        WasmCommand* command = &$$.commands.data[0];
        WasmAction* action = NULL;
        switch (command->type) {
          case WASM_COMMAND_TYPE_MODULE: {
            /* Wire up module name bindings. */
            WasmModule* module = &command->module;
            if (module->name.length == 0)
              continue;
            WasmStringSlice module_name =
                wasm_dup_string_slice(parser->allocator, module->name);
            INSERT_BINDING(&$$, module, commands, module->loc, module_name);
            last_module_index = i;
            break;
          }

          case WASM_COMMAND_TYPE_ASSERT_RETURN:
            action = &command->assert_return.action;
            goto has_action;
          case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
            action = &command->assert_return_nan.action;
            goto has_action;
          case WASM_COMMAND_TYPE_ASSERT_TRAP:
            action = &command->assert_trap.action;
            goto has_action;
          case WASM_COMMAND_TYPE_ACTION:
            action = &command->action;
            goto has_action;

          has_action: {
            /* Resolve actions with an invalid index to use the preceding
             * module. */
            WasmVar* var = &action->module_var;
            if (var->type == WASM_VAR_TYPE_INDEX &&
                var->index == INVALID_VAR_INDEX) {
              var->index = last_module_index;
            }
            break;
          }

          default:
            break;
        }
      }
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
  return sig->result_types.size == 0 && sig->param_types.size == 0;
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

WasmResult wasm_parse_ast(WasmAstLexer* lexer,
                          struct WasmScript* out_script,
                          WasmSourceErrorHandler* error_handler) {
  WasmAstParser parser;
  WASM_ZERO_MEMORY(parser);
  WasmAllocator* allocator = wasm_ast_lexer_get_allocator(lexer);
  parser.allocator = allocator;
  parser.script.allocator = allocator;
  parser.error_handler = error_handler;
  int result = wasm_ast_parser_parse(lexer, &parser);
  *out_script = parser.script;
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
