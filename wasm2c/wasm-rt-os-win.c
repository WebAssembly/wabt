// Based on https://web.archive.org/web/20191012035921/http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system#BSD
// Check for windows (non cygwin) environment
#if defined(_WIN32)

#include "wasm-rt-os.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-rt-os.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <Windows.h>

size_t os_getpagesize() {
    SYSTEM_INFO S;
    GetNativeSystemInfo(&S);
    return S.dwPageSize;
}

void * os_mmap(void *hint, size_t size, int prot, int flags)
{
    DWORD AllocType = MEM_RESERVE | MEM_COMMIT;
    DWORD flProtect = PAGE_NOACCESS;
    size_t request_size, page_size;
    void *addr;

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
    }
    else if (prot & MMAP_PROT_WRITE)
        flProtect = PAGE_READWRITE;
    else if (prot & MMAP_PROT_READ)
        flProtect = PAGE_READONLY;


    addr = VirtualAlloc((LPVOID)hint, request_size, AllocType,
                        flProtect);
    return addr;
}

void
os_munmap(void *addr, size_t size)
{
    size_t page_size = os_getpagesize();
    size_t request_size = (size + page_size - 1) & ~(page_size - 1);

    if (addr) {
        if (VirtualFree(addr, 0, MEM_RELEASE) == 0) {
            int64_t curr_err = errno;
            printf("os_munmap error addr:%p, size:0x%zx, errno:%" PRId64 "\n",
                    addr, request_size, curr_err);
        }
    }
}

int
os_mprotect(void *addr, size_t size, int prot)
{
    DWORD AllocType = MEM_RESERVE | MEM_COMMIT;
    DWORD flProtect = PAGE_NOACCESS;

    if (!addr)
        return 0;

    if (prot & MMAP_PROT_EXEC) {
        if (prot & MMAP_PROT_WRITE)
            flProtect = PAGE_EXECUTE_READWRITE;
        else
            flProtect = PAGE_EXECUTE_READ;
    }
    else if (prot & MMAP_PROT_WRITE)
        flProtect = PAGE_READWRITE;
    else if (prot & MMAP_PROT_READ)
        flProtect = PAGE_READONLY;

    DWORD old;
    return VirtualProtect((LPVOID)addr, size, flProtect, &old);
}

#endif