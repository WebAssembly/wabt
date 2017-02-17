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

#ifndef WABT_BINARY_WRITER_H_
#define WABT_BINARY_WRITER_H_

#include "common.h"
#include "stream.h"

struct WabtModule;
struct WabtScript;
struct WabtWriter;
struct WabtStream;

#define WABT_WRITE_BINARY_OPTIONS_DEFAULT \
  { NULL, true, false, false }

typedef struct WabtWriteBinaryOptions {
  struct WabtStream* log_stream;
  bool canonicalize_lebs;
  bool relocatable;
  bool write_debug_names;
} WabtWriteBinaryOptions;

WABT_EXTERN_C_BEGIN
WabtResult wabt_write_binary_module(struct WabtWriter*,
                                    const struct WabtModule*,
                                    const WabtWriteBinaryOptions*);

/* returns the length of the leb128 */
uint32_t wabt_u32_leb128_length(uint32_t value);

void wabt_write_u32_leb128(struct WabtStream* stream,
                           uint32_t value,
                           const char* desc);

void wabt_write_i32_leb128(struct WabtStream* stream,
                           int32_t value,
                           const char* desc);

void wabt_write_fixed_u32_leb128(struct WabtStream* stream,
                                 uint32_t value,
                                 const char* desc);

uint32_t wabt_write_fixed_u32_leb128_at(struct WabtStream* stream,
                                        uint32_t offset,
                                        uint32_t value,
                                        const char* desc);

uint32_t wabt_write_fixed_u32_leb128_raw(uint8_t* data,
                                         uint8_t* end,
                                         uint32_t value);

void wabt_write_type(struct WabtStream* stream, WabtType type);

void wabt_write_str(struct WabtStream* stream,
                    const char* s,
                    size_t length,
                    WabtPrintChars print_chars,
                    const char* desc);

void wabt_write_opcode(struct WabtStream* stream, WabtOpcode opcode);

void wabt_write_limits(struct WabtStream* stream, const WabtLimits* limits);
WABT_EXTERN_C_END

#endif /* WABT_BINARY_WRITER_H_ */
