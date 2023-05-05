
#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#if WASM_RT_USE_STACK_DEPTH_COUNT
#define FUNC_PROLOGUE                                            \
  if (++wasm_rt_call_stack_depth > WASM_RT_MAX_CALL_STACK_DEPTH) \
    TRAP(EXHAUSTION);

#define FUNC_EPILOGUE --wasm_rt_call_stack_depth
#else
#define FUNC_PROLOGUE

#define FUNC_EPILOGUE
#endif

#define UNREACHABLE TRAP(UNREACHABLE)

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;

static inline bool func_types_eq(const wasm_rt_func_type_t a,
                                 const wasm_rt_func_type_t b) {
  return (a == b) || LIKELY(a && b && !memcmp(a, b, 32));
}

#define CALL_INDIRECT(table, t, ft, x, ...)              \
  (LIKELY((x) < table.size && table.data[x].func &&      \
          func_types_eq(ft, table.data[x].func_type)) || \
       TRAP(CALL_INDIRECT),                              \
   ((t)table.data[x].func)(__VA_ARGS__))

#ifdef SUPPORT_MEMORY64
#define RANGE_CHECK(mem, offset, len)              \
  do {                                             \
    uint64_t res;                                  \
    if (__builtin_add_overflow(offset, len, &res)) \
      TRAP(OOB);                                   \
    if (UNLIKELY(res > mem->size))                 \
      TRAP(OOB);                                   \
  } while (0);
#else
#define RANGE_CHECK(mem, offset, len)               \
  if (UNLIKELY(offset + (uint64_t)len > mem->size)) \
    TRAP(OOB);
#endif

#if WASM_RT_MEMCHECK_GUARD_PAGES
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
  if (!n) {
    return;
  }
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
  if (!n) {
    return;
  }
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

#if defined(WASM_RT_ENABLE_SIMD)
typedef simde_v128_t v128;

#ifdef __x86_64__
#define SIMD_FORCE_READ(var) wasm_asm("" ::"x"(var));
#else
#define SIMD_FORCE_READ(var)
#endif
// TODO: equivalent constraint for ARM and other architectures

#define DEFINE_SIMD_LOAD_FUNC(name, func, t)                 \
  static inline v128 name(wasm_rt_memory_t* mem, u64 addr) { \
    MEMCHECK(mem, addr, t);                                  \
    v128 result = func((v128*)&mem->data[addr]);             \
    SIMD_FORCE_READ(result);                                 \
    return result;                                           \
  }

#define DEFINE_SIMD_LOAD_LANE(name, func, t, lane)                     \
  static inline v128 name(wasm_rt_memory_t* mem, u64 addr, v128 vec) { \
    MEMCHECK(mem, addr, t);                                            \
    v128 result = func((v128*)&mem->data[addr], vec, lane);            \
    SIMD_FORCE_READ(result);                                           \
    return result;                                                     \
  }

#define DEFINE_SIMD_STORE(name, t)                                       \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, v128 value) { \
    MEMCHECK(mem, addr, t);                                              \
    simde_wasm_v128_store((v128*)&mem->data[addr], value);               \
  }

#define DEFINE_SIMD_STORE_LANE(name, func, t, lane)                      \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, v128 value) { \
    MEMCHECK(mem, addr, t);                                              \
    func((v128*)&mem->data[addr], value, lane);                          \
  }

// clang-format off
DEFINE_SIMD_LOAD_FUNC(v128_load, simde_wasm_v128_load, v128)

DEFINE_SIMD_LOAD_FUNC(v128_load8_splat, simde_wasm_v128_load8_splat, u8)
DEFINE_SIMD_LOAD_FUNC(v128_load16_splat, simde_wasm_v128_load16_splat, u16)
DEFINE_SIMD_LOAD_FUNC(v128_load32_splat, simde_wasm_v128_load32_splat, u32)
DEFINE_SIMD_LOAD_FUNC(v128_load64_splat, simde_wasm_v128_load64_splat, u64)

DEFINE_SIMD_LOAD_FUNC(i16x8_load8x8, simde_wasm_i16x8_load8x8, u64)
DEFINE_SIMD_LOAD_FUNC(u16x8_load8x8, simde_wasm_u16x8_load8x8, u64)
DEFINE_SIMD_LOAD_FUNC(i32x4_load16x4, simde_wasm_i32x4_load16x4, u64)
DEFINE_SIMD_LOAD_FUNC(u32x4_load16x4, simde_wasm_u32x4_load16x4, u64)
DEFINE_SIMD_LOAD_FUNC(i64x2_load32x2, simde_wasm_i64x2_load32x2, u64)
DEFINE_SIMD_LOAD_FUNC(u64x2_load32x2, simde_wasm_u64x2_load32x2, u64)

DEFINE_SIMD_LOAD_FUNC(v128_load32_zero, simde_wasm_v128_load32_zero, u32)
DEFINE_SIMD_LOAD_FUNC(v128_load64_zero, simde_wasm_v128_load64_zero, u64)

DEFINE_SIMD_LOAD_LANE(v128_load8_lane0, simde_wasm_v128_load8_lane, u8, 0)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane1, simde_wasm_v128_load8_lane, u8, 1)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane2, simde_wasm_v128_load8_lane, u8, 2)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane3, simde_wasm_v128_load8_lane, u8, 3)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane4, simde_wasm_v128_load8_lane, u8, 4)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane5, simde_wasm_v128_load8_lane, u8, 5)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane6, simde_wasm_v128_load8_lane, u8, 6)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane7, simde_wasm_v128_load8_lane, u8, 7)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane8, simde_wasm_v128_load8_lane, u8, 8)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane9, simde_wasm_v128_load8_lane, u8, 9)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane10, simde_wasm_v128_load8_lane, u8, 10)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane11, simde_wasm_v128_load8_lane, u8, 11)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane12, simde_wasm_v128_load8_lane, u8, 12)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane13, simde_wasm_v128_load8_lane, u8, 13)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane14, simde_wasm_v128_load8_lane, u8, 14)
DEFINE_SIMD_LOAD_LANE(v128_load8_lane15, simde_wasm_v128_load8_lane, u8, 15)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane0, simde_wasm_v128_load16_lane, u16, 0)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane1, simde_wasm_v128_load16_lane, u16, 1)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane2, simde_wasm_v128_load16_lane, u16, 2)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane3, simde_wasm_v128_load16_lane, u16, 3)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane4, simde_wasm_v128_load16_lane, u16, 4)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane5, simde_wasm_v128_load16_lane, u16, 5)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane6, simde_wasm_v128_load16_lane, u16, 6)
DEFINE_SIMD_LOAD_LANE(v128_load16_lane7, simde_wasm_v128_load16_lane, u16, 7)
DEFINE_SIMD_LOAD_LANE(v128_load32_lane0, simde_wasm_v128_load32_lane, u32, 0)
DEFINE_SIMD_LOAD_LANE(v128_load32_lane1, simde_wasm_v128_load32_lane, u32, 1)
DEFINE_SIMD_LOAD_LANE(v128_load32_lane2, simde_wasm_v128_load32_lane, u32, 2)
DEFINE_SIMD_LOAD_LANE(v128_load32_lane3, simde_wasm_v128_load32_lane, u32, 3)
DEFINE_SIMD_LOAD_LANE(v128_load64_lane0, simde_wasm_v128_load64_lane, u64, 0)
DEFINE_SIMD_LOAD_LANE(v128_load64_lane1, simde_wasm_v128_load64_lane, u64, 1)

DEFINE_SIMD_STORE(v128_store, v128)

DEFINE_SIMD_STORE_LANE(v128_store8_lane0, simde_wasm_v128_store8_lane, u8, 0)
DEFINE_SIMD_STORE_LANE(v128_store8_lane1, simde_wasm_v128_store8_lane, u8, 1)
DEFINE_SIMD_STORE_LANE(v128_store8_lane2, simde_wasm_v128_store8_lane, u8, 2)
DEFINE_SIMD_STORE_LANE(v128_store8_lane3, simde_wasm_v128_store8_lane, u8, 3)
DEFINE_SIMD_STORE_LANE(v128_store8_lane4, simde_wasm_v128_store8_lane, u8, 4)
DEFINE_SIMD_STORE_LANE(v128_store8_lane5, simde_wasm_v128_store8_lane, u8, 5)
DEFINE_SIMD_STORE_LANE(v128_store8_lane6, simde_wasm_v128_store8_lane, u8, 6)
DEFINE_SIMD_STORE_LANE(v128_store8_lane7, simde_wasm_v128_store8_lane, u8, 7)
DEFINE_SIMD_STORE_LANE(v128_store8_lane8, simde_wasm_v128_store8_lane, u8, 8)
DEFINE_SIMD_STORE_LANE(v128_store8_lane9, simde_wasm_v128_store8_lane, u8, 9)
DEFINE_SIMD_STORE_LANE(v128_store8_lane10, simde_wasm_v128_store8_lane, u8, 10)
DEFINE_SIMD_STORE_LANE(v128_store8_lane11, simde_wasm_v128_store8_lane, u8, 11)
DEFINE_SIMD_STORE_LANE(v128_store8_lane12, simde_wasm_v128_store8_lane, u8, 12)
DEFINE_SIMD_STORE_LANE(v128_store8_lane13, simde_wasm_v128_store8_lane, u8, 13)
DEFINE_SIMD_STORE_LANE(v128_store8_lane14, simde_wasm_v128_store8_lane, u8, 14)
DEFINE_SIMD_STORE_LANE(v128_store8_lane15, simde_wasm_v128_store8_lane, u8, 15)
DEFINE_SIMD_STORE_LANE(v128_store16_lane0, simde_wasm_v128_store16_lane, u16, 0)
DEFINE_SIMD_STORE_LANE(v128_store16_lane1, simde_wasm_v128_store16_lane, u16, 1)
DEFINE_SIMD_STORE_LANE(v128_store16_lane2, simde_wasm_v128_store16_lane, u16, 2)
DEFINE_SIMD_STORE_LANE(v128_store16_lane3, simde_wasm_v128_store16_lane, u16, 3)
DEFINE_SIMD_STORE_LANE(v128_store16_lane4, simde_wasm_v128_store16_lane, u16, 4)
DEFINE_SIMD_STORE_LANE(v128_store16_lane5, simde_wasm_v128_store16_lane, u16, 5)
DEFINE_SIMD_STORE_LANE(v128_store16_lane6, simde_wasm_v128_store16_lane, u16, 6)
DEFINE_SIMD_STORE_LANE(v128_store16_lane7, simde_wasm_v128_store16_lane, u16, 7)
DEFINE_SIMD_STORE_LANE(v128_store32_lane0, simde_wasm_v128_store32_lane, u32, 0)
DEFINE_SIMD_STORE_LANE(v128_store32_lane1, simde_wasm_v128_store32_lane, u32, 1)
DEFINE_SIMD_STORE_LANE(v128_store32_lane2, simde_wasm_v128_store32_lane, u32, 2)
DEFINE_SIMD_STORE_LANE(v128_store32_lane3, simde_wasm_v128_store32_lane, u32, 3)
DEFINE_SIMD_STORE_LANE(v128_store64_lane0, simde_wasm_v128_store64_lane, u64, 0)
DEFINE_SIMD_STORE_LANE(v128_store64_lane1, simde_wasm_v128_store64_lane, u64, 1)
// clang-format on
#endif

#if defined(_MSC_VER)

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

static double wasm_quiet(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return x;
}

static float wasm_quietf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return x;
}

static double wasm_floor(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return floor(x);
}

static float wasm_floorf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return floorf(x);
}

static double wasm_ceil(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return ceil(x);
}

static float wasm_ceilf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return ceilf(x);
}

static double wasm_trunc(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return trunc(x);
}

static float wasm_truncf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return truncf(x);
}

static float wasm_nearbyintf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return nearbyintf(x);
}

static double wasm_nearbyint(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return nearbyint(x);
}

static float wasm_fabsf(float x) {
  if (UNLIKELY(isnan(x))) {
    uint32_t tmp;
    memcpy(&tmp, &x, 4);
    tmp = tmp & ~(1UL << 31);
    memcpy(&x, &tmp, 4);
    return x;
  }
  return fabsf(x);
}

static double wasm_fabs(double x) {
  if (UNLIKELY(isnan(x))) {
    uint64_t tmp;
    memcpy(&tmp, &x, 8);
    tmp = tmp & ~(1ULL << 63);
    memcpy(&x, &tmp, 8);
    return x;
  }
  return fabs(x);
}

static double wasm_sqrt(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return sqrt(x);
}

static float wasm_sqrtf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return sqrtf(x);
}

static inline void memory_fill(wasm_rt_memory_t* mem, u32 d, u32 val, u32 n) {
  RANGE_CHECK(mem, d, n);
  memset(mem->data + d, val, n);
}

static inline void memory_copy(wasm_rt_memory_t* dest,
                               const wasm_rt_memory_t* src,
                               u32 dest_addr,
                               u32 src_addr,
                               u32 n) {
  RANGE_CHECK(dest, dest_addr, n);
  RANGE_CHECK(src, src_addr, n);
  memmove(dest->data + dest_addr, src->data + src_addr, n);
}

static inline void memory_init(wasm_rt_memory_t* dest,
                               const u8* src,
                               u32 src_size,
                               u32 dest_addr,
                               u32 src_addr,
                               u32 n) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  LOAD_DATA((*dest), dest_addr, src + src_addr, n);
}

typedef struct {
  wasm_rt_func_type_t type;
  wasm_rt_function_ptr_t func;
  size_t module_offset;
} wasm_elem_segment_expr_t;

static inline void funcref_table_init(wasm_rt_funcref_table_t* dest,
                                      const wasm_elem_segment_expr_t* src,
                                      u32 src_size,
                                      u32 dest_addr,
                                      u32 src_addr,
                                      u32 n,
                                      void* module_instance) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))
    TRAP(OOB);
  for (u32 i = 0; i < n; i++) {
    const wasm_elem_segment_expr_t* src_expr = &src[src_addr + i];
    dest->data[dest_addr + i] =
        (wasm_rt_funcref_t){src_expr->type, src_expr->func,
                            (char*)module_instance + src_expr->module_offset};
  }
}

// Currently Wasm only supports initializing externref tables with ref.null.
static inline void externref_table_init(wasm_rt_externref_table_t* dest,
                                        u32 src_size,
                                        u32 dest_addr,
                                        u32 src_addr,
                                        u32 n) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))
    TRAP(OOB);
  for (u32 i = 0; i < n; i++) {
    dest->data[dest_addr + i] = wasm_rt_externref_null_value;
  }
}

#define DEFINE_TABLE_COPY(type)                                              \
  static inline void type##_table_copy(wasm_rt_##type##_table_t* dest,       \
                                       const wasm_rt_##type##_table_t* src,  \
                                       u32 dest_addr, u32 src_addr, u32 n) { \
    if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))                      \
      TRAP(OOB);                                                             \
    if (UNLIKELY(src_addr + (uint64_t)n > src->size))                        \
      TRAP(OOB);                                                             \
                                                                             \
    memmove(dest->data + dest_addr, src->data + src_addr,                    \
            n * sizeof(wasm_rt_##type##_t));                                 \
  }

DEFINE_TABLE_COPY(funcref)
DEFINE_TABLE_COPY(externref)

#define DEFINE_TABLE_GET(type)                        \
  static inline wasm_rt_##type##_t type##_table_get(  \
      const wasm_rt_##type##_table_t* table, u32 i) { \
    if (UNLIKELY(i >= table->size))                   \
      TRAP(OOB);                                      \
    return table->data[i];                            \
  }

DEFINE_TABLE_GET(funcref)
DEFINE_TABLE_GET(externref)

#define DEFINE_TABLE_SET(type)                                               \
  static inline void type##_table_set(const wasm_rt_##type##_table_t* table, \
                                      u32 i, const wasm_rt_##type##_t val) { \
    if (UNLIKELY(i >= table->size))                                          \
      TRAP(OOB);                                                             \
    table->data[i] = val;                                                    \
  }

DEFINE_TABLE_SET(funcref)
DEFINE_TABLE_SET(externref)

#define DEFINE_TABLE_FILL(type)                                               \
  static inline void type##_table_fill(const wasm_rt_##type##_table_t* table, \
                                       u32 d, const wasm_rt_##type##_t val,   \
                                       u32 n) {                               \
    if (UNLIKELY((uint64_t)d + n > table->size))                              \
      TRAP(OOB);                                                              \
    for (uint32_t i = d; i < d + n; i++) {                                    \
      table->data[i] = val;                                                   \
    }                                                                         \
  }

DEFINE_TABLE_FILL(funcref)
DEFINE_TABLE_FILL(externref)

#if defined(__GNUC__) || defined(__clang__)
#define FUNC_TYPE_DECL_EXTERN_T(x) extern const char* const x
#define FUNC_TYPE_EXTERN_T(x) const char* const x
#define FUNC_TYPE_T(x) static const char* const x
#else
#define FUNC_TYPE_DECL_EXTERN_T(x) extern const char x[]
#define FUNC_TYPE_EXTERN_T(x) const char x[]
#define FUNC_TYPE_T(x) static const char x[]
#endif
