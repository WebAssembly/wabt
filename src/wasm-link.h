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

#ifndef WABT_LINK_H_
#define WABT_LINK_H_

#include "binary.h"
#include "common.h"
#include "vector.h"

#define WABT_LINK_MODULE_NAME "__extern"

struct WabtLinkerInputBinary;

typedef struct WabtFunctionImport {
  WabtStringSlice name;
  uint32_t sig_index;
  bool active; /* Is this import present in the linked binary */
  struct WabtLinkerInputBinary* foreign_binary;
  uint32_t foreign_index;
} WabtFunctionImport;
WABT_DEFINE_VECTOR(function_import, WabtFunctionImport);

typedef struct WabtGlobalImport {
  WabtStringSlice name;
  WabtType type;
  bool mutable_;
} WabtGlobalImport;
WABT_DEFINE_VECTOR(global_import, WabtGlobalImport);

typedef struct WabtDataSegment {
  uint32_t memory_index;
  uint32_t offset;
  const uint8_t* data;
  size_t size;
} WabtDataSegment;
WABT_DEFINE_VECTOR(data_segment, WabtDataSegment);

typedef struct WabtReloc {
  WabtRelocType type;
  size_t offset;
} WabtReloc;
WABT_DEFINE_VECTOR(reloc, WabtReloc);

typedef struct WabtExport {
  WabtExternalKind kind;
  WabtStringSlice name;
  uint32_t index;
} WabtExport;
WABT_DEFINE_VECTOR(export, WabtExport);

typedef struct WabtSectionDataCustom {
  /* Reference to string data stored in the containing InputBinary */
  WabtStringSlice name;
} WabtSectionDataCustom;

typedef struct WabtSection {
  /* The binary to which this section belongs */
  struct WabtLinkerInputBinary* binary;
  WabtRelocVector relocations; /* The relocations for this section */

  WabtBinarySection section_code;
  size_t size;
  size_t offset;

  size_t payload_size;
  size_t payload_offset;

  /* For known sections, the count of the number of elements in the section */
  uint32_t count;

  union {
    /* CUSTOM section data */
    WabtSectionDataCustom data_custom;
    /* DATA section data */
    WabtDataSegmentVector data_segments;
    /* MEMORY section data */
    WabtLimits memory_limits;
  };

  /* The offset at which this section appears within the combined output
   * section. */
  size_t output_payload_offset;
} WabtSection;
WABT_DEFINE_VECTOR(section, WabtSection);

typedef WabtSection* WabtSectionPtr;
WABT_DEFINE_VECTOR(section_ptr, WabtSectionPtr);

WABT_DEFINE_VECTOR(string_slice, WabtStringSlice);

typedef struct WabtLinkerInputBinary {
  const char* filename;
  uint8_t* data;
  size_t size;
  WabtSectionVector sections;

  WabtExportVector exports;

  WabtFunctionImportVector function_imports;
  uint32_t active_function_imports;
  WabtGlobalImportVector global_imports;
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

  WabtStringSliceVector debug_names;
} WabtLinkerInputBinary;
WABT_DEFINE_VECTOR(binary, WabtLinkerInputBinary);

#endif /* WABT_LINK_H_ */
