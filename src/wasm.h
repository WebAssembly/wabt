#ifndef WASM_H_
#define WASM_H_

#include <stdint.h>
#include <stdlib.h>

#include "wasm-common.h"
#include "wasm-vector.h"
#include "wasm-writer.h"

typedef struct WasmStringSlice {
  const char* start;
  int length;
} WasmStringSlice;

typedef struct WasmLocation {
  const char* filename;
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} WasmLocation;

typedef enum WasmType {
  WASM_TYPE_VOID = 0,
  WASM_TYPE_I32 = 1,
  WASM_TYPE_I64 = 2,
  WASM_TYPE_F32 = 4,
  WASM_TYPE_F64 = 8,
  WASM_TYPE_ALL = 15,
} WasmType;
DECLARE_VECTOR(type, WasmType)

typedef enum WasmMemSize {
  WASM_MEM_SIZE_8 = 8,
  WASM_MEM_SIZE_16 = 16,
  WASM_MEM_SIZE_32 = 32,
  WASM_MEM_SIZE_64 = 64,
} WasmMemSize;

enum { WASM_USE_NATURAL_ALIGNMENT = 0xFFFFFFFF };

typedef enum WasmUnaryOpType {
  WASM_UNARY_OP_TYPE_F32_ABS,
  WASM_UNARY_OP_TYPE_F32_CEIL,
  WASM_UNARY_OP_TYPE_F32_FLOOR,
  WASM_UNARY_OP_TYPE_F32_NEAREST,
  WASM_UNARY_OP_TYPE_F32_NEG,
  WASM_UNARY_OP_TYPE_F32_SQRT,
  WASM_UNARY_OP_TYPE_F32_TRUNC,
  WASM_UNARY_OP_TYPE_F64_ABS,
  WASM_UNARY_OP_TYPE_F64_CEIL,
  WASM_UNARY_OP_TYPE_F64_FLOOR,
  WASM_UNARY_OP_TYPE_F64_NEAREST,
  WASM_UNARY_OP_TYPE_F64_NEG,
  WASM_UNARY_OP_TYPE_F64_SQRT,
  WASM_UNARY_OP_TYPE_F64_TRUNC,
  WASM_UNARY_OP_TYPE_I32_CLZ,
  WASM_UNARY_OP_TYPE_I32_CTZ,
  WASM_UNARY_OP_TYPE_I32_NOT,
  WASM_UNARY_OP_TYPE_I32_POPCNT,
  WASM_UNARY_OP_TYPE_I64_CLZ,
  WASM_UNARY_OP_TYPE_I64_CTZ,
  WASM_UNARY_OP_TYPE_I64_POPCNT,
  WASM_NUM_UNARY_OP_TYPES,
} WasmUnaryOpType;

typedef enum WasmBinaryOpType {
  WASM_BINARY_OP_TYPE_F32_ADD,
  WASM_BINARY_OP_TYPE_F32_COPYSIGN,
  WASM_BINARY_OP_TYPE_F32_DIV,
  WASM_BINARY_OP_TYPE_F32_MAX,
  WASM_BINARY_OP_TYPE_F32_MIN,
  WASM_BINARY_OP_TYPE_F32_MUL,
  WASM_BINARY_OP_TYPE_F32_SUB,
  WASM_BINARY_OP_TYPE_F64_ADD,
  WASM_BINARY_OP_TYPE_F64_COPYSIGN,
  WASM_BINARY_OP_TYPE_F64_DIV,
  WASM_BINARY_OP_TYPE_F64_MAX,
  WASM_BINARY_OP_TYPE_F64_MIN,
  WASM_BINARY_OP_TYPE_F64_MUL,
  WASM_BINARY_OP_TYPE_F64_SUB,
  WASM_BINARY_OP_TYPE_I32_ADD,
  WASM_BINARY_OP_TYPE_I32_AND,
  WASM_BINARY_OP_TYPE_I32_DIV_S,
  WASM_BINARY_OP_TYPE_I32_DIV_U,
  WASM_BINARY_OP_TYPE_I32_MUL,
  WASM_BINARY_OP_TYPE_I32_OR,
  WASM_BINARY_OP_TYPE_I32_REM_S,
  WASM_BINARY_OP_TYPE_I32_REM_U,
  WASM_BINARY_OP_TYPE_I32_SHL,
  WASM_BINARY_OP_TYPE_I32_SHR_S,
  WASM_BINARY_OP_TYPE_I32_SHR_U,
  WASM_BINARY_OP_TYPE_I32_SUB,
  WASM_BINARY_OP_TYPE_I32_XOR,
  WASM_BINARY_OP_TYPE_I64_ADD,
  WASM_BINARY_OP_TYPE_I64_AND,
  WASM_BINARY_OP_TYPE_I64_DIV_S,
  WASM_BINARY_OP_TYPE_I64_DIV_U,
  WASM_BINARY_OP_TYPE_I64_MUL,
  WASM_BINARY_OP_TYPE_I64_OR,
  WASM_BINARY_OP_TYPE_I64_REM_S,
  WASM_BINARY_OP_TYPE_I64_REM_U,
  WASM_BINARY_OP_TYPE_I64_SHL,
  WASM_BINARY_OP_TYPE_I64_SHR_S,
  WASM_BINARY_OP_TYPE_I64_SHR_U,
  WASM_BINARY_OP_TYPE_I64_SUB,
  WASM_BINARY_OP_TYPE_I64_XOR,
  WASM_NUM_BINARY_OP_TYPES,
} WasmBinaryOpType;

typedef enum WasmCompareOpType {
  WASM_COMPARE_OP_TYPE_F32_EQ,
  WASM_COMPARE_OP_TYPE_F32_GE,
  WASM_COMPARE_OP_TYPE_F32_GT,
  WASM_COMPARE_OP_TYPE_F32_LE,
  WASM_COMPARE_OP_TYPE_F32_LT,
  WASM_COMPARE_OP_TYPE_F32_NE,
  WASM_COMPARE_OP_TYPE_F64_EQ,
  WASM_COMPARE_OP_TYPE_F64_GE,
  WASM_COMPARE_OP_TYPE_F64_GT,
  WASM_COMPARE_OP_TYPE_F64_LE,
  WASM_COMPARE_OP_TYPE_F64_LT,
  WASM_COMPARE_OP_TYPE_F64_NE,
  WASM_COMPARE_OP_TYPE_I32_EQ,
  WASM_COMPARE_OP_TYPE_I32_GE_S,
  WASM_COMPARE_OP_TYPE_I32_GE_U,
  WASM_COMPARE_OP_TYPE_I32_GT_S,
  WASM_COMPARE_OP_TYPE_I32_GT_U,
  WASM_COMPARE_OP_TYPE_I32_LE_S,
  WASM_COMPARE_OP_TYPE_I32_LE_U,
  WASM_COMPARE_OP_TYPE_I32_LT_S,
  WASM_COMPARE_OP_TYPE_I32_LT_U,
  WASM_COMPARE_OP_TYPE_I32_NE,
  WASM_COMPARE_OP_TYPE_I64_EQ,
  WASM_COMPARE_OP_TYPE_I64_GE_S,
  WASM_COMPARE_OP_TYPE_I64_GE_U,
  WASM_COMPARE_OP_TYPE_I64_GT_S,
  WASM_COMPARE_OP_TYPE_I64_GT_U,
  WASM_COMPARE_OP_TYPE_I64_LE_S,
  WASM_COMPARE_OP_TYPE_I64_LE_U,
  WASM_COMPARE_OP_TYPE_I64_LT_S,
  WASM_COMPARE_OP_TYPE_I64_LT_U,
  WASM_COMPARE_OP_TYPE_I64_NE,
  WASM_NUM_COMPARE_OP_TYPES,
} WasmCompareOpType;

typedef enum WasmMemOpType {
  WASM_MEM_OP_TYPE_F32_LOAD,
  WASM_MEM_OP_TYPE_F32_STORE,
  WASM_MEM_OP_TYPE_F64_LOAD,
  WASM_MEM_OP_TYPE_F64_STORE,
  WASM_MEM_OP_TYPE_I32_LOAD,
  WASM_MEM_OP_TYPE_I32_LOAD8_S,
  WASM_MEM_OP_TYPE_I32_LOAD8_U,
  WASM_MEM_OP_TYPE_I32_LOAD16_S,
  WASM_MEM_OP_TYPE_I32_LOAD16_U,
  WASM_MEM_OP_TYPE_I32_STORE,
  WASM_MEM_OP_TYPE_I32_STORE8,
  WASM_MEM_OP_TYPE_I32_STORE16,
  WASM_MEM_OP_TYPE_I64_LOAD,
  WASM_MEM_OP_TYPE_I64_LOAD8_S,
  WASM_MEM_OP_TYPE_I64_LOAD8_U,
  WASM_MEM_OP_TYPE_I64_LOAD16_S,
  WASM_MEM_OP_TYPE_I64_LOAD16_U,
  WASM_MEM_OP_TYPE_I64_LOAD32_S,
  WASM_MEM_OP_TYPE_I64_LOAD32_U,
  WASM_MEM_OP_TYPE_I64_STORE,
  WASM_MEM_OP_TYPE_I64_STORE8,
  WASM_MEM_OP_TYPE_I64_STORE16,
  WASM_MEM_OP_TYPE_I64_STORE32,
  WASM_NUM_MEM_OP_TYPES,
} WasmMemOpType;

typedef enum WasmConvertOpType {
  WASM_CONVERT_OP_TYPE_F32_CONVERT_S_I32,
  WASM_CONVERT_OP_TYPE_F32_CONVERT_S_I64,
  WASM_CONVERT_OP_TYPE_F32_CONVERT_U_I32,
  WASM_CONVERT_OP_TYPE_F32_CONVERT_U_I64,
  WASM_CONVERT_OP_TYPE_F32_DEMOTE_F64,
  WASM_CONVERT_OP_TYPE_F64_CONVERT_S_I32,
  WASM_CONVERT_OP_TYPE_F64_CONVERT_S_I64,
  WASM_CONVERT_OP_TYPE_F64_CONVERT_U_I32,
  WASM_CONVERT_OP_TYPE_F64_CONVERT_U_I64,
  WASM_CONVERT_OP_TYPE_F64_PROMOTE_F32,
  WASM_CONVERT_OP_TYPE_I32_TRUNC_S_F32,
  WASM_CONVERT_OP_TYPE_I32_TRUNC_S_F64,
  WASM_CONVERT_OP_TYPE_I32_TRUNC_U_F32,
  WASM_CONVERT_OP_TYPE_I32_TRUNC_U_F64,
  WASM_CONVERT_OP_TYPE_I32_WRAP_I64,
  WASM_CONVERT_OP_TYPE_I64_EXTEND_S_I32,
  WASM_CONVERT_OP_TYPE_I64_EXTEND_U_I32,
  WASM_CONVERT_OP_TYPE_I64_TRUNC_S_F32,
  WASM_CONVERT_OP_TYPE_I64_TRUNC_S_F64,
  WASM_CONVERT_OP_TYPE_I64_TRUNC_U_F32,
  WASM_CONVERT_OP_TYPE_I64_TRUNC_U_F64,
  WASM_NUM_CONVERT_OP_TYPES,
} WasmConvertOpType;

typedef enum WasmCastOpType {
  WASM_CAST_OP_TYPE_F32_REINTERPRET_I32,
  WASM_CAST_OP_TYPE_F64_REINTERPRET_I64,
  WASM_CAST_OP_TYPE_I32_REINTERPRET_F32,
  WASM_CAST_OP_TYPE_I64_REINTERPRET_F64,
  WASM_NUM_CAST_OP_TYPES,
} WasmCastOpType;

typedef struct WasmUnaryOp {
  WasmType type;
  WasmUnaryOpType op_type;
} WasmUnaryOp;

typedef struct WasmBinaryOp {
  WasmType type;
  WasmBinaryOpType op_type;
} WasmBinaryOp;

typedef struct WasmCompareOp {
  WasmType type;
  WasmCompareOpType op_type;
} WasmCompareOp;

typedef struct WasmConvertOp {
  WasmType type;
  WasmConvertOpType op_type;
  WasmType type2;
} WasmConvertOp;

typedef struct WasmCastOp {
  WasmType type;
  WasmCastOpType op_type;
  WasmType type2;
} WasmCastOp;

typedef struct WasmMemOp {
  WasmType type;
  WasmMemOpType op_type;
  WasmMemSize size;
} WasmMemOp;

typedef enum WasmVarType {
  WASM_VAR_TYPE_INDEX,
  WASM_VAR_TYPE_NAME,
} WasmVarType;

typedef struct WasmVar {
  WasmLocation loc;
  WasmVarType type;
  union {
    int index;
    WasmStringSlice name;
  };
} WasmVar;
DECLARE_VECTOR(var, WasmVar);

typedef enum WasmTargetType {
  WASM_TARGET_TYPE_CASE,
  WASM_TARGET_TYPE_BR,
} WasmTargetType;

typedef struct WasmTarget {
  WasmTargetType type;
  WasmVar var;
} WasmTarget;
DECLARE_VECTOR(target, WasmTarget);

typedef WasmStringSlice WasmLabel;

typedef struct WasmExpr* WasmExprPtr;
DECLARE_VECTOR(expr_ptr, WasmExprPtr);

typedef struct WasmCase {
  WasmLocation loc;
  WasmLabel label;
  WasmExprPtrVector exprs;
} WasmCase;
DECLARE_VECTOR(case, WasmCase);

typedef struct WasmConst {
  WasmLocation loc;
  WasmType type;
  union {
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
  };
} WasmConst;
DECLARE_VECTOR(const, WasmConst);

typedef enum WasmExprType {
  WASM_EXPR_TYPE_BINARY,
  WASM_EXPR_TYPE_BLOCK,
  WASM_EXPR_TYPE_BR,
  WASM_EXPR_TYPE_BR_IF,
  WASM_EXPR_TYPE_CALL,
  WASM_EXPR_TYPE_CALL_IMPORT,
  WASM_EXPR_TYPE_CALL_INDIRECT,
  WASM_EXPR_TYPE_CAST,
  WASM_EXPR_TYPE_COMPARE,
  WASM_EXPR_TYPE_CONST,
  WASM_EXPR_TYPE_CONVERT,
  WASM_EXPR_TYPE_GET_LOCAL,
  WASM_EXPR_TYPE_GROW_MEMORY,
  WASM_EXPR_TYPE_HAS_FEATURE,
  WASM_EXPR_TYPE_IF,
  WASM_EXPR_TYPE_IF_ELSE,
  WASM_EXPR_TYPE_LABEL,
  WASM_EXPR_TYPE_LOAD,
  WASM_EXPR_TYPE_LOAD_EXTEND,
  WASM_EXPR_TYPE_LOAD_GLOBAL,
  WASM_EXPR_TYPE_LOOP,
  WASM_EXPR_TYPE_MEMORY_SIZE,
  WASM_EXPR_TYPE_NOP,
  WASM_EXPR_TYPE_PAGE_SIZE,
  WASM_EXPR_TYPE_RETURN,
  WASM_EXPR_TYPE_SELECT,
  WASM_EXPR_TYPE_SET_LOCAL,
  WASM_EXPR_TYPE_STORE,
  WASM_EXPR_TYPE_STORE_GLOBAL,
  WASM_EXPR_TYPE_STORE_WRAP,
  WASM_EXPR_TYPE_TABLESWITCH,
  WASM_EXPR_TYPE_UNARY,
  WASM_EXPR_TYPE_UNREACHABLE,
} WasmExprType;

typedef struct WasmBinding {
  WasmLocation loc;
  WasmStringSlice name;
  int index;
} WasmBinding;
DECLARE_VECTOR(binding, WasmBinding);

typedef struct WasmExpr WasmExpr;
struct WasmExpr {
  WasmLocation loc;
  WasmExprType type;
  union {
    struct { WasmLabel label; WasmExprPtrVector exprs; } block;
    struct { WasmExprPtr cond, true_, false_; } if_else;
    struct { WasmExprPtr cond, true_; } if_;
    struct { WasmVar var; WasmExprPtr cond; } br_if;
    struct { WasmLabel inner, outer; WasmExprPtrVector exprs; } loop;
    struct { WasmLabel label; WasmExprPtr expr; } label;
    struct { WasmVar var; WasmExprPtr expr; } br;
    struct { WasmExprPtr expr; } return_;
    struct {
      WasmLabel label;
      WasmExprPtr expr;
      WasmTargetVector targets;
      WasmTarget default_target;
      WasmBindingVector case_bindings;
      WasmCaseVector cases;
    } tableswitch;
    struct { WasmVar var; WasmExprPtrVector args; } call;
    struct {
      WasmVar var;
      WasmExprPtr expr;
      WasmExprPtrVector args;
    } call_indirect;
    struct { WasmVar var; } get_local;
    struct { WasmVar var; WasmExprPtr expr; } set_local;
    struct { WasmMemOp op; uint32_t offset, align; WasmExprPtr addr; } load;
    struct {
      WasmMemOp op;
      uint32_t offset, align;
      WasmExprPtr addr, value;
    } store;
    WasmConst const_;
    struct { WasmUnaryOp op; WasmExprPtr expr; } unary;
    struct { WasmBinaryOp op; WasmExprPtr left, right; } binary;
    struct { WasmType type; WasmExprPtr cond, true_, false_; } select;
    struct { WasmCompareOp op; WasmExprPtr left, right; } compare;
    struct { WasmConvertOp op; WasmExprPtr expr; } convert;
    struct { WasmCastOp op; WasmExprPtr expr; } cast;
    struct { WasmExprPtr expr; } grow_memory;
    struct { WasmStringSlice text; } has_feature;
    struct { WasmVar var; } load_global;
    struct { WasmVar var; WasmExprPtr expr; } store_global;
  };
};

typedef struct WasmTypeBindings {
  WasmTypeVector types;
  WasmBindingVector bindings;
} WasmTypeBindings;

enum {
  WASM_FUNC_FLAG_HAS_FUNC_TYPE = 1,
  WASM_FUNC_FLAG_HAS_SIGNATURE = 2,
};
typedef uint32_t WasmFuncFlags;

typedef struct WasmFunc {
  WasmLocation loc;
  WasmFuncFlags flags;
  WasmStringSlice name;
  WasmVar type_var;
  WasmTypeBindings params;
  WasmType result_type;
  WasmTypeBindings locals;
  WasmExprPtrVector exprs;

  /* combined from params and locals; the binding names share memory with the
   originals */
  WasmTypeBindings params_and_locals;
} WasmFunc;
typedef WasmFunc* WasmFuncPtr;
DECLARE_VECTOR(func_ptr, WasmFuncPtr);

typedef struct WasmSegment {
  WasmLocation loc;
  uint32_t addr;
  void* data;
  size_t size;
} WasmSegment;
DECLARE_VECTOR(segment, WasmSegment);

typedef struct WasmMemory {
  WasmLocation loc;
  uint32_t initial_size;
  uint32_t max_size;
  WasmSegmentVector segments;
} WasmMemory;

typedef struct WasmFuncSignature {
  WasmType result_type;
  WasmTypeVector param_types;
} WasmFuncSignature;

typedef struct WasmFuncType {
  WasmStringSlice name;
  WasmFuncSignature sig;
} WasmFuncType;
typedef WasmFuncType* WasmFuncTypePtr;
DECLARE_VECTOR(func_type_ptr, WasmFuncTypePtr);

typedef enum WasmImportType {
  WASM_IMPORT_HAS_TYPE,
  WASM_IMPORT_HAS_FUNC_SIGNATURE,
} WasmImportType;

typedef struct WasmImport {
  WasmLocation loc;
  WasmImportType import_type;
  WasmStringSlice name;
  WasmStringSlice module_name;
  WasmStringSlice func_name;
  WasmVar type_var;
  WasmFuncSignature func_sig;
} WasmImport;
typedef WasmImport* WasmImportPtr;
DECLARE_VECTOR(import_ptr, WasmImportPtr);

typedef struct WasmExport {
  WasmStringSlice name;
  WasmVar var;
} WasmExport;
typedef WasmExport* WasmExportPtr;
DECLARE_VECTOR(export_ptr, WasmExportPtr);

typedef enum WasmModuleFieldType {
  WASM_MODULE_FIELD_TYPE_FUNC,
  WASM_MODULE_FIELD_TYPE_IMPORT,
  WASM_MODULE_FIELD_TYPE_EXPORT,
  WASM_MODULE_FIELD_TYPE_TABLE,
  WASM_MODULE_FIELD_TYPE_FUNC_TYPE,
  WASM_MODULE_FIELD_TYPE_MEMORY,
  WASM_MODULE_FIELD_TYPE_GLOBAL,
} WasmModuleFieldType;

typedef struct WasmModuleField {
  WasmLocation loc;
  WasmModuleFieldType type;
  union {
    WasmFunc func;
    WasmImport import;
    WasmExport export;
    WasmVarVector table;
    WasmFuncType func_type;
    WasmMemory memory;
    WasmTypeBindings global;
  };
} WasmModuleField;
DECLARE_VECTOR(module_field, WasmModuleField);

typedef struct WasmModule {
  WasmLocation loc;
  WasmModuleFieldVector fields;

  /* cached for convenience */
  WasmFuncPtrVector funcs;
  WasmImportPtrVector imports;
  WasmExportPtrVector exports;
  WasmFuncTypePtrVector func_types;
  WasmTypeBindings globals;
  WasmVarVector* table;
  WasmMemory* memory;

  WasmBindingVector func_bindings;
  WasmBindingVector import_bindings;
  WasmBindingVector export_bindings;
  WasmBindingVector func_type_bindings;
} WasmModule;

typedef enum WasmCommandType {
  WASM_COMMAND_TYPE_MODULE,
  WASM_COMMAND_TYPE_INVOKE,
  WASM_COMMAND_TYPE_ASSERT_INVALID,
  WASM_COMMAND_TYPE_ASSERT_RETURN,
  WASM_COMMAND_TYPE_ASSERT_RETURN_NAN,
  WASM_COMMAND_TYPE_ASSERT_TRAP,
} WasmCommandType;

typedef struct WasmCommandInvoke {
  WasmLocation loc;
  WasmStringSlice name;
  WasmConstVector args;
} WasmCommandInvoke;

typedef struct WasmCommand {
  WasmCommandType type;
  union {
    WasmModule module;
    WasmCommandInvoke invoke;
    struct { WasmCommandInvoke invoke; WasmConst expected; } assert_return;
    struct { WasmCommandInvoke invoke; } assert_return_nan;
    struct { WasmCommandInvoke invoke; WasmStringSlice text; } assert_trap;
    struct { WasmModule module; WasmStringSlice text; } assert_invalid;
  };
} WasmCommand;
DECLARE_VECTOR(command, WasmCommand);

typedef struct WasmScript {
  WasmCommandVector commands;
} WasmScript;

typedef void* WasmScanner;

typedef struct WasmParser {
  WasmScanner scanner;
  WasmScript script;
  int errors;
} WasmParser;

typedef struct WasmWriteBinaryOptions {
  int spec;
  int log_writes;
} WasmWriteBinaryOptions;

WasmScanner wasm_new_scanner(const char* filename);
void wasm_free_scanner(WasmScanner scanner);
WasmResult wasm_check_script(WasmScript*);
WasmResult wasm_write_binary(WasmWriter*, WasmScript*, WasmWriteBinaryOptions*);

int wasm_get_index_from_var(WasmBindingVector* bindings, WasmVar* var);
int wasm_get_func_index_by_var(WasmModule* module, WasmVar* var);
int wasm_get_func_type_index_by_var(WasmModule* module, WasmVar* var);
int wasm_get_global_index_by_var(WasmModule* module, WasmVar* var);
int wasm_get_import_index_by_var(WasmModule* module, WasmVar* var);
int wasm_get_local_index_by_var(WasmFunc* func, WasmVar* var);

WasmFuncPtr wasm_get_func_by_var(WasmModule* module, WasmVar* var);
WasmFuncTypePtr wasm_get_func_type_by_var(WasmModule* module, WasmVar* var);
WasmImportPtr wasm_get_import_by_var(WasmModule* module, WasmVar* var);
WasmExportPtr wasm_get_export_by_name(WasmModule* module,
                                      WasmStringSlice* name);

void wasm_extend_type_bindings(WasmTypeBindings* dst, WasmTypeBindings* src);

int wasm_func_is_exported(WasmModule* module, WasmFunc* func);

#endif /* WASM_H_ */
