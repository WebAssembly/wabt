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

#include "binary-reader.h"

namespace wabt {

class Stream;

class BinaryReaderLogging : public BinaryReader {
 public:
  BinaryReaderLogging(Stream*, BinaryReader* forward);

  virtual bool OnError(const char* message);
  virtual void OnSetState(const State* s);

  virtual Result BeginModule(uint32_t version);
  virtual Result EndModule();

  virtual Result BeginSection(BinarySection section_type, uint32_t size);

  virtual Result BeginCustomSection(uint32_t size, StringSlice section_name);
  virtual Result EndCustomSection();

  virtual Result BeginTypeSection(uint32_t size);
  virtual Result OnTypeCount(uint32_t count);
  virtual Result OnType(uint32_t index,
                        uint32_t param_count,
                        Type* param_types,
                        uint32_t result_count,
                        Type* result_types);
  virtual Result EndTypeSection();

  virtual Result BeginImportSection(uint32_t size);
  virtual Result OnImportCount(uint32_t count);
  virtual Result OnImport(uint32_t index,
                          StringSlice module_name,
                          StringSlice field_name);
  virtual Result OnImportFunc(uint32_t import_index,
                              StringSlice module_name,
                              StringSlice field_name,
                              uint32_t func_index,
                              uint32_t sig_index);
  virtual Result OnImportTable(uint32_t import_index,
                               StringSlice module_name,
                               StringSlice field_name,
                               uint32_t table_index,
                               Type elem_type,
                               const Limits* elem_limits);
  virtual Result OnImportMemory(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t memory_index,
                                const Limits* page_limits);
  virtual Result OnImportGlobal(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t global_index,
                                Type type,
                                bool mutable_);
  virtual Result EndImportSection();

  virtual Result BeginFunctionSection(uint32_t size);
  virtual Result OnFunctionCount(uint32_t count);
  virtual Result OnFunction(uint32_t index, uint32_t sig_index);
  virtual Result EndFunctionSection();

  virtual Result BeginTableSection(uint32_t size);
  virtual Result OnTableCount(uint32_t count);
  virtual Result OnTable(uint32_t index,
                         Type elem_type,
                         const Limits* elem_limits);
  virtual Result EndTableSection();

  virtual Result BeginMemorySection(uint32_t size);
  virtual Result OnMemoryCount(uint32_t count);
  virtual Result OnMemory(uint32_t index, const Limits* limits);
  virtual Result EndMemorySection();

  virtual Result BeginGlobalSection(uint32_t size);
  virtual Result OnGlobalCount(uint32_t count);
  virtual Result BeginGlobal(uint32_t index, Type type, bool mutable_);
  virtual Result BeginGlobalInitExpr(uint32_t index);
  virtual Result EndGlobalInitExpr(uint32_t index);
  virtual Result EndGlobal(uint32_t index);
  virtual Result EndGlobalSection();

  virtual Result BeginExportSection(uint32_t size);
  virtual Result OnExportCount(uint32_t count);
  virtual Result OnExport(uint32_t index,
                          ExternalKind kind,
                          uint32_t item_index,
                          StringSlice name);
  virtual Result EndExportSection();

  virtual Result BeginStartSection(uint32_t size);
  virtual Result OnStartFunction(uint32_t func_index);
  virtual Result EndStartSection();

  virtual Result BeginCodeSection(uint32_t size);
  virtual Result OnFunctionBodyCount(uint32_t count);
  virtual Result BeginFunctionBody(uint32_t index);
  virtual Result OnLocalDeclCount(uint32_t count);
  virtual Result OnLocalDecl(uint32_t decl_index, uint32_t count, Type type);

  virtual Result OnOpcode(Opcode opcode);
  virtual Result OnOpcodeBare();
  virtual Result OnOpcodeUint32(uint32_t value);
  virtual Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2);
  virtual Result OnOpcodeUint64(uint64_t value);
  virtual Result OnOpcodeF32(uint32_t value);
  virtual Result OnOpcodeF64(uint64_t value);
  virtual Result OnOpcodeBlockSig(uint32_t num_types, Type* sig_types);
  virtual Result OnBinaryExpr(Opcode opcode);
  virtual Result OnBlockExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnBrExpr(uint32_t depth);
  virtual Result OnBrIfExpr(uint32_t depth);
  virtual Result OnBrTableExpr(uint32_t num_targets,
                               uint32_t* target_depths,
                               uint32_t default_target_depth);
  virtual Result OnCallExpr(uint32_t func_index);
  virtual Result OnCallIndirectExpr(uint32_t sig_index);
  virtual Result OnCompareExpr(Opcode opcode);
  virtual Result OnConvertExpr(Opcode opcode);
  virtual Result OnCurrentMemoryExpr();
  virtual Result OnDropExpr();
  virtual Result OnElseExpr();
  virtual Result OnEndExpr();
  virtual Result OnEndFunc();
  virtual Result OnF32ConstExpr(uint32_t value_bits);
  virtual Result OnF64ConstExpr(uint64_t value_bits);
  virtual Result OnGetGlobalExpr(uint32_t global_index);
  virtual Result OnGetLocalExpr(uint32_t local_index);
  virtual Result OnGrowMemoryExpr();
  virtual Result OnI32ConstExpr(uint32_t value);
  virtual Result OnI64ConstExpr(uint64_t value);
  virtual Result OnIfExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnLoadExpr(Opcode opcode,
                            uint32_t alignment_log2,
                            uint32_t offset);
  virtual Result OnLoopExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnNopExpr();
  virtual Result OnReturnExpr();
  virtual Result OnSelectExpr();
  virtual Result OnSetGlobalExpr(uint32_t global_index);
  virtual Result OnSetLocalExpr(uint32_t local_index);
  virtual Result OnStoreExpr(Opcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset);
  virtual Result OnTeeLocalExpr(uint32_t local_index);
  virtual Result OnUnaryExpr(Opcode opcode);
  virtual Result OnUnreachableExpr();
  virtual Result EndFunctionBody(uint32_t index);
  virtual Result EndCodeSection();

  virtual Result BeginElemSection(uint32_t size);
  virtual Result OnElemSegmentCount(uint32_t count);
  virtual Result BeginElemSegment(uint32_t index, uint32_t table_index);
  virtual Result BeginElemSegmentInitExpr(uint32_t index);
  virtual Result EndElemSegmentInitExpr(uint32_t index);
  virtual Result OnElemSegmentFunctionIndexCount(uint32_t index,
                                                 uint32_t count);
  virtual Result OnElemSegmentFunctionIndex(uint32_t index,
                                            uint32_t func_index);
  virtual Result EndElemSegment(uint32_t index);
  virtual Result EndElemSection();

  virtual Result BeginDataSection(uint32_t size);
  virtual Result OnDataSegmentCount(uint32_t count);
  virtual Result BeginDataSegment(uint32_t index, uint32_t memory_index);
  virtual Result BeginDataSegmentInitExpr(uint32_t index);
  virtual Result EndDataSegmentInitExpr(uint32_t index);
  virtual Result OnDataSegmentData(uint32_t index,
                                   const void* data,
                                   uint32_t size);
  virtual Result EndDataSegment(uint32_t index);
  virtual Result EndDataSection();

  virtual Result BeginNamesSection(uint32_t size);
  virtual Result OnFunctionNameSubsection(uint32_t index,
                                          uint32_t name_type,
                                          uint32_t subsection_size);
  virtual Result OnFunctionNamesCount(uint32_t num_functions);
  virtual Result OnFunctionName(uint32_t function_index,
                                StringSlice function_name);
  virtual Result OnLocalNameSubsection(uint32_t index,
                                       uint32_t name_type,
                                       uint32_t subsection_size);
  virtual Result OnLocalNameFunctionCount(uint32_t num_functions);
  virtual Result OnLocalNameLocalCount(uint32_t function_index,
                                       uint32_t num_locals);
  virtual Result OnLocalName(uint32_t function_index,
                             uint32_t local_index,
                             StringSlice local_name);
  virtual Result EndNamesSection();

  virtual Result BeginRelocSection(uint32_t size);
  virtual Result OnRelocCount(uint32_t count,
                              BinarySection section_code,
                              StringSlice section_name);
  virtual Result OnReloc(RelocType type,
                         uint32_t offset,
                         uint32_t index,
                         int32_t addend);
  virtual Result EndRelocSection();

  virtual Result OnInitExprF32ConstExpr(uint32_t index, uint32_t value);
  virtual Result OnInitExprF64ConstExpr(uint32_t index, uint64_t value);
  virtual Result OnInitExprGetGlobalExpr(uint32_t index, uint32_t global_index);
  virtual Result OnInitExprI32ConstExpr(uint32_t index, uint32_t value);
  virtual Result OnInitExprI64ConstExpr(uint32_t index, uint64_t value);

 private:
  void Indent();
  void Dedent();
  void WriteIndent();
  void LogTypes(uint32_t type_count, Type* types);

  Stream* stream;
  BinaryReader* reader;
  int indent;
};

}  // namespace wabt

#endif  // WABT_BINARY_READER_LOGGING_H_
