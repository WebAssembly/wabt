#ifndef WASM_WRITER_H_
#define WASM_WRITER_H_

#include <stdio.h>

#include "wasm-common.h"

typedef struct WasmWriter {
  void* user_data;
  WasmResult (*write_data)(size_t offset,
                           const void* data,
                           size_t size,
                           void* user_data);
} WasmWriter;

typedef struct WasmOutputBuffer {
  void* start;
  size_t size;
  size_t capacity;
} WasmOutputBuffer;

typedef struct WasmMemoryWriter {
  WasmWriter base;
  WasmOutputBuffer buf;
} WasmMemoryWriter;

typedef struct WasmFileWriter {
  WasmWriter base;
  FILE* file;
  size_t offset;
} WasmFileWriter;

WasmResult wasm_init_file_writer(WasmFileWriter* writer, const char* filename);
void wasm_close_file_writer(WasmFileWriter* writer);
WasmResult wasm_init_mem_writer(WasmMemoryWriter* writer);
void wasm_close_mem_writer(WasmMemoryWriter* writer);

#endif /* WASM_WRITER_H_ */
