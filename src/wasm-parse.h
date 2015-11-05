#ifndef WASM_PARSE_H
#define WASM_PARSE_H

#include "wasm.h"

typedef union WasmNumber {
  uint32_t i32;
  uint64_t i64;
  float f32;
  double f64;
} WasmNumber;

typedef uintptr_t WasmParserCookie;

typedef struct WasmParserCallbackInfo {
  WasmSourceLocation loc;
  WasmModule* module; /* may be NULL */
  WasmFunction* function; /* may be NULL */
  /* Saved and restored for matching before/after callback pairs */
  WasmParserCookie cookie;
  void* user_data;
} WasmParserCallbackInfo;

typedef struct WasmParserCallbacks {
  void* user_data;
  void (*error)(WasmParserCallbackInfo* info, const char* msg);
  void (*before_module)(WasmParserCallbackInfo* info);
  void (*after_module)(WasmParserCallbackInfo* info);
  /* This will not be called if the module fails to parse. */
  void (*before_module_destroy)(WasmParserCallbackInfo* info);
  void (*before_function)(WasmParserCallbackInfo* info);
  void (*after_function)(WasmParserCallbackInfo* info, int num_exprs);
  void (*before_export)(WasmParserCallbackInfo* info);
  void (*after_export)(WasmParserCallbackInfo* info, const char* exported_name);

  void (*before_binary)(WasmParserCallbackInfo* info, enum WasmOpcode opcode);
  void (*before_block)(WasmParserCallbackInfo* info, int with_label);
  void (*after_block)(WasmParserCallbackInfo* info,
                      WasmType type,
                      int num_exprs);
  void (*before_break)(WasmParserCallbackInfo* info,
                       int with_expr,
                       int label_depth);
  void (*after_break)(WasmParserCallbackInfo* info);
  void (*before_br_if)(WasmParserCallbackInfo* info, int label_depth);
  void (*after_br_if)(WasmParserCallbackInfo* info);
  void (*before_call)(WasmParserCallbackInfo* info, int function_index);
  void (*before_call_import)(WasmParserCallbackInfo* info, int import_index);
  void (*before_call_indirect)(WasmParserCallbackInfo* info,
                               int signature_index);
  void (*before_compare)(WasmParserCallbackInfo* info, enum WasmOpcode opcode);
  void (*after_const)(WasmParserCallbackInfo* info,
                      enum WasmOpcode opcode,
                      WasmType type,
                      WasmNumber value);
  void (*before_convert)(WasmParserCallbackInfo* info, enum WasmOpcode opcode);
  void (*before_label)(WasmParserCallbackInfo* info);
  void (*after_label)(WasmParserCallbackInfo* info, WasmType type);
  void (*after_get_local)(WasmParserCallbackInfo* info, int index);
  void (*before_loop)(WasmParserCallbackInfo* info,
                      int with_inner_label,
                      int with_outer_label);
  void (*after_loop)(WasmParserCallbackInfo* info, int num_exprs);
  void (*before_if)(WasmParserCallbackInfo* info);
  void (*after_if)(WasmParserCallbackInfo* info, WasmType type, int with_else);
  void (*before_load)(WasmParserCallbackInfo* info,
                      enum WasmOpcode opcode,
                      WasmMemType mem_type,
                      uint32_t alignment,
                      uint64_t offset,
                      int is_signed);
  void (*after_load_global)(WasmParserCallbackInfo* info, int index);
  void (*after_memory_size)(WasmParserCallbackInfo* info);
  void (*after_nop)(WasmParserCallbackInfo* info);
  void (*after_page_size)(WasmParserCallbackInfo* info);
  void (*before_grow_memory)(WasmParserCallbackInfo* info);
  void (*before_return)(WasmParserCallbackInfo* info);
  void (*after_return)(WasmParserCallbackInfo* info, WasmType type);
  void (*before_select)(WasmParserCallbackInfo* info);
  void (*before_set_local)(WasmParserCallbackInfo* info, int index);
  void (*before_store)(WasmParserCallbackInfo* info,
                       enum WasmOpcode opcode,
                       WasmMemType mem_type,
                       uint32_t alignment,
                       uint64_t offset);

  void (*before_store_global)(WasmParserCallbackInfo* info, int index);
  void (*before_switch)(WasmParserCallbackInfo* info, int with_label);
  void (*after_switch)(WasmParserCallbackInfo* info);
  void (*before_switch_case)(WasmParserCallbackInfo* info, WasmNumber number);
  void (*after_switch_case)(WasmParserCallbackInfo* info,
                            int num_exprs,
                            int with_fallthrough);
  void (*before_switch_default)(WasmParserCallbackInfo* info);
  void (*after_switch_default)(WasmParserCallbackInfo* info);
  void (*before_unary)(WasmParserCallbackInfo* info, enum WasmOpcode opcode);

  /* used in spec repo tests */
  void (*before_assert_return)(WasmParserCallbackInfo* info);
  void (*after_assert_return)(WasmParserCallbackInfo* info, WasmType type);
  void (*before_assert_return_nan)(WasmParserCallbackInfo* info);
  void (*after_assert_return_nan)(WasmParserCallbackInfo* info, WasmType type);
  void (*before_assert_trap)(WasmParserCallbackInfo* info);
  void (*after_assert_trap)(WasmParserCallbackInfo* info);
  void (*before_invoke)(WasmParserCallbackInfo* info,
                        const char* invoke_name,
                        int invoke_function_index);
  void (*after_invoke)(WasmParserCallbackInfo* info);

  /* called with the error from the module parsed inside of assert_invalid */
  void (*assert_invalid_error)(WasmParserCallbackInfo* info, const char* msg);
} WasmParserCallbacks;

typedef struct WasmParserOptions {
  int br_if;
} WasmParserOptions;

EXTERN_C int wasm_parse_module(WasmSource* source,
                               WasmParserCallbacks* parser,
                               WasmParserOptions* options);
EXTERN_C int wasm_parse_file(WasmSource* source,
                             WasmParserCallbacks* parser,
                             WasmParserOptions* options);
EXTERN_C void wasm_copy_segment_data(WasmSegmentData data,
                                     char* dest,
                                     size_t size);

#endif /* WASM_PARSE_H */
