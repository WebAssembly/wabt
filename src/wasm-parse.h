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

typedef struct WasmParserCallbacks {
  void* user_data;
  void (*error)(WasmSourceLocation loc, const char* msg, void* user_data);
  void (*before_module)(struct WasmModule* m, void* user_data);
  void (*after_module)(struct WasmModule* m, void* user_data);
  void (*before_function)(struct WasmModule* m,
                          struct WasmFunction* f,
                          void* user_data);
  void (*after_function)(struct WasmModule* m,
                         struct WasmFunction* f,
                         int num_exprs,
                         void* user_data);
  void (*before_export)(struct WasmModule* m, void* user_data);
  void (*after_export)(struct WasmModule* m,
                       struct WasmFunction* f,
                       void* user_data);

  void (*before_binary)(enum WasmOpcode opcode, void* user_data);
  WasmParserCookie (*before_block)(void* user_data);
  void (*after_block)(WasmType type,
                      int num_exprs,
                      WasmParserCookie cookie,
                      void* user_data);
  WasmParserCookie (*before_break)(int with_expr,
                                   int label_depth,
                                   void* user_data);
  void (*after_break)(WasmParserCookie cookie, void* user_data);
  void (*before_call)(int function_index, void* user_data);
  void (*before_call_import)(int import_index, void* user_data);
  void (*before_compare)(enum WasmOpcode opcode, void* user_data);
  void (*after_const)(enum WasmOpcode opcode,
                      WasmType type,
                      WasmNumber value,
                      void* user_data);
  void (*before_convert)(enum WasmOpcode opcode, void* user_data);
  WasmParserCookie (*before_label)(void* user_data);
  void (*after_label)(WasmType type,
                      int num_exprs,
                      WasmParserCookie cookie,
                      void* user_data);
  void (*after_get_local)(int remapped_index, void* user_data);
  WasmParserCookie (*before_loop)(void* user_data);
  void (*after_loop)(int num_exprs, WasmParserCookie cookie, void* user_data);
  WasmParserCookie (*before_if)(void* user_data);
  void (*after_if)(WasmType type,
                   int with_else,
                   WasmParserCookie cookie,
                   void* user_data);
  void (*before_load)(enum WasmOpcode opcode, uint8_t access, void* user_data);
  void (*after_load_global)(int index, void* user_data);
  void (*after_nop)(void* user_data);
  void (*before_return)(void* user_data);
  void (*after_return)(WasmType type, void* user_data);
  void (*before_set_local)(int index, void* user_data);
  void (*before_store)(enum WasmOpcode opcode, uint8_t access, void* user_data);
  void (*before_store_global)(int index, void* user_data);
  void (*before_unary)(enum WasmOpcode opcode, void* user_data);

  /* used in spec repo tests */
  WasmParserCookie (*before_assert_eq)(void* user_data);
  void (*after_assert_eq)(WasmType type,
                          WasmParserCookie cookie,
                          void* user_data);
  void (*before_assert_trap)(void* user_data);
  void (*after_assert_trap)(void* user_data);
  WasmParserCookie (*before_invoke)(const char* invoke_name,
                                    int invoke_function_index,
                                    void* user_data);
  void (*after_invoke)(WasmParserCookie cookie, void* user_data);

  /* called with the error from the module parsed inside of assert_invalid */
  void (*assert_invalid_error)(WasmSourceLocation loc,
                               const char* msg,
                               void* user_data);
} WasmParserCallbacks;

EXTERN_C int wasm_parse_module(WasmSource* source, WasmParserCallbacks* parser);
EXTERN_C int wasm_parse_file(WasmSource* source, WasmParserCallbacks* parser);
EXTERN_C void wasm_copy_segment_data(WasmSegmentData data,
                                     char* dest,
                                     size_t size);

#endif /* WASM_PARSE_H */
