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

#ifndef WABT_WRITER_H_
#define WABT_WRITER_H_

#include <stdio.h>

#include "allocator.h"
#include "common.h"

typedef struct WabtWriter {
  void* user_data;
  WabtResult (*write_data)(size_t offset,
                           const void* data,
                           size_t size,
                           void* user_data);
  WabtResult (*move_data)(size_t dst_offset,
                          size_t src_offset,
                          size_t size,
                          void* user_data);
} WabtWriter;

typedef struct WabtOutputBuffer {
  WabtAllocator* allocator;
  void* start;
  size_t size;
  size_t capacity;
} WabtOutputBuffer;

typedef struct WabtMemoryWriter {
  WabtWriter base;
  WabtOutputBuffer buf;
} WabtMemoryWriter;

typedef struct WabtFileWriter {
  WabtWriter base;
  FILE* file;
  size_t offset;
} WabtFileWriter;

WABT_EXTERN_C_BEGIN

/* WabtFileWriter */
WabtResult wabt_init_file_writer(WabtFileWriter* writer, const char* filename);
void wabt_init_file_writer_existing(WabtFileWriter* writer, FILE* file);
void wabt_close_file_writer(WabtFileWriter* writer);

/* WabtMemoryWriter */
WabtResult wabt_init_mem_writer(WabtAllocator* allocator,
                                WabtMemoryWriter* writer);
/* Passes ownership of the buffer to writer */
WabtResult wabt_init_mem_writer_existing(WabtMemoryWriter* writer,
                                         WabtOutputBuffer* buf);
void wabt_steal_mem_writer_output_buffer(WabtMemoryWriter* writer,
                                         WabtOutputBuffer* out_buf);
void wabt_close_mem_writer(WabtMemoryWriter* writer);

/* WabtOutputBuffer */
void wabt_init_output_buffer(WabtAllocator* allocator,
                             WabtOutputBuffer* buf,
                             size_t initial_capacity);
WabtResult wabt_write_output_buffer_to_file(WabtOutputBuffer* buf,
                                            const char* filename);
void wabt_destroy_output_buffer(WabtOutputBuffer* buf);

WABT_EXTERN_C_END

#endif /* WABT_WRITER_H_ */
