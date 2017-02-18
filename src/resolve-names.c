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

#include "ast.h"
#include "ast-parser-lexer-shared.h"

typedef WabtLabel* LabelPtr;
WABT_DEFINE_VECTOR(label_ptr, LabelPtr);

typedef struct Context {
  WabtSourceErrorHandler* error_handler;
  WabtAstLexer* lexer;
  WabtScript* script;
  WabtModule* current_module;
  WabtFunc* current_func;
  WabtExprVisitor visitor;
  LabelPtrVector labels;
  WabtResult result;
} Context;

static void WABT_PRINTF_FORMAT(3, 4)
    print_error(Context* ctx, const WabtLocation* loc, const char* fmt, ...) {
  ctx->result = WABT_ERROR;
  va_list args;
  va_start(args, fmt);
  wabt_ast_format_error(ctx->error_handler, loc, ctx->lexer, fmt, args);
  va_end(args);
}

static void push_label(Context* ctx, WabtLabel* label) {
  wabt_append_label_ptr_value(&ctx->labels, &label);
}

static void pop_label(Context* ctx) {
  assert(ctx->labels.size > 0);
  ctx->labels.size--;
}

typedef struct FindDuplicateBindingContext {
  Context* ctx;
  const char* desc;
} FindDuplicateBindingContext;

static void on_duplicate_binding(WabtBindingHashEntry* a,
                                 WabtBindingHashEntry* b,
                                 void* user_data) {
  FindDuplicateBindingContext* fdbc = user_data;
  /* choose the location that is later in the file */
  WabtLocation* a_loc = &a->binding.loc;
  WabtLocation* b_loc = &b->binding.loc;
  WabtLocation* loc = a_loc->line > b_loc->line ? a_loc : b_loc;
  print_error(fdbc->ctx, loc, "redefinition of %s \"" PRIstringslice "\"",
              fdbc->desc, WABT_PRINTF_STRING_SLICE_ARG(a->binding.name));
}

static void check_duplicate_bindings(Context* ctx,
                                     const WabtBindingHash* bindings,
                                     const char* desc) {
  FindDuplicateBindingContext fdbc;
  fdbc.ctx = ctx;
  fdbc.desc = desc;
  wabt_find_duplicate_bindings(bindings, on_duplicate_binding, &fdbc);
}

static void resolve_label_var(Context* ctx, WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME) {
    int i;
    for (i = ctx->labels.size - 1; i >= 0; --i) {
      WabtLabel* label = ctx->labels.data[i];
      if (wabt_string_slices_are_equal(label, &var->name)) {
        wabt_destroy_string_slice(&var->name);
        var->type = WABT_VAR_TYPE_INDEX;
        var->index = ctx->labels.size - i - 1;
        return;
      }
    }
    print_error(ctx, &var->loc,
                "undefined label variable \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(var->name));
  }
}

static void resolve_var(Context* ctx,
                        const WabtBindingHash* bindings,
                        WabtVar* var,
                        const char* desc) {
  if (var->type == WABT_VAR_TYPE_NAME) {
    int index = wabt_get_index_from_var(bindings, var);
    if (index == -1) {
      print_error(ctx, &var->loc,
                  "undefined %s variable \"" PRIstringslice "\"", desc,
                  WABT_PRINTF_STRING_SLICE_ARG(var->name));
      return;
    }

    wabt_destroy_string_slice(&var->name);
    var->index = index;
    var->type = WABT_VAR_TYPE_INDEX;
  }
}

static void resolve_func_var(Context* ctx, WabtVar* var) {
  resolve_var(ctx, &ctx->current_module->func_bindings, var, "function");
}

static void resolve_global_var(Context* ctx, WabtVar* var) {
  resolve_var(ctx, &ctx->current_module->global_bindings, var, "global");
}

static void resolve_func_type_var(Context* ctx, WabtVar* var) {
  resolve_var(ctx, &ctx->current_module->func_type_bindings, var,
              "function type");
}

static void resolve_table_var(Context* ctx, WabtVar* var) {
  resolve_var(ctx, &ctx->current_module->table_bindings, var, "table");
}

static void resolve_memory_var(Context* ctx, WabtVar* var) {
  resolve_var(ctx, &ctx->current_module->memory_bindings, var, "memory");
}

static void resolve_local_var(Context* ctx, WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME) {
    int index = wabt_get_local_index_by_var(ctx->current_func, var);
    if (index == -1) {
      print_error(ctx, &var->loc,
                  "undefined local variable \"" PRIstringslice "\"",
                  WABT_PRINTF_STRING_SLICE_ARG(var->name));
      return;
    }

    wabt_destroy_string_slice(&var->name);
    var->index = index;
    var->type = WABT_VAR_TYPE_INDEX;
  }
}

static WabtResult begin_block_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->block.label);
  return WABT_OK;
}

static WabtResult end_block_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WABT_OK;
}

static WabtResult begin_loop_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->loop.label);
  return WABT_OK;
}

static WabtResult end_loop_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WABT_OK;
}

static WabtResult on_br_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_label_var(ctx, &expr->br.var);
  return WABT_OK;
}

static WabtResult on_br_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_label_var(ctx, &expr->br_if.var);
  return WABT_OK;
}

static WabtResult on_br_table_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  size_t i;
  WabtVarVector* targets = &expr->br_table.targets;
  for (i = 0; i < targets->size; ++i) {
    WabtVar* target = &targets->data[i];
    resolve_label_var(ctx, target);
  }

  resolve_label_var(ctx, &expr->br_table.default_target);
  return WABT_OK;
}

static WabtResult on_call_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_func_var(ctx, &expr->call.var);
  return WABT_OK;
}

static WabtResult on_call_indirect_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_func_type_var(ctx, &expr->call_indirect.var);
  return WABT_OK;
}

static WabtResult on_get_global_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_global_var(ctx, &expr->get_global.var);
  return WABT_OK;
}

static WabtResult on_get_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->get_local.var);
  return WABT_OK;
}

static WabtResult begin_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  push_label(ctx, &expr->if_.true_.label);
  return WABT_OK;
}

static WabtResult end_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  pop_label(ctx);
  return WABT_OK;
}

static WabtResult on_set_global_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_global_var(ctx, &expr->set_global.var);
  return WABT_OK;
}

static WabtResult on_set_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->set_local.var);
  return WABT_OK;
}

static WabtResult on_tee_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  resolve_local_var(ctx, &expr->tee_local.var);
  return WABT_OK;
}

static void visit_func(Context* ctx, WabtFunc* func) {
  ctx->current_func = func;
  if (wabt_decl_has_func_type(&func->decl))
    resolve_func_type_var(ctx, &func->decl.type_var);

  check_duplicate_bindings(ctx, &func->param_bindings, "parameter");
  check_duplicate_bindings(ctx, &func->local_bindings, "local");

  wabt_visit_func(func, &ctx->visitor);
  ctx->current_func = NULL;
}

static void visit_export(Context* ctx, WabtExport* export) {
  switch (export->kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      resolve_func_var(ctx, &export->var);
      break;

    case WABT_EXTERNAL_KIND_TABLE:
      resolve_table_var(ctx, &export->var);
      break;

    case WABT_EXTERNAL_KIND_MEMORY:
      resolve_memory_var(ctx, &export->var);
      break;

    case WABT_EXTERNAL_KIND_GLOBAL:
      resolve_global_var(ctx, &export->var);
      break;

    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void visit_global(Context* ctx, WabtGlobal* global) {
  wabt_visit_expr_list(global->init_expr, &ctx->visitor);
}

static void visit_elem_segment(Context* ctx, WabtElemSegment* segment) {
  size_t i;
  resolve_table_var(ctx, &segment->table_var);
  wabt_visit_expr_list(segment->offset, &ctx->visitor);
  for (i = 0; i < segment->vars.size; ++i)
    resolve_func_var(ctx, &segment->vars.data[i]);
}

static void visit_data_segment(Context* ctx, WabtDataSegment* segment) {
  resolve_memory_var(ctx, &segment->memory_var);
  wabt_visit_expr_list(segment->offset, &ctx->visitor);
}

static void visit_module(Context* ctx, WabtModule* module) {
  ctx->current_module = module;
  check_duplicate_bindings(ctx, &module->func_bindings, "function");
  check_duplicate_bindings(ctx, &module->global_bindings, "global");
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

static void visit_raw_module(Context* ctx, WabtRawModule* raw_module) {
  if (raw_module->type == WABT_RAW_MODULE_TYPE_TEXT)
    visit_module(ctx, raw_module->text);
}

static void dummy_source_error_callback(const WabtLocation* loc,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data) {}

static void visit_command(Context* ctx, WabtCommand* command) {
  switch (command->type) {
    case WABT_COMMAND_TYPE_MODULE:
      visit_module(ctx, &command->module);
      break;

    case WABT_COMMAND_TYPE_ACTION:
    case WABT_COMMAND_TYPE_ASSERT_RETURN:
    case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN:
    case WABT_COMMAND_TYPE_ASSERT_TRAP:
    case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
    case WABT_COMMAND_TYPE_REGISTER:
      /* Don't resolve a module_var, since it doesn't really behave like other
       * vars. You can't reference a module by index. */
      break;

    case WABT_COMMAND_TYPE_ASSERT_MALFORMED:
      /* Malformed modules should not be text; the whole point of this
       * assertion is to test for malformed binary modules. */
      break;

    case WABT_COMMAND_TYPE_ASSERT_INVALID: {
      /* The module may be invalid because the names cannot be resolved; we
       * don't want to print errors or fail if that's the case, but we still
       * should try to resolve names when possible. */
      WabtSourceErrorHandler new_error_handler;
      new_error_handler.on_error = dummy_source_error_callback;
      new_error_handler.source_line_max_length =
          ctx->error_handler->source_line_max_length;

      Context new_ctx;
      WABT_ZERO_MEMORY(new_ctx);
      new_ctx.error_handler = &new_error_handler;
      new_ctx.lexer = ctx->lexer;
      new_ctx.visitor = ctx->visitor;
      new_ctx.visitor.user_data = &new_ctx;
      new_ctx.result = WABT_OK;

      visit_raw_module(&new_ctx, &command->assert_invalid.module);
      wabt_destroy_label_ptr_vector(&new_ctx.labels);
      if (WABT_FAILED(new_ctx.result)) {
        command->type = WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY;
      }
      break;
    }

    case WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY:
      /* The only reason a module would be "non-binary" is if the names cannot
       * be resolved. So we assume name resolution has already been tried and
       * failed, so skip it. */
      break;

    case WABT_COMMAND_TYPE_ASSERT_UNLINKABLE:
      visit_raw_module(ctx, &command->assert_unlinkable.module);
      break;

    case WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      visit_raw_module(ctx, &command->assert_uninstantiable.module);
      break;

    case WABT_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

static void visit_script(Context* ctx, WabtScript* script) {
  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    visit_command(ctx, &script->commands.data[i]);
}

static void init_context(Context* ctx,
                         WabtAstLexer* lexer,
                         WabtScript* script,
                         WabtSourceErrorHandler* error_handler) {
  WABT_ZERO_MEMORY(*ctx);
  ctx->lexer = lexer;
  ctx->error_handler = error_handler;
  ctx->result = WABT_OK;
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

WabtResult wabt_resolve_names_module(WabtAstLexer* lexer,
                                     WabtModule* module,
                                     WabtSourceErrorHandler* error_handler) {
  Context ctx;
  init_context(&ctx, lexer, NULL, error_handler);
  visit_module(&ctx, module);
  wabt_destroy_label_ptr_vector(&ctx.labels);
  return ctx.result;
}

WabtResult wabt_resolve_names_script(WabtAstLexer* lexer,
                                     WabtScript* script,
                                     WabtSourceErrorHandler* error_handler) {
  Context ctx;
  init_context(&ctx, lexer, script, error_handler);
  visit_script(&ctx, script);
  wabt_destroy_label_ptr_vector(&ctx.labels);
  return ctx.result;
}
