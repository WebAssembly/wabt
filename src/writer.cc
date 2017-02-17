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

#include "writer.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR0(msg) fprintf(stderr, "%s:%d: " msg, __FILE__, __LINE__)
#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)

static WabtResult write_data_to_file(size_t offset,
                                     const void* data,
                                     size_t size,
                                     void* user_data) {
  if (size == 0)
    return WABT_OK;
  WabtFileWriter* writer = (WabtFileWriter*)user_data;
  if (offset != writer->offset) {
    if (fseek(writer->file, offset, SEEK_SET) != 0) {
      ERROR("fseek offset=%" PRIzd " failed, errno=%d\n", size, errno);
      return WABT_ERROR;
    }
    writer->offset = offset;
  }
  if (fwrite(data, size, 1, writer->file) != 1) {
    ERROR("fwrite size=%" PRIzd " failed, errno=%d\n", size, errno);
    return WABT_ERROR;
  }
  writer->offset += size;
  return WABT_OK;
}

static WabtResult move_data_in_file(size_t dst_offset,
                                    size_t src_offset,
                                    size_t size,
                                    void* user_data) {
  if (size == 0)
    return WABT_OK;
  /* TODO(binji): implement if needed. */
  ERROR0("move_data_in_file not implemented!\n");
  return WABT_ERROR;
}

void wabt_init_file_writer_existing(WabtFileWriter* writer, FILE* file) {
  WABT_ZERO_MEMORY(*writer);
  writer->file = file;
  writer->offset = 0;
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_file;
  writer->base.move_data = move_data_in_file;
}

WabtResult wabt_init_file_writer(WabtFileWriter* writer, const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    ERROR("fopen name=\"%s\" failed, errno=%d\n", filename, errno);
    return WABT_ERROR;
  }

  wabt_init_file_writer_existing(writer, file);
  return WABT_OK;
}

void wabt_close_file_writer(WabtFileWriter* writer) {
  fclose(writer->file);
}

void wabt_init_output_buffer(WabtOutputBuffer* buf, size_t initial_capacity) {
  assert(initial_capacity != 0);
  buf->start = wabt_alloc(initial_capacity);
  buf->size = 0;
  buf->capacity = initial_capacity;
}

static void ensure_output_buffer_capacity(WabtOutputBuffer* buf,
                                          size_t ensure_capacity) {
  if (ensure_capacity > buf->capacity) {
    assert(buf->capacity != 0);
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < ensure_capacity)
      new_capacity *= 2;
    buf->start = wabt_realloc(buf->start, new_capacity);
    buf->capacity = new_capacity;
  }
}

static WabtResult write_data_to_output_buffer(size_t offset,
                                              const void* data,
                                              size_t size,
                                              void* user_data) {
  WabtMemoryWriter* writer = (WabtMemoryWriter*)user_data;
  size_t end = offset + size;
  ensure_output_buffer_capacity(&writer->buf, end);
  memcpy((void*)((size_t)writer->buf.start + offset), data, size);
  if (end > writer->buf.size)
    writer->buf.size = end;
  return WABT_OK;
}

static WabtResult move_data_in_output_buffer(size_t dst_offset,
                                             size_t src_offset,
                                             size_t size,
                                             void* user_data) {
  WabtMemoryWriter* writer = (WabtMemoryWriter*)user_data;
  size_t src_end = src_offset + size;
  size_t dst_end = dst_offset + size;
  size_t end = src_end > dst_end ? src_end : dst_end;
  ensure_output_buffer_capacity(&writer->buf, end);
  void* dst = (void*)((size_t)writer->buf.start + dst_offset);
  void* src = (void*)((size_t)writer->buf.start + src_offset);
  memmove(dst, src, size);
  if (end > writer->buf.size)
    writer->buf.size = end;
  return WABT_OK;
}

WabtResult wabt_init_mem_writer(WabtMemoryWriter* writer) {
  WABT_ZERO_MEMORY(*writer);
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_output_buffer;
  writer->base.move_data = move_data_in_output_buffer;
  wabt_init_output_buffer(&writer->buf, INITIAL_OUTPUT_BUFFER_CAPACITY);
  return WABT_OK;
}

WabtResult wabt_init_mem_writer_existing(WabtMemoryWriter* writer,
                                         WabtOutputBuffer* buf) {
  WABT_ZERO_MEMORY(*writer);
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_output_buffer;
  writer->base.move_data = move_data_in_output_buffer;
  writer->buf = *buf;
  /* Clear buffer, since ownership has passed to the writer. */
  WABT_ZERO_MEMORY(*buf);
  return WABT_OK;
}

void wabt_steal_mem_writer_output_buffer(WabtMemoryWriter* writer,
                                         WabtOutputBuffer* out_buf) {
  *out_buf= writer->buf;
  writer->buf.start = NULL;
  writer->buf.size = 0;
  writer->buf.capacity = 0;
}

void wabt_close_mem_writer(WabtMemoryWriter* writer) {
  wabt_destroy_output_buffer(&writer->buf);
}

WabtResult wabt_write_output_buffer_to_file(WabtOutputBuffer* buf,
                                            const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    ERROR("unable to open %s for writing\n", filename);
    return WABT_ERROR;
  }

  ssize_t bytes = fwrite(buf->start, 1, buf->size, file);
  if (bytes < 0 || (size_t)bytes != buf->size) {
    ERROR("failed to write %" PRIzd " bytes to %s\n", buf->size, filename);
    return WABT_ERROR;
  }

  fclose(file);
  return WABT_OK;
}

void wabt_destroy_output_buffer(WabtOutputBuffer* buf) {
  wabt_free(buf->start);
}
