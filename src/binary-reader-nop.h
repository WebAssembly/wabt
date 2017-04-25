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

#ifndef WABT_BINARY_READER_NOP_H_
#define WABT_BINARY_READER_NOP_H_

#include "binary-reader.h"

namespace wabt {

class BinaryReaderNop : public BinaryReader {
 public:
  virtual bool OnError(const char* message) { return false; }

  /* Module */
  virtual Result BeginModule(uint32_t version) { return Result::Ok; }
  virtual Result EndModule() { return Result::Ok; }

  virtual Result BeginSection(BinarySection section_type, uint32_t size) {
    return Result::Ok;
  }

  /* Custom section */
  virtual Result BeginCustomSection(uint32_t size, StringSlice section_name) {
    return Result::Ok;
  }
  virtual Result EndCustomSection() { return Result::Ok; }

  /* Type section */
  virtual Result BeginTypeSection(uint32_t size) { return Result::Ok; }
  virtual Result OnTypeCount(uint32_t count) { return Result::Ok; }
  virtual Result OnType(uint32_t index,
                        uint32_t param_count,
                        Type* param_types,
                        uint32_t result_count,
                        Type* result_types) {
    return Result::Ok;
  }
  virtual Result EndTypeSection() { return Result::Ok; }

  /* Import section */
  virtual Result BeginImportSection(uint32_t size) { return Result::Ok; }
  virtual Result OnImportCount(uint32_t count) { return Result::Ok; }
  virtual Result OnImport(uint32_t index,
                          StringSlice module_name,
                          StringSlice field_name) {
    return Result::Ok;
  }
  virtual Result OnImportFunc(uint32_t import_index,
                              StringSlice module_name,
                              StringSlice field_name,
                              uint32_t func_index,
                              uint32_t sig_index) {
    return Result::Ok;
  }
  virtual Result OnImportTable(uint32_t import_index,
                               StringSlice module_name,
                               StringSlice field_name,
                               uint32_t table_index,
                               Type elem_type,
                               const Limits* elem_limits) {
    return Result::Ok;
  }
  virtual Result OnImportMemory(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t memory_index,
                                const Limits* page_limits) {
    return Result::Ok;
  }
  virtual Result OnImportGlobal(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t global_index,
                                Type type,
                                bool mutable_) {
    return Result::Ok;
  }
  virtual Result EndImportSection() { return Result::Ok; }

  /* Function section */
  virtual Result BeginFunctionSection(uint32_t size) { return Result::Ok; }
  virtual Result OnFunctionCount(uint32_t count) { return Result::Ok; }
  virtual Result OnFunction(uint32_t index, uint32_t sig_index) {
    return Result::Ok;
  }
  virtual Result EndFunctionSection() { return Result::Ok; }

  /* Table section */
  virtual Result BeginTableSection(uint32_t size) { return Result::Ok; }
  virtual Result OnTableCount(uint32_t count) { return Result::Ok; }
  virtual Result OnTable(uint32_t index,
                         Type elem_type,
                         const Limits* elem_limits) {
    return Result::Ok;
  }
  virtual Result EndTableSection() { return Result::Ok; }

  /* Memory section */
  virtual Result BeginMemorySection(uint32_t size) { return Result::Ok; }
  virtual Result OnMemoryCount(uint32_t count) { return Result::Ok; }
  virtual Result OnMemory(uint32_t index, const Limits* limits) {
    return Result::Ok;
  }
  virtual Result EndMemorySection() { return Result::Ok; }

  /* Global section */
  virtual Result BeginGlobalSection(uint32_t size) { return Result::Ok; }
  virtual Result OnGlobalCount(uint32_t count) { return Result::Ok; }
  virtual Result BeginGlobal(uint32_t index, Type type, bool mutable_) {
    return Result::Ok;
  }
  virtual Result BeginGlobalInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result EndGlobalInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result EndGlobal(uint32_t index) { return Result::Ok; }
  virtual Result EndGlobalSection() { return Result::Ok; }

  /* Exports section */
  virtual Result BeginExportSection(uint32_t size) { return Result::Ok; }
  virtual Result OnExportCount(uint32_t count) { return Result::Ok; }
  virtual Result OnExport(uint32_t index,
                          ExternalKind kind,
                          uint32_t item_index,
                          StringSlice name) {
    return Result::Ok;
  }
  virtual Result EndExportSection() { return Result::Ok; }

  /* Start section */
  virtual Result BeginStartSection(uint32_t size) { return Result::Ok; }
  virtual Result OnStartFunction(uint32_t func_index) { return Result::Ok; }
  virtual Result EndStartSection() { return Result::Ok; }

  /* Code section */
  virtual Result BeginCodeSection(uint32_t size) { return Result::Ok; }
  virtual Result OnFunctionBodyCount(uint32_t count) { return Result::Ok; }
  virtual Result BeginFunctionBody(uint32_t index) { return Result::Ok; }
  virtual Result OnLocalDeclCount(uint32_t count) { return Result::Ok; }
  virtual Result OnLocalDecl(uint32_t decl_index, uint32_t count, Type type) {
    return Result::Ok;
  }

  /* Function expressions; called between BeginFunctionBody and
   EndFunctionBody */
  virtual Result OnOpcode(Opcode Opcode) { return Result::Ok; }
  virtual Result OnOpcodeBare() { return Result::Ok; }
  virtual Result OnOpcodeUint32(uint32_t value) { return Result::Ok; }
  virtual Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) {
    return Result::Ok;
  }
  virtual Result OnOpcodeUint64(uint64_t value) { return Result::Ok; }
  virtual Result OnOpcodeF32(uint32_t value) { return Result::Ok; }
  virtual Result OnOpcodeF64(uint64_t value) { return Result::Ok; }
  virtual Result OnOpcodeBlockSig(uint32_t num_types, Type* sig_types) {
    return Result::Ok;
  }
  virtual Result OnBinaryExpr(Opcode opcode) { return Result::Ok; }
  virtual Result OnBlockExpr(uint32_t num_types, Type* sig_types) {
    return Result::Ok;
  }
  virtual Result OnBrExpr(uint32_t depth) { return Result::Ok; }
  virtual Result OnBrIfExpr(uint32_t depth) { return Result::Ok; }
  virtual Result OnBrTableExpr(uint32_t num_targets,
                               uint32_t* target_depths,
                               uint32_t default_target_depth) {
    return Result::Ok;
  }
  virtual Result OnCallExpr(uint32_t func_index) { return Result::Ok; }
  virtual Result OnCallIndirectExpr(uint32_t sig_index) { return Result::Ok; }
  virtual Result OnCompareExpr(Opcode opcode) { return Result::Ok; }
  virtual Result OnConvertExpr(Opcode opcode) { return Result::Ok; }
  virtual Result OnCurrentMemoryExpr() { return Result::Ok; }
  virtual Result OnDropExpr() { return Result::Ok; }
  virtual Result OnElseExpr() { return Result::Ok; }
  virtual Result OnEndExpr() { return Result::Ok; }
  virtual Result OnEndFunc() { return Result::Ok; }
  virtual Result OnF32ConstExpr(uint32_t value_bits) { return Result::Ok; }
  virtual Result OnF64ConstExpr(uint64_t value_bits) { return Result::Ok; }
  virtual Result OnGetGlobalExpr(uint32_t global_index) { return Result::Ok; }
  virtual Result OnGetLocalExpr(uint32_t local_index) { return Result::Ok; }
  virtual Result OnGrowMemoryExpr() { return Result::Ok; }
  virtual Result OnI32ConstExpr(uint32_t value) { return Result::Ok; }
  virtual Result OnI64ConstExpr(uint64_t value) { return Result::Ok; }
  virtual Result OnIfExpr(uint32_t num_types, Type* sig_types) {
    return Result::Ok;
  }
  virtual Result OnLoadExpr(Opcode opcode,
                            uint32_t alignment_log2,
                            uint32_t offset) {
    return Result::Ok;
  }
  virtual Result OnLoopExpr(uint32_t num_types, Type* sig_types) {
    return Result::Ok;
  }
  virtual Result OnNopExpr() { return Result::Ok; }
  virtual Result OnReturnExpr() { return Result::Ok; }
  virtual Result OnSelectExpr() { return Result::Ok; }
  virtual Result OnSetGlobalExpr(uint32_t global_index) { return Result::Ok; }
  virtual Result OnSetLocalExpr(uint32_t local_index) { return Result::Ok; }
  virtual Result OnStoreExpr(Opcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset) {
    return Result::Ok;
  }
  virtual Result OnTeeLocalExpr(uint32_t local_index) { return Result::Ok; }
  virtual Result OnUnaryExpr(Opcode opcode) { return Result::Ok; }
  virtual Result OnUnreachableExpr() { return Result::Ok; }
  virtual Result EndFunctionBody(uint32_t index) { return Result::Ok; }
  virtual Result EndCodeSection() { return Result::Ok; }

  /* Elem section */
  virtual Result BeginElemSection(uint32_t size) { return Result::Ok; }
  virtual Result OnElemSegmentCount(uint32_t count) { return Result::Ok; }
  virtual Result BeginElemSegment(uint32_t index, uint32_t table_index) {
    return Result::Ok;
  }
  virtual Result BeginElemSegmentInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result EndElemSegmentInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result OnElemSegmentFunctionIndexCount(uint32_t index,
                                                 uint32_t count) {
    return Result::Ok;
  }
  virtual Result OnElemSegmentFunctionIndex(uint32_t index,
                                            uint32_t func_index) {
    return Result::Ok;
  }
  virtual Result EndElemSegment(uint32_t index) { return Result::Ok; }
  virtual Result EndElemSection() { return Result::Ok; }

  /* Data section */
  virtual Result BeginDataSection(uint32_t size) { return Result::Ok; }
  virtual Result OnDataSegmentCount(uint32_t count) { return Result::Ok; }
  virtual Result BeginDataSegment(uint32_t index, uint32_t memory_index) {
    return Result::Ok;
  }
  virtual Result BeginDataSegmentInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result EndDataSegmentInitExpr(uint32_t index) { return Result::Ok; }
  virtual Result OnDataSegmentData(uint32_t index,
                                   const void* data,
                                   uint32_t size) {
    return Result::Ok;
  }
  virtual Result EndDataSegment(uint32_t index) { return Result::Ok; }
  virtual Result EndDataSection() { return Result::Ok; }

  /* Names section */
  virtual Result BeginNamesSection(uint32_t size) { return Result::Ok; }
  virtual Result OnFunctionNameSubsection(uint32_t index,
                                          uint32_t name_type,
                                          uint32_t subsection_size) {
    return Result::Ok;
  }
  virtual Result OnFunctionNamesCount(uint32_t num_functions) {
    return Result::Ok;
  }
  virtual Result OnFunctionName(uint32_t function_index,
                                StringSlice function_name) {
    return Result::Ok;
  }
  virtual Result OnLocalNameSubsection(uint32_t index,
                                       uint32_t name_type,
                                       uint32_t subsection_size) {
    return Result::Ok;
  }
  virtual Result OnLocalNameFunctionCount(uint32_t num_functions) {
    return Result::Ok;
  }
  virtual Result OnLocalNameLocalCount(uint32_t function_index,
                                       uint32_t num_locals) {
    return Result::Ok;
  }
  virtual Result OnLocalName(uint32_t function_index,
                             uint32_t local_index,
                             StringSlice local_name) {
    return Result::Ok;
  }
  virtual Result EndNamesSection() { return Result::Ok; }

  /* Reloc section */
  virtual Result BeginRelocSection(uint32_t size) { return Result::Ok; }
  virtual Result OnRelocCount(uint32_t count,
                              BinarySection section_code,
                              StringSlice section_name) {
    return Result::Ok;
  }
  virtual Result OnReloc(RelocType type,
                         uint32_t offset,
                         uint32_t index,
                         uint32_t addend) {
    return Result::Ok;
  }
  virtual Result EndRelocSection() { return Result::Ok; }

  /* InitExpr - used by elem, data and global sections; these functions are
   * only called between calls to Begin*InitExpr and End*InitExpr */
  virtual Result OnInitExprF32ConstExpr(uint32_t index, uint32_t value) {
    return Result::Ok;
  }
  virtual Result OnInitExprF64ConstExpr(uint32_t index, uint64_t value) {
    return Result::Ok;
  }
  virtual Result OnInitExprGetGlobalExpr(uint32_t index,
                                         uint32_t global_index) {
    return Result::Ok;
  }
  virtual Result OnInitExprI32ConstExpr(uint32_t index, uint32_t value) {
    return Result::Ok;
  }
  virtual Result OnInitExprI64ConstExpr(uint32_t index, uint64_t value) {
    return Result::Ok;
  }
};

}  // namespace wabt

#endif /* WABT_BINARY_READER_H_ */
