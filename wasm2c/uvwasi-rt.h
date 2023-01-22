#ifndef UVWASI_RT_H
#define UVWASI_RT_H

#include "uvwasi.h"
#include "wasm-rt.h"

struct Z_wasi_snapshot_preview1_instance_t {
    uvwasi_t * uvwasi;
    wasm_rt_memory_t * instance_memory;
};

#endif



