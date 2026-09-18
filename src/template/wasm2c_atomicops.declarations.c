#include <stdatomic.h>

#ifndef WASM_RT_C11_AVAILABLE
#error "C11 is required for Wasm threads and shared memory support"
#endif

#define ATOMIC_ALIGNMENT_CHECK(addr, t1) \
  if (UNLIKELY(addr % sizeof(t1))) {     \
    TRAP(UNALIGNED);                     \
  }

#if !WABT_BIG_ENDIAN
#define DEFINE_SHARED_LOAD(name, t1, atomic_type, t2, t3, force_read)         \
  static inline t3 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_shared_memory_t* mem, u64 addr) { \
    t1 result = atomic_load_explicit((_Atomic volatile t1*)&mem->data[addr],  \
                                     memory_order_relaxed);                   \
    t3 ret = (t3)(t2)result;                                                  \
    force_read(ret);                                                          \
    return ret;                                                               \
  }                                                                           \
  DEF_MEM_CHECKS0(name, _shared_, t1, return, t3)
#else
// Big endian should not use atomic_load_explicit to load float* as Wasm stores
// these in little endian. Loading arbitrary values into float's may result in
// loading bytes that get interpreted as signalling nans etc. We avoid this by
// loading into an unsigned type of the same size.
#define DEFINE_SHARED_LOAD(name, t1, atomic_type, t2, t3, force_read)         \
  static inline t3 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_shared_memory_t* mem, u64 addr) { \
    atomic_type raw = WASM_ADJUST_ENDIAN_##atomic_type(                       \
        atomic_load_explicit((_Atomic volatile atomic_type*)&mem->data[addr], \
                             memory_order_relaxed));                          \
    t1 result;                                                                \
    wasm_rt_memcpy(&result, &raw, sizeof(t1));                                \
    t3 ret = (t3)(t2)result;                                                  \
    force_read(ret);                                                          \
    return ret;                                                               \
  }                                                                           \
  DEF_MEM_CHECKS0(name, _shared_, t1, return, t3)
#endif

DEFINE_SHARED_LOAD(i32_load_shared, u32, u32, u32, u32, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load_shared, u64, u64, u64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(f32_load_shared, f32, u32, f32, f32, FORCE_READ_FLOAT)
DEFINE_SHARED_LOAD(f64_load_shared, f64, u64, f64, f64, FORCE_READ_FLOAT)
DEFINE_SHARED_LOAD(i32_load8_s_shared, s8, u8, s32, u32, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load8_s_shared, s8, u8, s64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i32_load8_u_shared, u8, u8, u32, u32, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load8_u_shared, u8, u8, u64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i32_load16_s_shared, s16, u16, s32, u32, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load16_s_shared, s16, u16, s64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i32_load16_u_shared, u16, u16, u32, u32, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load16_u_shared, u16, u16, u64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load32_s_shared, s32, u32, s64, u64, FORCE_READ_INT)
DEFINE_SHARED_LOAD(i64_load32_u_shared, u32, u32, u64, u64, FORCE_READ_INT)

#if !WABT_BIG_ENDIAN
#define DEFINE_SHARED_STORE(name, t1, atomic_type, t2)                        \
  static inline void name##_unchecked(                                        \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr, t2 value) {                                                   \
    t1 wrapped = (t1)value;                                                   \
    atomic_store_explicit((_Atomic volatile t1*)&mem->data[addr], wrapped,    \
                          memory_order_relaxed);                              \
  }                                                                           \
  DEF_MEM_CHECKS1(name, _shared_, t1, , void, t2)
#else
// Big endian should not use atomic_store_explicit to store float* for the same
// reason as the load
#define DEFINE_SHARED_STORE(name, t1, atomic_type, t2)                        \
  static inline void name##_unchecked(                                        \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr, t2 value) {                                                   \
    t1 wrapped = (t1)value;                                                   \
    atomic_type raw;                                                          \
    wasm_rt_memcpy(&raw, &wrapped, sizeof(t1));                               \
    atomic_store_explicit((_Atomic volatile atomic_type*)&mem->data[addr],    \
                          WASM_ADJUST_ENDIAN_##atomic_type(raw),              \
                          memory_order_relaxed);                              \
  }                                                                           \
  DEF_MEM_CHECKS1(name, _shared_, t1, , void, t2)
#endif

DEFINE_SHARED_STORE(i32_store_shared, u32, u32, u32)
DEFINE_SHARED_STORE(i64_store_shared, u64, u64, u64)
DEFINE_SHARED_STORE(f32_store_shared, f32, u32, f32)
DEFINE_SHARED_STORE(f64_store_shared, f64, u64, f64)
DEFINE_SHARED_STORE(i32_store8_shared, u8, u8, u32)
DEFINE_SHARED_STORE(i32_store16_shared, u16, u16, u32)
DEFINE_SHARED_STORE(i64_store8_shared, u8, u8, u64)
DEFINE_SHARED_STORE(i64_store16_shared, u16, u16, u64)
DEFINE_SHARED_STORE(i64_store32_shared, u32, u32, u64)

#define DEFINE_ATOMIC_LOAD(name, t1, t2, t3, force_read)                      \
  static inline t3 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_memory_t* mem, u64 addr) {        \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 result;                                                                \
    wasm_rt_memcpy(&result, MEM_ADDR_MEMOP(mem, addr), sizeof(t1));           \
    t3 ret = (t3)(t2)WASM_ADJUST_ENDIAN_##t1(result);                         \
    force_read(ret);                                                          \
    return ret;                                                               \
  }                                                                           \
  DEF_MEM_CHECKS0(name, _, t1, return, t3)                                    \
  static inline t3 name##_shared_unchecked(                                   \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr) {                                                             \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 result = atomic_load((_Atomic volatile t1*)&mem->data[addr]);          \
    t3 ret = (t3)(t2)WASM_ADJUST_ENDIAN_##t1(result);                         \
    force_read(ret);                                                          \
    return ret;                                                               \
  }                                                                           \
  DEF_MEM_CHECKS0(name##_shared, _shared_, t1, return, t3)

DEFINE_ATOMIC_LOAD(i32_atomic_load, u32, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load, u64, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i32_atomic_load8_u, u8, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load8_u, u8, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i32_atomic_load16_u, u16, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load16_u, u16, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load32_u, u32, u64, u64, FORCE_READ_INT)

#define DEFINE_ATOMIC_STORE(name, t1, t2)                                     \
  static inline void name##_unchecked(                                        \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_memory_t* mem,        \
      u64 addr, t2 value) {                                                   \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 wrapped = WASM_ADJUST_ENDIAN_##t1((t1)value);                          \
    wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr), &wrapped, sizeof(t1));          \
  }                                                                           \
  DEF_MEM_CHECKS1(name, _, t1, , void, t2)                                    \
  static inline void name##_shared_unchecked(                                 \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr, t2 value) {                                                   \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 wrapped = WASM_ADJUST_ENDIAN_##t1((t1)value);                          \
    atomic_store((_Atomic volatile t1*)&mem->data[addr], wrapped);            \
  }                                                                           \
  DEF_MEM_CHECKS1(name##_shared, _shared_, t1, , void, t2)

DEFINE_ATOMIC_STORE(i32_atomic_store, u32, u32)
DEFINE_ATOMIC_STORE(i64_atomic_store, u64, u64)
DEFINE_ATOMIC_STORE(i32_atomic_store8, u8, u32)
DEFINE_ATOMIC_STORE(i32_atomic_store16, u16, u32)
DEFINE_ATOMIC_STORE(i64_atomic_store8, u8, u64)
DEFINE_ATOMIC_STORE(i64_atomic_store16, u16, u64)
DEFINE_ATOMIC_STORE(i64_atomic_store32, u32, u64)

#if WABT_BIG_ENDIAN
// Since native read-modify-write  operations expect values to have the host
// endianness, we can use them. So we have to do this in multiple steps, while
// checking that the value hasn't changed in the middle.
#define WASM_RT_ATOMIC_RMW(t, ptr, value, opname, op, ret)    \
  do {                                                        \
    t raw = atomic_load(ptr);                                 \
    while (1) {                                               \
      ret = WASM_ADJUST_ENDIAN_##t(raw);                      \
      t new_raw = WASM_ADJUST_ENDIAN_##t((ret)op(value));     \
      if (atomic_compare_exchange_weak(ptr, &raw, new_raw)) { \
        break;                                                \
      }                                                       \
    }                                                         \
  } while (0)
#else
#define WASM_RT_ATOMIC_RMW(t, ptr, value, opname, op, ret) \
  ret = atomic_##opname(ptr, value)
#endif

#define DEFINE_ATOMIC_RMW(name, opname, op, t1, t2)                           \
  static inline t2 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_memory_t* mem, u64 addr,          \
                                    t2 value) {                               \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 wrapped = (t1)value;                                                   \
    t1 ret;                                                                   \
    wasm_rt_memcpy(&ret, MEM_ADDR_MEMOP(mem, addr), sizeof(t1));              \
    ret = WASM_ADJUST_ENDIAN_##t1(ret);                                       \
    t1 newval = WASM_ADJUST_ENDIAN_##t1(ret op wrapped);                      \
    wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr), &newval, sizeof(t1));           \
    return (t2)ret;                                                           \
  }                                                                           \
  DEF_MEM_CHECKS1(name, _, t1, return, t2, t2)                                \
  static inline t2 name##_shared_unchecked(                                   \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr, t2 value) {                                                   \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                         \
    t1 wrapped = (t1)value;                                                   \
    t1 ret;                                                                   \
    WASM_RT_ATOMIC_RMW(t1, (_Atomic volatile t1*)&mem->data[addr], wrapped,   \
                       opname, op, ret);                                      \
    return (t2)ret;                                                           \
  }                                                                           \
  DEF_MEM_CHECKS1(name##_shared, _shared_, t1, return, t2, t2)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_add_u, fetch_add, +, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_add_u, fetch_add, +, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_add, fetch_add, +, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_add_u, fetch_add, +, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_add_u, fetch_add, +, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_add_u, fetch_add, +, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_add, fetch_add, +, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_sub_u, fetch_sub, -, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_sub_u, fetch_sub, -, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_sub, fetch_sub, -, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_sub_u, fetch_sub, -, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_sub_u, fetch_sub, -, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_sub_u, fetch_sub, -, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_sub, fetch_sub, -, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_and_u, fetch_and, &, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_and_u, fetch_and, &, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_and, fetch_and, &, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_and_u, fetch_and, &, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_and_u, fetch_and, &, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_and_u, fetch_and, &, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_and, fetch_and, &, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_or_u, fetch_or, |, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_or_u, fetch_or, |, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_or, fetch_or, |, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_or_u, fetch_or, |, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_or_u, fetch_or, |, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_or_u, fetch_or, |, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_or, fetch_or, |, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_xor_u, fetch_xor, ^, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_xor_u, fetch_xor, ^, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_xor, fetch_xor, ^, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_xor_u, fetch_xor, ^, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_xor_u, fetch_xor, ^, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_xor_u, fetch_xor, ^, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_xor, fetch_xor, ^, u64, u64)

#define DEFINE_ATOMIC_XCHG(name, opname, t1, t2)                               \
  static inline t2 name##_unchecked(uint8_t* const wasm_rt_local_memory_base,  \
                                    wasm_rt_memory_t* mem, u64 addr,           \
                                    t2 value) {                                \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                          \
    t1 wrapped = WASM_ADJUST_ENDIAN_##t1((t1)value);                           \
    t1 ret;                                                                    \
    wasm_rt_memcpy(&ret, MEM_ADDR_MEMOP(mem, addr), sizeof(t1));               \
    wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr), &wrapped, sizeof(t1));           \
    return (t2)WASM_ADJUST_ENDIAN_##t1(ret);                                   \
  }                                                                            \
  DEF_MEM_CHECKS1(name, _, t1, return, t2, t2)                                 \
  static inline t2 name##_shared_unchecked(                                    \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem,  \
      u64 addr, t2 value) {                                                    \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                          \
    t1 wrapped = WASM_ADJUST_ENDIAN_##t1((t1)value);                           \
    t1 ret = atomic_##opname((_Atomic volatile t1*)&mem->data[addr], wrapped); \
    return (t2)WASM_ADJUST_ENDIAN_##t1(ret);                                   \
  }                                                                            \
  DEF_MEM_CHECKS1(name##_shared, _shared_, t1, return, t2, t2)

DEFINE_ATOMIC_XCHG(i32_atomic_rmw8_xchg_u, exchange, u8, u32)
DEFINE_ATOMIC_XCHG(i32_atomic_rmw16_xchg_u, exchange, u16, u32)
DEFINE_ATOMIC_XCHG(i32_atomic_rmw_xchg, exchange, u32, u32)
DEFINE_ATOMIC_XCHG(i64_atomic_rmw8_xchg_u, exchange, u8, u64)
DEFINE_ATOMIC_XCHG(i64_atomic_rmw16_xchg_u, exchange, u16, u64)
DEFINE_ATOMIC_XCHG(i64_atomic_rmw32_xchg_u, exchange, u32, u64)
DEFINE_ATOMIC_XCHG(i64_atomic_rmw_xchg, exchange, u64, u64)

#define DEFINE_ATOMIC_CMP_XCHG(name, t1, t2)                                  \
  static inline t1 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_memory_t* mem, u64 addr,          \
                                    t1 expected, t1 replacement) {            \
    ATOMIC_ALIGNMENT_CHECK(addr, t2);                                         \
    t2 expected_wrapped = WASM_ADJUST_ENDIAN_##t2((t2)expected);              \
    t2 replacement_wrapped = WASM_ADJUST_ENDIAN_##t2((t2)replacement);        \
    t2 ret;                                                                   \
    wasm_rt_memcpy(&ret, MEM_ADDR_MEMOP(mem, addr), sizeof(t2));              \
    if (ret == expected_wrapped) {                                            \
      wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr), &replacement_wrapped,         \
                     sizeof(t2));                                             \
    }                                                                         \
    return (t1)WASM_ADJUST_ENDIAN_##t2(ret);                                  \
  }                                                                           \
  DEF_MEM_CHECKS2(name, _, t2, return, t1, t1, t1)                            \
  static inline t1 name##_shared_unchecked(                                   \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_shared_memory_t* mem, \
      u64 addr, t1 expected, t1 replacement) {                                \
    ATOMIC_ALIGNMENT_CHECK(addr, t2);                                         \
    t2 expected_wrapped = WASM_ADJUST_ENDIAN_##t2((t2)expected);              \
    t2 replacement_wrapped = WASM_ADJUST_ENDIAN_##t2((t2)replacement);        \
    atomic_compare_exchange_strong((_Atomic volatile t2*)&mem->data[addr],    \
                                   &expected_wrapped, replacement_wrapped);   \
    return (t1)WASM_ADJUST_ENDIAN_##t2(expected_wrapped);                     \
  }                                                                           \
  DEF_MEM_CHECKS2(name##_shared, _shared_, t2, return, t1, t1, t1)

DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw8_cmpxchg_u, u32, u8);
DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw16_cmpxchg_u, u32, u16);
DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw_cmpxchg, u32, u32);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw8_cmpxchg_u, u64, u8);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw16_cmpxchg_u, u64, u16);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw32_cmpxchg_u, u64, u32);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw_cmpxchg, u64, u64);

#define atomic_fence() atomic_thread_fence(memory_order_seq_cst)
