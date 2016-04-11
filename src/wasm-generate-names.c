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

#define CHECK_ALLOC_(cond)                                             \
  do {                                                                 \
    if (!(cond)) {                                                     \
      fprintf(stderr, "%s:%d: allocation failed", __FILE__, __LINE__); \
      return WASM_ERROR;                                               \
    }                                                                  \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v) != NULL)
#define CHECK_ALLOC_NULL_STR(v) CHECK_ALLOC_((v).start)

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

typedef uint32_t WasmUint32;
WASM_DEFINE_VECTOR(uint32, WasmUint32);

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmModule* module;
  WasmStringSliceVector index_to_name;
} WasmContext;

static WasmBool has_name(WasmStringSlice* str) {
  return str->length > 0;
}

static WasmResult generate_name(WasmAllocator* allocator,
                                WasmBindingHash* bindings,
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
  CHECK_ALLOC_NULL_STR(*str = wasm_dup_string_slice(allocator, buf));
  WasmBinding* binding;
  CHECK_ALLOC_NULL(binding = wasm_insert_binding(allocator, bindings, str));
  binding->index = index;
  return WASM_OK;
}

static WasmResult maybe_generate_name(WasmAllocator* allocator,
                                      WasmBindingHash* bindings,
                                      const char* prefix,
                                      uint32_t index,
                                      WasmStringSlice* str) {
  if (has_name(str))
    return WASM_OK;
  return generate_name(allocator, bindings, prefix, index, str);
}

static WasmResult generate_local_names(WasmAllocator* allocator,
                                       WasmStringSliceVector* index_to_name,
                                       WasmBindingHash* bindings,
                                       const char* prefix) {
  size_t i;
  for (i = 0; i < index_to_name->size; ++i) {
    WasmStringSlice* old_name = &index_to_name->data[i];
    if (has_name(old_name))
      continue;

    WasmStringSlice new_name;
    CHECK_RESULT(generate_name(allocator, bindings, prefix, i, &new_name));
    index_to_name->data[i] = new_name;
  }
  return WASM_OK;
}

static WasmResult visit_func(WasmContext* ctx,
                             uint32_t func_index,
                             WasmFunc* func) {
  CHECK_ALLOC(maybe_generate_name(ctx->allocator, &ctx->module->func_bindings,
                                  "$f", func_index, &func->name));

  CHECK_ALLOC(wasm_make_type_binding_reverse_mapping(
      ctx->allocator, &func->decl.sig.param_types, &func->param_bindings,
      &ctx->index_to_name));
  CHECK_RESULT(generate_local_names(ctx->allocator, &ctx->index_to_name,
                                    &func->param_bindings, "$p"));

  CHECK_ALLOC(wasm_make_type_binding_reverse_mapping(
      ctx->allocator, &func->local_types, &func->local_bindings,
      &ctx->index_to_name));
  CHECK_RESULT(generate_local_names(ctx->allocator, &ctx->index_to_name,
                                    &func->local_bindings, "$l"));

  return WASM_OK;
}

static WasmResult visit_func_type(WasmContext* ctx,
                                  uint32_t func_type_index,
                                  WasmFuncType* func_type) {
  CHECK_ALLOC(maybe_generate_name(ctx->allocator,
                                  &ctx->module->func_type_bindings, "$t",
                                  func_type_index, &func_type->name));
  return WASM_OK;
}

static WasmResult visit_import(WasmContext* ctx,
                               uint32_t import_index,
                               WasmImport* import) {
  CHECK_ALLOC(maybe_generate_name(ctx->allocator, &ctx->module->import_bindings,
                                  "$i", import_index, &import->name));
  return WASM_OK;
}

static WasmResult visit_module(WasmContext* ctx, WasmModule* module) {
  size_t i;
  for (i = 0; i < module->func_types.size; ++i)
    CHECK_RESULT(visit_func_type(ctx, i, module->func_types.data[i]));
  for (i = 0; i < module->imports.size; ++i)
    CHECK_RESULT(visit_import(ctx, i, module->imports.data[i]));
  for (i = 0; i < module->funcs.size; ++i)
    CHECK_RESULT(visit_func(ctx, i, module->funcs.data[i]));
  return WASM_OK;
}

WasmResult wasm_generate_names(WasmAllocator* allocator, WasmModule* module) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.module = module;
  WasmResult result = visit_module(&ctx, module);
  wasm_destroy_string_slice_vector(allocator, &ctx.index_to_name);
  return result;
}
