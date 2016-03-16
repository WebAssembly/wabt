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

#include <stdint.h>

#include "wasm-common.h"

/* These functions all return 1 on success and 0 on failure.
 *
 * NOTE: the functions are written for use with the flex lexer, assuming that
 * the literal has already matched the regular expressions defined there. As a
 * result, the only validation that is done is for overflow, not for otherwise
 * bogus input. */

EXTERN_C_BEGIN
int wasm_parse_hexdigit(char c, uint32_t* out);
int wasm_parse_int32(const char* s,
                     const char* end,
                     uint32_t* out,
                     int allow_signed);
int wasm_parse_int64(const char* s, const char* end, uint64_t* out);
int wasm_parse_uint64(const char* s, const char* end, uint64_t* out);
int wasm_parse_float(WasmLiteralType literal_type,
                     const char* s,
                     const char* end,
                     uint32_t* out_bits);
int wasm_parse_double(WasmLiteralType literal_type,
                      const char* s,
                      const char* end,
                      uint64_t* out_bits);
EXTERN_C_END
