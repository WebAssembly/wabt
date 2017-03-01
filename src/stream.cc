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

static WabtFileStream s_stdout_stream;
static WabtFileStream s_stderr_stream;

void wabt_init_stream(WabtStream* stream,
                      WabtWriter* writer,
                      WabtStream* log_stream) {
  stream->writer = writer;
  stream->offset = 0;
  stream->result = WabtResult::Ok;
  stream->log_stream = log_stream;
}

void wabt_init_file_stream_from_existing(WabtFileStream* stream, FILE* file) {
  wabt_init_file_writer_existing(&stream->writer, file);
  wabt_init_stream(&stream->base, &stream->writer.base, nullptr);
}

WabtStream* wabt_init_stdout_stream(void) {
  wabt_init_file_stream_from_existing(&s_stdout_stream, stdout);
  return &s_stdout_stream.base;
}

WabtStream* wabt_init_stderr_stream(void) {
  wabt_init_file_stream_from_existing(&s_stderr_stream, stderr);
  return &s_stderr_stream.base;
}

void wabt_write_data_at(WabtStream* stream,
                        size_t offset,
                        const void* src,
                        size_t size,
                        WabtPrintChars print_chars,
                        const char* desc) {
  if (WABT_FAILED(stream->result))
    return;
  if (stream->log_stream) {
    wabt_write_memory_dump(stream->log_stream, src, size, offset, print_chars,
                           nullptr, desc);
  }
  if (stream->writer->write_data) {
    stream->result = stream->writer->write_data(offset, src, size,
                                                stream->writer->user_data);
  }
}

void wabt_write_data(WabtStream* stream,
                     const void* src,
                     size_t size,
                     const char* desc) {
  wabt_write_data_at(stream, stream->offset, src, size, WabtPrintChars::No,
                     desc);
  stream->offset += size;
}

void wabt_move_data(WabtStream* stream,
                    size_t dst_offset,
                    size_t src_offset,
                    size_t size) {
  if (WABT_FAILED(stream->result))
    return;
  if (stream->log_stream) {
    wabt_writef(stream->log_stream, "; move data: [%" PRIzx ", %" PRIzx
                                    ") -> [%" PRIzx ", %" PRIzx ")\n",
                src_offset, src_offset + size, dst_offset, dst_offset + size);
  }
  if (stream->writer->write_data) {
    stream->result = stream->writer->move_data(dst_offset, src_offset, size,
                                               stream->writer->user_data);
  }
}

void wabt_writef(WabtStream* stream, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  wabt_write_data(stream, buffer, length, nullptr);
}

void wabt_write_u8(WabtStream* stream, uint32_t value, const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  wabt_write_data_at(stream, stream->offset, &value8, sizeof(value8),
                     WabtPrintChars::No, desc);
  stream->offset += sizeof(value8);
}

void wabt_write_u32(WabtStream* stream, uint32_t value, const char* desc) {
  wabt_write_data_at(stream, stream->offset, &value, sizeof(value),
                     WabtPrintChars::No, desc);
  stream->offset += sizeof(value);
}

void wabt_write_u64(WabtStream* stream, uint64_t value, const char* desc) {
  wabt_write_data_at(stream, stream->offset, &value, sizeof(value),
                     WabtPrintChars::No, desc);
  stream->offset += sizeof(value);
}

void wabt_write_memory_dump(WabtStream* stream,
                            const void* start,
                            size_t size,
                            size_t offset,
                            WabtPrintChars print_chars,
                            const char* prefix,
                            const char* desc) {
  const uint8_t* p = static_cast<const uint8_t*>(start);
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    if (prefix)
      wabt_writef(stream, "%s", prefix);
    wabt_writef(stream, "%07" PRIzx ": ",
                reinterpret_cast<intptr_t>(p) -
                    reinterpret_cast<intptr_t>(start) + offset);
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          wabt_writef(stream, "%02x", *p);
        } else {
          wabt_write_char(stream, ' ');
          wabt_write_char(stream, ' ');
        }
      }
      wabt_write_char(stream, ' ');
    }

    if (print_chars == WabtPrintChars::Yes) {
      wabt_write_char(stream, ' ');
      p = line;
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
        wabt_write_char(stream, isprint(*p) ? *p : '.');
    }

    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      wabt_writef(stream, "  ; %s", desc);
    wabt_write_char(stream, '\n');
  }
}
