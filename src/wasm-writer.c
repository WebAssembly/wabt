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

#include "wasm-writer.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR0(msg) fprintf(stderr, "%s:%d: " msg, __FILE__, __LINE__)
#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)

static WasmResult write_data_to_file(size_t offset,
                                     const void* data,
                                     size_t size,
                                     void* user_data) {
  if (size == 0)
    return WASM_OK;
  WasmFileWriter* writer = user_data;
  if (offset != writer->offset) {
    if (fseek(writer->file, offset, SEEK_SET) != 0) {
      ERROR("fseek offset=%" PRIzd " failed, errno=%d\n", size, errno);
      return WASM_ERROR;
    }
    writer->offset = offset;
  }
  if (fwrite(data, size, 1, writer->file) != 1) {
    ERROR("fwrite size=%" PRIzd " failed, errno=%d\n", size, errno);
    return WASM_ERROR;
  }
  writer->offset += size;
  return WASM_OK;
}

static WasmResult move_data_in_file(size_t dst_offset,
                                    size_t src_offset,
                                    size_t size,
                                    void* user_data) {
  if (size == 0)
    return WASM_OK;
  /* TODO(binji): implement if needed. */
  ERROR0("move_data_in_file not implemented!\n");
  return WASM_ERROR;
}

void wasm_init_file_writer_existing(WasmFileWriter* writer, FILE* file) {
  WASM_ZERO_MEMORY(*writer);
  writer->file = file;
  writer->offset = 0;
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_file;
  writer->base.move_data = move_data_in_file;
}

WasmResult wasm_init_file_writer(WasmFileWriter* writer, const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    ERROR("fopen name=\"%s\" failed, errno=%d\n", filename, errno);
    return WASM_ERROR;
  }

  wasm_init_file_writer_existing(writer, file);
  return WASM_OK;
}

void wasm_close_file_writer(WasmFileWriter* writer) {
  fclose(writer->file);
}

static WasmResult init_output_buffer(WasmAllocator* allocator,
                                     WasmOutputBuffer* buf,
                                     size_t initial_capacity) {
  buf->allocator = allocator;
  buf->start = wasm_alloc(allocator, initial_capacity, WASM_DEFAULT_ALIGN);
  if (!buf->start) {
    ERROR("allocation size=%" PRIzd " failed\n", initial_capacity);
    return WASM_ERROR;
  }
  buf->size = 0;
  buf->capacity = initial_capacity;
  return WASM_OK;
}

static WasmResult ensure_output_buffer_capacity(WasmOutputBuffer* buf,
                                                size_t ensure_capacity) {
  if (ensure_capacity > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < ensure_capacity)
      new_capacity *= 2;
    buf->start = wasm_realloc(buf->allocator, buf->start, new_capacity,
                              WASM_DEFAULT_ALIGN);
    if (!buf->start) {
      ERROR("allocation size=%" PRIzd " failed\n", new_capacity);
      return WASM_ERROR;
    }
    buf->capacity = new_capacity;
  }
  return WASM_OK;
}

static WasmResult write_data_to_output_buffer(size_t offset,
                                              const void* data,
                                              size_t size,
                                              void* user_data) {
  WasmMemoryWriter* writer = user_data;
  size_t end = offset + size;
  if (WASM_FAILED(ensure_output_buffer_capacity(&writer->buf, end)))
    return WASM_ERROR;
  memcpy((void*)((size_t)writer->buf.start + offset), data, size);
  if (end > writer->buf.size)
    writer->buf.size = end;
  return WASM_OK;
}

static WasmResult move_data_in_output_buffer(size_t dst_offset,
                                             size_t src_offset,
                                             size_t size,
                                             void* user_data) {
  WasmMemoryWriter* writer = user_data;
  size_t src_end = src_offset + size;
  size_t dst_end = dst_offset + size;
  size_t end = src_end > dst_end ? src_end : dst_end;
  if (WASM_FAILED(ensure_output_buffer_capacity(&writer->buf, end)))
    return WASM_ERROR;
  void* dst = (void*)((size_t)writer->buf.start + dst_offset);
  void* src = (void*)((size_t)writer->buf.start + src_offset);
  memmove(dst, src, size);
  if (end > writer->buf.size)
    writer->buf.size = end;
  return WASM_OK;
}

WasmResult wasm_init_mem_writer(WasmAllocator* allocator,
                                WasmMemoryWriter* writer) {
  WASM_ZERO_MEMORY(*writer);
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_output_buffer;
  writer->base.move_data = move_data_in_output_buffer;
  return init_output_buffer(allocator, &writer->buf,
                            INITIAL_OUTPUT_BUFFER_CAPACITY);
}

void wasm_steal_mem_writer_output_buffer(WasmMemoryWriter* writer,
                                         WasmOutputBuffer* out_buf) {
  *out_buf= writer->buf;
  writer->buf.start = NULL;
  writer->buf.size = 0;
  writer->buf.capacity = 0;
}

void wasm_close_mem_writer(WasmMemoryWriter* writer) {
  wasm_destroy_output_buffer(&writer->buf);
}

WasmResult wasm_write_output_buffer_to_file(WasmOutputBuffer* buf,
                                            const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    ERROR("unable to open %s for writing\n", filename);
    return WASM_ERROR;
  }

  ssize_t bytes = fwrite(buf->start, 1, buf->size, file);
  if (bytes < 0 || (size_t)bytes != buf->size) {
    ERROR("failed to write %" PRIzd " bytes to %s\n", buf->size, filename);
    return WASM_ERROR;
  }

  fclose(file);
  return WASM_OK;
}

void wasm_destroy_output_buffer(WasmOutputBuffer* buf) {
  if (buf->allocator)
    wasm_free(buf->allocator, buf->start);
}
