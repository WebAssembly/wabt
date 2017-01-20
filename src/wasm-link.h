/*
 * Copyright 2017 WebAssembly Community Group participants
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

#ifndef WASM_LINK_H_
#define WASM_LINK_H_

#include "binary.h"
#include "common.h"
#include "vector.h"

#define WASM_LINK_MODULE_NAME "__extern"

struct WasmLinkerInputBinary;

typedef struct WasmFunctionImport {
  WasmStringSlice name;
  uint32_t sig_index;
  WasmBool active; /* Is this import present in the linked binary */
  struct WasmLinkerInputBinary* foreign_binary;
  uint32_t foreign_index;
} WasmFunctionImport;
WASM_DEFINE_VECTOR(function_import, WasmFunctionImport);

typedef struct WasmGlobalImport {
  WasmStringSlice name;
  WasmType type;
  WasmBool mutable;
} WasmGlobalImport;
WASM_DEFINE_VECTOR(global_import, WasmGlobalImport);

typedef struct WasmDataSegment {
  uint32_t memory_index;
  uint32_t offset;
  const uint8_t* data;
  size_t size;
} WasmDataSegment;
WASM_DEFINE_VECTOR(data_segment, WasmDataSegment);

typedef struct WasmReloc {
  WasmRelocType type;
  size_t offset;
} WasmReloc;
WASM_DEFINE_VECTOR(reloc, WasmReloc);

typedef struct WasmExport {
  WasmExternalKind kind;
  WasmStringSlice name;
  uint32_t index;
} WasmExport;
WASM_DEFINE_VECTOR(export, WasmExport);

typedef struct WasmSectionDataCustom {
  /* Reference to string data stored in the containing InputBinary */
  WasmStringSlice name;
} WasmSectionDataCustom;

typedef struct WasmSection {
  /* The binary to which this section belongs */
  struct WasmLinkerInputBinary* binary;
  WasmRelocVector relocations; /* The relocations for this section */

  WasmBinarySection section_code;
  size_t size;
  size_t offset;

  size_t payload_size;
  size_t payload_offset;

  /* For known sections, the count of the number of elements in the section */
  uint32_t count;

  union {
    /* CUSTOM section data */
    WasmSectionDataCustom data_custom;
    /* DATA section data */
    WasmDataSegmentVector data_segments;
    /* MEMORY section data */
    WasmLimits memory_limits;
  };

  /* The offset at which this section appears within the combined output
   * section. */
  size_t output_payload_offset;
} WasmSection;
WASM_DEFINE_VECTOR(section, WasmSection);

typedef WasmSection* WasmSectionPtr;
WASM_DEFINE_VECTOR(section_ptr, WasmSectionPtr);

WASM_DEFINE_VECTOR(string_slice, WasmStringSlice);

typedef struct WasmLinkerInputBinary {
  const char* filename;
  uint8_t* data;
  size_t size;
  WasmSectionVector sections;

  WasmExportVector exports;

  WasmFunctionImportVector function_imports;
  uint32_t active_function_imports;
  WasmGlobalImportVector global_imports;
  uint32_t active_global_imports;

  uint32_t type_index_offset;
  uint32_t function_index_offset;
  uint32_t imported_function_index_offset;
  uint32_t global_index_offset;
  uint32_t imported_global_index_offset;
  uint32_t table_index_offset;
  uint32_t memory_page_count;
  uint32_t memory_page_offset;

  uint32_t table_elem_count;

  WasmStringSliceVector debug_names;
} WasmLinkerInputBinary;
WASM_DEFINE_VECTOR(binary, WasmLinkerInputBinary);

#endif /* WASM_LINK_H_ */
