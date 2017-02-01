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

#ifndef WABT_STREAM_H_
#define WABT_STREAM_H_

#include <stdio.h>

#include "common.h"
#include "writer.h"

typedef struct WabtStream {
  WabtWriter* writer;
  size_t offset;
  WabtResult result;
  /* if non-NULL, log all writes to this stream */
  struct WabtStream* log_stream;
} WabtStream;

typedef struct WabtFileStream {
  WabtStream base;
  WabtFileWriter writer;
} WabtFileStream;

/* whether to display the ASCII characters in the debug output for
 * wabt_write_memory */
typedef enum WabtPrintChars {
  WABT_DONT_PRINT_CHARS,
  WABT_PRINT_CHARS,
} WabtPrintChars;

WABT_EXTERN_C_BEGIN

void wabt_init_stream(WabtStream* stream,
                      WabtWriter* writer,
                      WabtStream* log_stream);
void wabt_init_file_stream_from_existing(WabtFileStream* stream, FILE* file);
WabtStream* wabt_init_stdout_stream(void);
WabtStream* wabt_init_stderr_stream(void);

/* helper functions for writing to a WabtStream. the |desc| parameter is
 * optional, and will be appended to the log stream if |stream.log_stream| is
 * non-NULL. */
void wabt_write_data_at(WabtStream*,
                        size_t offset,
                        const void* src,
                        size_t size,
                        WabtPrintChars print_chars,
                        const char* desc);
void wabt_write_data(WabtStream*,
                     const void* src,
                     size_t size,
                     const char* desc);
void wabt_move_data(WabtStream*,
                    size_t dst_offset,
                    size_t src_offset,
                    size_t size);

void WABT_PRINTF_FORMAT(2, 3) wabt_writef(WabtStream*, const char* format, ...);
/* specified as uint32_t instead of uint8_t so we can check if the value given
 * is in range before wrapping */
void wabt_write_u8(WabtStream*, uint32_t value, const char* desc);
void wabt_write_u32(WabtStream*, uint32_t value, const char* desc);
void wabt_write_u64(WabtStream*, uint64_t value, const char* desc);

static WABT_INLINE void wabt_write_char(WabtStream* stream, char c) {
  wabt_write_u8(stream, c, NULL);
}

/* dump memory as text, similar to the xxd format */
void wabt_write_memory_dump(WabtStream*,
                            const void* start,
                            size_t size,
                            size_t offset,
                            WabtPrintChars print_chars,
                            const char* prefix,
                            const char* desc);

static WABT_INLINE void wabt_write_output_buffer_memory_dump(
    WabtStream* stream,
    struct WabtOutputBuffer* buf) {
  wabt_write_memory_dump(stream, buf->start, buf->size, 0,
                         WABT_DONT_PRINT_CHARS, NULL, NULL);
}

WABT_EXTERN_C_END

#endif /* WABT_STREAM_H_ */
