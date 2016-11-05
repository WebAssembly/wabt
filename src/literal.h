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

#ifndef WABT_LITERAL_H_
#define WABT_LITERAL_H_

#include <stdint.h>

#include "common.h"

/* These functions all return WABT_OK on success and WABT_ERROR on failure.
 *
 * NOTE: the functions are written for use with the re2c lexer, assuming that
 * the literal has already matched the regular expressions defined there. As a
 * result, the only validation that is done is for overflow, not for otherwise
 * bogus input. */

typedef enum WabtParseIntType {
  WABT_PARSE_UNSIGNED_ONLY = 0,
  WABT_PARSE_SIGNED_AND_UNSIGNED = 1,
} WabtParseIntType;

/* Size of char buffer required to hold hex representation of a float/double */
#define WABT_MAX_FLOAT_HEX 20
#define WABT_MAX_DOUBLE_HEX 40

WABT_EXTERN_C_BEGIN
WabtResult wabt_parse_hexdigit(char c, uint32_t* out);
WabtResult wabt_parse_int32(const char* s,
                            const char* end,
                            uint32_t* out,
                            WabtParseIntType parse_type);
WabtResult wabt_parse_int64(const char* s,
                            const char* end,
                            uint64_t* out,
                            WabtParseIntType parse_type);
WabtResult wabt_parse_uint64(const char* s, const char* end, uint64_t* out);
WabtResult wabt_parse_float(WabtLiteralType literal_type,
                            const char* s,
                            const char* end,
                            uint32_t* out_bits);
WabtResult wabt_parse_double(WabtLiteralType literal_type,
                             const char* s,
                             const char* end,
                             uint64_t* out_bits);

void wabt_write_float_hex(char* buffer, size_t size, uint32_t bits);
void wabt_write_double_hex(char* buffer, size_t size, uint64_t bits);

WABT_EXTERN_C_END

#endif /* WABT_LITERAL_H_ */
