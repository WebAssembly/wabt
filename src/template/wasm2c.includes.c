#if WABT_BIG_ENDIAN
#if __has_builtin(__builtin_bswap16) && __has_builtin(__builtin_bswap32) && \
    __has_builtin(__builtin_bswap64)
#define htole16 __builtin_bswap16
#define htole32 __builtin_bswap32
#define htole64 __builtin_bswap64
#elif defined(__linux__) || defined(__CYGWIN__)
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define htole16(value) OSSwapHostToLittleInt16(value)
#define htole32(value) OSSwapHostToLittleInt32(value)
#define htole64(value) OSSwapHostToLittleInt64(value)
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || \
    defined(__DragonFly__)
#include <sys/endian.h>
#endif
#endif
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#if defined(__MINGW32__)
#include <malloc.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#include <malloc.h>
#define alloca _alloca
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#include <stdlib.h>
#else
#include <alloca.h>
#endif

#if WABT_BIG_ENDIAN && !defined(htole16)
// Fallback for missing byteswap definitions
static inline uint16_t wasm2c_bswap16(uint16_t value) {
  return (uint16_t)((value << 8) | (value >> 8));
}

static inline uint32_t wasm2c_bswap32(uint32_t value) {
  return ((value & 0x000000ffu) << 24) | ((value & 0x0000ff00u) << 8) |
         ((value & 0x00ff0000u) >> 8) | ((value & 0xff000000u) >> 24);
}

static inline uint64_t wasm2c_bswap64(uint64_t value) {
  return ((uint64_t)wasm2c_bswap32((uint32_t)value) << 32) |
         wasm2c_bswap32((uint32_t)(value >> 32));
}

#define htole16 wasm2c_bswap16
#define htole32 wasm2c_bswap32
#define htole64 wasm2c_bswap64
#endif
