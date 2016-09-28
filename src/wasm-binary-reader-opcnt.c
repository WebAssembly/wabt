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

#include "wasm-binary-reader-ast.h"

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
  size_t *opcode_counts;
  size_t opcode_counts_size;
} Context;

static void on_error(uint32_t offset, const char* message, void* user_data) {
  Context* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult on_opcode(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  if (opcode >= ctx->opcode_counts_size)
    return WASM_ERROR;
  ++ctx->opcode_counts[opcode];
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
  .user_data = NULL,
  .on_error = on_error,
  .on_opcode = on_opcode
};

WasmResult wasm_read_binary_opcnt(struct WasmAllocator* allocator,
                                  const void* data,
                                  size_t size,
                                  const struct WasmReadBinaryOptions* options,
                                  WasmBinaryErrorHandler* error_handler,
                                  size_t* opcode_counts,
                                  size_t opcode_counts_size) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.error_handler = error_handler;
  ctx.opcode_counts = opcode_counts;
  ctx.opcode_counts_size = opcode_counts_size;
  memset((void*)opcode_counts, 0, sizeof(size_t) * opcode_counts_size);

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  return wasm_read_binary(allocator, data, size, &reader, 1, options);
}
