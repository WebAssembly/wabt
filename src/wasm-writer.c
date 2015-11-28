#include "wasm-writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "wasm-internal.h"

#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)

static void write_data_to_file(size_t offset,
                               const void* data,
                               size_t size,
                               void* user_data) {
  WasmFileWriter* writer = user_data;
  if (offset != writer->offset) {
    fseek(writer->file, offset, SEEK_SET);
    writer->offset = offset;
  }
  fwrite(data, size, 1, writer->file);
  writer->offset += size;
}

WasmResult wasm_init_file_writer(WasmFileWriter* writer, const char* filename) {
  memset(writer, 0, sizeof(*writer));
  writer->file = fopen(filename, "wb");
  if (!writer->file)
    return WASM_ERROR;

  writer->offset = 0;
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_file;
  return WASM_OK;
}

void wasm_close_file_writer(WasmFileWriter* writer) {
  fclose(writer->file);
}

static void init_output_buffer(WasmOutputBuffer* buf, size_t initial_capacity) {
  buf->start = malloc(initial_capacity);
  if (!buf->start)
    FATAL("unable to allocate %zd bytes\n", initial_capacity);
  buf->size = 0;
  buf->capacity = initial_capacity;
}

static void ensure_output_buffer_capacity(WasmOutputBuffer* buf,
                                          size_t ensure_capacity) {
  if (ensure_capacity > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < ensure_capacity)
      new_capacity *= 2;
    buf->start = realloc(buf->start, new_capacity);
    if (!buf->start)
      FATAL("unable to allocate %zd bytes\n", new_capacity);
    buf->capacity = new_capacity;
  }
}

static void free_output_buffer(WasmOutputBuffer* buf) {
  free(buf->start);
}

static void write_data_to_output_buffer(size_t offset,
                                        const void* data,
                                        size_t size,
                                        void* user_data) {
  WasmMemoryWriter* writer = user_data;
  size_t end = offset + size;
  ensure_output_buffer_capacity(&writer->buf, end);
  memcpy(writer->buf.start + offset, data, size);
  if (end > writer->buf.size)
    writer->buf.size = end;
}

WasmResult wasm_init_mem_writer(WasmMemoryWriter* writer) {
  memset(writer, 0, sizeof(*writer));
  writer->base.user_data = writer;
  writer->base.write_data = write_data_to_output_buffer;
}

void wasm_close_mem_writer(WasmMemoryWriter* writer) {
  free(writer->buf.start);
}
