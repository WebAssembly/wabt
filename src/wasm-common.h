#ifndef WASM_COMMON_H_
#define WASM_COMMON_H_

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#define WARN_UNUSED __attribute__ ((warn_unused_result))

typedef enum WasmResult {
  WASM_OK,
  WASM_ERROR,
} WasmResult;

#endif /* WASM_COMMON_H_ */
