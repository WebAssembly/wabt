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

#ifndef WASM_COMMON_H_
#define WASM_COMMON_H_

#include <stddef.h>

#include "wasm-config.h"

#ifdef __cplusplus
#define WASM_EXTERN_C extern "C"
#define WASM_EXTERN_C_BEGIN extern "C" {
#define WASM_EXTERN_C_END }
#else
#define WASM_EXTERN_C
#define WASM_EXTERN_C_BEGIN
#define WASM_EXTERN_C_END
#endif

#define WASM_FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define WASM_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define WASM_ZERO_MEMORY(var) memset((void*)&(var), 0, sizeof(var))

#define WASM_PAGE_SIZE 0x10000 /* 64k */

struct WasmAllocator;

typedef enum WasmResult {
  WASM_OK,
  WASM_ERROR,
} WasmResult;

typedef struct WasmStringSlice {
  const char* start;
  size_t length;
} WasmStringSlice;

typedef struct WasmLocation {
  const char* filename;
  int line;
  int first_column;
  int last_column;
} WasmLocation;

/* matches binary format, do not change */
typedef enum WasmType {
  WASM_TYPE_VOID,
  WASM_TYPE_I32,
  WASM_TYPE_I64,
  WASM_TYPE_F32,
  WASM_TYPE_F64,
  WASM_NUM_TYPES,
  WASM_TYPE____ = WASM_TYPE_VOID, /* convenient for the opcode table below */
} WasmType;

enum { WASM_USE_NATURAL_ALIGNMENT = 0xFFFFFFFF };

#define WASM_FOREACH_OPCODE(V)              \
  V(___, ___, 0, 0x00, NOP)                 \
  V(___, ___, 0, 0x01, BLOCK)               \
  V(___, ___, 0, 0x02, LOOP)                \
  V(___, ___, 0, 0x03, IF)                  \
  V(___, ___, 0, 0x04, IF_ELSE)             \
  V(___, ___, 0, 0x05, SELECT)              \
  V(___, ___, 0, 0x06, BR)                  \
  V(___, ___, 0, 0x07, BR_IF)               \
  V(___, ___, 0, 0x08, BR_TABLE)            \
  V(___, ___, 0, 0x14, RETURN)              \
  V(___, ___, 0, 0x15, UNREACHABLE)         \
  V(___, ___, 0, 0x09, I8_CONST)            \
  V(I32, ___, 0, 0x0a, I32_CONST)           \
  V(I64, ___, 0, 0x0b, I64_CONST)           \
  V(F64, ___, 0, 0x0c, F64_CONST)           \
  V(F32, ___, 0, 0x0d, F32_CONST)           \
  V(___, ___, 0, 0x0e, GET_LOCAL)           \
  V(___, ___, 0, 0x0f, SET_LOCAL)           \
  V(___, ___, 0, 0x12, CALL_FUNCTION)       \
  V(___, ___, 0, 0x13, CALL_INDIRECT)       \
  V(___, ___, 0, 0x1f, CALL_IMPORT)         \
  V(I32, ___, 1, 0x20, I32_LOAD8_S)         \
  V(I32, ___, 1, 0x21, I32_LOAD8_U)         \
  V(I32, ___, 2, 0x22, I32_LOAD16_S)        \
  V(I32, ___, 2, 0x23, I32_LOAD16_U)        \
  V(I64, ___, 1, 0x24, I64_LOAD8_S)         \
  V(I64, ___, 1, 0x25, I64_LOAD8_U)         \
  V(I64, ___, 2, 0x26, I64_LOAD16_S)        \
  V(I64, ___, 2, 0x27, I64_LOAD16_U)        \
  V(I64, ___, 4, 0x28, I64_LOAD32_S)        \
  V(I64, ___, 4, 0x29, I64_LOAD32_U)        \
  V(I32, ___, 4, 0x2a, I32_LOAD)            \
  V(I64, ___, 8, 0x2b, I64_LOAD)            \
  V(F32, ___, 4, 0x2c, F32_LOAD)            \
  V(F64, ___, 8, 0x2d, F64_LOAD)            \
  V(I32, ___, 1, 0x2e, I32_STORE8)          \
  V(I32, ___, 2, 0x2f, I32_STORE16)         \
  V(I64, ___, 1, 0x30, I64_STORE8)          \
  V(I64, ___, 2, 0x31, I64_STORE16)         \
  V(I64, ___, 4, 0x32, I64_STORE32)         \
  V(I32, ___, 4, 0x33, I32_STORE)           \
  V(I64, ___, 8, 0x34, I64_STORE)           \
  V(F32, ___, 4, 0x35, F32_STORE)           \
  V(F64, ___, 8, 0x36, F64_STORE)           \
  V(___, ___, 0, 0x3b, MEMORY_SIZE)         \
  V(___, ___, 0, 0x39, GROW_MEMORY)         \
  V(I32, ___, 0, 0x40, I32_ADD)             \
  V(I32, ___, 0, 0x41, I32_SUB)             \
  V(I32, ___, 0, 0x42, I32_MUL)             \
  V(I32, ___, 0, 0x43, I32_DIV_S)           \
  V(I32, ___, 0, 0x44, I32_DIV_U)           \
  V(I32, ___, 0, 0x45, I32_REM_S)           \
  V(I32, ___, 0, 0x46, I32_REM_U)           \
  V(I32, ___, 0, 0x47, I32_AND)             \
  V(I32, ___, 0, 0x48, I32_OR)              \
  V(I32, ___, 0, 0x49, I32_XOR)             \
  V(I32, ___, 0, 0x4a, I32_SHL)             \
  V(I32, ___, 0, 0x4b, I32_SHR_U)           \
  V(I32, ___, 0, 0x4c, I32_SHR_S)           \
  V(I32, ___, 0, 0x4d, I32_EQ)              \
  V(I32, ___, 0, 0x4e, I32_NE)              \
  V(I32, ___, 0, 0x4f, I32_LT_S)            \
  V(I32, ___, 0, 0x50, I32_LE_S)            \
  V(I32, ___, 0, 0x51, I32_LT_U)            \
  V(I32, ___, 0, 0x52, I32_LE_U)            \
  V(I32, ___, 0, 0x53, I32_GT_S)            \
  V(I32, ___, 0, 0x54, I32_GE_S)            \
  V(I32, ___, 0, 0x55, I32_GT_U)            \
  V(I32, ___, 0, 0x56, I32_GE_U)            \
  V(I32, ___, 0, 0x57, I32_CLZ)             \
  V(I32, ___, 0, 0x58, I32_CTZ)             \
  V(I32, ___, 0, 0x59, I32_POPCNT)          \
  V(I32, I32, 0, 0x5a, I32_EQZ)             \
  V(I64, ___, 0, 0x5b, I64_ADD)             \
  V(I64, ___, 0, 0x5c, I64_SUB)             \
  V(I64, ___, 0, 0x5d, I64_MUL)             \
  V(I64, ___, 0, 0x5e, I64_DIV_S)           \
  V(I64, ___, 0, 0x5f, I64_DIV_U)           \
  V(I64, ___, 0, 0x60, I64_REM_S)           \
  V(I64, ___, 0, 0x61, I64_REM_U)           \
  V(I64, ___, 0, 0x62, I64_AND)             \
  V(I64, ___, 0, 0x63, I64_OR)              \
  V(I64, ___, 0, 0x64, I64_XOR)             \
  V(I64, ___, 0, 0x65, I64_SHL)             \
  V(I64, ___, 0, 0x66, I64_SHR_U)           \
  V(I64, ___, 0, 0x67, I64_SHR_S)           \
  V(I64, ___, 0, 0x68, I64_EQ)              \
  V(I64, ___, 0, 0x69, I64_NE)              \
  V(I64, ___, 0, 0x6a, I64_LT_S)            \
  V(I64, ___, 0, 0x6b, I64_LE_S)            \
  V(I64, ___, 0, 0x6c, I64_LT_U)            \
  V(I64, ___, 0, 0x6d, I64_LE_U)            \
  V(I64, ___, 0, 0x6e, I64_GT_S)            \
  V(I64, ___, 0, 0x6f, I64_GE_S)            \
  V(I64, ___, 0, 0x70, I64_GT_U)            \
  V(I64, ___, 0, 0x71, I64_GE_U)            \
  V(I64, ___, 0, 0x72, I64_CLZ)             \
  V(I64, ___, 0, 0x73, I64_CTZ)             \
  V(I64, ___, 0, 0x74, I64_POPCNT)          \
  V(F32, ___, 0, 0x75, F32_ADD)             \
  V(F32, ___, 0, 0x76, F32_SUB)             \
  V(F32, ___, 0, 0x77, F32_MUL)             \
  V(F32, ___, 0, 0x78, F32_DIV)             \
  V(F32, ___, 0, 0x79, F32_MIN)             \
  V(F32, ___, 0, 0x7a, F32_MAX)             \
  V(F32, ___, 0, 0x7b, F32_ABS)             \
  V(F32, ___, 0, 0x7c, F32_NEG)             \
  V(F32, ___, 0, 0x7d, F32_COPYSIGN)        \
  V(F32, ___, 0, 0x7e, F32_CEIL)            \
  V(F32, ___, 0, 0x7f, F32_FLOOR)           \
  V(F32, ___, 0, 0x80, F32_TRUNC)           \
  V(F32, ___, 0, 0x81, F32_NEAREST)         \
  V(F32, ___, 0, 0x82, F32_SQRT)            \
  V(F32, ___, 0, 0x83, F32_EQ)              \
  V(F32, ___, 0, 0x84, F32_NE)              \
  V(F32, ___, 0, 0x85, F32_LT)              \
  V(F32, ___, 0, 0x86, F32_LE)              \
  V(F32, ___, 0, 0x87, F32_GT)              \
  V(F32, ___, 0, 0x88, F32_GE)              \
  V(F64, ___, 0, 0x89, F64_ADD)             \
  V(F64, ___, 0, 0x8a, F64_SUB)             \
  V(F64, ___, 0, 0x8b, F64_MUL)             \
  V(F64, ___, 0, 0x8c, F64_DIV)             \
  V(F64, ___, 0, 0x8d, F64_MIN)             \
  V(F64, ___, 0, 0x8e, F64_MAX)             \
  V(F64, ___, 0, 0x8f, F64_ABS)             \
  V(F64, ___, 0, 0x90, F64_NEG)             \
  V(F64, ___, 0, 0x91, F64_COPYSIGN)        \
  V(F64, ___, 0, 0x92, F64_CEIL)            \
  V(F64, ___, 0, 0x93, F64_FLOOR)           \
  V(F64, ___, 0, 0x94, F64_TRUNC)           \
  V(F64, ___, 0, 0x95, F64_NEAREST)         \
  V(F64, ___, 0, 0x96, F64_SQRT)            \
  V(F64, ___, 0, 0x97, F64_EQ)              \
  V(F64, ___, 0, 0x98, F64_NE)              \
  V(F64, ___, 0, 0x99, F64_LT)              \
  V(F64, ___, 0, 0x9a, F64_LE)              \
  V(F64, ___, 0, 0x9b, F64_GT)              \
  V(F64, ___, 0, 0x9c, F64_GE)              \
  V(I32, F32, 0, 0x9d, I32_TRUNC_S_F32)     \
  V(I32, F64, 0, 0x9e, I32_TRUNC_S_F64)     \
  V(I32, F32, 0, 0x9f, I32_TRUNC_U_F32)     \
  V(I32, F64, 0, 0xa0, I32_TRUNC_U_F64)     \
  V(I32, I64, 0, 0xa1, I32_WRAP_I64)        \
  V(I64, F32, 0, 0xa2, I64_TRUNC_S_F32)     \
  V(I64, F64, 0, 0xa3, I64_TRUNC_S_F64)     \
  V(I64, F32, 0, 0xa4, I64_TRUNC_U_F32)     \
  V(I64, F64, 0, 0xa5, I64_TRUNC_U_F64)     \
  V(I64, I32, 0, 0xa6, I64_EXTEND_S_I32)    \
  V(I64, I32, 0, 0xa7, I64_EXTEND_U_I32)    \
  V(F32, I32, 0, 0xa8, F32_CONVERT_S_I32)   \
  V(F32, I32, 0, 0xa9, F32_CONVERT_U_I32)   \
  V(F32, I64, 0, 0xaa, F32_CONVERT_S_I64)   \
  V(F32, I64, 0, 0xab, F32_CONVERT_U_I64)   \
  V(F32, F64, 0, 0xac, F32_DEMOTE_F64)      \
  V(F32, I32, 0, 0xad, F32_REINTERPRET_I32) \
  V(F64, I32, 0, 0xae, F64_CONVERT_S_I32)   \
  V(F64, I32, 0, 0xaf, F64_CONVERT_U_I32)   \
  V(F64, I64, 0, 0xb0, F64_CONVERT_S_I64)   \
  V(F64, I64, 0, 0xb1, F64_CONVERT_U_I64)   \
  V(F64, F32, 0, 0xb2, F64_PROMOTE_F32)     \
  V(F64, I64, 0, 0xb3, F64_REINTERPRET_I64) \
  V(I32, F32, 0, 0xb4, I32_REINTERPRET_F32) \
  V(I64, F64, 0, 0xb5, I64_REINTERPRET_F64) \
  V(I32, ___, 0, 0xb6, I32_ROTR)            \
  V(I32, ___, 0, 0xb7, I32_ROTL)            \
  V(I64, ___, 0, 0xb8, I64_ROTR)            \
  V(I64, ___, 0, 0xb9, I64_ROTL)            \
  V(I32, I64, 0, 0xba, I64_EQZ)

typedef enum WasmOpcode {
#define V(type1, type2, mem_size, code, name) WASM_OPCODE_##name = code,
  WASM_FOREACH_OPCODE(V)
#undef V
} WasmOpcode;

typedef enum WasmLiteralType {
  WASM_LITERAL_TYPE_INT,
  WASM_LITERAL_TYPE_FLOAT,
  WASM_LITERAL_TYPE_HEXFLOAT,
  WASM_LITERAL_TYPE_INFINITY,
  WASM_LITERAL_TYPE_NAN,
} WasmLiteralType;

typedef struct WasmLiteral {
  WasmLiteralType type;
  WasmStringSlice text;
} WasmLiteral;

WASM_EXTERN_C_BEGIN
int wasm_string_slices_are_equal(const WasmStringSlice*,
                                 const WasmStringSlice*);
void wasm_destroy_string_slice(struct WasmAllocator*, WasmStringSlice*);
/* dump memory to stdout similar to the xxd format */
void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc);
WASM_EXTERN_C_END

#endif /* WASM_COMMON_H_ */
