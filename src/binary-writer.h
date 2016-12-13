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

#ifndef WASM_BINARY_WRITER_H_
#define WASM_BINARY_WRITER_H_

#include "common.h"

struct WasmAllocator;
struct WasmModule;
struct WasmScript;
struct WasmWriter;
struct WasmStream;
enum WasmPrintChars;

#define WASM_WRITE_BINARY_OPTIONS_DEFAULT \
  { NULL, WASM_TRUE, WASM_FALSE, WASM_FALSE, WASM_FALSE }

typedef struct WasmWriteBinaryOptions {
  struct WasmStream* log_stream;
  WasmBool canonicalize_lebs;
  WasmBool linkable;
  WasmBool write_debug_names;
  WasmBool is_invalid;
} WasmWriteBinaryOptions;

WASM_EXTERN_C_BEGIN
WasmResult wasm_write_binary_module(struct WasmAllocator*,
                                    struct WasmWriter*,
                                    const struct WasmModule*,
                                    const WasmWriteBinaryOptions*);

WasmResult wasm_write_binary_script(struct WasmAllocator*,
                                    struct WasmWriter*,
                                    const struct WasmScript*,
                                    const WasmWriteBinaryOptions*);

/* returns the length of the leb128 */
uint32_t wasm_u32_leb128_length(uint32_t value);

void wasm_write_u32_leb128(struct WasmStream* stream,
                           uint32_t value,
                           const char* desc);

void wasm_write_i32_leb128(struct WasmStream* stream,
                           int32_t value,
                           const char* desc);

void wasm_write_fixed_u32_leb128(struct WasmStream* stream,
                                 uint32_t value,
                                 const char* desc);

uint32_t wasm_write_fixed_u32_leb128_at(struct WasmStream* stream,
                                        uint32_t offset,
                                        uint32_t value,
                                        const char* desc);

uint32_t wasm_write_fixed_u32_leb128_raw(uint8_t* data,
                                         uint8_t* end,
                                         uint32_t value);

void wasm_write_type(struct WasmStream* stream, WasmType type);

void wasm_write_str(struct WasmStream* stream,
                    const char* s,
                    size_t length,
                    enum WasmPrintChars print_chars,
                    const char* desc);

void wasm_write_opcode(struct WasmStream* stream, uint8_t opcode);

void wasm_write_limits(struct WasmStream* stream, const WasmLimits* limits);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_WRITER_H_ */
