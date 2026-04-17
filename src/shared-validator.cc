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

SharedValidator::SharedValidator(Errors* errors,
                                 std::string_view filename,
                                 const ValidateOptions& options)
    : options_(options),
      errors_(errors),
      filename_(filename),
      typechecker_(options.features, type_fields_) {
  typechecker_.set_error_callback(
      [this](const char* msg) { OnTypecheckerError(msg); });
}

Result WABT_PRINTF_FORMAT(3, 4) SharedValidator::PrintError(const Location& loc,
                                                            const char* format,
                                                            ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  errors_->emplace_back(ErrorLevel::Error, loc, filename_, buffer);
  return Result::Error;
}

void SharedValidator::OnTypecheckerError(const char* msg) {
  PrintError(expr_loc_, "%s", msg);
}

Result SharedValidator::OnRecursiveGroup(Index first_type_index,
                                         Index type_count) {
  if (type_count > 0) {
    type_fields_.rec_groups.emplace_back(
        RecGroup(first_type_index, type_count));
    last_rec_type_end_ = first_type_index + type_count;
  }
  return Result::Ok;
}

Result SharedValidator::OnFuncType(const Location& loc,
                                   Index param_count,
                                   const Type* param_types,
                                   Index result_count,
                                   const Type* result_types,
                                   Index type_index,
                                   SupertypesInfo* supertypes) {
  Result result = Result::Ok;
  if (!options_.features.multi_value_enabled() && result_count > 1) {
    result |= PrintError(loc,
                         "multiple result values are not supported without "
                         "multi-value enabled.");
  }

  type_fields_.PushFunc(FuncType{ToTypeVector(param_count, param_types),
                                 ToTypeVector(result_count, result_types),
                                 type_index});

  if (options_.features.function_references_enabled()) {
    Index rec_end = GetRecGroupEnd();

    for (Index i = 0; i < param_count; i++) {
      result |= CheckReferenceType(loc, param_types[i], rec_end, "params");
    }
    for (Index i = 0; i < result_count; i++) {
      result |= CheckReferenceType(loc, result_types[i], rec_end, "results");
    }

    type_validation_result_ |= result;
  }
  result |= CheckSupertypes(loc, supertypes);

  return result;
}

Result SharedValidator::OnStructType(const Location& loc,
                                     Index field_count,
                                     TypeMut* fields,
                                     SupertypesInfo* supertypes) {
  type_fields_.PushStruct(
      StructType{TypeMutVector(&fields[0], &fields[field_count])});

  Result result = Result::Ok;
  Index rec_end = GetRecGroupEnd();

  for (Index i = 0; i < field_count; i++) {
    result |= CheckReferenceType(loc, fields[i].type, rec_end, "params");
  }

  type_validation_result_ |= result;
  result |= CheckSupertypes(loc, supertypes);
  return result;
}

Result SharedValidator::OnArrayType(const Location& loc,
                                    TypeMut field,
                                    SupertypesInfo* supertypes) {
  type_fields_.PushArray(ArrayType{field});

  Result result =
      CheckReferenceType(loc, field.type, GetRecGroupEnd(), "params");

  type_validation_result_ |= result;
  result |= CheckSupertypes(loc, supertypes);
  return result;
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
                                const Limits& limits,
                                TableImportStatus import_status,
                                TableInitExprStatus init_provided) {
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
  if (options_.features.reference_types_enabled()) {
    if (!elem_type.IsRef()) {
      result |= PrintError(loc, "ables may only contain reference types");
    } else if (import_status == TableImportStatus::TableIsNotImported &&
               init_provided ==
                   TableInitExprStatus::TableWithoutInitExpression &&
               !elem_type.IsNullableRef()) {
      result |= PrintError(loc, "missing table initializer");
    }
  } else if (elem_type != Type::FuncRef) {
    result |= PrintError(loc, "tables must have funcref type");
  }

  result |=
      CheckReferenceType(loc, elem_type, type_fields_.NumTypes(), "tables");

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
  ++last_initialized_global_;
  return result;
}

Result SharedValidator::BeginGlobal(const Location& loc,
                                    Type type,
                                    bool mutable_) {
  CHECK_RESULT(
      CheckReferenceType(loc, type, type_fields_.NumTypes(), "globals"));
  globals_.push_back(GlobalType{type, mutable_});
  return Result::Ok;
}

Result SharedValidator::EndGlobal(const Location&) {
  if (options_.features.gc_enabled()) {
    last_initialized_global_++;
  }
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
                                           Index end_index,
                                           const char* desc) {
  if (type.IsReferenceWithIndex() && type.GetReferenceIndex() >= end_index) {
    return PrintError(loc,
                      "reference %" PRIindex " is out of range (max: %" PRIindex
                      ") in %s",
                      type.GetReferenceIndex(), end_index, desc);
  }

  return Result::Ok;
}

Result SharedValidator::CheckSupertypes(const Location& loc,
                                        SupertypesInfo* supertypes) {
  TypeEntry& entry = type_fields_.type_entries.back();
  Index current_index = type_fields_.NumTypes() - 1;
  Index end_index;

  if (current_index < last_rec_type_end_) {
    end_index = last_rec_type_end_;
  } else {
    type_fields_.rec_groups.emplace_back(RecGroup(current_index, 1));
    end_index = current_index + 1;
  }

  // Check default.
  assert(entry.canonical_index == current_index && entry.is_final_sub_type &&
         entry.first_sub_type == kInvalidIndex);
  entry.is_final_sub_type = supertypes->is_final_sub_type;

  if (supertypes->sub_type_count > 1) {
    type_validation_result_ = Result::Error;
    return PrintError(loc, "sub type count %" PRIindex " is limited to 1",
                      supertypes->sub_type_count);
  }

  if (supertypes->sub_type_count == 1) {
    entry.first_sub_type = supertypes->sub_types[0];

    if (supertypes->sub_types[0] >= current_index) {
      type_validation_result_ = Result::Error;
      return PrintError(loc, "invalid sub type %" PRIindex,
                        supertypes->sub_types[0]);
    }

    if (type_fields_.type_entries[entry.first_sub_type].is_final_sub_type) {
      type_validation_result_ = Result::Error;
      return PrintError(loc, "sub type %" PRIindex " has final property",
                        entry.first_sub_type);
    }
  }

  if (Failed(type_validation_result_) || end_index != current_index + 1) {
    return Result::Ok;
  }

  Index start_index = type_fields_.rec_groups.back().start_index;

  uint32_t hash_code = 0;

  // Type checking could be done without computing the canonical_index,
  // but runtime and validation checks could be very slow without it.
  for (Index i = start_index; i < end_index; i++) {
    hash_code = typechecker_.UpdateHashCode(hash_code, i, start_index);
  }

  type_fields_.rec_groups.back().hash_code = hash_code;

  size_t size = type_fields_.rec_groups.size() - 1;
  Index type_count = end_index - start_index;

  for (Index i = 0; i < size; i++) {
    if (type_fields_.rec_groups[i].hash_code == hash_code &&
        type_fields_.rec_groups[i].type_count == type_count) {
      Index base_index = type_fields_.rec_groups[i].start_index;
      bool is_equal = true;

      for (Index j = 0; j < type_count; j++) {
        if (!typechecker_.CheckTypeFields(start_index + j, start_index,
                                          base_index + j, base_index, true)) {
          is_equal = false;
          break;
        }
      }

      if (is_equal) {
        for (Index j = start_index; j < end_index; j++) {
          type_fields_.type_entries[j].canonical_index = base_index++;
        }
        // An equal recurisve type is present in the list, there is
        // no need to compare other recursive types to this type.
        type_fields_.rec_groups.pop_back();
        break;
      }
    }
  }

  for (Index i = start_index; i < end_index; i++) {
    Index first_sub_type = type_fields_.type_entries[i].first_sub_type;
    if (first_sub_type != kInvalidIndex &&
        !typechecker_.CheckTypeFields(i, kInvalidIndex, first_sub_type,
                                      kInvalidIndex, false)) {
      PrintError(Location(),
                 "sub type %" PRIindex " does not match super type %" PRIindex,
                 type_fields_.type_entries[i].first_sub_type, i);
      type_validation_result_ = Result::Error;
      return Result::Error;
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

    if (index >= type_fields_.NumTypes()) {
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
  Result result = CheckIndex(sig_var, type_fields_.NumTypes(), "function type");
  if (Failed(result)) {
    out->type_index = kInvalidIndex;
    return Result::Error;
  }

  Index index = sig_var.index();
  assert(index < type_fields_.NumTypes());
  if (type_fields_.type_entries[index].kind != Type::FuncRef) {
    return PrintError(sig_var.loc, "type %d is not a function",
                      sig_var.index());
  }

  *out = type_fields_.func_types[type_fields_.type_entries[index].map_index];
  return Result::Ok;
}

Result SharedValidator::CheckStructTypeIndex(Var type_var,
                                             Type* out_ref,
                                             StructType* out) {
  Result result = CheckIndex(type_var, type_fields_.NumTypes(), "struct type");
  if (Failed(result)) {
    return Result::Error;
  }

  Index index = type_var.index();
  assert(index < type_fields_.NumTypes());
  if (type_fields_.type_entries[index].kind != Type::StructRef) {
    return PrintError(type_var.loc, "type %d is not a struct type",
                      type_var.index());
  }

  *out_ref =
      Type(out_ref->IsNullableNonTypedRef() ? Type::RefNull : Type::Ref, index);
  index = type_fields_.type_entries[index].map_index;
  *out = type_fields_.struct_types[index];
  return Result::Ok;
}

Result SharedValidator::CheckArrayTypeIndex(Var type_var,
                                            Type* out_ref,
                                            TypeMut* out) {
  Result result = CheckIndex(type_var, type_fields_.NumTypes(), "array type");
  if (Failed(result)) {
    return Result::Error;
  }

  Index index = type_var.index();
  assert(index < type_fields_.NumTypes());
  if (type_fields_.type_entries[index].kind != Type::ArrayRef) {
    return PrintError(type_var.loc, "type %d is not an array type",
                      type_var.index());
  }

  *out_ref =
      Type(out_ref->IsNullableNonTypedRef() ? Type::RefNull : Type::Ref, index);
  index = type_fields_.type_entries[index].map_index;
  *out = type_fields_.array_types[index].field;
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

      if (index >= type_fields_.NumTypes()) {
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
  std::fill(local_ref_is_set_.begin(), local_ref_is_set_.end(), true);
}

Index SharedValidator::GetRecGroupEnd() {
  assert(options_.features.reference_types_enabled());
  Index num_types = type_fields_.NumTypes();

  if (options_.features.gc_enabled()) {
    return (last_rec_type_end_ > num_types) ? last_rec_type_end_ : num_types;
  }

  return num_types - 1;
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
  local_ref_is_set_.clear();
  local_refs_map_.clear();
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

  CHECK_RESULT(
      CheckReferenceType(loc, type, type_fields_.NumTypes(), "locals"));

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

Index SharedValidator::GetCanonicalTypeIndex(Index type_index) {
  if (type_index >= type_fields_.NumTypes()) {
    return kInvalidIndex;
  }

  if (Succeeded(type_validation_result_)) {
    return type_fields_.type_entries[type_index].canonical_index;
  }

  return type_index;
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
  if (options_.features.gc_enabled()) {
    if (opcode == Opcode::AnyConvertExtern || opcode == Opcode::ArrayNew ||
        opcode == Opcode::ArrayNewDefault || opcode == Opcode::ArrayNewFixed ||
        opcode == Opcode::ExternConvertAny || opcode == Opcode::RefI31 ||
        opcode == Opcode::StructNew || opcode == Opcode::StructNewDefault) {
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

Result SharedValidator::OnArrayCopy(const Location& loc,
                                    Var dst_type,
                                    Var src_type) {
  Result result = CheckInstr(Opcode::ArrayCopy, loc);
  Type dst_ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  Type src_ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut dst_array_type, src_array_type;
  result |= CheckArrayTypeIndex(dst_type, &dst_ref_type, &dst_array_type);
  result |= CheckArrayTypeIndex(src_type, &src_ref_type, &src_array_type);
  result |= typechecker_.OnArrayCopy(dst_ref_type, dst_array_type, src_ref_type,
                                     src_array_type.type);
  return result;
}

Result SharedValidator::OnArrayFill(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::ArrayFill, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= typechecker_.OnArrayFill(ref_type, array_type);
  return result;
}

Result SharedValidator::OnArrayGet(const Location& loc,
                                   Opcode opcode,
                                   Var type) {
  Result result = CheckInstr(opcode, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= typechecker_.OnArrayGet(opcode, ref_type, array_type.type);
  return result;
}

Result SharedValidator::OnArrayInitData(const Location& loc,
                                        Var type,
                                        Var segment_var) {
  Result result = CheckInstr(Opcode::ArrayInitData, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= CheckDataSegmentIndex(segment_var);
  result |= typechecker_.OnArrayInitData(ref_type, array_type);
  return result;
}

Result SharedValidator::OnArrayInitElem(const Location& loc,
                                        Var type,
                                        Var segment_var) {
  Result result = CheckInstr(Opcode::ArrayInitElem, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut array_type;
  ElemType elem_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= CheckElemSegmentIndex(segment_var, &elem_type);
  result |=
      typechecker_.OnArrayInitElem(ref_type, array_type, elem_type.element);
  return result;
}

Result SharedValidator::OnArrayNew(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::ArrayNew, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceNonNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= typechecker_.OnArrayNew(ref_type, array_type.type);
  return result;
}

Result SharedValidator::OnArrayNewData(const Location& loc,
                                       Var type,
                                       Var segment_var) {
  Result result = CheckInstr(Opcode::ArrayNewData, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceNonNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= CheckDataSegmentIndex(segment_var);
  result |= typechecker_.OnArrayNewData(ref_type, array_type.type);
  return result;
}

Result SharedValidator::OnArrayNewDefault(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::ArrayNewDefault, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceNonNull);
  TypeMut array_type;

  if (Succeeded(CheckArrayTypeIndex(type, &ref_type, &array_type))) {
    if (array_type.type.IsNonNullableRef()) {
      result = PrintError(loc, "array type has no default value: %" PRIindex,
                          type.index());
    }
  } else {
    result = Result::Error;
  }

  result |= typechecker_.OnArrayNewDefault(ref_type);
  return result;
}

Result SharedValidator::OnArrayNewElem(const Location& loc,
                                       Var type,
                                       Var segment_var) {
  Result result = CheckInstr(Opcode::ArrayNewElem, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceNonNull);
  TypeMut array_type;
  ElemType elem_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= CheckElemSegmentIndex(segment_var, &elem_type);
  result |=
      typechecker_.OnArrayNewElem(ref_type, array_type.type, elem_type.element);
  return result;
}

Result SharedValidator::OnArrayNewFixed(const Location& loc,
                                        Var type,
                                        Index count) {
  Result result = CheckInstr(Opcode::ArrayNewFixed, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceNonNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= typechecker_.OnArrayNewFixed(ref_type, array_type.type, count);
  return result;
}

Result SharedValidator::OnArraySet(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::ArraySet, loc);
  Type ref_type(Type::ArrayRef, Type::ReferenceOrNull);
  TypeMut array_type;
  result |= CheckArrayTypeIndex(type, &ref_type, &array_type);
  result |= typechecker_.OnArraySet(ref_type, array_type);
  return result;
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

Result SharedValidator::OnTernary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnTernary(opcode);
  return result;
}

Result SharedValidator::OnQuaternary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnQuaternary(opcode);
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

Result SharedValidator::OnBrOnCast(const Location& loc,
                                   Opcode opcode,
                                   Var depth,
                                   Var type1_var,
                                   Var type2_var) {
  Result result = CheckInstr(Opcode::BrOnCast, loc);
  result |= typechecker_.OnBrOnCast(opcode, depth.index(), type1_var.to_type(),
                                    type2_var.to_type());
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
  if (Failed(typechecker_.CheckType(table_type.element, Type::FuncRef))) {
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
  result |= CheckFuncTypeIndex(function_type_var, &func_type);
  result |= typechecker_.OnCallRef(function_type_var.to_type(),
                                   func_type.params, func_type.results);
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

Result SharedValidator::OnGCUnary(const Location& loc, Opcode opcode) {
  Result result = CheckInstr(opcode, loc);
  result |= typechecker_.OnGCUnary(opcode);
  return result;
}

Result SharedValidator::OnGlobalGet(const Location& loc, Var global_var) {
  Result result = CheckInstr(Opcode::GlobalGet, loc);
  GlobalType global_type;
  result |= CheckGlobalIndex(global_var, &global_type);
  result |= typechecker_.OnGlobalGet(global_type.type);
  if (Succeeded(result) && in_init_expr_) {
    if (global_var.index() >= last_initialized_global_) {
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
        !local_ref_is_set_[it->second.local_ref_is_set]) {
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
      local_ref_is_set_[it->second.local_ref_is_set] = true;
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
      local_ref_is_set_[it->second.local_ref_is_set] = true;
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

Result SharedValidator::OnRefCast(const Location& loc, Var type_var) {
  Result result = CheckInstr(Opcode::RefCast, loc);
  result |= typechecker_.OnRefCast(type_var.to_type());
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

  if (type == Type::RefNull) {
    result |=
        CheckIndex(func_type_var, type_fields_.NumTypes(), "function type");
  } else if (!type.IsNonTypedRef()) {
    result |= PrintError(loc, "Only nullable reference types are allowed");
  }

  assert(!Type::EnumIsNonTypedGCRef(type) || options_.features.gc_enabled());
  result |= typechecker_.OnRefNullExpr(type);
  return result;
}

Result SharedValidator::OnRefTest(const Location& loc, Var type_var) {
  Result result = CheckInstr(Opcode::RefTest, loc);
  result |= typechecker_.OnRefTest(type_var.to_type());
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
  result |= CheckFuncTypeIndex(function_type_var, &func_type);
  result |= typechecker_.OnReturnCallRef(function_type_var.to_type(),
                                         func_type.params, func_type.results);
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

      if (index >= type_fields_.NumTypes() ||
          type_fields_.type_entries[index].kind != Type::FuncRef) {
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

Result SharedValidator::OnStructGet(const Location& loc,
                                    Opcode opcode,
                                    Var type,
                                    Var field) {
  Result result = CheckInstr(opcode, loc);
  Type ref_type(Type::StructRef, Type::ReferenceOrNull);
  StructType struct_type;
  result |= CheckStructTypeIndex(type, &ref_type, &struct_type);
  result |=
      typechecker_.OnStructGet(opcode, ref_type, struct_type, field.index());
  return result;
}

Result SharedValidator::OnStructNew(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::StructNew, loc);
  Type ref_type(Type::StructRef, Type::ReferenceNonNull);
  StructType struct_type;
  result |= CheckStructTypeIndex(type, &ref_type, &struct_type);
  result |= typechecker_.OnStructNew(ref_type, struct_type);
  return result;
}

Result SharedValidator::OnStructNewDefault(const Location& loc, Var type) {
  Result result = CheckInstr(Opcode::StructNewDefault, loc);
  Type ref_type(Type::StructRef, Type::ReferenceNonNull);
  StructType struct_type;

  if (Succeeded(CheckStructTypeIndex(type, &ref_type, &struct_type))) {
    for (auto it : struct_type.fields) {
      if (it.type.IsNonNullableRef()) {
        result =
            PrintError(loc, "type has field without default value: %" PRIindex,
                       type.index());
        break;
      }
    }
  } else {
    result = Result::Error;
  }

  result |= typechecker_.OnStructNewDefault(ref_type);
  return result;
}

Result SharedValidator::OnStructSet(const Location& loc, Var type, Var field) {
  Result result = CheckInstr(Opcode::StructSet, loc);
  Type ref_type(Type::StructRef, Type::ReferenceOrNull);
  StructType struct_type;
  result |= CheckStructTypeIndex(type, &ref_type, &struct_type);
  result |= typechecker_.OnStructSet(ref_type, struct_type, field.index());
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
  SaveLocalRefs();
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

SharedComponentValidator::SharedComponentValidator(
    Errors* errors,
    std::string_view filename,
    const ValidateOptions& options)
    : options_(options),
      errors_(errors),
      filename_(filename),
      string_table_(&string_list_) {
  auto component = std::make_unique<Component>(nullptr);
  current_ = component.get();
  objects_.push_back(std::move(component));
}

Result WABT_PRINTF_FORMAT(3, 4)
    SharedComponentValidator::PrintError(const Location& loc,
                                         const char* format,
                                         ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  errors_->emplace_back(ErrorLevel::Error, loc, filename_, buffer);
  return Result::Error;
}

Result SharedComponentValidator::BeginComponent() {
  Component* component = CurrentAsComponent();
  auto new_component = std::make_unique<Component>(component);
  CurrentAsComponent()->components.push_back(new_component.get());
  current_ = new_component.get();
  objects_.push_back(std::move(new_component));
  return Result::Ok;
}

Result SharedComponentValidator::EndComponent() {
  assert(current_ != nullptr);
  current_ = current_->parent;
  return Result::Ok;
}

Result SharedComponentValidator::OnCoreInstance(
    const ComponentIndexLoc& module_index,
    uint32_t argument_count) {
  assert(caseful_names_.empty());
  argument_count_ = argument_count;
  return Result::Ok;
}

Result SharedComponentValidator::OnCoreInstanceArg(
    const ComponentStringLoc& name,
    ComponentSort sort,
    const ComponentIndexLoc& index) {
  Result result = Result::Ok;
  const std::string* import_name = string_table_.Append(name.str);

  std::map<const std::string*, Index>::iterator it =
      caseful_names_.find(import_name);
  if (it != caseful_names_.end()) {
    if (it->second == kInvalidIndex) {
      result |=
          PrintError(name.loc, "with \"" PRIstringview "\" assigned twice.",
                     WABT_PRINTF_STRING_VIEW_ARG(name.str));
    }
    it->second = kInvalidIndex;
  } else {
    // First assignment is ignored even if the name is not needed.
    caseful_names_[import_name] = kInvalidIndex;
  }

  argument_count_--;
  if (argument_count_ == 0) {
    caseful_names_.clear();
  }
  return result;
}

Result SharedComponentValidator::OnInlineCoreInstance(uint32_t argument_count) {
  assert(caseful_names_.empty());
  argument_count_ = argument_count;
  return Result::Ok;
}

Result SharedComponentValidator::OnInlineCoreInstanceArg(
    const ComponentStringLoc& name,
    ComponentSort sort,
    const ComponentIndexLoc& index) {
  Result result = Result::Ok;
  const std::string* import_name = string_table_.Append(name.str);

  std::map<const std::string*, Index>::iterator it =
      caseful_names_.find(import_name);
  if (it != caseful_names_.end()) {
    result |=
        PrintError(name.loc, "export \"" PRIstringview "\" assigned twice.",
                   WABT_PRINTF_STRING_VIEW_ARG(name.str));
  } else {
    caseful_names_[import_name] = kInvalidIndex;
  }

  argument_count_--;
  if (argument_count_ == 0) {
    caseful_names_.clear();
  }
  return result;
}

Result SharedComponentValidator::OnInstance(
    const ComponentIndexLoc& component_index,
    uint32_t argument_count) {
  TypeBase* type_base = nullptr;
  Result result =
      CheckIndex(ComponentSort::Component, component_index, &type_base);

  argument_count_ = argument_count;
  current_loc_ = component_index.loc;

  if (type_base != nullptr) {
    assert(caseful_names_.empty());
    TypeExternalList* external_list = type_base->AsTypeExternalList();
    Index size = static_cast<Index>(external_list->imports.size());
    not_found_count_ = size;

    if (argument_count == 0) {
      if (not_found_count_ > 0) {
        result |= PrintError(current_loc_, "not all imports are satisfied.");
      }
    } else {
      for (Index i = 0; i < size; i++) {
        caseful_names_[external_list->imports[i].name] = i;
      }
    }
  } else {
    not_found_count_ = 0;
  }

  CurrentAsComponent()->instances.push_back(type_base);
  return result;
}

Result SharedComponentValidator::OnInstanceArg(const ComponentStringLoc& name,
                                               ComponentSort sort,
                                               const ComponentIndexLoc& index) {
  Result result = Result::Ok;
  const std::string* import_name = string_table_.Append(name.str);
  Index import_index = kInvalidIndex;

  std::map<const std::string*, Index>::iterator it =
      caseful_names_.find(import_name);
  if (it != caseful_names_.end()) {
    import_index = it->second;
    if (import_index == kInvalidIndex) {
      result |=
          PrintError(name.loc, "with \"" PRIstringview "\" assigned twice.",
                     WABT_PRINTF_STRING_VIEW_ARG(name.str));
    }
    it->second = kInvalidIndex;
  } else {
    // First assignment is ignored even if the name is not needed.
    caseful_names_[import_name] = kInvalidIndex;
  }

  if (import_index != kInvalidIndex) {
    not_found_count_--;

    TypeExternalList* external_list =
        CurrentAsComponent()->instances.back()->AsTypeExternalList();
    ComponentSort import_sort = external_list->imports[import_index].sort;

    if (sort != import_sort) {
      result |= PrintError(index.loc,
                           "expected import sort (%s) does not match to (%s)",
                           import_sort.GetName(), sort.GetName());
    }
  }

  argument_count_--;
  if (argument_count_ == 0) {
    if (not_found_count_ > 0) {
      result |= PrintError(current_loc_, "not all imports are satisfied.");
    }
    caseful_names_.clear();
  }
  return Result::Ok;
}

Result SharedComponentValidator::OnInlineInstance() {
  auto type_value =
      std::make_unique<TypeExternalList>(ComponentTypeDef::Instance);
  CurrentAsComponent()->instances.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return Result::Ok;
}

Result SharedComponentValidator::OnInlineInstanceArg(
    const ComponentStringLoc& name,
    bool has_version_suffix,
    std::string_view version_suffix,
    ComponentSort sort,
    const ComponentIndexLoc& index) {
  const std::string* export_name = string_table_.Append(name.str);
  TypeBase* type_base = nullptr;
  CheckMode mode = sort == ComponentSort::Instance ? ExcludeLast : IncludeLast;
  Result result = CheckIndex(index.loc, sort, index.index, mode, &type_base);
  TypeExternalList* instance =
      CurrentAsComponent()->instances.back()->AsTypeExternalList();
  instance->exports.push_back(
      TypeExternalList::External{export_name, sort, type_base});
  return result;
}

Result SharedComponentValidator::OnAliasExport(
    ComponentSort sort,
    const ComponentIndexLoc& instance_index,
    const ComponentStringLoc& name) {
  TypeBase* type_base = nullptr;
  Result result =
      CheckIndex(ComponentSort::Instance, instance_index, &type_base);
  if (type_base == nullptr) {
    return result;
  }

  TypeExternalList* instance = type_base->AsTypeExternalList();
  const std::string* export_name = string_table_.Find(name.str);
  bool found = false;
  type_base = nullptr;
  if (export_name != nullptr) {
    for (auto& item : instance->exports) {
      if (item.name == export_name) {
        found = true;
        type_base = item.type_base;
        break;
      }
    }
  }

  if (!found) {
    result |= PrintError(name.loc, "export \"" PRIstringview "\" not found.",
                         WABT_PRINTF_STRING_VIEW_ARG(name.str));
  }

  TypeBaseVector* type_vector = GetSort(current_, sort);
  if (type_vector != nullptr) {
    type_vector->push_back(type_base);
  }
  return result;
}

Result SharedComponentValidator::OnAliasCoreExport(
    ComponentSort sort,
    const ComponentIndexLoc& core_instance_index,
    const ComponentStringLoc& name) {
  return Result::Ok;
}

Result SharedComponentValidator::OnAliasOuter(const Location& loc,
                                              ComponentSort sort,
                                              uint32_t counter,
                                              uint32_t index) {
  TypeDefList* target = current_;
  bool exclude_last = false;
  while (counter > 0 && target != nullptr) {
    if ((target->info_bits & IsObject) != 0) {
      exclude_last = (sort == ComponentSort::Component);
    } else {
      exclude_last = (sort == ComponentSort::Type);
    }
    target = target->parent;
    counter--;
  }

  TypeBaseVector* current_sort_vector = GetSort(current_, sort);
  if (target == nullptr) {
    if (current_sort_vector != nullptr) {
      current_sort_vector->push_back(nullptr);
    }

    return PrintError(loc, "invalid outer target");
  }

  TypeBaseVector* target_sort_vector = GetSort(target, sort);
  if (target_sort_vector == nullptr) {
    return Result::Ok;
  }

  size_t size = target_sort_vector->size();
  if (exclude_last) {
    assert(size > 0);
    size--;
  }

  Result result = Result::Ok;
  TypeBase* type_base;

  if (index >= size) {
    result |= PrintError(
        loc, "alias outer index (%" PRIindex ") must be less than %" PRIindex,
        index, static_cast<Index>(size));
    type_base = nullptr;
  } else {
    type_base = (*target_sort_vector)[index];
    if (type_base != nullptr && (type_base->info_bits & HasResource) != 0) {
      result |= PrintError(loc, "resource type is rejected by outer alias");
      type_base = nullptr;
    }
  }

  assert(current_sort_vector != nullptr);
  current_sort_vector->push_back(type_base);
  return result;
}

Result SharedComponentValidator::OnPrimitiveType(const ComponentType& type) {
  assert(!type.IsIndex() && !type.IsNone());
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::ValueType, type.GetType());
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return Result::Ok;
}

Result SharedComponentValidator::OnRecordType(const Location& loc,
                                              uint32_t field_count) {
  Result result = Result::Ok;
  if (field_count == 0) {
    result |= PrintError(loc, "record type must have at least one field.");
  }
  auto type_value = std::make_unique<TypeItems>(ComponentTypeDef::Record);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnRecordField(
    const ComponentStringLoc& field_name,
    const ComponentTypeLoc& field_type) {
  TypeRef type_ref;
  Result result = CheckType(field_type, ExcludeLast, &type_ref);
  TypeItems::Item item{string_table_.Append(field_name.str), type_ref};
  UpdateTypeInfo(type_ref);
  current_->types.back()->AsTypeItems()->items.push_back(item);
  return result;
}

Result SharedComponentValidator::OnVariantType(const Location& loc,
                                               uint32_t case_count) {
  Result result = Result::Ok;
  if (case_count == 0) {
    result |= PrintError(loc, "variant type must have at least one case.");
  }
  auto type_value = std::make_unique<TypeItems>(ComponentTypeDef::Variant);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnVariantCase(
    const ComponentStringLoc& case_name,
    const ComponentTypeLoc& case_type) {
  TypeRef type_ref;
  Result result = CheckType(case_type, ExcludeLast, &type_ref);
  TypeItems::Item item{string_table_.Append(case_name.str), type_ref};
  UpdateTypeInfo(type_ref);
  current_->types.back()->AsTypeItems()->items.push_back(item);
  return result;
}

Result SharedComponentValidator::OnListType(const ComponentTypeLoc& type) {
  TypeRef type_ref;
  Result result = CheckType(type, IncludeLast, &type_ref);
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::List, type_ref);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnListFixedType(const Location& loc,
                                                 const ComponentTypeLoc& type,
                                                 uint32_t size) {
  TypeRef type_ref;
  Result result = Result::Ok;
  if (size == 0) {
    result |= PrintError(loc, "size of fixed list must be greater than 0.");
  }
  result |= CheckType(type, IncludeLast, &type_ref);
  auto type_value = std::make_unique<TypeListFixed>(type_ref, size);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnTupleType(const Location& loc,
                                             uint32_t type_count) {
  Result result = Result::Ok;
  if (type_count == 0) {
    result |= PrintError(loc, "tuple type must have at least one item.");
  }
  auto type_value = std::make_unique<TypeTuple>();
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnTupleItem(const ComponentTypeLoc& item) {
  TypeRef type_ref;
  Result result = CheckType(item, ExcludeLast, &type_ref);
  UpdateTypeInfo(type_ref);
  current_->types.back()->AsTypeTuple()->items.push_back(type_ref);
  return result;
}

Result SharedComponentValidator::OnFlagsType(const Location& loc,
                                             uint32_t flag_count) {
  Result result = Result::Ok;
  if (flag_count == 0 || flag_count > 32) {
    result |=
        PrintError(loc, "number of labels must be within the 1..32 range.");
  }
  auto type_value = std::make_unique<TypeLabels>(ComponentTypeDef::Flags);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnFlagsLabel(const ComponentStringLoc& label) {
  const std::string* item = string_table_.Append(label.str);
  current_->types.back()->AsTypeLabels()->items.push_back(item);
  return Result::Ok;
}

Result SharedComponentValidator::OnEnumType(const Location& loc,
                                            uint32_t enum_count) {
  Result result = Result::Ok;
  if (enum_count == 0) {
    result |= PrintError(loc, "enum type must have at least one label.");
  }
  auto type_value = std::make_unique<TypeLabels>(ComponentTypeDef::Enum);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnEnumLabel(const ComponentStringLoc& label) {
  const std::string* item = string_table_.Append(label.str);
  current_->types.back()->AsTypeLabels()->items.push_back(item);
  return Result::Ok;
}

Result SharedComponentValidator::OnOptionType(const ComponentTypeLoc& type) {
  TypeRef type_ref;
  Result result = CheckType(type, IncludeLast, &type_ref);
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::Option, type_ref);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnResultType(
    const ComponentTypeLoc& result_type,
    const ComponentTypeLoc& error_type) {
  TypeRef result_ref, error_ref;
  Result result = CheckType(result_type, IncludeLast, &result_ref);
  result |= CheckType(error_type, IncludeLast, &error_ref);
  auto type_value = std::make_unique<ValueTypePair>(ComponentTypeDef::Result,
                                                    result_ref, error_ref);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnOwnType(const ComponentIndexLoc& index) {
  TypeBase* type_base = nullptr;
  Result result = CheckIndex(index.loc, ComponentSort::Type, index.index,
                             IncludeLast, &type_base);
  result |= CheckResource(index.loc, &type_base);
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::Own, TypeRef(type_base));
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnBorrowType(const ComponentIndexLoc& index) {
  TypeBase* type_base = nullptr;
  Result result = CheckIndex(index.loc, ComponentSort::Type, index.index,
                             IncludeLast, &type_base);
  result |= CheckResource(index.loc, &type_base);
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::Borrow, TypeRef(type_base));
  type_value->info_bits |= HasBorrow;
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnStreamType(const ComponentTypeLoc& type) {
  TypeRef type_ref;
  Result result = CheckType(type, IncludeLast, &type_ref);
  result |= CheckBorrow(type.loc, &type_ref, "stream");
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::Stream, type_ref);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnFutureType(const ComponentTypeLoc& type) {
  TypeRef type_ref;
  Result result = CheckType(type, IncludeLast, &type_ref);
  result |= CheckBorrow(type.loc, &type_ref, "future");
  auto type_value =
      std::make_unique<ValueType>(ComponentTypeDef::Future, type_ref);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnFuncType(ComponentTypeDef type,
                                            uint32_t param_count) {
  auto type_value = std::make_unique<TypeFunc>(type);
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return Result::Ok;
}

Result SharedComponentValidator::OnFuncParam(ComponentStringLoc name,
                                             ComponentTypeLoc type) {
  TypeRef type_ref;
  Result result = CheckType(type, ExcludeLast, &type_ref);
  TypeFunc::Param param{string_table_.Append(name.str), type_ref};
  UpdateTypeInfo(type_ref);
  current_->types.back()->AsTypeFunc()->params.push_back(param);
  return result;
}

Result SharedComponentValidator::OnFuncResult(const ComponentTypeLoc& type) {
  TypeRef type_ref;
  Result result = CheckType(type, ExcludeLast, &type_ref);
  result |= CheckBorrow(type.loc, &type_ref, "func result");
  UpdateTypeInfo(type_ref);
  current_->types.back()->AsTypeFunc()->result = type_ref;
  return result;
}

Result SharedComponentValidator::OnResourceType(const Location& loc,
                                                ComponentResourceRep rep,
                                                const ComponentIndexLoc& dtor) {
  Result result = Result::Ok;
  if ((current_->info_bits & IsObject) == 0) {
    result |=
        PrintError(loc, "resources can only be defined in concrete components");
  }
  auto type_value = std::make_unique<TypeBase>(ComponentTypeDef::Resource);
  type_value->info_bits |= HasResource;
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::OnResourceAsyncType(
    const Location& loc,
    ComponentResourceRep rep,
    const ComponentIndexLoc& dtor,
    const ComponentIndexLoc& callback) {
  Result result = Result::Ok;
  if ((current_->info_bits & IsObject) == 0) {
    result |=
        PrintError(loc, "resources can only be defined in concrete components");
  }
  auto type_value = std::make_unique<TypeBase>(ComponentTypeDef::ResourceAsync);
  type_value->info_bits |= HasResource;
  current_->types.push_back(type_value.get());
  objects_.push_back(std::move(type_value));
  return result;
}

Result SharedComponentValidator::BeginInstanceType(uint32_t count) {
  auto new_instance =
      std::make_unique<TypeDefList>(ComponentTypeDef::Instance, current_);
  TypeDefList* parent = current_;
  current_ = new_instance.get();
  parent->types.push_back(current_);
  objects_.push_back(std::move(new_instance));
  return Result::Ok;
}

Result SharedComponentValidator::EndInstanceType() {
  assert(current_ != nullptr);
  current_ = current_->parent;
  return Result::Ok;
}

Result SharedComponentValidator::BeginComponentType(uint32_t count) {
  auto new_component =
      std::make_unique<TypeDefList>(ComponentTypeDef::Component, current_);
  TypeDefList* parent = current_;
  current_ = new_component.get();
  parent->types.push_back(current_);
  objects_.push_back(std::move(new_component));
  return Result::Ok;
}

Result SharedComponentValidator::EndComponentType() {
  assert(current_ != nullptr);
  current_ = current_->parent;
  return Result::Ok;
}

Result SharedComponentValidator::OnCanonLift(
    const ComponentIndexLoc& core_func_index,
    uint32_t option_count,
    const ComponentCanonOption* options,
    const ComponentIndexLoc& type_index) {
  Result result = Result::Ok;
  result |= CheckCanonOptions(option_count, options);
  return result;
}

Result SharedComponentValidator::OnCanonLower(
    const ComponentIndexLoc& func_index,
    uint32_t option_count,
    const ComponentCanonOption* options) {
  Result result = Result::Ok;
  result |= CheckCanonOptions(option_count, options);
  return result;
}

Result SharedComponentValidator::OnCanonType(
    ComponentCanon canon,
    const ComponentIndexLoc& type_index) {
  return Result::Ok;
}

Result SharedComponentValidator::OnImport(
    const ComponentStringLoc& name,
    std::string_view* version_suffix,
    const ComponentExternalInfo& external_info) {
  Result result = Result::Ok;
  const std::string* import_name = string_table_.Append(name.str);
  TypeBase* type_base = nullptr;
  ComponentSort sort = external_info.sort;

  result |= CheckExternalInfo(external_info, &type_base);

  std::vector<TypeBase*>* sort_vector = GetSort(current_, sort);
  if (sort_vector != nullptr) {
    sort_vector->push_back(type_base);
  }
  current_->imports.push_back(
      TypeExternalList::External{import_name, sort, type_base});
  return Result::Ok;
}

Result SharedComponentValidator::OnExport(const ComponentStringLoc& name,
                                          std::string_view* version_suffix,
                                          ComponentExternalInfo* external_info,
                                          ComponentExportInfo* export_info) {
  Result result = Result::Ok;
  const std::string* export_name = string_table_.Append(name.str);
  TypeBase* type_base = nullptr;
  ComponentSort sort;

  if (external_info != nullptr) {
    sort = external_info->sort;
    if (export_info != nullptr && sort != export_info->sort) {
      result |= PrintError(export_info->index.loc,
                           "expected export sort (%s) does not match to (%s)",
                           export_info->sort.GetName(), sort.GetName());
    }
    result |= CheckExternalInfo(*external_info, &type_base);
  } else {
    sort = export_info->sort;
    result |= CheckIndex(export_info->index.loc, sort, export_info->index.index,
                         IncludeLast, &type_base);
  }

  std::vector<TypeBase*>* sort_vector = GetSort(current_, sort);
  if (sort_vector != nullptr) {
    sort_vector->push_back(type_base);
  }
  current_->exports.push_back(
      TypeExternalList::External{export_name, sort, type_base});
  return Result::Ok;
}

SharedComponentValidator::TypeBaseVector* SharedComponentValidator::GetSort(
    TypeDefList* def_list,
    ComponentSort sort) {
  if (sort == ComponentSort::Type) {
    return &def_list->types;
  }

  if ((def_list->info_bits & IsObject) == 0) {
    return nullptr;
  }

  Component* component = reinterpret_cast<Component*>(def_list);

  switch (sort) {
    case ComponentSort::Instance:
      return &component->instances;
    case ComponentSort::Component:
      return &component->components;
    default:
      // TODO: implement more checks.
      return nullptr;
  }
}

Result SharedComponentValidator::CheckIndex(const Location& loc,
                                            ComponentSort sort,
                                            Index index,
                                            CheckMode mode,
                                            TypeBase** out_type) {
  std::vector<TypeBase*>* sort_vector = GetSort(current_, sort);
  if (sort_vector == nullptr) {
    return Result::Ok;
  }

  Index max = static_cast<Index>(sort_vector->size());
  if (mode == ExcludeLast) {
    assert(max > 0);
    max--;
  }

  if (index >= max) {
    return PrintError(loc,
                      "type index (%" PRIindex ") must be less than %" PRIindex,
                      index, max);
  }
  *out_type = (*sort_vector)[index];
  return Result::Ok;
}

Result SharedComponentValidator::CheckIndex(ComponentSort sort,
                                            const ComponentIndexLoc& index,
                                            TypeBase** out_type) {
  return CheckIndex(index.loc, sort, index.index, IncludeLast, out_type);
}

Result SharedComponentValidator::CheckIndex(const ComponentIndexLoc& index,
                                            TypeRef* out_type_ref) {
  TypeBase* type_base;
  CHECK_RESULT(CheckIndex(index.loc, ComponentSort::Type, index.index,
                          IncludeLast, &type_base));
  Result result = CheckDefValType(index.loc, type_base);
  *out_type_ref = TypeRef(type_base);
  return result;
}

Result SharedComponentValidator::CheckType(const ComponentTypeLoc& type,
                                           CheckMode mode,
                                           TypeRef* out_type_ref) {
  if (!type.type.IsIndex()) {
    *out_type_ref = TypeRef(type.type.GetType());
    return Result::Ok;
  }
  TypeBase* type_base;
  CHECK_RESULT(CheckIndex(type.loc, ComponentSort::Type, type.type.GetIndex(),
                          mode, &type_base));
  Result result = CheckDefValType(type.loc, type_base);
  *out_type_ref = TypeRef(type_base);
  return result;
}

Result SharedComponentValidator::CheckDefValType(const Location& loc,
                                                 const TypeBase* type) {
  if (type != nullptr && !type->IsValueType() && !type->IsTypeItems() &&
      !type->IsTypeLabels() && type->type_def != ComponentTypeDef::Tuple &&
      type->type_def != ComponentTypeDef::Result) {
    return PrintError(loc, "Type must be defined value type");
  }
  return Result::Ok;
}

Result SharedComponentValidator::CheckResource(const Location& loc,
                                               TypeBase** type) {
  if (*type != nullptr && (*type)->type_def != ComponentTypeDef::Resource) {
    *type = nullptr;
    return PrintError(loc, "type must be resource");
  }
  return Result::Ok;
}

Result SharedComponentValidator::CheckBorrow(const Location& loc,
                                             TypeRef* type_ref,
                                             const char* desc) {
  if (type_ref->ref != nullptr && (type_ref->ref->info_bits & HasBorrow) != 0) {
    type_ref->ref = nullptr;
    return PrintError(loc, "resource borrow is not allowed in %s", desc);
  }
  return Result::Ok;
}

Result SharedComponentValidator::CheckCanonOptions(
    uint32_t option_count,
    const ComponentCanonOption* options) {
  bool seen_string_encoding = false;
  const ComponentCanonOption* memory = nullptr;
  const ComponentCanonOption* realloc = nullptr;
  const ComponentCanonOption* post_return = nullptr;
  bool seen_async = false;
  const ComponentCanonOption* callback = nullptr;

  while (option_count > 0) {
    switch (options->option) {
      case ComponentCanonOption::StrEncUtf8:
      case ComponentCanonOption::StrEncUtf16:
      case ComponentCanonOption::StrEncLatin1Utf16:
        if (seen_string_encoding) {
          return PrintError(options->loc, "duplicated string encoding option.");
        }
        seen_string_encoding = true;
        break;
      case ComponentCanonOption::Memory:
        if (memory != nullptr) {
          return PrintError(options->loc, "duplicated memory option.");
        }
        memory = options;
        break;
      case ComponentCanonOption::Realloc:
        if (realloc != nullptr) {
          return PrintError(options->loc, "duplicated realloc option.");
        }
        realloc = options;
        break;
      case ComponentCanonOption::PostReturn:
        if (post_return != nullptr) {
          return PrintError(options->loc, "duplicated post-return option.");
        }
        post_return = options;
        break;
      case ComponentCanonOption::Async:
        if (seen_async) {
          return PrintError(options->loc, "duplicated async option.");
        }
        seen_async = true;
        break;
      case ComponentCanonOption::Callback:
        if (callback != nullptr) {
          return PrintError(options->loc, "duplicated callback option.");
        }
        callback = options;
        break;
    }
    options++;
    option_count--;
  }

  if (realloc != nullptr && memory == nullptr) {
    return PrintError(realloc->loc,
                      "realloc option is present without memory option.");
  }
  if (callback != nullptr && !seen_async) {
    return PrintError(callback->loc,
                      "callback option is present without async option.");
  }
  if (seen_async && post_return != nullptr) {
    return PrintError(post_return->loc,
                      "both async and post-return options cannot be present.");
  }
  return Result::Ok;
}

Result SharedComponentValidator::CheckExternalInfo(
    const ComponentExternalInfo& external_info,
    TypeBase** out_type_base) {
  ComponentSort sort = external_info.sort;
  if (sort == ComponentSort::Type &&
      external_info.external == ComponentExternalDesc::TypeSubRes) {
    auto type_value = std::make_unique<TypeBase>(ComponentTypeDef::Resource);
    *out_type_base = type_value.get();
    objects_.push_back(std::move(type_value));
    return Result::Ok;
  }

  TypeBase* type_base = nullptr;
  Result result =
      CheckIndex(external_info.index.loc, ComponentSort::Type,
                 external_info.index.index, IncludeLast, &type_base);
  *out_type_base = type_base;

  if (type_base != nullptr) {
    switch (sort) {
      case ComponentSort::Func:
        if (!type_base->IsTypeFunc()) {
          result |=
              PrintError(external_info.index.loc, "type is not a function");
        }
        break;
      case ComponentSort::Component:
        if (type_base->type_def != ComponentTypeDef::Component) {
          result |=
              PrintError(external_info.index.loc, "type is not a component");
        }
        break;
      case ComponentSort::Instance:
        if (type_base->type_def != ComponentTypeDef::Instance) {
          result |=
              PrintError(external_info.index.loc, "type is not an instance");
        }
        break;
      default:
        break;
    }
  }
  return result;
}

}  // namespace wabt
