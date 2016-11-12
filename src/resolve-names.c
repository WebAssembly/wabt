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

#include "resolve-names.h"

#include <assert.h>
#include <stdio.h>

#include "allocator.h"
#include "ast.h"
#include "ast-parser-lexer-shared.h"

typedef WasmLabel* LabelPtr;
WASM_DEFINE_VECTOR(label_ptr, LabelPtr);

typedef struct Context {
  WasmAllocator* allocator;
  WasmSourceErrorHandler* error_handler;
  WasmAstLexer* lexer;
  WasmScript* script;
  WasmModule* current_module;
  WasmFunc* current_func;
  WasmExprVisitor visitor;
  LabelPtrVector labels;
  WasmResult result;
} Context;

static void WASM_PRINTF_FORMAT(3, 4)
    print_error(Context* ctx, const WasmLocation* loc, const char* fmt, ...) {
  ctx->result = WASM_ERROR;
  va_list args;
  va_start(args, fmt);
  wasm_ast_format_error(ctx->error_handler, loc, ctx->lexer, fmt, args);
  va_end(args);
}

static void push_label(Context* ctx, WasmLabel* label) {
  wasm_append_label_ptr_value(ctx->allocator, &ctx->labels, &label);
}

static void pop_label(Context* ctx) {
  assert(ctx->labels.size > 0);
  ctx->labels.size--;
}

static void check_duplicate_bindings(Context* ctx,
                                     const WasmBindingHash* bindings,
                                     const char* desc) {
  size_t i;
  for (i = 0; i < bindings->entries.capacity; ++i) {
    WasmBindingHashEntry* entry = &bindings->entries.data[i];
    if (wasm_hash_entry_is_free(entry))
      continue;

    /* only follow the chain if this is the first entry in the chain */
    if (entry->prev != NULL)
      continue;

    WasmBindingHashEntry* a = entry;
    for (; a; a = a->next) {
      WasmBindingHashEntry* b = a->next;
      for (; b; b = b->next) {
        if (wasm_string_slices_are_equal(&a->binding.name, &b->binding.name)) {
          /* choose the location that is later in the file */
          WasmLocation* a_loc = &a->binding.loc;
          WasmLocation* b_loc = &b->binding.loc;
          WasmLocation* loc = a_loc->line > b_loc->line ? a_loc : b_loc;
          print_error(ctx, loc, "redefinition of %s \"" PRIstringslice "\"",
                      desc, WASM_PRINTF_STRING_SLICE_ARG(a->binding.name));
        }
      }
    }
  }
}

static void resolve_label_var(Context* ctx, WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME) {
    int i;
    for (i = ctx->labels.size - 1; i >= 0; --i) {
      WasmLabel* label = ctx->labels.data[i];
      if (wasm_string_slices_are_equal(label, &var->name)) {
        wasm_destroy_string_slice(ctx->allocator, &var->name);
        var->type = WASM_VAR_TYPE_INDEX;
        var->index = ctx->labels.size - i - 1;
        return;
      }
    }
    print_error(ctx, &var->loc,
                "undefined label variable \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  }
}

static void resolve_var(Context* ctx,
                        const WasmBindingHash* bindings,
                        WasmVar* var,
                        const char* desc) {
  if (var->type == WASM_VAR_TYPE_NAME) {
    int index = wasm_get_index_from_var(bindings, var);
    if (index == -1) {
      print_error(ctx, &var->loc,
                  "undefined %s variable \"" PRIstringslice "\"", desc,
                  WASM_PRINTF_STRING_SLICE_ARG(var->name));
      return;
    }

    wasm_destroy_string_slice(ctx->allocator, &var->name);
    var->index = index;
    var->type = WASM_VAR_TYPE_INDEX;
  }
}

static void resolve_func_var(Context* ctx, WasmVar* var) {
  resolve_var(ctx, &ctx->current_module->func_bindings, var, "function");
}

static void resolve_global_var(Context* ctx, WasmVar* var) {
  resolve_var(ctx, &ctx->current_module->global_bindings, var, "global");
}

static void resolve_func_type_var(Context* ctx, WasmVar* var) {
  resolve_var(ctx, &ctx->current_module->func_type_bindings, var,
              "function type");
}

static void resolve_table_var(Context* ctx, WasmVar* var) {
  resolve_var(ctx, &ctx->current_module->table_bindings, var, "table");
}

static void resolve_memory_var(Context* ctx, WasmVar* var) {
  resolve_var(ctx, &ctx->current_module->memory_bindings, var, "memory");
}

static void resolve_local_var(Context* ctx, WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME) {
    int index = wasm_get_local_index_by_var(ctx->current_func, var);
    if (index == -1) {
      print_error(ctx, &var->loc,
                  "undefined local variable \"" PRIstringslice "\"",
                  WASM_PRINTF_STRING_SLICE_ARG(var->name));
      return;
    }

    wasm_destroy_string_slice(ctx->allocator, &var->name);
    var->index = index;
    var->type = WASM_VAR_TYPE_INDEX;
  }
}

static WasmResult begin_block_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->block.label);
  return WASM_OK;
}

static WasmResult end_block_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WASM_OK;
}

static WasmResult begin_loop_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->loop.label);
  return WASM_OK;
}

static WasmResult end_loop_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WASM_OK;
}

static WasmResult on_br_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_label_var(ctx, &expr->br.var);
  return WASM_OK;
}

static WasmResult on_br_if_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_label_var(ctx, &expr->br_if.var);
  return WASM_OK;
}

static WasmResult on_br_table_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  size_t i;
  WasmVarVector* targets = &expr->br_table.targets;
  for (i = 0; i < targets->size; ++i) {
    WasmVar* target = &targets->data[i];
    resolve_label_var(ctx, target);
  }

  resolve_label_var(ctx, &expr->br_table.default_target);
  return WASM_OK;
}

static WasmResult on_call_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_func_var(ctx, &expr->call.var);
  return WASM_OK;
}

static WasmResult on_call_indirect_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_func_type_var(ctx, &expr->call_indirect.var);
  return WASM_OK;
}

static WasmResult on_get_global_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_global_var(ctx, &expr->get_global.var);
  return WASM_OK;
}

static WasmResult on_get_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->get_local.var);
  return WASM_OK;
}

static WasmResult begin_if_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->if_.true_.label);
  return WASM_OK;
}

static WasmResult end_if_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WASM_OK;
}

static WasmResult on_set_global_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_global_var(ctx, &expr->set_global.var);
  return WASM_OK;
}

static WasmResult on_set_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->set_local.var);
  return WASM_OK;
}

static WasmResult on_tee_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->tee_local.var);
  return WASM_OK;
}

static void visit_func(Context* ctx, WasmFunc* func) {
  ctx->current_func = func;
  if (wasm_decl_has_func_type(&func->decl))
    resolve_func_type_var(ctx, &func->decl.type_var);

  check_duplicate_bindings(ctx, &func->param_bindings, "parameter");
  check_duplicate_bindings(ctx, &func->local_bindings, "local");

  wasm_visit_func(func, &ctx->visitor);
  ctx->current_func = NULL;
}

static void visit_export(Context* ctx, WasmExport* export) {
  switch (export->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      resolve_func_var(ctx, &export->var);
      break;

    case WASM_EXTERNAL_KIND_TABLE:
      resolve_table_var(ctx, &export->var);
      break;

    case WASM_EXTERNAL_KIND_MEMORY:
      resolve_memory_var(ctx, &export->var);
      break;

    case WASM_EXTERNAL_KIND_GLOBAL:
      resolve_global_var(ctx, &export->var);
      break;

    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void visit_global(Context* ctx, WasmGlobal* global) {
  wasm_visit_expr_list(global->init_expr, &ctx->visitor);
}

static void visit_elem_segment(Context* ctx, WasmElemSegment* segment) {
  size_t i;
  resolve_table_var(ctx, &segment->table_var);
  wasm_visit_expr_list(segment->offset, &ctx->visitor);
  for (i = 0; i < segment->vars.size; ++i)
    resolve_func_var(ctx, &segment->vars.data[i]);
}

static void visit_data_segment(Context* ctx, WasmDataSegment* segment) {
  resolve_memory_var(ctx, &segment->memory_var);
  wasm_visit_expr_list(segment->offset, &ctx->visitor);
}

static void visit_module(Context* ctx, WasmModule* module) {
  ctx->current_module = module;
  check_duplicate_bindings(ctx, &module->func_bindings, "function");
  check_duplicate_bindings(ctx, &module->global_bindings, "global");
  check_duplicate_bindings(ctx, &module->export_bindings, "export");
  check_duplicate_bindings(ctx, &module->func_type_bindings, "function type");
  check_duplicate_bindings(ctx, &module->table_bindings, "table");
  check_duplicate_bindings(ctx, &module->memory_bindings, "memory");

  size_t i;
  for (i = 0; i < module->funcs.size; ++i)
    visit_func(ctx, module->funcs.data[i]);
  for (i = 0; i < module->exports.size; ++i)
    visit_export(ctx, module->exports.data[i]);
  for (i = 0; i < module->globals.size; ++i)
    visit_global(ctx, module->globals.data[i]);
  for (i = 0; i < module->elem_segments.size; ++i)
    visit_elem_segment(ctx, module->elem_segments.data[i]);
  for (i = 0; i < module->data_segments.size; ++i)
    visit_data_segment(ctx, module->data_segments.data[i]);
  if (module->start)
    resolve_func_var(ctx, module->start);
  ctx->current_module = NULL;
}

static void visit_raw_module(Context* ctx, WasmRawModule* raw_module) {
  if (raw_module->type == WASM_RAW_MODULE_TYPE_TEXT)
    visit_module(ctx, raw_module->text);
}

static void visit_command(Context* ctx, WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      visit_module(ctx, &command->module);
      break;

    case WASM_COMMAND_TYPE_ACTION:
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
    case WASM_COMMAND_TYPE_REGISTER:
      /* Don't resolve a module_var, since it doesn't really behave like other
       * vars. You can't reference a module by index. */
      break;

    case WASM_COMMAND_TYPE_ASSERT_MALFORMED:
      /* Malformed modules should not be text; the whole point of this
       * assertion is to test for malformed binary modules. */
      break;

    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      /* The module may be invalid because the names cannot be resolved (or are
       * duplicates). In either case, don't resolve. */
      break;

    case WASM_COMMAND_TYPE_ASSERT_UNLINKABLE:
      visit_raw_module(ctx, &command->assert_unlinkable.module);
      break;

    case WASM_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      visit_raw_module(ctx, &command->assert_uninstantiable.module);
      break;

    case WASM_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

static void visit_script(Context* ctx, WasmScript* script) {
  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    visit_command(ctx, &script->commands.data[i]);
}

static void init_context(Context* ctx,
                         WasmAllocator* allocator,
                         WasmAstLexer* lexer,
                         WasmScript* script,
                         WasmSourceErrorHandler* error_handler) {
  WASM_ZERO_MEMORY(*ctx);
  ctx->allocator = allocator;
  ctx->lexer = lexer;
  ctx->error_handler = error_handler;
  ctx->result = WASM_OK;
  ctx->script = script;
  ctx->visitor.user_data = ctx;
  ctx->visitor.begin_block_expr = begin_block_expr;
  ctx->visitor.end_block_expr = end_block_expr;
  ctx->visitor.begin_loop_expr = begin_loop_expr;
  ctx->visitor.end_loop_expr = end_loop_expr;
  ctx->visitor.on_br_expr = on_br_expr;
  ctx->visitor.on_br_if_expr = on_br_if_expr;
  ctx->visitor.on_br_table_expr = on_br_table_expr;
  ctx->visitor.on_call_expr = on_call_expr;
  ctx->visitor.on_call_indirect_expr = on_call_indirect_expr;
  ctx->visitor.on_get_global_expr = on_get_global_expr;
  ctx->visitor.on_get_local_expr = on_get_local_expr;
  ctx->visitor.begin_if_expr = begin_if_expr;
  ctx->visitor.end_if_expr = end_if_expr;
  ctx->visitor.on_set_global_expr = on_set_global_expr;
  ctx->visitor.on_set_local_expr = on_set_local_expr;
  ctx->visitor.on_tee_local_expr = on_tee_local_expr;
}

WasmResult wasm_resolve_names_module(WasmAllocator* allocator,
                                     WasmAstLexer* lexer,
                                     WasmModule* module,
                                     WasmSourceErrorHandler* error_handler) {
  Context ctx;
  init_context(&ctx, allocator, lexer, NULL, error_handler);
  visit_module(&ctx, module);
  wasm_destroy_label_ptr_vector(allocator, &ctx.labels);
  return ctx.result;
}

WasmResult wasm_resolve_names_script(WasmAllocator* allocator,
                                     WasmAstLexer* lexer,
                                     WasmScript* script,
                                     WasmSourceErrorHandler* error_handler) {
  Context ctx;
  init_context(&ctx, allocator, lexer, script, error_handler);
  visit_script(&ctx, script);
  wasm_destroy_label_ptr_vector(allocator, &ctx.labels);
  return ctx.result;
}
