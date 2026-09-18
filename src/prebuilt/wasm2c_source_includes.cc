const char* s_source_includes = R"w2c_template(#if WABT_BIG_ENDIAN
)w2c_template"
R"w2c_template(#if __has_builtin(__builtin_bswap16) && __has_builtin(__builtin_bswap32) && \
)w2c_template"
R"w2c_template(    __has_builtin(__builtin_bswap64)
)w2c_template"
R"w2c_template(#define htole16 __builtin_bswap16
)w2c_template"
R"w2c_template(#define htole32 __builtin_bswap32
)w2c_template"
R"w2c_template(#define htole64 __builtin_bswap64
)w2c_template"
R"w2c_template(#elif defined(__linux__) || defined(__CYGWIN__)
)w2c_template"
R"w2c_template(#ifndef _DEFAULT_SOURCE
)w2c_template"
R"w2c_template(#define _DEFAULT_SOURCE
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(#include <endian.h>
)w2c_template"
R"w2c_template(#elif defined(__APPLE__)
)w2c_template"
R"w2c_template(#include <libkern/OSByteOrder.h>
)w2c_template"
R"w2c_template(#define htole16(value) OSSwapHostToLittleInt16(value)
)w2c_template"
R"w2c_template(#define htole32(value) OSSwapHostToLittleInt32(value)
)w2c_template"
R"w2c_template(#define htole64(value) OSSwapHostToLittleInt64(value)
)w2c_template"
R"w2c_template(#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || \
)w2c_template"
R"w2c_template(    defined(__DragonFly__)
)w2c_template"
R"w2c_template(#include <sys/endian.h>
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(#include <assert.h>
)w2c_template"
R"w2c_template(#include <math.h>
)w2c_template"
R"w2c_template(#include <stdarg.h>
)w2c_template"
R"w2c_template(#include <stddef.h>
)w2c_template"
R"w2c_template(#include <string.h>
)w2c_template"
R"w2c_template(#if defined(__MINGW32__)
)w2c_template"
R"w2c_template(#include <malloc.h>
)w2c_template"
R"w2c_template(#elif defined(_MSC_VER)
)w2c_template"
R"w2c_template(#include <intrin.h>
)w2c_template"
R"w2c_template(#include <malloc.h>
)w2c_template"
R"w2c_template(#define alloca _alloca
)w2c_template"
R"w2c_template(#elif defined(__FreeBSD__) || defined(__OpenBSD__)
)w2c_template"
R"w2c_template(#include <stdlib.h>
)w2c_template"
R"w2c_template(#else
)w2c_template"
R"w2c_template(#include <alloca.h>
)w2c_template"
R"w2c_template(#endif
)w2c_template"
R"w2c_template(
#if WABT_BIG_ENDIAN && !defined(htole16)
)w2c_template"
R"w2c_template(// Fallback for missing byteswap definitions
)w2c_template"
R"w2c_template(static inline uint16_t wasm2c_bswap16(uint16_t value) {
)w2c_template"
R"w2c_template(  return (uint16_t)((value << 8) | (value >> 8));
)w2c_template"
R"w2c_template(}
)w2c_template"
R"w2c_template(
static inline uint32_t wasm2c_bswap32(uint32_t value) {
)w2c_template"
R"w2c_template(  return ((value & 0x000000ffu) << 24) | ((value & 0x0000ff00u) << 8) |
)w2c_template"
R"w2c_template(         ((value & 0x00ff0000u) >> 8) | ((value & 0xff000000u) >> 24);
)w2c_template"
R"w2c_template(}
)w2c_template"
R"w2c_template(
static inline uint64_t wasm2c_bswap64(uint64_t value) {
)w2c_template"
R"w2c_template(  return ((uint64_t)wasm2c_bswap32((uint32_t)value) << 32) |
)w2c_template"
R"w2c_template(         wasm2c_bswap32((uint32_t)(value >> 32));
)w2c_template"
R"w2c_template(}
)w2c_template"
R"w2c_template(
#define htole16 wasm2c_bswap16
)w2c_template"
R"w2c_template(#define htole32 wasm2c_bswap32
)w2c_template"
R"w2c_template(#define htole64 wasm2c_bswap64
)w2c_template"
R"w2c_template(#endif
)w2c_template"
;
