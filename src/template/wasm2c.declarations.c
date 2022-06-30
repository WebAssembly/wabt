
#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER
#define FUNC_PROLOGUE

#define FUNC_EPILOGUE
#else
#define FUNC_PROLOGUE                                            \
  if (++wasm_rt_call_stack_depth > WASM_RT_MAX_CALL_STACK_DEPTH) \
    TRAP(EXHAUSTION);

#define FUNC_EPILOGUE --wasm_rt_call_stack_depth
#endif

#define UNREACHABLE TRAP(UNREACHABLE)

#define CALL_INDIRECT(table, t, ft, x, ...)             \
  (LIKELY((x) < table.size && table.data[x].func &&     \
          table.data[x].func_type == func_types[ft]) || \
       TRAP(CALL_INDIRECT),                             \
   ((t)table.data[x].func)(__VA_ARGS__))

#define RANGE_CHECK(mem, offset, len)               \
  if (UNLIKELY(offset + (uint64_t)len > mem->size)) \
    TRAP(OOB);

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER
#define MEMCHECK(mem, a, t)
#else
#define MEMCHECK(mem, a, t) RANGE_CHECK(mem, a, sizeof(t))
#endif

#ifdef __GNUC__
#define wasm_asm __asm__
#else
#define wasm_asm(X)
#endif

#if WABT_BIG_ENDIAN
static inline void load_data(void* dest, const void* src, size_t n) {
  size_t i = 0;
  u8* dest_chars = dest;
  memcpy(dest, src, n);
  for (i = 0; i < (n >> 1); i++) {
    u8 cursor = dest_chars[i];
    dest_chars[i] = dest_chars[n - i - 1];
    dest_chars[n - i - 1] = cursor;
  }
}
#define LOAD_DATA(m, o, i, s)                   \
  do {                                          \
    RANGE_CHECK((&m), m.size - o - s, s);       \
    load_data(&(m.data[m.size - o - s]), i, s); \
  } while (0)
#define DEFINE_LOAD(name, t1, t2, t3)                                  \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {             \
    MEMCHECK(mem, addr, t1);                                           \
    t1 result;                                                         \
    wasm_rt_memcpy(&result, &mem->data[mem->size - addr - sizeof(t1)], \
                   sizeof(t1));                                        \
    wasm_asm("" ::"r"(result));                                        \
    return (t3)(t2)result;                                             \
  }

#define DEFINE_STORE(name, t1, t2)                                      \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) {  \
    MEMCHECK(mem, addr, t1);                                            \
    t1 wrapped = (t1)value;                                             \
    wasm_rt_memcpy(&mem->data[mem->size - addr - sizeof(t1)], &wrapped, \
                   sizeof(t1));                                         \
  }
#else
static inline void load_data(void* dest, const void* src, size_t n) {
  memcpy(dest, src, n);
}
#define LOAD_DATA(m, o, i, s)      \
  do {                             \
    RANGE_CHECK((&m), o, s);       \
    load_data(&(m.data[o]), i, s); \
  } while (0)
#define DEFINE_LOAD(name, t1, t2, t3)                      \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) { \
    MEMCHECK(mem, addr, t1);                               \
    t1 result;                                             \
    wasm_rt_memcpy(&result, &mem->data[addr], sizeof(t1)); \
    wasm_asm("" ::"r"(result));                            \
    return (t3)(t2)result;                                 \
  }

#define DEFINE_STORE(name, t1, t2)                                     \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                           \
    t1 wrapped = (t1)value;                                            \
    wasm_rt_memcpy(&mem->data[addr], &wrapped, sizeof(t1));            \
  }
#endif

DEFINE_LOAD(i32_load, u32, u32, u32)
DEFINE_LOAD(i64_load, u64, u64, u64)
DEFINE_LOAD(f32_load, f32, f32, f32)
DEFINE_LOAD(f64_load, f64, f64, f64)
DEFINE_LOAD(i32_load8_s, s8, s32, u32)
DEFINE_LOAD(i64_load8_s, s8, s64, u64)
DEFINE_LOAD(i32_load8_u, u8, u32, u32)
DEFINE_LOAD(i64_load8_u, u8, u64, u64)
DEFINE_LOAD(i32_load16_s, s16, s32, u32)
DEFINE_LOAD(i64_load16_s, s16, s64, u64)
DEFINE_LOAD(i32_load16_u, u16, u32, u32)
DEFINE_LOAD(i64_load16_u, u16, u64, u64)
DEFINE_LOAD(i64_load32_s, s32, s64, u64)
DEFINE_LOAD(i64_load32_u, u32, u64, u64)
DEFINE_STORE(i32_store, u32, u32)
DEFINE_STORE(i64_store, u64, u64)
DEFINE_STORE(f32_store, f32, f32)
DEFINE_STORE(f64_store, f64, f64)
DEFINE_STORE(i32_store8, u8, u32)
DEFINE_STORE(i32_store16, u16, u32)
DEFINE_STORE(i64_store8, u8, u64)
DEFINE_STORE(i64_store16, u16, u64)
DEFINE_STORE(i64_store32, u32, u64)

#if defined(_MSC_VER)

#include <intrin.h>

// Adapted from
// https://github.com/nemequ/portable-snippets/blob/master/builtin/builtin.h

static inline int I64_CLZ(unsigned long long v) {
  unsigned long r = 0;
#if defined(_M_AMD64) || defined(_M_ARM)
  if (_BitScanReverse64(&r, v)) {
    return 63 - r;
  }
#else
  if (_BitScanReverse(&r, (unsigned long)(v >> 32))) {
    return 31 - r;
  } else if (_BitScanReverse(&r, (unsigned long)v)) {
    return 63 - r;
  }
#endif
  return 64;
}

static inline int I32_CLZ(unsigned long v) {
  unsigned long r = 0;
  if (_BitScanReverse(&r, v)) {
    return 31 - r;
  }
  return 32;
}

static inline int I64_CTZ(unsigned long long v) {
  if (!v) {
    return 64;
  }
  unsigned long r = 0;
#if defined(_M_AMD64) || defined(_M_ARM)
  _BitScanForward64(&r, v);
  return (int)r;
#else
  if (_BitScanForward(&r, (unsigned int)(v))) {
    return (int)(r);
  }

  _BitScanForward(&r, (unsigned int)(v >> 32));
  return (int)(r + 32);
#endif
}

static inline int I32_CTZ(unsigned long v) {
  if (!v) {
    return 32;
  }
  unsigned long r = 0;
  _BitScanForward(&r, v);
  return (int)r;
}

#define POPCOUNT_DEFINE_PORTABLE(f_n, T)                            \
  static inline u32 f_n(T x) {                                      \
    x = x - ((x >> 1) & (T) ~(T)0 / 3);                             \
    x = (x & (T) ~(T)0 / 15 * 3) + ((x >> 2) & (T) ~(T)0 / 15 * 3); \
    x = (x + (x >> 4)) & (T) ~(T)0 / 255 * 15;                      \
    return (T)(x * ((T) ~(T)0 / 255)) >> (sizeof(T) - 1) * 8;       \
  }

POPCOUNT_DEFINE_PORTABLE(I32_POPCNT, u32)
POPCOUNT_DEFINE_PORTABLE(I64_POPCNT, u64)

#undef POPCOUNT_DEFINE_PORTABLE

#else

#define I32_CLZ(x) ((x) ? __builtin_clz(x) : 32)
#define I64_CLZ(x) ((x) ? __builtin_clzll(x) : 64)
#define I32_CTZ(x) ((x) ? __builtin_ctz(x) : 32)
#define I64_CTZ(x) ((x) ? __builtin_ctzll(x) : 64)
#define I32_POPCNT(x) (__builtin_popcount(x))
#define I64_POPCNT(x) (__builtin_popcountll(x))

#endif

#define DIV_S(ut, min, x, y)                                      \
  ((UNLIKELY((y) == 0))                                           \
       ? TRAP(DIV_BY_ZERO)                                        \
       : (UNLIKELY((x) == min && (y) == -1)) ? TRAP(INT_OVERFLOW) \
                                             : (ut)((x) / (y)))

#define REM_S(ut, min, x, y) \
  ((UNLIKELY((y) == 0))      \
       ? TRAP(DIV_BY_ZERO)   \
       : (UNLIKELY((x) == min && (y) == -1)) ? 0 : (ut)((x) % (y)))

#define I32_DIV_S(x, y) DIV_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIV_S(u64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) REM_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) REM_S(u64, INT64_MIN, (s64)x, (s64)y)

#define DIVREM_U(op, x, y) \
  ((UNLIKELY((y) == 0)) ? TRAP(DIV_BY_ZERO) : ((x)op(y)))

#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define ROTL(x, y, mask) \
  (((x) << ((y) & (mask))) | ((x) >> (((mask) - (y) + 1) & (mask))))
#define ROTR(x, y, mask) \
  (((x) >> ((y) & (mask))) | ((x) << (((mask) - (y) + 1) & (mask))))

#define I32_ROTL(x, y) ROTL(x, y, 31)
#define I64_ROTL(x, y) ROTL(x, y, 63)
#define I32_ROTR(x, y) ROTR(x, y, 31)
#define I64_ROTR(x, y) ROTR(x, y, 63)

#define FMIN(x, y)                                                     \
  ((UNLIKELY((x) != (x)))                                              \
       ? NAN                                                           \
       : (UNLIKELY((y) != (y)))                                        \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? x : y) \
                                                : (x < y) ? x : y)

#define FMAX(x, y)                                                     \
  ((UNLIKELY((x) != (x)))                                              \
       ? NAN                                                           \
       : (UNLIKELY((y) != (y)))                                        \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? y : x) \
                                                : (x > y) ? x : y)

#define TRUNC_S(ut, st, ft, min, minop, max, x)                           \
  ((UNLIKELY((x) != (x)))                                                 \
       ? TRAP(INVALID_CONVERSION)                                         \
       : (UNLIKELY(!((x)minop(min) && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                     : (ut)(st)(x))

#define I32_TRUNC_S_F32(x) \
  TRUNC_S(u32, s32, f32, (f32)INT32_MIN, >=, 2147483648.f, x)
#define I64_TRUNC_S_F32(x) \
  TRUNC_S(u64, s64, f32, (f32)INT64_MIN, >=, (f32)INT64_MAX, x)
#define I32_TRUNC_S_F64(x) \
  TRUNC_S(u32, s32, f64, -2147483649., >, 2147483648., x)
#define I64_TRUNC_S_F64(x) \
  TRUNC_S(u64, s64, f64, (f64)INT64_MIN, >=, (f64)INT64_MAX, x)

#define TRUNC_U(ut, ft, max, x)                                          \
  ((UNLIKELY((x) != (x)))                                                \
       ? TRAP(INVALID_CONVERSION)                                        \
       : (UNLIKELY(!((x) > (ft)-1 && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                    : (ut)(x))

#define I32_TRUNC_U_F32(x) TRUNC_U(u32, f32, 4294967296.f, x)
#define I64_TRUNC_U_F32(x) TRUNC_U(u64, f32, (f32)UINT64_MAX, x)
#define I32_TRUNC_U_F64(x) TRUNC_U(u32, f64, 4294967296., x)
#define I64_TRUNC_U_F64(x) TRUNC_U(u64, f64, (f64)UINT64_MAX, x)

#define TRUNC_SAT_S(ut, st, ft, min, smin, minop, max, smax, x) \
  ((UNLIKELY((x) != (x)))                                       \
       ? 0                                                      \
       : (UNLIKELY(!((x)minop(min))))                           \
             ? smin                                             \
             : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(st)(x))

#define I32_TRUNC_SAT_S_F32(x)                                            \
  TRUNC_SAT_S(u32, s32, f32, (f32)INT32_MIN, INT32_MIN, >=, 2147483648.f, \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F32(x)                                              \
  TRUNC_SAT_S(u64, s64, f32, (f32)INT64_MIN, INT64_MIN, >=, (f32)INT64_MAX, \
              INT64_MAX, x)
#define I32_TRUNC_SAT_S_F64(x)                                        \
  TRUNC_SAT_S(u32, s32, f64, -2147483649., INT32_MIN, >, 2147483648., \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F64(x)                                              \
  TRUNC_SAT_S(u64, s64, f64, (f64)INT64_MIN, INT64_MIN, >=, (f64)INT64_MAX, \
              INT64_MAX, x)

#define TRUNC_SAT_U(ut, ft, max, smax, x)               \
  ((UNLIKELY((x) != (x))) ? 0                           \
                          : (UNLIKELY(!((x) > (ft)-1))) \
                                ? 0                     \
                                : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(x))

#define I32_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u32, f32, 4294967296.f, UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u64, f32, (f32)UINT64_MAX, UINT64_MAX, x)
#define I32_TRUNC_SAT_U_F64(x) TRUNC_SAT_U(u32, f64, 4294967296., UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F64(x) \
  TRUNC_SAT_U(u64, f64, (f64)UINT64_MAX, UINT64_MAX, x)

#define DEFINE_REINTERPRET(name, t1, t2) \
  static inline t2 name(t1 x) {          \
    t2 result;                           \
    memcpy(&result, &x, sizeof(result)); \
    return result;                       \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)

static float quiet_nanf(float x) {
  uint32_t tmp;
  memcpy(&tmp, &x, 4);
  tmp |= 0x7fc00000lu;
  memcpy(&x, &tmp, 4);
  return x;
}

static double quiet_nan(double x) {
  uint64_t tmp;
  memcpy(&tmp, &x, 8);
  tmp |= 0x7ff8000000000000llu;
  memcpy(&x, &tmp, 8);
  return x;
}

static double wasm_floor(double x) {
  if (isnan(x)) {
    return quiet_nan(x);
  }
  return floor(x);
}

static float wasm_floorf(float x) {
  if (isnan(x)) {
    return quiet_nanf(x);
  }
  return floorf(x);
}

static double wasm_ceil(double x) {
  if (isnan(x)) {
    return quiet_nan(x);
  }
  return ceil(x);
}

static float wasm_ceilf(float x) {
  if (isnan(x)) {
    return quiet_nanf(x);
  }
  return ceilf(x);
}

static double wasm_trunc(double x) {
  if (isnan(x)) {
    return quiet_nan(x);
  }
  return trunc(x);
}

static float wasm_truncf(float x) {
  if (isnan(x)) {
    return quiet_nanf(x);
  }
  return truncf(x);
}

static float wasm_nearbyintf(float x) {
  if (isnan(x)) {
    return quiet_nanf(x);
  }
  return nearbyintf(x);
}

static double wasm_nearbyint(double x) {
  if (isnan(x)) {
    return quiet_nan(x);
  }
  return nearbyint(x);
}

static float wasm_fabsf(float x) {
  if (isnan(x)) {
    uint32_t tmp;
    memcpy(&tmp, &x, 4);
    tmp = tmp & ~(1 << 31);
    memcpy(&x, &tmp, 4);
    return x;
  }
  return fabsf(x);
}

static double wasm_fabs(double x) {
  if (isnan(x)) {
    uint64_t tmp;
    memcpy(&tmp, &x, 8);
    tmp = tmp & ~(1ll << 63);
    memcpy(&x, &tmp, 8);
    return x;
  }
  return fabs(x);
}
