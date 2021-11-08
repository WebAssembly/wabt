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
  typedef uint8_t input_errno_t;
  typedef struct {
    uint8_t *ptr;
    size_t len;
  } input_list_u8_t;
  #define INPUT_ERRNO_BASE 0
  #define INPUT_ERRNO_BASEX 1
  input_errno_t input_wat2wasm(input_string_t *wat, input_wasm_feature_t features, input_list_u8_t *ret0);
  #ifdef __cplusplus
}
#endif
#endif
