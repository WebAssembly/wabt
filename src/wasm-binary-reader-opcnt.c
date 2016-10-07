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

#include "wasm-binary-reader-opcnt.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-binary-reader.h"
#include "wasm-common.h"

typedef struct Context {
  WasmAllocator* allocator;
  WasmBinaryErrorHandler* error_handler;
  WasmOpcntData* opcnt_data;
} Context;

static void on_error(uint32_t offset, const char* message, void* user_data) {
  Context* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult add_int_counter_value(struct WasmAllocator* allocator,
                                        WasmIntCounterVector* vec,
                                        intmax_t value) {
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].value == value) {
      ++vec->data[i].count;
      return WASM_OK;
    }
  }
  WasmIntCounter counter;
  counter.value = value;
  counter.count = 1;
  wasm_append_int_counter_value(allocator, vec, &counter);
  return WASM_OK;
}

static WasmResult add_int_pair_counter_value(struct WasmAllocator* allocator,
                                             WasmIntPairCounterVector* vec,
                                             intmax_t first, intmax_t second) {
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].first == first && vec->data[i].second == second) {
      ++vec->data[i].count;
      return WASM_OK;
    }
  }
  WasmIntPairCounter counter;
  counter.first = first;
  counter.second = second;
  counter.count = 1;
  wasm_append_int_pair_counter_value(allocator, vec, &counter);
  return WASM_OK;
}

static WasmResult on_opcode(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmIntCounterVector* opcnt_vec = &ctx->opcnt_data->opcode_vec;
  while (opcode >= opcnt_vec->size) {
    WasmIntCounter Counter;
    Counter.value = opcnt_vec->size;
    Counter.count = 0;
    wasm_append_int_counter_value(ctx->opcnt_data->allocator,
                                  opcnt_vec, &Counter);
  }
  ++opcnt_vec->data[opcode].count;
  return WASM_OK;
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  return add_int_counter_value(ctx->opcnt_data->allocator,
                               &ctx->opcnt_data->i32_const_vec, (int32_t)value);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  return add_int_counter_value(ctx->opcnt_data->allocator,
                               &ctx->opcnt_data->get_local_vec, local_index);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  return add_int_counter_value(ctx->opcnt_data->allocator,
                               &ctx->opcnt_data->set_local_vec, local_index);
}

static  WasmResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  return add_int_counter_value(ctx->opcnt_data->allocator,
                               &ctx->opcnt_data->tee_local_vec, local_index);
}

static  WasmResult on_load_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  if (opcode == WASM_OPCODE_I32_LOAD)
    return add_int_pair_counter_value(ctx->opcnt_data->allocator,
                                      &ctx->opcnt_data->i32_load_vec,
                                      alignment_log2, offset);
  return WASM_OK;
}

static  WasmResult on_store_expr(WasmOpcode opcode,
                                 uint32_t alignment_log2,
                                 uint32_t offset,
                                 void* user_data) {
  Context* ctx = user_data;
  if (opcode == WASM_OPCODE_I32_STORE)
    return add_int_pair_counter_value(ctx->opcnt_data->allocator,
                                      &ctx->opcnt_data->i32_store_vec,
                                      alignment_log2, offset);
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
  .user_data = NULL,
  .on_error = on_error,
  .on_opcode = on_opcode,
  .on_i32_const_expr = on_i32_const_expr,
  .on_get_local_expr = on_get_local_expr,
  .on_set_local_expr = on_set_local_expr,
  .on_tee_local_expr = on_tee_local_expr,
  .on_load_expr = on_load_expr,
  .on_store_expr = on_store_expr
};

void wasm_init_opcnt_data(struct WasmAllocator* allocator,
                          WasmOpcntData* data) {
  WASM_ZERO_MEMORY(*data);
  data->allocator = allocator;
}

void wasm_destroy_opcnt_data(struct WasmAllocator* allocator,
                             WasmOpcntData* data) {
  assert(allocator == data->allocator);
  wasm_destroy_int_counter_vector(allocator, &data->opcode_vec);
  wasm_destroy_int_counter_vector(allocator, &data->i32_const_vec);
  wasm_destroy_int_counter_vector(allocator, &data->get_local_vec);
  wasm_destroy_int_pair_counter_vector(allocator, &data->i32_load_vec);
}

WasmResult wasm_read_binary_opcnt(struct WasmAllocator* allocator,
                                  const void* data,
                                  size_t size,
                                  const struct WasmReadBinaryOptions* options,
                                  WasmBinaryErrorHandler* error_handler,
                                  WasmOpcntData* opcnt_data) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.error_handler = error_handler;
  ctx.opcnt_data = opcnt_data;

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  return wasm_read_binary(allocator, data, size, &reader, 1, options);
}
