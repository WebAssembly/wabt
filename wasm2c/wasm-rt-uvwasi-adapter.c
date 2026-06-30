/*
 * Copyright 2018 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wasm-rt-uvwasi-adapter.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef WASM_RT_CORE_TYPES_DEFINED
#define WASM_RT_CORE_TYPES_DEFINED
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
#endif

#define WASM2C_UVWASI_MAX_IOVS 128
#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#if WABT_BIG_ENDIAN
#define MEM_ADDR(mem, addr, n) ((mem)->data_end - (addr) - (n))
#else
#define MEM_ADDR(mem, addr, n) &((mem)->data[addr])
#endif

void wasm2c_uvwasi_link(struct w2c_wasi__snapshot__preview1* wasi,
                        wasm_rt_memory_t* memory,
                        uvwasi_t* uvwasi) {
  wasi->memory = memory;
  wasi->uvwasi = uvwasi;
}

static inline bool add_overflow(uint64_t a, uint64_t b, uint64_t* resptr) {
#if __has_builtin(__builtin_add_overflow)
  return __builtin_add_overflow(a, b, resptr);
#elif defined(_MSC_VER)
  return _addcarry_u64(0, a, b, resptr);
#else
  *resptr = a + b;
  return *resptr < a;
#endif
}

#define RANGE_CHECK(mem, offset, len)              \
  do {                                             \
    uint64_t res;                                  \
    if (UNLIKELY(add_overflow(offset, len, &res))) \
      TRAP(OOB);                                   \
    if (UNLIKELY(res > (mem)->size))               \
      TRAP(OOB);                                   \
  } while (0);

static inline void* mem_get_checked_pointer(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 addr,
    u64 len) {
  RANGE_CHECK(wasi->memory, addr, len);
  return MEM_ADDR(wasi->memory, addr, len);
}

static inline void memory_fill(struct w2c_wasi__snapshot__preview1* wasi,
                               u32 addr,
                               u32 value,
                               u32 len) {
  memset(mem_get_checked_pointer(wasi, addr, len), value, len);
}

static uvwasi_errno_t copy_path_from_wasm(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 path,
    u32 path_len,
    char** path_copy) {
  const char* path_src = mem_get_checked_pointer(wasi, path, path_len);
  char* copy = malloc((size_t)path_len + 1);
  if (!copy) {
    return UVWASI_ENOMEM;
  }

  wasm_rt_memcpy(copy, path_src, path_len);
  copy[path_len] = '\0';
  *path_copy = copy;
  return UVWASI_ESUCCESS;
}

#define DEFINE_LOAD(name, t, ret)                                           \
  static inline ret name(struct w2c_wasi__snapshot__preview1* wasi,         \
                         u32 addr) {                                        \
    t result;                                                               \
    wasm_rt_memcpy(&result, mem_get_checked_pointer(wasi, addr, sizeof(t)), \
                   sizeof(t));                                              \
    return (ret)result;                                                     \
  }

#define DEFINE_STORE(name, t, val_t)                                           \
  static inline void name(struct w2c_wasi__snapshot__preview1* wasi, u32 addr, \
                          val_t value) {                                       \
    t wrapped = (t)value;                                                      \
    wasm_rt_memcpy(mem_get_checked_pointer(wasi, addr, sizeof(t)), &wrapped,   \
                   sizeof(t));                                                 \
  }

DEFINE_LOAD(load_u32, u32, u32)
DEFINE_STORE(store_u8, u8, u32)
DEFINE_STORE(store_u16, u16, u32)
DEFINE_STORE(store_u32, u32, u32)
DEFINE_STORE(store_u64, u64, u64)

static uvwasi_errno_t read_iovs(struct w2c_wasi__snapshot__preview1* wasi,
                                u32 iovs_offset,
                                u32 iovs_len,
                                uvwasi_iovec_t* iovs) {
  RANGE_CHECK(wasi->memory, iovs_offset, (u64)iovs_len * 8);
  for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
    u32 iov = iovs_offset + i * 8;
    u32 buf = load_u32(wasi, iov);
    u32 buf_len = load_u32(wasi, iov + 4);
    iovs[i].buf = mem_get_checked_pointer(wasi, buf, buf_len);
    iovs[i].buf_len = buf_len;
  }
  return UVWASI_ESUCCESS;
}

static uvwasi_errno_t read_ciovs(struct w2c_wasi__snapshot__preview1* wasi,
                                 u32 iovs_offset,
                                 u32 iovs_len,
                                 uvwasi_ciovec_t* iovs) {
  RANGE_CHECK(wasi->memory, iovs_offset, (u64)iovs_len * 8);
  for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
    u32 iov = iovs_offset + i * 8;
    u32 buf = load_u32(wasi, iov);
    u32 buf_len = load_u32(wasi, iov + 4);
    iovs[i].buf = mem_get_checked_pointer(wasi, buf, buf_len);
    iovs[i].buf_len = buf_len;
  }
  return UVWASI_ESUCCESS;
}

u32 w2c_wasi__snapshot__preview1_fd_prestat_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 buf) {
  uvwasi_prestat_t prestat;
  uvwasi_errno_t ret = uvwasi_fd_prestat_get(wasi->uvwasi, fd, &prestat);
  if (ret == UVWASI_ESUCCESS) {
    memory_fill(wasi, buf, 0, 8);
    store_u8(wasi, buf, prestat.pr_type);
    store_u32(wasi, buf + 4, prestat.u.dir.pr_name_len);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 path,
    u32 path_len) {
  return uvwasi_fd_prestat_dir_name(
      wasi->uvwasi, fd, mem_get_checked_pointer(wasi, path, path_len),
      path_len);
}

u32 w2c_wasi__snapshot__preview1_environ_sizes_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 env_count,
    u32 env_buf_size) {
  uvwasi_size_t count, buf_size;
  uvwasi_errno_t ret =
      uvwasi_environ_sizes_get(wasi->uvwasi, &count, &buf_size);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, env_count, count);
    store_u32(wasi, env_buf_size, buf_size);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_environ_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 env,
    u32 buf) {
  uvwasi_size_t count, buf_size;
  uvwasi_errno_t ret =
      uvwasi_environ_sizes_get(wasi->uvwasi, &count, &buf_size);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }
  if (count == 0) {
    return UVWASI_ESUCCESS;
  }

  RANGE_CHECK(wasi->memory, env, (u64)count * 4);
  char* buf_base = mem_get_checked_pointer(wasi, buf, buf_size);
  char** uv_env = calloc(count, sizeof(char*));
  if (!uv_env) {
    return UVWASI_ENOMEM;
  }

  ret = uvwasi_environ_get(wasi->uvwasi, uv_env, buf_base);
  if (ret == UVWASI_ESUCCESS) {
    /* Convert uvwasi's host pointers back to WASM linear-memory offsets. */
    for (uvwasi_size_t i = 0; i < count; ++i) {
      store_u32(wasi, env + i * 4, buf + (u32)(uv_env[i] - buf_base));
    }
  }

  free(uv_env);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_args_sizes_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 argc,
    u32 argv_buf_size) {
  uvwasi_size_t count, buf_size;
  uvwasi_errno_t ret = uvwasi_args_sizes_get(wasi->uvwasi, &count, &buf_size);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, argc, count);
    store_u32(wasi, argv_buf_size, buf_size);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_args_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 argv,
    u32 buf) {
  uvwasi_size_t count, buf_size;
  uvwasi_errno_t ret = uvwasi_args_sizes_get(wasi->uvwasi, &count, &buf_size);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }
  if (count == 0) {
    return UVWASI_ESUCCESS;
  }

  RANGE_CHECK(wasi->memory, argv, (u64)count * 4);
  char* buf_base = mem_get_checked_pointer(wasi, buf, buf_size);
  char** uv_argv = calloc(count, sizeof(char*));
  if (!uv_argv) {
    return UVWASI_ENOMEM;
  }

  ret = uvwasi_args_get(wasi->uvwasi, uv_argv, buf_base);
  if (ret == UVWASI_ESUCCESS) {
    /* Convert uvwasi's host pointers back to WASM linear-memory offsets. */
    for (uvwasi_size_t i = 0; i < count; ++i) {
      store_u32(wasi, argv + i * 4, buf + (u32)(uv_argv[i] - buf_base));
    }
  }

  free(uv_argv);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 stat) {
  uvwasi_fdstat_t uvstat;
  uvwasi_errno_t ret = uvwasi_fd_fdstat_get(wasi->uvwasi, fd, &uvstat);
  if (ret == UVWASI_ESUCCESS) {
    memory_fill(wasi, stat, 0, 24);
    store_u8(wasi, stat, uvstat.fs_filetype);
    store_u16(wasi, stat + 2, uvstat.fs_flags);
    store_u64(wasi, stat + 8, uvstat.fs_rights_base);
    store_u64(wasi, stat + 16, uvstat.fs_rights_inheriting);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_fdstat_set_flags(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 flags) {
  return uvwasi_fd_fdstat_set_flags(wasi->uvwasi, fd, flags);
}

u32 w2c_wasi__snapshot__preview1_fd_fdstat_set_rights(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 fs_rights_base,
    u64 fs_rights_inheriting) {
  return uvwasi_fd_fdstat_set_rights(wasi->uvwasi, fd, fs_rights_base,
                                     fs_rights_inheriting);
}

u32 w2c_wasi__snapshot__preview1_path_filestat_set_times(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 flags,
    u32 path,
    u32 path_len,
    u64 atim,
    u64 mtim,
    u32 fst_flags) {
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_filestat_set_times(wasi->uvwasi, fd, flags, path_copy,
                                       path_len, atim, mtim, fst_flags);
  free(path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_filestat_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 flags,
    u32 path,
    u32 path_len,
    u32 stat) {
  uvwasi_filestat_t uvstat;
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_filestat_get(wasi->uvwasi, fd, flags, path_copy, path_len,
                                 &uvstat);
  free(path_copy);
  if (ret == UVWASI_ESUCCESS) {
    memory_fill(wasi, stat, 0, 64);
    store_u64(wasi, stat, uvstat.st_dev);
    store_u64(wasi, stat + 8, uvstat.st_ino);
    store_u8(wasi, stat + 16, uvstat.st_filetype);
    store_u64(wasi, stat + 24, uvstat.st_nlink);
    store_u64(wasi, stat + 32, uvstat.st_size);
    store_u64(wasi, stat + 40, uvstat.st_atim);
    store_u64(wasi, stat + 48, uvstat.st_mtim);
    store_u64(wasi, stat + 56, uvstat.st_ctim);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_filestat_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 stat) {
  uvwasi_filestat_t uvstat;
  uvwasi_errno_t ret = uvwasi_fd_filestat_get(wasi->uvwasi, fd, &uvstat);
  if (ret == UVWASI_ESUCCESS) {
    memory_fill(wasi, stat, 0, 64);
    store_u64(wasi, stat, uvstat.st_dev);
    store_u64(wasi, stat + 8, uvstat.st_ino);
    store_u8(wasi, stat + 16, uvstat.st_filetype);
    store_u64(wasi, stat + 24, uvstat.st_nlink);
    store_u64(wasi, stat + 32, uvstat.st_size);
    store_u64(wasi, stat + 40, uvstat.st_atim);
    store_u64(wasi, stat + 48, uvstat.st_mtim);
    store_u64(wasi, stat + 56, uvstat.st_ctim);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_seek(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 offset,
    u32 whence,
    u32 pos) {
  uvwasi_filesize_t uvpos;
  if (offset > (u64)INT64_MAX) {
    return UVWASI_EINVAL;
  }

  uvwasi_filedelta_t uv_offset = (uvwasi_filedelta_t)offset;
  uvwasi_errno_t ret =
      uvwasi_fd_seek(wasi->uvwasi, fd, uv_offset, whence, &uvpos);
  if (ret == UVWASI_ESUCCESS) {
    store_u64(wasi, pos, uvpos);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_tell(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 pos) {
  uvwasi_filesize_t uvpos;
  uvwasi_errno_t ret = uvwasi_fd_tell(wasi->uvwasi, fd, &uvpos);
  if (ret == UVWASI_ESUCCESS) {
    store_u64(wasi, pos, uvpos);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_filestat_set_size(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 filesize) {
  return uvwasi_fd_filestat_set_size(wasi->uvwasi, fd, filesize);
}

u32 w2c_wasi__snapshot__preview1_fd_filestat_set_times(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 atim,
    u64 mtim,
    u32 fst_flags) {
  return uvwasi_fd_filestat_set_times(wasi->uvwasi, fd, atim, mtim, fst_flags);
}

u32 w2c_wasi__snapshot__preview1_fd_sync(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd) {
  return uvwasi_fd_sync(wasi->uvwasi, fd);
}

u32 w2c_wasi__snapshot__preview1_fd_datasync(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd) {
  return uvwasi_fd_datasync(wasi->uvwasi, fd);
}

u32 w2c_wasi__snapshot__preview1_fd_renumber(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd_from,
    u32 fd_to) {
  return uvwasi_fd_renumber(wasi->uvwasi, fd_from, fd_to);
}

u32 w2c_wasi__snapshot__preview1_fd_allocate(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 offset,
    u64 len) {
  return uvwasi_fd_allocate(wasi->uvwasi, fd, offset, len);
}

u32 w2c_wasi__snapshot__preview1_fd_advise(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u64 offset,
    u64 len,
    u32 advice) {
  return uvwasi_fd_advise(wasi->uvwasi, fd, offset, len, advice);
}

u32 w2c_wasi__snapshot__preview1_path_open(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 dirfd,
    u32 dirflags,
    u32 path,
    u32 path_len,
    u32 oflags,
    u64 fs_rights_base,
    u64 fs_rights_inheriting,
    u32 fs_flags,
    u32 fd) {
  uvwasi_fd_t uvfd;
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_open(wasi->uvwasi, dirfd, dirflags, path_copy, path_len,
                         oflags, fs_rights_base, fs_rights_inheriting, fs_flags,
                         &uvfd);
  free(path_copy);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, fd, uvfd);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_close(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd) {
  return uvwasi_fd_close(wasi->uvwasi, fd);
}

u32 w2c_wasi__snapshot__preview1_path_symlink(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 old_path,
    u32 old_path_len,
    u32 fd,
    u32 new_path,
    u32 new_path_len) {
  char *old_path_copy, *new_path_copy;
  uvwasi_errno_t ret =
      copy_path_from_wasm(wasi, old_path, old_path_len, &old_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = copy_path_from_wasm(wasi, new_path, new_path_len, &new_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    free(old_path_copy);
    return ret;
  }

  ret = uvwasi_path_symlink(wasi->uvwasi, old_path_copy, old_path_len, fd,
                            new_path_copy, new_path_len);
  free(new_path_copy);
  free(old_path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_rename(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 old_fd,
    u32 old_path,
    u32 old_path_len,
    u32 new_fd,
    u32 new_path,
    u32 new_path_len) {
  char *old_path_copy, *new_path_copy;
  uvwasi_errno_t ret =
      copy_path_from_wasm(wasi, old_path, old_path_len, &old_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = copy_path_from_wasm(wasi, new_path, new_path_len, &new_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    free(old_path_copy);
    return ret;
  }

  ret = uvwasi_path_rename(wasi->uvwasi, old_fd, old_path_copy, old_path_len,
                           new_fd, new_path_copy, new_path_len);
  free(new_path_copy);
  free(old_path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_link(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 old_fd,
    u32 old_flags,
    u32 old_path,
    u32 old_path_len,
    u32 new_fd,
    u32 new_path,
    u32 new_path_len) {
  char *old_path_copy, *new_path_copy;
  uvwasi_errno_t ret =
      copy_path_from_wasm(wasi, old_path, old_path_len, &old_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = copy_path_from_wasm(wasi, new_path, new_path_len, &new_path_copy);
  if (ret != UVWASI_ESUCCESS) {
    free(old_path_copy);
    return ret;
  }

  ret = uvwasi_path_link(wasi->uvwasi, old_fd, old_flags, old_path_copy,
                         old_path_len, new_fd, new_path_copy, new_path_len);
  free(new_path_copy);
  free(old_path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_unlink_file(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 path,
    u32 path_len) {
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_unlink_file(wasi->uvwasi, fd, path_copy, path_len);
  free(path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_readlink(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 path,
    u32 path_len,
    u32 buf,
    u32 buf_len,
    u32 bufused) {
  uvwasi_size_t uvbufused;
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_readlink(wasi->uvwasi, fd, path_copy, path_len,
                             mem_get_checked_pointer(wasi, buf, buf_len),
                             buf_len, &uvbufused);
  free(path_copy);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, bufused, uvbufused);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_create_directory(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 path,
    u32 path_len) {
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_create_directory(wasi->uvwasi, fd, path_copy, path_len);
  free(path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_path_remove_directory(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 path,
    u32 path_len) {
  char* path_copy;
  uvwasi_errno_t ret = copy_path_from_wasm(wasi, path, path_len, &path_copy);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  ret = uvwasi_path_remove_directory(wasi->uvwasi, fd, path_copy, path_len);
  free(path_copy);
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_readdir(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 buf,
    u32 buf_len,
    u64 cookie,
    u32 bufused) {
  uvwasi_size_t uvbufused;
  uvwasi_errno_t ret = uvwasi_fd_readdir(
      wasi->uvwasi, fd, mem_get_checked_pointer(wasi, buf, buf_len), buf_len,
      cookie, &uvbufused);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, bufused, uvbufused);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_write(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 iovs_offset,
    u32 iovs_len,
    u32 nwritten) {
  if (iovs_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_ciovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_ciovs(wasi, iovs_offset, iovs_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t num_written;
  ret = uvwasi_fd_write(wasi->uvwasi, fd, iovs, iovs_len, &num_written);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, nwritten, num_written);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_pwrite(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 iovs_offset,
    u32 iovs_len,
    u64 offset,
    u32 nwritten) {
  if (iovs_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_ciovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_ciovs(wasi, iovs_offset, iovs_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t num_written;
  ret =
      uvwasi_fd_pwrite(wasi->uvwasi, fd, iovs, iovs_len, offset, &num_written);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, nwritten, num_written);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_read(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 iovs_offset,
    u32 iovs_len,
    u32 nread) {
  if (iovs_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_iovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_iovs(wasi, iovs_offset, iovs_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t num_read;
  ret = uvwasi_fd_read(wasi->uvwasi, fd, iovs, iovs_len, &num_read);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, nread, num_read);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_fd_pread(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 fd,
    u32 iovs_offset,
    u32 iovs_len,
    u64 offset,
    u32 nread) {
  if (iovs_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_iovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_iovs(wasi, iovs_offset, iovs_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t num_read;
  ret = uvwasi_fd_pread(wasi->uvwasi, fd, iovs, iovs_len, offset, &num_read);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, nread, num_read);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_poll_oneoff(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 in,
    u32 out,
    u32 nsubscriptions,
    u32 nevents) {
  uvwasi_size_t uvnevents;
  void* in_ptr = mem_get_checked_pointer(
      wasi, in, (u64)nsubscriptions * sizeof(uvwasi_subscription_t));
  void* out_ptr = mem_get_checked_pointer(
      wasi, out, (u64)nsubscriptions * sizeof(uvwasi_event_t));
  uvwasi_errno_t ret = uvwasi_poll_oneoff(wasi->uvwasi, in_ptr, out_ptr,
                                          nsubscriptions, &uvnevents);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, nevents, uvnevents);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_clock_res_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 clk_id,
    u32 result) {
  uvwasi_timestamp_t t;
  uvwasi_errno_t ret = uvwasi_clock_res_get(wasi->uvwasi, clk_id, &t);
  if (ret == UVWASI_ESUCCESS) {
    store_u64(wasi, result, t);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_clock_time_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 clk_id,
    u64 precision,
    u32 result) {
  uvwasi_timestamp_t t;
  uvwasi_errno_t ret =
      uvwasi_clock_time_get(wasi->uvwasi, clk_id, precision, &t);
  if (ret == UVWASI_ESUCCESS) {
    store_u64(wasi, result, t);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_random_get(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 buf,
    u32 buf_len) {
  return uvwasi_random_get(
      wasi->uvwasi, mem_get_checked_pointer(wasi, buf, buf_len), buf_len);
}

u32 w2c_wasi__snapshot__preview1_sched_yield(
    struct w2c_wasi__snapshot__preview1* wasi) {
  return uvwasi_sched_yield(wasi->uvwasi);
}

u32 w2c_wasi__snapshot__preview1_proc_raise(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 sig) {
  return uvwasi_proc_raise(wasi->uvwasi, sig);
}

void w2c_wasi__snapshot__preview1_proc_exit(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 code) {
  uvwasi_proc_exit(wasi->uvwasi, code);
  wasm_rt_unreachable();
}

u32 w2c_wasi__snapshot__preview1_sock_accept(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 sock,
    u32 flags,
    u32 fd) {
  // uvwasi_sock_accept was only added in v0.0.19
#if (UVWASI_VERSION_MAJOR > 0) || (UVWASI_VERSION_MINOR > 0) || \
    (UVWASI_VERSION_PATCH > 18)
  uvwasi_fd_t accepted_fd;
  uvwasi_errno_t ret =
      uvwasi_sock_accept(wasi->uvwasi, sock, flags, &accepted_fd);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, fd, accepted_fd);
  }
  return ret;
#else
  // The Wabt toolkit is currently pinned to an old version of uvwasi, so this
  // fallback is necessary.
  return UVWASI_ENOSYS;
#endif
}

u32 w2c_wasi__snapshot__preview1_sock_recv(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 sock,
    u32 ri_data,
    u32 ri_data_len,
    u32 ri_flags,
    u32 ro_datalen,
    u32 ro_flags) {
  if (ri_data_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_iovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_iovs(wasi, ri_data, ri_data_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t datalen;
  uvwasi_roflags_t flags;
  ret = uvwasi_sock_recv(wasi->uvwasi, sock, iovs, ri_data_len, ri_flags,
                         &datalen, &flags);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, ro_datalen, datalen);
    store_u16(wasi, ro_flags, flags);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_sock_send(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 sock,
    u32 si_data,
    u32 si_data_len,
    u32 si_flags,
    u32 so_datalen) {
  if (si_data_len > WASM2C_UVWASI_MAX_IOVS) {
    return UVWASI_EINVAL;
  }

  uvwasi_ciovec_t iovs[WASM2C_UVWASI_MAX_IOVS];
  uvwasi_errno_t ret = read_ciovs(wasi, si_data, si_data_len, iovs);
  if (ret != UVWASI_ESUCCESS) {
    return ret;
  }

  uvwasi_size_t datalen;
  ret = uvwasi_sock_send(wasi->uvwasi, sock, iovs, si_data_len, si_flags,
                         &datalen);
  if (ret == UVWASI_ESUCCESS) {
    store_u32(wasi, so_datalen, datalen);
  }
  return ret;
}

u32 w2c_wasi__snapshot__preview1_sock_shutdown(
    struct w2c_wasi__snapshot__preview1* wasi,
    u32 sock,
    u32 how) {
  return uvwasi_sock_shutdown(wasi->uvwasi, sock, how);
}
