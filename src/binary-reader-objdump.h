/*
 * Copyright 2016 WebAssembly Community Group participants
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

#ifndef WABT_BINARY_READER_OBJDUMP_H_
#define WABT_BINARY_READER_OBJDUMP_H_

#include "common.h"
#include "stream.h"
#include "vector.h"

struct WabtModule;
struct WabtReadBinaryOptions;

typedef struct WabtReloc {
  WabtRelocType type;
  size_t offset;
} WabtReloc;
WABT_DEFINE_VECTOR(reloc, WabtReloc);

WABT_DEFINE_VECTOR(string_slice, WabtStringSlice);

typedef enum WabtObjdumpMode {
  WABT_DUMP_PREPASS,
  WABT_DUMP_HEADERS,
  WABT_DUMP_DETAILS,
  WABT_DUMP_DISASSEMBLE,
  WABT_DUMP_RAW_DATA,
} WabtObjdumpMode;

typedef struct WabtObjdumpOptions {
  bool headers;
  bool details;
  bool raw;
  bool disassemble;
  bool debug;
  bool relocs;
  WabtObjdumpMode mode;
  const char* infile;
  const char* section_name;
  bool print_header;
  WabtStringSliceVector function_names;
  WabtRelocVector code_relocations;
} WabtObjdumpOptions;

WABT_EXTERN_C_BEGIN

WabtResult wabt_read_binary_objdump(const uint8_t* data,
                                    size_t size,
                                    WabtObjdumpOptions* options);

WABT_EXTERN_C_END

#endif /* WABT_BINARY_READER_OBJDUMP_H_ */
