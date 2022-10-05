#ifndef __BINDINGS_BINDINGS_H
#define __BINDINGS_BINDINGS_H
#ifdef __cplusplus
extern "C"
{
  #endif
  
  #include <stdint.h>
  #include <stdbool.h>
  
  typedef struct {
    char *ptr;
    size_t len;
  } bindings_string_t;
  
  void bindings_string_set(bindings_string_t *ret, const char *s);
  void bindings_string_dup(bindings_string_t *ret, const char *s);
  void bindings_string_free(bindings_string_t *ret);
  typedef uint16_t bindings_wasm_feature_t;
  #define BINDINGS_WASM_FEATURE_EXCEPTIONS (1 << 0)
  #define BINDINGS_WASM_FEATURE_MUTABLE_GLOBALS (1 << 1)
  #define BINDINGS_WASM_FEATURE_SAT_FLOAT_TO_INT (1 << 2)
  #define BINDINGS_WASM_FEATURE_SIGN_EXTENSION (1 << 3)
  #define BINDINGS_WASM_FEATURE_SIMD (1 << 4)
  #define BINDINGS_WASM_FEATURE_THREADS (1 << 5)
  #define BINDINGS_WASM_FEATURE_MULTI_VALUE (1 << 6)
  #define BINDINGS_WASM_FEATURE_TAIL_CALL (1 << 7)
  #define BINDINGS_WASM_FEATURE_BULK_MEMORY (1 << 8)
  #define BINDINGS_WASM_FEATURE_REFERENCE_TYPES (1 << 9)
  #define BINDINGS_WASM_FEATURE_ANNOTATIONS (1 << 10)
  #define BINDINGS_WASM_FEATURE_GC (1 << 11)
  typedef struct {
    uint8_t *ptr;
    size_t len;
  } bindings_list_u8_t;
  void bindings_list_u8_free(bindings_list_u8_t *ptr);
  typedef struct {
    bool is_err;
    union {
      bindings_list_u8_t ok;
      bindings_string_t err;
    } val;
  } bindings_expected_list_u8_string_t;
  void bindings_expected_list_u8_string_free(bindings_expected_list_u8_string_t *ptr);
  typedef struct {
    bool is_err;
    union {
      bindings_string_t ok;
      bindings_string_t err;
    } val;
  } bindings_expected_string_string_t;
  void bindings_expected_string_string_free(bindings_expected_string_string_t *ptr);
  void bindings_wat2wasm(bindings_string_t *wat, bindings_wasm_feature_t features, bindings_expected_list_u8_string_t *ret0);
  void bindings_wasm2wat(bindings_list_u8_t *wasm, bindings_wasm_feature_t features, bindings_expected_string_string_t *ret0);
  #ifdef __cplusplus
}
#endif
#endif
