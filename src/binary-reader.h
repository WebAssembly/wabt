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

#ifndef WABT_BINARY_READER_H_
#define WABT_BINARY_READER_H_

#include <stddef.h>
#include <stdint.h>

#include "binary.h"
#include "common.h"

#define WABT_READ_BINARY_OPTIONS_DEFAULT \
  { nullptr, false }

namespace wabt {

class Stream;

struct ReadBinaryOptions {
  Stream* log_stream;
  bool read_debug_names;
};

class BinaryReader {
 public:
  struct State {
    const uint8_t* data;
    size_t size;
    size_t offset;
  };

  virtual ~BinaryReader() {}

  virtual bool OnError(const char* message) = 0;
  virtual void OnSetState(const State* s) { state = s; }

  /* Module */
  virtual Result BeginModule(uint32_t version) = 0;
  virtual Result EndModule() = 0;

  virtual Result BeginSection(BinarySection section_type, uint32_t size) = 0;

  /* Custom section */
  virtual Result BeginCustomSection(uint32_t size,
                                    StringSlice section_name) = 0;
  virtual Result EndCustomSection() = 0;

  /* Type section */
  virtual Result BeginTypeSection(uint32_t size) = 0;
  virtual Result OnTypeCount(uint32_t count) = 0;
  virtual Result OnType(uint32_t index,
                        uint32_t param_count,
                        Type* param_types,
                        uint32_t result_count,
                        Type* result_types) = 0;
  virtual Result EndTypeSection() = 0;

  /* Import section */
  virtual Result BeginImportSection(uint32_t size) = 0;
  virtual Result OnImportCount(uint32_t count) = 0;
  virtual Result OnImport(uint32_t index,
                          StringSlice module_name,
                          StringSlice field_name) = 0;
  virtual Result OnImportFunc(uint32_t import_index,
                              StringSlice module_name,
                              StringSlice field_name,
                              uint32_t func_index,
                              uint32_t sig_index) = 0;
  virtual Result OnImportTable(uint32_t import_index,
                               StringSlice module_name,
                               StringSlice field_name,
                               uint32_t table_index,
                               Type elem_type,
                               const Limits* elem_limits) = 0;
  virtual Result OnImportMemory(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t memory_index,
                                const Limits* page_limits) = 0;
  virtual Result OnImportGlobal(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t global_index,
                                Type type,
                                bool mutable_) = 0;
  virtual Result EndImportSection() = 0;

  /* Function section */
  virtual Result BeginFunctionSection(uint32_t size) = 0;
  virtual Result OnFunctionCount(uint32_t count) = 0;
  virtual Result OnFunction(uint32_t index, uint32_t sig_index) = 0;
  virtual Result EndFunctionSection() = 0;

  /* Table section */
  virtual Result BeginTableSection(uint32_t size) = 0;
  virtual Result OnTableCount(uint32_t count) = 0;
  virtual Result OnTable(uint32_t index,
                         Type elem_type,
                         const Limits* elem_limits) = 0;
  virtual Result EndTableSection() = 0;

  /* Memory section */
  virtual Result BeginMemorySection(uint32_t size) = 0;
  virtual Result OnMemoryCount(uint32_t count) = 0;
  virtual Result OnMemory(uint32_t index, const Limits* limits) = 0;
  virtual Result EndMemorySection() = 0;

  /* Global section */
  virtual Result BeginGlobalSection(uint32_t size) = 0;
  virtual Result OnGlobalCount(uint32_t count) = 0;
  virtual Result BeginGlobal(uint32_t index, Type type, bool mutable_) = 0;
  virtual Result BeginGlobalInitExpr(uint32_t index) = 0;
  virtual Result EndGlobalInitExpr(uint32_t index) = 0;
  virtual Result EndGlobal(uint32_t index) = 0;
  virtual Result EndGlobalSection() = 0;

  /* Exports section */
  virtual Result BeginExportSection(uint32_t size) = 0;
  virtual Result OnExportCount(uint32_t count) = 0;
  virtual Result OnExport(uint32_t index,
                          ExternalKind kind,
                          uint32_t item_index,
                          StringSlice name) = 0;
  virtual Result EndExportSection() = 0;

  /* Start section */
  virtual Result BeginStartSection(uint32_t size) = 0;
  virtual Result OnStartFunction(uint32_t func_index) = 0;
  virtual Result EndStartSection() = 0;

  /* Code section */
  virtual Result BeginCodeSection(uint32_t size) = 0;
  virtual Result OnFunctionBodyCount(uint32_t count) = 0;
  virtual Result BeginFunctionBody(uint32_t index) = 0;
  virtual Result OnLocalDeclCount(uint32_t count) = 0;
  virtual Result OnLocalDecl(uint32_t decl_index,
                             uint32_t count,
                             Type type) = 0;

  /* Function expressions; called between BeginFunctionBody and
   EndFunctionBody */
  virtual Result OnOpcode(Opcode Opcode) = 0;
  virtual Result OnOpcodeBare() = 0;
  virtual Result OnOpcodeUint32(uint32_t value) = 0;
  virtual Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) = 0;
  virtual Result OnOpcodeUint64(uint64_t value) = 0;
  virtual Result OnOpcodeF32(uint32_t value) = 0;
  virtual Result OnOpcodeF64(uint64_t value) = 0;
  virtual Result OnOpcodeBlockSig(uint32_t num_types, Type* sig_types) = 0;
  virtual Result OnBinaryExpr(Opcode opcode) = 0;
  virtual Result OnBlockExpr(uint32_t num_types, Type* sig_types) = 0;
  virtual Result OnBrExpr(uint32_t depth) = 0;
  virtual Result OnBrIfExpr(uint32_t depth) = 0;
  virtual Result OnBrTableExpr(uint32_t num_targets,
                               uint32_t* target_depths,
                               uint32_t default_target_depth) = 0;
  virtual Result OnCallExpr(uint32_t func_index) = 0;
  virtual Result OnCallIndirectExpr(uint32_t sig_index) = 0;
  virtual Result OnCompareExpr(Opcode opcode) = 0;
  virtual Result OnConvertExpr(Opcode opcode) = 0;
  virtual Result OnCurrentMemoryExpr() = 0;
  virtual Result OnDropExpr() = 0;
  virtual Result OnElseExpr() = 0;
  virtual Result OnEndExpr() = 0;
  virtual Result OnEndFunc() = 0;
  virtual Result OnF32ConstExpr(uint32_t value_bits) = 0;
  virtual Result OnF64ConstExpr(uint64_t value_bits) = 0;
  virtual Result OnGetGlobalExpr(uint32_t global_index) = 0;
  virtual Result OnGetLocalExpr(uint32_t local_index) = 0;
  virtual Result OnGrowMemoryExpr() = 0;
  virtual Result OnI32ConstExpr(uint32_t value) = 0;
  virtual Result OnI64ConstExpr(uint64_t value) = 0;
  virtual Result OnIfExpr(uint32_t num_types, Type* sig_types) = 0;
  virtual Result OnLoadExpr(Opcode opcode,
                            uint32_t alignment_log2,
                            uint32_t offset) = 0;
  virtual Result OnLoopExpr(uint32_t num_types, Type* sig_types) = 0;
  virtual Result OnNopExpr() = 0;
  virtual Result OnReturnExpr() = 0;
  virtual Result OnSelectExpr() = 0;
  virtual Result OnSetGlobalExpr(uint32_t global_index) = 0;
  virtual Result OnSetLocalExpr(uint32_t local_index) = 0;
  virtual Result OnStoreExpr(Opcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset) = 0;
  virtual Result OnTeeLocalExpr(uint32_t local_index) = 0;
  virtual Result OnUnaryExpr(Opcode opcode) = 0;
  virtual Result OnUnreachableExpr() = 0;
  virtual Result EndFunctionBody(uint32_t index) = 0;
  virtual Result EndCodeSection() = 0;

  /* Elem section */
  virtual Result BeginElemSection(uint32_t size) = 0;
  virtual Result OnElemSegmentCount(uint32_t count) = 0;
  virtual Result BeginElemSegment(uint32_t index, uint32_t table_index) = 0;
  virtual Result BeginElemSegmentInitExpr(uint32_t index) = 0;
  virtual Result EndElemSegmentInitExpr(uint32_t index) = 0;
  virtual Result OnElemSegmentFunctionIndexCount(uint32_t index,
                                                 uint32_t count) = 0;
  virtual Result OnElemSegmentFunctionIndex(uint32_t index,
                                            uint32_t func_index) = 0;
  virtual Result EndElemSegment(uint32_t index) = 0;
  virtual Result EndElemSection() = 0;

  /* Data section */
  virtual Result BeginDataSection(uint32_t size) = 0;
  virtual Result OnDataSegmentCount(uint32_t count) = 0;
  virtual Result BeginDataSegment(uint32_t index, uint32_t memory_index) = 0;
  virtual Result BeginDataSegmentInitExpr(uint32_t index) = 0;
  virtual Result EndDataSegmentInitExpr(uint32_t index) = 0;
  virtual Result OnDataSegmentData(uint32_t index,
                                   const void* data,
                                   uint32_t size) = 0;
  virtual Result EndDataSegment(uint32_t index) = 0;
  virtual Result EndDataSection() = 0;

  /* Names section */
  virtual Result BeginNamesSection(uint32_t size) = 0;
  virtual Result OnFunctionNameSubsection(uint32_t index,
                                          uint32_t name_type,
                                          uint32_t subsection_size) = 0;
  virtual Result OnFunctionNamesCount(uint32_t num_functions) = 0;
  virtual Result OnFunctionName(uint32_t function_index,
                                StringSlice function_name) = 0;
  virtual Result OnLocalNameSubsection(uint32_t index,
                                       uint32_t name_type,
                                       uint32_t subsection_size) = 0;
  virtual Result OnLocalNameFunctionCount(uint32_t num_functions) = 0;
  virtual Result OnLocalNameLocalCount(uint32_t function_index,
                                       uint32_t num_locals) = 0;
  virtual Result OnLocalName(uint32_t function_index,
                             uint32_t local_index,
                             StringSlice local_name) = 0;
  virtual Result EndNamesSection() = 0;

  /* Reloc section */
  virtual Result BeginRelocSection(uint32_t size) = 0;
  virtual Result OnRelocCount(uint32_t count,
                              BinarySection section_code,
                              StringSlice section_name) = 0;
  virtual Result OnReloc(RelocType type,
                         uint32_t offset,
                         uint32_t index,
                         uint32_t addend) = 0;
  virtual Result EndRelocSection() = 0;

  /* InitExpr - used by elem, data and global sections; these functions are
   * only called between calls to Begin*InitExpr and End*InitExpr */
  virtual Result OnInitExprF32ConstExpr(uint32_t index, uint32_t value) = 0;
  virtual Result OnInitExprF64ConstExpr(uint32_t index, uint64_t value) = 0;
  virtual Result OnInitExprGetGlobalExpr(uint32_t index,
                                         uint32_t global_index) = 0;
  virtual Result OnInitExprI32ConstExpr(uint32_t index, uint32_t value) = 0;
  virtual Result OnInitExprI64ConstExpr(uint32_t index, uint64_t value) = 0;

  const State* state = nullptr;
};

Result read_binary(const void* data,
                   size_t size,
                   BinaryReader* reader,
                   const ReadBinaryOptions* options);

size_t read_u32_leb128(const uint8_t* ptr,
                       const uint8_t* end,
                       uint32_t* out_value);

size_t read_i32_leb128(const uint8_t* ptr,
                       const uint8_t* end,
                       uint32_t* out_value);

}  // namespace wabt

#endif /* WABT_BINARY_READER_H_ */
