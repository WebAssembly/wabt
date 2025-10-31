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

#ifndef WABT_BINARY_READER_LOGGING_H_
#define WABT_BINARY_READER_LOGGING_H_

#include "wabt/binary-reader.h"

namespace wabt {

class Stream;

class BinaryReaderLogging : public BinaryReaderDelegate {
 public:
  BinaryReaderLogging(Stream*, BinaryReaderDelegate* forward);

  bool OnError(const Error&) override;
  void OnSetState(const State* s) override;

  Result BeginModule(uint32_t version) override;
  Result EndModule() override;

  Result BeginSection(Index section_index,
                      BinarySection section_type,
                      Offset size) override;

  Result BeginCustomSection(Index section_index,
                            Offset size,
                            std::string_view section_name) override;
  Result EndCustomSection() override;

  Result BeginTypeSection(Offset size) override;
  Result OnTypeCount(Index count) override;
  Result OnFuncType(Index index,
                    Index param_count,
                    Type* param_types,
                    Index result_count,
                    Type* result_types) override;
  Result OnStructType(Index index, Index field_count, TypeMut* fields) override;
  Result OnArrayType(Index index, TypeMut field) override;
  Result EndTypeSection() override;

  Result BeginImportSection(Offset size) override;
  Result OnImportCount(Index count) override;
  Result OnImport(Index index,
                  ExternalKind kind,
                  std::string_view module_name,
                  std::string_view field_name) override;
  Result OnImportFunc(Index import_index,
                      std::string_view module_name,
                      std::string_view field_name,
                      Index func_index,
                      Index sig_index) override;
  Result OnImportTable(Index import_index,
                       std::string_view module_name,
                       std::string_view field_name,
                       Index table_index,
                       Type elem_type,
                       const Limits* elem_limits) override;
  Result OnImportMemory(Index import_index,
                        std::string_view module_name,
                        std::string_view field_name,
                        Index memory_index,
                        const Limits* page_limits,
                        uint32_t page_size) override;
  Result OnImportGlobal(Index import_index,
                        std::string_view module_name,
                        std::string_view field_name,
                        Index global_index,
                        Type type,
                        bool mutable_) override;
  Result OnImportTag(Index import_index,
                     std::string_view module_name,
                     std::string_view field_name,
                     Index tag_index,
                     Index sig_index) override;
  Result EndImportSection() override;

  Result BeginFunctionSection(Offset size) override;
  Result OnFunctionCount(Index count) override;
  Result OnFunction(Index index, Index sig_index) override;
  Result EndFunctionSection() override;

  Result BeginTableSection(Offset size) override;
  Result OnTableCount(Index count) override;
  Result OnTable(Index index,
                 Type elem_type,
                 const Limits* elem_limits) override;
  Result EndTableSection() override;

  Result BeginMemorySection(Offset size) override;
  Result OnMemoryCount(Index count) override;
  Result OnMemory(Index index,
                  const Limits* limits,
                  uint32_t page_size) override;
  Result EndMemorySection() override;

  Result BeginGlobalSection(Offset size) override;
  Result OnGlobalCount(Index count) override;
  Result BeginGlobal(Index index, Type type, bool mutable_) override;
  Result BeginGlobalInitExpr(Index index) override;
  Result EndGlobalInitExpr(Index index) override;
  Result EndGlobal(Index index) override;
  Result EndGlobalSection() override;

  Result BeginExportSection(Offset size) override;
  Result OnExportCount(Index count) override;
  Result OnExport(Index index,
                  ExternalKind kind,
                  Index item_index,
                  std::string_view name) override;
  Result EndExportSection() override;

  Result BeginStartSection(Offset size) override;
  Result OnStartFunction(Index func_index) override;
  Result EndStartSection() override;

  Result BeginCodeSection(Offset size) override;
  Result OnFunctionBodyCount(Index count) override;
  Result BeginFunctionBody(Index index, Offset size) override;
  Result OnLocalDeclCount(Index count) override;
  Result OnLocalDecl(Index decl_index, Index count, Type type) override;
  Result EndLocalDecls() override;

  Result OnOpcode(Opcode opcode) override;
  Result OnOpcodeBare() override;
  Result OnOpcodeIndex(Index value) override;
  Result OnOpcodeIndexIndex(Index value, Index value2) override;
  Result OnOpcodeUint32(uint32_t value) override;
  Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) override;
  Result OnOpcodeUint32Uint32Uint32(uint32_t value,
                                    uint32_t value2,
                                    uint32_t value3) override;
  Result OnOpcodeUint32Uint32Uint32Uint32(uint32_t value,
                                          uint32_t value2,
                                          uint32_t value3,
                                          uint32_t value4) override;
  Result OnOpcodeUint64(uint64_t value) override;
  Result OnOpcodeF32(uint32_t value) override;
  Result OnOpcodeF64(uint64_t value) override;
  Result OnOpcodeV128(v128 value) override;
  Result OnOpcodeBlockSig(Type sig_type) override;
  Result OnOpcodeType(Type type) override;
  Result OnAtomicLoadExpr(Opcode opcode,
                          Index memidx,
                          Address alignment_log2,
                          Address offset) override;
  Result OnAtomicStoreExpr(Opcode opcode,
                           Index memidx,
                           Address alignment_log2,
                           Address offset) override;
  Result OnAtomicRmwExpr(Opcode opcode,
                         Index memidx,
                         Address alignment_log2,
                         Address offset) override;
  Result OnAtomicRmwCmpxchgExpr(Opcode opcode,
                                Index memidx,
                                Address alignment_log2,
                                Address offset) override;
  Result OnBinaryExpr(Opcode opcode) override;
  Result OnBlockExpr(Type sig_type) override;
  Result OnBrExpr(Index depth) override;
  Result OnBrIfExpr(Index depth) override;
  Result OnBrTableExpr(Index num_targets,
                       Index* target_depths,
                       Index default_target_depth) override;
  Result OnCallExpr(Index func_index) override;
  Result OnCatchExpr(Index tag_index) override;
  Result OnCatchAllExpr() override;
  Result OnCallIndirectExpr(Index sig_index, Index table_index) override;
  Result OnCallRefExpr() override;
  Result OnCompareExpr(Opcode opcode) override;
  Result OnConvertExpr(Opcode opcode) override;
  Result OnDelegateExpr(Index depth) override;
  Result OnDropExpr() override;
  Result OnElseExpr() override;
  Result OnEndExpr() override;
  Result OnSkipFunctionBodyExpr(std::vector<uint8_t>& opcode_buffer) override;
  Result OnF32ConstExpr(uint32_t value_bits) override;
  Result OnF64ConstExpr(uint64_t value_bits) override;
  Result OnV128ConstExpr(v128 value_bits) override;
  Result OnGlobalGetExpr(Index global_index) override;
  Result OnGlobalSetExpr(Index global_index) override;
  Result OnI32ConstExpr(uint32_t value) override;
  Result OnI64ConstExpr(uint64_t value) override;
  Result OnIfExpr(Type sig_type) override;
  Result OnLoadExpr(Opcode opcode,
                    Index memidx,
                    Address alignment_log2,
                    Address offset) override;
  Result OnLocalGetExpr(Index local_index) override;
  Result OnLocalSetExpr(Index local_index) override;
  Result OnLocalTeeExpr(Index local_index) override;
  Result OnLoopExpr(Type sig_type) override;
  Result OnMemoryCopyExpr(Index destmemidx, Index srcmemidx) override;
  Result OnDataDropExpr(Index segment_index) override;
  Result OnMemoryFillExpr(Index memidx) override;
  Result OnMemoryGrowExpr(Index memidx) override;
  Result OnMemoryInitExpr(Index segment_index, Index memidx) override;
  Result OnMemorySizeExpr(Index memidx) override;
  Result OnTableCopyExpr(Index dst_index, Index src_index) override;
  Result OnElemDropExpr(Index segment_index) override;
  Result OnTableInitExpr(Index segment_index, Index table_index) override;
  Result OnTableGetExpr(Index table) override;
  Result OnTableSetExpr(Index table) override;
  Result OnTableGrowExpr(Index table) override;
  Result OnTableSizeExpr(Index table) override;
  Result OnTableFillExpr(Index table) override;
  Result OnRefFuncExpr(Index index) override;
  Result OnRefNullExpr(Type type) override;
  Result OnRefIsNullExpr() override;
  Result OnNopExpr() override;
  Result OnRethrowExpr(Index depth) override;
  Result OnReturnCallExpr(Index func_index) override;
  Result OnReturnCallIndirectExpr(Index sig_index, Index table_index) override;
  Result OnReturnExpr() override;
  Result OnSelectExpr(Index result_count, Type* result_types) override;
  Result OnStoreExpr(Opcode opcode,
                     Index memidx,
                     Address alignment_log2,
                     Address offset) override;
  Result OnThrowExpr(Index tag_index) override;
  Result OnThrowRefExpr() override;
  Result OnTryExpr(Type sig_type) override;
  Result OnTryTableExpr(Type sig_type,
                        const CatchClauseVector& catches) override;
  Result OnUnaryExpr(Opcode opcode) override;
  Result OnTernaryExpr(Opcode opcode) override;
  Result OnUnreachableExpr() override;
  Result OnAtomicWaitExpr(Opcode opcode,
                          Index memidx,
                          Address alignment_log2,
                          Address offset) override;
  Result OnAtomicFenceExpr(uint32_t consistency_model) override;
  Result OnAtomicNotifyExpr(Opcode opcode,
                            Index memidx,
                            Address alignment_log2,
                            Address offset) override;
  Result EndFunctionBody(Index index) override;
  Result EndCodeSection() override;
  Result OnSimdLaneOpExpr(Opcode opcode, uint64_t value) override;
  Result OnSimdLoadLaneExpr(Opcode opcode,
                            Index memidx,
                            Address alignment_log2,
                            Address offset,
                            uint64_t value) override;
  Result OnSimdStoreLaneExpr(Opcode opcode,
                             Index memidx,
                             Address alignment_log2,
                             Address offset,
                             uint64_t value) override;
  Result OnSimdShuffleOpExpr(Opcode opcode, v128 value) override;
  Result OnLoadSplatExpr(Opcode opcode,
                         Index memidx,
                         Address alignment_log2,
                         Address offset) override;
  Result OnLoadZeroExpr(Opcode opcode,
                        Index memidx,
                        Address alignment_log2,
                        Address offset) override;

  Result BeginElemSection(Offset size) override;
  Result OnElemSegmentCount(Index count) override;
  Result BeginElemSegment(Index index,
                          Index table_index,
                          uint8_t flags) override;
  Result BeginElemSegmentInitExpr(Index index) override;
  Result EndElemSegmentInitExpr(Index index) override;
  Result OnElemSegmentElemType(Index index, Type elem_type) override;
  Result OnElemSegmentElemExprCount(Index index, Index count) override;
  Result BeginElemExpr(Index elem_index, Index expr_index) override;
  Result EndElemExpr(Index elem_index, Index expr_index) override;
  Result EndElemSegment(Index index) override;
  Result EndElemSection() override;

  Result BeginDataSection(Offset size) override;
  Result OnDataSegmentCount(Index count) override;
  Result BeginDataSegment(Index index,
                          Index memory_index,
                          uint8_t flags) override;
  Result BeginDataSegmentInitExpr(Index index) override;
  Result EndDataSegmentInitExpr(Index index) override;
  Result OnDataSegmentData(Index index,
                           const void* data,
                           Address size) override;
  Result EndDataSegment(Index index) override;
  Result EndDataSection() override;

  Result BeginDataCountSection(Offset size) override;
  Result OnDataCount(Index count) override;
  Result EndDataCountSection() override;

  Result BeginNamesSection(Offset size) override;
  Result OnModuleNameSubsection(Index index,
                                uint32_t name_type,
                                Offset subsection_size) override;
  Result OnModuleName(std::string_view name) override;
  Result OnFunctionNameSubsection(Index index,
                                  uint32_t name_type,
                                  Offset subsection_size) override;
  Result OnFunctionNamesCount(Index num_functions) override;
  Result OnFunctionName(Index function_index,
                        std::string_view function_name) override;
  Result OnLocalNameSubsection(Index index,
                               uint32_t name_type,
                               Offset subsection_size) override;
  Result OnLocalNameFunctionCount(Index num_functions) override;
  Result OnLocalNameLocalCount(Index function_index, Index num_locals) override;
  Result OnLocalName(Index function_index,
                     Index local_index,
                     std::string_view local_name) override;
  Result OnNameSubsection(Index index,
                          NameSectionSubsection subsection_type,
                          Offset subsection_size) override;
  Result OnNameEntry(NameSectionSubsection type,
                     Index index,
                     std::string_view name) override;
  Result OnNameCount(Index num_names) override;
  Result EndNamesSection() override;

  Result BeginRelocSection(Offset size) override;
  Result OnRelocCount(Index count, Index section_index) override;
  Result OnReloc(RelocType type,
                 Offset offset,
                 Index index,
                 uint32_t addend) override;
  Result EndRelocSection() override;

  Result BeginDylinkSection(Offset size) override;
  Result OnDylinkInfo(uint32_t mem_size,
                      uint32_t mem_align,
                      uint32_t table_size,
                      uint32_t table_align) override;
  Result OnDylinkNeededCount(Index count) override;
  Result OnDylinkNeeded(std::string_view needed) override;
  Result OnDylinkImportCount(Index count) override;
  Result OnDylinkExportCount(Index count) override;
  Result OnDylinkImport(std::string_view module,
                        std::string_view name,
                        uint32_t flags) override;
  Result OnDylinkExport(std::string_view name, uint32_t flags) override;
  Result EndDylinkSection() override;

  Result BeginGenericCustomSection(Offset size) override;
  Result OnGenericCustomSection(std::string_view name,
                                const void* data,
                                Offset size) override;
  Result EndGenericCustomSection() override;

  Result BeginTargetFeaturesSection(Offset size) override;
  Result OnFeatureCount(Index count) override;
  Result OnFeature(uint8_t prefix, std::string_view name) override;
  Result EndTargetFeaturesSection() override;

  Result BeginLinkingSection(Offset size) override;
  Result OnSymbolCount(Index count) override;
  Result OnDataSymbol(Index index,
                      uint32_t flags,
                      std::string_view name,
                      Index segment,
                      uint32_t offset,
                      uint32_t size) override;
  Result OnFunctionSymbol(Index index,
                          uint32_t flags,
                          std::string_view name,
                          Index func_index) override;
  Result OnGlobalSymbol(Index index,
                        uint32_t flags,
                        std::string_view name,
                        Index global_index) override;
  Result OnSectionSymbol(Index index,
                         uint32_t flags,
                         Index section_index) override;
  Result OnTagSymbol(Index index,
                     uint32_t flags,
                     std::string_view name,
                     Index tag_index) override;
  Result OnTableSymbol(Index index,
                       uint32_t flags,
                       std::string_view name,
                       Index tag_index) override;
  Result OnSegmentInfoCount(Index count) override;
  Result OnSegmentInfo(Index index,
                       std::string_view name,
                       Address alignment,
                       uint32_t flags) override;
  Result OnInitFunctionCount(Index count) override;
  Result OnInitFunction(uint32_t priority, Index symbol_index) override;
  Result OnComdatCount(Index count) override;
  Result OnComdatBegin(std::string_view name,
                       uint32_t flags,
                       Index count) override;
  Result OnComdatEntry(ComdatType kind, Index index) override;
  Result EndLinkingSection() override;

  Result BeginTagSection(Offset size) override;
  Result OnTagCount(Index count) override;
  Result OnTagType(Index index, Index sig_index) override;
  Result EndTagSection() override;

  /* Code Metadata sections */
  Result BeginCodeMetadataSection(std::string_view name, Offset size) override;
  Result OnCodeMetadataFuncCount(Index count) override;
  Result OnCodeMetadataCount(Index function_index, Index count) override;
  Result OnCodeMetadata(Offset offset, const void* data, Address size) override;
  Result EndCodeMetadataSection() override;

 private:
  void Indent();
  void Dedent();
  void WriteIndent();
  void LogType(Type type);
  void LogTypes(Index type_count, Type* types);
  void LogTypes(TypeVector& types);
  void LogField(TypeMut field);

  Stream* stream_;
  BinaryReaderDelegate* reader_;
  int indent_;
};

}  // namespace wabt

#endif  // WABT_BINARY_READER_LOGGING_H_
