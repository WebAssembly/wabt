
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

#define IMPORT_IMPL(ret, name, params, body) \
ret _##name params { \
  body \
} \
ret (*name) params = _##name;

#define STUB_IMPORT_IMPL(ret, name, params, errorcode) IMPORT_IMPL(ret, name, params, { return errorcode; });

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
  exit(x);
});

static FILE* FS_get_stream(u32 fd) {
  if (fd == 1) {
    return stdout;
  } else if (fd == 2) {
    return stderr;
  }
  return NULL;
}

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  // TODO full file handling
  FILE* stream = FS_get_stream(fd);
  if (!stream) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = i32_load(Z_memory, iov + i * 8);
    u32 len = i32_load(Z_memory, iov + i * 8 + 4);
    fwrite(Z_memory->data + ptr, 1, len, stream);
    num += len;
  }
  i32_store(Z_memory, pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (u32 fd), {
  // TODO full file support
  FILE* stream = FS_get_stream(fd);
  if (!stream) {
    return WASI_DEFAULT_ERROR;
  }
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (u32 pcount, u32 pbuf_size), {
  // TODO: connect to actual env?
  i32_store(Z_memory, pcount, 0);
  i32_store(Z_memory, pbuf_size, 0);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (u32 __environ, u32 environ_buf), {
  // TODO: connect to actual env?
  return 0;
});

STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iijii, (u32 a, u64 b, u32 c, u32 d), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_fdstat_getZ_iii, (u32 a, u32 b), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_syncZ_ii, (u32 a), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (u32 a, u32 b, u32 c, u32 d), WASI_DEFAULT_ERROR);

// TODO: set errno for these
STUB_IMPORT_IMPL(u32, Z_envZ_dlopenZ_iii, (u32 a, u32 b), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlcloseZ_ii, (u32 a), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlsymZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_dlerrorZ_iv, (), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_signalZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_systemZ_ii, (u32 a), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_utimesZ_iii, (u32 a, u32 b), -1);

#define EM_EACCES 2

STUB_IMPORT_IMPL(u32, Z_envZ___sys_unlinkZ_ii, (u32 a), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_rmdirZ_ii, (u32 a), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_renameZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_lstat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup3Z_iiii, (u32 a, u32 b, u32 c), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup2Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_stat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_accessZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_getcwdZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_fstat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_ftruncate64Z_iiiii, (u32 a, u32 b, u32 c, u32 d), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_readZ_iiii, (u32 a, u32 b, u32 c), EM_EACCES);

STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_initZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_settypeZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_destroyZ_ii, (u32 a), 0);

// TODO Z_envZ_emscripten_longjmpZ_vii
//      Z_envZ_saveSetjmpZ_iiiii
//      Z_envZ_testSetjmpZ_iiii
//      Z_envZ_getTempRet0Z_iv
//      Z_envZ_setTempRet0Z_vi
//      Z_envZ_invoke_viiZ_viii
//
//      Z_wasi_snapshot_preview1Z_args_sizes_getZ_iii
//      Z_wasi_snapshot_preview1Z_args_getZ_iii

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (u32 clock_id, u64 max_lag, u32 out), {
  // TODO: handle realtime vs monotonic etc.
  // wasi expects a result in nanoseconds, and we know how to convert clock()
  // to seconds, so compute from there
  const double NSEC_PER_SEC = 1000.0 * 1000.0 * 1000.0;
  i64_store(Z_memory, out, (u64)(clock() / (CLOCKS_PER_SEC / NSEC_PER_SEC)));
  return 0;
});

// Main

int main(int argc, char** argv) {
  init();

  int trap_code;
  if ((trap_code = setjmp(g_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    return 1;
  } else {
    return Z_mainZ_iii(0, 0);
  }
}
