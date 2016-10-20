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

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

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
#define WASM_USE(x) (void)x

#define WASM_UNKNOWN_OFFSET ((uint32_t)~0)
#define WASM_PAGE_SIZE 0x10000 /* 64k */
#define WASM_MAX_PAGES 0x10000 /* # of pages that fit in 32-bit address space */
#define WASM_BYTES_TO_PAGES(x) ((x) >> 16)
#define WASM_ALIGN_UP_TO_PAGE(x) \
  (((x) + WASM_PAGE_SIZE - 1) & ~(WASM_PAGE_SIZE - 1))

#define PRIstringslice "%.*s"
#define WASM_PRINTF_STRING_SLICE_ARG(x) (int)((x).length), (x).start

#define WASM_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE 128
#define WASM_SNPRINTF_ALLOCA(buffer, len, format)                          \
  va_list args;                                                            \
  va_list args_copy;                                                       \
  va_start(args, format);                                                  \
  va_copy(args_copy, args);                                                \
  char fixed_buf[WASM_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE];                    \
  char* buffer = fixed_buf;                                                \
  size_t len = wasm_vsnprintf(fixed_buf, sizeof(fixed_buf), format, args); \
  va_end(args);                                                            \
  if (len + 1 > sizeof(fixed_buf)) {                                       \
    buffer = alloca(len + 1);                                              \
    len = wasm_vsnprintf(buffer, len + 1, format, args_copy);              \
  }                                                                        \
  va_end(args_copy)

struct WasmAllocator;

typedef enum WasmBool {
  WASM_FALSE,
  WASM_TRUE,
} WasmBool;

typedef enum WasmResult {
  WASM_OK,
  WASM_ERROR,
} WasmResult;

#define WASM_SUCCEEDED(x) ((x) == WASM_OK)
#define WASM_FAILED(x) ((x) == WASM_ERROR)

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

typedef void (*WasmSourceErrorCallback)(const WasmLocation*,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data);

typedef struct WasmSourceErrorHandler {
  WasmSourceErrorCallback on_error;
  /* on_error will be called with with source_line trimmed to this length */
  size_t source_line_max_length;
  void* user_data;
} WasmSourceErrorHandler;

#define WASM_SOURCE_ERROR_HANDLER_DEFAULT \
  { wasm_default_source_error_callback, 80, NULL }

#define WASM_ASSERT_INVALID_SOURCE_ERROR_HANDLER_DEFAULT \
  { wasm_default_assert_invalid_source_error_callback, 80, NULL }

#define WASM_ASSERT_MALFORMED_SOURCE_ERROR_HANDLER_DEFAULT \
  { wasm_default_assert_malformed_source_error_callback, 80, NULL }

typedef void (*WasmBinaryErrorCallback)(uint32_t offset,
                                        const char* error,
                                        void* user_data);

typedef struct WasmBinaryErrorHandler {
  WasmBinaryErrorCallback on_error;
  void* user_data;
} WasmBinaryErrorHandler;

#define WASM_BINARY_ERROR_HANDLER_DEFAULT \
  { wasm_default_binary_error_callback, NULL }

/* matches binary format, do not change */
enum {
  WASM_TYPE_VOID = 0,
  WASM_TYPE_I32 = 1,
  WASM_TYPE_I64 = 2,
  WASM_TYPE_F32 = 3,
  WASM_TYPE_F64 = 4,
  WASM_NUM_TYPES,
  WASM_TYPE____ = WASM_TYPE_VOID, /* convenient for the opcode table below */
  /* used when parsing multiple return types to signify an error */
  /* TODO(binji): remove and support properly */
  WASM_TYPE_MULTIPLE = WASM_NUM_TYPES,
};
typedef unsigned char WasmType;

/* matches binary format, do not change */
typedef enum WasmExternalKind {
  WASM_EXTERNAL_KIND_FUNC = 0,
  WASM_EXTERNAL_KIND_TABLE = 1,
  WASM_EXTERNAL_KIND_MEMORY = 2,
  WASM_EXTERNAL_KIND_GLOBAL = 3,
  WASM_NUM_EXTERNAL_KINDS,
} WasmExternalKind;

typedef struct WasmLimits {
  uint64_t initial;
  uint64_t max;
  WasmBool has_max;
} WasmLimits;

enum { WASM_USE_NATURAL_ALIGNMENT = 0xFFFFFFFF };

/*
 *   tr: result type
 *   t1: type of the 1st parameter
 *   t2: type of the 2nd parameter
 *    m: memory size of the operation, if any
 * code: opcode
 * NAME: used to generate the opcode enum
 * text: a string of the opcode name in the AST format
 *
 *  tr  t1    t2   m  code  NAME text
 *  ============================ */
#define WASM_FOREACH_OPCODE(V)                                          \
  V(___, ___, ___, 0, 0x00, UNREACHABLE, "unreachable")                 \
  V(___, ___, ___, 0, 0x01, NOP, "nop")                                 \
  V(___, ___, ___, 0, 0x02, BLOCK, "block")                             \
  V(___, ___, ___, 0, 0x03, LOOP, "loop")                               \
  V(___, ___, ___, 0, 0x04, IF, "if")                                   \
  V(___, ___, ___, 0, 0x05, ELSE, "else")                               \
  V(___, ___, ___, 0, 0x0b, END, "end")                                 \
  V(___, ___, ___, 0, 0x0c, BR, "br")                                   \
  V(___, ___, ___, 0, 0x0d, BR_IF, "br_if")                             \
  V(___, ___, ___, 0, 0x0e, BR_TABLE, "br_table")                       \
  V(___, ___, ___, 0, 0x0f, RETURN, "return")                           \
  V(___, ___, ___, 0, 0x10, CALL, "call")                               \
  V(___, ___, ___, 0, 0x11, CALL_INDIRECT, "call_indirect")             \
  V(___, ___, ___, 0, 0x1a, DROP, "drop")                               \
  V(___, ___, ___, 0, 0x1b, SELECT, "select")                           \
  V(___, ___, ___, 0, 0x20, GET_LOCAL, "get_local")                     \
  V(___, ___, ___, 0, 0x21, SET_LOCAL, "set_local")                     \
  V(___, ___, ___, 0, 0x22, TEE_LOCAL, "tee_local")                     \
  V(___, ___, ___, 0, 0x23, GET_GLOBAL, "get_global")                   \
  V(___, ___, ___, 0, 0x24, SET_GLOBAL, "set_global")                   \
  V(I32, I32, ___, 4, 0x28, I32_LOAD, "i32.load")                       \
  V(I64, I32, ___, 8, 0x29, I64_LOAD, "i64.load")                       \
  V(F32, I32, ___, 4, 0x2a, F32_LOAD, "f32.load")                       \
  V(F64, I32, ___, 8, 0x2b, F64_LOAD, "f64.load")                       \
  V(I32, I32, ___, 1, 0x2c, I32_LOAD8_S, "i32.load8_s")                 \
  V(I32, I32, ___, 1, 0x2d, I32_LOAD8_U, "i32.load8_u")                 \
  V(I32, I32, ___, 2, 0x2e, I32_LOAD16_S, "i32.load16_s")               \
  V(I32, I32, ___, 2, 0x2f, I32_LOAD16_U, "i32.load16_u")               \
  V(I64, I32, ___, 1, 0x30, I64_LOAD8_S, "i64.load8_s")                 \
  V(I64, I32, ___, 1, 0x31, I64_LOAD8_U, "i64.load8_u")                 \
  V(I64, I32, ___, 2, 0x32, I64_LOAD16_S, "i64.load16_s")               \
  V(I64, I32, ___, 2, 0x33, I64_LOAD16_U, "i64.load16_u")               \
  V(I64, I32, ___, 4, 0x34, I64_LOAD32_S, "i64.load32_s")               \
  V(I64, I32, ___, 4, 0x35, I64_LOAD32_U, "i64.load32_u")               \
  V(___, I32, I32, 4, 0x36, I32_STORE, "i32.store")                     \
  V(___, I32, I64, 8, 0x37, I64_STORE, "i64.store")                     \
  V(___, I32, F32, 4, 0x38, F32_STORE, "f32.store")                     \
  V(___, I32, F64, 8, 0x39, F64_STORE, "f64.store")                     \
  V(___, I32, I32, 1, 0x3a, I32_STORE8, "i32.store8")                   \
  V(___, I32, I32, 2, 0x3b, I32_STORE16, "i32.store16")                 \
  V(___, I32, I64, 1, 0x3c, I64_STORE8, "i64.store8")                   \
  V(___, I32, I64, 2, 0x3d, I64_STORE16, "i64.store16")                 \
  V(___, I32, I64, 4, 0x3e, I64_STORE32, "i64.store32")                 \
  V(I32, ___, ___, 0, 0x3f, CURRENT_MEMORY, "current_memory")           \
  V(I32, I32, ___, 0, 0x40, GROW_MEMORY, "grow_memory")                 \
  V(I32, ___, ___, 0, 0x41, I32_CONST, "i32.const")                     \
  V(I64, ___, ___, 0, 0x42, I64_CONST, "i64.const")                     \
  V(F32, ___, ___, 0, 0x43, F32_CONST, "f32.const")                     \
  V(F64, ___, ___, 0, 0x44, F64_CONST, "f64.const")                     \
  V(I32, I32, ___, 0, 0x45, I32_EQZ, "i32.eqz")                         \
  V(I32, I32, I32, 0, 0x46, I32_EQ, "i32.eq")                           \
  V(I32, I32, I32, 0, 0x47, I32_NE, "i32.ne")                           \
  V(I32, I32, I32, 0, 0x48, I32_LT_S, "i32.lt_s")                       \
  V(I32, I32, I32, 0, 0x49, I32_LT_U, "i32.lt_u")                       \
  V(I32, I32, I32, 0, 0x4a, I32_GT_S, "i32.gt_s")                       \
  V(I32, I32, I32, 0, 0x4b, I32_GT_U, "i32.gt_u")                       \
  V(I32, I32, I32, 0, 0x4c, I32_LE_S, "i32.le_s")                       \
  V(I32, I32, I32, 0, 0x4d, I32_LE_U, "i32.le_u")                       \
  V(I32, I32, I32, 0, 0x4e, I32_GE_S, "i32.ge_s")                       \
  V(I32, I32, I32, 0, 0x4f, I32_GE_U, "i32.ge_u")                       \
  V(I32, I64, ___, 0, 0x50, I64_EQZ, "i64.eqz")                         \
  V(I32, I64, I64, 0, 0x51, I64_EQ, "i64.eq")                           \
  V(I32, I64, I64, 0, 0x52, I64_NE, "i64.ne")                           \
  V(I32, I64, I64, 0, 0x53, I64_LT_S, "i64.lt_s")                       \
  V(I32, I64, I64, 0, 0x54, I64_LT_U, "i64.lt_u")                       \
  V(I32, I64, I64, 0, 0x55, I64_GT_S, "i64.gt_s")                       \
  V(I32, I64, I64, 0, 0x56, I64_GT_U, "i64.gt_u")                       \
  V(I32, I64, I64, 0, 0x57, I64_LE_S, "i64.le_s")                       \
  V(I32, I64, I64, 0, 0x58, I64_LE_U, "i64.le_u")                       \
  V(I32, I64, I64, 0, 0x59, I64_GE_S, "i64.ge_s")                       \
  V(I32, I64, I64, 0, 0x5a, I64_GE_U, "i64.ge_u")                       \
  V(I32, F32, F32, 0, 0x5b, F32_EQ, "f32.eq")                           \
  V(I32, F32, F32, 0, 0x5c, F32_NE, "f32.ne")                           \
  V(I32, F32, F32, 0, 0x5d, F32_LT, "f32.lt")                           \
  V(I32, F32, F32, 0, 0x5e, F32_GT, "f32.gt")                           \
  V(I32, F32, F32, 0, 0x5f, F32_LE, "f32.le")                           \
  V(I32, F32, F32, 0, 0x60, F32_GE, "f32.ge")                           \
  V(I32, F64, F64, 0, 0x61, F64_EQ, "f64.eq")                           \
  V(I32, F64, F64, 0, 0x62, F64_NE, "f64.ne")                           \
  V(I32, F64, F64, 0, 0x63, F64_LT, "f64.lt")                           \
  V(I32, F64, F64, 0, 0x64, F64_GT, "f64.gt")                           \
  V(I32, F64, F64, 0, 0x65, F64_LE, "f64.le")                           \
  V(I32, F64, F64, 0, 0x66, F64_GE, "f64.ge")                           \
  V(I32, I32, ___, 0, 0x67, I32_CLZ, "i32.clz")                         \
  V(I32, I32, ___, 0, 0x68, I32_CTZ, "i32.ctz")                         \
  V(I32, I32, ___, 0, 0x69, I32_POPCNT, "i32.popcnt")                   \
  V(I32, I32, I32, 0, 0x6a, I32_ADD, "i32.add")                         \
  V(I32, I32, I32, 0, 0x6b, I32_SUB, "i32.sub")                         \
  V(I32, I32, I32, 0, 0x6c, I32_MUL, "i32.mul")                         \
  V(I32, I32, I32, 0, 0x6d, I32_DIV_S, "i32.div_s")                     \
  V(I32, I32, I32, 0, 0x6e, I32_DIV_U, "i32.div_u")                     \
  V(I32, I32, I32, 0, 0x6f, I32_REM_S, "i32.rem_s")                     \
  V(I32, I32, I32, 0, 0x70, I32_REM_U, "i32.rem_u")                     \
  V(I32, I32, I32, 0, 0x71, I32_AND, "i32.and")                         \
  V(I32, I32, I32, 0, 0x72, I32_OR, "i32.or")                           \
  V(I32, I32, I32, 0, 0x73, I32_XOR, "i32.xor")                         \
  V(I32, I32, I32, 0, 0x74, I32_SHL, "i32.shl")                         \
  V(I32, I32, I32, 0, 0x75, I32_SHR_S, "i32.shr_s")                     \
  V(I32, I32, I32, 0, 0x76, I32_SHR_U, "i32.shr_u")                     \
  V(I32, I32, I32, 0, 0x77, I32_ROTL, "i32.rotl")                       \
  V(I32, I32, I32, 0, 0x78, I32_ROTR, "i32.rotr")                       \
  V(I64, I64, I64, 0, 0x79, I64_CLZ, "i64.clz")                         \
  V(I64, I64, I64, 0, 0x7a, I64_CTZ, "i64.ctz")                         \
  V(I64, I64, I64, 0, 0x7b, I64_POPCNT, "i64.popcnt")                   \
  V(I64, I64, I64, 0, 0x7c, I64_ADD, "i64.add")                         \
  V(I64, I64, I64, 0, 0x7d, I64_SUB, "i64.sub")                         \
  V(I64, I64, I64, 0, 0x7e, I64_MUL, "i64.mul")                         \
  V(I64, I64, I64, 0, 0x7f, I64_DIV_S, "i64.div_s")                     \
  V(I64, I64, I64, 0, 0x80, I64_DIV_U, "i64.div_u")                     \
  V(I64, I64, I64, 0, 0x81, I64_REM_S, "i64.rem_s")                     \
  V(I64, I64, I64, 0, 0x82, I64_REM_U, "i64.rem_u")                     \
  V(I64, I64, I64, 0, 0x83, I64_AND, "i64.and")                         \
  V(I64, I64, I64, 0, 0x84, I64_OR, "i64.or")                           \
  V(I64, I64, I64, 0, 0x85, I64_XOR, "i64.xor")                         \
  V(I64, I64, I64, 0, 0x86, I64_SHL, "i64.shl")                         \
  V(I64, I64, I64, 0, 0x87, I64_SHR_S, "i64.shr_s")                     \
  V(I64, I64, I64, 0, 0x88, I64_SHR_U, "i64.shr_u")                     \
  V(I64, I64, I64, 0, 0x89, I64_ROTL, "i64.rotl")                       \
  V(I64, I64, I64, 0, 0x8a, I64_ROTR, "i64.rotr")                       \
  V(F32, F32, F32, 0, 0x8b, F32_ABS, "f32.abs")                         \
  V(F32, F32, F32, 0, 0x8c, F32_NEG, "f32.neg")                         \
  V(F32, F32, F32, 0, 0x8d, F32_COPYSIGN, "f32.copysign")               \
  V(F32, F32, F32, 0, 0x8e, F32_CEIL, "f32.ceil")                       \
  V(F32, F32, F32, 0, 0x8f, F32_FLOOR, "f32.floor")                     \
  V(F32, F32, F32, 0, 0x90, F32_TRUNC, "f32.trunc")                     \
  V(F32, F32, F32, 0, 0x91, F32_NEAREST, "f32.nearest")                 \
  V(F32, F32, F32, 0, 0x92, F32_SQRT, "f32.sqrt")                       \
  V(F32, F32, F32, 0, 0x93, F32_ADD, "f32.add")                         \
  V(F32, F32, F32, 0, 0x94, F32_SUB, "f32.sub")                         \
  V(F32, F32, F32, 0, 0x95, F32_MUL, "f32.mul")                         \
  V(F32, F32, F32, 0, 0x96, F32_DIV, "f32.div")                         \
  V(F32, F32, F32, 0, 0x97, F32_MIN, "f32.min")                         \
  V(F32, F32, F32, 0, 0x98, F32_MAX, "f32.max")                         \
  V(F64, F64, F64, 0, 0x99, F64_ABS, "f64.abs")                         \
  V(F64, F64, F64, 0, 0x9a, F64_NEG, "f64.neg")                         \
  V(F64, F64, F64, 0, 0x9b, F64_COPYSIGN, "f64.copysign")               \
  V(F64, F64, F64, 0, 0x9c, F64_CEIL, "f64.ceil")                       \
  V(F64, F64, F64, 0, 0x9d, F64_FLOOR, "f64.floor")                     \
  V(F64, F64, F64, 0, 0x9e, F64_TRUNC, "f64.trunc")                     \
  V(F64, F64, F64, 0, 0x9f, F64_NEAREST, "f64.nearest")                 \
  V(F64, F64, F64, 0, 0xa0, F64_SQRT, "f64.sqrt")                       \
  V(F64, F64, F64, 0, 0xa1, F64_ADD, "f64.add")                         \
  V(F64, F64, F64, 0, 0xa2, F64_SUB, "f64.sub")                         \
  V(F64, F64, F64, 0, 0xa3, F64_MUL, "f64.mul")                         \
  V(F64, F64, F64, 0, 0xa4, F64_DIV, "f64.div")                         \
  V(F64, F64, F64, 0, 0xa5, F64_MIN, "f64.min")                         \
  V(F64, F64, F64, 0, 0xa6, F64_MAX, "f64.max")                         \
  V(I32, I64, ___, 0, 0xa7, I32_WRAP_I64, "i32.wrap/i64")               \
  V(I32, F32, ___, 0, 0xa8, I32_TRUNC_S_F32, "i32.trunc_s/f32")         \
  V(I32, F32, ___, 0, 0xa9, I32_TRUNC_U_F32, "i32.trunc_u/f32")         \
  V(I32, F64, ___, 0, 0xaa, I32_TRUNC_S_F64, "i32.trunc_s/f64")         \
  V(I32, F64, ___, 0, 0xab, I32_TRUNC_U_F64, "i32.trunc_u/f64")         \
  V(I64, I32, ___, 0, 0xac, I64_EXTEND_S_I32, "i64.extend_s/i32")       \
  V(I64, I32, ___, 0, 0xad, I64_EXTEND_U_I32, "i64.extend_u/i32")       \
  V(I64, F32, ___, 0, 0xae, I64_TRUNC_S_F32, "i64.trunc_s/f32")         \
  V(I64, F32, ___, 0, 0xaf, I64_TRUNC_U_F32, "i64.trunc_u/f32")         \
  V(I64, F64, ___, 0, 0xb0, I64_TRUNC_S_F64, "i64.trunc_s/f64")         \
  V(I64, F64, ___, 0, 0xb1, I64_TRUNC_U_F64, "i64.trunc_u/f64")         \
  V(F32, I32, ___, 0, 0xb2, F32_CONVERT_S_I32, "f32.convert_s/i32")     \
  V(F32, I32, ___, 0, 0xb3, F32_CONVERT_U_I32, "f32.convert_u/i32")     \
  V(F32, I64, ___, 0, 0xb4, F32_CONVERT_S_I64, "f32.convert_s/i64")     \
  V(F32, I64, ___, 0, 0xb5, F32_CONVERT_U_I64, "f32.convert_u/i64")     \
  V(F32, F64, ___, 0, 0xb6, F32_DEMOTE_F64, "f32.demote/f64")           \
  V(F64, I32, ___, 0, 0xb7, F64_CONVERT_S_I32, "f64.convert_s/i32")     \
  V(F64, I32, ___, 0, 0xb8, F64_CONVERT_U_I32, "f64.convert_u/i32")     \
  V(F64, I64, ___, 0, 0xb9, F64_CONVERT_S_I64, "f64.convert_s/i64")     \
  V(F64, I64, ___, 0, 0xba, F64_CONVERT_U_I64, "f64.convert_u/i64")     \
  V(F64, F32, ___, 0, 0xbb, F64_PROMOTE_F32, "f64.promote/f32")         \
  V(I32, F32, ___, 0, 0xbc, I32_REINTERPRET_F32, "i32.reinterpret/f32") \
  V(I64, F64, ___, 0, 0xbd, I64_REINTERPRET_F64, "i64.reinterpret/f64") \
  V(F32, I32, ___, 0, 0xbe, F32_REINTERPRET_I32, "f32.reinterpret/i32") \
  V(F64, I64, ___, 0, 0xbf, F64_REINTERPRET_I64, "f64.reinterpret/i64")

typedef enum WasmOpcode {
#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  WASM_OPCODE_##NAME = code,
  WASM_FOREACH_OPCODE(V)
#undef V
  WASM_NUM_OPCODES
} WasmOpcode;

typedef struct WasmOpcodeInfo {
  const char* name;
  WasmType result_type;
  WasmType param1_type;
  WasmType param2_type;
  int memory_size;
} WasmOpcodeInfo;

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
/* return 1 if |alignment| matches the alignment of |opcode|, or if |alignment|
 * is WASM_USE_NATURAL_ALIGNMENT */
WasmBool wasm_is_naturally_aligned(WasmOpcode opcode, uint32_t alignment);

/* if |alignment| is WASM_USE_NATURAL_ALIGNMENT, return the alignment of
 * |opcode|, else return |alignment| */
uint32_t wasm_get_opcode_alignment(WasmOpcode opcode, uint32_t alignment);

WasmStringSlice wasm_empty_string_slice(void);
WasmStringSlice wasm_string_slice_from_cstr(const char* string);
WasmBool wasm_string_slices_are_equal(const WasmStringSlice*,
                                      const WasmStringSlice*);
void wasm_destroy_string_slice(struct WasmAllocator*, WasmStringSlice*);
WasmResult wasm_read_file(struct WasmAllocator* allocator,
                          const char* filename,
                          void** out_data,
                          size_t* out_size);

void wasm_default_source_error_callback(const WasmLocation*,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data);
void wasm_default_assert_invalid_source_error_callback(
    const WasmLocation*,
    const char* error,
    const char* source_line,
    size_t source_line_length,
    size_t source_line_column_offset,
    void* user_data);
void wasm_default_assert_malformed_source_error_callback(
    const WasmLocation*,
    const char* error,
    const char* source_line,
    size_t source_line_length,
    size_t source_line_column_offset,
    void* user_data);
void wasm_default_binary_error_callback(uint32_t offset,
                                        const char* error,
                                        void* user_data);

void wasm_init_stdio();

/* opcode info */
extern WasmOpcodeInfo g_wasm_opcode_info[];

static WASM_INLINE const char* wasm_get_opcode_name(WasmOpcode opcode) {
  assert(opcode < WASM_NUM_OPCODES);
  return g_wasm_opcode_info[opcode].name;
}

static WASM_INLINE WasmType wasm_get_opcode_result_type(WasmOpcode opcode) {
  assert(opcode < WASM_NUM_OPCODES);
  return g_wasm_opcode_info[opcode].result_type;
}

static WASM_INLINE WasmType wasm_get_opcode_param_type_1(WasmOpcode opcode) {
  assert(opcode < WASM_NUM_OPCODES);
  return g_wasm_opcode_info[opcode].param1_type;
}

static WASM_INLINE WasmType wasm_get_opcode_param_type_2(WasmOpcode opcode) {
  assert(opcode < WASM_NUM_OPCODES);
  return g_wasm_opcode_info[opcode].param2_type;
}

static WASM_INLINE int wasm_get_opcode_memory_size(WasmOpcode opcode) {
  assert(opcode < WASM_NUM_OPCODES);
  return g_wasm_opcode_info[opcode].memory_size;
}

/* external kind */

extern const char* g_wasm_kind_name[];

static WASM_INLINE const char* wasm_get_kind_name(WasmExternalKind kind) {
  assert(kind < WASM_NUM_EXTERNAL_KINDS);
  return g_wasm_kind_name[kind];
}

WASM_EXTERN_C_END

#endif /* WASM_COMMON_H_ */
