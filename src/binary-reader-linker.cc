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

#include "binary-reader-linker.h"

#include <vector>

#include "binary-reader-nop.h"
#include "wasm-link.h"

#define RELOC_SIZE 5

namespace wabt {
namespace link {

namespace {

class BinaryReaderLinker : public BinaryReaderNop {
 public:
  explicit BinaryReaderLinker(LinkerInputBinary* binary);

  virtual Result BeginSection(BinarySection section_type, uint32_t size);

  virtual Result OnImport(uint32_t index,
                          StringSlice module_name,
                          StringSlice field_name);
  virtual Result OnImportFunc(uint32_t import_index,
                              StringSlice module_name,
                              StringSlice field_name,
                              uint32_t func_index,
                              uint32_t sig_index);
  virtual Result OnImportGlobal(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t global_index,
                                Type type,
                                bool mutable_);

  virtual Result OnFunctionCount(uint32_t count);

  virtual Result OnTable(uint32_t index,
                         Type elem_type,
                         const Limits* elem_limits);

  virtual Result OnMemory(uint32_t index, const Limits* limits);

  virtual Result OnExport(uint32_t index,
                          ExternalKind kind,
                          uint32_t item_index,
                          StringSlice name);

  virtual Result OnElemSegmentFunctionIndexCount(uint32_t index,
                                                 uint32_t count);

  virtual Result BeginDataSegment(uint32_t index, uint32_t memory_index);
  virtual Result OnDataSegmentData(uint32_t index,
                                   const void* data,
                                   uint32_t size);

  virtual Result BeginNamesSection(uint32_t size);

  virtual Result OnFunctionName(uint32_t function_index,
                                StringSlice function_name);

  virtual Result OnRelocCount(uint32_t count,
                              BinarySection section_code,
                              StringSlice section_name);
  virtual Result OnReloc(RelocType type,
                         uint32_t offset,
                         uint32_t index,
                         int32_t addend);

  virtual Result OnInitExprI32ConstExpr(uint32_t index, uint32_t value);

 private:
  LinkerInputBinary* binary;

  Section* reloc_section = nullptr;
  Section* current_section = nullptr;
  uint32_t function_count = 0;
};

BinaryReaderLinker::BinaryReaderLinker(LinkerInputBinary* binary)
    : binary(binary) {}

Result BinaryReaderLinker::OnRelocCount(uint32_t count,
                                        BinarySection section_code,
                                        StringSlice section_name) {
  if (section_code == BinarySection::Custom) {
    WABT_FATAL("relocation for custom sections not yet supported\n");
  }

  for (const std::unique_ptr<Section>& section : binary->sections) {
    if (section->section_code != section_code)
      continue;
    reloc_section = section.get();
    return Result::Ok;
  }

  WABT_FATAL("section not found: %d\n", static_cast<int>(section_code));
  return Result::Error;
}

Result BinaryReaderLinker::OnReloc(RelocType type,
                                   uint32_t offset,
                                   uint32_t index,
                                   int32_t addend) {
  if (offset + RELOC_SIZE > reloc_section->size) {
    WABT_FATAL("invalid relocation offset: %#x\n", offset);
  }

  reloc_section->relocations.emplace_back(type, offset, index, addend);

  return Result::Ok;
}

Result BinaryReaderLinker::OnImport(uint32_t index,
                                    StringSlice module_name,
                                    StringSlice field_name) {
  if (!string_slice_eq_cstr(&module_name, WABT_LINK_MODULE_NAME)) {
    WABT_FATAL("unsupported import module: " PRIstringslice,
               WABT_PRINTF_STRING_SLICE_ARG(module_name));
  }
  return Result::Ok;
}

Result BinaryReaderLinker::OnImportFunc(uint32_t import_index,
                                        StringSlice module_name,
                                        StringSlice field_name,
                                        uint32_t global_index,
                                        uint32_t sig_index) {
  binary->function_imports.emplace_back();
  FunctionImport* import = &binary->function_imports.back();
  import->name = field_name;
  import->sig_index = sig_index;
  import->active = true;
  binary->active_function_imports++;
  return Result::Ok;
}

Result BinaryReaderLinker::OnImportGlobal(uint32_t import_index,
                                          StringSlice module_name,
                                          StringSlice field_name,
                                          uint32_t global_index,
                                          Type type,
                                          bool mutable_) {
  binary->global_imports.emplace_back();
  GlobalImport* import = &binary->global_imports.back();
  import->name = field_name;
  import->type = type;
  import->mutable_ = mutable_;
  binary->active_global_imports++;
  return Result::Ok;
}

Result BinaryReaderLinker::OnFunctionCount(uint32_t count) {
  function_count = count;
  return Result::Ok;
}

Result BinaryReaderLinker::BeginSection(BinarySection section_code,
                                        uint32_t size) {
  Section* sec = new Section();
  binary->sections.emplace_back(sec);
  current_section = sec;
  sec->section_code = section_code;
  sec->size = size;
  sec->offset = state->offset;
  sec->binary = binary;

  if (sec->section_code != BinarySection::Custom &&
      sec->section_code != BinarySection::Start) {
    size_t bytes_read = read_u32_leb128(
        &binary->data[sec->offset], &binary->data[binary->size], &sec->count);
    if (bytes_read == 0)
      WABT_FATAL("error reading section element count\n");
    sec->payload_offset = sec->offset + bytes_read;
    sec->payload_size = sec->size - bytes_read;
  }
  return Result::Ok;
}

Result BinaryReaderLinker::OnTable(uint32_t index,
                                   Type elem_type,
                                   const Limits* elem_limits) {
  if (elem_limits->has_max && (elem_limits->max != elem_limits->initial))
    WABT_FATAL("Tables with max != initial not supported by wabt-link\n");

  binary->table_elem_count = elem_limits->initial;
  return Result::Ok;
}

Result BinaryReaderLinker::OnElemSegmentFunctionIndexCount(uint32_t index,
                                                           uint32_t count) {
  Section* sec = current_section;

  /* Modify the payload to include only the actual function indexes */
  size_t delta = state->offset - sec->payload_offset;
  sec->payload_offset += delta;
  sec->payload_size -= delta;
  return Result::Ok;
}

Result BinaryReaderLinker::OnMemory(uint32_t index, const Limits* page_limits) {
  Section* sec = current_section;
  sec->data.memory_limits = *page_limits;
  binary->memory_page_count = page_limits->initial;
  return Result::Ok;
}

Result BinaryReaderLinker::BeginDataSegment(uint32_t index,
                                            uint32_t memory_index) {
  Section* sec = current_section;
  if (!sec->data.data_segments) {
    sec->data.data_segments = new std::vector<DataSegment>();
  }
  sec->data.data_segments->emplace_back();
  DataSegment& segment = sec->data.data_segments->back();
  segment.memory_index = memory_index;
  return Result::Ok;
}

Result BinaryReaderLinker::OnInitExprI32ConstExpr(uint32_t index,
                                                  uint32_t value) {
  Section* sec = current_section;
  if (sec->section_code != BinarySection::Data)
    return Result::Ok;
  DataSegment& segment = sec->data.data_segments->back();
  segment.offset = value;
  return Result::Ok;
}

Result BinaryReaderLinker::OnDataSegmentData(uint32_t index,
                                             const void* src_data,
                                             uint32_t size) {
  Section* sec = current_section;
  DataSegment& segment = sec->data.data_segments->back();
  segment.data = static_cast<const uint8_t*>(src_data);
  segment.size = size;
  return Result::Ok;
}

Result BinaryReaderLinker::OnExport(uint32_t index,
                                    ExternalKind kind,
                                    uint32_t item_index,
                                    StringSlice name) {
  binary->exports.emplace_back();
  Export* export_ = &binary->exports.back();
  export_->name = name;
  export_->kind = kind;
  export_->index = item_index;
  return Result::Ok;
}

Result BinaryReaderLinker::BeginNamesSection(uint32_t size) {
  binary->debug_names.resize(function_count + binary->function_imports.size());
  return Result::Ok;
}

Result BinaryReaderLinker::OnFunctionName(uint32_t index, StringSlice name) {
  binary->debug_names[index] = string_slice_to_string(name);
  return Result::Ok;
}

}  // namespace

Result read_binary_linker(LinkerInputBinary* input_info, LinkOptions* options) {
  BinaryReaderLinker reader(input_info);

  ReadBinaryOptions read_options = WABT_READ_BINARY_OPTIONS_DEFAULT;
  read_options.read_debug_names = true;
  read_options.log_stream = options->log_stream;
  return read_binary(input_info->data, input_info->size, &reader,
                     &read_options);
}

}  // namespace link
}  // namespace wabt
