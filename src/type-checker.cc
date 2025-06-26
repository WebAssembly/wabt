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

#include "wabt/type-checker.h"

#include <cinttypes>

namespace wabt {

namespace {

std::string TypesToString(const TypeVector& types,
                          const char* prefix = nullptr) {
  std::string result = "[";
  if (prefix) {
    result += prefix;
  }

  for (size_t i = 0; i < types.size(); ++i) {
    Type ty = types[i];
    // NOTE: Reference (and GetName) is also used by (e.g.) objdump, which does
    // not apply validation. do this here so as to not break that.
    if (ty.IsReferenceWithIndex() && ty.GetReferenceIndex() == kInvalidIndex) {
      result += "reference";
    } else {
      result += types[i].GetName();
    }
    if (i < types.size() - 1) {
      result += ", ";
    }
  }
  result += "]";
  return result;
}

}  // end anonymous namespace

TypeChecker::Label::Label(LabelType label_type,
                          const TypeVector& param_types,
                          const TypeVector& result_types,
                          size_t limit)
    : label_type(label_type),
      param_types(param_types),
      result_types(result_types),
      type_stack_limit(limit),
      unreachable(false) {}

void TypeChecker::PrintError(const char* fmt, ...) {
  if (error_callback_) {
    WABT_SNPRINTF_ALLOCA(buffer, length, fmt);
    error_callback_(buffer);
  }
}

Result TypeChecker::GetLabel(Index depth, Label** out_label) {
  if (depth >= label_stack_.size()) {
    assert(label_stack_.size() > 0);
    PrintError("invalid depth: %" PRIindex " (max %" PRIzd ")", depth,
               label_stack_.size() - 1);
    *out_label = nullptr;
    return Result::Error;
  }
  *out_label = &label_stack_[label_stack_.size() - depth - 1];
  return Result::Ok;
}

Result TypeChecker::GetRethrowLabel(Index depth, Label** out_label) {
  if (Failed(GetLabel(depth, out_label))) {
    return Result::Error;
  }

  if ((*out_label)->label_type == LabelType::Catch) {
    return Result::Ok;
  }

  std::string candidates;
  for (Index idx = 0; idx < label_stack_.size(); idx++) {
    LabelType type = label_stack_[label_stack_.size() - idx - 1].label_type;
    if (type == LabelType::Catch) {
      if (!candidates.empty()) {
        candidates.append(", ");
      }
      candidates.append(std::to_string(idx));
    }
  }

  if (candidates.empty()) {
    PrintError("rethrow not in try catch block");
  } else {
    PrintError("invalid rethrow depth: %" PRIindex " (catches: %s)", depth,
               candidates.c_str());
  }
  *out_label = nullptr;
  return Result::Error;
}

Result TypeChecker::GetCatchCount(Index depth, Index* out_count) {
  Label* unused;
  if (Failed(GetLabel(depth, &unused))) {
    return Result::Error;
  }

  Index catch_count = 0;
  for (Index idx = 0; idx <= depth; idx++) {
    LabelType type = label_stack_[label_stack_.size() - idx - 1].label_type;
    if (type == LabelType::Catch) {
      catch_count++;
    }
  }
  *out_count = catch_count;

  return Result::Ok;
}

Result TypeChecker::TopLabel(Label** out_label) {
  return GetLabel(0, out_label);
}

bool TypeChecker::IsUnreachable() {
  Label* label;
  if (Failed(TopLabel(&label))) {
    return true;
  }
  return label->unreachable;
}

void TypeChecker::ResetTypeStackToLabel(Label* label) {
  type_stack_.resize(label->type_stack_limit);
}

Result TypeChecker::SetUnreachable() {
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  label->unreachable = true;
  ResetTypeStackToLabel(label);
  return Result::Ok;
}

void TypeChecker::PushLabel(LabelType label_type,
                            const TypeVector& param_types,
                            const TypeVector& result_types) {
  label_stack_.emplace_back(label_type, param_types, result_types,
                            type_stack_.size());
}

Result TypeChecker::PopLabel() {
  label_stack_.pop_back();
  return Result::Ok;
}

Result TypeChecker::CheckLabelType(Label* label, LabelType label_type) {
  return label->label_type == label_type ? Result::Ok : Result::Error;
}

Result TypeChecker::Check2LabelTypes(Label* label,
                                     LabelType label_type1,
                                     LabelType label_type2) {
  return label->label_type == label_type1 || label->label_type == label_type2
             ? Result::Ok
             : Result::Error;
}

Result TypeChecker::GetThisFunctionLabel(Label** label) {
  return GetLabel(label_stack_.size() - 1, label);
}

Result TypeChecker::PeekType(Index depth, Type* out_type) {
  Label* label;
  CHECK_RESULT(TopLabel(&label));

  if (label->type_stack_limit + depth >= type_stack_.size()) {
    *out_type = Type::Any;
    return label->unreachable ? Result::Ok : Result::Error;
  }
  *out_type = type_stack_[type_stack_.size() - depth - 1];
  return Result::Ok;
}

Result TypeChecker::PeekAndCheckType(Index depth, Type expected) {
  Type actual = Type::Any;
  Result result = PeekType(depth, &actual);
  return result | CheckType(actual, expected);
}

Result TypeChecker::DropTypes(size_t drop_count) {
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  if (label->type_stack_limit + drop_count > type_stack_.size()) {
    ResetTypeStackToLabel(label);
    return label->unreachable ? Result::Ok : Result::Error;
  }
  type_stack_.erase(type_stack_.end() - drop_count, type_stack_.end());
  return Result::Ok;
}

void TypeChecker::PushType(Type type) {
  if (type != Type::Void) {
    type_stack_.push_back(type);
  }
}

void TypeChecker::PushTypes(const TypeVector& types) {
  for (Type type : types) {
    PushType(type);
  }
}

Result TypeChecker::CheckTypeStackEnd(const char* desc) {
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  Result result = (type_stack_.size() == label->type_stack_limit)
                      ? Result::Ok
                      : Result::Error;
  PrintStackIfFailedV(result, desc, {}, /*is_end=*/true);
  return result;
}

static bool CompareTypeVector(
    const std::map<Index, TypeChecker::FuncType>& func_types,
    const TypeVector& left,
    const TypeVector& right) {
  size_t size = left.size();

  if (size != right.size()) {
    return false;
  }

  for (size_t i = 0; i < size; i++) {
    const Type& left_type = left[i];
    const Type& right_type = right[i];

    if (left_type != right_type) {
      if (!left_type.IsReferenceWithIndex() ||
          left_type != static_cast<Type::Enum>(right_type)) {
        return false;
      }

      const TypeChecker::FuncType& left_func_type =
          func_types.at(left_type.GetReferenceIndex());
      const TypeChecker::FuncType& right_func_type =
          func_types.at(right_type.GetReferenceIndex());

      // Circular references were checked during validation.
      if (!CompareTypeVector(func_types, left_func_type.params,
                             right_func_type.params) ||
          !CompareTypeVector(func_types, left_func_type.results,
                             right_func_type.results)) {
        return false;
      }
    }
  }

  return true;
}

Result TypeChecker::CheckType(Type actual, Type expected) {
  if (expected == Type::Any || actual == Type::Any) {
    return Result::Ok;
  }

  Type::Enum actual_type = actual;
  Type::Enum expected_type = expected;

  if (actual_type == expected_type) {
    switch (actual_type) {
      case Type::ExternRef:
      case Type::FuncRef:
        return (expected.IsNullableNonTypedRef() ||
                !actual.IsNullableNonTypedRef())
                   ? Result::Ok
                   : Result::Error;

      case Type::Reference:
      case Type::Ref:
      case Type::RefNull:
        break;

      default:
        return Result::Ok;
    }
  }

  if (!actual.IsReferenceWithIndex()) {
    return Result::Error;
  }

  if (expected_type == Type::FuncRef) {
    return (actual == Type::Ref || expected.IsNullableNonTypedRef())
               ? Result::Ok
               : Result::Error;
  }

  if (!expected.IsReferenceWithIndex()) {
    return Result::Error;
  }

  if (expected_type == Type::Ref && actual_type == Type::RefNull) {
    return Result::Error;
  }

  FuncType& actual_func_type = func_types_[actual.GetReferenceIndex()];
  FuncType& expected_func_type = func_types_[expected.GetReferenceIndex()];

  if (CompareTypeVector(func_types_, actual_func_type.params,
                        expected_func_type.params) &&
      CompareTypeVector(func_types_, actual_func_type.results,
                        expected_func_type.results)) {
    return Result::Ok;
  }

  return Result::Error;
}

Result TypeChecker::CheckTypes(const TypeVector& actual,
                               const TypeVector& expected) {
  if (actual.size() != expected.size()) {
    return Result::Error;
  } else {
    Result result = Result::Ok;
    for (size_t i = 0; i < actual.size(); i++)
      result |= CheckType(actual[i], expected[i]);
    return result;
  }
}

Result TypeChecker::CheckSignature(const TypeVector& sig, const char* desc) {
  Result result = Result::Ok;
  for (size_t i = 0; i < sig.size(); ++i) {
    result |= PeekAndCheckType(sig.size() - i - 1, sig[i]);
  }
  PrintStackIfFailedV(result, desc, sig, /*is_end=*/false);
  return result;
}

Result TypeChecker::CheckReturnSignature(const TypeVector& actual,
                                         const TypeVector& expected,
                                         const char* desc) {
  Result result = CheckTypes(actual, expected);
  if (Failed(result)) {
    PrintError("return signatures have inconsistent types: expected %s, got %s",
               TypesToString(expected).c_str(), TypesToString(actual).c_str());
  }
  return result;
}

Result TypeChecker::PopAndCheckSignature(const TypeVector& sig,
                                         const char* desc) {
  Result result = CheckSignature(sig, desc);
  result |= DropTypes(sig.size());
  return result;
}

Result TypeChecker::PopAndCheckCall(const TypeVector& param_types,
                                    const TypeVector& result_types,
                                    const char* desc) {
  Result result = CheckSignature(param_types, desc);
  result |= DropTypes(param_types.size());
  PushTypes(result_types);
  return result;
}

Result TypeChecker::PopAndCheck1Type(Type expected, const char* desc) {
  Result result = Result::Ok;
  result |= PeekAndCheckType(0, expected);
  PrintStackIfFailed(result, desc, expected);
  result |= DropTypes(1);
  return result;
}

Result TypeChecker::PopAndCheck2Types(Type expected1,
                                      Type expected2,
                                      const char* desc) {
  Result result = Result::Ok;
  result |= PeekAndCheckType(0, expected2);
  result |= PeekAndCheckType(1, expected1);
  PrintStackIfFailed(result, desc, expected1, expected2);
  result |= DropTypes(2);
  return result;
}

Result TypeChecker::PopAndCheck3Types(Type expected1,
                                      Type expected2,
                                      Type expected3,
                                      const char* desc) {
  Result result = Result::Ok;
  result |= PeekAndCheckType(0, expected3);
  result |= PeekAndCheckType(1, expected2);
  result |= PeekAndCheckType(2, expected1);
  PrintStackIfFailed(result, desc, expected1, expected2, expected3);
  result |= DropTypes(3);
  return result;
}

Result TypeChecker::PopAndCheckReference(Type* actual, const char* desc) {
  *actual = Type::Any;
  Result result = PeekType(0, actual);

  // Type::Any is a valid value for dead code, and replacing
  // it with anything might break the syntax checker.
  if (*actual != Type::Any && !actual->IsRef()) {
    result = Result::Error;
  }

  PrintStackIfFailed(result, desc, Type::FuncRef);
  result |= DropTypes(1);
  return result;
}

// Some paramater types depend on the memory being used.
// For example load/store operands, or memory.fill operands.
static Type GetMemoryParam(Type param, const Limits* limits) {
  return limits ? limits->IndexType() : param;
}

Result TypeChecker::CheckOpcode1(Opcode opcode, const Limits* limits) {
  Result result = PopAndCheck1Type(
      GetMemoryParam(opcode.GetParamType1(), limits), opcode.GetName());
  PushType(opcode.GetResultType());
  return result;
}

Result TypeChecker::CheckOpcode2(Opcode opcode, const Limits* limits) {
  Result result =
      PopAndCheck2Types(GetMemoryParam(opcode.GetParamType1(), limits),
                        opcode.GetParamType2(), opcode.GetName());
  PushType(opcode.GetResultType());
  return result;
}

Result TypeChecker::CheckOpcode3(Opcode opcode,
                                 const Limits* limits1,
                                 const Limits* limits2,
                                 const Limits* limits3) {
  Result result = PopAndCheck3Types(
      GetMemoryParam(opcode.GetParamType1(), limits1),
      GetMemoryParam(opcode.GetParamType2(), limits2),
      GetMemoryParam(opcode.GetParamType3(), limits3), opcode.GetName());
  PushType(opcode.GetResultType());
  return result;
}

void TypeChecker::PrintStackIfFailedV(Result result,
                                      const char* desc,
                                      const TypeVector& expected,
                                      bool is_end) {
  if (Succeeded(result)) {
    return;
  }

  size_t limit = 0;
  Label* label;
  if (Succeeded(TopLabel(&label))) {
    limit = label->type_stack_limit;
  }

  TypeVector actual;
  size_t max_depth = type_stack_.size() - limit;

  // In general we want to print as many values of the actual stack as were
  // expected. However, if the stack was expected to be empty, we should
  // print some amount of the actual stack.
  size_t actual_size;
  if (expected.size() == 0) {
    // Don't print too many elements if the stack is really deep.
    const size_t kMaxActualStackToPrint = 4;
    actual_size = std::min(kMaxActualStackToPrint, max_depth);
  } else {
    actual_size = std::min(expected.size(), max_depth);
  }

  bool incomplete_actual_stack = actual_size != max_depth;

  for (size_t i = 0; i < actual_size; ++i) {
    Type type;
    Result result = PeekType(actual_size - i - 1, &type);
    WABT_USE(result);
    assert(Succeeded(result));
    actual.push_back(type);
  }

  std::string message = "type mismatch in ";
  if (is_end) {
    message = "type mismatch at end of ";
  }
  message += desc;
  message += ", expected ";
  message += TypesToString(expected);
  message += " but got ";
  message += TypesToString(actual, incomplete_actual_stack ? "... " : nullptr);

  PrintError("%s", message.c_str());
}

Result TypeChecker::BeginFunction(const TypeVector& sig) {
  type_stack_.clear();
  label_stack_.clear();
  PushLabel(LabelType::Func, TypeVector(), sig);
  return Result::Ok;
}

Result TypeChecker::OnAtomicLoad(Opcode opcode, const Limits& limits) {
  return CheckOpcode1(opcode, &limits);
}

Result TypeChecker::OnAtomicStore(Opcode opcode, const Limits& limits) {
  return CheckOpcode2(opcode, &limits);
}

Result TypeChecker::OnAtomicRmw(Opcode opcode, const Limits& limits) {
  return CheckOpcode2(opcode, &limits);
}

Result TypeChecker::OnAtomicRmwCmpxchg(Opcode opcode, const Limits& limits) {
  return CheckOpcode3(opcode, &limits);
}

Result TypeChecker::OnAtomicWait(Opcode opcode, const Limits& limits) {
  return CheckOpcode3(opcode, &limits);
}

Result TypeChecker::OnAtomicFence(uint32_t consistency_model) {
  return Result::Ok;
}

Result TypeChecker::OnAtomicNotify(Opcode opcode, const Limits& limits) {
  return CheckOpcode2(opcode, &limits);
}

Result TypeChecker::OnBinary(Opcode opcode) {
  return CheckOpcode2(opcode);
}

Result TypeChecker::OnBlock(const TypeVector& param_types,
                            const TypeVector& result_types) {
  Result result = PopAndCheckSignature(param_types, "block");
  PushLabel(LabelType::Block, param_types, result_types);
  PushTypes(param_types);
  return result;
}

Result TypeChecker::OnBr(Index depth) {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  result |= CheckSignature(label->br_types(), "br");
  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnBrIf(Index depth) {
  Result result = PopAndCheck1Type(Type::I32, "br_if");
  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  result |= PopAndCheckSignature(label->br_types(), "br_if");
  PushTypes(label->br_types());
  return result;
}

static Type convertRefNullToRef(Type type) {
  if (type == Type::ExternRef || type == Type::FuncRef) {
    return Type(type, Type::ReferenceNonNull);
  }

  assert(type.IsReferenceWithIndex());
  return Type(Type::Ref, type.GetReferenceIndex());
}

Result TypeChecker::OnBrOnNonNull(Index depth) {
  Type actual;
  CHECK_RESULT(PopAndCheckReference(&actual, "br_on_non_null"));
  if (actual != Type::Any) {
    PushType(convertRefNullToRef(actual));
  }

  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  Result result = PopAndCheckSignature(label->br_types(), "br_on_non_null");
  PushTypes(label->br_types());

  if (actual != Type::Any) {
    result |= DropTypes(1);
  }
  return result;
}

Result TypeChecker::OnBrOnNull(Index depth) {
  Type actual;
  CHECK_RESULT(PopAndCheckReference(&actual, "br_on_null"));

  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  Result result = PopAndCheckSignature(label->br_types(), "br_on_null");
  PushTypes(label->br_types());

  if (actual != Type::Any) {
    actual = convertRefNullToRef(actual);
  }
  PushType(actual);
  return result;
}

Result TypeChecker::BeginBrTable() {
  br_table_sig_ = nullptr;
  return PopAndCheck1Type(Type::I32, "br_table");
}

Result TypeChecker::OnBrTableTarget(Index depth) {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  TypeVector& label_sig = label->br_types();
  result |= CheckSignature(label_sig, "br_table");

  // Make sure this label's signature is consistent with the previous labels'
  // signatures.
  if (br_table_sig_ == nullptr) {
    br_table_sig_ = &label_sig;
  } else {
    if (br_table_sig_->size() != label_sig.size()) {
      result |= Result::Error;
      PrintError("br_table labels have inconsistent types: expected %s, got %s",
                 TypesToString(*br_table_sig_).c_str(),
                 TypesToString(label_sig).c_str());
    }
  }

  return result;
}

Result TypeChecker::EndBrTable() {
  return SetUnreachable();
}

Result TypeChecker::OnCall(const TypeVector& param_types,
                           const TypeVector& result_types) {
  return PopAndCheckCall(param_types, result_types, "call");
}

Result TypeChecker::OnCallIndirect(const TypeVector& param_types,
                                   const TypeVector& result_types,
                                   const Limits& table_limits) {
  Result result = PopAndCheck1Type(table_limits.is_64 ? Type::I64 : Type::I32,
                                   "call_indirect");
  result |= PopAndCheckCall(param_types, result_types, "call_indirect");
  return result;
}

Result TypeChecker::OnCallRef(Type type) {
  return PopAndCheck1Type(type, "call_ref");
}

Result TypeChecker::OnReturnCall(const TypeVector& param_types,
                                 const TypeVector& result_types) {
  Result result = PopAndCheckSignature(param_types, "return_call");
  Label* func_label;
  CHECK_RESULT(GetThisFunctionLabel(&func_label));
  result |= CheckReturnSignature(result_types, func_label->result_types,
                                 "return_call");

  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnReturnCallIndirect(const TypeVector& param_types,
                                         const TypeVector& result_types) {
  Result result = PopAndCheck1Type(Type::I32, "return_call_indirect");

  result |= PopAndCheckSignature(param_types, "return_call_indirect");
  Label* func_label;
  CHECK_RESULT(GetThisFunctionLabel(&func_label));
  result |= CheckReturnSignature(result_types, func_label->result_types,
                                 "return_call_indirect");

  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnReturnCallRef(Type type) {
  return PopAndCheck1Type(type, "return_call_ref");
}

Result TypeChecker::OnCompare(Opcode opcode) {
  return CheckOpcode2(opcode);
}

Result TypeChecker::OnCatch(const TypeVector& sig) {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  result |= Check2LabelTypes(label, LabelType::Try, LabelType::Catch);
  result |= PopAndCheckSignature(label->result_types, "try block");
  result |= CheckTypeStackEnd("try block");
  ResetTypeStackToLabel(label);
  label->label_type = LabelType::Catch;
  label->unreachable = false;
  PushTypes(sig);
  return result;
}

Result TypeChecker::OnConst(Type type) {
  PushType(type);
  return Result::Ok;
}

Result TypeChecker::OnConvert(Opcode opcode) {
  return CheckOpcode1(opcode);
}

Result TypeChecker::OnDelegate(Index depth) {
  Result result = Result::Ok;
  Label* label;
  // Delegate starts counting after the current try, as the delegate
  // instruction is not actually in the try block.
  CHECK_RESULT(GetLabel(depth + 1, &label));

  Label* try_label;
  CHECK_RESULT(TopLabel(&try_label));
  result |= CheckLabelType(try_label, LabelType::Try);
  result |= PopAndCheckSignature(try_label->result_types, "try block");
  result |= CheckTypeStackEnd("try block");
  ResetTypeStackToLabel(try_label);

  // Since an end instruction does not follow a delegate, we push
  // the block results here and pop the label.
  PushTypes(try_label->result_types);
  PopLabel();
  return result;
}

Result TypeChecker::OnDrop() {
  Result result = Result::Ok;
  result |= DropTypes(1);
  PrintStackIfFailed(result, "drop", Type::Any);
  return result;
}

Result TypeChecker::OnElse() {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  result |= CheckLabelType(label, LabelType::If);
  result |= PopAndCheckSignature(label->result_types, "`if true` branch");
  result |= CheckTypeStackEnd("`if true` branch");
  ResetTypeStackToLabel(label);
  PushTypes(label->param_types);
  label->label_type = LabelType::Else;
  label->unreachable = false;
  return result;
}

Result TypeChecker::OnEnd(Label* label,
                          const char* sig_desc,
                          const char* end_desc) {
  Result result = Result::Ok;
  result |= PopAndCheckSignature(label->result_types, sig_desc);
  result |= CheckTypeStackEnd(end_desc);
  ResetTypeStackToLabel(label);
  PushTypes(label->result_types);
  PopLabel();
  return result;
}

Result TypeChecker::OnEnd() {
  Result result = Result::Ok;
  static const char* s_label_type_name[] = {
      "function", "initializer expression", "block", "loop",
      "if",       "`if false` branch",      "try",   "try table",
      "try catch"};
  WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(s_label_type_name) == kLabelTypeCount);
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  assert(static_cast<int>(label->label_type) < kLabelTypeCount);
  if (label->label_type == LabelType::If) {
    // An if without an else will just pass the params through, so the result
    // types must be the same as the param types. It has the same behavior as
    // an empty else block.
    CHECK_RESULT(OnElse());
  }

  const char* desc = s_label_type_name[static_cast<int>(label->label_type)];
  result |= OnEnd(label, desc, desc);
  return result;
}

Result TypeChecker::OnIf(const TypeVector& param_types,
                         const TypeVector& result_types) {
  Result result = PopAndCheck1Type(Type::I32, "if");
  result |= PopAndCheckSignature(param_types, "if");
  PushLabel(LabelType::If, param_types, result_types);
  PushTypes(param_types);
  return result;
}

Result TypeChecker::OnGlobalGet(Type type) {
  PushType(type);
  return Result::Ok;
}

Result TypeChecker::OnGlobalSet(Type type) {
  return PopAndCheck1Type(type, "global.set");
}

Result TypeChecker::OnLoad(Opcode opcode, const Limits& limits) {
  return CheckOpcode1(opcode, &limits);
}

Result TypeChecker::OnLocalGet(Type type) {
  PushType(type);
  return Result::Ok;
}

Result TypeChecker::OnLocalSet(Type type) {
  return PopAndCheck1Type(type, "local.set");
}

Result TypeChecker::OnLocalTee(Type type) {
  Result result = Result::Ok;
  result |= PopAndCheck1Type(type, "local.tee");
  PushType(type);
  return result;
}

Result TypeChecker::OnLoop(const TypeVector& param_types,
                           const TypeVector& result_types) {
  Result result = PopAndCheckSignature(param_types, "loop");
  PushLabel(LabelType::Loop, param_types, result_types);
  PushTypes(param_types);
  return result;
}

Result TypeChecker::OnMemoryCopy(const Limits& dst_limits,
                                 const Limits& src_limits) {
  Limits size_limits = src_limits;
  // The memory64 proposal specifies that the type of the size argument should
  // be the mimimum of the two memory types.
  if (src_limits.is_64 && !dst_limits.is_64) {
    size_limits = dst_limits;
  }
  return CheckOpcode3(Opcode::MemoryCopy, &dst_limits, &src_limits,
                      &size_limits);
}

Result TypeChecker::OnDataDrop(uint32_t segment) {
  return Result::Ok;
}

Result TypeChecker::OnMemoryFill(const Limits& limits) {
  return CheckOpcode3(Opcode::MemoryFill, &limits, nullptr, &limits);
}

Result TypeChecker::OnMemoryGrow(const Limits& limits) {
  Result result = PopAndCheck1Type(limits.IndexType(), "memory.grow");
  PushType(limits.IndexType());
  return result;
}

Result TypeChecker::OnMemoryInit(uint32_t segment, const Limits& limits) {
  return CheckOpcode3(Opcode::MemoryInit, &limits);
}

Result TypeChecker::OnMemorySize(const Limits& limits) {
  PushType(limits.IndexType());
  return Result::Ok;
}

Result TypeChecker::OnTableCopy(const Limits& dst_limits,
                                const Limits& src_limits) {
  Limits size_limits = src_limits;
  // The memory64 proposal specifies that the type of the size argument should
  // be the mimimum of the two table types.
  if (src_limits.is_64 && !dst_limits.is_64) {
    size_limits = dst_limits;
  }
  return CheckOpcode3(Opcode::TableCopy, &dst_limits, &src_limits,
                      &size_limits);
}

Result TypeChecker::OnElemDrop(uint32_t segment) {
  return Result::Ok;
}

Result TypeChecker::OnTableInit(uint32_t segment, const Limits& limits) {
  return CheckOpcode3(Opcode::TableInit, &limits);
}

Result TypeChecker::OnTableGet(Type elem_type, const Limits& limits) {
  Result result = CheckOpcode1(Opcode::TableGet, &limits);
  PushType(elem_type);
  return result;
}

Result TypeChecker::OnTableSet(Type elem_type, const Limits& limits) {
  return PopAndCheck2Types(limits.IndexType(), elem_type, "table.set");
}

Result TypeChecker::OnTableGrow(Type elem_type, const Limits& limits) {
  Result result =
      PopAndCheck2Types(elem_type, limits.IndexType(), "table.grow");
  PushType(limits.IndexType());
  return result;
}

Result TypeChecker::OnTableSize(const Limits& limits) {
  PushType(limits.IndexType());
  return Result::Ok;
}

Result TypeChecker::OnTableFill(Type elem_type, const Limits& limits) {
  return PopAndCheck3Types(limits.IndexType(), elem_type, limits.IndexType(),
                           "table.fill");
}

Result TypeChecker::OnRefAsNonNullExpr() {
  Type actual;
  CHECK_RESULT(PopAndCheckReference(&actual, "ref.as_non_null"));
  if (actual != Type::Any) {
    actual = convertRefNullToRef(actual);
  }
  PushType(actual);
  return Result::Ok;
}

Result TypeChecker::OnRefFuncExpr(Index func_type) {
  PushType(Type(Type::Ref, func_type));
  return Result::Ok;
}

Result TypeChecker::OnRefNullExpr(Type type) {
  PushType(type);
  return Result::Ok;
}

Result TypeChecker::OnRefIsNullExpr() {
  Type type;
  Result result = PeekType(0, &type);
  if (!type.IsRef()) {
    type = Type(Type::Reference, kInvalidIndex);
  }
  result |= PopAndCheck1Type(type, "ref.is_null");
  PushType(Type::I32);
  return result;
}

Result TypeChecker::OnRethrow(Index depth) {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(GetRethrowLabel(depth, &label));
  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnThrow(const TypeVector& sig) {
  Result result = Result::Ok;
  result |= PopAndCheckSignature(sig, "throw");
  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnThrowRef() {
  Result result = Result::Ok;
  result |= PopAndCheck1Type(Type::ExnRef, "throw_ref");
  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnReturn() {
  Result result = Result::Ok;
  Label* func_label;
  CHECK_RESULT(GetThisFunctionLabel(&func_label));
  result |= PopAndCheckSignature(func_label->result_types, "return");
  CHECK_RESULT(SetUnreachable());
  return result;
}

Result TypeChecker::OnSelect(const TypeVector& expected) {
  Result result = Result::Ok;
  Type type1 = Type::Any;
  Type type2 = Type::Any;
  Type result_type = Type::Any;
  result |= PeekAndCheckType(0, Type::I32);
  result |= PeekType(1, &type1);
  result |= PeekType(2, &type2);
  if (expected.empty()) {
    if (type1.IsRef() || type2.IsRef()) {
      result = Result::Error;
    } else {
      result |= CheckType(type1, type2);
      result_type = type1;
    }
  } else {
    assert(expected.size() == 1);
    result |= CheckType(type1, expected[0]);
    result |= CheckType(type2, expected[0]);
  }
  PrintStackIfFailed(result, "select", result_type, result_type, Type::I32);
  result |= DropTypes(3);
  PushType(result_type);
  return result;
}

Result TypeChecker::OnStore(Opcode opcode, const Limits& limits) {
  return CheckOpcode2(opcode, &limits);
}

Result TypeChecker::OnTry(const TypeVector& param_types,
                          const TypeVector& result_types) {
  Result result = PopAndCheckSignature(param_types, "try");
  PushLabel(LabelType::Try, param_types, result_types);
  PushTypes(param_types);
  return result;
}

Result TypeChecker::BeginTryTable(const TypeVector& param_types) {
  Result result = PopAndCheckSignature(param_types, "try_table");
  return result;
}

Result TypeChecker::OnTryTableCatch(const TypeVector& sig, Index depth) {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(GetLabel(depth, &label));
  TypeVector& label_sig = label->br_types();
  result |= CheckTypes(label_sig, sig);
  if (Failed(result)) {
    PrintError("catch signature doesn't match target: expected %s, got %s",
               TypesToString(sig).c_str(), TypesToString(label_sig).c_str());
  }
  return result;
}

Result TypeChecker::EndTryTable(const TypeVector& param_types,
                                const TypeVector& result_types) {
  PushLabel(LabelType::TryTable, param_types, result_types);
  PushTypes(param_types);
  return Result::Ok;
}

Result TypeChecker::OnUnary(Opcode opcode) {
  return CheckOpcode1(opcode);
}

Result TypeChecker::OnTernary(Opcode opcode) {
  return CheckOpcode3(opcode);
}

Result TypeChecker::OnSimdLaneOp(Opcode opcode, uint64_t lane_idx) {
  Result result = Result::Ok;
  uint32_t lane_count = opcode.GetSimdLaneCount();
  if (lane_idx >= lane_count) {
    PrintError("lane index must be less than %d (got %" PRIu64 ")", lane_count,
               lane_idx);
    result = Result::Error;
  }

  switch (opcode) {
    case Opcode::I8X16ExtractLaneS:
    case Opcode::I8X16ExtractLaneU:
    case Opcode::I16X8ExtractLaneS:
    case Opcode::I16X8ExtractLaneU:
    case Opcode::I32X4ExtractLane:
    case Opcode::F32X4ExtractLane:
    case Opcode::I64X2ExtractLane:
    case Opcode::F64X2ExtractLane:
      result |= CheckOpcode1(opcode);
      break;
    case Opcode::I8X16ReplaceLane:
    case Opcode::I16X8ReplaceLane:
    case Opcode::I32X4ReplaceLane:
    case Opcode::F32X4ReplaceLane:
    case Opcode::I64X2ReplaceLane:
    case Opcode::F64X2ReplaceLane:
      result |= CheckOpcode2(opcode);
      break;
    default:
      WABT_UNREACHABLE;
  }
  return result;
}

Result TypeChecker::OnSimdLoadLane(Opcode opcode,
                                   const Limits& limits,
                                   uint64_t lane_idx) {
  Result result = Result::Ok;
  uint32_t lane_count = opcode.GetSimdLaneCount();
  if (lane_idx >= lane_count) {
    PrintError("lane index must be less than %d (got %" PRIu64 ")", lane_count,
               lane_idx);
    result = Result::Error;
  }
  result |= CheckOpcode2(opcode, &limits);
  return result;
}

Result TypeChecker::OnSimdStoreLane(Opcode opcode,
                                    const Limits& limits,
                                    uint64_t lane_idx) {
  Result result = Result::Ok;
  uint32_t lane_count = opcode.GetSimdLaneCount();
  if (lane_idx >= lane_count) {
    PrintError("lane index must be less than %d (got %" PRIu64 ")", lane_count,
               lane_idx);
    result = Result::Error;
  }
  result |= CheckOpcode2(opcode, &limits);
  return result;
}

Result TypeChecker::OnSimdShuffleOp(Opcode opcode, v128 lane_idx) {
  Result result = Result::Ok;
  uint8_t simd_data[16];
  memcpy(simd_data, &lane_idx, 16);
  for (int i = 0; i < 16; i++) {
    if (simd_data[i] >= 32) {
      PrintError("lane index must be less than 32 (got %d)", simd_data[i]);
      result = Result::Error;
    }
  }

  result |= CheckOpcode2(opcode);
  return result;
}

Result TypeChecker::OnUnreachable() {
  return SetUnreachable();
}

Result TypeChecker::EndFunction() {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  result |= CheckLabelType(label, LabelType::Func);
  result |= OnEnd(label, "implicit return", "function");
  return result;
}

Result TypeChecker::BeginInitExpr(Type type) {
  type_stack_.clear();
  label_stack_.clear();
  PushLabel(LabelType::InitExpr, TypeVector(), {type});
  return Result::Ok;
}

Result TypeChecker::EndInitExpr() {
  Result result = Result::Ok;
  Label* label;
  CHECK_RESULT(TopLabel(&label));
  result |= CheckLabelType(label, LabelType::InitExpr);
  result |= OnEnd(label, "initializer expression", "initializer expression");
  return result;
}

}  // namespace wabt
