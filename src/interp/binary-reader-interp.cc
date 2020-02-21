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

#include "src/interp/binary-reader-interp.h"

#include <map>
#include <set>

#include "src/binary-reader-nop.h"
#include "src/feature.h"
#include "src/interp/interp.h"
#include "src/stream.h"
#include "src/type-checker.h"

// TODO: Remove wabt:: namespace everywhere.

namespace wabt {
namespace interp {

namespace {

ValueTypes ToInterp(Index count, Type* types) {
  return ValueTypes(&types[0], &types[count]);
}

Mutability ToMutability(bool mut) {
  return mut ? Mutability::Var : Mutability::Const;
}

SegmentMode ToSegmentMode(uint8_t flags) {
  if ((flags & SegDeclared) == SegDeclared) {
    return SegmentMode::Declared;
  } else if ((flags & SegPassive) == SegPassive) {
    return SegmentMode::Passive;
  } else {
    return SegmentMode::Active;
  }
}

struct Label {
  Istream::Offset offset;
  Istream::Offset fixup_offset;
};

struct FixupMap {
  using Offset = Istream::Offset;
  using Fixups = std::vector<Offset>;

  void Clear();
  void Append(Index, Offset);
  void Resolve(Istream&, Index);

  std::map<Index, Fixups> map;
};

class BinaryReaderInterp : public BinaryReaderNop {
 public:
  BinaryReaderInterp(ModuleDesc* module,
                     Errors* errors,
                     const Features& features);

  ValueType GetType(InitExpr);

  // Implement BinaryReader.
  bool OnError(const Error&) override;

  wabt::Result EndModule() override;

  wabt::Result OnTypeCount(Index count) override;
  wabt::Result OnType(Index index,
                      Index param_count,
                      Type* param_types,
                      Index result_count,
                      Type* result_types) override;

  wabt::Result OnImportFunc(Index import_index,
                            string_view module_name,
                            string_view field_name,
                            Index func_index,
                            Index sig_index) override;
  wabt::Result OnImportTable(Index import_index,
                             string_view module_name,
                             string_view field_name,
                             Index table_index,
                             Type elem_type,
                             const Limits* elem_limits) override;
  wabt::Result OnImportMemory(Index import_index,
                              string_view module_name,
                              string_view field_name,
                              Index memory_index,
                              const Limits* page_limits) override;
  wabt::Result OnImportGlobal(Index import_index,
                              string_view module_name,
                              string_view field_name,
                              Index global_index,
                              Type type,
                              bool mutable_) override;

  wabt::Result OnFunctionCount(Index count) override;
  wabt::Result OnFunction(Index index, Index sig_index) override;

  wabt::Result OnTableCount(Index count) override;
  wabt::Result OnTable(Index index,
                       Type elem_type,
                       const Limits* elem_limits) override;

  wabt::Result OnMemoryCount(Index count) override;
  wabt::Result OnMemory(Index index, const Limits* limits) override;

  wabt::Result OnGlobalCount(Index count) override;
  wabt::Result BeginGlobal(Index index, Type type, bool mutable_) override;
  wabt::Result EndGlobalInitExpr(Index index) override;

  wabt::Result OnExport(Index index,
                        ExternalKind kind,
                        Index item_index,
                        string_view name) override;

  wabt::Result OnStartFunction(Index func_index) override;

  wabt::Result BeginFunctionBody(Index index, Offset size) override;
  wabt::Result OnLocalDeclCount(Index count) override;
  wabt::Result OnLocalDecl(Index decl_index, Index count, Type type) override;

  wabt::Result OnOpcode(Opcode Opcode) override;
  wabt::Result OnAtomicLoadExpr(Opcode opcode,
                                uint32_t alignment_log2,
                                Address offset) override;
  wabt::Result OnAtomicStoreExpr(Opcode opcode,
                                 uint32_t alignment_log2,
                                 Address offset) override;
  wabt::Result OnAtomicRmwExpr(Opcode opcode,
                               uint32_t alignment_log2,
                               Address offset) override;
  wabt::Result OnAtomicRmwCmpxchgExpr(Opcode opcode,
                                      uint32_t alignment_log2,
                                      Address offset) override;
  wabt::Result OnAtomicWaitExpr(Opcode opcode,
                                uint32_t alignment_log2,
                                Address offset) override;
  wabt::Result OnAtomicNotifyExpr(Opcode opcode,
                                uint32_t alignment_log2,
                                Address offset) override;
  wabt::Result OnBinaryExpr(wabt::Opcode opcode) override;
  wabt::Result OnBlockExpr(Type sig_type) override;
  wabt::Result OnBrExpr(Index depth) override;
  wabt::Result OnBrIfExpr(Index depth) override;
  wabt::Result OnBrTableExpr(Index num_targets,
                             Index* target_depths,
                             Index default_target_depth) override;
  wabt::Result OnCallExpr(Index func_index) override;
  wabt::Result OnCallIndirectExpr(Index sig_index, Index table_index) override;
  wabt::Result OnReturnCallExpr(Index func_index) override;
  wabt::Result OnReturnCallIndirectExpr(Index sig_index, Index table_index) override;
  wabt::Result OnCompareExpr(wabt::Opcode opcode) override;
  wabt::Result OnConvertExpr(wabt::Opcode opcode) override;
  wabt::Result OnDropExpr() override;
  wabt::Result OnElseExpr() override;
  wabt::Result OnEndExpr() override;
  wabt::Result OnF32ConstExpr(uint32_t value_bits) override;
  wabt::Result OnF64ConstExpr(uint64_t value_bits) override;
  wabt::Result OnV128ConstExpr(v128 value_bits) override;
  wabt::Result OnGlobalGetExpr(Index global_index) override;
  wabt::Result OnGlobalSetExpr(Index global_index) override;
  wabt::Result OnI32ConstExpr(uint32_t value) override;
  wabt::Result OnI64ConstExpr(uint64_t value) override;
  wabt::Result OnIfExpr(Type sig_type) override;
  wabt::Result OnLoadExpr(wabt::Opcode opcode,
                          uint32_t alignment_log2,
                          Address offset) override;
  wabt::Result OnLocalGetExpr(Index local_index) override;
  wabt::Result OnLocalSetExpr(Index local_index) override;
  wabt::Result OnLocalTeeExpr(Index local_index) override;
  wabt::Result OnLoopExpr(Type sig_type) override;
  wabt::Result OnMemoryCopyExpr() override;
  wabt::Result OnDataDropExpr(Index segment_index) override;
  wabt::Result OnMemoryGrowExpr() override;
  wabt::Result OnMemoryFillExpr() override;
  wabt::Result OnMemoryInitExpr(Index segment_index) override;
  wabt::Result OnMemorySizeExpr() override;
  wabt::Result OnRefFuncExpr(Index func_index) override;
  wabt::Result OnRefNullExpr() override;
  wabt::Result OnRefIsNullExpr() override;
  wabt::Result OnNopExpr() override;
  wabt::Result OnReturnExpr() override;
  wabt::Result OnSelectExpr(Type result_type) override;
  wabt::Result OnStoreExpr(wabt::Opcode opcode,
                           uint32_t alignment_log2,
                           Address offset) override;
  wabt::Result OnUnaryExpr(wabt::Opcode opcode) override;
  wabt::Result OnTableCopyExpr(Index dst_index, Index src_index) override;
  wabt::Result OnTableGetExpr(Index table_index) override;
  wabt::Result OnTableSetExpr(Index table_index) override;
  wabt::Result OnTableGrowExpr(Index table_index) override;
  wabt::Result OnTableSizeExpr(Index table_index) override;
  wabt::Result OnTableFillExpr(Index table_index) override;
  wabt::Result OnElemDropExpr(Index segment_index) override;
  wabt::Result OnTableInitExpr(Index segment_index, Index table_index) override;
  wabt::Result OnTernaryExpr(wabt::Opcode opcode) override;
  wabt::Result OnUnreachableExpr() override;
  wabt::Result EndFunctionBody(Index index) override;
  wabt::Result OnSimdLaneOpExpr(wabt::Opcode opcode, uint64_t value) override;
  wabt::Result OnSimdShuffleOpExpr(wabt::Opcode opcode, v128 value) override;
  wabt::Result OnLoadSplatExpr(wabt::Opcode opcode,
                               uint32_t alignment_log2,
                               Address offset) override;

  wabt::Result BeginElemSection(Offset size) override;
  wabt::Result OnElemSegmentCount(Index count) override;
  wabt::Result BeginElemSegment(Index index,
                                Index table_index,
                                uint8_t flags,
                                Type elem_type) override;
  wabt::Result EndElemSegmentInitExpr(Index index) override;
  wabt::Result OnElemSegmentElemExprCount(Index index, Index count) override;
  wabt::Result OnElemSegmentElemExpr_RefNull(Index segment_index) override;
  wabt::Result OnElemSegmentElemExpr_RefFunc(Index segment_index,
                                             Index func_index) override;

  wabt::Result OnDataCount(Index count) override;
  wabt::Result EndDataSegmentInitExpr(Index index) override;
  wabt::Result BeginDataSegment(Index index,
                                Index memory_index,
                                uint8_t flags) override;
  wabt::Result OnDataSegmentData(Index index,
                                 const void* data,
                                 Address size) override;

  wabt::Result OnInitExprF32ConstExpr(Index index, uint32_t value) override;
  wabt::Result OnInitExprF64ConstExpr(Index index, uint64_t value) override;
  wabt::Result OnInitExprV128ConstExpr(Index index, v128 value) override;
  wabt::Result OnInitExprGlobalGetExpr(Index index,
                                       Index global_index) override;
  wabt::Result OnInitExprI32ConstExpr(Index index, uint32_t value) override;
  wabt::Result OnInitExprI64ConstExpr(Index index, uint64_t value) override;
  wabt::Result OnInitExprRefNull(Index index) override;
  wabt::Result OnInitExprRefFunc(Index index, Index func_index) override;

 private:
  Label* GetLabel(Index depth);
  Label* TopLabel();
  void PushLabel(Istream::Offset offset = Istream::kInvalidOffset,
                 Istream::Offset fixup_offset = Istream::kInvalidOffset);
  void PopLabel();

  void PrintError(const char* format, ...);

  wabt::Result GetDropCount(Index keep_count,
                            size_t type_stack_limit,
                            Index* out_drop_count);
  wabt::Result GetBrDropKeepCount(Index depth,
                                  Index* out_drop_count,
                                  Index* out_keep_count);
  wabt::Result GetReturnDropKeepCount(Index* out_drop_count,
                                      Index* out_keep_count);
  wabt::Result GetReturnCallDropKeepCount(const FuncType&,
                                          Index keep_extra,
                                          Index* out_drop_count,
                                          Index* out_keep_count);
  void EmitBr(Index depth, Index drop_count, Index keep_count);
  void FixupTopLabel();
  u32 GetFuncOffset(Index func_index);

  void GetBlockSignature(Type sig_type,
                         TypeVector* out_param_types,
                         TypeVector* out_result_types);
  Index TranslateLocalIndex(Index local_index);
  ValueType GetLocalType(Index local_index);

  wabt::Result CheckDeclaredFunc(Index func_index);
  wabt::Result CheckLocal(Index local_index);
  wabt::Result CheckGlobal(Index global_index);
  wabt::Result CheckDataSegment(Index data_segment_index);
  wabt::Result CheckElemSegment(Index elem_segment_index);
  wabt::Result CheckHasMemory(wabt::Opcode opcode);
  wabt::Result CheckHasTable(wabt::Opcode opcode);
  wabt::Result CheckAlign(uint32_t alignment_log2, Address natural_alignment);
  wabt::Result CheckAtomicAlign(uint32_t alignment_log2,
                                Address natural_alignment);

  Index num_func_imports() const;
  Index num_global_imports() const;

  Features features_;
  Errors* errors_ = nullptr;
  ModuleDesc& module_;
  Istream& istream_;

  TypeChecker typechecker_;

  FuncDesc* func_;
  std::vector<Label> label_stack_;
  ValueTypes param_and_local_types_;
  FixupMap depth_fixups_;
  FixupMap func_fixups_;

  InitExpr init_expr_;
  u32 local_decl_count_;
  u32 local_count_;

  std::vector<FuncType> func_types_;      // Includes imported and defined.
  std::vector<TableType> table_types_;    // Includes imported and defined.
  std::vector<MemoryType> memory_types_;  // Includes imported and defined.
  std::vector<GlobalType> global_types_;  // Includes imported and defined.
  std::vector<EventType> event_types_;    // Includes imported and defined.

  std::set<std::string> export_names_;  // Used to check for duplicates.
  std::vector<bool> declared_funcs_;
  std::vector<Index> init_expr_funcs_;

  static const Index kMemoryIndex0 = 0;
};

void FixupMap::Clear() {
  map.clear();
}

void FixupMap::Append(Index index, Offset offset) {
  map[index].push_back(offset);
}

void FixupMap::Resolve(Istream& istream, Index index) {
  auto iter = map.find(index);
  if (iter == map.end()) {
    return;
  }
  for (Offset offset : iter->second) {
    istream.ResolveFixupU32(offset);
  }
  map.erase(iter);
}

BinaryReaderInterp::BinaryReaderInterp(ModuleDesc* module,
                                       Errors* errors,
                                       const Features& features)
    : features_(features),
      errors_(errors),
      module_(*module),
      istream_(module->istream),
      typechecker_(features) {
  typechecker_.set_error_callback(
      [this](const char* msg) { PrintError("%s", msg); });
}

Label* BinaryReaderInterp::GetLabel(Index depth) {
  assert(depth < label_stack_.size());
  return &label_stack_[label_stack_.size() - depth - 1];
}

Label* BinaryReaderInterp::TopLabel() {
  return GetLabel(0);
}

void WABT_PRINTF_FORMAT(2, 3) BinaryReaderInterp::PrintError(const char* format,
                                                             ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  errors_->emplace_back(ErrorLevel::Error, Location(kInvalidOffset), buffer);
}

void BinaryReaderInterp::GetBlockSignature(Type sig_type,
                                           TypeVector* out_param_types,
                                           TypeVector* out_result_types) {
  if (IsTypeIndex(sig_type)) {
    FuncType& func_type = module_.func_types[GetTypeIndex(sig_type)];
    *out_param_types = func_type.params;
    *out_result_types = func_type.results;
  } else {
    out_param_types->clear();
    *out_result_types = GetInlineTypeVector(sig_type);
  }
}

wabt::Result BinaryReaderInterp::GetDropCount(Index keep_count,
                                              size_t type_stack_limit,
                                              Index* out_drop_count) {
  assert(typechecker_.type_stack_size() >= type_stack_limit);
  Index type_stack_count = typechecker_.type_stack_size() - type_stack_limit;
  // The keep_count may be larger than the type_stack_count if the typechecker
  // is currently unreachable. In that case, it doesn't matter what value we
  // drop, but 0 is a reasonable choice.
  *out_drop_count =
      type_stack_count >= keep_count ? type_stack_count - keep_count : 0;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::GetBrDropKeepCount(Index depth,
                                                    Index* out_drop_count,
                                                    Index* out_keep_count) {
  TypeChecker::Label* label;
  CHECK_RESULT(typechecker_.GetLabel(depth, &label));
  Index keep_count = label->br_types().size();
  CHECK_RESULT(
      GetDropCount(keep_count, label->type_stack_limit, out_drop_count));
  *out_keep_count = keep_count;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::GetReturnDropKeepCount(Index* out_drop_count,
                                                        Index* out_keep_count) {
  CHECK_RESULT(GetBrDropKeepCount(label_stack_.size() - 1, out_drop_count,
                                  out_keep_count));
  *out_drop_count += param_and_local_types_.size();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::GetReturnCallDropKeepCount(
    const FuncType& func_type,
    Index keep_extra,
    Index* out_drop_count,
    Index* out_keep_count) {
  Index keep_count = static_cast<Index>(func_type.params.size()) + keep_extra;
  CHECK_RESULT(GetDropCount(keep_count, 0, out_drop_count));
  *out_drop_count += param_and_local_types_.size();
  *out_keep_count = keep_count;
  return wabt::Result::Ok;
}

void BinaryReaderInterp::EmitBr(Index depth,
                                Index drop_count,
                                Index keep_count) {
  istream_.EmitDropKeep(drop_count, keep_count);
  Istream::Offset offset = GetLabel(depth)->offset;
  istream_.Emit(Opcode::Br);
  if (offset == Istream::kInvalidOffset) {
    // depth_fixups_ stores the depth counting up from zero, where zero is the
    // top-level function scope.
    depth_fixups_.Append(label_stack_.size() - 1 - depth, istream_.end());
  }
  istream_.Emit(offset);
}

void BinaryReaderInterp::FixupTopLabel() {
  depth_fixups_.Resolve(istream_, label_stack_.size() - 1);
}

u32 BinaryReaderInterp::GetFuncOffset(Index func_index) {
  assert(func_index >= num_func_imports());
  FuncDesc& func = module_.funcs[func_index - num_func_imports()];
  if (func.code_offset == Istream::kInvalidOffset) {
    func_fixups_.Append(func_index, istream_.end());
  }
  return func.code_offset;
}

bool BinaryReaderInterp::OnError(const Error& error) {
  errors_->push_back(error);
  return true;
}

wabt::Result BinaryReaderInterp::CheckDeclaredFunc(Index func_index) {
  if (func_index >= declared_funcs_.size() || !declared_funcs_[func_index]) {
    PrintError("function is not declared in any elem sections: %" PRIindex,
               func_index);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndModule() {
  // Verify that any ref.func used in init expressions for globals are
  // mentioned in an elems section.  This can't be done while process the
  // globals because the global section comes before the elem section.
  for (Index func_index : init_expr_funcs_) {
    CHECK_RESULT(CheckDeclaredFunc(func_index));
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTypeCount(Index count) {
  module_.func_types.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnType(Index index,
                                        Index param_count,
                                        Type* param_types,
                                        Index result_count,
                                        Type* result_types) {
  module_.func_types.push_back(FuncType(ToInterp(param_count, param_types),
                                        ToInterp(result_count, result_types)));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckLocal(Index local_index) {
  Index max_local_index = param_and_local_types_.size();
  if (local_index >= max_local_index) {
    PrintError("invalid local_index: %" PRIindex " (max %" PRIindex ")",
               local_index, max_local_index);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckGlobal(Index global_index) {
  Index max_global_index = global_types_.size();
  if (global_index >= max_global_index) {
    PrintError("invalid global_index: %" PRIindex " (max %" PRIindex ")",
               global_index, max_global_index);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckDataSegment(Index data_segment_index) {
  Index max_data_segment_index = module_.datas.capacity();
  if (data_segment_index >= max_data_segment_index) {
    // TODO: change to "data segment index"
    PrintError("invalid data_segment_index: %" PRIindex " (max %" PRIindex ")",
               data_segment_index, max_data_segment_index);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckElemSegment(Index elem_segment_index) {
  Index max_elem_segment_index = module_.elems.size();
  if (elem_segment_index >= max_elem_segment_index) {
    // TODO: change to "elem segment index"
    PrintError("invalid elem_segment_index: %" PRIindex " (max %" PRIindex ")",
               elem_segment_index, max_elem_segment_index);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnImportFunc(Index import_index,
                                              string_view module_name,
                                              string_view field_name,
                                              Index func_index,
                                              Index sig_index) {
  FuncType& func_type = module_.func_types[sig_index];
  module_.imports.push_back(ImportDesc{ImportType(
      module_name.to_string(), field_name.to_string(), func_type.Clone())});
  func_types_.push_back(func_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnImportTable(Index import_index,
                                               string_view module_name,
                                               string_view field_name,
                                               Index table_index,
                                               Type elem_type,
                                               const Limits* elem_limits) {
  TableType table_type{elem_type, *elem_limits};
  module_.imports.push_back(ImportDesc{ImportType(
      module_name.to_string(), field_name.to_string(), table_type.Clone())});
  table_types_.push_back(table_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnImportMemory(Index import_index,
                                                string_view module_name,
                                                string_view field_name,
                                                Index memory_index,
                                                const Limits* page_limits) {
  MemoryType memory_type{*page_limits};
  module_.imports.push_back(ImportDesc{ImportType(
      module_name.to_string(), field_name.to_string(), memory_type.Clone())});
  memory_types_.push_back(memory_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnImportGlobal(Index import_index,
                                                string_view module_name,
                                                string_view field_name,
                                                Index global_index,
                                                Type type,
                                                bool mutable_) {
  GlobalType global_type{type, ToMutability(mutable_)};
  module_.imports.push_back(ImportDesc{ImportType(
      module_name.to_string(), field_name.to_string(), global_type.Clone())});
  global_types_.push_back(global_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnFunctionCount(Index count) {
  module_.funcs.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnFunction(Index index, Index sig_index) {
  FuncType& func_type = module_.func_types[sig_index];
  module_.funcs.push_back(FuncDesc{func_type, {}, 0});
  func_types_.push_back(func_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableCount(Index count) {
  module_.tables.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTable(Index index,
                                         Type elem_type,
                                         const Limits* elem_limits) {
  TableType table_type{elem_type, *elem_limits};
  module_.tables.push_back(TableDesc{table_type});
  table_types_.push_back(table_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryCount(Index count) {
  module_.memories.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemory(Index index, const Limits* limits) {
  MemoryType memory_type{*limits};
  module_.memories.push_back(MemoryDesc{memory_type});
  memory_types_.push_back(memory_type);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnGlobalCount(Index count) {
  module_.globals.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginGlobal(Index index,
                                             Type type,
                                             bool mutable_) {
  GlobalType global_type{type, ToMutability(mutable_)};
  module_.globals.push_back(GlobalDesc{global_type, InitExpr{}});
  global_types_.push_back(global_type);
  init_expr_.kind = InitExprKind::None;
  return wabt::Result::Ok;
}

ValueType BinaryReaderInterp::GetType(InitExpr init) {
  switch (init_expr_.kind) {
    case InitExprKind::None:       return ValueType::Void;
    case InitExprKind::I32:        return ValueType::I32;
    case InitExprKind::I64:        return ValueType::I64;
    case InitExprKind::F32:        return ValueType::F32;
    case InitExprKind::F64:        return ValueType::F64;
    case InitExprKind::V128:       return ValueType::V128;
    case InitExprKind::GlobalGet:  return global_types_[init.index_].type;
    case InitExprKind::RefNull:    return ValueType::Nullref;
    case InitExprKind::RefFunc:    return ValueType::Funcref;
    default: WABT_UNREACHABLE;
  }
}

wabt::Result BinaryReaderInterp::EndGlobalInitExpr(Index index) {
  GlobalDesc& global = module_.globals.back();
  ValueType type = GetType(global.init);
  if (!TypesMatch(global.type.type, type)) {
    PrintError("type mismatch in global, expected %s but got %s.",
               GetName(global.type.type), GetName(type));
    return wabt::Result::Error;
  }

  global.init = init_expr_;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprF32ConstExpr(Index index,
                                                        uint32_t value_bits) {
  init_expr_.kind = InitExprKind::F32;
  init_expr_.f32_ = Bitcast<f32>(value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprF64ConstExpr(Index index,
                                                        uint64_t value_bits) {
  init_expr_.kind = InitExprKind::F64;
  init_expr_.f64_ = Bitcast<f64>(value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprV128ConstExpr(Index index,
                                                         v128 value_bits) {
  init_expr_.kind = InitExprKind::V128;
  init_expr_.v128_ = Bitcast<v128>(value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprGlobalGetExpr(Index index,
                                                         Index global_index) {
  if (global_index >= num_global_imports()) {
    PrintError("initializer expression can only reference an imported global");
    return wabt::Result::Error;
  }
  if (global_types_[global_index].mut == Mutability::Var) {
    PrintError("initializer expression cannot reference a mutable global");
    return wabt::Result::Error;
  }

  init_expr_.kind = InitExprKind::GlobalGet;
  init_expr_.index_ = global_index;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprI32ConstExpr(Index index,
                                                        uint32_t value) {
  init_expr_.kind = InitExprKind::I32;
  init_expr_.i32_ = value;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprI64ConstExpr(Index index,
                                                        uint64_t value) {
  init_expr_.kind = InitExprKind::I64;
  init_expr_.i64_ = value;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprRefNull(Index index) {
  init_expr_.kind = InitExprKind::RefNull;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnInitExprRefFunc(Index index,
                                                   Index func_index) {
  init_expr_.kind = InitExprKind::RefFunc;
  init_expr_.index_ = func_index;
  init_expr_funcs_.push_back(func_index);
  return wabt::Result::Ok;
}

Result BinaryReaderInterp::OnExport(Index index,
                                    ExternalKind kind,
                                    Index item_index,
                                    string_view name) {
  auto name_str = name.to_string();
  if (export_names_.find(name_str) != export_names_.end()) {
    PrintError("duplicate export \"" PRIstringview "\"",
               WABT_PRINTF_STRING_VIEW_ARG(name));
    return Result::Error;
  }
  export_names_.insert(name_str);

  std::unique_ptr<ExternType> type;
  switch (kind) {
    case ExternalKind::Func:   type = func_types_[item_index].Clone(); break;
    case ExternalKind::Table:  type = table_types_[item_index].Clone(); break;
    case ExternalKind::Memory: type = memory_types_[item_index].Clone(); break;
    case ExternalKind::Global: type = global_types_[item_index].Clone(); break;
    case ExternalKind::Event:  type = event_types_[item_index].Clone(); break;
  }
  module_.exports.push_back(
      ExportDesc{ExportType(name_str, std::move(type)), item_index});
  return Result::Ok;
}

wabt::Result BinaryReaderInterp::OnStartFunction(Index func_index) {
  FuncType& func_type = func_types_[func_index];
  if (func_type.params.size() != 0) {
    PrintError("start function must be nullary");
    return wabt::Result::Error;
  }
  if (func_type.results.size() != 0) {
    PrintError("start function must not return anything");
    return wabt::Result::Error;
  }
  module_.starts.push_back(StartDesc{func_index});
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginElemSection(Offset size) {
  // Delay resizing `declared_funcs_` until we we know the total range of
  // function indexes (not until after imports sections is read) and that
  // an elem section exists (therefore the possiblity of declared functions).
  Index max_func_index = func_types_.size();
  declared_funcs_.resize(max_func_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentCount(Index count) {
  module_.elems.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginElemSegment(Index index,
                                                  Index table_index,
                                                  uint8_t flags,
                                                  Type elem_type) {
  ElemDesc desc;
  desc.type = elem_type;
  desc.mode = ToSegmentMode(flags);
  desc.table_index = table_index;
  module_.elems.push_back(desc);
  init_expr_.kind = InitExprKind::None;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndElemSegmentInitExpr(Index index) {
  ValueType type = GetType(init_expr_);
  if (type != ValueType::I32) {
    PrintError(
        "type mismatch in elem segment initializer expression, expected i32 "
        "but got %s",
        GetName(type));
    return wabt::Result::Error;
  }

  ElemDesc& elem = module_.elems.back();
  elem.offset = init_expr_;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentElemExprCount(Index index,
                                                            Index count) {
  ElemDesc& elem = module_.elems.back();
  elem.elements.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentElemExpr_RefNull(
    Index segment_index) {
  ElemDesc& elem = module_.elems.back();
  elem.elements.push_back(ElemExpr{ElemKind::RefNull, 0});
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentElemExpr_RefFunc(
    Index segment_index,
    Index func_index) {
  Index max_func_index = func_types_.size();
  if (func_index >= max_func_index) {
    PrintError("invalid func_index: %" PRIindex " (max %" PRIindex ")",
               func_index, max_func_index);
    return wabt::Result::Error;
  }
  ElemDesc& elem = module_.elems.back();
  elem.elements.push_back(ElemExpr{ElemKind::RefFunc, func_index});
  declared_funcs_[func_index] = true;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDataCount(Index count) {
  module_.datas.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndDataSegmentInitExpr(Index index) {
  ValueType type = GetType(init_expr_);
  if (type != ValueType::I32) {
    PrintError(
        "type mismatch in data segment initializer expression, expected i32 "
        "but got %s",
        GetName(type));
    return wabt::Result::Error;
  }

  DataDesc& data = module_.datas.back();
  data.offset = init_expr_;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginDataSegment(Index index,
                                                  Index memory_index,
                                                  uint8_t flags) {
  DataDesc desc;
  desc.mode = ToSegmentMode(flags);
  desc.memory_index = memory_index;
  module_.datas.push_back(desc);
  init_expr_.kind = InitExprKind::None;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDataSegmentData(Index index,
                                                   const void* src_data,
                                                   Address size) {
  DataDesc& dst_data = module_.datas.back();
  if (size > 0) {
    dst_data.data.resize(size);
    memcpy(dst_data.data.data(), src_data, size);
  }
  return wabt::Result::Ok;
}

void BinaryReaderInterp::PushLabel(Istream::Offset offset,
                                   Istream::Offset fixup_offset) {
  label_stack_.push_back(Label{offset, fixup_offset});
}

void BinaryReaderInterp::PopLabel() {
  label_stack_.pop_back();
}

wabt::Result BinaryReaderInterp::BeginFunctionBody(Index index, Offset size) {
  Index defined_index = index - num_func_imports();
  func_ = &module_.funcs[defined_index];
  func_->code_offset = istream_.end();

  depth_fixups_.Clear();
  label_stack_.clear();

  func_fixups_.Resolve(istream_, defined_index);

  param_and_local_types_ = func_->type.params;

  CHECK_RESULT(typechecker_.BeginFunction(func_->type.results));

  // Push implicit func label (equivalent to return).
  PushLabel();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndFunctionBody(Index index) {
  FixupTopLabel();
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(typechecker_.EndFunction());
  istream_.EmitDropKeep(drop_count, keep_count);
  istream_.Emit(Opcode::Return);
  PopLabel();
  func_ = nullptr;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalDeclCount(Index count) {
  local_decl_count_ = count;
  local_count_ = 0;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalDecl(Index decl_index,
                                             Index count,
                                             Type type) {
  local_count_ += count;
  func_->locals.push_back(LocalDesc{type, count, local_count_});

  for (Index i = 0; i < count; ++i) {
    param_and_local_types_.push_back(type);
  }

  if (decl_index == local_decl_count_ - 1) {
    istream_.Emit(Opcode::InterpAlloca, local_count_);
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckHasMemory(wabt::Opcode opcode) {
  if (memory_types_.empty()) {
    PrintError("%s requires an imported or defined memory.", opcode.GetName());
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckHasTable(wabt::Opcode opcode) {
  if (table_types_.empty()) {
    PrintError("found %s operator, but no table", opcode.GetName());
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckAlign(uint32_t alignment_log2,
                                            Address natural_alignment) {
  if (alignment_log2 >= 32 || (1U << alignment_log2) > natural_alignment) {
    PrintError("alignment must not be larger than natural alignment (%u)",
               natural_alignment);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::CheckAtomicAlign(uint32_t alignment_log2,
                                                  Address natural_alignment) {
  if (alignment_log2 >= 32 || (1U << alignment_log2) != natural_alignment) {
    PrintError("alignment must be equal to natural alignment (%u)",
               natural_alignment);
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

Index BinaryReaderInterp::num_func_imports() const {
  return func_types_.size() - module_.funcs.size();
}

Index BinaryReaderInterp::num_global_imports() const {
  return global_types_.size() - module_.globals.size();
}

wabt::Result BinaryReaderInterp::OnOpcode(Opcode opcode) {
  if (func_ == nullptr || label_stack_.empty()) {
    PrintError("Unexpected instruction after end of function");
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnUnaryExpr(Opcode opcode) {
  CHECK_RESULT(typechecker_.OnUnary(opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTernaryExpr(Opcode opcode) {
  CHECK_RESULT(typechecker_.OnTernary(opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSimdLaneOpExpr(Opcode opcode,
                                                  uint64_t value) {
  CHECK_RESULT(typechecker_.OnSimdLaneOp(opcode, value));
  istream_.Emit(opcode, static_cast<u8>(value));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSimdShuffleOpExpr(Opcode opcode,
                                                     v128 value) {
  CHECK_RESULT(typechecker_.OnSimdShuffleOp(opcode, value));
  istream_.Emit(opcode, value);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLoadSplatExpr(Opcode opcode,
                                                 uint32_t alignment_log2,
                                                 Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnLoad(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicLoadExpr(Opcode opcode,
                                                  uint32_t alignment_log2,
                                                  Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicLoad(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicStoreExpr(Opcode opcode,
                                                   uint32_t alignment_log2,
                                                   Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicStore(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicRmwExpr(Opcode opcode,
                                                 uint32_t alignment_log2,
                                                 Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicRmw(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicRmwCmpxchgExpr(Opcode opcode,
                                                        uint32_t alignment_log2,
                                                        Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicRmwCmpxchg(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBinaryExpr(Opcode opcode) {
  CHECK_RESULT(typechecker_.OnBinary(opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBlockExpr(Type sig_type) {
  TypeVector param_types, result_types;
  GetBlockSignature(sig_type, &param_types, &result_types);
  CHECK_RESULT(typechecker_.OnBlock(param_types, result_types));
  PushLabel();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLoopExpr(Type sig_type) {
  TypeVector param_types, result_types;
  GetBlockSignature(sig_type, &param_types, &result_types);
  CHECK_RESULT(typechecker_.OnLoop(param_types, result_types));
  PushLabel(istream_.end());
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnIfExpr(Type sig_type) {
  TypeVector param_types, result_types;
  GetBlockSignature(sig_type, &param_types, &result_types);
  CHECK_RESULT(typechecker_.OnIf(param_types, result_types));
  istream_.Emit(Opcode::InterpBrUnless);
  auto fixup = istream_.EmitFixupU32();
  PushLabel(Istream::kInvalidOffset, fixup);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElseExpr() {
  CHECK_RESULT(typechecker_.OnElse());
  Label* label = TopLabel();
  Istream::Offset fixup_cond_offset = label->fixup_offset;
  istream_.Emit(Opcode::Br);
  label->fixup_offset = istream_.EmitFixupU32();
  istream_.ResolveFixupU32(fixup_cond_offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnEndExpr() {
  TypeChecker::Label* label;
  CHECK_RESULT(typechecker_.GetLabel(0, &label));
  LabelType label_type = label->label_type;
  CHECK_RESULT(typechecker_.OnEnd());
  if (label_type == LabelType::If || label_type == LabelType::Else) {
    istream_.ResolveFixupU32(TopLabel()->fixup_offset);
  }
  FixupTopLabel();
  PopLabel();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBrExpr(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  CHECK_RESULT(typechecker_.OnBr(depth));
  EmitBr(depth, drop_count, keep_count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBrIfExpr(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(typechecker_.OnBrIf(depth));
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  // Flip the br_if so if <cond> is true it can drop values from the stack.
  istream_.Emit(Opcode::InterpBrUnless);
  auto fixup = istream_.EmitFixupU32();
  EmitBr(depth, drop_count, keep_count);
  istream_.ResolveFixupU32(fixup);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBrTableExpr(Index num_targets,
                                               Index* target_depths,
                                               Index default_target_depth) {
  CHECK_RESULT(typechecker_.BeginBrTable());
  Index drop_count, keep_count;
  istream_.Emit(Opcode::BrTable, num_targets);

  for (Index i = 0; i < num_targets; ++i) {
    Index depth = target_depths[i];
    CHECK_RESULT(typechecker_.OnBrTableTarget(depth));
    CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
    // Emit DropKeep directly (instead of using EmitDropKeep) so the
    // instruction has a fixed size.
    istream_.Emit(Opcode::InterpDropKeep, drop_count, keep_count);
    EmitBr(depth, 0, 0);
  }
  CHECK_RESULT(typechecker_.OnBrTableTarget(default_target_depth));
  CHECK_RESULT(
      GetBrDropKeepCount(default_target_depth, &drop_count, &keep_count));
  // The default case doesn't need a fixed size, since it is never jumped over.
  istream_.EmitDropKeep(drop_count, keep_count);
  EmitBr(default_target_depth, 0, 0);

  CHECK_RESULT(typechecker_.EndBrTable());
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnCallExpr(Index func_index) {
  FuncType& func_type = func_types_[func_index];
  CHECK_RESULT(typechecker_.OnCall(func_type.params, func_type.results));

  if (func_index >= num_func_imports()) {
    istream_.Emit(Opcode::Call, func_index);
  } else {
    // TODO: rename CallImport
    istream_.Emit(Opcode::InterpCallHost, func_index);
  }

  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnCallIndirectExpr(Index sig_index,
                                                    Index table_index) {
  CHECK_RESULT(CheckHasTable(Opcode::CallIndirect));
  FuncType& func_type = module_.func_types[sig_index];
  CHECK_RESULT(
      typechecker_.OnCallIndirect(func_type.params, func_type.results));

  istream_.Emit(Opcode::CallIndirect, table_index, sig_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnReturnCallExpr(Index func_index) {
  FuncType& func_type = func_types_[func_index];

  Index drop_count, keep_count;
  CHECK_RESULT(
      GetReturnCallDropKeepCount(func_type, 0, &drop_count, &keep_count));
  // The typechecker must be run after we get the drop/keep counts, since it
  // will change the type stack.
  CHECK_RESULT(typechecker_.OnReturnCall(func_type.params, func_type.results));
  istream_.EmitDropKeep(drop_count, keep_count);

  if (func_index >= num_func_imports()) {
    istream_.Emit(Opcode::Br, GetFuncOffset(func_index));
  } else {
    // TODO: rename CallImport
    istream_.Emit(Opcode::InterpCallHost, func_index);
    istream_.Emit(Opcode::Return);
  }

  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnReturnCallIndirectExpr(Index sig_index,
                                                          Index table_index) {
  CHECK_RESULT(CheckHasTable(Opcode::ReturnCallIndirect));
  FuncType& func_type = module_.func_types[sig_index];

  Index drop_count, keep_count;
  // +1 to include the index of the function.
  CHECK_RESULT(
      GetReturnCallDropKeepCount(func_type, +1, &drop_count, &keep_count));
  // The typechecker must be run after we get the drop/keep counts, since it
  // changes the type stack.
  CHECK_RESULT(
      typechecker_.OnReturnCallIndirect(func_type.params, func_type.results));
  istream_.EmitDropKeep(drop_count, keep_count);
  istream_.Emit(Opcode::ReturnCallIndirect, table_index, sig_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnCompareExpr(Opcode opcode) {
  return OnBinaryExpr(opcode);
}

wabt::Result BinaryReaderInterp::OnConvertExpr(Opcode opcode) {
  return OnUnaryExpr(opcode);
}

wabt::Result BinaryReaderInterp::OnDropExpr() {
  CHECK_RESULT(typechecker_.OnDrop());
  istream_.Emit(Opcode::Drop);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnI32ConstExpr(uint32_t value) {
  CHECK_RESULT(typechecker_.OnConst(Type::I32));
  istream_.Emit(Opcode::I32Const, value);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnI64ConstExpr(uint64_t value) {
  CHECK_RESULT(typechecker_.OnConst(Type::I64));
  istream_.Emit(Opcode::I64Const, value);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnF32ConstExpr(uint32_t value_bits) {
  CHECK_RESULT(typechecker_.OnConst(Type::F32));
  istream_.Emit(Opcode::F32Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnF64ConstExpr(uint64_t value_bits) {
  CHECK_RESULT(typechecker_.OnConst(Type::F64));
  istream_.Emit(Opcode::F64Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnV128ConstExpr(v128 value_bits) {
  CHECK_RESULT(typechecker_.OnConst(Type::V128));
  istream_.Emit(Opcode::V128Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnGlobalGetExpr(Index global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  GlobalType& global_type = global_types_[global_index];
  CHECK_RESULT(typechecker_.OnGlobalGet(global_type.type));
  istream_.Emit(Opcode::GlobalGet, global_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnGlobalSetExpr(Index global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  GlobalType& global_type = global_types_[global_index];
  if (global_type.mut == Mutability::Const) {
    PrintError("can't global.set on immutable global at index %" PRIindex ".",
               global_index);
    return wabt::Result::Error;
  }
  CHECK_RESULT(typechecker_.OnGlobalSet(global_type.type));
  istream_.Emit(Opcode::GlobalSet, global_index);
  return wabt::Result::Ok;
}

ValueType BinaryReaderInterp::GetLocalType(Index local_index) {
  return param_and_local_types_[local_index];
}

Index BinaryReaderInterp::TranslateLocalIndex(Index local_index) {
  return typechecker_.type_stack_size() + param_and_local_types_.size() -
         local_index;
}

wabt::Result BinaryReaderInterp::OnLocalGetExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  // Get the translated index before calling typechecker_.OnLocalGet because it
  // will update the type stack size. We need the index to be relative to the
  // old stack size.
  Index translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(typechecker_.OnLocalGet(GetLocalType(local_index)));
  istream_.Emit(Opcode::LocalGet, translated_local_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalSetExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  // See comment in OnLocalGetExpr above.
  Index translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(typechecker_.OnLocalSet(GetLocalType(local_index)));
  istream_.Emit(Opcode::LocalSet, translated_local_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalTeeExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  CHECK_RESULT(typechecker_.OnLocalTee(GetLocalType(local_index)));
  istream_.Emit(Opcode::LocalTee, TranslateLocalIndex(local_index));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLoadExpr(Opcode opcode,
                                            uint32_t alignment_log2,
                                            Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnLoad(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnStoreExpr(Opcode opcode,
                                             uint32_t alignment_log2,
                                             Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnStore(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryGrowExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::MemoryGrow));
  CHECK_RESULT(typechecker_.OnMemoryGrow());
  istream_.Emit(Opcode::MemoryGrow, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemorySizeExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::MemorySize));
  CHECK_RESULT(typechecker_.OnMemorySize());
  istream_.Emit(Opcode::MemorySize, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableGrowExpr(Index table_index) {
  TableType& table_type = table_types_[table_index];
  CHECK_RESULT(typechecker_.OnTableGrow(table_type.element));
  istream_.Emit(Opcode::TableGrow, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableSizeExpr(Index table_index) {
  CHECK_RESULT(typechecker_.OnTableSize());
  istream_.Emit(Opcode::TableSize, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableFillExpr(Index table_index) {
  TableType& table_type = table_types_[table_index];
  CHECK_RESULT(typechecker_.OnTableFill(table_type.element));
  istream_.Emit(Opcode::TableFill, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefFuncExpr(Index func_index) {
  CHECK_RESULT(CheckDeclaredFunc(func_index));
  CHECK_RESULT(typechecker_.OnRefFuncExpr(func_index));
  istream_.Emit(Opcode::RefFunc, func_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefNullExpr() {
  CHECK_RESULT(typechecker_.OnRefNullExpr());
  istream_.Emit(Opcode::RefNull);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefIsNullExpr() {
  CHECK_RESULT(typechecker_.OnRefIsNullExpr());
  istream_.Emit(Opcode::RefIsNull);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnNopExpr() {
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnReturnExpr() {
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(typechecker_.OnReturn());
  istream_.EmitDropKeep(drop_count, keep_count);
  istream_.Emit(Opcode::Return);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSelectExpr(Type result_type) {
  CHECK_RESULT(typechecker_.OnSelect(result_type));
  istream_.Emit(Opcode::Select);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnUnreachableExpr() {
  CHECK_RESULT(typechecker_.OnUnreachable());
  istream_.Emit(Opcode::Unreachable);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicWaitExpr(Opcode opcode,
                                                  uint32_t alignment_log2,
                                                  Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicWait(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicNotifyExpr(Opcode opcode,
                                                    uint32_t alignment_log2,
                                                    Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAtomicAlign(alignment_log2, opcode.GetMemorySize()));
  CHECK_RESULT(typechecker_.OnAtomicNotify(opcode));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryCopyExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::MemoryCopy));
  CHECK_RESULT(typechecker_.OnMemoryCopy());
  istream_.Emit(Opcode::MemoryCopy, kMemoryIndex0, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDataDropExpr(Index segment_index) {
  CHECK_RESULT(CheckDataSegment(segment_index));
  CHECK_RESULT(typechecker_.OnDataDrop(segment_index));
  istream_.Emit(Opcode::DataDrop, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryFillExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::MemoryFill));
  CHECK_RESULT(typechecker_.OnMemoryFill());
  istream_.Emit(Opcode::MemoryFill, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryInitExpr(Index segment_index) {
  CHECK_RESULT(CheckHasMemory(Opcode::MemoryInit));
  CHECK_RESULT(CheckDataSegment(segment_index));
  CHECK_RESULT(typechecker_.OnMemoryInit(segment_index));
  istream_.Emit(Opcode::MemoryInit, kMemoryIndex0, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableGetExpr(Index table_index) {
  TableType& table_type = table_types_[table_index];
  CHECK_RESULT(typechecker_.OnTableGet(table_type.element));
  istream_.Emit(Opcode::TableGet, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableSetExpr(Index table_index) {
  TableType& table_type = table_types_[table_index];
  CHECK_RESULT(typechecker_.OnTableSet(table_type.element));
  istream_.Emit(Opcode::TableSet, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableCopyExpr(Index dst_index,
                                                 Index src_index) {
  CHECK_RESULT(CheckHasTable(Opcode::TableCopy));
  CHECK_RESULT(typechecker_.OnTableCopy());
  istream_.Emit(Opcode::TableCopy, dst_index, src_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemDropExpr(Index segment_index) {
  CHECK_RESULT(CheckElemSegment(segment_index));
  CHECK_RESULT(typechecker_.OnElemDrop(segment_index));
  istream_.Emit(Opcode::ElemDrop, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableInitExpr(Index segment_index,
                                                 Index table_index) {
  CHECK_RESULT(CheckHasTable(Opcode::TableInit));
  CHECK_RESULT(CheckElemSegment(segment_index));
  CHECK_RESULT(typechecker_.OnTableInit(table_index, segment_index));
  istream_.Emit(Opcode::TableInit, table_index, segment_index);
  return wabt::Result::Ok;
}

}  // namespace

wabt::Result ReadBinaryInterp(const void* data,
                              size_t size,
                              const ReadBinaryOptions& options,
                              Errors* errors,
                              ModuleDesc* out_module) {
  BinaryReaderInterp reader(out_module, errors, options.features);
  return ReadBinary(data, size, &reader, options);
}

}  // namespace interp
}  // namespace wabt
