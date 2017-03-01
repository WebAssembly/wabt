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

#include "ast.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WabtResult::Error;   \
  } while (0)

typedef WabtLabel* LabelPtr;
WABT_DEFINE_VECTOR(label_ptr, LabelPtr);

struct Context {
  WabtModule* module;
  WabtFunc* current_func;
  WabtExprVisitor visitor;
  /* mapping from param index to its name, if any, for the current func */
  WabtStringSliceVector param_index_to_name;
  WabtStringSliceVector local_index_to_name;
  LabelPtrVector labels;
};

static void push_label(Context* ctx, WabtLabel* label) {
  wabt_append_label_ptr_value(&ctx->labels, &label);
}

static void pop_label(Context* ctx) {
  assert(ctx->labels.size > 0);
  ctx->labels.size--;
}

static WabtLabel* find_label_by_var(Context* ctx, WabtVar* var) {
  if (var->type == WabtVarType::Name) {
    int i;
    for (i = ctx->labels.size - 1; i >= 0; --i) {
      WabtLabel* label = ctx->labels.data[i];
      if (wabt_string_slices_are_equal(label, &var->name))
        return label;
    }
    return nullptr;
  } else {
    if (var->index < 0 || static_cast<size_t>(var->index) >= ctx->labels.size)
      return nullptr;
    return ctx->labels.data[ctx->labels.size - 1 - var->index];
  }
}

static void use_name_for_var(WabtStringSlice* name, WabtVar* var) {
  if (var->type == WabtVarType::Name) {
    assert(wabt_string_slices_are_equal(name, &var->name));
  }

  if (name && name->start) {
    var->type = WabtVarType::Name;
    var->name = wabt_dup_string_slice(*name);
  }
}

static WabtResult use_name_for_func_type_var(WabtModule* module, WabtVar* var) {
  WabtFuncType* func_type = wabt_get_func_type_by_var(module, var);
  if (!func_type)
    return WabtResult::Error;
  use_name_for_var(&func_type->name, var);
  return WabtResult::Ok;
}

static WabtResult use_name_for_func_var(WabtModule* module, WabtVar* var) {
  WabtFunc* func = wabt_get_func_by_var(module, var);
  if (!func)
    return WabtResult::Error;
  use_name_for_var(&func->name, var);
  return WabtResult::Ok;
}

static WabtResult use_name_for_global_var(WabtModule* module, WabtVar* var) {
  WabtGlobal* global = wabt_get_global_by_var(module, var);
  if (!global)
    return WabtResult::Error;
  use_name_for_var(&global->name, var);
  return WabtResult::Ok;
}

static WabtResult use_name_for_table_var(WabtModule* module, WabtVar* var) {
  WabtTable* table = wabt_get_table_by_var(module, var);
  if (!table)
    return WabtResult::Error;
  use_name_for_var(&table->name, var);
  return WabtResult::Ok;
}

static WabtResult use_name_for_memory_var(WabtModule* module, WabtVar* var) {
  WabtMemory* memory = wabt_get_memory_by_var(module, var);
  if (!memory)
    return WabtResult::Error;
  use_name_for_var(&memory->name, var);
  return WabtResult::Ok;
}

static WabtResult use_name_for_param_and_local_var(Context* ctx,
                                                   WabtFunc* func,
                                                   WabtVar* var) {
  int local_index = wabt_get_local_index_by_var(func, var);
  if (local_index < 0 ||
      static_cast<size_t>(local_index) >= wabt_get_num_params_and_locals(func))
    return WabtResult::Error;

  uint32_t num_params = wabt_get_num_params(func);
  WabtStringSlice* name;
  if (static_cast<uint32_t>(local_index) < num_params) {
    /* param */
    assert(static_cast<size_t>(local_index) < ctx->param_index_to_name.size);
    name = &ctx->param_index_to_name.data[local_index];
  } else {
    /* local */
    local_index -= num_params;
    assert(static_cast<size_t>(local_index) < ctx->local_index_to_name.size);
    name = &ctx->local_index_to_name.data[local_index];
  }

  if (var->type == WabtVarType::Name) {
    assert(wabt_string_slices_are_equal(name, &var->name));
    return WabtResult::Ok;
  }

  if (name->start) {
    var->type = WabtVarType::Name;
    var->name = wabt_dup_string_slice(*name);
    return var->name.start ? WabtResult::Ok : WabtResult::Error;
  }
  return WabtResult::Ok;
}

static WabtResult begin_block_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  push_label(ctx, &expr->block.label);
  return WabtResult::Ok;
}

static WabtResult end_block_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  pop_label(ctx);
  return WabtResult::Ok;
}

static WabtResult begin_loop_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  push_label(ctx, &expr->loop.label);
  return WabtResult::Ok;
}

static WabtResult end_loop_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  pop_label(ctx);
  return WabtResult::Ok;
}

static WabtResult on_br_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  WabtLabel* label = find_label_by_var(ctx, &expr->br.var);
  use_name_for_var(label, &expr->br.var);
  return WabtResult::Ok;
}

static WabtResult on_br_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  WabtLabel* label = find_label_by_var(ctx, &expr->br_if.var);
  use_name_for_var(label, &expr->br_if.var);
  return WabtResult::Ok;
}

static WabtResult on_br_table_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  size_t i;
  WabtVarVector* targets = &expr->br_table.targets;
  for (i = 0; i < targets->size; ++i) {
    WabtVar* target = &targets->data[i];
    WabtLabel* label = find_label_by_var(ctx, target);
    use_name_for_var(label, target);
  }

  WabtLabel* label = find_label_by_var(ctx, &expr->br_table.default_target);
  use_name_for_var(label, &expr->br_table.default_target);
  return WabtResult::Ok;
}

static WabtResult on_call_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_func_var(ctx->module, &expr->call.var));
  return WabtResult::Ok;
}

static WabtResult on_call_indirect_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(
      use_name_for_func_type_var(ctx->module, &expr->call_indirect.var));
  return WabtResult::Ok;
}

static WabtResult on_get_global_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_global_var(ctx->module, &expr->get_global.var));
  return WabtResult::Ok;
}

static WabtResult on_get_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->get_local.var));
  return WabtResult::Ok;
}

static WabtResult begin_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  push_label(ctx, &expr->if_.true_.label);
  return WabtResult::Ok;
}

static WabtResult end_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  pop_label(ctx);
  return WabtResult::Ok;
}

static WabtResult on_set_global_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_global_var(ctx->module, &expr->set_global.var));
  return WabtResult::Ok;
}

static WabtResult on_set_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->set_local.var));
  return WabtResult::Ok;
}

static WabtResult on_tee_local_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = static_cast<Context*>(user_data);
  CHECK_RESULT(use_name_for_param_and_local_var(ctx, ctx->current_func,
                                                &expr->tee_local.var));
  return WabtResult::Ok;
}

static WabtResult visit_func(Context* ctx,
                             uint32_t func_index,
                             WabtFunc* func) {
  ctx->current_func = func;
  if (wabt_decl_has_func_type(&func->decl)) {
    CHECK_RESULT(use_name_for_func_type_var(ctx->module, &func->decl.type_var));
  }

  wabt_make_type_binding_reverse_mapping(&func->decl.sig.param_types,
                                         &func->param_bindings,
                                         &ctx->param_index_to_name);

  wabt_make_type_binding_reverse_mapping(
      &func->local_types, &func->local_bindings, &ctx->local_index_to_name);

  CHECK_RESULT(wabt_visit_func(func, &ctx->visitor));
  ctx->current_func = nullptr;
  return WabtResult::Ok;
}

static WabtResult visit_export(Context* ctx,
                               uint32_t export_index,
                               WabtExport* export_) {
  if (export_->kind == WabtExternalKind::Func) {
    use_name_for_func_var(ctx->module, &export_->var);
  }
  return WabtResult::Ok;
}

static WabtResult visit_elem_segment(Context* ctx,
                                     uint32_t elem_segment_index,
                                     WabtElemSegment* segment) {
  size_t i;
  CHECK_RESULT(use_name_for_table_var(ctx->module, &segment->table_var));
  for (i = 0; i < segment->vars.size; ++i) {
    CHECK_RESULT(use_name_for_func_var(ctx->module, &segment->vars.data[i]));
  }
  return WabtResult::Ok;
}

static WabtResult visit_data_segment(Context* ctx,
                                     uint32_t data_segment_index,
                                     WabtDataSegment* segment) {
  CHECK_RESULT(use_name_for_memory_var(ctx->module, &segment->memory_var));
  return WabtResult::Ok;
}

static WabtResult visit_module(Context* ctx, WabtModule* module) {
  size_t i;
  for (i = 0; i < module->funcs.size; ++i)
    CHECK_RESULT(visit_func(ctx, i, module->funcs.data[i]));
  for (i = 0; i < module->exports.size; ++i)
    CHECK_RESULT(visit_export(ctx, i, module->exports.data[i]));
  for (i = 0; i < module->elem_segments.size; ++i)
    CHECK_RESULT(visit_elem_segment(ctx, i, module->elem_segments.data[i]));
  for (i = 0; i < module->data_segments.size; ++i)
    CHECK_RESULT(visit_data_segment(ctx, i, module->data_segments.data[i]));
  return WabtResult::Ok;
}

WabtResult wabt_apply_names(WabtModule* module) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
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
  WabtResult result = visit_module(&ctx, module);
  wabt_destroy_string_slice_vector(&ctx.param_index_to_name);
  wabt_destroy_string_slice_vector(&ctx.local_index_to_name);
  wabt_destroy_label_ptr_vector(&ctx.labels);
  return result;
}
