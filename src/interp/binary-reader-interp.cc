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
#include "src/shared-validator.h"
#include "src/stream.h"

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

  wabt::Result OnElemSegmentCount(Index count) override;
  wabt::Result BeginElemSegment(Index index,
                                Index table_index,
                                uint8_t flags) override;
  wabt::Result EndElemSegmentInitExpr(Index index) override;
  wabt::Result OnElemSegmentElemType(Index index, Type elem_type) override;
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

  Index TranslateLocalIndex(Index local_index);

  Index num_func_imports() const;

  Errors* errors_ = nullptr;
  ModuleDesc& module_;
  Istream& istream_;

  SharedValidator validator_;

  FuncDesc* func_;
  std::vector<Label> label_stack_;
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

  static const Index kMemoryIndex0 = 0;

  // TODO: Use this in all locations below, for now. In the future we'll want
  // to use the real locations.
  static const Location loc;
};

// static
const Location BinaryReaderInterp::loc{kInvalidOffset};

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
    : errors_(errors),
      module_(*module),
      istream_(module->istream),
      validator_(errors, ValidateOptions(features)) {}

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

wabt::Result BinaryReaderInterp::GetDropCount(Index keep_count,
                                              size_t type_stack_limit,
                                              Index* out_drop_count) {
  assert(validator_.type_stack_size() >= type_stack_limit);
  Index type_stack_count = validator_.type_stack_size() - type_stack_limit;
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
  SharedValidator::Label* label;
  CHECK_RESULT(validator_.GetLabel(depth, &label));
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
  *out_drop_count += validator_.GetLocalCount();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::GetReturnCallDropKeepCount(
    const FuncType& func_type,
    Index keep_extra,
    Index* out_drop_count,
    Index* out_keep_count) {
  Index keep_count = static_cast<Index>(func_type.params.size()) + keep_extra;
  CHECK_RESULT(GetDropCount(keep_count, 0, out_drop_count));
  *out_drop_count += validator_.GetLocalCount();
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

wabt::Result BinaryReaderInterp::EndModule() {
  CHECK_RESULT(validator_.EndModule());
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
  CHECK_RESULT(validator_.OnType(loc, param_count, param_types, result_count,
                                 result_types));
  module_.func_types.push_back(FuncType(ToInterp(param_count, param_types),
                                        ToInterp(result_count, result_types)));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnImportFunc(Index import_index,
                                              string_view module_name,
                                              string_view field_name,
                                              Index func_index,
                                              Index sig_index) {
  CHECK_RESULT(validator_.OnFunction(loc, Var(sig_index)));
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
  CHECK_RESULT(validator_.OnTable(loc, elem_type, *elem_limits));
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
  CHECK_RESULT(validator_.OnMemory(loc, *page_limits));
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
  CHECK_RESULT(validator_.OnGlobalImport(loc, type, mutable_));
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
  CHECK_RESULT(validator_.OnFunction(loc, Var(sig_index)));
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
  CHECK_RESULT(validator_.OnTable(loc, elem_type, *elem_limits));
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
  CHECK_RESULT(validator_.OnMemory(loc, *limits));
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
  CHECK_RESULT(validator_.OnGlobal(loc, type, mutable_));
  GlobalType global_type{type, ToMutability(mutable_)};
  module_.globals.push_back(GlobalDesc{global_type, InitExpr{}});
  global_types_.push_back(global_type);
  init_expr_.kind = InitExprKind::None;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndGlobalInitExpr(Index index) {
  switch (init_expr_.kind) {
    case InitExprKind::I32:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Const(loc, ValueType::I32));
      break;

    case InitExprKind::I64:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Const(loc, ValueType::I64));
      break;

    case InitExprKind::F32:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Const(loc, ValueType::F32));
      break;

    case InitExprKind::F64:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Const(loc, ValueType::F64));
      break;

    case InitExprKind::V128:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Const(loc, ValueType::V128));
      break;

    case InitExprKind::GlobalGet:
      CHECK_RESULT(
          validator_.OnGlobalInitExpr_GlobalGet(loc, Var(init_expr_.index_)));
      break;

    case InitExprKind::RefNull:
      CHECK_RESULT(validator_.OnGlobalInitExpr_RefNull(loc));
      break;

    case InitExprKind::RefFunc:
      CHECK_RESULT(
          validator_.OnGlobalInitExpr_RefFunc(loc, Var(init_expr_.index_)));
      break;

    default:
      CHECK_RESULT(validator_.OnGlobalInitExpr_Other(loc));
      break;
  }

  GlobalDesc& global = module_.globals.back();
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
  return wabt::Result::Ok;
}

Result BinaryReaderInterp::OnExport(Index index,
                                    ExternalKind kind,
                                    Index item_index,
                                    string_view name) {
  CHECK_RESULT(validator_.OnExport(loc, kind, Var(item_index), name));

  std::unique_ptr<ExternType> type;
  switch (kind) {
    case ExternalKind::Func:   type = func_types_[item_index].Clone(); break;
    case ExternalKind::Table:  type = table_types_[item_index].Clone(); break;
    case ExternalKind::Memory: type = memory_types_[item_index].Clone(); break;
    case ExternalKind::Global: type = global_types_[item_index].Clone(); break;
    case ExternalKind::Event:  type = event_types_[item_index].Clone(); break;
  }
  module_.exports.push_back(
      ExportDesc{ExportType(name.to_string(), std::move(type)), item_index});
  return Result::Ok;
}

wabt::Result BinaryReaderInterp::OnStartFunction(Index func_index) {
  CHECK_RESULT(validator_.OnStart(loc, Var(func_index)));
  module_.starts.push_back(StartDesc{func_index});
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentCount(Index count) {
  module_.elems.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginElemSegment(Index index,
                                                  Index table_index,
                                                  uint8_t flags) {
  auto mode = ToSegmentMode(flags);
  CHECK_RESULT(validator_.OnElemSegment(loc, Var(table_index), mode));

  ElemDesc desc;
  desc.type = ValueType::Void;  // Initialized later in OnElemSegmentElemType.
  desc.mode = mode;
  desc.table_index = table_index;
  module_.elems.push_back(desc);
  init_expr_.kind = InitExprKind::None;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndElemSegmentInitExpr(Index index) {
  switch (init_expr_.kind) {
    case InitExprKind::I32:
      CHECK_RESULT(validator_.OnElemSegmentInitExpr_Const(loc, ValueType::I32));
      break;

    case InitExprKind::GlobalGet:
      CHECK_RESULT(validator_.OnElemSegmentInitExpr_GlobalGet(
          loc, Var(init_expr_.index_)));
      break;

    default:
      CHECK_RESULT(validator_.OnElemSegmentInitExpr_Other(loc));
      break;
  }

  ElemDesc& elem = module_.elems.back();
  elem.offset = init_expr_;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentElemType(Index index,
                                                       Type elem_type) {
  validator_.OnElemSegmentElemType(elem_type);
  ElemDesc& elem = module_.elems.back();
  elem.type = elem_type;
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
  CHECK_RESULT(validator_.OnElemSegmentElemExpr_RefNull(loc));
  ElemDesc& elem = module_.elems.back();
  elem.elements.push_back(ElemExpr{ElemKind::RefNull, 0});
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemSegmentElemExpr_RefFunc(
    Index segment_index,
    Index func_index) {
  CHECK_RESULT(validator_.OnElemSegmentElemExpr_RefFunc(loc, Var(func_index)));
  ElemDesc& elem = module_.elems.back();
  elem.elements.push_back(ElemExpr{ElemKind::RefFunc, func_index});
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDataCount(Index count) {
  validator_.OnDataCount(count);
  module_.datas.reserve(count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndDataSegmentInitExpr(Index index) {
  switch (init_expr_.kind) {
    case InitExprKind::I32:
      CHECK_RESULT(validator_.OnDataSegmentInitExpr_Const(loc, ValueType::I32));
      break;

    case InitExprKind::GlobalGet:
      CHECK_RESULT(validator_.OnDataSegmentInitExpr_GlobalGet(
          loc, Var(init_expr_.index_)));
      break;

    default:
      CHECK_RESULT(validator_.OnDataSegmentInitExpr_Other(loc));
      break;
  }

  DataDesc& data = module_.datas.back();
  data.offset = init_expr_;
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::BeginDataSegment(Index index,
                                                  Index memory_index,
                                                  uint8_t flags) {
  auto mode = ToSegmentMode(flags);
  CHECK_RESULT(validator_.OnDataSegment(loc, Var(memory_index), mode));

  DataDesc desc;
  desc.mode = mode;
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

  CHECK_RESULT(validator_.BeginFunctionBody(loc, index));

  // Push implicit func label (equivalent to return).
  PushLabel();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::EndFunctionBody(Index index) {
  FixupTopLabel();
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(validator_.EndFunctionBody(loc));
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
  CHECK_RESULT(validator_.OnLocalDecl(loc, count, type));

  local_count_ += count;
  func_->locals.push_back(LocalDesc{type, count, local_count_});

  if (decl_index == local_decl_count_ - 1) {
    istream_.Emit(Opcode::InterpAlloca, local_count_);
  }
  return wabt::Result::Ok;
}

Index BinaryReaderInterp::num_func_imports() const {
  return func_types_.size() - module_.funcs.size();
}

wabt::Result BinaryReaderInterp::OnOpcode(Opcode opcode) {
  if (func_ == nullptr || label_stack_.empty()) {
    PrintError("Unexpected instruction after end of function");
    return wabt::Result::Error;
  }
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnUnaryExpr(Opcode opcode) {
  CHECK_RESULT(validator_.OnUnary(loc, opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTernaryExpr(Opcode opcode) {
  CHECK_RESULT(validator_.OnTernary(loc, opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSimdLaneOpExpr(Opcode opcode,
                                                  uint64_t value) {
  CHECK_RESULT(validator_.OnSimdLaneOp(loc, opcode, value));
  istream_.Emit(opcode, static_cast<u8>(value));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSimdShuffleOpExpr(Opcode opcode,
                                                     v128 value) {
  CHECK_RESULT(validator_.OnSimdShuffleOp(loc, opcode, value));
  istream_.Emit(opcode, value);
  return wabt::Result::Ok;
}

uint32_t GetAlignment(uint32_t alignment_log2) {
  return alignment_log2 < 32 ? 1 << alignment_log2 : ~0u;
}

wabt::Result BinaryReaderInterp::OnLoadSplatExpr(Opcode opcode,
                                                 uint32_t align_log2,
                                                 Address offset) {
  CHECK_RESULT(validator_.OnLoadSplat(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicLoadExpr(Opcode opcode,
                                                  uint32_t align_log2,
                                                  Address offset) {
  CHECK_RESULT(validator_.OnAtomicLoad(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicStoreExpr(Opcode opcode,
                                                   uint32_t align_log2,
                                                   Address offset) {
  CHECK_RESULT(validator_.OnAtomicStore(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicRmwExpr(Opcode opcode,
                                                 uint32_t align_log2,
                                                 Address offset) {
  CHECK_RESULT(validator_.OnAtomicRmw(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicRmwCmpxchgExpr(Opcode opcode,
                                                        uint32_t align_log2,
                                                        Address offset) {
  CHECK_RESULT(
      validator_.OnAtomicRmwCmpxchg(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBinaryExpr(Opcode opcode) {
  CHECK_RESULT(validator_.OnBinary(loc, opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBlockExpr(Type sig_type) {
  CHECK_RESULT(validator_.OnBlock(loc, sig_type));
  PushLabel();
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLoopExpr(Type sig_type) {
  CHECK_RESULT(validator_.OnLoop(loc, sig_type));
  PushLabel(istream_.end());
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnIfExpr(Type sig_type) {
  CHECK_RESULT(validator_.OnIf(loc, sig_type));
  istream_.Emit(Opcode::InterpBrUnless);
  auto fixup = istream_.EmitFixupU32();
  PushLabel(Istream::kInvalidOffset, fixup);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElseExpr() {
  CHECK_RESULT(validator_.OnElse(loc));
  Label* label = TopLabel();
  Istream::Offset fixup_cond_offset = label->fixup_offset;
  istream_.Emit(Opcode::Br);
  label->fixup_offset = istream_.EmitFixupU32();
  istream_.ResolveFixupU32(fixup_cond_offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnEndExpr() {
  SharedValidator::Label* label;
  CHECK_RESULT(validator_.GetLabel(0, &label));
  LabelType label_type = label->label_type;
  CHECK_RESULT(validator_.OnEnd(loc));
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
  CHECK_RESULT(validator_.OnBr(loc, Var(depth)));
  EmitBr(depth, drop_count, keep_count);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnBrIfExpr(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(validator_.OnBrIf(loc, Var(depth)));
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
  CHECK_RESULT(validator_.BeginBrTable(loc));
  Index drop_count, keep_count;
  istream_.Emit(Opcode::BrTable, num_targets);

  for (Index i = 0; i < num_targets; ++i) {
    Index depth = target_depths[i];
    CHECK_RESULT(validator_.OnBrTableTarget(loc, Var(depth)));
    CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
    // Emit DropKeep directly (instead of using EmitDropKeep) so the
    // instruction has a fixed size.
    istream_.Emit(Opcode::InterpDropKeep, drop_count, keep_count);
    EmitBr(depth, 0, 0);
  }
  CHECK_RESULT(validator_.OnBrTableTarget(loc, Var(default_target_depth)));
  CHECK_RESULT(
      GetBrDropKeepCount(default_target_depth, &drop_count, &keep_count));
  // The default case doesn't need a fixed size, since it is never jumped over.
  istream_.EmitDropKeep(drop_count, keep_count);
  EmitBr(default_target_depth, 0, 0);

  CHECK_RESULT(validator_.EndBrTable(loc));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnCallExpr(Index func_index) {
  CHECK_RESULT(validator_.OnCall(loc, Var(func_index)));

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
  CHECK_RESULT(validator_.OnCallIndirect(loc, Var(sig_index), Var(table_index)));
  istream_.Emit(Opcode::CallIndirect, table_index, sig_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnReturnCallExpr(Index func_index) {
  FuncType& func_type = func_types_[func_index];

  Index drop_count, keep_count;
  CHECK_RESULT(
      GetReturnCallDropKeepCount(func_type, 0, &drop_count, &keep_count));
  // The validator must be run after we get the drop/keep counts, since it
  // will change the type stack.
  CHECK_RESULT(validator_.OnReturnCall(loc, Var(func_index)));
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
  FuncType& func_type = module_.func_types[sig_index];

  Index drop_count, keep_count;
  // +1 to include the index of the function.
  CHECK_RESULT(
      GetReturnCallDropKeepCount(func_type, +1, &drop_count, &keep_count));
  // The validator must be run after we get the drop/keep counts, since it
  // changes the type stack.
  CHECK_RESULT(
      validator_.OnReturnCallIndirect(loc, Var(sig_index), Var(table_index)));
  istream_.EmitDropKeep(drop_count, keep_count);
  istream_.Emit(Opcode::ReturnCallIndirect, table_index, sig_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnCompareExpr(Opcode opcode) {
  CHECK_RESULT(validator_.OnCompare(loc, opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnConvertExpr(Opcode opcode) {
  CHECK_RESULT(validator_.OnConvert(loc, opcode));
  istream_.Emit(opcode);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDropExpr() {
  CHECK_RESULT(validator_.OnDrop(loc));
  istream_.Emit(Opcode::Drop);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnI32ConstExpr(uint32_t value) {
  CHECK_RESULT(validator_.OnConst(loc, Type::I32));
  istream_.Emit(Opcode::I32Const, value);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnI64ConstExpr(uint64_t value) {
  CHECK_RESULT(validator_.OnConst(loc, Type::I64));
  istream_.Emit(Opcode::I64Const, value);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnF32ConstExpr(uint32_t value_bits) {
  CHECK_RESULT(validator_.OnConst(loc, Type::F32));
  istream_.Emit(Opcode::F32Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnF64ConstExpr(uint64_t value_bits) {
  CHECK_RESULT(validator_.OnConst(loc, Type::F64));
  istream_.Emit(Opcode::F64Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnV128ConstExpr(v128 value_bits) {
  CHECK_RESULT(validator_.OnConst(loc, Type::V128));
  istream_.Emit(Opcode::V128Const, value_bits);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnGlobalGetExpr(Index global_index) {
  CHECK_RESULT(validator_.OnGlobalGet(loc, Var(global_index)));
  istream_.Emit(Opcode::GlobalGet, global_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnGlobalSetExpr(Index global_index) {
  CHECK_RESULT(validator_.OnGlobalSet(loc, Var(global_index)));
  istream_.Emit(Opcode::GlobalSet, global_index);
  return wabt::Result::Ok;
}

Index BinaryReaderInterp::TranslateLocalIndex(Index local_index) {
  return validator_.type_stack_size() + validator_.GetLocalCount() -
         local_index;
}

wabt::Result BinaryReaderInterp::OnLocalGetExpr(Index local_index) {
  // Get the translated index before calling validator_.OnLocalGet because it
  // will update the type stack size. We need the index to be relative to the
  // old stack size.
  Index translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(validator_.OnLocalGet(loc, Var(local_index)));
  istream_.Emit(Opcode::LocalGet, translated_local_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalSetExpr(Index local_index) {
  // See comment in OnLocalGetExpr above.
  Index translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(validator_.OnLocalSet(loc, Var(local_index)));
  istream_.Emit(Opcode::LocalSet, translated_local_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLocalTeeExpr(Index local_index) {
  CHECK_RESULT(validator_.OnLocalTee(loc, Var(local_index)));
  istream_.Emit(Opcode::LocalTee, TranslateLocalIndex(local_index));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnLoadExpr(Opcode opcode,
                                            uint32_t align_log2,
                                            Address offset) {
  CHECK_RESULT(validator_.OnLoad(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnStoreExpr(Opcode opcode,
                                             uint32_t align_log2,
                                             Address offset) {
  CHECK_RESULT(validator_.OnStore(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryGrowExpr() {
  CHECK_RESULT(validator_.OnMemoryGrow(loc));
  istream_.Emit(Opcode::MemoryGrow, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemorySizeExpr() {
  CHECK_RESULT(validator_.OnMemorySize(loc));
  istream_.Emit(Opcode::MemorySize, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableGrowExpr(Index table_index) {
  CHECK_RESULT(validator_.OnTableGrow(loc, Var(table_index)));
  istream_.Emit(Opcode::TableGrow, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableSizeExpr(Index table_index) {
  CHECK_RESULT(validator_.OnTableSize(loc, Var(table_index)));
  istream_.Emit(Opcode::TableSize, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableFillExpr(Index table_index) {
  CHECK_RESULT(validator_.OnTableFill(loc, Var(table_index)));
  istream_.Emit(Opcode::TableFill, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefFuncExpr(Index func_index) {
  CHECK_RESULT(validator_.OnRefFunc(loc, Var(func_index)));
  istream_.Emit(Opcode::RefFunc, func_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefNullExpr() {
  CHECK_RESULT(validator_.OnRefNull(loc));
  istream_.Emit(Opcode::RefNull);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnRefIsNullExpr() {
  CHECK_RESULT(validator_.OnRefIsNull(loc));
  istream_.Emit(Opcode::RefIsNull);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnNopExpr() {
  CHECK_RESULT(validator_.OnNop(loc));
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnReturnExpr() {
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(validator_.OnReturn(loc));
  istream_.EmitDropKeep(drop_count, keep_count);
  istream_.Emit(Opcode::Return);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnSelectExpr(Type result_type) {
  CHECK_RESULT(validator_.OnSelect(loc, result_type));
  istream_.Emit(Opcode::Select);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnUnreachableExpr() {
  CHECK_RESULT(validator_.OnUnreachable(loc));
  istream_.Emit(Opcode::Unreachable);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicWaitExpr(Opcode opcode,
                                                  uint32_t align_log2,
                                                  Address offset) {
  CHECK_RESULT(validator_.OnAtomicWait(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnAtomicNotifyExpr(Opcode opcode,
                                                    uint32_t align_log2,
                                                    Address offset) {
  CHECK_RESULT(validator_.OnAtomicNotify(loc, opcode, GetAlignment(align_log2)));
  istream_.Emit(opcode, kMemoryIndex0, offset);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryCopyExpr() {
  CHECK_RESULT(validator_.OnMemoryCopy(loc));
  istream_.Emit(Opcode::MemoryCopy, kMemoryIndex0, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnDataDropExpr(Index segment_index) {
  CHECK_RESULT(validator_.OnDataDrop(loc, Var(segment_index)));
  istream_.Emit(Opcode::DataDrop, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryFillExpr() {
  CHECK_RESULT(validator_.OnMemoryFill(loc));
  istream_.Emit(Opcode::MemoryFill, kMemoryIndex0);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnMemoryInitExpr(Index segment_index) {
  CHECK_RESULT(validator_.OnMemoryInit(loc, Var(segment_index)));
  istream_.Emit(Opcode::MemoryInit, kMemoryIndex0, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableGetExpr(Index table_index) {
  CHECK_RESULT(validator_.OnTableGet(loc, Var(table_index)));
  istream_.Emit(Opcode::TableGet, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableSetExpr(Index table_index) {
  CHECK_RESULT(validator_.OnTableSet(loc, Var(table_index)));
  istream_.Emit(Opcode::TableSet, table_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableCopyExpr(Index dst_index,
                                                 Index src_index) {
  CHECK_RESULT(validator_.OnTableCopy(loc, Var(dst_index), Var(src_index)));
  istream_.Emit(Opcode::TableCopy, dst_index, src_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnElemDropExpr(Index segment_index) {
  CHECK_RESULT(validator_.OnElemDrop(loc, Var(segment_index)));
  istream_.Emit(Opcode::ElemDrop, segment_index);
  return wabt::Result::Ok;
}

wabt::Result BinaryReaderInterp::OnTableInitExpr(Index segment_index,
                                                 Index table_index) {
  CHECK_RESULT(
      validator_.OnTableInit(loc, Var(segment_index), Var(table_index)));
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
