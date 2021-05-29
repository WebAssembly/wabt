// This file contains modified versions of the emscripten runtime (as well as the wasi support it includes ) adapted for
//    - library sandboxing --- these apis have to support multiple wasm2c sandboxed components and cannot use global variables.
//                             Instead they operate on the sbx context specified as the first parameter to the API
//    - compatibility --- APIs like setjmp/longjmp are implemented in a very limited way upstream. Expanding this.
//    - security --- APIs like args_get, sys_open, fd_write, sys_read seemed to have security bugs upstream.
//                   Additionally, we don't want to allow any file system access. Restrict opens to the null file (/dev/null on unix, nul on windows)

/////////////////////////////////////////////////////////////
////////// File: Missing stuff from emscripten
/////////////////////////////////////////////////////////////
#include <stdint.h>
#include <time.h>

#if defined(__APPLE__) && defined(__MACH__)
  #include <sys/time.h>
  #include <mach/mach_time.h> /* mach_absolute_time */
  // #include <mach/mach.h>      /* host_get_clock_service, mach_... */
  // #include <mach/clock.h>     /* clock_get_time */
#endif

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

#ifndef WASM_RT_ADD_PREFIX
#define WASM_RT_ADD_PREFIX
#endif

/////////////////////////////////////////////////////////////
////////// File: Modified emscripten/tools/wasm2c/base.c
/////////////////////////////////////////////////////////////

/*
 * Base of all support for wasm2c code.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ssize_t detection: usually stdint provides it, but not on windows apparently
#ifdef _WIN32
#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else // _MSC_VER
#ifdef _WIN64
typedef signed long long ssize_t;
#else // _WIN64
typedef signed long ssize_t;
#endif // _WIN64
#endif // _MSC_VER
#endif // _WIN32

#include "wasm-rt.h"
#include "wasm-rt-impl.h"

#if defined(__GNUC__)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define UNLIKELY(x) (x)
#define LIKELY(x) (x)
#endif

#if defined(_WIN32)
#define POSIX_PREFIX(func) _##func
#else
#define POSIX_PREFIX(func) func
#endif

#define TRAP(x) wasm_rt_trap(WASM_RT_TRAP_##x)

#define MEMACCESS(mem, addr) ((void*)&(mem->data[addr]))

#define UNCOND_MEMCHECK_SIZE(mem, a, sz)  \
  if (UNLIKELY((a) + sz > mem->size)) TRAP(OOB)

#define UNCOND_MEMCHECK(mem, a, t)  UNCOND_MEMCHECK_SIZE(mem, a, sizeof(t))

#define DEFINE_WASI_LOAD(name, t1, t2, t3)              \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {   \
    UNCOND_MEMCHECK(mem, addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, MEMACCESS(mem, addr), sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_WASI_STORE(name, t1, t2)                           \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    UNCOND_MEMCHECK(mem, addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(MEMACCESS(mem, addr), &wrapped, sizeof(t1));          \
  }

DEFINE_WASI_LOAD(wasm_i32_load, u32, u32, u32);
DEFINE_WASI_LOAD(wasm_i64_load, u64, u64, u64);
DEFINE_WASI_LOAD(wasm_f32_load, f32, f32, f32);
DEFINE_WASI_LOAD(wasm_f64_load, f64, f64, f64);
DEFINE_WASI_LOAD(wasm_i32_load8_s, s8, s32, u32);
DEFINE_WASI_LOAD(wasm_i64_load8_s, s8, s64, u64);
DEFINE_WASI_LOAD(wasm_i32_load8_u, u8, u32, u32);
DEFINE_WASI_LOAD(wasm_i64_load8_u, u8, u64, u64);
DEFINE_WASI_LOAD(wasm_i32_load16_s, s16, s32, u32);
DEFINE_WASI_LOAD(wasm_i64_load16_s, s16, s64, u64);
DEFINE_WASI_LOAD(wasm_i32_load16_u, u16, u32, u32);
DEFINE_WASI_LOAD(wasm_i64_load16_u, u16, u64, u64);
DEFINE_WASI_LOAD(wasm_i64_load32_s, s32, s64, u64);
DEFINE_WASI_LOAD(wasm_i64_load32_u, u32, u64, u64);
DEFINE_WASI_STORE(wasm_i32_store, u32, u32);
DEFINE_WASI_STORE(wasm_i64_store, u64, u64);
DEFINE_WASI_STORE(wasm_f32_store, f32, f32);
DEFINE_WASI_STORE(wasm_f64_store, f64, f64);
DEFINE_WASI_STORE(wasm_i32_store8, u8, u32);
DEFINE_WASI_STORE(wasm_i32_store16, u16, u32);
DEFINE_WASI_STORE(wasm_i64_store8, u8, u64);
DEFINE_WASI_STORE(wasm_i64_store16, u16, u64);
DEFINE_WASI_STORE(wasm_i64_store32, u32, u64);

// Imports

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(...) { printf(__VA_ARGS__); }
#else
#define VERBOSE_LOG(...)
#endif

#define IMPORT_IMPL(ret, name, params, body) \
static ret _##name params { \
  VERBOSE_LOG("[import: " #name "]\n"); \
  body \
} \
ret (*name) params = _##name;

#define STUB_IMPORT_IMPL(ret, name, params, returncode) IMPORT_IMPL(ret, name, params, { return returncode; });

#define BILLION 1000000000L

// Generic abort method for a runtime error in the runtime.

static void abort_with_message(const char* message) {
  fprintf(stderr, "%s\n", message);
  TRAP(UNREACHABLE);
}

// This is a custom version of setjmp that is better than what is included in emscripten upstream
// This allows the app to invoke many setjmps and use any one of the prior jmp_buffs for longjmp
// This is done with adaquete checks to make sure this can't break sfi safety, and well bracketed control flow
// Emscripten upstream stores the value of the last longjmp in a setThrew --- this doesn't seem to be used anywhere
//    but preserving the behavior just in case

IMPORT_IMPL(void, Z_envZ_emscripten_longjmpZ_vii, (wasm_sandbox_wasi_data* wasi_data, u32 buf, u32 value), {
  // Check that at least one setjmp was called
  if (wasi_data->next_setjmp_index == 0) {
    abort_with_message("longjmp without setjmp");
  }

  // we use the setjmp buff itself to store the index to setjmp_stack we must use
  u32 buf_val = wasm_i32_load(wasi_data->heap_memory, buf);
  // check that index is one of the valid entries
  if (buf_val > wasi_data->next_setjmp_index) {
    abort_with_message("longjmp on an invalid jmp_buff");
  }

  // invalidate all setjmp buffers after index
  wasi_data->next_setjmp_index = buf_val;

  longjmp(wasi_data->setjmp_stack[buf_val], value);
});

// Not sure where the emscripten setjmp is, so not sure of the API format. Hopefully this is the correct name.
// Worst case, code will fail to compile; it will not be a runtime security issue.
IMPORT_IMPL(u32, Z_envZ_emscripten_setjmpZ_ii, (wasm_sandbox_wasi_data* wasi_data, u32 buf), {
  // Check that there is space in the setjmp stack
  u32 buf_val = wasi_data->next_setjmp_index;
  if (buf_val >= WASM2C_WASI_MAX_SETJMP_STACK) {
    abort_with_message("too many setjmps called");
  }

  // we use the setjmp buff itself to store the index to setjmp_stack we have chosen
  wasi_data->next_setjmp_index++;
  wasm_i32_store(wasi_data->heap_memory, buf, buf_val);
  return setjmp(wasi_data->setjmp_stack[buf_val]);
});

IMPORT_IMPL(void, Z_envZ_emscripten_notify_memory_growthZ_vi, (wasm_sandbox_wasi_data* wasi_data, u32 size), {});

IMPORT_IMPL(u32, Z_envZ_getTempRet0Z_iv, (wasm_sandbox_wasi_data* wasi_data), {
  return wasi_data->tempRet0;
});

IMPORT_IMPL(void, Z_envZ_setTempRet0Z_vi, (wasm_sandbox_wasi_data* wasi_data, u32 x), {
  wasi_data->tempRet0 = x;
});

// Shared OS support in both sandboxed and unsandboxed mode

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */
#define WASI_EINVAL 28

// Syscalls return a negative error code
#define EM_EACCES -2

STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_fdstat_getZ_iii, (u32 a, u32 b), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_syncZ_ii, (u32 a), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ_dlopenZ_iii, (u32 a, u32 b), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlcloseZ_ii, (u32 a), 1);
STUB_IMPORT_IMPL(u32, Z_envZ_dlsymZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_dlerrorZ_iv, (), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_signalZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_systemZ_ii, (u32 a), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_utimesZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_rmdirZ_ii, (u32 a), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_renameZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_lstat64Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup3Z_iiii, (u32 a, u32 b, u32 c), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_dup2Z_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_getcwdZ_iii, (u32 a, u32 b), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_ftruncate64Z_iiiii, (u32 a, u32 b, u32 c, u32 d), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_initZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_settypeZ_iii, (u32 a, u32 b), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_mutexattr_destroyZ_ii, (u32 a), 0);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_createZ_iiiii, (u32 a, u32 b, u32 c, u32 d), -1);
STUB_IMPORT_IMPL(u32, Z_envZ_pthread_joinZ_iii, (u32 a, u32 b), -1);
STUB_IMPORT_IMPL(u32, Z_envZ___cxa_thread_atexitZ_iiii, (u32 a, u32 b, u32 c), -1);

/////////////////////////////////////////////////////////////
////////// File: Modified emscripten/tools/wasm2c/main.c
/////////////////////////////////////////////////////////////

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_sizes_getZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 pargc, u32 pargv_buf_size), {
  wasm_i32_store(wasi_data->heap_memory, pargc, wasi_data->main_argc);
  u32 buf_size = 0;
  for (u32 i = 0; i < wasi_data->main_argc; i++) {
    buf_size += strlen(wasi_data->main_argv[i]) + 1;
  }
  wasm_i32_store(wasi_data->heap_memory, pargv_buf_size, buf_size);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_getZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 argv, u32 argv_buf), {
  u32 buf_size = 0;
  for (u32 i = 0; i < wasi_data->main_argc; i++) {
    u32 ptr = argv_buf + buf_size;
    wasm_i32_store(wasi_data->heap_memory, argv + i * 4, ptr);

    char* arg = wasi_data->main_argv[i];
    // need to check length of argv strings before copying
    // deliberately truncate length to u32, else UNCOND_MEMCHECK_SIZE may have overflows
    u32 len = strlen(arg) + 1;
    UNCOND_MEMCHECK_SIZE(wasi_data->heap_memory, ptr, len);

    memcpy(MEMACCESS(wasi_data->heap_memory, ptr), arg, len);
    // make sure string is null terminated
    wasm_i32_store8(wasi_data->heap_memory, ptr + (len - 1), 0);
    buf_size += len;
  }
  return 0;
});

/////////////////////////////////////////////////////////////
////////// File: Modified emscripten/tools/wasm2c/os.c
/////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (wasm_sandbox_wasi_data* wasi_data, u32 x), {
    // upstream emscripten implements this as exit(x)
    // This seems like a bad idea as a misbehaving sandbox will cause the app to exit
});

#define WASM_STDIN  0
#define WASM_STDOUT 1
#define WASM_STDERR 2


static void init_fds(wasm_sandbox_wasi_data* wasi_data) {
#ifndef _WIN32
  wasi_data->wasm_fd_to_native[WASM_STDIN] = STDIN_FILENO;
  wasi_data->wasm_fd_to_native[WASM_STDOUT] = STDOUT_FILENO;
  wasi_data->wasm_fd_to_native[WASM_STDERR] = STDERR_FILENO;
#else
  wasi_data->wasm_fd_to_native[WASM_STDIN] = _fileno(stdin);
  wasi_data->wasm_fd_to_native[WASM_STDOUT] = _fileno(stdout);
  wasi_data->wasm_fd_to_native[WASM_STDERR] = _fileno(stderr);
#endif
  wasi_data->next_wasm_fd = 3;
}

static u32 get_or_allocate_wasm_fd(wasm_sandbox_wasi_data* wasi_data, int nfd) {
  // If the native fd is already mapped, return the same wasm fd for it.
  for (uint32_t i = 0; i < wasi_data->next_wasm_fd; i++) {
    if (wasi_data->wasm_fd_to_native[i] == nfd) {
      return i;
    }
  }
  u32 fd = wasi_data->next_wasm_fd;
  if (fd >= WASM2C_WASI_MAX_FDS) {
    abort_with_message("ran out of fds");
  }
  wasi_data->wasm_fd_to_native[fd] = nfd;
  wasi_data->next_wasm_fd++;
  return fd;
}

static int get_native_fd(wasm_sandbox_wasi_data* wasi_data, u32 fd) {
  if (fd >= WASM2C_WASI_MAX_FDS || fd >= wasi_data->next_wasm_fd) {
    return -1;
  }
  return wasi_data->wasm_fd_to_native[fd];
}

static const char* get_null_file_path()
{
  #ifdef _WIN32
    const char* null_file = "nul";
  #else
    const char* null_file = "/dev/null";
  #endif
  return null_file;
}

static int get_null_file_mode()
{
  #ifdef _WIN32
    const int null_file_mode = _S_IREAD | _S_IWRITE;
  #else
    const int null_file_mode = S_IRUSR | S_IWUSR;
  #endif
  return null_file_mode;
}

static int get_null_file_flags()
{
  #ifdef _WIN32
    const int null_file_flags = _O_CREAT;
  #else
    const int null_file_flags = O_CREAT;
  #endif
  return null_file_flags;
}

static int is_null_file(wasm_sandbox_wasi_data* wasi_data, u32 path) {
  const char* tainted_string = (const char*) MEMACCESS(wasi_data->heap_memory, path);
  // deliberately truncate length to u32, else UNCOND_MEMCHECK_SIZE may have overflows
  u32 path_length = strlen(tainted_string) + 1;
  UNCOND_MEMCHECK_SIZE(wasi_data->heap_memory, path, path_length);

  const char* null_file = get_null_file_path();
  size_t null_file_len = strlen(null_file) + 1;
  if (null_file_len != path_length) {
    return 0;
  }

  if (memcmp(null_file, tainted_string, null_file_len) != 0) {
    return 0;
  }

  return 1;
}

IMPORT_IMPL(u32, Z_envZ___sys_accessZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 pathname, u32 mode), {
  VERBOSE_LOG("  access: %s 0x%x\n", MEMACCESS(wasi_data->heap_memory, pathname), mode);

  // only permit access checks on the the null file
  if (!is_null_file(wasi_data, pathname)) {
    return -1;
  }

  const char* null_file = get_null_file_path();
  const int null_file_mode = get_null_file_mode();

  int result = POSIX_PREFIX(access)(null_file, null_file_mode);
  if (result < 0) {
    VERBOSE_LOG("    access error: %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return 0;
});


IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (wasm_sandbox_wasi_data* wasi_data, u32 path, u32 flags, u32 varargs), {
  VERBOSE_LOG("  open: %s %d %d\n", MEMACCESS(wasi_data->heap_memory, path), flags, wasm_i32_load(varargs));

  // only permit opening the null file
  if (!is_null_file(wasi_data, path)) {
    return -1;
  }

  const char* null_file = get_null_file_path();
  const int null_file_flags = get_null_file_flags();
  const int null_file_mode = get_null_file_mode();

  // Specify the mode manually to make sure the sandboxed code is not using unusual values
  int nfd = POSIX_PREFIX(open)(null_file, null_file_flags, null_file_mode);
  VERBOSE_LOG("    => native %d\n", nfd);
  if (nfd >= 0) {
    u32 fd = get_or_allocate_wasm_fd(wasi_data, nfd);
    VERBOSE_LOG("      => wasm %d\n", fd);
    return fd;
  }
  return -1;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (wasm_sandbox_wasi_data* wasi_data, u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(wasi_data, fd);
  VERBOSE_LOG("  fd_write wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }

  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = wasm_i32_load(wasi_data->heap_memory, iov + i * 8);
    u32 len = wasm_i32_load(wasi_data->heap_memory, iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);

    UNCOND_MEMCHECK_SIZE(wasi_data->heap_memory, ptr, len);

    ssize_t result;
    // Use stdio for stdout/stderr to avoid mixing a low-level write() with
    // other logging code, which can change the order from the expected.
    if (fd == WASM_STDOUT) {
      result = fwrite(MEMACCESS(wasi_data->heap_memory, ptr), 1, len, stdout);
    } else if (fd == WASM_STDERR) {
      result = fwrite(MEMACCESS(wasi_data->heap_memory, ptr), 1, len, stderr);
    } else {
      result = POSIX_PREFIX(write)(nfd, MEMACCESS(wasi_data->heap_memory, ptr), len);
    }
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    if (result != len) {
      VERBOSE_LOG("    amount error, %ld %d\n", result, len);
      return WASI_DEFAULT_ERROR;
    }
    num += len;
  }
  VERBOSE_LOG("    success: %d\n", num);
  wasm_i32_store(wasi_data->heap_memory, pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (wasm_sandbox_wasi_data* wasi_data, u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(wasi_data, fd);
  VERBOSE_LOG("  fd_read wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }

  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = wasm_i32_load(wasi_data->heap_memory, iov + i * 8);
    u32 len = wasm_i32_load(wasi_data->heap_memory, iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);

    UNCOND_MEMCHECK_SIZE(wasi_data->heap_memory, ptr, len);

    ssize_t result = POSIX_PREFIX(read)(nfd, MEMACCESS(wasi_data->heap_memory, ptr), len);
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    num += result;
    if (result != len) {
      break; // nothing more to read
    }
  }
  VERBOSE_LOG("    success: %d\n", num);
  wasm_i32_store(wasi_data->heap_memory, pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (wasm_sandbox_wasi_data* wasi_data, u32 fd), {
  int nfd = get_native_fd(wasi_data, fd);
  VERBOSE_LOG("  close wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  POSIX_PREFIX(close)(nfd);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 pcount, u32 pbuf_size), {
  // TODO: Allow the sandbox to have its own env
  wasm_i32_store(wasi_data->heap_memory, pcount, 0);
  wasm_i32_store(wasi_data->heap_memory, pbuf_size, 0);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 __environ, u32 environ_buf), {
  // TODO: Allow the sandbox to have its own env
  return 0;
});

static int whence_to_native(u32 whence) {
  if (whence == 0) return SEEK_SET;
  if (whence == 1) return SEEK_CUR;
  if (whence == 2) return SEEK_END;
  return -1;
}

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iijii, (wasm_sandbox_wasi_data* wasi_data, u32 fd, u64 offset, u32 whence, u32 new_offset), {
  int nfd = get_native_fd(wasi_data, fd);
  int nwhence = whence_to_native(whence);
  VERBOSE_LOG("  seek %d (=> native %d) %ld %d (=> %d) %d\n", fd, nfd, offset, whence, nwhence, new_offset);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }

  // For additional safety don't allow seeking on the input, output and error streams
  if (nfd == WASM_STDOUT || nfd == WASM_STDERR || nfd == WASM_STDIN) {
      return WASI_DEFAULT_ERROR;
  }

  off_t off = POSIX_PREFIX(lseek)(nfd, offset, nwhence);
  VERBOSE_LOG("    off: %ld\n", off);
  if (off == (off_t)-1) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return WASI_DEFAULT_ERROR;
  }
  wasm_i64_store(wasi_data->heap_memory, new_offset, off);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iiiiii, (wasm_sandbox_wasi_data* wasi_data, u32 a, u32 b, u32 c, u32 d, u32 e), {
  return Z_wasi_snapshot_preview1Z_fd_seekZ_iijii(wasi_data, a, b + (((u64)c) << 32), d, e);
});

// TODO: set errno in wasm for things that need it

IMPORT_IMPL(u32, Z_envZ___sys_unlinkZ_ii, (wasm_sandbox_wasi_data* wasi_data, u32 path), {
  return EM_EACCES;
});

static u32 do_stat(wasm_sandbox_wasi_data* wasi_data, int nfd, u32 buf) {
  struct stat nbuf;
  if (fstat(nfd, &nbuf)) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  VERBOSE_LOG("    success, size=%ld\n", nbuf.st_size);
  wasm_i32_store(wasi_data->heap_memory, buf + 0, nbuf.st_dev);
  wasm_i32_store(wasi_data->heap_memory, buf + 4, 0);
  wasm_i32_store(wasi_data->heap_memory, buf + 8, nbuf.st_ino);
  wasm_i32_store(wasi_data->heap_memory, buf + 12, nbuf.st_mode);
  wasm_i32_store(wasi_data->heap_memory, buf + 16, nbuf.st_nlink);
  wasm_i32_store(wasi_data->heap_memory, buf + 20, nbuf.st_uid);
  wasm_i32_store(wasi_data->heap_memory, buf + 24, nbuf.st_gid);
  wasm_i32_store(wasi_data->heap_memory, buf + 28, nbuf.st_rdev);
  wasm_i32_store(wasi_data->heap_memory, buf + 32, 0);
  wasm_i64_store(wasi_data->heap_memory, buf + 40, nbuf.st_size);
#ifdef _WIN32
  wasm_i32_store(wasi_data->heap_memory, buf + 48, 512); // fixed blocksize on windows
  wasm_i32_store(wasi_data->heap_memory, buf + 52, 0);   // but no reported blocks...
#else
  wasm_i32_store(wasi_data->heap_memory, buf + 48, nbuf.st_blksize);
  wasm_i32_store(wasi_data->heap_memory, buf + 52, nbuf.st_blocks);
#endif
#if defined(__APPLE__) || defined(__NetBSD__)
  wasm_i32_store(wasi_data->heap_memory, buf + 56, nbuf.st_atimespec.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 60, nbuf.st_atimespec.tv_nsec);
  wasm_i32_store(wasi_data->heap_memory, buf + 64, nbuf.st_mtimespec.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 68, nbuf.st_mtimespec.tv_nsec);
  wasm_i32_store(wasi_data->heap_memory, buf + 72, nbuf.st_ctimespec.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 76, nbuf.st_ctimespec.tv_nsec);
#elif defined(_WIN32)
  wasm_i32_store(wasi_data->heap_memory, buf + 56, gmtime(&nbuf.st_atime)->tm_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 60, 0);
  wasm_i32_store(wasi_data->heap_memory, buf + 64, gmtime(&nbuf.st_mtime)->tm_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 68, 0);
  wasm_i32_store(wasi_data->heap_memory, buf + 72, gmtime(&nbuf.st_ctime)->tm_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 76, 0);
#else
  wasm_i32_store(wasi_data->heap_memory, buf + 56, nbuf.st_atim.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 60, nbuf.st_atim.tv_nsec);
  wasm_i32_store(wasi_data->heap_memory, buf + 64, nbuf.st_mtim.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 68, nbuf.st_mtim.tv_nsec);
  wasm_i32_store(wasi_data->heap_memory, buf + 72, nbuf.st_ctim.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, buf + 76, nbuf.st_ctim.tv_nsec);
#endif
  wasm_i64_store(wasi_data->heap_memory, buf + 80, nbuf.st_ino);
  return 0;
}

IMPORT_IMPL(u32, Z_envZ___sys_fstat64Z_iii, (wasm_sandbox_wasi_data* wasi_data, u32 fd, u32 buf), {
  int nfd = get_native_fd(wasi_data, fd);
  VERBOSE_LOG("  fstat64 %d (=> %d) %d\n", fd, nfd, buf);
  if (nfd < 0) {
    return EM_EACCES;
  }
  return do_stat(wasi_data, nfd, buf);
});

IMPORT_IMPL(u32, Z_envZ___sys_stat64Z_iii, (wasm_sandbox_wasi_data* wasi_data, u32 path, u32 buf), {
  VERBOSE_LOG("  stat64: %s\n", MEMACCESS(wasi_data->heap_memory, path));
  int fd = Z_envZ___sys_openZ_iiii(wasi_data, path, 0 /* read_only */, 0);
  int nfd = get_native_fd(wasi_data, fd);
  if (nfd < 0) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return do_stat(wasi_data, nfd, buf);
});

IMPORT_IMPL(u32, Z_envZ___sys_readZ_iiii, (wasm_sandbox_wasi_data* wasi_data, u32 fd, u32 buf, u32 count), {
  int nfd = get_native_fd(wasi_data, fd);
  VERBOSE_LOG("  read %d (=> %d) %d %d\n", fd, nfd, buf, count);
  if (nfd < 0) {
    VERBOSE_LOG("    bad fd\n");
    return EM_EACCES;
  }

  void* target_buf = MEMACCESS(wasi_data->heap_memory, buf);
  UNCOND_MEMCHECK_SIZE(wasi_data->heap_memory, buf, count);

  ssize_t ret = POSIX_PREFIX(read)(nfd, target_buf, count);
  VERBOSE_LOG("    native read: %ld\n", ret);
  if (ret < 0) {
    VERBOSE_LOG("    read error %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return ret;
});

#define WASM_CLOCK_REALTIME 0
#define WASM_CLOCK_MONOTONIC 1
#define WASM_CLOCK_PROCESS_CPUTIME 2
#define WASM_CLOCK_THREAD_CPUTIME_ID 3

static int check_clock(u32 clock_id) {
  return clock_id == WASM_CLOCK_REALTIME || clock_id == WASM_CLOCK_MONOTONIC ||
         clock_id == WASM_CLOCK_PROCESS_CPUTIME || clock_id == WASM_CLOCK_THREAD_CPUTIME_ID;
}

#if defined(__APPLE__) && defined(__MACH__)
  static mach_timebase_info_data_t timebase = { 0, 0 }; /* numer = 0, denom = 0 */
  static struct timespec           inittime = { 0, 0 }; /* nanoseconds since 1-Jan-1970 to init() */
  static uint64_t                  initclock;           /* ticks since boot to init() */
#endif

static void init_clock() {
  #if defined(__APPLE__) && defined(__MACH__)
    // From here: https://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x/21352348#21352348
    if (mach_timebase_info(&timebase) != 0) {
      abort();
    }

    // microseconds since 1 Jan 1970
    struct timeval micro;
    if (gettimeofday(&micro, NULL) != 0) {
      abort();
    }

    initclock = mach_absolute_time();

    inittime.tv_sec = micro.tv_sec;
    inittime.tv_nsec = micro.tv_usec * 1000;
  #endif
}

// out is a pointer index to a struct of the form
// // https://github.com/WebAssembly/wasi-libc/blob/659ff414560721b1660a19685110e484a081c3d4/libc-bottom-half/headers/public/__struct_timespec.h
// struct timespec {
//   // https://github.com/WebAssembly/wasi-libc/blob/659ff414560721b1660a19685110e484a081c3d4/libc-bottom-half/headers/public/__typedef_time_t.h
//   // time is long long in wasm32 which is an i64
//   time_t tv_sec
//   // long in wasm is an i32
//   long tv_nsec;
// }
IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (wasm_sandbox_wasi_data* wasi_data, u32 clock_id, u64 max_lag, u32 out), {
  if (!check_clock(clock_id)) {
    return WASI_EINVAL;
  }

  struct timespec out_struct;
  int ret = 0;

  #if defined(__APPLE__) && defined(__MACH__)
    // From here: https://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x/21352348#21352348

    // ticks since init
    u64 clock = mach_absolute_time() - initclock;
    // nanoseconds since init
    u64 nano = clock * (u64)timebase.numer / (u64)timebase.denom;
    out_struct = inittime;

    out_struct.tv_sec += nano / BILLION;
    out_struct.tv_nsec += nano % BILLION;
    // normalize
    out_struct.tv_sec += out_struct.tv_nsec / BILLION;
    out_struct.tv_nsec = out_struct.tv_nsec % BILLION;
  #else
    ret = clock_gettime(clock_id, &out_struct);
  #endif

  wasm_i64_store(wasi_data->heap_memory, out, (u64) out_struct.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, out + sizeof(u64), (u32) out_struct.tv_nsec);
  return ret;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_res_getZ_iii, (wasm_sandbox_wasi_data* wasi_data, u32 clock_id, u32 out), {
  if (!check_clock(clock_id)) {
    return WASI_EINVAL;
  }

  struct timespec out_struct;
  int ret = 0;
  #if defined(__APPLE__) && defined(__MACH__)
    out_struct.tv_sec = 0;
    out_struct.tv_nsec = 1;
  #else
    ret = clock_getres(clock_id, &out_struct);
  #endif

  wasm_i64_store(wasi_data->heap_memory, out, (u64) out_struct.tv_sec);
  wasm_i32_store(wasi_data->heap_memory, out + sizeof(u64), (u32) out_struct.tv_nsec);

  return ret;
});

void wasm_rt_init_wasi(wasm_sandbox_wasi_data* wasi_data) {
  init_clock();
  init_fds(wasi_data);
  // Remove unused function warnings
  (void) wasm_i32_load;
  (void) wasm_i64_load;
  (void) wasm_f32_load;
  (void) wasm_f64_load;
  (void) wasm_i32_load8_s;
  (void) wasm_i64_load8_s;
  (void) wasm_i32_load8_u;
  (void) wasm_i64_load8_u;
  (void) wasm_i32_load16_s;
  (void) wasm_i64_load16_s;
  (void) wasm_i32_load16_u;
  (void) wasm_i64_load16_u;
  (void) wasm_i64_load32_s;
  (void) wasm_i64_load32_u;
  (void) wasm_i32_store;
  (void) wasm_i64_store;
  (void) wasm_f32_store;
  (void) wasm_f64_store;
  (void) wasm_i32_store8;
  (void) wasm_i32_store16;
  (void) wasm_i64_store8;
  (void) wasm_i64_store16;
  (void) wasm_i64_store32;
}
