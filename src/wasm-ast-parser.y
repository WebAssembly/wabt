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

static void copy_signature_from_func_type(
    WasmAllocator* allocator, WasmModule* module, WasmFuncDeclaration* decl);

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
%token INT FLOAT TEXT VAR VALUE_TYPE
%token NOP BLOCK IF THEN ELSE LOOP BR BR_IF BR_TABLE CASE
%token CALL CALL_IMPORT CALL_INDIRECT RETURN
%token GET_LOCAL SET_LOCAL LOAD STORE OFFSET ALIGN
%token CONST UNARY BINARY COMPARE CONVERT SELECT
%token FUNC START TYPE PARAM RESULT LOCAL
%token MODULE MEMORY SEGMENT IMPORT EXPORT TABLE
%token UNREACHABLE CURRENT_MEMORY GROW_MEMORY
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
%type<exported_func> func
%type<expr> expr expr1 expr_opt
%type<expr_list> expr_list non_empty_expr_list
%type<func_fields> func_fields
%type<func> func_info
%type<func_sig> func_type
%type<func_type> type_def
%type<import> import
%type<memory> memory
%type<module> module module_fields
%type<raw_module> raw_module
%type<literal> literal
%type<script> script
%type<segment> segment segment_contents
%type<segments> segment_list
%type<text> bind_var labeling quoted_text export_opt
%type<text_list> text_list
%type<types> value_type_list
%type<u32> align segment_address
%type<u64> offset initial_pages max_pages
%type<vars> table var_list
%type<var> start type_use var

%destructor { wasm_destroy_text_list(parser->allocator, &$$); } text_list
%destructor { wasm_destroy_string_slice(parser->allocator, &$$); } bind_var labeling quoted_text
%destructor { wasm_destroy_string_slice(parser->allocator, &$$.text); } literal
%destructor { wasm_destroy_type_vector(parser->allocator, &$$); } value_type_list
%destructor { wasm_destroy_var(parser->allocator, &$$); } var
%destructor { wasm_destroy_var_vector_and_elements(parser->allocator, &$$); } table var_list
%destructor { wasm_destroy_expr(parser->allocator, $$); } expr expr1 expr_opt
%destructor { wasm_destroy_expr_list(parser->allocator, $$.first); } expr_list non_empty_expr_list
%destructor { wasm_destroy_func_fields(parser->allocator, $$); } func_fields
%destructor { wasm_destroy_func(parser->allocator, $$); wasm_free(parser->allocator, $$); } func_info
%destructor { wasm_destroy_segment(parser->allocator, &$$); } segment segment_contents
%destructor { wasm_destroy_segment_vector_and_elements(parser->allocator, &$$); } segment_list
%destructor { wasm_destroy_memory(parser->allocator, &$$); } memory
%destructor { wasm_destroy_func_signature(parser->allocator, &$$); } func_type
%destructor { wasm_destroy_func_type(parser->allocator, &$$); } type_def
%destructor { wasm_destroy_import(parser->allocator, $$); wasm_free(parser->allocator, $$); } import
%destructor { wasm_destroy_export(parser->allocator, &$$); } export
%destructor { wasm_destroy_exported_func(parser->allocator, &$$); } func
%destructor { wasm_destroy_module(parser->allocator, $$); wasm_free(parser->allocator, $$); } module module_fields
%destructor { wasm_destroy_raw_module(parser->allocator, &$$); } raw_module
%destructor { wasm_destroy_const_vector(parser->allocator, &$$); } const_list
%destructor { wasm_destroy_command(parser->allocator, $$); wasm_free(parser->allocator, $$); } cmd
%destructor { wasm_destroy_command_vector_and_elements(parser->allocator, &$$); } cmd_list
%destructor { wasm_destroy_script(&$$); } script

%nonassoc LOW
%nonassoc VAR

%start script_start

%%

text_list :
    TEXT {
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, $1);
      node->next = NULL;
      $$.first = $$.last = node;
    }
  | text_list TEXT {
      $$ = $1;
      WasmTextListNode* node = new_text_list_node(parser->allocator);
      DUPTEXT(node->text, $2);
      node->next = NULL;
      $$.last->next = node;
      $$.last = node;
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
    }
  | FLOAT {
      $$.type = $1.type;
      DUPTEXT($$.text, $1.text);
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

segment_contents :
    text_list {
      dup_text_list(parser->allocator, &$1, &$$.data, &$$.size);
      wasm_destroy_text_list(parser->allocator, &$1);
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
        wasm_ast_parser_error(&@1, lexer, parser,
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
        wasm_ast_parser_error(&@1, lexer, parser,
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
    }
  | BLOCK labeling expr_list {
      $$ = wasm_new_block_expr(parser->allocator);
      $$->block.label = $2;
      $$->block.first = $3.first;
    }
  | IF expr expr {
      $$ = wasm_new_if_expr(parser->allocator);
      $$->if_.cond = $2;
      $$->if_.true_.first = $3;
    }
  | IF expr LPAR THEN labeling expr_list RPAR {
      $$ = wasm_new_if_expr(parser->allocator);
      $$->if_.cond = $2;
      $$->if_.true_.label = $5;
      $$->if_.true_.first = $6.first;
    }
  | IF expr expr expr {
      $$ = wasm_new_if_else_expr(parser->allocator);
      $$->if_else.cond = $2;
      $$->if_else.true_.first = $3;
      $$->if_else.false_.first = $4;
    }
  | IF expr LPAR THEN labeling expr_list RPAR LPAR ELSE labeling expr_list RPAR {
      $$ = wasm_new_if_else_expr(parser->allocator);
      $$->if_else.cond = $2;
      $$->if_else.true_.label = $5;
      $$->if_else.true_.first = $6.first;
      $$->if_else.false_.label = $10;
      $$->if_else.false_.first = $11.first;
    }
  | BR_IF var expr {
      $$ = wasm_new_br_if_expr(parser->allocator);
      $$->br_if.var = $2;
      $$->br_if.cond = $3;
    }
  | BR_IF var expr expr {
      $$ = wasm_new_br_if_expr(parser->allocator);
      $$->br_if.var = $2;
      $$->br_if.expr = $3;
      $$->br_if.cond = $4;
    }
  | LOOP labeling expr_list {
      $$ = wasm_new_loop_expr(parser->allocator);
      WASM_ZERO_MEMORY($$->loop.outer);
      $$->loop.inner = $2;
      $$->loop.first = $3.first;
    }
  | LOOP bind_var bind_var expr_list {
      $$ = wasm_new_loop_expr(parser->allocator);
      $$->loop.outer = $2;
      $$->loop.inner = $3;
      $$->loop.first = $4.first;
    }
  | BR var expr_opt {
      $$ = wasm_new_br_expr(parser->allocator);
      $$->br.var = $2;
      $$->br.expr = $3;
    }
  | RETURN expr_opt {
      $$ = wasm_new_return_expr(parser->allocator);
      $$->return_.expr = $2;
    }
  | BR_TABLE var_list var expr {
      $$ = wasm_new_br_table_expr(parser->allocator);
      $$->br_table.key = $4;
      $$->br_table.expr = NULL;
      $$->br_table.targets = $2;
      $$->br_table.default_target = $3;
    }
  | BR_TABLE var_list var expr expr {
      $$ = wasm_new_br_table_expr(parser->allocator);
      $$->br_table.key = $5;
      $$->br_table.expr = $4;
      $$->br_table.targets = $2;
      $$->br_table.default_target = $3;
    }
  | CALL var expr_list {
      $$ = wasm_new_call_expr(parser->allocator);
      $$->call.var = $2;
      $$->call.first_arg = $3.first;
      $$->call.num_args = $3.size;
    }
  | CALL_IMPORT var expr_list {
      $$ = wasm_new_call_import_expr(parser->allocator);
      $$->call.var = $2;
      $$->call.first_arg = $3.first;
      $$->call.num_args = $3.size;
    }
  | CALL_INDIRECT var expr expr_list {
      $$ = wasm_new_call_indirect_expr(parser->allocator);
      $$->call_indirect.var = $2;
      $$->call_indirect.expr = $3;
      $$->call_indirect.first_arg = $4.first;
      $$->call_indirect.num_args = $4.size;
    }
  | GET_LOCAL var {
      $$ = wasm_new_get_local_expr(parser->allocator);
      $$->get_local.var = $2;
    }
  | SET_LOCAL var expr {
      $$ = wasm_new_set_local_expr(parser->allocator);
      $$->set_local.var = $2;
      $$->set_local.expr = $3;
    }
  | LOAD offset align expr {
      $$ = wasm_new_load_expr(parser->allocator);
      $$->load.opcode = $1;
      $$->load.offset = $2;
      $$->load.align = $3;
      $$->load.addr = $4;
    }
  | STORE offset align expr expr {
      $$ = wasm_new_store_expr(parser->allocator);
      $$->store.opcode = $1;
      $$->store.offset = $2;
      $$->store.align = $3;
      $$->store.addr = $4;
      $$->store.value = $5;
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
  | UNARY expr {
      $$ = wasm_new_unary_expr(parser->allocator);
      $$->unary.opcode = $1;
      $$->unary.expr = $2;
    }
  | BINARY expr expr {
      $$ = wasm_new_binary_expr(parser->allocator);
      $$->binary.opcode = $1;
      $$->binary.left = $2;
      $$->binary.right = $3;
    }
  | SELECT expr expr expr {
      $$ = wasm_new_select_expr(parser->allocator);
      $$->select.true_ = $2;
      $$->select.false_ = $3;
      $$->select.cond = $4;
    }
  | COMPARE expr expr {
      $$ = wasm_new_compare_expr(parser->allocator);
      $$->compare.opcode = $1;
      $$->compare.left = $2;
      $$->compare.right = $3;
    }
  | CONVERT expr {
      $$ = wasm_new_convert_expr(parser->allocator);
      $$->convert.opcode = $1;
      $$->convert.expr = $2;
    }
  | UNREACHABLE {
      $$ = wasm_new_empty_expr(parser->allocator, WASM_EXPR_TYPE_UNREACHABLE);
    }
  | CURRENT_MEMORY {
      $$ = wasm_new_empty_expr(parser->allocator,
                               WASM_EXPR_TYPE_CURRENT_MEMORY);
    }
  | GROW_MEMORY expr {
      $$ = wasm_new_grow_memory_expr(parser->allocator);
      $$->grow_memory.expr = $2;
    }
;
expr_opt :
    /* empty */ { $$ = NULL; }
  | expr
;
non_empty_expr_list :
    expr {
      $$.first = $$.last = $1;
      $$.size = 1;
    }
  | non_empty_expr_list expr {
      $$ = $1;
      $$.last->next = $2;
      $$.last = $2;
      $$.size++;
    }
;
expr_list :
    /* empty */ { WASM_ZERO_MEMORY($$); }
  | non_empty_expr_list
;

/* Functions */
func_fields :
    expr_list {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_EXPRS;
      $$->first_expr = $1.first;
      $$->next = NULL;
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
  | LPAR RESULT VALUE_TYPE RPAR func_fields {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_RESULT_TYPE;
      $$->result_type = $3;
      $$->next = $5;
    }
  | LPAR LOCAL value_type_list RPAR func_fields {
      $$ = new_func_field(parser->allocator);
      $$->type = WASM_FUNC_FIELD_TYPE_LOCAL_TYPES;
      $$->types = $3;
      $$->next = $5;
    }
  | LPAR LOCAL bind_var VALUE_TYPE RPAR func_fields {
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

        if (field->type == WASM_FUNC_FIELD_TYPE_PARAM_TYPES ||
            field->type == WASM_FUNC_FIELD_TYPE_BOUND_PARAM ||
            field->type == WASM_FUNC_FIELD_TYPE_RESULT_TYPE) {
          $$->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
        }

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

          case WASM_FUNC_FIELD_TYPE_RESULT_TYPE:
            $$->decl.sig.result_type = field->result_type;
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
      $$.func->loc = @2;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt bind_var type_use func_info RPAR {
      $$.func = $6;
      $$.func->loc = @2;
      $$.func->decl.flags |= WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
      $$.func->decl.type_var = $5;
      $$.func->name = $4;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt func_info RPAR {
      $$.func = $4;
      $$.func->loc = @2;
      $$.func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      $$.export_.name = $3;
      $$.export_.var.type = WASM_VAR_TYPE_INDEX;
      $$.export_.var.index = -1;
    }
  | LPAR FUNC export_opt bind_var func_info RPAR {
      $$.func = $5;
      $$.func->loc = @2;
      $$.func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
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


/* Modules */

start :
    LPAR START var RPAR { $$ = $3; }
;

segment_address :
    INT {
      if (WASM_FAILED(wasm_parse_int32($1.text.start,
                                       $1.text.start + $1.text.length, &$$,
                                       WASM_PARSE_UNSIGNED_ONLY))) {
        wasm_ast_parser_error(&@1, lexer, parser,
                              "invalid memory segment address \"" PRIstringslice
                              "\"",
                              WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

segment :
    LPAR SEGMENT segment_address segment_contents RPAR {
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
      wasm_append_segment_value(parser->allocator, &$$, &$2);
    }
;

initial_pages :
    INT {
      if (WASM_FAILED(wasm_parse_uint64($1.text.start,
                                        $1.text.start + $1.text.length, &$$))) {
        wasm_ast_parser_error(&@1, lexer, parser,
                              "invalid initial memory pages \"" PRIstringslice
                              "\"",
                              WASM_PRINTF_STRING_SLICE_ARG($1.text));
      }
    }
;

max_pages :
    INT {
      if (WASM_FAILED(wasm_parse_uint64($1.text.start,
                                        $1.text.start + $1.text.length, &$$))) {
        wasm_ast_parser_error(&@1, lexer, parser,
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
      $$->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
      $$->decl.sig = $5;
    }
  | LPAR IMPORT bind_var quoted_text quoted_text func_type RPAR  /* Sugar */ {
      $$ = new_import(parser->allocator);
      $$->name = $3;
      $$->module_name = $4;
      $$->func_name = $5;
      $$->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
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
  | module_fields import {
      $$ = $1;
      WasmModuleField* field = wasm_append_module_field(parser->allocator, $$);
      field->loc = @2;
      field->type = WASM_MODULE_FIELD_TYPE_IMPORT;
      field->import = *$2;
      wasm_free(parser->allocator, $2);

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
      field->table = $2;
      $$->table = &field->table;
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
      field->memory = $2;
      $$->memory = &field->memory;
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
      WasmModule* module = $$.text;

      size_t i;
      for (i = 0; i < module->funcs.size; ++i) {
        WasmFunc* func = module->funcs.data[i];
        copy_signature_from_func_type(parser->allocator, module, &func->decl);
      }

      for (i = 0; i < module->imports.size; ++i) {
        WasmImport* import = module->imports.data[i];
        copy_signature_from_func_type(parser->allocator, module, &import->decl);
      }
    }
  | LPAR MODULE text_list RPAR {
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

static void copy_signature_from_func_type(WasmAllocator* allocator,
                                          WasmModule* module,
                                          WasmFuncDeclaration* decl) {
  /* if a function or import only defines a func type (and no explicit
   * signature), copy the signature over for convenience */
  if (wasm_decl_has_func_type(decl) && !wasm_decl_has_signature(decl)) {
    int index = wasm_get_func_type_index_by_var(module, &decl->type_var);
    if (index >= 0 && (size_t)index < module->func_types.size) {
      WasmFuncType* func_type = module->func_types.data[index];
      decl->sig.result_type = func_type->sig.result_type;
      wasm_extend_types(allocator, &decl->sig.param_types,
                        &func_type->sig.param_types);
    } else {
      /* technically not OK, but we'll catch this error later in the AST
       * checker */
    }
  }
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
