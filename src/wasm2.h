#ifndef WASM2_H_
#define WASM2_H_

#include <stdint.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define DECLARE_VECTOR(name, type)                               \
  typedef struct type##Vector {                                  \
    type* data;                                                  \
    size_t size;                                                 \
    size_t capacity;                                             \
  } type##Vector;                                                \
  EXTERN_C void wasm_destroy_##name##_vector(type##Vector* vec); \
  EXTERN_C type* wasm_append_##name(type##Vector* vec);          \
  EXTERN_C int wasm_extend_##name##s(type##Vector* dst, type##Vector* src);

typedef struct WasmStringSlice {
  const char* start;
  int length;
} WasmStringSlice;

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

typedef enum WasmSigned {
  WASM_SIGNED,
  WASM_UNSIGNED,
} WasmSigned;

typedef enum WasmUnaryOpType {
  WASM_UNARY_OP_TYPE_NOT,
  WASM_UNARY_OP_TYPE_CLZ,
  WASM_UNARY_OP_TYPE_CTZ,
  WASM_UNARY_OP_TYPE_POPCNT,
  WASM_UNARY_OP_TYPE_NEG,
  WASM_UNARY_OP_TYPE_ABS,
  WASM_UNARY_OP_TYPE_SQRT,
  WASM_UNARY_OP_TYPE_CEIL,
  WASM_UNARY_OP_TYPE_FLOOR,
  WASM_UNARY_OP_TYPE_TRUNC,
  WASM_UNARY_OP_TYPE_NEAREST,
} WasmUnaryOpType;

typedef enum WasmBinaryOpType {
  WASM_BINARY_OP_TYPE_ADD,
  WASM_BINARY_OP_TYPE_SUB,
  WASM_BINARY_OP_TYPE_MUL,
  WASM_BINARY_OP_TYPE_DIV,
  WASM_BINARY_OP_TYPE_REM,
  WASM_BINARY_OP_TYPE_AND,
  WASM_BINARY_OP_TYPE_OR,
  WASM_BINARY_OP_TYPE_XOR,
  WASM_BINARY_OP_TYPE_SHL,
  WASM_BINARY_OP_TYPE_SHR,
  WASM_BINARY_OP_TYPE_MIN,
  WASM_BINARY_OP_TYPE_MAX,
  WASM_BINARY_OP_TYPE_COPYSIGN,
} WasmBinaryOpType;

typedef enum WasmCompareOpType {
  WASM_COMPARE_OP_TYPE_EQ,
  WASM_COMPARE_OP_TYPE_NE,
  WASM_COMPARE_OP_TYPE_LT,
  WASM_COMPARE_OP_TYPE_LE,
  WASM_COMPARE_OP_TYPE_GT,
  WASM_COMPARE_OP_TYPE_GE,
} WasmCompareOpType;

typedef struct WasmUnaryOp {
  WasmType type;
  WasmUnaryOpType op_type;
} WasmUnaryOp;

typedef struct WasmBinaryOp {
  WasmType type;
  WasmBinaryOpType op_type;
  WasmSigned sign;
} WasmBinaryOp;

typedef struct WasmCompareOp {
  WasmType type;
  WasmCompareOpType op_type;
  WasmSigned sign;
} WasmCompareOp;

typedef struct WasmConvertOp {
  WasmType type;
  WasmType type2;
  WasmSigned sign;
} WasmConvertOp;

typedef struct WasmCastOp {
  WasmType type;
  WasmType type2;
} WasmCastOp;

typedef struct WasmMemOp {
  WasmType type;
  WasmMemSize size;
  WasmSigned sign;
} WasmMemOp;

typedef enum WasmVarType {
  WASM_VAR_TYPE_INDEX,
  WASM_VAR_TYPE_NAME,
} WasmVarType;

typedef struct WasmVar {
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
  WasmLabel label;
  WasmExprPtrVector exprs;
} WasmCase;
DECLARE_VECTOR(case, WasmCase);

typedef struct WasmConst {
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

typedef struct WasmExpr WasmExpr;
struct WasmExpr {
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

typedef struct WasmBinding {
  WasmStringSlice name;
  int index;
} WasmBinding;
DECLARE_VECTOR(binding, WasmBinding);

typedef struct WasmTypeBindings {
  WasmTypeVector types;
  WasmBindingVector bindings;
} WasmTypeBindings;

typedef struct WasmFunc {
  WasmStringSlice name;
  WasmVar type_var;
  WasmTypeBindings params;
  WasmType result_type;
  WasmTypeBindings locals;
  WasmExprPtrVector exprs;
} WasmFunc;

typedef struct WasmSegment {
  uint32_t addr;
  void* data;
  size_t size;
} WasmSegment;
DECLARE_VECTOR(segment, WasmSegment);

typedef struct WasmMemory {
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

typedef enum WasmImportType {
  WASM_IMPORT_HAS_TYPE,
  WASM_IMPORT_HAS_FUNC_SIGNATURE,
} WasmImportType;

typedef struct WasmImport {
  WasmImportType import_type;
  WasmStringSlice name;
  WasmStringSlice module_name;
  WasmStringSlice func_name;
  WasmVar type_var;
  WasmFuncSignature func_sig;
} WasmImport;

typedef struct WasmExport {
  WasmStringSlice name;
  WasmVar var;
} WasmExport;

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
  WasmModuleFieldVector fields;
} WasmModule;

typedef enum WasmCommandType {
  WASM_COMMAND_TYPE_MODULE,
  WASM_COMMAND_TYPE_INVOKE,
  WASM_COMMAND_TYPE_ASSERT_INVALID,
  WASM_COMMAND_TYPE_ASSERT_RETURN,
  WASM_COMMAND_TYPE_ASSERT_RETURN_NAN,
  WASM_COMMAND_TYPE_ASSERT_RETURN_TRAP,
} WasmCommandType;

typedef struct WasmCommandInvoke {
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

typedef union WasmToken {
  /* terminals */
  WasmStringSlice text;
  WasmType type;
  WasmUnaryOp unary;
  WasmBinaryOp binary;
  WasmCompareOp compare;
  WasmConvertOp convert;
  WasmCastOp cast;
  WasmMemOp mem;

  /* non-terminals */
  uint32_t u32;
  WasmTypeVector types;
  WasmVar var;
  WasmVarVector vars;
  WasmExprPtr expr;
  WasmExprPtrVector exprs;
  WasmTarget target;
  WasmTargetVector targets;
  WasmCase case_;
  WasmCaseVector cases;
  WasmTypeBindings type_bindings;
  WasmFunc func;
  WasmSegment segment;
  WasmSegmentVector segments;
  WasmMemory memory;
  WasmFuncSignature func_sig;
  WasmFuncType func_type;
  WasmImport import;
  WasmExport export;
  WasmModuleFieldVector module_fields;
  WasmModule module;
  WasmConst const_;
  WasmConstVector consts;
  WasmCommand command;
  WasmCommandVector commands;
  WasmScript script;
} WasmToken;

typedef struct WasmLocation {
  const char* filename;
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} WasmLocation;

#define YYSTYPE WasmToken
#define YYLTYPE WasmLocation

typedef void* WasmScanner;

typedef struct WasmParser {
  WasmScanner scanner;
  WasmScript script;
  int errors;
} WasmParser;

int yylex(WasmToken*, WasmLocation*, WasmScanner, WasmParser*);
void yyerror(WasmLocation*, WasmScanner, WasmParser*, const char*, ...);
int yyparse(WasmScanner scanner, WasmParser* parser);

WasmScanner new_scanner(const char* filename);
void free_scanner(WasmScanner scanner);

#endif /* WASM2_H_ */
