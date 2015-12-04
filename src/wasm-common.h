#ifndef WASM_COMMON_H_
#define WASM_COMMON_H_

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define WARN_UNUSED __attribute__ ((warn_unused_result))

typedef enum WasmResult {
  WASM_OK,
  WASM_ERROR,
} WasmResult;

#endif /* WASM_COMMON_H_ */
