/*
 * Copyright 2020 WebAssembly Community Group participants
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

#include "wabt/shared-validator.h"

#include <algorithm>
#include <cinttypes>
#include <limits>

namespace wabt {

TypeVector SharedValidator::ToTypeVector(Index count, const Type* types) {
  return TypeVector(&types[0], &types[count]);
}

SharedValidator::SharedValidator(Errors* errors, const ValidateOptions& options)
    : options_(options),
      errors_(errors),
      typechecker_(options.features, func_types_) {
  typechecker_.set_error_callback(
      [this](const char* msg) { OnTypecheckerError(msg); });
}

Result WABT_PRINTF_FORMAT(3, 4) SharedValidator::PrintError(const Location& loc,
                                                            const char* format,
                                                            ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  errors_->emplace_back(ErrorLevel::Error, loc, buffer);
  return Result::Error;
}

void SharedValidator::OnTypecheckerError(const char* msg) {
  PrintError(expr_loc_, "%s", msg);
}

Result SharedValidator::OnFuncType(const Location& loc,
                                   Index param_count,
                                   const Type* param_types,
                                   Index result_count,
                                   const Type* result_types,
                                   Index type_index) {
  Result result = Result::Ok;
  if (!options_.features.multi_value_enabled() && result_count > 1) {
    result |= PrintError(loc,
                         "multiple result values are not supported without "
                         "multi-value enabled.");
  }
  if (options_.features.reference_types_enabled()) {
    for (Index i = 0; i < param_count; i++) {
      result |= CheckReferenceType(loc, param_types[i], "params");
    }
    for (Index i = 0; i < result_count; i++) {
      result |= CheckReferenceType(loc, result_types[i], "results");
    }
  }
  func_types_.emplace(
      num_types_++,
      FuncType{ToTypeVector(param_count, param_types),
               ToTypeVector(result_count, result_types), type_index});
  return result;
}

Result SharedValidator::OnStructType(const Location&,
                                     Index field_count,
                                     TypeMut* fields) {
  struct_types_.emplace(num_types_++, StructType{TypeMutVector(
                                          &fields[0], &fields[field_count])});
  return Result::Ok;
}

Result SharedValidator::OnArrayType(const Location&, TypeMut field) {
  array_types_.emplace(num_types_++, ArrayType{field});
  return Result::Ok;
}

Result SharedValidator::OnFunction(const Location& loc, Var sig_var) {
  Result result = Result::Ok;
  FuncType type;
  result |= CheckFuncTypeIndex(sig_var, &type);
  funcs_.push_back(type);
  return result;
}

Result SharedValidator::CheckLimits(const Location& loc,
                                    const Limits& limits,
                                    uint64_t absolute_max,
                                    const char* desc) {
  Result result = Result::Ok;
  if (limits.initial > absolute_max) {
    result |=
        PrintError(loc, "initial %s (%" PRIu64 ") must be <= (%" PRIu64 ")",
                   desc, limits.initial, absolute_max);
  }

  if (limits.has_max) {
    if (limits.max > absolute_max) {
      result |= PrintError(loc, "max %s (%" PRIu64 ") must be <= (%" PRIu64 ")",
                           desc, limits.max, absolute_max);
    }

    if (limits.max < limits.initial) {
      result |= PrintError(
          loc, "max %s (%" PRIu64 ") must be >= initial %s (%" PRIu64 ")", desc,
          limits.max, desc, limits.initial);
    }
  }
  return result;
}

Result SharedValidator::OnTable(const Location& loc,
                                Type elem_type,
                                const Limits& limits) {
  Result result = Result::Ok;
  // Must be checked by parser or binary reader.
  assert(elem_type.IsRef());
  if (tables_.size() > 0 && !options_.features.reference_types_enabled()) {
    result |= PrintError(loc, "only one table allowed");
  }
  result |= CheckLimits(loc, limits, UINT32_MAX, "elems");

  if (limits.is_shared) {
    result |= PrintError(loc, "tables may not be shared");
  }
  if (elem_type != Type::FuncRef &&
      !options_.features.reference_types_enabled()) {
    result |= PrintError(loc, "tables must have funcref type");
  }

  result |= CheckReferenceType(loc, elem_type, "tables");

  // TODO: support table initializers
  if (elem_type.IsRef() && !elem_type.IsNullableRef()) {
    result |=
        PrintError(loc, "currently non-nullable references are not supported");
  }

  tables_.push_back(TableType{elem_type, limits});
  return result;
}

Result SharedValidator::OnMemory(const Location& loc,
                                 const Limits& limits,
                                 uint32_t page_size) {
  Result result = Result::Ok;
  if (memories_.size() > 0 && !options_.features.multi_memory_enabled()) {
    result |= PrintError(loc, "only one memory block allowed");
  }

  if (page_size != WABT_DEFAULT_PAGE_SIZE) {
    if (!options_.features.custom_page_sizes_enabled()) {
      result |= PrintError(loc, "only default page size (64 KiB) is allowed");
    } else if (page_size != 1) {
      result |= PrintError(loc, "only page sizes of 1 B or 64 KiB are allowed");
    }
  }

  uint64_t absolute_max = WABT_BYTES_TO_MIN_PAGES(
      (limits.is_64 ? UINT64_MAX : UINT32_MAX), page_size);
  result |= CheckLimits(loc, limits, absolute_max, "pages");

  if (limits.is_shared) {
    if (!options_.features.threads_enabled()) {
      result |= PrintError(loc, "memories may not be shared");
    } else if (!limits.has_max) {
      result |= PrintError(loc, "shared memories must have max sizes");
    }
  }

  memories_.push_back(MemoryType{limits});
  return result;
}

Result SharedValidator::OnGlobalImport(const Location& loc,
                                       Type type,
                                       bool mutable_) {
  Result result = Result::Ok;
  if (mutable_ && !options_.features.mutable_globals_enabled()) {
    result |= PrintError(loc, "mutable globals cannot be imported");
  }
  globals_.push_back(GlobalType{type, mutable_});
  ++num_imported_globals_;
  return result;
}

Result SharedValidator::OnGlobal(const Location& loc,
                                 Type type,
                                 bool mutable_) {
  CHECK_RESULT(CheckReferenceType(loc, type, "globals"));
  globals_.push_back(GlobalType{type, mutable_});
  return Result::Ok;
}

Result SharedValidator::CheckType(const Location& loc,
                                  Type actual,
                                  Type expected,
                                  const char* desc) {
  if (Failed(typechecker_.CheckType(actual, expected))) {
    PrintError(loc, "type mismatch at %s. got %s, expected %s", desc,
               actual.GetName().c_str(), expected.GetName().c_str());
    return Result::Error;
  }
  return Result::Ok;
}

Result SharedValidator::CheckReferenceType(const Location& loc,
                                           Type type,
                                           const char* desc) {
  if (type.IsReferenceWithIndex()) {
    Index index = type.GetReferenceIndex();
    auto iter = func_types_.find(index);

    if (iter == func_types_.end()) {
      return PrintError(loc, "reference %d is out of range in %s",
                        static_cast<int>(index), desc);
    }
  }

  return Result::Ok;
}

Result SharedValidator::OnTag(const Location& loc, Var sig_var) {
  Result result = Result::Ok;
  FuncType type;
  result |= CheckFuncTypeIndex(sig_var, &type);
  if (!type.results.empty()) {
    result |= PrintError(loc, "Tag signature must have 0 results.");
  }
  tags_.push_back(TagType{type.params});
  return result;
}

Result SharedValidator::OnExport(const Location& loc,
                                 ExternalKind kind,
                                 Var item_var,
                                 std::string_view name) {
  Result result = Result::Ok;
  auto name_str = std::string(name);
  if (export_names_.find(name_str) != export_names_.end()) {
    result |= PrintError(loc, "duplicate export \"" PRIstringview "\"",
                         WABT_PRINTF_STRING_VIEW_ARG(name));
  }
  export_names_.insert(name_str);

  switch (kind) {
    case ExternalKind::Func:
      result |= CheckFuncIndex(item_var);
      declared_funcs_.insert(item_var.index());
      break;

    case ExternalKind::Table:
      result |= CheckTableIndex(item_var);
      break;

    case ExternalKind::Memory:
      result |= CheckMemoryIndex(item_var);
      break;

    case ExternalKind::Global:
      result |= CheckGlobalIndex(item_var);
      break;

    case ExternalKind::Tag:
      result |= CheckTagIndex(item_var);
      break;
  }
  return result;
}

Result SharedValidator::OnStart(const Location& loc, Var func_var) {
  Result result = Result::Ok;
  if (starts_++ > 0) {
    result |= PrintError(loc, "only one start function allowed");
  }
  FuncType func_type;
  result |= CheckFuncIndex(func_var, &func_type);
  if (func_type.params.size() != 0) {
    result |= PrintError(loc, "start function must be nullary");
  }
  if (func_type.results.size() != 0) {
    result |= PrintError(loc, "start function must not return anything");
  }
  return result;
}

Result SharedValidator::OnElemSegment(const Location& loc,
                                      Var table_var,
                                      SegmentKind kind) {
  Result result = Result::Ok;
  TableType table_type;
  if (kind == SegmentKind::Active) {
    result |= CheckTableIndex(table_var, &table_type);
  }
  // Type gets set later in OnElemSegmentElemType.
  elems_.push_back(
      ElemType{Type::Void, kind == SegmentKind::Active, table_type.element});
  return result;
}

Result SharedValidator::OnElemSegmentElemType(const Location& loc,
                                              Type elem_type) {
  Result result = Result::Ok;
  auto& elem = elems_.back();
  if (elem.is_active) {
    // Check that the type of the elem segment matches the table in which
    // it is active.
    result |= CheckType(loc, elem_type, elem.table_type, "elem segment");
  }

  if (elem_type.IsReferenceWithIndex()) {
    Index index = elem_type.GetReferenceIndex();
    auto iter = func_types_.find(index);

    if (iter == func_types_.end()) {
      result |=
          PrintError(loc, "reference %" PRIindex " is out of range", index);
    }
  }

  elem.element = elem_type;
  return result;
}

void SharedValidator::OnDataCount(Index count) {
  data_segments_ = count;
}

Result SharedValidator::OnDataSegment(const Location& loc,
                                      Var memory_var,
                                      SegmentKind kind) {
  Result result = Result::Ok;
  if (kind == SegmentKind::Active) {
    result |= CheckMemoryIndex(memory_var);
  }
  return result;
}

Result SharedValidator::CheckDeclaredFunc(Var func_var) {
  if (declared_funcs_.count(func_var.index()) == 0) {
    return PrintError(func_var.loc,
                      "function %" PRIindex
                      " is not declared in any elem sections",
                      func_var.index());
  }
  return Result::Ok;
}

Result SharedValidator::EndModule() {
  // Verify that any ref.func used in init expressions for globals are
  // mentioned in an elems section.  This can't be done while process the
  // globals because the global section comes before the elem section.
  Result result = Result::Ok;
  for (Var func_var : check_declared_funcs_) {
    result |= CheckDeclaredFunc(func_var);
  }
  return result;
}

Result SharedValidator::CheckIndex(Var var, Index max_index, const char* desc) {
  if (var.index() >= max_index) {
    return PrintError(
        var.loc, "%s variable out of range: %" PRIindex " (max %" PRIindex ")",
        desc, var.index(), max_index);
  }
  return Result::Ok;
}

template <typename T>
Result SharedValidator::CheckIndexWithValue(Var var,
                                            const std::vector<T>& values,
                                            T* out,
                                            const char* desc) {
  Result result = CheckIndex(var, values.size(), desc);
  if (out) {
    *out = Succeeded(result) ? values[var.index()] : T{};
  }
  return result;
}

Result SharedValidator::CheckLocalIndex(Var local_var, Type* out_type) {
  auto iter = std::upper_bound(
      locals_.begin(), locals_.end(), local_var.index(),
      [](Index index, const LocalDecl& decl) { return index < decl.end; });
  if (iter == locals_.end()) {
    // TODO: better error
    return PrintError(local_var.loc, "local variable out of range (max %u)",
                      GetLocalCount());
  }
  *out_type = iter->type;
  return Result::Ok;
}

Result SharedValidator::CheckFuncTypeIndex(Var sig_var, FuncType* out) {
  Result result = CheckIndex(sig_var, num_types_, "function type");
  if (Failed(result)) {
    *out = FuncType{};
    return Result::Error;
  }

  auto iter = func_types_.find(sig_var.index());
  if (iter == func_types_.end()) {
    return PrintError(sig_var.loc, "type %d is not a function",
                      sig_var.index());
  }

  if (out) {
    *out = iter->second;
  }
  return Result::Ok;
}

Result SharedValidator::CheckFuncIndex(Var func_var, FuncType* out) {
  return CheckIndexWithValue(func_var, funcs_, out, "function");
}

Result SharedValidator::CheckMemoryIndex(Var memory_var, MemoryType* out) {
  return CheckIndexWithValue(memory_var, memories_, out, "memory");
}

Result SharedValidator::CheckTableIndex(Var table_var, TableType* out) {
  return CheckIndexWithValue(table_var, tables_, out, "table");
}

Result SharedValidator::CheckGlobalIndex(Var global_var, GlobalType* out) {
  return CheckIndexWithValue(global_var, globals_, out, "global");
}

Result SharedValidator::CheckTagIndex(Var tag_var, TagType* out) {
  return CheckIndexWithValue(tag_var, tags_, out, "tag");
}

Result SharedValidator::CheckElemSegmentIndex(Var elem_segment_var,
                                              ElemType* out) {
  return CheckIndexWithValue(elem_segment_var, elems_, out, "elem_segment");
}

Result SharedValidator::CheckDataSegmentIndex(Var data_segment_var) {
  return CheckIndex(data_segment_var, data_segments_, "data_segment");
}

Result SharedValidator::CheckBlockSignature(const Location& loc,
                                            Opcode opcode,
                                            Type sig_type,
                                            TypeVector* out_param_types,
                                            TypeVector* out_result_types) {
  Result result = Result::Ok;

  if (sig_type.IsIndex()) {
    Index sig_index = sig_type.GetIndex();
    FuncType func_type;
    result |= CheckFuncTypeIndex(Var(sig_index, loc), &func_type);

    if (!func_type.params.empty() && !options_.features.multi_value_enabled()) {
      result |= PrintError(loc, "%s params not currently supported.",
                           opcode.GetName());
    }
    // Multiple results without --enable-multi-value is checked above in
    // OnType.

    *out_param_types = func_type.params;
    *out_result_types = func_type.results;
  } else {
    if (sig_type.IsReferenceWithIndex()) {
      Index index = sig_type.GetReferenceIndex();
      auto iter = func_types_.find(index);

      if (iter == func_types_.end()) {
        result |=
            PrintError(loc, "reference %" PRIindex " is out of range", index);
      }
    }

    out_param_types->clear();
    *out_result_types = sig_type.GetInlineVector();
  }

  return result;
}

Index SharedValidator::GetFunctionTypeIndex(Index func_index) const {
  assert(func_index < funcs_.size());
  return funcs_[func_index].type_index;
}

void SharedValidator::SaveLocalRefs() {
  if (!local_ref_is_set_.empty()) {
    Label* label;
    typechecker_.GetLabel(0, &label);
    label->local_ref_is_set_ = local_ref_is_set_;
  }
}

void SharedValidator::RestoreLocalRefs(Result result) {
  if (!local_ref_is_set_.empty()) {
    if (Succeeded(result)) {
      Label* label;
      typechecker_.GetLabel(0, &label);
      assert(local_ref_is_set_.size() == label->local_ref_is_set_.size());
      local_ref_is_set_ = label->local_ref_is_set_;
    } else {
      IgnoreLocalRefs();
    }
  }
}

void SharedValidator::IgnoreLocalRefs() {
  if (!local_ref_is_set_.empty()) {
    std::fill(local_ref_is_set_.begin(), local_ref_is_set_.end(), true);
  }
}

Result SharedValidator::BeginInitExpr(const Location& loc, Type type) {
  expr_loc_ = loc;
  in_init_expr_ = true;
  return typechecker_.BeginInitExpr(type);
}

Result SharedValidator::EndInitExpr() {
  in_init_expr_ = false;
  return typechecker_.EndInitExpr();
}

Result SharedValidator::BeginFunctionBody(const Location& loc,
                                          Index func_index) {
  expr_loc_ = loc;
  locals_.clear();
  if (func_index < funcs_.size()) {
    for (Type type : funcs_[func_index].params) {
      // TODO: Coalesce parameters of the same type?
      locals_.push_back(LocalDecl{type, GetLocalCount() + 1});
    }
    return typechecker_.BeginFunction(funcs_[func_index].results);
  } else {
    // Signature isn't available, use empty.
    return typechecker_.BeginFunction(TypeVector());
  }
}

Result SharedValidator::EndFunctionBody(const Location& loc) {
  expr_loc_ = loc;
  return typechecker_.EndFunction();
}

Result SharedValidator::OnLocalDecl(const Location& loc,
                                    Index count,
                                    Type type) {
  const auto max_locals = std::numeric_limits<Index>::max();
  if (count > max_locals - GetLocalCount()) {
    PrintError(loc, "local count must be < 0x10000000");
    return Result::Error;
  }

  CHECK_RESULT(CheckReferenceType(loc, type, "locals"));

  Index local_count = GetLocalCount();

  if (type.IsNonNullableRef()) {
    for (Index i = 0; i < count; i++) {
      local_refs_map_[local_count + i] =
          LocalReferenceMap{type, static_cast<Index>(local_ref_is_set_.size())};
      local_ref_is_set_.push_back(false);
    }
  }

  locals_.push_back(LocalDecl{type, local_count + count});
  return Result::Ok;
}

Index SharedValidator::GetLocalCount() const {
  return locals_.empty() ? 0 : locals_.back().end;
}

static bool is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

Result SharedValidator::CheckAlign(const Location& loc,
                                   Address alignment,
                                   Address natural_alignment) {
  if (!is_power_of_two(alignment)) {
    PrintError(loc, "alignment (%" PRIaddress ") must be a power of 2",
               alignment);
    return Result::Error;
  }
  if (alignment > natural_alignment) {
    PrintError(
        loc,
        "alignment must not be larger than natural alignment (%" PRIaddress ")",
        natural_alignment);
    return Result::Error;
  }
  return Result::Ok;
}

Result SharedValidator::CheckOffset(const Location& loc,
                                    Address offset,
                                    const Limits& limits) {
  if ((!limits.is_64) && (offset > UINT32_MAX)) {
    PrintError(loc, "offset must be less than or equal to 0xffffffff");
    return Result::Error;
  }

  return Result::Ok;
}

Result SharedValidator::CheckAtomicAlign(const Location& loc,
                                         Address alignment,
                                         Address natural_alignment) {
  if (!is_power_of_two(alignment)) {
    PrintError(loc, "alignment (%" PRIaddress ") must be a power of 2",
               alignment);
    return Result::Error;
  }
  if (alignment != natural_alignment) {
    PrintError(loc,
               "alignment must be equal to natural alignment (%" PRIaddress ")",
               natural_alignment);
    return Result::Error;
  }
  return Result::Ok;
}

bool SharedValidator::ValidInitOpcode(Opcode opcode) const {
  if (opcode == Opcode::GlobalGet || opcode == Opcode::I32Const ||
      opcode == Opcode::I64Const || opcode == Opcode::F32Const ||
      opcode == Opcode::F64Const || opcode == Opcode::RefFunc ||
      opcode == Opcode::RefNull) {
    return true;
  }
  if (options_.features.extended_const_enabled()) {
    if (opcode == Opcode::I32Mul || opcode == Opcode::I64Mul ||
        opcode == Opcode::I32Sub || opcode == Opcode::I64Sub ||
        opcode == Opcode::I32Add || opcode == Opcode::I64Add) {
      return true;
    }
  }
  return false;
}

Result SharedValidator::CheckInstr(Opcode opcode, const Location& loc) {
  expr_loc_ = loc;
  if (in_init_expr_ && !ValidInitOpcode(opcode)) {
    PrintError(loc,
               "invalid initializer: instruction not valid in initializer "
               "expression: %s",
               opcode.GetName());
    return Result::Error;
  }
  return Result::Ok;
}

Result SharedValidator::OnAtomicFence(const Location& loc,
                                      uint32_t consistency_model) {
  Result result = CheckInstr(Opcode::AtomicFence, loc);
  if (consistency_model != 0) {
    result |= PrintError(
        loc, "unexpected atomic.fence consistency model (expected 0): %u",
        consistency_model);
  }
  result |= typechecker_.OnAtomicFence(consistency_model);
  return result;
}

Result SharedValidator::OnAtomicLoad(const Location& loc,
                                     Opcode opcode,
                                     Var memidx,
                                     Address alignment,
                                     Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicLoad(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnAtomicNotify(const Location& loc,
                                       Opcode opcode,
                                       Var memidx,
                                       Address alignment,
                                       Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicNotify(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnAtomicRmwCmpxchg(const Location& loc,
                                           Opcode opcode,
                                           Var memidx,
                                           Address alignment,
                                           Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicRmwCmpxchg(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnAtomicRmw(const Location& loc,
                                    Opcode opcode,
                                    Var memidx,
                                    Address alignment,
                                    Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicRmw(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnAtomicStore(const Location& loc,
                                      Opcode opcode,
                                      Var memidx,
                                      Address alignment,
                                      Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicStore(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnAtomicWait(const Location& loc,
                                     Opcode opcode,
                                     Var memidx,
                                     Address alignment,
                                     Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAtomicAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnAtomicWait(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnBinary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnBinary(opcode);
  return result;
}

Result SharedValidator::OnBlock(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::Block, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::Block, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.OnBlock(param_types, result_types);
  SaveLocalRefs();
  return result;
}

Result SharedValidator::OnBr(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::Br, loc);
  result |= typechecker_.OnBr(depth.index());
  IgnoreLocalRefs();
  return result;
}

Result SharedValidator::OnBrIf(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::BrIf, loc);
  result |= typechecker_.OnBrIf(depth.index());
  return result;
}

Result SharedValidator::OnBrOnNonNull(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::BrOnNonNull, loc);
  result |= typechecker_.OnBrOnNonNull(depth.index());
  return result;
}

Result SharedValidator::OnBrOnNull(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::BrOnNull, loc);
  result |= typechecker_.OnBrOnNull(depth.index());
  return result;
}

Result SharedValidator::BeginBrTable(const Location& loc) {
  Result result = CheckInstr(Opcode::BrTable, loc);
  result |= typechecker_.BeginBrTable();
  return result;
}

Result SharedValidator::OnBrTableTarget(const Location& loc, Var depth) {
  Result result = Result::Ok;
  expr_loc_ = loc;
  result |= typechecker_.OnBrTableTarget(depth.index());
  return result;
}

Result SharedValidator::EndBrTable(const Location& loc) {
  Result result = CheckInstr(Opcode::BrTable, loc);
  result |= typechecker_.EndBrTable();
  IgnoreLocalRefs();
  return result;
}

Result SharedValidator::OnCall(const Location& loc, Var func_var) {
  Result result = CheckInstr(Opcode::Call, loc);
  FuncType func_type;
  result |= CheckFuncIndex(func_var, &func_type);
  result |= typechecker_.OnCall(func_type.params, func_type.results);
  return result;
}

Result SharedValidator::OnCallIndirect(const Location& loc,
                                       Var sig_var,
                                       Var table_var) {
  Result result = CheckInstr(Opcode::CallIndirect, loc);
  FuncType func_type;
  TableType table_type;
  result |= CheckFuncTypeIndex(sig_var, &func_type);
  result |= CheckTableIndex(table_var, &table_type);
  if (table_type.element != Type::FuncRef) {
    result |= PrintError(
        loc,
        "type mismatch: call_indirect must reference table of funcref type");
  }
  result |= typechecker_.OnCallIndirect(func_type.params, func_type.results,
                                        table_type.limits);
  return result;
}

Result SharedValidator::OnCallRef(const Location& loc, Var function_type_var) {
  Result result = CheckInstr(Opcode::CallRef, loc);
  FuncType func_type;
  result |= typechecker_.OnCallRef(function_type_var.to_type());
  result |= CheckFuncTypeIndex(function_type_var, &func_type);
  result |= typechecker_.OnCall(func_type.params, func_type.results);
  return result;
}

Result SharedValidator::OnCatch(const Location& loc,
                                Var tag_var,
                                bool is_catch_all) {
  Result result = CheckInstr(Opcode::Catch, loc);
  if (is_catch_all) {
    TypeVector empty;
    result |= typechecker_.OnCatch(empty);
  } else {
    TagType tag_type;
    result |= CheckTagIndex(tag_var, &tag_type);
    result |= typechecker_.OnCatch(tag_type.params);
  }
  RestoreLocalRefs(result);
  return result;
}

Result SharedValidator::OnCompare(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnCompare(opcode);
  return result;
}

Result SharedValidator::OnConst(const Location& loc, Type type) {
  Result result = Result::Ok;
  expr_loc_ = loc;
  result |= typechecker_.OnConst(type);
  return result;
}

Result SharedValidator::OnConvert(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnConvert(opcode);
  return result;
}

Result SharedValidator::OnDataDrop(const Location& loc, Var segment_var) {
  Result result = CheckInstr(Opcode::DataDrop, loc);
  result |= CheckDataSegmentIndex(segment_var);
  result |= typechecker_.OnDataDrop(segment_var.index());
  return result;
}

Result SharedValidator::OnDelegate(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::Delegate, loc);
  result |= typechecker_.OnDelegate(depth.index());
  return result;
}

Result SharedValidator::OnDrop(const Location& loc) {
  Result result = CheckInstr(Opcode::Drop, loc);
  result |= typechecker_.OnDrop();
  return result;
}

Result SharedValidator::OnElemDrop(const Location& loc, Var segment_var) {
  Result result = CheckInstr(Opcode::ElemDrop, loc);
  result |= CheckElemSegmentIndex(segment_var);
  result |= typechecker_.OnElemDrop(segment_var.index());
  return result;
}

Result SharedValidator::OnElse(const Location& loc) {
  // Don't call CheckInstr or update expr_loc_ here because if we fail we want
  // the last expression in the If block to be reported as the error location,
  // not the else itself.
  Result result = Result::Ok;
  result |= typechecker_.OnElse();
  RestoreLocalRefs(result);
  return result;
}

Result SharedValidator::OnEnd(const Location& loc) {
  Result result = CheckInstr(Opcode::End, loc);
  RestoreLocalRefs(result);
  result |= typechecker_.OnEnd();
  return result;
}

Result SharedValidator::OnGlobalGet(const Location& loc, Var global_var) {
  Result result = CheckInstr(Opcode::GlobalGet, loc);
  GlobalType global_type;
  result |= CheckGlobalIndex(global_var, &global_type);
  result |= typechecker_.OnGlobalGet(global_type.type);
  if (Succeeded(result) && in_init_expr_) {
    if (global_var.index() >= num_imported_globals_) {
      result |= PrintError(
          global_var.loc,
          "initializer expression can only reference an imported global");
    }
    if (global_type.mutable_) {
      result |= PrintError(
          loc, "initializer expression cannot reference a mutable global");
    }
  }

  return result;
}

Result SharedValidator::OnGlobalSet(const Location& loc, Var global_var) {
  Result result = CheckInstr(Opcode::GlobalSet, loc);
  GlobalType global_type;
  result |= CheckGlobalIndex(global_var, &global_type);
  if (!global_type.mutable_) {
    result |= PrintError(
        loc, "can't global.set on immutable global at index %" PRIindex ".",
        global_var.index());
  }
  result |= typechecker_.OnGlobalSet(global_type.type);
  return result;
}

Result SharedValidator::OnIf(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::If, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::If, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.OnIf(param_types, result_types);
  SaveLocalRefs();
  return result;
}

Result SharedValidator::OnLoad(const Location& loc,
                               Opcode opcode,
                               Var memidx,
                               Address alignment,
                               Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnLoad(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnLoadSplat(const Location& loc,
                                    Opcode opcode,
                                    Var memidx,
                                    Address alignment,
                                    Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnLoad(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnLoadZero(const Location& loc,
                                   Opcode opcode,
                                   Var memidx,
                                   Address alignment,
                                   Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnLoad(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnLocalGet(const Location& loc, Var local_var) {
  CHECK_RESULT(CheckInstr(Opcode::LocalGet, loc));
  Result result = Result::Ok;
  Type type = Type::Any;
  result |= CheckLocalIndex(local_var, &type);
  result |= typechecker_.OnLocalGet(type);
  if (Succeeded(result) && type.IsNonNullableRef()) {
    auto it = local_refs_map_.find(local_var.index());
    if (it != local_refs_map_.end() &&
        !local_ref_is_set_[it->second.bit_index]) {
      return PrintError(local_var.loc, "uninitialized local reference");
    }
  }
  return result;
}

Result SharedValidator::OnLocalSet(const Location& loc, Var local_var) {
  CHECK_RESULT(CheckInstr(Opcode::LocalSet, loc));
  Result result = Result::Ok;
  Type type = Type::Any;
  result |= CheckLocalIndex(local_var, &type);
  result |= typechecker_.OnLocalSet(type);
  if (Succeeded(result) && type.IsNonNullableRef()) {
    auto it = local_refs_map_.find(local_var.index());
    if (it != local_refs_map_.end()) {
      local_ref_is_set_[it->second.bit_index] = true;
    }
  }
  return result;
}

Result SharedValidator::OnLocalTee(const Location& loc, Var local_var) {
  CHECK_RESULT(CheckInstr(Opcode::LocalTee, loc));
  Result result = Result::Ok;
  Type type = Type::Any;
  result |= CheckLocalIndex(local_var, &type);
  result |= typechecker_.OnLocalTee(type);
  if (Succeeded(result) && type.IsNonNullableRef()) {
    auto it = local_refs_map_.find(local_var.index());
    if (it != local_refs_map_.end()) {
      local_ref_is_set_[it->second.bit_index] = true;
    }
  }
  return result;
}

Result SharedValidator::OnLoop(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::Loop, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::Loop, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.OnLoop(param_types, result_types);
  SaveLocalRefs();
  return result;
}

Result SharedValidator::OnMemoryCopy(const Location& loc,
                                     Var destmemidx,
                                     Var srcmemidx) {
  Result result = CheckInstr(Opcode::MemoryCopy, loc);
  MemoryType srcmt;
  MemoryType dstmt;
  result |= CheckMemoryIndex(destmemidx, &dstmt);
  result |= CheckMemoryIndex(srcmemidx, &srcmt);
  result |= typechecker_.OnMemoryCopy(dstmt.limits, srcmt.limits);
  return result;
}

Result SharedValidator::OnMemoryFill(const Location& loc, Var memidx) {
  Result result = CheckInstr(Opcode::MemoryFill, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= typechecker_.OnMemoryFill(mt.limits);
  return result;
}

Result SharedValidator::OnMemoryGrow(const Location& loc, Var memidx) {
  Result result = CheckInstr(Opcode::MemoryGrow, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= typechecker_.OnMemoryGrow(mt.limits);
  return result;
}

Result SharedValidator::OnMemoryInit(const Location& loc,
                                     Var segment_var,
                                     Var memidx) {
  Result result = CheckInstr(Opcode::MemoryInit, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckDataSegmentIndex(segment_var);
  result |= typechecker_.OnMemoryInit(segment_var.index(), mt.limits);
  return result;
}

Result SharedValidator::OnMemorySize(const Location& loc, Var memidx) {
  Result result = CheckInstr(Opcode::MemorySize, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= typechecker_.OnMemorySize(mt.limits);
  return result;
}

Result SharedValidator::OnNop(const Location& loc) {
  Result result = CheckInstr(Opcode::Nop, loc);
  return result;
}

Result SharedValidator::OnRefAsNonNull(const Location& loc) {
  Result result = CheckInstr(Opcode::Nop, loc);
  result |= typechecker_.OnRefAsNonNullExpr();
  return result;
}

Result SharedValidator::OnRefFunc(const Location& loc, Var func_var) {
  Result result = CheckInstr(Opcode::RefFunc, loc);
  result |= CheckFuncIndex(func_var);
  if (Succeeded(result)) {
    // References in initializer expressions are considered declarations, as
    // opposed to references in function bodies that are considered usages.
    if (in_init_expr_) {
      declared_funcs_.insert(func_var.index());
    } else {
      check_declared_funcs_.push_back(func_var);
    }
    Index func_type = GetFunctionTypeIndex(func_var.index());
    result |= typechecker_.OnRefFuncExpr(func_type);
  }
  return result;
}

Result SharedValidator::OnRefIsNull(const Location& loc) {
  Result result = CheckInstr(Opcode::RefIsNull, loc);
  result |= typechecker_.OnRefIsNullExpr();
  return result;
}

Result SharedValidator::OnRefNull(const Location& loc, Var func_type_var) {
  Result result = CheckInstr(Opcode::RefNull, loc);

  Type type = func_type_var.to_type();

  switch (type) {
    case Type::RefNull:
      result |= CheckIndex(func_type_var, num_types_, "function type");
      break;
    case Type::FuncRef:
    case Type::ExnRef:
    case Type::ExternRef:
      break;
    default:
      result |= PrintError(
          loc, "Only ref, externref, exnref, funcref are allowed for ref.null");
      break;
  }

  result |= typechecker_.OnRefNullExpr(type);
  return result;
}

Result SharedValidator::OnRethrow(const Location& loc, Var depth) {
  Result result = CheckInstr(Opcode::Rethrow, loc);
  result |= typechecker_.OnRethrow(depth.index());
  return result;
}

Result SharedValidator::OnReturnCall(const Location& loc, Var func_var) {
  Result result = CheckInstr(Opcode::ReturnCall, loc);
  FuncType func_type;
  result |= CheckFuncIndex(func_var, &func_type);
  result |= typechecker_.OnReturnCall(func_type.params, func_type.results);
  IgnoreLocalRefs();
  return result;
}

Result SharedValidator::OnReturnCallIndirect(const Location& loc,
                                             Var sig_var,
                                             Var table_var) {
  Result result = CheckInstr(Opcode::CallIndirect, loc);
  FuncType func_type;
  TableType table_type;
  result |= CheckFuncTypeIndex(sig_var, &func_type);
  result |= CheckTableIndex(table_var, &table_type);
  if (table_type.element != Type::FuncRef) {
    result |= PrintError(loc,
                         "type mismatch: return_call_indirect must reference "
                         "table of funcref type");
  }
  result |=
      typechecker_.OnReturnCallIndirect(func_type.params, func_type.results);
  IgnoreLocalRefs();
  return result;
}

Result SharedValidator::OnReturnCallRef(const Location& loc,
                                        Var function_type_var) {
  Result result = CheckInstr(Opcode::ReturnCallRef, loc);
  FuncType func_type;
  result |= typechecker_.OnReturnCallRef(function_type_var.to_type());
  result |= CheckFuncTypeIndex(function_type_var, &func_type);
  result |= typechecker_.OnReturnCall(func_type.params, func_type.results);
  return result;
}

Result SharedValidator::OnReturn(const Location& loc) {
  Result result = CheckInstr(Opcode::Return, loc);
  result |= typechecker_.OnReturn();
  IgnoreLocalRefs();
  return result;
}

Result SharedValidator::OnSelect(const Location& loc,
                                 Index result_count,
                                 Type* result_types) {
  Result result = CheckInstr(Opcode::Select, loc);

  for (Index i = 0; i < result_count; i++) {
    if (result_types[i].IsReferenceWithIndex()) {
      Index index = result_types[i].GetReferenceIndex();
      auto iter = func_types_.find(index);

      if (iter == func_types_.end()) {
        result |=
            PrintError(loc, "reference %" PRIindex " is out of range", index);
      }
    }
  }

  if (result_count > 1) {
    result |=
        PrintError(loc, "invalid arity in select instruction: %" PRIindex ".",
                   result_count);
  } else {
    result |= typechecker_.OnSelect(ToTypeVector(result_count, result_types));
  }
  return result;
}

Result SharedValidator::OnSimdLaneOp(const Location& loc,
                                     Opcode opcode,
                                     uint64_t value) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnSimdLaneOp(opcode, value);
  return result;
}

Result SharedValidator::OnSimdLoadLane(const Location& loc,
                                       Opcode opcode,
                                       Var memidx,
                                       Address alignment,
                                       Address offset,
                                       uint64_t value) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnSimdLoadLane(opcode, mt.limits, value);
  return result;
}

Result SharedValidator::OnSimdStoreLane(const Location& loc,
                                        Opcode opcode,
                                        Var memidx,
                                        Address alignment,
                                        Address offset,
                                        uint64_t value) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnSimdStoreLane(opcode, mt.limits, value);
  return result;
}

Result SharedValidator::OnSimdShuffleOp(const Location& loc,
                                        Opcode opcode,
                                        v128 value) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnSimdShuffleOp(opcode, value);
  return result;
}

Result SharedValidator::OnStore(const Location& loc,
                                Opcode opcode,
                                Var memidx,
                                Address alignment,
                                Address offset) {
  Result result = CheckInstr(opcode, loc);
  MemoryType mt;
  result |= CheckMemoryIndex(memidx, &mt);
  result |= CheckAlign(loc, alignment, opcode.GetMemorySize());
  result |= CheckOffset(loc, offset, mt.limits);
  result |= typechecker_.OnStore(opcode, mt.limits);
  return result;
}

Result SharedValidator::OnTableCopy(const Location& loc,
                                    Var dst_var,
                                    Var src_var) {
  Result result = CheckInstr(Opcode::TableCopy, loc);
  TableType dst_table;
  TableType src_table;
  result |= CheckTableIndex(dst_var, &dst_table);
  result |= CheckTableIndex(src_var, &src_table);
  result |= typechecker_.OnTableCopy(dst_table.limits, src_table.limits);
  result |= CheckType(loc, src_table.element, dst_table.element, "table.copy");
  return result;
}

Result SharedValidator::OnTableFill(const Location& loc, Var table_var) {
  Result result = CheckInstr(Opcode::TableFill, loc);
  TableType table_type;
  result |= CheckTableIndex(table_var, &table_type);
  result |= typechecker_.OnTableFill(table_type.element, table_type.limits);
  return result;
}

Result SharedValidator::OnTableGet(const Location& loc, Var table_var) {
  Result result = CheckInstr(Opcode::TableGet, loc);
  TableType table_type;
  result |= CheckTableIndex(table_var, &table_type);
  result |= typechecker_.OnTableGet(table_type.element, table_type.limits);
  return result;
}

Result SharedValidator::OnTableGrow(const Location& loc, Var table_var) {
  Result result = CheckInstr(Opcode::TableGrow, loc);
  TableType table_type;
  result |= CheckTableIndex(table_var, &table_type);
  result |= typechecker_.OnTableGrow(table_type.element, table_type.limits);
  return result;
}

Result SharedValidator::OnTableInit(const Location& loc,
                                    Var segment_var,
                                    Var table_var) {
  Result result = CheckInstr(Opcode::TableInit, loc);
  TableType table_type;
  ElemType elem_type;
  result |= CheckTableIndex(table_var, &table_type);
  result |= CheckElemSegmentIndex(segment_var, &elem_type);
  result |= typechecker_.OnTableInit(segment_var.index(), table_type.limits);
  result |= CheckType(loc, elem_type.element, table_type.element, "table.init");
  return result;
}

Result SharedValidator::OnTableSet(const Location& loc, Var table_var) {
  Result result = CheckInstr(Opcode::TableSet, loc);
  TableType table_type;
  result |= CheckTableIndex(table_var, &table_type);
  result |= typechecker_.OnTableSet(table_type.element, table_type.limits);
  return result;
}

Result SharedValidator::OnTableSize(const Location& loc, Var table_var) {
  Result result = CheckInstr(Opcode::TableSize, loc);
  TableType tt;
  result |= CheckTableIndex(table_var, &tt);
  result |= typechecker_.OnTableSize(tt.limits);
  return result;
}

Result SharedValidator::OnTernary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnTernary(opcode);
  return result;
}

Result SharedValidator::OnThrow(const Location& loc, Var tag_var) {
  Result result = CheckInstr(Opcode::Throw, loc);
  TagType tag_type;
  result |= CheckTagIndex(tag_var, &tag_type);
  result |= typechecker_.OnThrow(tag_type.params);
  return result;
}

Result SharedValidator::OnThrowRef(const Location& loc) {
  Result result = CheckInstr(Opcode::ThrowRef, loc);
  result |= typechecker_.OnThrowRef();
  return result;
}

Result SharedValidator::OnTry(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::Try, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::Try, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.OnTry(param_types, result_types);
  SaveLocalRefs();
  return result;
}

Result SharedValidator::BeginTryTable(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::TryTable, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::TryTable, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.BeginTryTable(param_types);
  return result;
}

Result SharedValidator::OnTryTableCatch(const Location& loc,
                                        const TableCatch& catch_) {
  Result result = Result::Ok;
  TagType tag_type;
  expr_loc_ = loc;
  if (!catch_.IsCatchAll()) {
    result |= CheckTagIndex(catch_.tag, &tag_type);
  }
  if (catch_.IsRef()) {
    tag_type.params.push_back(Type::ExnRef);
  }
  result |=
      typechecker_.OnTryTableCatch(tag_type.params, catch_.target.index());
  return result;
}

Result SharedValidator::EndTryTable(const Location& loc, Type sig_type) {
  Result result = CheckInstr(Opcode::TryTable, loc);
  TypeVector param_types, result_types;
  result |= CheckBlockSignature(loc, Opcode::TryTable, sig_type, &param_types,
                                &result_types);
  result |= typechecker_.EndTryTable(param_types, result_types);
  return result;
}

Result SharedValidator::OnUnary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnUnary(opcode);
  return result;
}

Result SharedValidator::OnUnreachable(const Location& loc) {
  Result result = CheckInstr(Opcode::Unreachable, loc);
  result |= typechecker_.OnUnreachable();
  IgnoreLocalRefs();
  return result;
}

}  // namespace wabt
