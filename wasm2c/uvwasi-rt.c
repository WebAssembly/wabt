#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wasm-rt.h"
#include "uvwasi.h"
#include "uvwasi-rt.h"

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

#define MEMACCESS(addr) ((void*)&wp->instance_memory->data[(addr)])

#define MEM_SET(addr, value, len) memset(MEMACCESS(addr), value, len)
#define MEM_WRITE8(addr, value)  (*(u8*) MEMACCESS(addr)) = value
#define MEM_WRITE16(addr, value) (*(u16*)MEMACCESS(addr)) = value
#define MEM_WRITE32(addr, value) (*(u32*)MEMACCESS(addr)) = value
#define MEM_WRITE64(addr, value) (*(u64*)MEMACCESS(addr)) = value

#define MEM_READ32(addr) (*(u32*)MEMACCESS(addr))
#define READ32(x)   (*(u32*)(x))

// XXX TODO: Add linear memory boundary checks



typedef u32 wasm_ptr;

u32 Z_wasi_snapshot_preview1Z_fd_prestat_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr buf)
{
    uvwasi_prestat_t prestat;
    uvwasi_errno_t ret = uvwasi_fd_prestat_get(wp->uvwasi, fd, &prestat);
    if (ret == UVWASI_ESUCCESS) {
        MEM_WRITE32(buf+0, prestat.pr_type);
        MEM_WRITE32(buf+4, prestat.u.dir.pr_name_len);
    }
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_fd_prestat_dir_name(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr path, u32 path_len)
{
    uvwasi_errno_t ret = uvwasi_fd_prestat_dir_name(wp->uvwasi, fd, (char*)MEMACCESS(path), path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_environ_sizes_get(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr env_count, wasm_ptr env_buf_size)
{
    uvwasi_size_t uvcount;
    uvwasi_size_t uvbufsize;
    uvwasi_errno_t ret = uvwasi_environ_sizes_get(wp->uvwasi, &uvcount, &uvbufsize);
    if (ret == UVWASI_ESUCCESS) {
        MEM_WRITE32(env_count,      uvcount);
        MEM_WRITE32(env_buf_size,   uvbufsize);
    }
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_environ_get(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr env, wasm_ptr buf)
{
    uvwasi_size_t uvcount;
    uvwasi_size_t uvbufsize;
    uvwasi_errno_t ret;
    ret = uvwasi_environ_sizes_get(wp->uvwasi, &uvcount, &uvbufsize);
    if (ret != UVWASI_ESUCCESS) {
        return ret;
    }

    // TODO XXX: check mem

    char** uvenv = calloc(uvcount, sizeof(char*));
    if (uvenv == NULL) {
        return UVWASI_ENOMEM;
    }

    ret = uvwasi_environ_get(wp->uvwasi, uvenv, (char*)MEMACCESS(buf));
    if (ret != UVWASI_ESUCCESS) {
        free(uvenv);
        return ret;
    }

    for (u32 i = 0; i < uvcount; ++i)
    {
        uint32_t offset = buf + (uvenv[i] - uvenv[0]);
        MEM_WRITE32(env+(i*sizeof(wasm_ptr)), offset);
    }

    free(uvenv);

    return ret;
}

u32 Z_wasi_snapshot_preview1Z_args_sizes_get(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr argc, wasm_ptr argv_buf_size)
{
    uvwasi_size_t uvcount;
    uvwasi_size_t uvbufsize;
    uvwasi_errno_t ret = uvwasi_args_sizes_get(wp->uvwasi, &uvcount, &uvbufsize);
    if (ret == UVWASI_ESUCCESS) {
        MEM_WRITE32(argc,            uvcount);
        MEM_WRITE32(argv_buf_size,   uvbufsize);
    }
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_args_get(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr argv, wasm_ptr buf)
{
    uvwasi_size_t uvcount;
    uvwasi_size_t uvbufsize;
    uvwasi_errno_t ret;
    ret = uvwasi_args_sizes_get(wp->uvwasi, &uvcount, &uvbufsize);
    if (ret != UVWASI_ESUCCESS) {
        return ret;
    }

    // TODO XXX: check mem

    char** uvarg = calloc(uvcount, sizeof(char*));
    if (uvarg == NULL) {
        return UVWASI_ENOMEM;
    }

    ret = uvwasi_args_get(wp->uvwasi, uvarg, (char*)MEMACCESS(buf));
    if (ret != UVWASI_ESUCCESS) {
        free(uvarg);
        return ret;
    }

    for (u32 i = 0; i < uvcount; ++i)
    {
        uint32_t offset = buf + (uvarg[i] - uvarg[0]);
        MEM_WRITE32(argv+(i*sizeof(wasm_ptr)), offset);
    }

    free(uvarg);

    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_fdstat_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr stat)
{
    uvwasi_fdstat_t uvstat;
    uvwasi_errno_t ret = uvwasi_fd_fdstat_get(wp->uvwasi, fd, &uvstat);
    if (ret == UVWASI_ESUCCESS) {
        MEM_SET(stat, 0, 24);
        MEM_WRITE8 (stat+0,  uvstat.fs_filetype);
        MEM_WRITE16(stat+2,  uvstat.fs_flags);
        MEM_WRITE64(stat+8,  uvstat.fs_rights_base);
        MEM_WRITE64(stat+16, uvstat.fs_rights_inheriting);
    }
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_fdstat_set_flags(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u32 flags)
{
    uvwasi_errno_t ret = uvwasi_fd_fdstat_set_flags(wp->uvwasi, fd, flags);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_fdstat_set_rights(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 fs_rights_base, u64 fs_rights_inheriting)
{
    uvwasi_errno_t ret = uvwasi_fd_fdstat_set_rights(wp->uvwasi, fd, fs_rights_base, fs_rights_inheriting);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_filestat_set_times(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u32 flags, wasm_ptr path, u32 path_len, u64 atim, u64 mtim, u32 fst_flags)
{
    uvwasi_errno_t ret = uvwasi_path_filestat_set_times(wp->uvwasi, fd, flags, (char*)MEMACCESS(path), path_len, atim, mtim, fst_flags);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_filestat_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u32 flags, wasm_ptr path, u32 path_len, wasm_ptr stat)
{
    uvwasi_filestat_t uvstat;
    uvwasi_errno_t ret = uvwasi_path_filestat_get(wp->uvwasi, fd, flags, (char*)MEMACCESS(path), path_len, &uvstat);
    if (ret == UVWASI_ESUCCESS) {
        MEM_SET(stat, 0, 64);
        MEM_WRITE64(stat+0,  uvstat.st_dev);
        MEM_WRITE64(stat+8,  uvstat.st_ino);
        MEM_WRITE8 (stat+16, uvstat.st_filetype);
        MEM_WRITE64(stat+24, uvstat.st_nlink);
        MEM_WRITE64(stat+32, uvstat.st_size);
        MEM_WRITE64(stat+40, uvstat.st_atim);
        MEM_WRITE64(stat+48, uvstat.st_mtim);
        MEM_WRITE64(stat+56, uvstat.st_ctim);
    }
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_filestat_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr stat)
{
    uvwasi_filestat_t uvstat;
    uvwasi_errno_t ret = uvwasi_fd_filestat_get(wp->uvwasi, fd, &uvstat);
    if (ret == UVWASI_ESUCCESS) {
        MEM_SET(stat, 0, 64);
        MEM_WRITE64(stat+0,  uvstat.st_dev);
        MEM_WRITE64(stat+8,  uvstat.st_ino);
        MEM_WRITE8 (stat+16, uvstat.st_filetype);
        MEM_WRITE64(stat+24, uvstat.st_nlink);
        MEM_WRITE64(stat+32, uvstat.st_size);
        MEM_WRITE64(stat+40, uvstat.st_atim);
        MEM_WRITE64(stat+48, uvstat.st_mtim);
        MEM_WRITE64(stat+56, uvstat.st_ctim);
    }
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_seek(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 offset, u32 wasi_whence, wasm_ptr pos)
{
    uvwasi_whence_t whence = -1;
    switch (wasi_whence) {
    case 0: whence = UVWASI_WHENCE_SET; break;
    case 1: whence = UVWASI_WHENCE_CUR; break;
    case 2: whence = UVWASI_WHENCE_END; break;
    }

    uvwasi_filesize_t uvpos;
    uvwasi_errno_t ret = uvwasi_fd_seek(wp->uvwasi, fd, offset, whence, &uvpos);
    MEM_WRITE64(pos, uvpos);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_tell(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr pos)
{
    uvwasi_filesize_t uvpos;
    uvwasi_errno_t ret = uvwasi_fd_tell(wp->uvwasi, fd, &uvpos);
    MEM_WRITE64(pos, uvpos);
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_fd_filestat_set_size(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 filesize)
{
    uvwasi_errno_t ret = uvwasi_fd_filestat_set_size(wp->uvwasi, fd, filesize);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_filestat_set_times(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 atim, u64 mtim, u32 fst_flags)
{
    uvwasi_errno_t ret = uvwasi_fd_filestat_set_times(wp->uvwasi, fd, atim, mtim, fst_flags);
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_fd_sync(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd)
{
    uvwasi_errno_t ret = uvwasi_fd_sync(wp->uvwasi, fd);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_datasync(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd)
{
    uvwasi_errno_t ret = uvwasi_fd_datasync(wp->uvwasi, fd);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_renumber(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd_from, u32 fd_to)
{
    uvwasi_errno_t ret = uvwasi_fd_renumber(wp->uvwasi, fd_from, fd_to);
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_fd_allocate(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 offset, u64 len)
{
    uvwasi_errno_t ret = uvwasi_fd_allocate(wp->uvwasi, fd, offset, len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_advise(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, u64 offset, u64 len, u32 advice)
{
    uvwasi_errno_t ret = uvwasi_fd_advise(wp->uvwasi, fd, offset, len, advice);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_open(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 dirfd, u32 dirflags,
                                                    wasm_ptr path, u32 path_len,
                                                    u32 oflags, u64 fs_rights_base, u64 fs_rights_inheriting,
                                                    u32 fs_flags, wasm_ptr fd)
{
    uvwasi_fd_t uvfd;
    uvwasi_errno_t ret = uvwasi_path_open(wp->uvwasi,
                                 dirfd,
                                 dirflags,
                                 (char*)MEMACCESS(path),
                                 path_len,
                                 oflags,
                                 fs_rights_base,
                                 fs_rights_inheriting,
                                 fs_flags,
                                 &uvfd);
    MEM_WRITE32(fd, uvfd);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_close(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd)
{
    uvwasi_errno_t ret = uvwasi_fd_close(wp->uvwasi, fd);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_symlink(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr old_path, u32 old_path_len,
	u32 fd, wasm_ptr new_path, u32 new_path_len)
{
    uvwasi_errno_t ret = uvwasi_path_symlink(wp->uvwasi, (char*)MEMACCESS(old_path), old_path_len,
                                                  fd, (char*)MEMACCESS(new_path), new_path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_rename(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 old_fd, wasm_ptr old_path,
	u32 old_path_len, u32 new_fd, wasm_ptr new_path, u32 new_path_len)
{
    uvwasi_errno_t ret = uvwasi_path_rename(wp->uvwasi, old_fd, (char*)MEMACCESS(old_path), old_path_len,
                                                     new_fd, (char*)MEMACCESS(new_path), new_path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_link(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 old_fd, u32 old_flags,
					wasm_ptr old_path, u32 old_path_len, u32 new_fd, wasm_ptr new_path, u32 new_path_len)
{
    uvwasi_errno_t ret = uvwasi_path_link(wp->uvwasi, old_fd, old_flags, (char*)MEMACCESS(old_path), old_path_len,
                                                   new_fd,            (char*)MEMACCESS(new_path), new_path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_unlink_file(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr path, u32 path_len)
{
    uvwasi_errno_t ret = uvwasi_path_unlink_file(wp->uvwasi, fd, (char*)MEMACCESS(path), path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_readlink(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr path, u32 path_len, wasm_ptr buf, u32 buf_len, wasm_ptr bufused)
{
    uvwasi_size_t uvbufused;
    uvwasi_errno_t ret = uvwasi_path_readlink(wp->uvwasi, fd, (char*)MEMACCESS(path), path_len, MEMACCESS(buf), buf_len, &uvbufused);

    MEM_WRITE32(bufused, uvbufused);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_create_directory(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr path, u32 path_len)
{
    uvwasi_errno_t ret = uvwasi_path_create_directory(wp->uvwasi, fd, (char*)MEMACCESS(path), path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_path_remove_directory(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr path, u32 path_len)
{
    uvwasi_errno_t ret = uvwasi_path_remove_directory(wp->uvwasi, fd, (char*)MEMACCESS(path), path_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_readdir(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr buf, u32 buf_len, u64 cookie, wasm_ptr bufused)
{
    uvwasi_size_t uvbufused;
    uvwasi_errno_t ret = uvwasi_fd_readdir(wp->uvwasi, fd, MEMACCESS(buf), buf_len, cookie, &uvbufused);
    MEM_WRITE32(bufused, uvbufused);
    return ret;
}

typedef struct wasi_iovec_t
{
    uvwasi_size_t buf;
    uvwasi_size_t buf_len;
} wasi_iovec_t;

u32 Z_wasi_snapshot_preview1Z_fd_write(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr iovs_offset, u32 iovs_len, wasm_ptr nwritten)
{
    wasi_iovec_t * wasi_iovs = (wasi_iovec_t *)MEMACCESS(iovs_offset);

#if defined(_MSC_VER)
    if (iovs_len > 32) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[32];
#else
    if (iovs_len > 128) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[iovs_len];
#endif
    for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
        iovs[i].buf = MEMACCESS(READ32(&wasi_iovs[i].buf));
        iovs[i].buf_len = READ32(&wasi_iovs[i].buf_len);
    }

    uvwasi_size_t num_written;
    uvwasi_errno_t ret = uvwasi_fd_write(wp->uvwasi, fd, iovs, iovs_len, &num_written);
    MEM_WRITE32(nwritten, num_written);
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_fd_pwrite(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr iovs_offset, u32 iovs_len, u64 offset, wasm_ptr nwritten)
{
    wasi_iovec_t * wasi_iovs = (wasi_iovec_t *)MEMACCESS(iovs_offset);

#if defined(_MSC_VER)
    if (iovs_len > 32) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[32];
#else
    if (iovs_len > 128) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[iovs_len];
#endif
    for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
        iovs[i].buf = MEMACCESS(READ32(&wasi_iovs[i].buf));
        iovs[i].buf_len = READ32(&wasi_iovs[i].buf_len);
    }

    uvwasi_size_t num_written;
    uvwasi_errno_t ret = uvwasi_fd_pwrite(wp->uvwasi, fd, iovs, iovs_len, offset, &num_written);
    MEM_WRITE32(nwritten, num_written);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_read(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr iovs_offset, u32 iovs_len, wasm_ptr nread)
{
    wasi_iovec_t * wasi_iovs = (wasi_iovec_t *)MEMACCESS(iovs_offset);

#if defined(_MSC_VER)
    if (iovs_len > 32) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[32];
#else
    if (iovs_len > 128) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[iovs_len];
#endif

    for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
        iovs[i].buf = MEMACCESS(READ32(&wasi_iovs[i].buf));
        iovs[i].buf_len = READ32(&wasi_iovs[i].buf_len);
    }

    uvwasi_size_t num_read;
    uvwasi_errno_t ret = uvwasi_fd_read(wp->uvwasi, fd, (const uvwasi_iovec_t *)iovs, iovs_len, &num_read);
    MEM_WRITE32(nread, num_read);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_fd_pread(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 fd, wasm_ptr iovs_offset, u32 iovs_len, u64 offset, wasm_ptr nread)
{
    wasi_iovec_t * wasi_iovs = (wasi_iovec_t *)MEMACCESS(iovs_offset);

#if defined(_MSC_VER)
    if (iovs_len > 32) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[32];
#else
    if (iovs_len > 128) return UVWASI_EINVAL;
    uvwasi_ciovec_t  iovs[iovs_len];
#endif

    for (uvwasi_size_t i = 0; i < iovs_len; ++i) {
        iovs[i].buf = MEMACCESS(READ32(&wasi_iovs[i].buf));
        iovs[i].buf_len = READ32(&wasi_iovs[i].buf_len);
    }

    uvwasi_size_t num_read;
    uvwasi_errno_t ret = uvwasi_fd_pread(wp->uvwasi, fd, (const uvwasi_iovec_t *)iovs, iovs_len, offset, &num_read);
    MEM_WRITE32(nread, num_read);
    return ret;
}

// TODO XXX: audit, compare with spec
u32 Z_wasi_snapshot_preview1Z_poll_oneoff(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr in, wasm_ptr out, u32 nsubscriptions, wasm_ptr nevents)
{
    uvwasi_size_t uvnevents;
    uvwasi_errno_t ret = uvwasi_poll_oneoff(wp->uvwasi, MEMACCESS(in), MEMACCESS(out), nsubscriptions, &uvnevents);
    MEM_WRITE32(nevents, uvnevents);
    return ret;
}


u32 Z_wasi_snapshot_preview1Z_clock_res_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 clk_id, wasm_ptr result)
{
    uvwasi_timestamp_t t;
    uvwasi_errno_t ret = uvwasi_clock_res_get(wp->uvwasi, clk_id, &t);
    MEM_WRITE64(result, t);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_clock_time_get(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 clk_id, u64 precision, wasm_ptr result)
{
    uvwasi_timestamp_t t;
    uvwasi_errno_t ret = uvwasi_clock_time_get(wp->uvwasi, clk_id, precision, &t);
    MEM_WRITE64(result, t);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_random_get(struct Z_wasi_snapshot_preview1_instance_t* wp, wasm_ptr buf, u32 buf_len)
{
    uvwasi_errno_t ret = uvwasi_random_get(wp->uvwasi, MEMACCESS(buf), buf_len);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_sched_yield(struct Z_wasi_snapshot_preview1_instance_t* wp)
{
    uvwasi_errno_t ret = uvwasi_sched_yield(wp->uvwasi);
    return ret;
}

u32 Z_wasi_snapshot_preview1Z_proc_raise(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 sig)
{
    uvwasi_errno_t ret = uvwasi_proc_raise(wp->uvwasi, sig);
    return ret;
}

void Z_wasi_snapshot_preview1Z_proc_exit(struct Z_wasi_snapshot_preview1_instance_t* wp, u32 code)
{
    uvwasi_destroy(wp->uvwasi);
    exit(code);
}
