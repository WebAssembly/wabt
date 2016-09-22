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

#include "wasm-generate-names.h"

#include <assert.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

typedef struct Context {
  WasmAllocator* allocator;
  WasmModule* module;
  WasmExprVisitor visitor;
  WasmStringSliceVector index_to_name;
  uint32_t label_count;
} Context;

static WasmBool has_name(WasmStringSlice* str) {
  return str->length > 0;
}

static void generate_name(WasmAllocator* allocator,
                          const char* prefix,
                          uint32_t index,
                          WasmStringSlice* str) {
  size_t prefix_len = strlen(prefix);
  size_t buffer_len = prefix_len + 20; /* add space for the number */
  char* buffer = alloca(buffer_len);
  int actual_len = wasm_snprintf(buffer, buffer_len, "%s%u", prefix, index);

  WasmStringSlice buf;
  buf.length = actual_len;
  buf.start = buffer;
  *str = wasm_dup_string_slice(allocator, buf);
}

static void maybe_generate_name(WasmAllocator* allocator,
                                const char* prefix,
                                uint32_t index,
                                WasmStringSlice* str) {
  if (!has_name(str))
    generate_name(allocator, prefix, index, str);
}

static void generate_and_bind_name(WasmAllocator* allocator,
                                   WasmBindingHash* bindings,
                                   const char* prefix,
                                   uint32_t index,
                                   WasmStringSlice* str) {
  generate_name(allocator, prefix, index, str);
  WasmBinding* binding;
  binding = wasm_insert_binding(allocator, bindings, str);
  binding->index = index;
}

static void maybe_generate_and_bind_name(WasmAllocator* allocator,
                                         WasmBindingHash* bindings,
                                         const char* prefix,
                                         uint32_t index,
                                         WasmStringSlice* str) {
  if (!has_name(str))
    generate_and_bind_name(allocator, bindings, prefix, index, str);
}

static void generate_and_bind_local_names(WasmAllocator* allocator,
                                          WasmStringSliceVector* index_to_name,
                                          WasmBindingHash* bindings,
                                          const char* prefix) {
  size_t i;
  for (i = 0; i < index_to_name->size; ++i) {
    WasmStringSlice* old_name = &index_to_name->data[i];
    if (has_name(old_name))
      continue;

    WasmStringSlice new_name;
    generate_and_bind_name(allocator, bindings, prefix, i, &new_name);
    index_to_name->data[i] = new_name;
  }
}

static WasmResult begin_block_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name(ctx->allocator, "$B", ctx->label_count++,
                      &expr->block.label);
  return WASM_OK;
}

static WasmResult begin_loop_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name(ctx->allocator, "$L", ctx->label_count++,
                      &expr->loop.label);
  return WASM_OK;
}

static WasmResult begin_if_expr(WasmExpr* expr, void* user_data) {
  Context* ctx = user_data;
  maybe_generate_name(ctx->allocator, "$L", ctx->label_count++,
                      &expr->if_.true_.label);
  return WASM_OK;
}


static WasmResult visit_func(Context* ctx,
                             uint32_t func_index,
                             WasmFunc* func) {
  maybe_generate_and_bind_name(ctx->allocator, &ctx->module->func_bindings,
                               "$f", func_index, &func->name);

  wasm_make_type_binding_reverse_mapping(
      ctx->allocator, &func->decl.sig.param_types, &func->param_bindings,
      &ctx->index_to_name);
  generate_and_bind_local_names(ctx->allocator, &ctx->index_to_name,
                                &func->param_bindings, "$p");

  wasm_make_type_binding_reverse_mapping(ctx->allocator, &func->local_types,
                                         &func->local_bindings,
                                         &ctx->index_to_name);
  generate_and_bind_local_names(ctx->allocator, &ctx->index_to_name,
                                &func->local_bindings, "$l");

  ctx->label_count = 0;
  CHECK_RESULT(wasm_visit_func(func, &ctx->visitor));
  return WASM_OK;
}

static WasmResult visit_global(Context* ctx,
                               uint32_t global_index,
                               WasmGlobal* global) {
  maybe_generate_and_bind_name(ctx->allocator, &ctx->module->global_bindings,
                               "$g", global_index, &global->name);
  return WASM_OK;
}

static WasmResult visit_func_type(Context* ctx,
                                  uint32_t func_type_index,
                                  WasmFuncType* func_type) {
  maybe_generate_and_bind_name(ctx->allocator, &ctx->module->func_type_bindings,
                               "$t", func_type_index, &func_type->name);
  return WASM_OK;
}

static WasmResult visit_table(Context* ctx,
                              uint32_t table_index,
                              WasmTable* table) {
  maybe_generate_and_bind_name(ctx->allocator, &ctx->module->table_bindings,
                               "$T", table_index, &table->name);
  return WASM_OK;
}

static WasmResult visit_memory(Context* ctx,
                               uint32_t memory_index,
                               WasmMemory* memory) {
  maybe_generate_and_bind_name(ctx->allocator, &ctx->module->memory_bindings,
                               "$M", memory_index, &memory->name);
  return WASM_OK;
}

static WasmResult visit_module(Context* ctx, WasmModule* module) {
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
  return WASM_OK;
}

WasmResult wasm_generate_names(WasmAllocator* allocator, WasmModule* module) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.visitor.user_data = &ctx;
  ctx.visitor.begin_block_expr = begin_block_expr;
  ctx.visitor.begin_loop_expr = begin_loop_expr;
  ctx.visitor.begin_if_expr = begin_if_expr;
  ctx.module = module;
  WasmResult result = visit_module(&ctx, module);
  wasm_destroy_string_slice_vector(allocator, &ctx.index_to_name);
  return result;
}
