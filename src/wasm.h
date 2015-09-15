#ifndef WASM_H
#define WASM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

/* These values match the v8-native-prototype's type's values */
typedef enum WasmType {
  WASM_TYPE_VOID,
  WASM_TYPE_I32,
  WASM_TYPE_I64,
  WASM_TYPE_F32,
  WASM_TYPE_F64,
  WASM_NUM_TYPES,
} WasmType;

typedef enum WasmMemType {
  WASM_MEM_TYPE_I8,
  WASM_MEM_TYPE_I16,
  WASM_MEM_TYPE_I32,
  WASM_MEM_TYPE_I64,
  WASM_MEM_TYPE_F32,
  WASM_MEM_TYPE_F64,
} WasmMemType;

typedef enum WasmOpType {
  WASM_OP_NONE,
  WASM_OP_ASSERT_EQ,
  WASM_OP_ASSERT_INVALID,
  WASM_OP_BINARY,
  WASM_OP_BLOCK,
  WASM_OP_BREAK,
  WASM_OP_CALL,
  WASM_OP_CALL_INDIRECT,
  WASM_OP_COMPARE,
  WASM_OP_CONST,
  WASM_OP_CONVERT,
  WASM_OP_DESTRUCT,
  WASM_OP_EXPORT,
  WASM_OP_FUNC,
  WASM_OP_GET_LOCAL,
  WASM_OP_GLOBAL,
  WASM_OP_IF,
  WASM_OP_IMPORT,
  WASM_OP_INVOKE,
  WASM_OP_LABEL,
  WASM_OP_LOAD,
  WASM_OP_LOAD_GLOBAL,
  WASM_OP_LOCAL,
  WASM_OP_LOOP,
  WASM_OP_MEMORY,
  WASM_OP_MODULE,
  WASM_OP_NOP,
  WASM_OP_PARAM,
  WASM_OP_RESULT,
  WASM_OP_RETURN,
  WASM_OP_SET_LOCAL,
  WASM_OP_STORE,
  WASM_OP_STORE_GLOBAL,
  WASM_OP_SWITCH,
  WASM_OP_TABLE,
  WASM_OP_UNARY,
} WasmOpType;

#define OPCODES(V)             \
  V(NOP, 0x00)                 \
  V(IF, 0x01)                  \
  V(IF_THEN, 0x02)             \
  V(BLOCK, 0x03)               \
  V(SWITCH, 0x04)              \
  V(SWITCH_NF, 0x05)           \
  V(LOOP, 0x06)                \
  V(CONTINUE, 0x07)            \
  V(BREAK, 0x08)               \
  V(RETURN, 0x09)              \
  V(I8_CONST, 0x10)            \
  V(I32_CONST, 0x11)           \
  V(I64_CONST, 0x12)           \
  V(F64_CONST, 0x13)           \
  V(F32_CONST, 0x14)           \
  V(GET_LOCAL, 0x15)           \
  V(SET_LOCAL, 0x16)           \
  V(GET_GLOBAL, 0x17)          \
  V(SET_GLOBAL, 0x18)          \
  V(CALL, 0x19)                \
  V(CALL_INDIRECT, 0x1a)       \
  V(TERNARY, 0x1b)             \
  V(COMMA, 0x1c)               \
  V(I32_LOAD_I32, 0x20)        \
  V(I64_LOAD_I32, 0x21)        \
  V(F32_LOAD_I32, 0x22)        \
  V(F64_LOAD_I32, 0x23)        \
  V(I32_LOAD_I64, 0x24)        \
  V(I64_LOAD_I64, 0x25)        \
  V(F32_LOAD_I64, 0x26)        \
  V(F64_LOAD_I64, 0x27)        \
  V(I32_STORE_I32, 0x30)       \
  V(I64_STORE_I32, 0x31)       \
  V(F32_STORE_I32, 0x32)       \
  V(F64_STORE_I32, 0x33)       \
  V(I32_STORE_I64, 0x34)       \
  V(I64_STORE_I64, 0x35)       \
  V(F32_STORE_I64, 0x36)       \
  V(F64_STORE_I64, 0x37)       \
  V(I32_ADD, 0x40)             \
  V(I32_SUB, 0x41)             \
  V(I32_MUL, 0x42)             \
  V(I32_SDIV, 0x43)            \
  V(I32_UDIV, 0x44)            \
  V(I32_SREM, 0x45)            \
  V(I32_UREM, 0x46)            \
  V(I32_AND, 0x47)             \
  V(I32_OR, 0x48)              \
  V(I32_XOR, 0x49)             \
  V(I32_SHL, 0x4a)             \
  V(I32_SHR, 0x4b)             \
  V(I32_SAR, 0x4c)             \
  V(I32_EQ, 0x4d)              \
  V(I32_NE, 0x4e)              \
  V(I32_SLT, 0x4f)             \
  V(I32_SLE, 0x50)             \
  V(I32_ULT, 0x51)             \
  V(I32_ULE, 0x52)             \
  V(I32_SGT, 0x53)             \
  V(I32_SGE, 0x54)             \
  V(I32_UGT, 0x55)             \
  V(I32_UGE, 0x56)             \
  V(I32_CLZ, 0x57)             \
  V(I32_CTZ, 0x58)             \
  V(I32_POPCNT, 0x59)          \
  V(I32_NOT, 0x5a)             \
  V(I64_ADD, 0x5b)             \
  V(I64_SUB, 0x5c)             \
  V(I64_MUL, 0x5d)             \
  V(I64_SDIV, 0x5e)            \
  V(I64_UDIV, 0x5f)            \
  V(I64_SREM, 0x60)            \
  V(I64_UREM, 0x61)            \
  V(I64_AND, 0x62)             \
  V(I64_OR, 0x63)              \
  V(I64_XOR, 0x64)             \
  V(I64_SHL, 0x65)             \
  V(I64_SHR, 0x66)             \
  V(I64_SAR, 0x67)             \
  V(I64_EQ, 0x68)              \
  V(I64_NE, 0x69)              \
  V(I64_SLT, 0x6a)             \
  V(I64_SLE, 0x6b)             \
  V(I64_ULT, 0x6c)             \
  V(I64_ULE, 0x6d)             \
  V(I64_SGT, 0x6e)             \
  V(I64_SGE, 0x6f)             \
  V(I64_UGT, 0x70)             \
  V(I64_UGE, 0x71)             \
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
  V(F32_NEAREST, 0x81)         \
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
  V(F64_NEAREST, 0x95)         \
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
  V(F64_REINTERPRET_I64, 0xb3)

typedef enum WasmOpcode {
  WASM_OPCODE_INVALID = -1,
#define OPCODE(name, code) WASM_OPCODE_##name = code,
  OPCODES(OPCODE)
#undef OPCODE
} WasmOpcode;

typedef enum WasmTokenType {
  WASM_TOKEN_TYPE_EOF,
  WASM_TOKEN_TYPE_OPEN_PAREN,
  WASM_TOKEN_TYPE_CLOSE_PAREN,
  WASM_TOKEN_TYPE_ATOM,
  WASM_TOKEN_TYPE_STRING,
} WasmTokenType;

typedef struct WasmSourceLocation {
  const char* pos;
  int line;
  int col;
} WasmSourceLocation;

typedef struct WasmSourceRange {
  WasmSourceLocation start;
  WasmSourceLocation end;
} WasmSourceRange;

typedef struct WasmToken {
  WasmTokenType type;
  WasmSourceRange range;
} WasmToken;

#define DECLARE_VECTOR(name, type)                      \
  typedef struct type##Vector {                         \
    type* data;                                         \
    size_t size;                                        \
    size_t capacity;                                    \
  } type##Vector;                                       \
  EXTERN_C void wasm_destroy_##name##_vector(type##Vector* vec); \
  EXTERN_C type* wasm_append_##name(type##Vector* vec);

DECLARE_VECTOR(type, WasmType)

typedef struct WasmBinding {
  char* name;
  int index;
} WasmBinding;
DECLARE_VECTOR(binding, WasmBinding)

typedef struct WasmVariable {
  size_t offset;
  WasmType type;
  /* The v8-native-prototype stores locals in i32/i64/f32/f64 order, where all
   * variables of one type are grouped. index maps to this variable to its
   * correct location in that order */
  int index;
} WasmVariable;
DECLARE_VECTOR(variable, WasmVariable)

typedef struct WasmFunction {
  WasmTypeVector result_types;
  WasmVariableVector locals; /* Includes args, they're at the start */
  WasmBindingVector local_bindings;
  WasmBindingVector labels;
  size_t offset; /* offset in the output buffer (function bindings skip the
                    signature */
  int num_args;
  int depth;
} WasmFunction;
DECLARE_VECTOR(function, WasmFunction)

typedef struct WasmExport {
  char* name;
  int index;
} WasmExport;
DECLARE_VECTOR(export, WasmExport)

typedef struct WasmImport {
  char* module_name;
  char* func_name;
  WasmType result_type;
  WasmVariableVector args;
  size_t offset; /* offset of the import name in the output buffer */
} WasmImport;
DECLARE_VECTOR(import, WasmImport)

typedef struct Segment {
  size_t offset;
  size_t size;
  uint32_t address;
  WasmToken data;
} WasmSegment;
DECLARE_VECTOR(segment, WasmSegment)

typedef struct WasmModule {
  WasmFunctionVector functions;
  WasmBindingVector function_bindings;
  WasmVariableVector globals;
  WasmBindingVector global_bindings;
  WasmExportVector exports;
  WasmImportVector imports;
  WasmBindingVector import_bindings;
  WasmSegmentVector segments;
  uint32_t initial_memory_size;
  uint32_t max_memory_size;
} WasmModule;

#endif /* WASM_H */
