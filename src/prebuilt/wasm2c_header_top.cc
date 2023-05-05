const char* s_header_top = R"w2c_template(#include "wasm-rt.h"
)w2c_template"
R"w2c_template(
#if defined(WASM_RT_ENABLE_EXCEPTION_HANDLING)
)w2c_template"
R"w2c_template(#include "wasm-rt-exceptions.h"
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(
#if defined(WASM_RT_ENABLE_SIMD)
)w2c_template"
R"w2c_template(#include "simde/wasm/simd128.h"
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(
#ifdef __cplusplus
)w2c_template"
R"w2c_template(extern "C" {
)w2c_template"
R"w2c_template(#endif
)w2c_template"
;
