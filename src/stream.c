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

#include "stream.h"

#include <assert.h>
#include <ctype.h>

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

static WasmFileStream s_stdout_stream;
static WasmFileStream s_stderr_stream;

void wasm_init_stream(WasmStream* stream,
                      WasmWriter* writer,
                      WasmStream* log_stream) {
  stream->writer = writer;
  stream->offset = 0;
  stream->result = WASM_OK;
  stream->log_stream = log_stream;
}

void wasm_init_file_stream_from_existing(WasmFileStream* stream, FILE* file) {
  wasm_init_file_writer_existing(&stream->writer, file);
  wasm_init_stream(&stream->base, &stream->writer.base, NULL);
}

WasmStream* wasm_init_stdout_stream(void) {
  wasm_init_file_stream_from_existing(&s_stdout_stream, stdout);
  return &s_stdout_stream.base;
}

WasmStream* wasm_init_stderr_stream(void) {
  wasm_init_file_stream_from_existing(&s_stderr_stream, stderr);
  return &s_stderr_stream.base;
}

void wasm_write_data_at(WasmStream* stream,
                        size_t offset,
                        const void* src,
                        size_t size,
                        WasmPrintChars print_chars,
                        const char* desc) {
  if (WASM_FAILED(stream->result))
    return;
  if (stream->log_stream) {
    wasm_write_memory_dump(stream->log_stream, src, size, offset, print_chars,
                           NULL, desc);
  }
  if (stream->writer->write_data) {
    stream->result = stream->writer->write_data(offset, src, size,
                                                stream->writer->user_data);
  }
}

void wasm_write_data(WasmStream* stream,
                     const void* src,
                     size_t size,
                     const char* desc) {
  wasm_write_data_at(stream, stream->offset, src, size, WASM_DONT_PRINT_CHARS,
                     desc);
  stream->offset += size;
}
void wasm_move_data(WasmStream* stream,
                    size_t dst_offset,
                    size_t src_offset,
                    size_t size) {
  if (WASM_FAILED(stream->result))
    return;
  if (stream->log_stream) {
    wasm_writef(stream->log_stream, "; move data: [%" PRIzx ", %" PRIzx
                                    ") -> [%" PRIzx ", %" PRIzx ")\n",
                src_offset, src_offset + size, dst_offset, dst_offset + size);
  }
  if (stream->writer->write_data) {
    stream->result = stream->writer->move_data(dst_offset, src_offset, size,
                                               stream->writer->user_data);
  }
}

void wasm_writef(WasmStream* stream, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  wasm_write_data(stream, buffer, length, NULL);
}

void wasm_write_u8(WasmStream* stream, uint32_t value, const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  wasm_write_data_at(stream, stream->offset, &value8, sizeof(value8),
                     WASM_DONT_PRINT_CHARS, desc);
  stream->offset += sizeof(value8);
}

void wasm_write_u32(WasmStream* stream, uint32_t value, const char* desc) {
  wasm_write_data_at(stream, stream->offset, &value, sizeof(value),
                     WASM_DONT_PRINT_CHARS, desc);
  stream->offset += sizeof(value);
}

void wasm_write_u64(WasmStream* stream, uint64_t value, const char* desc) {
  wasm_write_data_at(stream, stream->offset, &value, sizeof(value),
                     WASM_DONT_PRINT_CHARS, desc);
  stream->offset += sizeof(value);
}

void wasm_write_memory_dump(WasmStream* stream,
                            const void* start,
                            size_t size,
                            size_t offset,
                            WasmPrintChars print_chars,
                            const char* prefix,
                            const char* desc) {
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    if (prefix)
      wasm_writef(stream, "%s", prefix);
    wasm_writef(stream, "%07" PRIzx ": ", (size_t)p - (size_t)start + offset);
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          wasm_writef(stream, "%02x", *p);
        } else {
          wasm_write_char(stream, ' ');
          wasm_write_char(stream, ' ');
        }
      }
      wasm_write_char(stream, ' ');
    }

    if (print_chars == WASM_PRINT_CHARS) {
      wasm_write_char(stream, ' ');
      p = line;
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
        wasm_write_char(stream, isprint(*p) ? *p : '.');
    }

    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      wasm_writef(stream, "  ; %s", desc);
    wasm_write_char(stream, '\n');
  }
}
