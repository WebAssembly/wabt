#if defined(_MSC_VER)

#include <intrin.h>

// Use MSVC intrinsics

// For loads and stores, its not clear if we can rely on register width loads
// and stores to be atomic as reported here
// https://learn.microsoft.com/en-us/windows/win32/sync/interlocked-variable-access?redirectedfrom=MSDN
// or if we have to reuse other instrinsics
// https://stackoverflow.com/questions/42660091/atomic-load-in-c-with-msvc
// We reuse other intrinsics to be cautious
#define atomic_load_u8(a) _InterlockedOr8(a, 0)
#define atomic_load_u16(a) _InterlockedOr16(a, 0)
#define atomic_load_u32(a) _InterlockedOr(a, 0)
#define atomic_load_u64(a) _InterlockedOr64(a, 0)

#define atomic_store_u8(a, v) _InterlockedExchange8(a, v)
#define atomic_store_u16(a, v) _InterlockedExchange16(a, v)
#define atomic_store_u32(a, v) _InterlockedExchange(a, v)
#define atomic_store_u64(a, v) _InterlockedExchange64(a, v)

#define atomic_add_u8(a, v) _InterlockedExchangeAdd8(a, v)
#define atomic_add_u16(a, v) _InterlockedExchangeAdd16(a, v)
#define atomic_add_u32(a, v) _InterlockedExchangeAdd(a, v)
#define atomic_add_u64(a, v) _InterlockedExchangeAdd64(a, v)

#define atomic_sub_u8(a, v) _InterlockedExchangeAdd8(a, -(v))
#define atomic_sub_u16(a, v) _InterlockedExchangeAdd16(a, -(v))
#define atomic_sub_u32(a, v) _InterlockedExchangeAdd(a, -(v))
#define atomic_sub_u64(a, v) _InterlockedExchangeAdd64(a, -(v))

#define atomic_and_u8(a, v) _InterlockedAnd8(a, v)
#define atomic_and_u16(a, v) _InterlockedAnd16(a, v)
#define atomic_and_u32(a, v) _InterlockedAnd(a, v)
#define atomic_and_u64(a, v) _InterlockedAnd64(a, v)

#define atomic_or_u8(a, v) _InterlockedOr8(a, v)
#define atomic_or_u16(a, v) _InterlockedOr16(a, v)
#define atomic_or_u32(a, v) _InterlockedOr(a, v)
#define atomic_or_u64(a, v) _InterlockedOr64(a, v)

#define atomic_xor_u8(a, v) _InterlockedXor8(a, v)
#define atomic_xor_u16(a, v) _InterlockedXor16(a, v)
#define atomic_xor_u32(a, v) _InterlockedXor(a, v)
#define atomic_xor_u64(a, v) _InterlockedXor64(a, v)

#define atomic_exchange_u8(a, v) _InterlockedExchange8(a, v)
#define atomic_exchange_u16(a, v) _InterlockedExchange16(a, v)
#define atomic_exchange_u32(a, v) _InterlockedExchange(a, v)
#define atomic_exchange_u64(a, v) _InterlockedExchange64(a, v)

// clang-format off
#define atomic_compare_exchange_u8(a, expected_ptr, desired) _InterlockedCompareExchange8(a, desired, *(expected_ptr))
#define atomic_compare_exchange_u16(a, expected_ptr, desired) _InterlockedCompareExchange16(a, desired, *(expected_ptr))
#define atomic_compare_exchange_u32(a, expected_ptr, desired) _InterlockedCompareExchange(a, desired, *(expected_ptr))
#define atomic_compare_exchange_u64(a, expected_ptr, desired) _InterlockedCompareExchange64(a, desired, *(expected_ptr))
// clang-format on

#define atomic_fence() _ReadWriteBarrier()

#else

// Use gcc/clang/icc intrinsics
#define atomic_load_u8(a) __atomic_load_n((u8*)(a), __ATOMIC_SEQ_CST)
#define atomic_load_u16(a) __atomic_load_n((u16*)(a), __ATOMIC_SEQ_CST)
#define atomic_load_u32(a) __atomic_load_n((u32*)(a), __ATOMIC_SEQ_CST)
#define atomic_load_u64(a) __atomic_load_n((u64*)(a), __ATOMIC_SEQ_CST)

#define atomic_store_u8(a, v) __atomic_store_n((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_store_u16(a, v) __atomic_store_n((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_store_u32(a, v) __atomic_store_n((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_store_u64(a, v) __atomic_store_n((u64*)(a), v, __ATOMIC_SEQ_CST)

#define atomic_add_u8(a, v) __atomic_fetch_add((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_add_u16(a, v) __atomic_fetch_add((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_add_u32(a, v) __atomic_fetch_add((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_add_u64(a, v) __atomic_fetch_add((u64*)(a), v, __ATOMIC_SEQ_CST)

#define atomic_sub_u8(a, v) __atomic_fetch_sub((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_sub_u16(a, v) __atomic_fetch_sub((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_sub_u32(a, v) __atomic_fetch_sub((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_sub_u64(a, v) __atomic_fetch_sub((u64*)(a), v, __ATOMIC_SEQ_CST)

#define atomic_and_u8(a, v) __atomic_fetch_and((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_and_u16(a, v) __atomic_fetch_and((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_and_u32(a, v) __atomic_fetch_and((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_and_u64(a, v) __atomic_fetch_and((u64*)(a), v, __ATOMIC_SEQ_CST)

#define atomic_or_u8(a, v) __atomic_fetch_or((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_or_u16(a, v) __atomic_fetch_or((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_or_u32(a, v) __atomic_fetch_or((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_or_u64(a, v) __atomic_fetch_or((u64*)(a), v, __ATOMIC_SEQ_CST)

#define atomic_xor_u8(a, v) __atomic_fetch_xor((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_xor_u16(a, v) __atomic_fetch_xor((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_xor_u32(a, v) __atomic_fetch_xor((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_xor_u64(a, v) __atomic_fetch_xor((u64*)(a), v, __ATOMIC_SEQ_CST)

// clang-format off
#define atomic_exchange_u8(a, v) __atomic_exchange_n((u8*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_exchange_u16(a, v) __atomic_exchange_n((u16*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_exchange_u32(a, v) __atomic_exchange_n((u32*)(a), v, __ATOMIC_SEQ_CST)
#define atomic_exchange_u64(a, v) __atomic_exchange_n((u64*)(a), v, __ATOMIC_SEQ_CST)
// clang-format on

#define __atomic_compare_exchange_helper(a, expected_ptr, desired)        \
  (__atomic_compare_exchange_n(a, expected_ptr, desired, 0 /* is_weak */, \
                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST),       \
   *(expected_ptr))

// clang-format off
#define atomic_compare_exchange_u8(a, expected_ptr, desired) __atomic_compare_exchange_helper((u8*)(a), expected_ptr, desired)
#define atomic_compare_exchange_u16(a, expected_ptr, desired) __atomic_compare_exchange_helper((u16*)(a), expected_ptr, desired)
#define atomic_compare_exchange_u32(a, expected_ptr, desired) __atomic_compare_exchange_helper((u32*)(a), expected_ptr, desired)
#define atomic_compare_exchange_u64(a, expected_ptr, desired) __atomic_compare_exchange_helper((u64*)(a), expected_ptr, desired)
// clang-format on

#define atomic_fence() __atomic_thread_fence(__ATOMIC_SEQ_CST)

#endif

#define ATOMIC_ALIGNMENT_CHECK(addr, t1) \
  if (UNLIKELY(addr % sizeof(t1))) {     \
    TRAP(UNALIGNED);                     \
  }

#define DEFINE_ATOMIC_LOAD(name, t1, t2, t3, force_read)                  \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {                \
    MEMCHECK(mem, addr, t1);                                              \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                     \
    t1 result;                                                            \
    wasm_rt_memcpy(&result, MEM_ADDR(mem, addr, sizeof(t1)), sizeof(t1)); \
    result = atomic_load_##t1(MEM_ADDR(mem, addr, sizeof(t1)));           \
    force_read(result);                                                   \
    return (t3)(t2)result;                                                \
  }

DEFINE_ATOMIC_LOAD(i32_atomic_load, u32, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load, u64, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i32_atomic_load8_u, u8, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load8_u, u8, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i32_atomic_load16_u, u16, u32, u32, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load16_u, u16, u64, u64, FORCE_READ_INT)
DEFINE_ATOMIC_LOAD(i64_atomic_load32_u, u32, u64, u64, FORCE_READ_INT)

#define DEFINE_ATOMIC_STORE(name, t1, t2)                              \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                           \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                  \
    t1 wrapped = (t1)value;                                            \
    atomic_store_##t1(MEM_ADDR(mem, addr, sizeof(t1)), wrapped);       \
  }

DEFINE_ATOMIC_STORE(i32_atomic_store, u32, u32)
DEFINE_ATOMIC_STORE(i64_atomic_store, u64, u64)
DEFINE_ATOMIC_STORE(i32_atomic_store8, u8, u32)
DEFINE_ATOMIC_STORE(i32_atomic_store16, u16, u32)
DEFINE_ATOMIC_STORE(i64_atomic_store8, u8, u64)
DEFINE_ATOMIC_STORE(i64_atomic_store16, u16, u64)
DEFINE_ATOMIC_STORE(i64_atomic_store32, u32, u64)

#define DEFINE_ATOMIC_RMW(name, op, t1, t2)                                \
  static inline t2 name(wasm_rt_memory_t* mem, u64 addr, t2 value) {       \
    MEMCHECK(mem, addr, t1);                                               \
    ATOMIC_ALIGNMENT_CHECK(addr, t1);                                      \
    t1 wrapped = (t1)value;                                                \
    t1 ret = atomic_##op##_##t1(MEM_ADDR(mem, addr, sizeof(t1)), wrapped); \
    return (t2)ret;                                                        \
  }

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_add_u, add, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_add_u, add, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_add, add, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_add_u, add, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_add_u, add, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_add_u, add, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_add, add, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_sub_u, sub, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_sub_u, sub, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_sub, sub, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_sub_u, sub, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_sub_u, sub, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_sub_u, sub, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_sub, sub, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_and_u, and, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_and_u, and, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_and, and, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_and_u, and, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_and_u, and, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_and_u, and, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_and, and, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_or_u, or, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_or_u, or, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_or, or, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_or_u, or, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_or_u, or, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_or_u, or, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_or, or, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_xor_u, xor, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_xor_u, xor, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_xor, xor, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_xor_u, xor, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_xor_u, xor, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_xor_u, xor, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_xor, xor, u64, u64)

DEFINE_ATOMIC_RMW(i32_atomic_rmw8_xchg_u, exchange, u8, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw16_xchg_u, exchange, u16, u32)
DEFINE_ATOMIC_RMW(i32_atomic_rmw_xchg, exchange, u32, u32)
DEFINE_ATOMIC_RMW(i64_atomic_rmw8_xchg_u, exchange, u8, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw16_xchg_u, exchange, u16, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw32_xchg_u, exchange, u32, u64)
DEFINE_ATOMIC_RMW(i64_atomic_rmw_xchg, exchange, u64, u64)

#define DEFINE_ATOMIC_CMP_XCHG(name, t1, t2)                                  \
  static inline t1 name(wasm_rt_memory_t* mem, u64 addr, t1 expected,         \
                        t1 replacement) {                                     \
    MEMCHECK(mem, addr, t2);                                                  \
    ATOMIC_ALIGNMENT_CHECK(addr, t2);                                         \
    t2 expected_wrapped = (t2)expected;                                       \
    t2 replacement_wrapped = (t2)replacement;                                 \
    t2 old =                                                                  \
        atomic_compare_exchange_##t2(MEM_ADDR(mem, addr, sizeof(t2)),         \
                                     &expected_wrapped, replacement_wrapped); \
    return (t1)old;                                                           \
  }

DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw8_cmpxchg_u, u32, u8);
DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw16_cmpxchg_u, u32, u16);
DEFINE_ATOMIC_CMP_XCHG(i32_atomic_rmw_cmpxchg, u32, u32);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw8_cmpxchg_u, u64, u8);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw16_cmpxchg_u, u64, u16);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw32_cmpxchg_u, u64, u32);
DEFINE_ATOMIC_CMP_XCHG(i64_atomic_rmw_cmpxchg, u64, u64);
