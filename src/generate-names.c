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

#include "generate-names.h"

#include <assert.h>
#include <stdio.h>

#include "ast.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WABT_ERROR;   \
  } while (0)

typedef struct Context {
  WabtModule* module;
  WabtExprVisitor visitor;
  WabtStringSliceVector index_to_name;
  uint32_t label_count;
} Context;

static WabtBool has_name(WabtStringSlice* str) {
  return str->length > 0;
}

static void generate_name(const char* prefix,
                          uint32_t index,
                          WabtStringSlice* str) {
  size_t prefix_len = strlen(prefix);
  size_t buffer_len = prefix_len + 20; /* add space for the number */
  char* buffer = alloca(buffer_len);
  int actual_len = wabt_snprintf(buffer, buffer_len, "%s%u", prefix, index);

  WabtStringSlice buf;
  buf.length = actual_len;
  buf.start = buffer;
  *str = wabt_dup_string_slice(buf);
}

static void maybe_generate_name(const char* prefix,
                                uint32_t index,
                                WabtStringSlice* str) {
  if (!has_name(str))
    generate_name(prefix, index, str);
}

static void generate_and_bind_name(WabtBindingHash* bindings,
                                   const char* prefix,
                                   uint32_t index,
                                   WabtStringSlice* str) {
  generate_name(prefix, index, str);
  WabtBinding* binding;
  binding = wabt_insert_binding(bindings, str);
  binding->index = index;
}

static void maybe_generate_and_bind_name(WabtBindingHash* bindings,
                                         const char* prefix,
                                         uint32_t index,
                                         WabtStringSlice* str) {
  if (!has_name(str))
    generate_and_bind_name(bindings, prefix, index, str);
}

static void generate_and_bind_local_names(WabtStringSliceVector* index_to_name,
                                          WabtBindingHash* bindings,
                                          const char* prefix) {
  size_t i;
  for (i = 0; i < index_to_name->size; ++i) {
    WabtStringSlice* old_name = &index_to_name->data[i];
    if (has_name(old_name))
      continue;

    WabtStringSlice new_name;
    generate_and_bind_name(bindings, prefix, i, &new_name);
    index_to_name->data[i] = new_name;
  }
}

static WabtResult begin_block_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name("$B", ctx->label_count++, &expr->block.label);
  return WABT_OK;
}

static WabtResult begin_loop_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name("$L", ctx->label_count++, &expr->loop.label);
  return WABT_OK;
}

static WabtResult begin_if_expr(WabtExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name("$L", ctx->label_count++, &expr->if_.true_.label);
  return WABT_OK;
}


static WabtResult visit_func(Context* ctx,
                             uint32_t func_index,
                             WabtFunc* func) {
  maybe_generate_and_bind_name(&ctx->module->func_bindings, "$f", func_index,
                               &func->name);

  wabt_make_type_binding_reverse_mapping(
      &func->decl.sig.param_types, &func->param_bindings, &ctx->index_to_name);
  generate_and_bind_local_names(&ctx->index_to_name, &func->param_bindings,
                                "$p");

  wabt_make_type_binding_reverse_mapping(
      &func->local_types, &func->local_bindings, &ctx->index_to_name);
  generate_and_bind_local_names(&ctx->index_to_name, &func->local_bindings,
                                "$l");

  ctx->label_count = 0;
  CHECK_RESULT(wabt_visit_func(func, &ctx->visitor));
  return WABT_OK;
}

static WabtResult visit_global(Context* ctx,
                               uint32_t global_index,
                               WabtGlobal* global) {
  maybe_generate_and_bind_name(&ctx->module->global_bindings, "$g",
                               global_index, &global->name);
  return WABT_OK;
}

static WabtResult visit_func_type(Context* ctx,
                                  uint32_t func_type_index,
                                  WabtFuncType* func_type) {
  maybe_generate_and_bind_name(&ctx->module->func_type_bindings, "$t",
                               func_type_index, &func_type->name);
  return WABT_OK;
}

static WabtResult visit_table(Context* ctx,
                              uint32_t table_index,
                              WabtTable* table) {
  maybe_generate_and_bind_name(&ctx->module->table_bindings, "$T", table_index,
                               &table->name);
  return WABT_OK;
}

static WabtResult visit_memory(Context* ctx,
                               uint32_t memory_index,
                               WabtMemory* memory) {
  maybe_generate_and_bind_name(&ctx->module->memory_bindings, "$M",
                               memory_index, &memory->name);
  return WABT_OK;
}

static WabtResult visit_module(Context* ctx, WabtModule* module) {
  size_t i;
  for (i = 0; i < module->globals.size; ++i)
    CHECK_RESULT(visit_global(ctx, i, module->globals.data[i]));
  for (i = 0; i < module->func_types.size; ++i)
    CHECK_RESULT(visit_func_type(ctx, i, module->func_types.data[i]));
  for (i = 0; i < module->funcs.size; ++i)
    CHECK_RESULT(visit_func(ctx, i, module->funcs.data[i]));
  for (i = 0; i < module->tables.size; ++i)
    CHECK_RESULT(visit_table(ctx, i, module->tables.data[i]));
  for (i = 0; i < module->memories.size; ++i)
    CHECK_RESULT(visit_memory(ctx, i, module->memories.data[i]));
  return WABT_OK;
}

WabtResult wabt_generate_names(WabtModule* module) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.visitor.user_data = &ctx;
  ctx.visitor.begin_block_expr = begin_block_expr;
  ctx.visitor.begin_loop_expr = begin_loop_expr;
  ctx.visitor.begin_if_expr = begin_if_expr;
  ctx.module = module;
  WabtResult result = visit_module(&ctx, module);
  wabt_destroy_string_slice_vector(&ctx.index_to_name);
  return result;
}
