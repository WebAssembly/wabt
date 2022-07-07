/* Generated from 'wasm2c.h.tmpl' by wasm2c_tmpl.py, do not edit! */
const char SECTION_NAME(top)[] =
"#include <stdint.h>\n"
"\n"
"#include \"wasm-rt.h\"\n"
"\n"
"/* TODO(binji): only use stdint.h types in header */\n"
"#ifndef WASM_RT_CORE_TYPES_DEFINED\n"
"#define WASM_RT_CORE_TYPES_DEFINED\n"
"typedef uint8_t u8;\n"
"typedef int8_t s8;\n"
"typedef uint16_t u16;\n"
"typedef int16_t s16;\n"
"typedef uint32_t u32;\n"
"typedef int32_t s32;\n"
"typedef uint64_t u64;\n"
"typedef int64_t s64;\n"
"typedef float f32;\n"
"typedef double f64;\n"
"#endif\n"
"\n"
"#ifdef __cplusplus\n"
"extern \"C\" {\n"
"#endif\n"
;

const char SECTION_NAME(bottom)[] =
"\n"
"#ifdef __cplusplus\n"
"}\n"
"#endif\n"
;
