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

#include "apply-names.h"

#include <assert.h>
#include <stdio.h>

#include "allocator.h"
#include "ast.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

typedef WasmLabel* LabelPtr;
WASM_DEFINE_VECTOR(label_ptr, LabelPtr);

typedef struct Context {
  WasmAllocator* allocator;
  WasmModule* module;
  WasmFunc* current_func;
  WasmExprVisitor visitor;
  /* mapping from param index to its name, if any, for the current func */
  WasmStringSliceVector param_index_to_name;
  WasmStringSliceVector local_index_to_name;
  LabelPtrVector labels;
} Context;

static void push_label(Context* ctx, WasmLabel* label) {
  wasm_append_label_ptr_value(ctx->allocator, &ctx->labels, &label);
}

static void pop_label(Context* ctx) {
  assert(ctx->labels.size > 0);
  ctx->labels.size--;
}

static WasmLabel* find_label_by_var(Context* ctx, WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME) {
    int i;
    for (i = ctx->labels.size - 1; i >= 0; --i) {
      WasmLabel* label = ctx->labels.data[i];
      if (wasm_string_slices_are_equal(label, &var->name))
        return label;
    }
    return NULL;
  } else {
    if (var->index < 0 || (size_t)var->index >= ctx->labels.size)
      return NULL;
    return ctx->labels.data[ctx->labels.size - 1 - var->index];
  }
}

static void use_name_for_var(WasmAllocator* allocator,
                             WasmStringSlice* name,
                             WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME) {
    assert(wasm_string_slices_are_equal(name, &var->name));
  }

  if (name && name->start) {
    var->type = WASM_VAR_TYPE_NAME;
    var->name = wasm_dup_string_slice(allocator, *name);
  }
}

static WasmResult use_name_for_func_type_var(WasmAllocator* allocator,
                                             WasmModule* module,
                                             WasmVar* var) {
  WasmFuncType* func_type = wasm_get_func_type_by_var(module, var);
  if (func_type == NULL)
    return WASM_ERROR;
  use_name_for_var(allocator, &func_type->name, var);
  return WASM_OK;
}

static WasmResult use_name_for_func_var(WasmAllocator* allocator,
                                        WasmModule* module,
                                        WasmVar* var) {
  WasmFunc* func = wasm_get_func_by_var(module, var);
  if (func == NULL)
    return WASM_ERROR;
  use_name_for_var(allocator, &func->name, var);
  return WASM_OK;
}

static WasmResult use_name_for_global_var(WasmAllocator* allocator,
                                          WasmModule* module,
                                          WasmVar* var) {
  WasmGlobal* global = wasm_get_global_by_var(module, var);
  if (global == NULL)
    return WASM_ERROR;
  use_name_for_var(allocator, &global->name, var);
  return WASM_OK;
}

static WasmResult use_name_for_table_var(WasmAllocator* allocator,
                                         WasmModule* module,
                                         WasmVar* var) {
  WasmTable* table = wasm_get_table_by_var(module, var);
  if (table == NULL)
    return WASM_ERROR;
  use_name_for_var(allocator, &table->name, var);
  return WASM_OK;
}

static WasmResult use_name_for_memory_var(WasmAllocator* allocator,
                                          WasmModule* module,
                                          WasmVar* var) {
  WasmMemory* memory = wasm_get_memory_by_var(module, var);
  if (memory == NULL)
    return WASM_ERROR;
  use_name_for_var(allocator, &memory->name, var);
  return WASM_OK;
}

static WasmResult use_name_for_param_and_local_var(Context* ctx,
                                                   WasmFunc* func,
                                                   WasmVar* var) {
  int local_index = wasm_get_local_index_by_var(func, var);
  if (local_index < 0 ||
      (size_t)local_index >= wasm_get_num_params_and_locals(func))
    return WASM_ERROR;

  uint32_t num_params = wasm_get_num_params(func);
  WasmStringSlice* name;
  if ((uint32_t)local_index < num_params) {
    /* param */
    assert((size_t)local_index < ctx->param_index_to_name.size);
    name = &ctx->param_index_to_name.data[local_index];
  } else {
    /* local */
    local_index -= num_params;
    assert((size_t)local_index < ctx->local_index_to_name.size);
    name = &ctx->local_index_to_name.data[local_index];
  }

  if (var->type == WASM_VAR_TYPE_NAME) {
    assert(wasm_string_slices_are_equal(name, &var->name));
    return WASM_OK;
  }

  if (name->start) {
    var->type = WASM_VAR_TYPE_NAME;
    var->name = wasm_dup_string_slice(ctx->allocator, *name);
    return var->name.start != NULL ? WASM_OK : WASM_ERROR;
  }
  return WASM_OK;
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
  WasmLabel* label = find_label_by_var(ctx, &expr->br.var);
  use_name_for_var(ctx->allocator, label, &expr->br.var);
  return WASM_OK;
}

static WasmResult on_br_if_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  WasmLabel* label = find_label_by_var(ctx, &expr->br_if.var);
  use_name_for_var(ctx->allocator, label, &expr->br_if.var);
  return WASM_OK;
}

static WasmResult on_br_table_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  size_t i;
  WasmVarVector* targets = &expr->br_table.targets;
  for (i = 0; i < targets->size; ++i) {
    WasmVar* target = &targets->data[i];
    WasmLabel* label = find_label_by_var(ctx, target);
    use_name_for_var(ctx->allocator, label, target);
  }

  WasmLabel* label = find_label_by_var(ctx, &expr->br_table.default_target);
  use_name_for_var(ctx->allocator, label, &expr->br_table.default_target);
  return WASM_OK;
}

static WasmResult on_call_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(
      use_name_for_func_var(ctx->allocator, ctx->module, &expr->call.var));
  return WASM_OK;
}

static WasmResult on_call_indirect_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(use_name_for_func_type_var(ctx->allocator, ctx->module,
                                          &expr->call_indirect.var));
  return WASM_OK;
}

static WasmResult on_get_global_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(use_name_for_global_var(ctx->allocator, ctx->module,
                                       &expr->get_global.var));
  return WASM_OK;
}

static WasmResult on_get_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->get_local.var));
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
  CHECK_RESULT(use_name_for_global_var(ctx->allocator, ctx->module,
                                       &expr->set_global.var));
  return WASM_OK;
}

static WasmResult on_set_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->set_local.var));
  return WASM_OK;
}

static WasmResult on_tee_local_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->tee_local.var));
  return WASM_OK;
}

static WasmResult visit_func(Context* ctx,
                             uint32_t func_index,
                             WasmFunc* func) {
  ctx->current_func = func;
  if (wasm_decl_has_func_type(&func->decl)) {
    CHECK_RESULT(use_name_for_func_type_var(ctx->allocator, ctx->module,
                                            &func->decl.type_var));
  }

  wasm_make_type_binding_reverse_mapping(
      ctx->allocator, &func->decl.sig.param_types, &func->param_bindings,
      &ctx->param_index_to_name);

  wasm_make_type_binding_reverse_mapping(ctx->allocator, &func->local_types,
                                         &func->local_bindings,
                                         &ctx->local_index_to_name);

  CHECK_RESULT(wasm_visit_func(func, &ctx->visitor));
  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult visit_export(Context* ctx,
                               uint32_t export_index,
                               WasmExport* export) {
  use_name_for_func_var(ctx->allocator, ctx->module, &export->var);
  return WASM_OK;
}

static WasmResult visit_elem_segment(Context* ctx,
                                     uint32_t elem_segment_index,
                                     WasmElemSegment* segment) {
  size_t i;
  CHECK_RESULT(
      use_name_for_table_var(ctx->allocator, ctx->module, &segment->table_var));
  for (i = 0; i < segment->vars.size; ++i) {
    CHECK_RESULT(use_name_for_func_var(ctx->allocator, ctx->module,
                                       &segment->vars.data[i]));
  }
  return WASM_OK;
}

static WasmResult visit_data_segment(Context* ctx,
                                     uint32_t data_segment_index,
                                     WasmDataSegment* segment) {
  CHECK_RESULT(use_name_for_memory_var(ctx->allocator, ctx->module,
                                       &segment->memory_var));
  return WASM_OK;
}

static WasmResult visit_module(Context* ctx, WasmModule* module) {
  size_t i;
  for (i = 0; i < module->funcs.size; ++i)
    CHECK_RESULT(visit_func(ctx, i, module->funcs.data[i]));
  for (i = 0; i < module->exports.size; ++i)
    CHECK_RESULT(visit_export(ctx, i, module->exports.data[i]));
  for (i = 0; i < module->elem_segments.size; ++i)
    CHECK_RESULT(visit_elem_segment(ctx, i, module->elem_segments.data[i]));
  for (i = 0; i < module->data_segments.size; ++i)
    CHECK_RESULT(visit_data_segment(ctx, i, module->data_segments.data[i]));
  return WASM_OK;
}

WasmResult wasm_apply_names(WasmAllocator* allocator, WasmModule* module) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.module = module;
  ctx.visitor.user_data = &ctx;
  ctx.visitor.begin_block_expr = begin_block_expr;
  ctx.visitor.end_block_expr = end_block_expr;
  ctx.visitor.begin_loop_expr = begin_loop_expr;
  ctx.visitor.end_loop_expr = end_loop_expr;
  ctx.visitor.on_br_expr = on_br_expr;
  ctx.visitor.on_br_if_expr = on_br_if_expr;
  ctx.visitor.on_br_table_expr = on_br_table_expr;
  ctx.visitor.on_call_expr = on_call_expr;
  ctx.visitor.on_call_indirect_expr = on_call_indirect_expr;
  ctx.visitor.on_get_global_expr = on_get_global_expr;
  ctx.visitor.on_get_local_expr = on_get_local_expr;
  ctx.visitor.begin_if_expr = begin_if_expr;
  ctx.visitor.end_if_expr = end_if_expr;
  ctx.visitor.on_set_global_expr = on_set_global_expr;
  ctx.visitor.on_set_local_expr = on_set_local_expr;
  ctx.visitor.on_tee_local_expr = on_tee_local_expr;
  WasmResult result = visit_module(&ctx, module);
  wasm_destroy_string_slice_vector(allocator, &ctx.param_index_to_name);
  wasm_destroy_string_slice_vector(allocator, &ctx.local_index_to_name);
  wasm_destroy_label_ptr_vector(allocator, &ctx.labels);
  return result;
}
