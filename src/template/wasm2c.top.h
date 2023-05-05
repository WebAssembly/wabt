#include "wasm-rt.h"

#if defined(WASM_RT_ENABLE_EXCEPTION_HANDLING)
#include "wasm-rt-exceptions.h"
#endif

#if defined(WASM_RT_ENABLE_SIMD)
#include "simde/wasm/simd128.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
