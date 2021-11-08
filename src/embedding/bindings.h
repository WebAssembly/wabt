#ifndef __BINDINGS_INPUT_H
#define __BINDINGS_INPUT_H
#ifdef __cplusplus
extern "C"
{
  #endif
  
  #include <stdint.h>
  #include <stdbool.h>
  
  typedef struct {
    char *ptr;
    size_t len;
  } input_string_t;
  
  void input_string_set(input_string_t *ret, const char *s);
  void input_string_dup(input_string_t *ret, const char *s);
  void input_string_free(input_string_t *ret);
  typedef uint8_t input_wasm_feature_t;
  #define INPUT_WASM_FEATURE_EXCEPTIONS (1 << 0)
  #define INPUT_WASM_FEATURE_MUTABLE_GLOBALS (1 << 1)
  #define INPUT_WASM_FEATURE_SAT_FLOAT_TO_INT (1 << 2)
  #define INPUT_WASM_FEATURE_SIGN_EXTENSION (1 << 3)
  #define INPUT_WASM_FEATURE_SIMD (1 << 4)
  #define INPUT_WASM_FEATURE_THREADS (1 << 5)
  #define INPUT_WASM_FEATURE_MULTI_VALUE (1 << 6)
  #define INPUT_WASM_FEATURE_TAIL_CALL (1 << 7)
  #define INPUT_WASM_FEATURE_BULK_MEMORY (1 << 8)
  #define INPUT_WASM_FEATURE_REFERENCE_TYPES (1 << 9)
  #define INPUT_WASM_FEATURE_ANNOTATIONS (1 << 10)
  #define INPUT_WASM_FEATURE_GC (1 << 11)
  typedef struct {
    uint8_t *ptr;
    size_t len;
  } input_list_u8_t;
  void input_list_u8_free(input_list_u8_t *ptr);
  typedef struct {
    // 0 if `val` is `ok`, 1 otherwise
    uint8_t tag;
    union {
      input_list_u8_t ok;
      input_string_t err;
    } val;
  } input_expected_list_u8_string_t;
  void input_expected_list_u8_string_free(input_expected_list_u8_string_t *ptr);
  void input_wat2wasm(input_string_t *wat, input_wasm_feature_t features, input_expected_list_u8_string_t *ret0);
  #ifdef __cplusplus
}
#endif
#endif
