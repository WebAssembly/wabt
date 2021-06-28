/*
 * Copyright 2020 WebAssembly Community Group participants
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

/*
 * This is an experiment and currently extremely limited implementation
 * of the WASI syscall API.  The implementation of the API itself is coming from
 * uvwasi: https://github.com/cjihrig/uvwasi.
 *
 * Most of the code in the file is mostly marshelling data between the wabt
 * interpreter and uvwasi.
 *
 * For details of the WASI api see:
 * https://github.com/WebAssembly/WASI/blob/master/phases/snapshot/docs.md
 * and the C headers version:
 * https://github.com/WebAssembly/wasi-libc/blob/master/libc-bottom-half/headers/public/wasi/api.h
 */

#include "src/interp/interp-wasi.h"
#include "src/interp/interp-util.h"

#ifdef WITH_WASI

#include "uvwasi.h"

#include <cinttypes>
#include <unordered_map>

using namespace wabt;
using namespace wabt::interp;

namespace {

// Types that align with WASI spec on size and alignment.  These are
// copied directly from wasi-lib's auto-generated api.h
// TODO(sbc): Auto-generate this from witx.

// BEGIN: wasi.h types from wasi-libc

typedef uint32_t __wasi_size_t;
typedef uint32_t __wasi_ptr_t;

static_assert(sizeof(__wasi_size_t) == 4, "witx calculated size");
static_assert(alignof(__wasi_size_t) == 4, "witx calculated align");
static_assert(sizeof(__wasi_ptr_t) == 4, "witx calculated size");
static_assert(alignof(__wasi_ptr_t) == 4, "witx calculated align");

typedef struct __wasi_prestat_dir_t {
  __wasi_size_t pr_name_len;
} __wasi_prestat_dir_t;

typedef uint8_t __wasi_preopentype_t;
typedef uint64_t __wasi_rights_t;
typedef uint16_t __wasi_fdflags_t;
typedef uint8_t __wasi_filetype_t;
typedef uint16_t __wasi_oflags_t;
typedef uint32_t __wasi_lookupflags_t;
typedef uint32_t __wasi_fd_t;
typedef uint64_t __wasi_timestamp_t;
typedef uint8_t __wasi_whence_t;
typedef int64_t __wasi_filedelta_t;
typedef uint64_t __wasi_filesize_t;

typedef union __wasi_prestat_u_t {
  __wasi_prestat_dir_t dir;
} __wasi_prestat_u_t;

struct __wasi_prestat_t {
  __wasi_preopentype_t tag;
  __wasi_prestat_u_t u;
};

typedef struct __wasi_fdstat_t {
  __wasi_filetype_t fs_filetype;
  __wasi_fdflags_t fs_flags;
  __wasi_rights_t fs_rights_base;
  __wasi_rights_t fs_rights_inheriting;
} __wasi_fdstat_t;

static_assert(sizeof(__wasi_fdstat_t) == 24, "witx calculated size");
static_assert(alignof(__wasi_fdstat_t) == 8, "witx calculated align");
static_assert(offsetof(__wasi_fdstat_t, fs_filetype) == 0,
              "witx calculated offset");
static_assert(offsetof(__wasi_fdstat_t, fs_flags) == 2,
              "witx calculated offset");
static_assert(offsetof(__wasi_fdstat_t, fs_rights_base) == 8,
              "witx calculated offset");
static_assert(offsetof(__wasi_fdstat_t, fs_rights_inheriting) == 16,
              "witx calculated offset");

struct __wasi_iovec_t {
  __wasi_ptr_t buf;
  __wasi_size_t buf_len;
};

static_assert(sizeof(__wasi_iovec_t) == 8, "witx calculated size");
static_assert(alignof(__wasi_iovec_t) == 4, "witx calculated align");
static_assert(offsetof(__wasi_iovec_t, buf) == 0, "witx calculated offset");
static_assert(offsetof(__wasi_iovec_t, buf_len) == 4, "witx calculated offset");

typedef uint64_t __wasi_device_t;

static_assert(sizeof(__wasi_device_t) == 8, "witx calculated size");
static_assert(alignof(__wasi_device_t) == 8, "witx calculated align");

typedef uint64_t __wasi_inode_t;

static_assert(sizeof(__wasi_inode_t) == 8, "witx calculated size");
static_assert(alignof(__wasi_inode_t) == 8, "witx calculated align");

typedef uint64_t __wasi_linkcount_t;

static_assert(sizeof(__wasi_linkcount_t) == 8, "witx calculated size");
static_assert(alignof(__wasi_linkcount_t) == 8, "witx calculated align");

typedef struct __wasi_filestat_t {
  __wasi_device_t dev;
  __wasi_inode_t ino;
  __wasi_filetype_t filetype;
  __wasi_linkcount_t nlink;
  __wasi_filesize_t size;
  __wasi_timestamp_t atim;
  __wasi_timestamp_t mtim;
  __wasi_timestamp_t ctim;
} __wasi_filestat_t;

static_assert(sizeof(__wasi_filestat_t) == 64, "witx calculated size");
static_assert(alignof(__wasi_filestat_t) == 8, "witx calculated align");
static_assert(offsetof(__wasi_filestat_t, dev) == 0, "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, ino) == 8, "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, filetype) == 16,
              "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, nlink) == 24,
              "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, size) == 32,
              "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, atim) == 40,
              "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, mtim) == 48,
              "witx calculated offset");
static_assert(offsetof(__wasi_filestat_t, ctim) == 56,
              "witx calculated offset");

#define __WASI_ERRNO_SUCCESS (UINT16_C(0))
#define __WASI_ERRNO_NOENT (UINT16_C(44))

// END wasi.h types from wasi-lib

class WasiInstance {
 public:
  WasiInstance(Instance::Ptr instance,
               uvwasi_s* uvwasi,
               Memory* memory,
               Stream* trace_stream)
      : trace_stream(trace_stream),
        instance(instance),
        uvwasi(uvwasi),
        memory(memory) {}

  Result random_get(const Values& params, Values& results, Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_random_get(uint8_t * buf, __wasi_size_t buf_len) */
    assert(false);
    return Result::Ok;
  }

  Result proc_exit(const Values& params, Values& results, Trap::Ptr* trap) {
    const Value arg0 = params[0];
    uvwasi_proc_exit(uvwasi, arg0.Get<u32>());
    return Result::Ok;
  }

  Result poll_oneoff(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result clock_time_get(const Values& params,
                        Values& results,
                        Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_clock_time_get(__wasi_clockid_t id,
     *                                      __wasi_timestamp_t precision,
     *                                      __wasi_timestamp_t *time)
     */
    __wasi_timestamp_t t;
    results[0].Set<u32>(uvwasi_clock_time_get(uvwasi, params[0].Get<u32>(),
                                              params[1].Get<u64>(), &t));
    uint32_t time_ptr = params[2].Get<u32>();
    CHECK_RESULT(writeValue<__wasi_timestamp_t>(t, time_ptr, trap));
    return Result::Ok;
  }

  Result path_rename(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result path_open(const Values& params, Values& results, Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_path_open(__wasi_fd_t fd,
                                       __wasi_lookupflags_t dirflags,
                                       const char *path,
                                       size_t path_len,
                                       __wasi_oflags_t oflags,
                                       __wasi_rights_t fs_rights_base,
                                       __wasi_rights_t fs_rights_inherting,
                                       __wasi_fdflags_t fdflags,
                                       __wasi_fd_t *opened_fd) */
    uvwasi_fd_t dirfd = params[0].Get<u32>();
    __wasi_lookupflags_t dirflags = params[1].Get<u32>();
    uint32_t path_ptr = params[2].Get<u32>();
    __wasi_size_t path_len = params[3].Get<u32>();
    __wasi_oflags_t oflags = params[4].Get<u32>();
    __wasi_rights_t fs_rights_base = params[5].Get<u32>();
    __wasi_rights_t fs_rights_inherting = params[6].Get<u32>();
    __wasi_fdflags_t fs_flags = params[7].Get<u32>();
    uint32_t out_ptr = params[8].Get<u32>();
    char* path;
    CHECK_RESULT(getMemPtr<char>(path_ptr, path_len, &path, trap));
    if (trace_stream) { trace_stream->Writef("path_open : %s\n", path); }
    uvwasi_fd_t outfd;
    results[0].Set<u32>(uvwasi_path_open(
        uvwasi, dirfd, dirflags, path, path_len, oflags, fs_rights_base,
        fs_rights_inherting, fs_flags, &outfd));
    if (trace_stream) {
      trace_stream->Writef("path_open -> %d\n", results[0].Get<u32>());
    }
    CHECK_RESULT(writeValue<__wasi_fd_t>(outfd, out_ptr, trap));
    return Result::Ok;
  }

  Result path_filestat_get(const Values& params,
                           Values& results,
                           Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_path_filestat_get(__wasi_fd_t fd,
     *                                         __wasi_lookupflags_t flags,
     *                                         const char *path,
     *                                         size_t path_len,
     *                                         __wasi_filestat_t *buf
     */
    uvwasi_fd_t fd = params[0].Get<u32>();
    __wasi_lookupflags_t flags = params[1].Get<u32>();
    uint32_t path_ptr = params[2].Get<u32>();
    uvwasi_size_t path_len = params[3].Get<u32>();
    uint32_t filestat_ptr = params[4].Get<u32>();
    char* path;
    CHECK_RESULT(getMemPtr<char>(path_ptr, path_len, &path, trap));
    if (trace_stream) {
      trace_stream->Writef("path_filestat_get : %d %s\n", fd, path);
    }
    uvwasi_filestat_t buf;
    results[0].Set<u32>(
        uvwasi_path_filestat_get(uvwasi, fd, flags, path, path_len, &buf));
    __wasi_filestat_t* filestat;
    CHECK_RESULT(getMemPtr<__wasi_filestat_t>(
        filestat_ptr, sizeof(__wasi_filestat_t), &filestat, trap));
    uvwasi_serdes_write_filestat_t(filestat, 0, &buf);
    if (trace_stream) {
      trace_stream->Writef("path_filestat_get -> size=%" PRIu64 " %d\n",
                           buf.st_size, results[0].Get<u32>());
    }
    return Result::Ok;
  }

  Result path_symlink(const Values& params, Values& results, Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_path_symlink(const char *old_path,
     *                                    size_t old_path_len,
     *                                    __wasi_fd_t fd,
     *                                    const char *new_path,
     *                                    size_t new_path_len);
     */

    uint32_t old_path_ptr = params[0].Get<u32>();
    __wasi_size_t old_path_len = params[1].Get<u32>();
    uvwasi_fd_t fd = params[2].Get<u32>();
    uint32_t new_path_ptr = params[3].Get<u32>();
    __wasi_size_t new_path_len = params[4].Get<u32>();
    char* old_path;
    char* new_path;
    CHECK_RESULT(getMemPtr<char>(old_path_ptr, old_path_len, &old_path, trap));
    CHECK_RESULT(getMemPtr<char>(new_path_ptr, new_path_len, &new_path, trap));
    if (trace_stream) {
      trace_stream->Writef("path_symlink %d %s : %s\n", fd, old_path, new_path);
    }
    results[0].Set<u32>(uvwasi_path_symlink(uvwasi, old_path, old_path_len, fd,
                                            new_path, new_path_len));
    if (trace_stream) {
      trace_stream->Writef("path_symlink -> %d\n", results[0].Get<u32>());
    }
    return Result::Ok;
  }

  Result path_readlink(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result path_create_directory(const Values& params,
                               Values& results,
                               Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result path_remove_directory(const Values& params,
                               Values& results,
                               Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result path_unlink_file(const Values& params,
                          Values& results,
                          Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_path_unlink_file(__wasi_fd_t fd,
     *                                        const char *path,
     *                                        size_t path_len)
     */
    uvwasi_fd_t fd = params[0].Get<u32>();
    uint32_t path_ptr = params[1].Get<u32>();
    __wasi_size_t path_len = params[2].Get<u32>();
    char* path;
    CHECK_RESULT(getMemPtr<char>(path_ptr, path_len, &path, trap));
    if (trace_stream) {
      trace_stream->Writef("path_unlink_file %d %s\n", fd, path);
    }
    results[0].Set<u32>(uvwasi_path_unlink_file(uvwasi, fd, path, path_len));
    return Result::Ok;
  }

  Result fd_prestat_get(const Values& params,
                        Values& results,
                        Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd,
     *                                      __wasi_prestat_t *buf))
     */
    uvwasi_fd_t fd = params[0].Get<u32>();
    uint32_t prestat_ptr = params[1].Get<u32>();
    if (trace_stream) { trace_stream->Writef("fd_prestat_get %d\n", fd); }
    uvwasi_prestat_t buf;
    results[0].Set<u32>(uvwasi_fd_prestat_get(uvwasi, fd, &buf));
    __wasi_prestat_t* prestat;
    CHECK_RESULT(getMemPtr<__wasi_prestat_t>(prestat_ptr, 1, &prestat, trap));
    uvwasi_serdes_write_prestat_t(prestat, 0, &buf);
    if (trace_stream) {
      trace_stream->Writef("fd_prestat_get -> %d\n", results[0].Get<u32>());
    }
    return Result::Ok;
  }

  Result fd_prestat_dir_name(const Values& params,
                             Values& results,
                             Trap::Ptr* trap) {
    uvwasi_fd_t fd = params[0].Get<u32>();
    uint32_t path_ptr = params[1].Get<u32>();
    uvwasi_size_t path_len = params[2].Get<u32>();
    if (trace_stream) {
      trace_stream->Writef("fd_prestat_dir_name %d %d %d\n", fd, path_ptr,
                           path_len);
    }
    char* path;
    CHECK_RESULT(getMemPtr<char>(path_ptr, path_len, &path, trap));
    results[0].Set<u32>(uvwasi_fd_prestat_dir_name(uvwasi, fd, path, path_len));
    if (trace_stream) {
      trace_stream->Writef("fd_prestat_dir_name %d -> %d %s %d\n", fd,
                           results[0].Get<u32>(), path, path_len);
    }
    return Result::Ok;
  }

  Result fd_filestat_get(const Values& params,
                         Values& results,
                         Trap::Ptr* trap) {
    /* __wasi_fd_filestat_get(__wasi_fd_t f, __wasi_filestat_t *buf) */
    uvwasi_fd_t fd = params[0].Get<u32>();
    uint32_t filestat_ptr = params[1].Get<u32>();
    uvwasi_filestat_t buf;
    results[0].Set<u32>(uvwasi_fd_filestat_get(uvwasi, fd, &buf));
    __wasi_filestat_t* filestat;
    CHECK_RESULT(getMemPtr<__wasi_filestat_t>(
        filestat_ptr, sizeof(__wasi_filestat_t), &filestat, trap));
    uvwasi_serdes_write_filestat_t(filestat, 0, &buf);
    if (trace_stream) {
      trace_stream->Writef("fd_filestat_get -> size=%" PRIu64 " %d\n",
                           buf.st_size, results[0].Get<u32>());
    }
    return Result::Ok;
  }

  Result fd_fdstat_set_flags(const Values& params,
                             Values& results,
                             Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result fd_fdstat_get(const Values& params, Values& results, Trap::Ptr* trap) {
    int32_t fd = params[0].Get<u32>();
    uint32_t stat_ptr = params[1].Get<u32>();
    if (trace_stream) { trace_stream->Writef("fd_fdstat_get %d\n", fd); }
    CHECK_RESULT(getMemPtr<__wasi_fdstat_t>(stat_ptr, 1, nullptr, trap));
    uvwasi_fdstat_t host_statbuf;
    results[0].Set<u32>(uvwasi_fd_fdstat_get(uvwasi, fd, &host_statbuf));

    // Write the host statbuf into the target wasm memory
    __wasi_fdstat_t* statbuf;
    CHECK_RESULT(getMemPtr<__wasi_fdstat_t>(stat_ptr, 1, &statbuf, trap));
    uvwasi_serdes_write_fdstat_t(statbuf, 0, &host_statbuf);
    return Result::Ok;
  }

  Result fd_read(const Values& params, Values& results, Trap::Ptr* trap) {
    int32_t fd = params[0].Get<u32>();
    int32_t iovptr = params[1].Get<u32>();
    int32_t iovcnt = params[2].Get<u32>();
    int32_t out_ptr = params[2].Get<u32>();
    if (trace_stream) { trace_stream->Writef("fd_read %d [%d]\n", fd, iovcnt); }
    __wasi_iovec_t* wasm_iovs;
    CHECK_RESULT(getMemPtr<__wasi_iovec_t>(iovptr, iovcnt, &wasm_iovs, trap));
    std::vector<uvwasi_iovec_t> iovs(iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      iovs[i].buf_len = wasm_iovs[i].buf_len;

      CHECK_RESULT(getMemPtr<uint8_t>(wasm_iovs[i].buf, wasm_iovs[i].buf_len,
                                      reinterpret_cast<uint8_t**>(&iovs[i].buf),
                                      trap));
    }
    __wasi_ptr_t* out_addr;
    CHECK_RESULT(getMemPtr<__wasi_ptr_t>(out_ptr, 1, &out_addr, trap));
    results[0].Set<u32>(
        uvwasi_fd_read(uvwasi, fd, iovs.data(), iovs.size(), out_addr));
    if (trace_stream) {
      trace_stream->Writef("fd_read -> %d\n", results[0].Get<u32>());
    }
    return Result::Ok;
  }

  Result fd_pread(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result fd_readdir(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result fd_write(const Values& params, Values& results, Trap::Ptr* trap) {
    int32_t fd = params[0].Get<u32>();
    int32_t iovptr = params[1].Get<u32>();
    int32_t iovcnt = params[2].Get<u32>();
    __wasi_iovec_t* wasm_iovs;
    CHECK_RESULT(getMemPtr<__wasi_iovec_t>(iovptr, iovcnt, &wasm_iovs, trap));
    std::vector<uvwasi_ciovec_t> iovs(iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      iovs[i].buf_len = wasm_iovs[i].buf_len;
      CHECK_RESULT(getMemPtr<const uint8_t>(
          wasm_iovs[i].buf, wasm_iovs[i].buf_len,
          reinterpret_cast<const uint8_t**>(&iovs[i].buf), trap));
    }
    __wasi_ptr_t* out_addr;
    CHECK_RESULT(
        getMemPtr<__wasi_ptr_t>(params[3].Get<u32>(), 1, &out_addr, trap));
    results[0].Set<u32>(
        uvwasi_fd_write(uvwasi, fd, iovs.data(), iovs.size(), out_addr));
    return Result::Ok;
  }

  Result fd_pwrite(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result fd_close(const Values& params, Values& results, Trap::Ptr* trap) {
    assert(false);
    return Result::Ok;
  }

  Result fd_seek(const Values& params, Values& results, Trap::Ptr* trap) {
    /* __wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
     *                               __wasi_filedelta_t offset,
     *                               __wasi_whence_t whence,
     *                               __wasi_filesize_t *newoffset)
     */
    int32_t fd = params[0].Get<u32>();
    __wasi_filedelta_t offset = params[1].Get<u32>();
    __wasi_whence_t whence = params[2].Get<u32>();
    uint32_t newoffset_ptr = params[3].Get<u32>();
    uvwasi_filesize_t newoffset;
    results[0].Set<u32>(uvwasi_fd_seek(uvwasi, fd, offset, whence, &newoffset));
    CHECK_RESULT(writeValue<__wasi_filesize_t>(newoffset, newoffset_ptr, trap));
    return Result::Ok;
  }

  Result environ_get(const Values& params, Values& results, Trap::Ptr* trap) {
    uvwasi_size_t environc;
    uvwasi_size_t environ_buf_size;
    uvwasi_environ_sizes_get(uvwasi, &environc, &environ_buf_size);
    uint32_t wasm_buf = params[1].Get<u32>();
    char* buf;
    CHECK_RESULT(getMemPtr<char>(wasm_buf, environ_buf_size, &buf, trap));
    std::vector<char*> host_env(environc);
    uvwasi_environ_get(uvwasi, host_env.data(), buf);

    // Copy host_env pointer array wasm_env)
    for (uvwasi_size_t i = 0; i < environc; i++) {
      uint32_t rel_address = host_env[i] - buf;
      uint32_t dest = params[0].Get<u32>() + (i * sizeof(uint32_t));
      CHECK_RESULT(writeValue<uint32_t>(wasm_buf + rel_address, dest, trap));
    }

    results[0].Set<u32>(__WASI_ERRNO_SUCCESS);
    return Result::Ok;
  }

  Result environ_sizes_get(const Values& params,
                           Values& results,
                           Trap::Ptr* trap) {
    uvwasi_size_t environc;
    uvwasi_size_t environ_buf_size;
    uvwasi_environ_sizes_get(uvwasi, &environc, &environ_buf_size);
    CHECK_RESULT(writeValue<uint32_t>(environc, params[0].Get<u32>(), trap));
    CHECK_RESULT(
        writeValue<uint32_t>(environ_buf_size, params[1].Get<u32>(), trap));
    if (trace_stream) {
      trace_stream->Writef("environ_sizes_get -> %d %d\n", environc,
                           environ_buf_size);
    }
    results[0].Set<u32>(__WASI_ERRNO_SUCCESS);
    return Result::Ok;
  }

  Result args_get(const Values& params, Values& results, Trap::Ptr* trap) {
    uvwasi_size_t argc;
    uvwasi_size_t arg_buf_size;
    uvwasi_args_sizes_get(uvwasi, &argc, &arg_buf_size);
    uint32_t wasm_buf = params[1].Get<u32>();
    char* buf;
    CHECK_RESULT(getMemPtr<char>(wasm_buf, arg_buf_size, &buf, trap));
    std::vector<char*> host_args(argc);
    uvwasi_args_get(uvwasi, host_args.data(), buf);

    // Copy host_args pointer array wasm_args)
    for (uvwasi_size_t i = 0; i < argc; i++) {
      uint32_t rel_address = host_args[i] - buf;
      uint32_t dest = params[0].Get<u32>() + (i * sizeof(uint32_t));
      CHECK_RESULT(writeValue<uint32_t>(wasm_buf + rel_address, dest, trap));
    }
    results[0].Set<u32>(__WASI_ERRNO_SUCCESS);
    return Result::Ok;
  }

  Result args_sizes_get(const Values& params,
                        Values& results,
                        Trap::Ptr* trap) {
    uvwasi_size_t argc;
    uvwasi_size_t arg_buf_size;
    uvwasi_args_sizes_get(uvwasi, &argc, &arg_buf_size);
    CHECK_RESULT(writeValue<uint32_t>(argc, params[0].Get<u32>(), trap));
    CHECK_RESULT(
        writeValue<uint32_t>(arg_buf_size, params[1].Get<u32>(), trap));
    if (trace_stream) {
      trace_stream->Writef("args_sizes_get -> %d %d\n", argc, arg_buf_size);
    }
    results[0].Set<u32>(__WASI_ERRNO_SUCCESS);
    return Result::Ok;
  }

  // The trace stream accosiated with the instance.
  Stream* trace_stream;

  Instance::Ptr instance;

 private:
  // Write a value into wasm-memory and the given memory offset.
  template <typename T>
  Result writeValue(T value, uint32_t target_address, Trap::Ptr* trap) {
    T* abs_address;
    CHECK_RESULT(getMemPtr<T>(target_address, sizeof(T), &abs_address, trap));
    *abs_address = value;
    return Result::Ok;
  }

  // Result a wasm-memory-local address to an absolute memory location.
  template <typename T>
  Result getMemPtr(uint32_t address,
                   uint32_t num_elems,
                   T** result,
                   Trap::Ptr* trap) {
    if (!memory->IsValidAccess(address, 0, num_elems * sizeof(T))) {
      *trap =
          Trap::New(*instance.store(),
                    StringPrintf("out of bounds memory access: "
                                 "[%u, %" PRIu64 ") >= max value %" PRIu64,
                                 address, u64{address} + num_elems * sizeof(T),
                                 memory->ByteSize()));
      return Result::Error;
    }
    if (result) {
      *result = reinterpret_cast<T*>(memory->UnsafeData() + address);
    }
    return Result::Ok;
  }

  uvwasi_s* uvwasi;
  // The memory accociated with the instance.  Looked up once on startup
  // and cached here.
  Memory* memory;
};

std::unordered_map<Instance*, WasiInstance*> wasiInstances;

// TODO(sbc): Auto-generate this.

#define WASI_CALLBACK(NAME)                                                 \
  static Result NAME(Thread& thread, const Values& params, Values& results, \
                     Trap::Ptr* trap) {                                     \
    Instance* instance = thread.GetCallerInstance();                        \
    assert(instance);                                                       \
    WasiInstance* wasi_instance = wasiInstances[instance];                  \
    if (wasi_instance->trace_stream) {                                      \
      wasi_instance->trace_stream->Writef(                                  \
          ">>> running wasi function \"%s\":\n", #NAME);                    \
    }                                                                       \
    return wasi_instance->NAME(params, results, trap);                      \
  }

#define WASI_FUNC(NAME) WASI_CALLBACK(NAME)
#include "wasi_api.def"
#undef WASI_FUNC

}  // namespace

namespace wabt {
namespace interp {

Result WasiBindImports(const Module::Ptr& module,
                       RefVec& imports,
                       Stream* stream,
                       Stream* trace_stream) {
  Store* store = module.store();
  for (auto&& import : module->desc().imports) {
    if (import.type.type->kind != ExternKind::Func) {
      stream->Writef("wasi error: invalid import type: %s\n",
                     import.type.name.c_str());
      return Result::Error;
    }

    if (import.type.module != "wasi_snapshot_preview1" &&
        import.type.module != "wasi_unstable") {
      stream->Writef("wasi error: unknown module import: `%s`\n",
                     import.type.module.c_str());
      return Result::Error;
    }

    auto func_type = *cast<FuncType>(import.type.type.get());
    auto import_name = StringPrintf("%s.%s", import.type.module.c_str(),
                                    import.type.name.c_str());
    HostFunc::Ptr host_func;

    // TODO(sbc): Validate signatures of imports.
#define WASI_FUNC(NAME)                                 \
  if (import.type.name == #NAME) {                      \
    host_func = HostFunc::New(*store, func_type, NAME); \
    goto found;                                         \
  }
#include "wasi_api.def"
#undef WASI_FUNC

    stream->Writef("unknown wasi API import: `%s`\n", import.type.name.c_str());
    return Result::Error;
  found:
    imports.push_back(host_func.ref());
  }

  return Result::Ok;
}

Result WasiRunStart(const Instance::Ptr& instance,
                    uvwasi_s* uvwasi,
                    Stream* err_stream,
                    Stream* trace_stream) {
  Store* store = instance.store();
  auto module = store->UnsafeGet<Module>(instance->module());
  auto&& module_desc = module->desc();

  Func::Ptr start;
  Memory::Ptr memory;
  for (auto&& export_ : module_desc.exports) {
    if (export_.type.name == "memory") {
      if (export_.type.type->kind != ExternalKind::Memory) {
        err_stream->Writef("wasi error: memory export has incorrect type\n");
        return Result::Error;
      }
      memory = store->UnsafeGet<Memory>(instance->memories()[export_.index]);
    }
    if (export_.type.name == "_start") {
      if (export_.type.type->kind != ExternalKind::Func) {
        err_stream->Writef("wasi error: _start export is not a function\n");
        return Result::Error;
      }
      start = store->UnsafeGet<Func>(instance->funcs()[export_.index]);
    }
    if (start && memory) { break; }
  }

  if (!start) {
    err_stream->Writef("wasi error: _start export not found\n");
    return Result::Error;
  }

  if (!memory) {
    err_stream->Writef("wasi error: memory export not found\n");
    return Result::Error;
  }

  if (start->type().params.size() || start->type().results.size()) {
    err_stream->Writef("wasi error: invalid _start signature\n");
    return Result::Error;
  }

  // Register memory
  WasiInstance wasi(instance, uvwasi, memory.get(), trace_stream);
  wasiInstances[instance.get()] = &wasi;

  // Call start ([] -> [])
  Values params;
  Values results;
  Trap::Ptr trap;
  Result res = start->Call(*store, params, results, &trap, trace_stream);
  if (trap) { WriteTrap(err_stream, "error", trap); }

  // Unregister memory
  wasiInstances.erase(instance.get());
  return res;
}

}  // namespace interp
}  // namespace wabt

#endif
