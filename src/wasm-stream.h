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

#ifndef WASM_STREAM_H_
#define WASM_STREAM_H_

#include <stdio.h>

#include "wasm-common.h"
#include "wasm-writer.h"

typedef struct WasmStream {
  WasmWriter* writer;
  size_t offset;
  WasmResult result;
  /* if non-NULL, log all writes to this stream */
  struct WasmStream* log_stream;
} WasmStream;

typedef struct WasmFileStream {
  WasmStream base;
  WasmFileWriter writer;
} WasmFileStream;

/* whether to display the ASCII characters in the debug output for
 * wasm_write_memory */
typedef enum WasmPrintChars {
  WASM_DONT_PRINT_CHARS,
  WASM_PRINT_CHARS,
} WasmPrintChars;

WASM_EXTERN_C_BEGIN

void wasm_init_stream(WasmStream* stream,
                      WasmWriter* writer,
                      WasmStream* log_stream);
void wasm_init_file_stream_from_existing(WasmFileStream* stream, FILE* file);
WasmStream* wasm_init_stdout_stream(void);
WasmStream* wasm_init_stderr_stream(void);

/* helper functions for writing to a WasmStream. the |desc| parameter is
 * optional, and will be appended to the log stream if |stream.log_stream| is
 * non-NULL. */
void wasm_write_data_at(WasmStream*,
                        size_t offset,
                        const void* src,
                        size_t size,
                        WasmPrintChars print_chars,
                        const char* desc);
void wasm_write_data(WasmStream*,
                     const void* src,
                     size_t size,
                     const char* desc);
void wasm_move_data(WasmStream*,
                    size_t dst_offset,
                    size_t src_offset,
                    size_t size);

void WASM_PRINTF_FORMAT(2, 3) wasm_writef(WasmStream*, const char* format, ...);
/* specified as uint32_t instead of uint8_t so we can check if the value given
 * is in range before wrapping */
void wasm_write_u8(WasmStream*, uint32_t value, const char* desc);
void wasm_write_u32(WasmStream*, uint32_t value, const char* desc);
void wasm_write_u64(WasmStream*, uint64_t value, const char* desc);

static WASM_INLINE void wasm_write_char(WasmStream* stream, char c) {
  wasm_write_u8(stream, c, NULL);
}

/* dump memory as text, similar to the xxd format */
void wasm_write_memory_dump(WasmStream*,
                            const void* start,
                            size_t size,
                            size_t offset,
                            WasmPrintChars print_chars,
                            const char* desc);

static WASM_INLINE void wasm_write_output_buffer_memory_dump(
    WasmStream* stream,
    struct WasmOutputBuffer* buf) {
  wasm_write_memory_dump(stream, buf->start, buf->size, 0,
                         WASM_DONT_PRINT_CHARS, NULL);
}

WASM_EXTERN_C_END

#endif /* WASM_STREAM_H_ */
