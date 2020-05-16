
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wasm-rt.h"
#include "wasm-rt-impl.h"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#define MEMCHECK(mem, a, t)  \
  if (UNLIKELY((a) + sizeof(t) > mem->size)) TRAP(OOB)

#define DEFINE_LOAD(name, t1, t2, t3)              \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {   \
    MEMCHECK(mem, addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, &mem->data[addr], sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_STORE(name, t1, t2)                           \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(&mem->data[addr], &wrapped, sizeof(t1));          \
  }

DEFINE_LOAD(i32_load, u32, u32, u32);
DEFINE_LOAD(i64_load, u64, u64, u64);
DEFINE_LOAD(f32_load, f32, f32, f32);
DEFINE_LOAD(f64_load, f64, f64, f64);
DEFINE_LOAD(i32_load8_s, s8, s32, u32);
DEFINE_LOAD(i64_load8_s, s8, s64, u64);
DEFINE_LOAD(i32_load8_u, u8, u32, u32);
DEFINE_LOAD(i64_load8_u, u8, u64, u64);
DEFINE_LOAD(i32_load16_s, s16, s32, u32);
DEFINE_LOAD(i64_load16_s, s16, s64, u64);
DEFINE_LOAD(i32_load16_u, u16, u32, u32);
DEFINE_LOAD(i64_load16_u, u16, u64, u64);
DEFINE_LOAD(i64_load32_s, s32, s64, u64);
DEFINE_LOAD(i64_load32_u, u32, u64, u64);
DEFINE_STORE(i32_store, u32, u32);
DEFINE_STORE(i64_store, u64, u64);
DEFINE_STORE(f32_store, f32, f32);
DEFINE_STORE(f64_store, f64, f64);
DEFINE_STORE(i32_store8, u8, u32);
DEFINE_STORE(i32_store16, u16, u32);
DEFINE_STORE(i64_store8, u8, u64);
DEFINE_STORE(i64_store16, u16, u64);
DEFINE_STORE(i64_store32, u32, u64);

// Imports

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */

void _Z_wasi_snapshot_preview1Z_proc_exitZ_vi(u32 x) {
  exit(x);
}
void (*Z_wasi_snapshot_preview1Z_proc_exitZ_vi)(u32) = _Z_wasi_snapshot_preview1Z_proc_exitZ_vi;

u32 _Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii(u32 fd, u32 iov, u32 iovcnt, u32 pnum) {
  // TODO full file support
  if (fd != 1) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = i32_load(Z_memory, iov + i * 8);
    u32 len = i32_load(Z_memory, iov + i * 8 + 4);
    for (u32 j = 0; j < len; j++) {
      putchar(i32_load8_u(Z_memory, ptr + j));
    }
    num += len;
  }
  i32_store(Z_memory, pnum, num);
  return 0;
}
u32 (*Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii)(u32, u32, u32, u32) = _Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii;

u32 _Z_wasi_snapshot_preview1Z_fd_closeZ_ii(u32 fd) {
  // TODO full file support
  if (fd != 1) {
    return WASI_DEFAULT_ERROR;
  }
  return 0;
}
u32 (*Z_wasi_snapshot_preview1Z_fd_closeZ_ii)(u32) = _Z_wasi_snapshot_preview1Z_fd_closeZ_ii;

u32 _Z_wasi_snapshot_preview1Z_fd_seekZ_iijii(u32 x, u64 y, u32 z, u32 w) {
  // TODO full file support
  return WASI_DEFAULT_ERROR;
}
u32 (*Z_wasi_snapshot_preview1Z_fd_seekZ_iijii)(u32, u64, u32, u32) = _Z_wasi_snapshot_preview1Z_fd_seekZ_iijii;

u32 _Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji(u32 clock_id, u64 max_lag, u32 out) {
  // TODO: handle realtime vs monotonic etc.
  // wasi expects a result in nanoseconds, and we know how to convert clock()
  // to seconds, so compute from there
  const double NSEC_PER_SEC = 1000.0 * 1000.0 * 1000.0;
  i64_store(Z_memory, out, (u64)(clock() / (CLOCKS_PER_SEC / NSEC_PER_SEC)));
  return 0;
}

u32 (*Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji)(u32, u64, u32) = _Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji;


// Main

int main(int argc, char** argv) {
  init();

  if (setjmp(g_jmp_buf) != 0) {
    puts("[wasm trap, halting]");
  } else {
    return Z_mainZ_iii(0, 0);
  }
}
