#include <assert.h>
#include <ctype.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm.h"
#include "wasm-internal.h"

#define DEFAULT_MEMORY_EXPORT 1
#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

#define SEGMENT_SIZE 13
#define SEGMENT_OFFSET_OFFSET 4

#define IMPORT_SIZE 7
#define IMPORT_NAME_OFFSET 3

#define FUNC_NAME_OFFSET 3

typedef enum WasmSectionType {
  WASM_SECTION_MEMORY = 0,
  WASM_SECTION_SIGNATURES = 1,
  WASM_SECTION_FUNCTIONS = 2,
  WASM_SECTION_GLOBALS = 3,
  WASM_SECTION_DATA_SEGMENTS = 4,
  WASM_SECTION_FUNCTION_TABLE = 5,
  WASM_SECTION_END = 6,
} WasmSectionType;

enum {
  WASM_FUNCTION_FLAG_NAME = 1,
  WASM_FUNCTION_FLAG_IMPORT = 2,
  WASM_FUNCTION_FLAG_LOCALS = 4,
  WASM_FUNCTION_FLAG_EXPORT = 8,
};
typedef uint8_t WasmFunctionFlags;

typedef enum WasmTypeV8 {
  WASM_TYPE_V8_VOID,
  WASM_TYPE_V8_I32,
  WASM_TYPE_V8_I64,
  WASM_TYPE_V8_F32,
  WASM_TYPE_V8_F64,
  WASM_NUM_V8_TYPES,
} WasmTypeV8;

#define FOREACH_OPCODE(V)      \
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
  V(LOAD_GLOBAL, 0x10)         \
  V(STORE_GLOBAL, 0x11)        \
  V(CALL_FUNCTION, 0x12)       \
  V(CALL_INDIRECT, 0x13)       \
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
  V(RESIZE_MEM_L, 0x39)        \
  V(RESIZE_MEM_H, 0x3a)        \
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
  FOREACH_OPCODE(V)
#undef V
} WasmOpcode;

#define V(name, code) [code] = "OPCODE_" #name,
static const char* s_opcode_names[] = {
  FOREACH_OPCODE(V)
};
#undef V

static uint8_t s_binary_opcodes[] = {
    WASM_OPCODE_F32_ADD,   WASM_OPCODE_F32_COPYSIGN, WASM_OPCODE_F32_DIV,
    WASM_OPCODE_F32_MAX,   WASM_OPCODE_F32_MIN,      WASM_OPCODE_F32_MUL,
    WASM_OPCODE_F32_SUB,   WASM_OPCODE_F64_ADD,      WASM_OPCODE_F64_COPYSIGN,
    WASM_OPCODE_F64_DIV,   WASM_OPCODE_F64_MAX,      WASM_OPCODE_F64_MIN,
    WASM_OPCODE_F64_MUL,   WASM_OPCODE_F64_SUB,      WASM_OPCODE_I32_ADD,
    WASM_OPCODE_I32_AND,   WASM_OPCODE_I32_DIV_S,    WASM_OPCODE_I32_DIV_U,
    WASM_OPCODE_I32_MUL,   WASM_OPCODE_I32_OR,       WASM_OPCODE_I32_REM_S,
    WASM_OPCODE_I32_REM_U, WASM_OPCODE_I32_SHL,      WASM_OPCODE_I32_SHR_S,
    WASM_OPCODE_I32_SHR_U, WASM_OPCODE_I32_SUB,      WASM_OPCODE_I32_XOR,
    WASM_OPCODE_I64_ADD,   WASM_OPCODE_I64_AND,      WASM_OPCODE_I64_DIV_S,
    WASM_OPCODE_I64_DIV_U, WASM_OPCODE_I64_MUL,      WASM_OPCODE_I64_OR,
    WASM_OPCODE_I64_REM_S, WASM_OPCODE_I64_REM_U,    WASM_OPCODE_I64_SHL,
    WASM_OPCODE_I64_SHR_S, WASM_OPCODE_I64_SHR_U,    WASM_OPCODE_I64_SUB,
    WASM_OPCODE_I64_XOR,
};

static uint8_t s_compare_opcodes[] = {
    WASM_OPCODE_F32_EQ,   WASM_OPCODE_F32_GE,   WASM_OPCODE_F32_GT,
    WASM_OPCODE_F32_LE,   WASM_OPCODE_F32_LT,   WASM_OPCODE_F32_NE,
    WASM_OPCODE_F64_EQ,   WASM_OPCODE_F64_GE,   WASM_OPCODE_F64_GT,
    WASM_OPCODE_F64_LE,   WASM_OPCODE_F64_LT,   WASM_OPCODE_F64_NE,
    WASM_OPCODE_I32_EQ,   WASM_OPCODE_I32_GE_S, WASM_OPCODE_I32_GE_U,
    WASM_OPCODE_I32_GT_S, WASM_OPCODE_I32_GT_U, WASM_OPCODE_I32_LE_S,
    WASM_OPCODE_I32_LE_U, WASM_OPCODE_I32_LT_S, WASM_OPCODE_I32_LT_U,
    WASM_OPCODE_I32_NE,   WASM_OPCODE_I64_EQ,   WASM_OPCODE_I64_GE_S,
    WASM_OPCODE_I64_GE_U, WASM_OPCODE_I64_GT_S, WASM_OPCODE_I64_GT_U,
    WASM_OPCODE_I64_LE_S, WASM_OPCODE_I64_LE_U, WASM_OPCODE_I64_LT_S,
    WASM_OPCODE_I64_LT_U, WASM_OPCODE_I64_NE,
};

static uint8_t s_convert_opcodes[] = {
    WASM_OPCODE_F32_SCONVERT_I32, WASM_OPCODE_F32_SCONVERT_I64,
    WASM_OPCODE_F32_UCONVERT_I32, WASM_OPCODE_F32_UCONVERT_I64,
    WASM_OPCODE_F32_CONVERT_F64,  WASM_OPCODE_F64_SCONVERT_I32,
    WASM_OPCODE_F64_SCONVERT_I64, WASM_OPCODE_F64_UCONVERT_I32,
    WASM_OPCODE_F64_UCONVERT_I64, WASM_OPCODE_F64_CONVERT_F32,
    WASM_OPCODE_I32_SCONVERT_F32, WASM_OPCODE_I32_SCONVERT_F64,
    WASM_OPCODE_I32_UCONVERT_F32, WASM_OPCODE_I32_UCONVERT_F64,
    WASM_OPCODE_I32_CONVERT_I64,  WASM_OPCODE_I64_SCONVERT_I32,
    WASM_OPCODE_I64_UCONVERT_I32, WASM_OPCODE_I64_SCONVERT_F32,
    WASM_OPCODE_I64_SCONVERT_F64, WASM_OPCODE_I64_UCONVERT_F32,
    WASM_OPCODE_I64_UCONVERT_F64,
};

static uint8_t s_mem_opcodes[] = {
  WASM_OPCODE_F32_LOAD_MEM,
  WASM_OPCODE_F32_STORE_MEM,
  WASM_OPCODE_F64_LOAD_MEM,
  WASM_OPCODE_F64_STORE_MEM,
  WASM_OPCODE_I32_LOAD_MEM,
  WASM_OPCODE_I32_LOAD_MEM8_S,
  WASM_OPCODE_I32_LOAD_MEM8_U,
  WASM_OPCODE_I32_LOAD_MEM16_S,
  WASM_OPCODE_I32_LOAD_MEM16_U,
  WASM_OPCODE_I32_STORE_MEM,
  WASM_OPCODE_I32_STORE_MEM8,
  WASM_OPCODE_I32_STORE_MEM16,
  WASM_OPCODE_I64_LOAD_MEM,
  WASM_OPCODE_I64_LOAD_MEM8_S,
  WASM_OPCODE_I64_LOAD_MEM8_U,
  WASM_OPCODE_I64_LOAD_MEM16_S,
  WASM_OPCODE_I64_LOAD_MEM16_U,
  WASM_OPCODE_I64_LOAD_MEM32_S,
  WASM_OPCODE_I64_LOAD_MEM32_U,
  WASM_OPCODE_I64_STORE_MEM,
  WASM_OPCODE_I64_STORE_MEM8,
  WASM_OPCODE_I64_STORE_MEM16,
  WASM_OPCODE_I64_STORE_MEM32,
};

static uint8_t s_unary_opcodes[] = {
    WASM_OPCODE_F32_ABS,         WASM_OPCODE_F32_CEIL,
    WASM_OPCODE_F32_FLOOR,       WASM_OPCODE_F32_NEAREST_INT,
    WASM_OPCODE_F32_NEG,         WASM_OPCODE_F32_SQRT,
    WASM_OPCODE_F32_TRUNC,       WASM_OPCODE_F64_ABS,
    WASM_OPCODE_F64_CEIL,        WASM_OPCODE_F64_FLOOR,
    WASM_OPCODE_F64_NEAREST_INT, WASM_OPCODE_F64_NEG,
    WASM_OPCODE_F64_SQRT,        WASM_OPCODE_F64_TRUNC,
    WASM_OPCODE_I32_CLZ,         WASM_OPCODE_I32_CTZ,
    WASM_OPCODE_BOOL_NOT,        WASM_OPCODE_I32_POPCNT,
    WASM_OPCODE_I64_CLZ,         WASM_OPCODE_I64_CTZ,
    WASM_OPCODE_I64_POPCNT,
};

typedef struct WasmWriteContext {
  WasmBinaryWriter* writer;
  size_t offset;
  int verbose;

  int* import_sig_indexes;
  int* func_sig_indexes;
  int* remapped_locals;
  size_t* func_offsets;
} WasmWriteContext;

DECLARE_VECTOR(func_signature, WasmFuncSignature);
DEFINE_VECTOR(func_signature, WasmFuncSignature);

static uint32_t log_two_u32(uint32_t x) {
  if (!x)
    return 0;
  return sizeof(unsigned int) * 8 - __builtin_clz(x - 1);
}

static WasmTypeV8 wasm_type_to_v8_type(WasmType type) {
  switch (type) {
    case WASM_TYPE_VOID:
      return WASM_TYPE_V8_VOID;
    case WASM_TYPE_I32:
      return WASM_TYPE_V8_I32;
    case WASM_TYPE_I64:
      return WASM_TYPE_V8_I64;
    case WASM_TYPE_F32:
      return WASM_TYPE_V8_F32;
    case WASM_TYPE_F64:
      return WASM_TYPE_V8_F64;
    default:
      FATAL("v8-native does not support type %d\n", type);
  }
}

static void dump_memory(const void* start,
                        size_t size,
                        size_t offset,
                        int print_chars,
                        const char* desc) {
  /* mimic xxd output */
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    printf("%07x: ", (int)((void*)p - start + offset));
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          printf("%02x", *p);
        } else {
          putchar(' ');
          putchar(' ');
        }
      }
      putchar(' ');
    }

    putchar(' ');
    p = line;
    int i;
    for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
      if (print_chars)
        printf("%c", isprint(*p) ? *p : '.');
    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      printf("  ; %s", desc);
    putchar('\n');
  }
}

static void print_header(WasmWriteContext* ctx, const char* name, int index) {
  if (1 || ctx->verbose)
    printf("; %s %d\n", name, index);
}

static void out_data(WasmWriteContext* ctx,
                     size_t offset,
                     const void* src,
                     size_t size,
                     const char* desc) {
  if (ctx->writer->log_writes)
    dump_memory(src, size, offset, 0, desc);
  if (ctx->writer->write_data)
    ctx->writer->write_data(offset, src, size, ctx->writer->user_data);
}

/* TODO(binji): endianness */
#define OUT_TYPE(name, ctype)                                   \
  static void out_##name(WasmWriteContext* ctx, ctype value,    \
                         const char* desc) {                    \
    out_data(ctx, ctx->offset, &value, sizeof(value), desc); \
    ctx->offset += sizeof(value);                               \
  }

OUT_TYPE(u8, uint8_t)
OUT_TYPE(u16, uint16_t)
OUT_TYPE(u32, uint32_t)
OUT_TYPE(u64, uint64_t)
OUT_TYPE(f32, float)
OUT_TYPE(f64, double)

#undef OUT_TYPE

#define OUT_AT_TYPE(name, ctype)                                      \
  static void out_##name##_at(WasmWriteContext* ctx, uint32_t offset, \
                              ctype value, const char* desc) {        \
    out_data(ctx, offset, &value, sizeof(value), desc);               \
  }

OUT_AT_TYPE(u8, uint8_t)
OUT_AT_TYPE(u16, uint16_t)
OUT_AT_TYPE(u32, uint32_t)

#undef OUT_AT_TYPE

/* returns the number of bytes written */
static int out_leb128_at(WasmWriteContext* ctx,
                         size_t offset,
                         uint32_t value,
                         const char* desc) {
  uint8_t data[5]; /* max 5 bytes */
  int i = 0;
  do {
    assert(i < 5);
    data[i] = value & 0x7f;
    value >>= 7;
    if (value)
      data[i] |= 0x80;
    ++i;
  } while (value);
  out_data(ctx, offset, data, i, desc);
  return i;
}

static void out_leb128(WasmWriteContext* ctx,
                       uint32_t value,
                       const char* desc) {
  ctx->offset += out_leb128_at(ctx, ctx->offset, value, desc);
}

static void out_str(WasmWriteContext* ctx,
                    const char* s,
                    size_t length,
                    const char* desc) {
  out_data(ctx, ctx->offset, s, length, desc);
  ctx->offset += length;
  out_u8(ctx, 0, "\\0");
}

static void out_opcode(WasmWriteContext* ctx, uint8_t opcode) {
  out_u8(ctx, opcode, s_opcode_names[opcode]);
}

static uint8_t binary_opcode(WasmBinaryOp* op) {
  assert(op->op_type >= 0 && op->op_type < ARRAY_SIZE(s_binary_opcodes));
  return s_binary_opcodes[op->op_type];
}

static uint8_t compare_opcode(WasmCompareOp* op) {
  assert(op->op_type >= 0 && op->op_type < ARRAY_SIZE(s_compare_opcodes));
  return s_compare_opcodes[op->op_type];
}

static uint8_t convert_opcode(WasmConvertOp* op) {
  assert(op->op_type >= 0 && op->op_type < ARRAY_SIZE(s_convert_opcodes));
  return s_convert_opcodes[op->op_type];
}

static uint8_t mem_opcode(WasmMemOp* op) {
  assert(op->op_type >= 0 && op->op_type < ARRAY_SIZE(s_mem_opcodes));
  return s_mem_opcodes[op->op_type];
}

static uint8_t unary_opcode(WasmUnaryOp* op) {
  assert(op->op_type >= 0 && op->op_type < ARRAY_SIZE(s_unary_opcodes));
  return s_unary_opcodes[op->op_type];
}

static int find_func_signature(WasmFuncSignatureVector* sigs,
                               WasmType result_type,
                               WasmTypeVector* param_types) {
  int i;
  for (i = 0; i < sigs->size; ++i) {
    WasmFuncSignature* sig2 = &sigs->data[i];
    if (sig2->result_type != result_type)
      continue;
    if (sig2->param_types.size != param_types->size)
      continue;
    int j;
    for (j = 0; j < param_types->size; ++j) {
      if (sig2->param_types.data[j] != param_types->data[j])
        break;
    }
    if (j == param_types->size)
      return i;
  }
  return -1;
}

static void get_func_signatures(WasmWriteContext* ctx,
                                WasmModule* module,
                                WasmFuncSignatureVector* sigs) {
  /* function types are not deduped; we don't want the signature index to match
   if they were specified separately in the source */
  int i;
  for (i = 0; i < module->func_types.size; ++i) {
    WasmFuncType* func_type = module->func_types.data[i];
    WasmFuncSignature* sig = wasm_append_func_signature(sigs);
    sig->result_type = func_type->sig.result_type;
    wasm_extend_types(&sig->param_types, &func_type->sig.param_types);
  }

  ctx->import_sig_indexes =
      realloc(ctx->import_sig_indexes, module->imports.size * sizeof(int));
  for (i = 0; i < module->imports.size; ++i) {
    WasmImport* import = module->imports.data[i];
    int index;
    if (import->import_type == WASM_IMPORT_HAS_FUNC_SIGNATURE) {
      index = find_func_signature(sigs, import->func_sig.result_type,
                                  &import->func_sig.param_types);
      if (index == -1) {
        index = sigs->size;
        WasmFuncSignature* sig = wasm_append_func_signature(sigs);
        sig->result_type = import->func_sig.result_type;
        wasm_extend_types(&sig->param_types, &import->func_sig.param_types);
      }
    } else {
      assert(import->import_type == WASM_IMPORT_HAS_TYPE);
      WasmFuncType* func_type =
          wasm_get_func_type_by_var(module, &import->type_var);
      assert(func_type);
      index = find_func_signature(sigs, func_type->sig.result_type,
                                  &func_type->sig.param_types);
      assert(index != -1);
    }

    ctx->import_sig_indexes[i] = index;
  }

  ctx->func_sig_indexes =
      realloc(ctx->func_sig_indexes, module->funcs.size * sizeof(int));
  for (i = 0; i < module->funcs.size; ++i) {
    WasmFunc* func = module->funcs.data[i];
    int index;
    if (func->flags & WASM_FUNC_FLAG_HAS_SIGNATURE) {
      index = find_func_signature(sigs, func->result_type, &func->params.types);
      if (index == -1) {
        index = sigs->size;
        WasmFuncSignature* sig = wasm_append_func_signature(sigs);
        sig->result_type = func->result_type;
        wasm_extend_types(&sig->param_types, &func->params.types);
      }
    } else {
      assert(func->flags & WASM_FUNC_FLAG_HAS_FUNC_TYPE);
      WasmFuncType* func_type =
          wasm_get_func_type_by_var(module, &func->type_var);
      assert(func_type);
      index = find_func_signature(sigs, func_type->sig.result_type,
                                  &func_type->sig.param_types);
      assert(index != -1);
    }

    ctx->func_sig_indexes[i] = index;
  }
}

static void remap_locals(WasmWriteContext* ctx, WasmFunc* func) {
  ctx->remapped_locals =
      realloc(ctx->remapped_locals, func->locals.types.size * sizeof(int));

  int max[WASM_NUM_V8_TYPES] = {};
  int i;
  for (i = 0; i < func->locals.types.size; ++i) {
    WasmType type = func->locals.types.data[i];
    max[wasm_type_to_v8_type(type)]++;
  }

  /* Args don't need remapping */
  for (i = 0; i < func->params.types.size; ++i)
    ctx->remapped_locals[i] = i;

  int start[WASM_NUM_V8_TYPES];
  start[WASM_TYPE_V8_I32] = func->params.types.size;
  start[WASM_TYPE_V8_I64] = start[WASM_TYPE_V8_I32] + max[WASM_TYPE_V8_I32];
  start[WASM_TYPE_V8_F32] = start[WASM_TYPE_V8_I64] + max[WASM_TYPE_V8_I64];
  start[WASM_TYPE_V8_F64] = start[WASM_TYPE_V8_F32] + max[WASM_TYPE_V8_F32];

  int seen[WASM_NUM_V8_TYPES] = {};
  for (i = 0; i < func->locals.types.size; ++i) {
    WasmType type = func->locals.types.data[i];
    WasmTypeV8 v8_type = wasm_type_to_v8_type(type);
    ctx->remapped_locals[i] = start[v8_type] + seen[v8_type]++;
  }
}

static WasmResult write_expr_list(WasmWriteContext* ctx,
                                  WasmModule* module,
                                  WasmFunc* func,
                                  WasmExprPtrVector* exprs);

static WasmResult write_expr(WasmWriteContext* ctx,
                             WasmModule* module,
                             WasmFunc* func,
                             WasmExpr* expr) {
  WasmResult result = WASM_OK;
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      out_opcode(ctx, binary_opcode(&expr->binary.op));
      result |= write_expr(ctx, module, func, expr->binary.left);
      result |= write_expr(ctx, module, func, expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      out_opcode(ctx, WASM_OPCODE_BLOCK);
      out_u8(ctx, expr->block.exprs.size, "num expressions");
      result |= write_expr_list(ctx, module, func, &expr->block.exprs);
      break;
    case WASM_EXPR_TYPE_BR:
      break;
    case WASM_EXPR_TYPE_BR_IF:
      break;
    case WASM_EXPR_TYPE_CALL:
      break;
    case WASM_EXPR_TYPE_CALL_IMPORT:
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      break;
    case WASM_EXPR_TYPE_CAST:
      break;
    case WASM_EXPR_TYPE_COMPARE:
      out_opcode(ctx, compare_opcode(&expr->compare.op));
      result |= write_expr(ctx, module, func, expr->compare.left);
      result |= write_expr(ctx, module, func, expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONST:
      switch (expr->const_.type) {
        case WASM_TYPE_I32: {
          int32_t i32;
          memcpy(&i32, &expr->const_.u32, sizeof(i32));
          if (i32 >= -128 && i32 <= 127) {
            out_opcode(ctx, WASM_OPCODE_I8_CONST);
            out_u8(ctx, expr->const_.u32, "u8 literal");
          } else {
            out_opcode(ctx, WASM_OPCODE_I32_CONST);
            out_u32(ctx, expr->const_.u32, "u32 literal");
          }
          break;
        }
        case WASM_TYPE_I64:
          out_opcode(ctx, WASM_OPCODE_I64_CONST);
          out_u64(ctx, expr->const_.u64, "u64 literal");
          break;
        case WASM_TYPE_F32:
          out_opcode(ctx, WASM_OPCODE_F32_CONST);
          out_f32(ctx, expr->const_.f32, "f32 literal");
          break;
        case WASM_TYPE_F64:
          out_opcode(ctx, WASM_OPCODE_F64_CONST);
          out_f64(ctx, expr->const_.f64, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WASM_EXPR_TYPE_CONVERT:
      out_opcode(ctx, convert_opcode(&expr->convert.op));
      result |= write_expr(ctx, module, func, expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      break;
    case WASM_EXPR_TYPE_GROW_MEMORY:
      break;
    case WASM_EXPR_TYPE_HAS_FEATURE:
      break;
    case WASM_EXPR_TYPE_IF:
      out_opcode(ctx, WASM_OPCODE_IF);
      result |= write_expr(ctx, module, func, expr->if_.cond);
      result |= write_expr(ctx, module, func, expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      out_opcode(ctx, WASM_OPCODE_IF_THEN);
      result |= write_expr(ctx, module, func, expr->if_else.cond);
      result |= write_expr(ctx, module, func, expr->if_else.true_);
      result |= write_expr(ctx, module, func, expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LABEL:
      out_opcode(ctx, WASM_OPCODE_BLOCK);
      out_u8(ctx, 1, "num expressions");
      result |= write_expr(ctx, module, func, expr->label.expr);
      break;
    case WASM_EXPR_TYPE_LOAD:
      out_opcode(ctx, mem_opcode(&expr->load.op));
      result |= write_expr(ctx, module, func, expr->load.addr);
      break;
    case WASM_EXPR_TYPE_LOAD_EXTEND:
      break;
    case WASM_EXPR_TYPE_LOAD_GLOBAL:
      break;
    case WASM_EXPR_TYPE_LOOP:
      out_opcode(ctx, WASM_OPCODE_LOOP);
      result |= write_expr(ctx, module, func, expr->label.expr);
      break;
    case WASM_EXPR_TYPE_MEMORY_SIZE:
      out_opcode(ctx, WASM_OPCODE_MEMORY_SIZE);
      break;
    case WASM_EXPR_TYPE_NOP:
      out_opcode(ctx, WASM_OPCODE_NOP);
      break;
    case WASM_EXPR_TYPE_PAGE_SIZE:
      break;
    case WASM_EXPR_TYPE_RETURN:
      out_opcode(ctx, WASM_OPCODE_RETURN);
      if (expr->return_.expr)
        result |= write_expr(ctx, module, func, expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      break;
    case WASM_EXPR_TYPE_STORE:
      break;
    case WASM_EXPR_TYPE_STORE_GLOBAL:
      break;
    case WASM_EXPR_TYPE_STORE_WRAP:
      break;
    case WASM_EXPR_TYPE_TABLESWITCH:
      break;
    case WASM_EXPR_TYPE_UNARY:
      out_opcode(ctx, unary_opcode(&expr->unary.op));
      result |= write_expr(ctx, module, func, expr->unary.expr);
      break;
    case WASM_EXPR_TYPE_UNREACHABLE:
      out_opcode(ctx, WASM_OPCODE_UNREACHABLE);
      break;
  }
  return result;
}

static WasmResult write_expr_list(WasmWriteContext* ctx,
                                  WasmModule* module,
                                  WasmFunc* func,
                                  WasmExprPtrVector* exprs) {
  WasmResult result = WASM_OK;
  int i;
  for (i = 0; i < exprs->size; ++i)
    result |= write_expr(ctx, module, func, exprs->data[i]);
  return result;
}

static WasmResult write_func(WasmWriteContext* ctx,
                             WasmModule* module,
                             WasmFunc* func) {
  return write_expr_list(ctx, module, func, &func->exprs);
}

static WasmResult write_module(WasmWriteContext* ctx, WasmModule* module) {
  WasmResult result = WASM_OK;
  int i;

  size_t segments_offset;
  if (module->memory) {
    out_u8(ctx, WASM_SECTION_MEMORY, "WASM_SECTION_MEMORY");
    out_u8(ctx, log_two_u32(module->memory->initial_size), "min mem size log 2");
    out_u8(ctx, log_two_u32(module->memory->max_size), "max mem size log 2");
    out_u8(ctx, DEFAULT_MEMORY_EXPORT, "export mem");

    if (module->memory->segments.size) {
      out_u8(ctx, WASM_SECTION_DATA_SEGMENTS, "WASM_SECTION_DATA_SEGMENTS");
      out_leb128(ctx, module->memory->segments.size, "num data segments");
      segments_offset = ctx->offset;
      for (i = 0; i < module->memory->segments.size; ++i) {
        WasmSegment* segment = &module->memory->segments.data[i];
        print_header(ctx, "segment header", i);
        out_u32(ctx, segment->addr, "segment address");
        out_u32(ctx, 0, "segment data offset");
        out_u32(ctx, segment->size, "segment size");
        out_u8(ctx, 1, "segment init");
      }
    }
  } else {
    /* TODO(binji): remove? */
    out_u8(ctx, WASM_SECTION_MEMORY, "WASM_SECTION_MEMORY");
    out_u8(ctx, 0, "min mem size log 2");
    out_u8(ctx, 0, "max mem size log 2");
    out_u8(ctx, DEFAULT_MEMORY_EXPORT, "export mem");
  }

  if (module->globals.types.size) {
    out_u8(ctx, WASM_SECTION_GLOBALS, "WASM_SECTION_GLOBALS");
    out_leb128(ctx, module->globals.types.size, "num globals");
    for (i = 0; i < module->globals.types.size; ++i) {
      WasmType global_type = module->globals.types.data[i];
      print_header(ctx, "global header", i);
      const uint8_t global_type_codes[WASM_NUM_V8_TYPES] = {-1, 4, 6, 8, 9};
      out_u32(ctx, 0, "global name offset");
      out_u8(ctx, global_type_codes[wasm_type_to_v8_type(global_type)],
             "global mem type");
      out_u8(ctx, 0, "export global");
    }
  }

  WasmFuncSignatureVector sigs = {};
  get_func_signatures(ctx, module, &sigs);
  if (sigs.size) {
    out_u8(ctx, WASM_SECTION_SIGNATURES, "WASM_SECTION_SIGNATURES");
    out_leb128(ctx, sigs.size, "num signatures");
    for (i = 0; i < sigs.size; ++i) {
      WasmFuncSignature* sig = &sigs.data[i];
      print_header(ctx, "signature", i);
      out_u8(ctx, sig->param_types.size, "num params");
      out_u8(ctx, wasm_type_to_v8_type(sig->result_type), "result_type");
      int j;
      for (j = 0; j < sig->param_types.size; ++j)
        out_u8(ctx, wasm_type_to_v8_type(sig->param_types.data[j]),
               "param type");
    }
  }

  size_t imports_offset;
  int num_funcs = module->imports.size + module->funcs.size;
  if (num_funcs) {
    out_u8(ctx, WASM_SECTION_FUNCTIONS, "WASM_SECTION_FUNCTIONS");
    out_leb128(ctx, num_funcs, "num functions");

    imports_offset = ctx->offset;
    for (i = 0; i < module->imports.size; ++i) {
      // WasmImport* import = module->imports.data[i];
      print_header(ctx, "import header", i);
      WasmFunctionFlags flags =
          WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_IMPORT;
      out_u8(ctx, flags, "import flags");
      out_u16(ctx, ctx->import_sig_indexes[i], "import signature index");
      out_u32(ctx, 0, "import name offset");
    }

    ctx->func_offsets =
        realloc(ctx->func_offsets, module->funcs.size * sizeof(size_t));
    for (i = 0; i < module->funcs.size; ++i) {
      WasmFunc* func = module->funcs.data[i];
      print_header(ctx, "function", i);
      ctx->func_offsets[i] = ctx->offset;
      int is_exported = wasm_func_is_exported(module, func);
      /* TODO(binji): remove? */
      int has_name = 1;  /* is_exported */
      int has_locals = func->locals.types.size > 0;
      uint8_t flags = 0;
      if (has_name)
        flags |= WASM_FUNCTION_FLAG_NAME;
      if (is_exported)
        flags |= WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_EXPORT;
      if (has_locals)
        flags |= WASM_FUNCTION_FLAG_LOCALS;
      out_u8(ctx, flags, "func flags");
      out_u16(ctx, ctx->func_sig_indexes[i], "func signature index");
      if (has_name)
        out_u32(ctx, 0, "func name offset");
      if (has_locals) {
        remap_locals(ctx, func);
        int num_locals[WASM_NUM_V8_TYPES] = {};
        int j;
        for (j = 0; j < func->locals.types.size; ++j)
          num_locals[wasm_type_to_v8_type(func->locals.types.data[j])]++;

        out_u16(ctx, num_locals[WASM_TYPE_V8_I32], "num local i32");
        out_u16(ctx, num_locals[WASM_TYPE_V8_I64], "num local i64");
        out_u16(ctx, num_locals[WASM_TYPE_V8_F32], "num local f32");
        out_u16(ctx, num_locals[WASM_TYPE_V8_F64], "num local f64");
      }

      size_t func_body_offset = ctx->offset;
      out_u16(ctx, 0, "func body size");
      write_func(ctx, module, func);
      int func_size = ctx->offset - func_body_offset - sizeof(uint16_t);
      out_u16_at(ctx, func_body_offset, func_size, "FIXUP func body size");
    }
  }

  if (module->table && module->table->size) {
    out_u8(ctx, WASM_SECTION_FUNCTION_TABLE, "WASM_SECTION_FUNCTION_TABLE");
    out_leb128(ctx, module->table->size, "num function table entries");
    for (i = 0; i < module->table->size; ++i) {
      int index = wasm_get_func_index_by_var(module, &module->table->data[i]);
      assert(index >= 0 && index < module->funcs.size);
      out_u16(ctx, index, "function table entry");
    }
  }

  out_u8(ctx, WASM_SECTION_END, "WASM_SECTION_END");

  /* output segment data */
  size_t offset;
  if (module->memory) {
    offset = segments_offset;
    for (i = 0; i < module->memory->segments.size; ++i) {
      print_header(ctx, "segment data", i);
      WasmSegment* segment = &module->memory->segments.data[i];
      out_u32_at(ctx, offset + SEGMENT_OFFSET_OFFSET, ctx->offset,
                 "FIXUP segment data offset");
      out_data(ctx, ctx->offset, segment->data, segment->size, "segment data");
      ctx->offset += segment->size;
      offset += SEGMENT_SIZE;
    }
  }

  /* output import names */
  offset = imports_offset;
  for (i = 0; i < module->imports.size; ++i) {
    print_header(ctx, "import", i);
    WasmImport* import = module->imports.data[i];
    out_u32_at(ctx, offset + IMPORT_NAME_OFFSET, ctx->offset,
               "FIXUP import name offset");
    out_str(ctx, import->name.start, import->name.length, "import name");
    offset += IMPORT_SIZE;
  }

  /* output exported func names */
  for (i = 0; i < module->exports.size; ++i) {
    print_header(ctx, "export", i);
    WasmExport* export = module->exports.data[i];
    int func_index = wasm_get_func_index_by_var(module, &export->var);
    assert(func_index >= 0 && func_index < module->funcs.size);
    offset = ctx->func_offsets[func_index];
    out_u32_at(ctx, offset + FUNC_NAME_OFFSET, ctx->offset,
               "FIXUP func name offset");
    out_str(ctx, export->name.start, export->name.length, "export name");
  }

  return result;
}

static WasmResult write_command(WasmWriteContext* ctx, WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      return write_module(ctx, &command->module);
#if 0
    case WASM_COMMAND_TYPE_INVOKE:
      return write_invoke(writer, &command->invoke, WASM_TYPE_VOID);
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      return write_invoke(writer, &command->assert_return.invoke,
                          command->assert_return.expected.type);
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      return write_invoke(writer, &command->assert_return_nan.invoke,
                          WASM_TYPE_F32 | WASM_TYPE_F64);
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      return write_invoke(writer, &command->assert_trap.invoke, WASM_TYPE_VOID);
#endif
    default:
      break;
  }
}

WasmResult wasm_write_binary(WasmBinaryWriter* writer, WasmScript* script) {
  WasmWriteContext ctx = {};
  ctx.writer = writer;
  WasmResult result = WASM_OK;
  int i;
  for (i = 0; i < script->commands.size; ++i)
    result |= write_command(&ctx, &script->commands.data[i]);
  return result;
}
