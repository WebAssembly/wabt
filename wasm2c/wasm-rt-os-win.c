// Based on
// https://web.archive.org/web/20191012035921/http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system#BSD
// Check for windows (non cygwin) environment
#if defined(_WIN32)

#include "wasm-rt-os.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

#include <Windows.h>

#define DONT_USE_VIRTUAL_ALLOC2

size_t os_getpagesize() {
  SYSTEM_INFO S;
  GetNativeSystemInfo(&S);
  return S.dwPageSize;
}

void* os_mmap(void* hint, size_t size, int prot, int flags) {
  DWORD alloc_flag = MEM_RESERVE | MEM_COMMIT;
  DWORD flProtect = PAGE_NOACCESS;
  size_t request_size, page_size;
  void* addr;

  page_size = os_getpagesize();
  request_size = (size + page_size - 1) & ~(page_size - 1);

  if (request_size < size)
    /* integer overflow */
    return NULL;

  if (request_size == 0)
    request_size = page_size;

  if (prot & MMAP_PROT_EXEC) {
    if (prot & MMAP_PROT_WRITE)
      flProtect = PAGE_EXECUTE_READWRITE;
    else
      flProtect = PAGE_EXECUTE_READ;
  } else if (prot & MMAP_PROT_WRITE)
    flProtect = PAGE_READWRITE;
  else if (prot & MMAP_PROT_READ)
    flProtect = PAGE_READONLY;

  addr = VirtualAlloc((LPVOID)hint, request_size, alloc_flag, flProtect);
  return addr;
}

#ifndef DONT_USE_VIRTUAL_ALLOC2
static void* win_mmap_aligned(void* hint,
                              size_t size,
                              int prot,
                              int flags,
                              size_t pow2alignment) {
  DWORD alloc_flag = MEM_RESERVE | MEM_COMMIT;
  DWORD flProtect = PAGE_NOACCESS;
  size_t request_size, page_size;
  void* addr;

  page_size = os_getpagesize();
  request_size = (size + page_size - 1) & ~(page_size - 1);

  if (request_size < size)
    /* integer overflow */
    return NULL;

  if (request_size == 0)
    request_size = page_size;

  if (prot & MMAP_PROT_EXEC) {
    if (prot & MMAP_PROT_WRITE)
      flProtect = PAGE_EXECUTE_READWRITE;
    else
      flProtect = PAGE_EXECUTE_READ;
  } else if (prot & MMAP_PROT_WRITE)
    flProtect = PAGE_READWRITE;
  else if (prot & MMAP_PROT_READ)
    flProtect = PAGE_READONLY;

  MEM_ADDRESS_REQUIREMENTS addressReqs = {0};
  MEM_EXTENDED_PARAMETER param = {0};

  addressReqs.Alignment = pow2alignment;
  addressReqs.HighestEndingAddress = 0;
  addressReqs.LowestStartingAddress = 0;

  param.Type = MemExtendedParameterAddressRequirements;
  param.Pointer = &addressReqs;

  addr = VirtualAlloc2(0, (LPVOID)addr, request_size, alloc_flag, flProtect,
                       &param, 1);
  return addr;
}
#endif

void os_munmap(void* addr, size_t size) {
  DWORD alloc_flag = MEM_RELEASE;
  if (addr) {
    if (VirtualFree(addr, 0, alloc_flag) == 0) {
      size_t page_size = os_getpagesize();
      size_t request_size = (size + page_size - 1) & ~(page_size - 1);
      int64_t curr_err = errno;
      printf("os_munmap error addr:%p, size:0x%zx, errno:%" PRId64 "\n", addr,
             request_size, curr_err);
    }
  }
}

int os_mprotect(void* addr, size_t size, int prot) {
  DWORD AllocType = MEM_RESERVE | MEM_COMMIT;
  DWORD flProtect = PAGE_NOACCESS;

  if (!addr)
    return 0;

  if (prot & MMAP_PROT_EXEC) {
    if (prot & MMAP_PROT_WRITE)
      flProtect = PAGE_EXECUTE_READWRITE;
    else
      flProtect = PAGE_EXECUTE_READ;
  } else if (prot & MMAP_PROT_WRITE)
    flProtect = PAGE_READWRITE;
  else if (prot & MMAP_PROT_READ)
    flProtect = PAGE_READONLY;

  DWORD old;
  return VirtualProtect((LPVOID)addr, size, flProtect, &old);
}

#ifndef DONT_USE_VIRTUAL_ALLOC2
static int IsPowerOfTwoOrZero(size_t x) {
  return (x & (x - 1)) == 0;
}
#endif

void* os_mmap_aligned(void* addr,
                      size_t requested_length,
                      int prot,
                      int flags,
                      size_t alignment,
                      size_t alignment_offset) {
#ifndef DONT_USE_VIRTUAL_ALLOC2
  if (IsPowerOfTwoOrZero(alignment) && alignment_offset == 0) {
    return win_mmap_aligned(addr, requested_length, prot, flags, alignment);
  } else
#endif
  {
    size_t padded_length = requested_length + alignment + alignment_offset;
    uintptr_t unaligned = (uintptr_t)os_mmap(addr, padded_length, prot, flags);

    if (!unaligned) {
      return (void*)unaligned;
    }

    // Round up the next address that has addr % alignment = 0
    uintptr_t aligned_nonoffset =
        (unaligned + (alignment - 1)) & ~(alignment - 1);

    // Currently offset 0 is aligned according to alignment
    // Alignment needs to be enforced at the given offset
    uintptr_t aligned = 0;
    if ((aligned_nonoffset - alignment_offset) >= unaligned) {
      aligned = aligned_nonoffset - alignment_offset;
    } else {
      aligned = aligned_nonoffset - alignment_offset + alignment;
    }

    if (aligned == unaligned && padded_length == requested_length) {
      return (void*)aligned;
    }

    // Sanity check
    if (aligned < unaligned ||
        (aligned + (requested_length - 1)) >
            (unaligned + (padded_length - 1)) ||
        (aligned + alignment_offset) % alignment != 0) {
      os_munmap((void*)unaligned, padded_length);
      return NULL;
    }

    // windows does not support partial unmapping, so unmap and remap
    os_munmap((void*)unaligned, padded_length);
    aligned =
        (uintptr_t)os_mmap((void*)aligned, requested_length, prot, flags);
    return (void*)aligned;
  }
}

void os_clock_init() {}

int os_clock_gettime(int clock_id, struct timespec* out_struct) {
  return clock_gettime(clock_id, out_struct);
}

int os_clock_getres(int clock_id, struct timespec* out_struct) {
  return clock_getres(clock_id, out_struct);
}

#endif