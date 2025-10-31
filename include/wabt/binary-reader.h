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

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "wabt/binary.h"
#include "wabt/common.h"
#include "wabt/error.h"
#include "wabt/feature.h"
#include "wabt/opcode.h"

namespace wabt {

class Stream;

struct ReadBinaryOptions {
  ReadBinaryOptions() = default;
  ReadBinaryOptions(const Features& features,
                    Stream* log_stream,
                    bool read_debug_names,
                    bool stop_on_first_error,
                    bool fail_on_custom_section_error)
      : features(features),
        log_stream(log_stream),
        read_debug_names(read_debug_names),
        stop_on_first_error(stop_on_first_error),
        fail_on_custom_section_error(fail_on_custom_section_error) {}

  Features features;
  Stream* log_stream = nullptr;
  bool read_debug_names = false;
  bool stop_on_first_error = true;
  bool fail_on_custom_section_error = true;
  bool skip_function_bodies = false;
};

// TODO: Move somewhere else?
struct TypeMut {
  Type type;
  bool mutable_;
};
using TypeMutVector = std::vector<TypeMut>;

struct CatchClause {
  CatchKind kind;
  Index tag;
  Index depth;
};
using CatchClauseVector = std::vector<CatchClause>;

class BinaryReaderDelegate {
 public:
  struct State {
    State(const uint8_t* data, Offset size)
        : data(data), size(size), offset(0) {}

    const uint8_t* data;
    Offset size;
    Offset offset;
  };

  virtual ~BinaryReaderDelegate() {}

  virtual bool OnError(const Error&) = 0;
  virtual void OnSetState(const State* s) { state = s; }

  /* Module */
  virtual Result BeginModule(uint32_t version) = 0;
  virtual Result EndModule() = 0;

  virtual Result BeginSection(Index section_index,
                              BinarySection section_type,
                              Offset size) = 0;

  /* Custom section */
  virtual Result BeginCustomSection(Index section_index,
                                    Offset size,
                                    std::string_view section_name) = 0;
  virtual Result EndCustomSection() = 0;

  /* Type section */
  virtual Result BeginTypeSection(Offset size) = 0;
  virtual Result OnTypeCount(Index count) = 0;
  virtual Result OnFuncType(Index index,
                            Index param_count,
                            Type* param_types,
                            Index result_count,
                            Type* result_types) = 0;
  virtual Result OnStructType(Index index,
                              Index field_count,
                              TypeMut* fields) = 0;
  virtual Result OnArrayType(Index index, TypeMut field) = 0;
  virtual Result EndTypeSection() = 0;

  /* Import section */
  virtual Result BeginImportSection(Offset size) = 0;
  virtual Result OnImportCount(Index count) = 0;
  virtual Result OnImport(Index index,
                          ExternalKind kind,
                          std::string_view module_name,
                          std::string_view field_name) = 0;
  virtual Result OnImportFunc(Index import_index,
                              std::string_view module_name,
                              std::string_view field_name,
                              Index func_index,
                              Index sig_index) = 0;
  virtual Result OnImportTable(Index import_index,
                               std::string_view module_name,
                               std::string_view field_name,
                               Index table_index,
                               Type elem_type,
                               const Limits* elem_limits) = 0;
  virtual Result OnImportMemory(Index import_index,
                                std::string_view module_name,
                                std::string_view field_name,
                                Index memory_index,
                                const Limits* page_limits,
                                uint32_t page_size) = 0;
  virtual Result OnImportGlobal(Index import_index,
                                std::string_view module_name,
                                std::string_view field_name,
                                Index global_index,
                                Type type,
                                bool mutable_) = 0;
  virtual Result OnImportTag(Index import_index,
                             std::string_view module_name,
                             std::string_view field_name,
                             Index tag_index,
                             Index sig_index) = 0;
  virtual Result EndImportSection() = 0;

  /* Function section */
  virtual Result BeginFunctionSection(Offset size) = 0;
  virtual Result OnFunctionCount(Index count) = 0;
  virtual Result OnFunction(Index index, Index sig_index) = 0;
  virtual Result EndFunctionSection() = 0;

  /* Table section */
  virtual Result BeginTableSection(Offset size) = 0;
  virtual Result OnTableCount(Index count) = 0;
  virtual Result OnTable(Index index,
                         Type elem_type,
                         const Limits* elem_limits) = 0;
  virtual Result EndTableSection() = 0;

  /* Memory section */
  virtual Result BeginMemorySection(Offset size) = 0;
  virtual Result OnMemoryCount(Index count) = 0;
  virtual Result OnMemory(Index index,
                          const Limits* limits,
                          uint32_t page_size) = 0;
  virtual Result EndMemorySection() = 0;

  /* Global section */
  virtual Result BeginGlobalSection(Offset size) = 0;
  virtual Result OnGlobalCount(Index count) = 0;
  virtual Result BeginGlobal(Index index, Type type, bool mutable_) = 0;
  virtual Result BeginGlobalInitExpr(Index index) = 0;
  virtual Result EndGlobalInitExpr(Index index) = 0;
  virtual Result EndGlobal(Index index) = 0;
  virtual Result EndGlobalSection() = 0;

  /* Exports section */
  virtual Result BeginExportSection(Offset size) = 0;
  virtual Result OnExportCount(Index count) = 0;
  virtual Result OnExport(Index index,
                          ExternalKind kind,
                          Index item_index,
                          std::string_view name) = 0;
  virtual Result EndExportSection() = 0;

  /* Start section */
  virtual Result BeginStartSection(Offset size) = 0;
  virtual Result OnStartFunction(Index func_index) = 0;
  virtual Result EndStartSection() = 0;

  /* Code section */
  virtual Result BeginCodeSection(Offset size) = 0;
  virtual Result OnFunctionBodyCount(Index count) = 0;
  virtual Result BeginFunctionBody(Index index, Offset size) = 0;
  virtual Result OnLocalDeclCount(Index count) = 0;
  virtual Result OnLocalDecl(Index decl_index, Index count, Type type) = 0;
  virtual Result EndLocalDecls() = 0;

  /* Function expressions; called between BeginFunctionBody and
   EndFunctionBody */
  virtual Result OnOpcode(Opcode Opcode) = 0;
  virtual Result OnOpcodeBare() = 0;
  virtual Result OnOpcodeUint32(uint32_t value) = 0;
  virtual Result OnOpcodeIndex(Index value) = 0;
  virtual Result OnOpcodeIndexIndex(Index value, Index value2) = 0;
  virtual Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) = 0;
  virtual Result OnOpcodeUint32Uint32Uint32(uint32_t value,
                                            uint32_t value2,
                                            uint32_t value3) = 0;
  virtual Result OnOpcodeUint32Uint32Uint32Uint32(uint32_t value,
                                                  uint32_t value2,
                                                  uint32_t value3,
                                                  uint32_t value4) = 0;
  virtual Result OnOpcodeUint64(uint64_t value) = 0;
  virtual Result OnOpcodeF32(uint32_t value) = 0;
  virtual Result OnOpcodeF64(uint64_t value) = 0;
  virtual Result OnOpcodeV128(v128 value) = 0;
  virtual Result OnOpcodeBlockSig(Type sig_type) = 0;
  virtual Result OnOpcodeType(Type type) = 0;
  virtual Result OnAtomicLoadExpr(Opcode opcode,
                                  Index memidx,
                                  Address alignment_log2,
                                  Address offset) = 0;
  virtual Result OnAtomicStoreExpr(Opcode opcode,
                                   Index memidx,
                                   Address alignment_log2,
                                   Address offset) = 0;
  virtual Result OnAtomicRmwExpr(Opcode opcode,
                                 Index memidx,
                                 Address alignment_log2,
                                 Address offset) = 0;
  virtual Result OnAtomicRmwCmpxchgExpr(Opcode opcode,
                                        Index memidx,
                                        Address alignment_log2,
                                        Address offset) = 0;
  virtual Result OnAtomicWaitExpr(Opcode opcode,
                                  Index memidx,
                                  Address alignment_log2,
                                  Address offset) = 0;
  virtual Result OnAtomicFenceExpr(uint32_t consistency_model) = 0;
  virtual Result OnAtomicNotifyExpr(Opcode opcode,
                                    Index memidx,
                                    Address alignment_log2,
                                    Address offset) = 0;
  virtual Result OnBinaryExpr(Opcode opcode) = 0;
  virtual Result OnBlockExpr(Type sig_type) = 0;
  virtual Result OnBrExpr(Index depth) = 0;
  virtual Result OnBrIfExpr(Index depth) = 0;
  virtual Result OnBrTableExpr(Index num_targets,
                               Index* target_depths,
                               Index default_target_depth) = 0;
  virtual Result OnCallExpr(Index func_index) = 0;
  virtual Result OnCallIndirectExpr(Index sig_index, Index table_index) = 0;
  virtual Result OnCallRefExpr() = 0;
  virtual Result OnCatchExpr(Index tag_index) = 0;
  virtual Result OnCatchAllExpr() = 0;
  virtual Result OnCompareExpr(Opcode opcode) = 0;
  virtual Result OnConvertExpr(Opcode opcode) = 0;
  virtual Result OnDelegateExpr(Index depth) = 0;
  virtual Result OnDropExpr() = 0;
  virtual Result OnElseExpr() = 0;
  virtual Result OnEndExpr() = 0;
  virtual Result OnSkipFunctionBodyExpr(
      std::vector<uint8_t>& opcode_buffer) = 0;
  virtual Result OnF32ConstExpr(uint32_t value_bits) = 0;
  virtual Result OnF64ConstExpr(uint64_t value_bits) = 0;
  virtual Result OnV128ConstExpr(v128 value_bits) = 0;
  virtual Result OnGlobalGetExpr(Index global_index) = 0;
  virtual Result OnGlobalSetExpr(Index global_index) = 0;
  virtual Result OnI32ConstExpr(uint32_t value) = 0;
  virtual Result OnI64ConstExpr(uint64_t value) = 0;
  virtual Result OnIfExpr(Type sig_type) = 0;
  virtual Result OnLoadExpr(Opcode opcode,
                            Index memidx,
                            Address alignment_log2,
                            Address offset) = 0;
  virtual Result OnLocalGetExpr(Index local_index) = 0;
  virtual Result OnLocalSetExpr(Index local_index) = 0;
  virtual Result OnLocalTeeExpr(Index local_index) = 0;
  virtual Result OnLoopExpr(Type sig_type) = 0;
  virtual Result OnMemoryCopyExpr(Index destmemidx, Index srcmemidx) = 0;
  virtual Result OnDataDropExpr(Index segment_index) = 0;
  virtual Result OnMemoryFillExpr(Index memidx) = 0;
  virtual Result OnMemoryGrowExpr(Index memidx) = 0;
  virtual Result OnMemoryInitExpr(Index segment_index, Index memidx) = 0;
  virtual Result OnMemorySizeExpr(Index memidx) = 0;
  virtual Result OnTableCopyExpr(Index dst_index, Index src_index) = 0;
  virtual Result OnElemDropExpr(Index segment_index) = 0;
  virtual Result OnTableInitExpr(Index segment_index, Index table_index) = 0;
  virtual Result OnTableGetExpr(Index table_index) = 0;
  virtual Result OnTableSetExpr(Index table_index) = 0;
  virtual Result OnTableGrowExpr(Index table_index) = 0;
  virtual Result OnTableSizeExpr(Index table_index) = 0;
  virtual Result OnTableFillExpr(Index table_index) = 0;
  virtual Result OnRefFuncExpr(Index func_index) = 0;
  virtual Result OnRefNullExpr(Type type) = 0;
  virtual Result OnRefIsNullExpr() = 0;
  virtual Result OnNopExpr() = 0;
  virtual Result OnRethrowExpr(Index depth) = 0;
  virtual Result OnReturnExpr() = 0;
  virtual Result OnReturnCallExpr(Index func_index) = 0;
  virtual Result OnReturnCallIndirectExpr(Index sig_index,
                                          Index table_index) = 0;
  virtual Result OnSelectExpr(Index result_count, Type* result_types) = 0;
  virtual Result OnStoreExpr(Opcode opcode,
                             Index memidx,
                             Address alignment_log2,
                             Address offset) = 0;
  virtual Result OnThrowExpr(Index tag_index) = 0;
  virtual Result OnThrowRefExpr() = 0;
  virtual Result OnTryExpr(Type sig_type) = 0;
  virtual Result OnTryTableExpr(Type sig_type,
                                const CatchClauseVector& catches) = 0;

  virtual Result OnUnaryExpr(Opcode opcode) = 0;
  virtual Result OnTernaryExpr(Opcode opcode) = 0;
  virtual Result OnUnreachableExpr() = 0;
  virtual Result EndFunctionBody(Index index) = 0;
  virtual Result EndCodeSection() = 0;

  /* Simd instructions with Lane Imm operand*/
  virtual Result OnSimdLaneOpExpr(Opcode opcode, uint64_t value) = 0;
  virtual Result OnSimdShuffleOpExpr(Opcode opcode, v128 value) = 0;
  virtual Result OnSimdLoadLaneExpr(Opcode opcode,
                                    Index memidx,
                                    Address alignment_log2,
                                    Address offset,
                                    uint64_t value) = 0;
  virtual Result OnSimdStoreLaneExpr(Opcode opcode,
                                     Index memidx,
                                     Address alignment_log2,
                                     Address offset,
                                     uint64_t value) = 0;

  virtual Result OnLoadSplatExpr(Opcode opcode,
                                 Index memidx,
                                 Address alignment_log2,
                                 Address offset) = 0;
  virtual Result OnLoadZeroExpr(Opcode opcode,
                                Index memidx,
                                Address alignment_log2,
                                Address offset) = 0;

  /* Elem section */
  virtual Result BeginElemSection(Offset size) = 0;
  virtual Result OnElemSegmentCount(Index count) = 0;
  virtual Result BeginElemSegment(Index index,
                                  Index table_index,
                                  uint8_t flags) = 0;
  virtual Result BeginElemSegmentInitExpr(Index index) = 0;
  virtual Result EndElemSegmentInitExpr(Index index) = 0;
  virtual Result OnElemSegmentElemType(Index index, Type elem_type) = 0;
  virtual Result OnElemSegmentElemExprCount(Index index, Index count) = 0;
  virtual Result BeginElemExpr(Index elem_index, Index expr_index) = 0;
  virtual Result EndElemExpr(Index elem_index, Index expr_index) = 0;
  virtual Result EndElemSegment(Index index) = 0;
  virtual Result EndElemSection() = 0;

  /* Data section */
  virtual Result BeginDataSection(Offset size) = 0;
  virtual Result OnDataSegmentCount(Index count) = 0;
  virtual Result BeginDataSegment(Index index,
                                  Index memory_index,
                                  uint8_t flags) = 0;
  virtual Result BeginDataSegmentInitExpr(Index index) = 0;
  virtual Result EndDataSegmentInitExpr(Index index) = 0;
  virtual Result OnDataSegmentData(Index index,
                                   const void* data,
                                   Address size) = 0;
  virtual Result EndDataSegment(Index index) = 0;
  virtual Result EndDataSection() = 0;

  /* DataCount section */
  virtual Result BeginDataCountSection(Offset size) = 0;
  virtual Result OnDataCount(Index count) = 0;
  virtual Result EndDataCountSection() = 0;

  /* Names section */
  virtual Result BeginNamesSection(Offset size) = 0;
  virtual Result OnModuleNameSubsection(Index index,
                                        uint32_t name_type,
                                        Offset subsection_size) = 0;
  virtual Result OnModuleName(std::string_view name) = 0;
  virtual Result OnFunctionNameSubsection(Index index,
                                          uint32_t name_type,
                                          Offset subsection_size) = 0;
  virtual Result OnFunctionNamesCount(Index num_functions) = 0;
  virtual Result OnFunctionName(Index function_index,
                                std::string_view function_name) = 0;
  virtual Result OnLocalNameSubsection(Index index,
                                       uint32_t name_type,
                                       Offset subsection_size) = 0;
  virtual Result OnLocalNameFunctionCount(Index num_functions) = 0;
  virtual Result OnLocalNameLocalCount(Index function_index,
                                       Index num_locals) = 0;
  virtual Result OnLocalName(Index function_index,
                             Index local_index,
                             std::string_view local_name) = 0;
  virtual Result OnNameSubsection(Index index,
                                  NameSectionSubsection subsection_type,
                                  Offset subsection_size) = 0;
  virtual Result OnNameCount(Index num_names) = 0;
  virtual Result OnNameEntry(NameSectionSubsection type,
                             Index index,
                             std::string_view name) = 0;
  virtual Result EndNamesSection() = 0;

  /* Reloc section */
  virtual Result BeginRelocSection(Offset size) = 0;
  virtual Result OnRelocCount(Index count, Index section_index) = 0;
  virtual Result OnReloc(RelocType type,
                         Offset offset,
                         Index index,
                         uint32_t addend) = 0;
  virtual Result EndRelocSection() = 0;

  /* Dylink section */
  virtual Result BeginDylinkSection(Offset size) = 0;
  virtual Result OnDylinkInfo(uint32_t mem_size,
                              uint32_t mem_align_log2,
                              uint32_t table_size,
                              uint32_t table_align_log2) = 0;
  virtual Result OnDylinkImportCount(Index count) = 0;
  virtual Result OnDylinkExportCount(Index count) = 0;
  virtual Result OnDylinkImport(std::string_view module,
                                std::string_view name,
                                uint32_t flags) = 0;
  virtual Result OnDylinkExport(std::string_view name, uint32_t flags) = 0;
  virtual Result OnDylinkNeededCount(Index count) = 0;
  virtual Result OnDylinkNeeded(std::string_view so_name) = 0;
  virtual Result EndDylinkSection() = 0;

  /* target_features section */
  virtual Result BeginTargetFeaturesSection(Offset size) = 0;
  virtual Result OnFeatureCount(Index count) = 0;
  virtual Result OnFeature(uint8_t prefix, std::string_view name) = 0;
  virtual Result EndTargetFeaturesSection() = 0;

  /* Generic custom section */
  virtual Result BeginGenericCustomSection(Offset size) = 0;
  virtual Result OnGenericCustomSection(std::string_view name,
                                        const void* data,
                                        Offset size) = 0;
  virtual Result EndGenericCustomSection() = 0;

  /* Linking section */
  virtual Result BeginLinkingSection(Offset size) = 0;
  virtual Result OnSymbolCount(Index count) = 0;
  virtual Result OnDataSymbol(Index index,
                              uint32_t flags,
                              std::string_view name,
                              Index segment,
                              uint32_t offset,
                              uint32_t size) = 0;
  virtual Result OnFunctionSymbol(Index index,
                                  uint32_t flags,
                                  std::string_view name,
                                  Index function_index) = 0;
  virtual Result OnGlobalSymbol(Index index,
                                uint32_t flags,
                                std::string_view name,
                                Index global_index) = 0;
  virtual Result OnSectionSymbol(Index index,
                                 uint32_t flags,
                                 Index section_index) = 0;
  virtual Result OnTagSymbol(Index index,
                             uint32_t flags,
                             std::string_view name,
                             Index tag_index) = 0;
  virtual Result OnTableSymbol(Index index,
                               uint32_t flags,
                               std::string_view name,
                               Index table_index) = 0;
  virtual Result OnSegmentInfoCount(Index count) = 0;
  virtual Result OnSegmentInfo(Index index,
                               std::string_view name,
                               Address alignment_log2,
                               uint32_t flags) = 0;
  virtual Result OnInitFunctionCount(Index count) = 0;
  virtual Result OnInitFunction(uint32_t priority, Index symbol_index) = 0;
  virtual Result OnComdatCount(Index count) = 0;
  virtual Result OnComdatBegin(std::string_view name,
                               uint32_t flags,
                               Index count) = 0;
  virtual Result OnComdatEntry(ComdatType kind, Index index) = 0;
  virtual Result EndLinkingSection() = 0;

  /* Tag section */
  virtual Result BeginTagSection(Offset size) = 0;
  virtual Result OnTagCount(Index count) = 0;
  virtual Result OnTagType(Index index, Index sig_index) = 0;
  virtual Result EndTagSection() = 0;

  /* Code Metadata sections */
  virtual Result BeginCodeMetadataSection(std::string_view name,
                                          Offset size) = 0;
  virtual Result OnCodeMetadataFuncCount(Index count) = 0;
  virtual Result OnCodeMetadataCount(Index function_index, Index count) = 0;
  virtual Result OnCodeMetadata(Offset offset,
                                const void* data,
                                Address size) = 0;
  virtual Result EndCodeMetadataSection() = 0;

  const State* state = nullptr;
};

Result ReadBinary(const void* data,
                  size_t size,
                  BinaryReaderDelegate* reader,
                  const ReadBinaryOptions& options);

Result ExtractFunctionBody(const void* data,
                           size_t size,
                           Location& loc,
                           BinaryReaderDelegate* delegate,
                           const ReadBinaryOptions& options);

size_t ReadU32Leb128(const uint8_t* ptr,
                     const uint8_t* end,
                     uint32_t* out_value);

size_t ReadI32Leb128(const uint8_t* ptr,
                     const uint8_t* end,
                     uint32_t* out_value);

}  // namespace wabt

#endif /* WABT_BINARY_READER_H_ */
