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

#ifndef WASM_WRITER_H_
#define WASM_WRITER_H_

#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-common.h"

typedef struct WasmWriter {
  void* user_data;
  WasmResult (*write_data)(size_t offset,
                           const void* data,
                           size_t size,
                           void* user_data);
  WasmResult (*move_data)(size_t dst_offset,
                          size_t src_offset,
                          size_t size,
                          void* user_data);
} WasmWriter;

typedef struct WasmOutputBuffer {
  WasmAllocator* allocator;
  void* start;
  size_t size;
  size_t capacity;
} WasmOutputBuffer;

typedef struct WasmMemoryWriter {
  WasmWriter base;
  WasmOutputBuffer buf;
} WasmMemoryWriter;

typedef struct WasmFileWriter {
  WasmWriter base;
  FILE* file;
  size_t offset;
} WasmFileWriter;

WASM_EXTERN_C_BEGIN

/* WasmFileWriter */
WasmResult wasm_init_file_writer(WasmFileWriter* writer, const char* filename);
WasmResult wasm_init_file_writer_existing(WasmFileWriter* writer, FILE* file);
void wasm_close_file_writer(WasmFileWriter* writer);

/* WasmMemoryWriter */
WasmResult wasm_init_mem_writer(WasmAllocator* allocator,
                                WasmMemoryWriter* writer);
void wasm_steal_mem_writer_output_buffer(WasmMemoryWriter* writer,
                                         WasmOutputBuffer* out_buf);
void wasm_close_mem_writer(WasmMemoryWriter* writer);

/* WasmOutputBuffer */
WasmResult wasm_write_output_buffer_to_file(WasmOutputBuffer* buf,
                                            const char* filename);
void wasm_destroy_output_buffer(WasmOutputBuffer* buf);

WASM_EXTERN_C_END

#endif /* WASM_WRITER_H_ */
