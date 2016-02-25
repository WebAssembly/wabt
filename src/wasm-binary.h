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

#ifndef WASM_BINARY_H_
#define WASM_BINARY_H_

#include <stdint.h>

#define WASM_BINARY_MAGIC 0x6d736100
#define WASM_BINARY_VERSION 0x0a

typedef enum WasmBinarySectionType {
  WASM_BINARY_SECTION_MEMORY = 0,
  WASM_BINARY_SECTION_SIGNATURES = 1,
  WASM_BINARY_SECTION_FUNCTIONS = 2,
  WASM_BINARY_SECTION_DATA_SEGMENTS = 4,
  WASM_BINARY_SECTION_FUNCTION_TABLE = 5,
  WASM_BINARY_SECTION_END = 6,
  WASM_BINARY_SECTION_START = 7,
  WASM_BINARY_SECTION_IMPORTS = 8,
  WASM_BINARY_SECTION_EXPORTS = 9,
} WasmBinarySectionType;

enum {
  WASM_BINARY_FUNCTION_FLAG_NAME = 1,
  WASM_BINARY_FUNCTION_FLAG_IMPORT = 2,
  WASM_BINARY_FUNCTION_FLAG_LOCALS = 4,
  WASM_BINARY_FUNCTION_FLAG_EXPORT = 8,
};
typedef uint8_t WasmBinaryFunctionFlags;

#define WASM_FOREACH_OPCODE(V) \
  V(NOP, 0x00)                 \
  V(BLOCK, 0x01)               \
  V(LOOP, 0x02)                \
  V(IF, 0x03)                  \
  V(IF_THEN, 0x04)             \
  V(SELECT, 0x05)              \
  V(BR, 0x06)                  \
  V(BR_IF, 0x07)               \
  V(TABLESWITCH, 0x08)         \
  V(RETURN, 0x14)              \
  V(UNREACHABLE, 0x15)         \
  V(I8_CONST, 0x09)            \
  V(I32_CONST, 0x0a)           \
  V(I64_CONST, 0x0b)           \
  V(F64_CONST, 0x0c)           \
  V(F32_CONST, 0x0d)           \
  V(GET_LOCAL, 0x0e)           \
  V(SET_LOCAL, 0x0f)           \
  V(CALL_FUNCTION, 0x12)       \
  V(CALL_INDIRECT, 0x13)       \
  V(CALL_IMPORT, 0x1f)         \
  V(I32_LOAD_MEM8_S, 0x20)     \
  V(I32_LOAD_MEM8_U, 0x21)     \
  V(I32_LOAD_MEM16_S, 0x22)    \
  V(I32_LOAD_MEM16_U, 0x23)    \
  V(I64_LOAD_MEM8_S, 0x24)     \
  V(I64_LOAD_MEM8_U, 0x25)     \
  V(I64_LOAD_MEM16_S, 0x26)    \
  V(I64_LOAD_MEM16_U, 0x27)    \
  V(I64_LOAD_MEM32_S, 0x28)    \
  V(I64_LOAD_MEM32_U, 0x29)    \
  V(I32_LOAD_MEM, 0x2a)        \
  V(I64_LOAD_MEM, 0x2b)        \
  V(F32_LOAD_MEM, 0x2c)        \
  V(F64_LOAD_MEM, 0x2d)        \
  V(I32_STORE_MEM8, 0x2e)      \
  V(I32_STORE_MEM16, 0x2f)     \
  V(I64_STORE_MEM8, 0x30)      \
  V(I64_STORE_MEM16, 0x31)     \
  V(I64_STORE_MEM32, 0x32)     \
  V(I32_STORE_MEM, 0x33)       \
  V(I64_STORE_MEM, 0x34)       \
  V(F32_STORE_MEM, 0x35)       \
  V(F64_STORE_MEM, 0x36)       \
  V(MEMORY_SIZE, 0x3b)         \
  V(GROW_MEMORY, 0x39)         \
  V(I32_ADD, 0x40)             \
  V(I32_SUB, 0x41)             \
  V(I32_MUL, 0x42)             \
  V(I32_DIV_S, 0x43)           \
  V(I32_DIV_U, 0x44)           \
  V(I32_REM_S, 0x45)           \
  V(I32_REM_U, 0x46)           \
  V(I32_AND, 0x47)             \
  V(I32_OR, 0x48)              \
  V(I32_XOR, 0x49)             \
  V(I32_SHL, 0x4a)             \
  V(I32_SHR_U, 0x4b)           \
  V(I32_SHR_S, 0x4c)           \
  V(I32_EQ, 0x4d)              \
  V(I32_NE, 0x4e)              \
  V(I32_LT_S, 0x4f)            \
  V(I32_LE_S, 0x50)            \
  V(I32_LT_U, 0x51)            \
  V(I32_LE_U, 0x52)            \
  V(I32_GT_S, 0x53)            \
  V(I32_GE_S, 0x54)            \
  V(I32_GT_U, 0x55)            \
  V(I32_GE_U, 0x56)            \
  V(I32_CLZ, 0x57)             \
  V(I32_CTZ, 0x58)             \
  V(I32_POPCNT, 0x59)          \
  V(BOOL_NOT, 0x5a)            \
  V(I64_ADD, 0x5b)             \
  V(I64_SUB, 0x5c)             \
  V(I64_MUL, 0x5d)             \
  V(I64_DIV_S, 0x5e)           \
  V(I64_DIV_U, 0x5f)           \
  V(I64_REM_S, 0x60)           \
  V(I64_REM_U, 0x61)           \
  V(I64_AND, 0x62)             \
  V(I64_OR, 0x63)              \
  V(I64_XOR, 0x64)             \
  V(I64_SHL, 0x65)             \
  V(I64_SHR_U, 0x66)           \
  V(I64_SHR_S, 0x67)           \
  V(I64_EQ, 0x68)              \
  V(I64_NE, 0x69)              \
  V(I64_LT_S, 0x6a)            \
  V(I64_LE_S, 0x6b)            \
  V(I64_LT_U, 0x6c)            \
  V(I64_LE_U, 0x6d)            \
  V(I64_GT_S, 0x6e)            \
  V(I64_GE_S, 0x6f)            \
  V(I64_GT_U, 0x70)            \
  V(I64_GE_U, 0x71)            \
  V(I64_CLZ, 0x72)             \
  V(I64_CTZ, 0x73)             \
  V(I64_POPCNT, 0x74)          \
  V(F32_ADD, 0x75)             \
  V(F32_SUB, 0x76)             \
  V(F32_MUL, 0x77)             \
  V(F32_DIV, 0x78)             \
  V(F32_MIN, 0x79)             \
  V(F32_MAX, 0x7a)             \
  V(F32_ABS, 0x7b)             \
  V(F32_NEG, 0x7c)             \
  V(F32_COPYSIGN, 0x7d)        \
  V(F32_CEIL, 0x7e)            \
  V(F32_FLOOR, 0x7f)           \
  V(F32_TRUNC, 0x80)           \
  V(F32_NEAREST_INT, 0x81)     \
  V(F32_SQRT, 0x82)            \
  V(F32_EQ, 0x83)              \
  V(F32_NE, 0x84)              \
  V(F32_LT, 0x85)              \
  V(F32_LE, 0x86)              \
  V(F32_GT, 0x87)              \
  V(F32_GE, 0x88)              \
  V(F64_ADD, 0x89)             \
  V(F64_SUB, 0x8a)             \
  V(F64_MUL, 0x8b)             \
  V(F64_DIV, 0x8c)             \
  V(F64_MIN, 0x8d)             \
  V(F64_MAX, 0x8e)             \
  V(F64_ABS, 0x8f)             \
  V(F64_NEG, 0x90)             \
  V(F64_COPYSIGN, 0x91)        \
  V(F64_CEIL, 0x92)            \
  V(F64_FLOOR, 0x93)           \
  V(F64_TRUNC, 0x94)           \
  V(F64_NEAREST_INT, 0x95)     \
  V(F64_SQRT, 0x96)            \
  V(F64_EQ, 0x97)              \
  V(F64_NE, 0x98)              \
  V(F64_LT, 0x99)              \
  V(F64_LE, 0x9a)              \
  V(F64_GT, 0x9b)              \
  V(F64_GE, 0x9c)              \
  V(I32_SCONVERT_F32, 0x9d)    \
  V(I32_SCONVERT_F64, 0x9e)    \
  V(I32_UCONVERT_F32, 0x9f)    \
  V(I32_UCONVERT_F64, 0xa0)    \
  V(I32_CONVERT_I64, 0xa1)     \
  V(I64_SCONVERT_F32, 0xa2)    \
  V(I64_SCONVERT_F64, 0xa3)    \
  V(I64_UCONVERT_F32, 0xa4)    \
  V(I64_UCONVERT_F64, 0xa5)    \
  V(I64_SCONVERT_I32, 0xa6)    \
  V(I64_UCONVERT_I32, 0xa7)    \
  V(F32_SCONVERT_I32, 0xa8)    \
  V(F32_UCONVERT_I32, 0xa9)    \
  V(F32_SCONVERT_I64, 0xaa)    \
  V(F32_UCONVERT_I64, 0xab)    \
  V(F32_CONVERT_F64, 0xac)     \
  V(F32_REINTERPRET_I32, 0xad) \
  V(F64_SCONVERT_I32, 0xae)    \
  V(F64_UCONVERT_I32, 0xaf)    \
  V(F64_SCONVERT_I64, 0xb0)    \
  V(F64_UCONVERT_I64, 0xb1)    \
  V(F64_CONVERT_F32, 0xb2)     \
  V(F64_REINTERPRET_I64, 0xb3) \
  V(I32_REINTERPRET_F32, 0xb4) \
  V(I64_REINTERPRET_F64, 0xb5)

typedef enum WasmOpcode {
#define V(name, code) WASM_OPCODE_##name = code,
  WASM_FOREACH_OPCODE(V)
#undef V
} WasmOpcode;

#endif /* WASM_BINARY_H_ */
