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

#ifndef WABT_TYPE_H_
#define WABT_TYPE_H_

#include <cassert>
#include <cstdint>
#include <vector>

#include "wabt/base-types.h"
#include "wabt/config.h"
#include "wabt/string-format.h"

namespace wabt {

class Type;

using TypeVector = std::vector<Type>;

class Type {
 public:
  // Matches binary format, do not change.
  enum Enum : int32_t {
    I32 = -0x01,   // 0x7f
    I64 = -0x02,   // 0x7e
    F32 = -0x03,   // 0x7d
    F64 = -0x04,   // 0x7c
    V128 = -0x05,  // 0x7b

    I8 = -0x08,
    I16 = -0x09,
    Reference = -0x44,
    // Heap type
    NoFunc = -0x0D,
    NoExtern = -0x0E,
    NoneRef = -0x0F,
    FuncRef = -0x10,
    ExternRef = -0x11,
    Any = -0x12,
    Eq = -0x13,
    I31 = -0x14,
    StructRef = -0x15,
    ArrayRef = -0x16,
    // reference type
    Null = -0x1b,
    Ref = -0x1c,
    RefNull = -0x1d,
    // composite type
    Func = -0x20,
    Struct = -0x21,
    Array = -0x22,
    // sub type and recursive type
    Sub = -0x30,
    SubFinal = -0x31,
    Rec = -0x32,

    Void = -0x40,
    ___ = Void,
    I8U = 4,
    I16U = 6,
    I32U = 7,
  };

  Type() = default;  // Provided so Type can be member of a union.
  Type(int32_t code)
      : enum_(static_cast<Enum>(code)), type_index_(kInvalidIndex) {}
  Type(Enum e) : enum_(e), type_index_(kInvalidIndex) {}
  Type(Enum e, Index type_index) : enum_(e), type_index_(type_index) {
    // assert(e == Enum::Reference);
  }
  constexpr operator Enum() const { return enum_; }

  bool IsRef() const {
    return enum_ == Type::ExternRef || enum_ == Type::FuncRef ||
           enum_ == Type::Reference || enum_ == Type::Ref ||
           enum_ == Type::RefNull;
  }

  bool IsReferenceWithIndex() const {
    return enum_ == Type::Reference || enum_ == Type::Ref ||
           enum_ == Type::RefNull;
  }

  bool IsNullableRef() const {
    // Currently all reftypes are nullable
    return IsRef();
  }

  std::string GetName() const {
    switch (enum_) {
      case Type::I32:
        return "i32";
      case Type::I64:
        return "i64";
      case Type::F32:
        return "f32";
      case Type::F64:
        return "f64";
      case Type::V128:
        return "v128";
      case Type::I8:
        return "i8";
      case Type::I16:
        return "i16";
      case Type::NoneRef:
        return "noneref";
      case Type::FuncRef:
        return "funcref";
      case Type::Func:
        return "func";
      case Type::Void:
        return "void";
      case Type::ExternRef:
        return "externref";
      case Type::Reference:
        return StringPrintf("(ref %d)", type_index_);
      default:
        return StringPrintf("<type_index[%d]>", enum_);
    }
  }

  const char* GetRefKindName() const {
    switch (enum_) {
      case Type::NoneRef:
        return "none";
      case Type::FuncRef:
        return "func";
      case Type::ExternRef:
        return "extern";
      case Type::Struct:
        return "struct";
      case Type::Array:
        return "array";
      default:
        return "<invalid>";
    }
  }

  // Functions for handling types that are an index into the type section.
  // These are always positive integers. They occur in the binary format in
  // block signatures, e.g.
  //
  //   (block (result i32 i64) ...)
  //
  // is encoded as
  //
  //   (type $T (func (result i32 i64)))
  //   ...
  //   (block (type $T) ...)
  //
  bool IsIndex() const { return static_cast<int32_t>(enum_) >= 0; }

  bool isRef() const { return enum_ == Type::Ref; }
  bool isRefNull() const { return enum_ == Type::RefNull; }

  Index GetIndex() const {
    assert(IsIndex());
    return static_cast<Index>(enum_);
  }

  Index GetReferenceIndex() const {
    assert(enum_ == Enum::Reference || enum_ == Type::Ref ||
           enum_ == Type::RefNull);
    return type_index_;
  }

  TypeVector GetInlineVector() const {
    assert(!IsIndex());
    switch (enum_) {
      case Type::Void:
        return TypeVector();

      case Type::I32:
      case Type::I64:
      case Type::F32:
      case Type::F64:
      case Type::V128:
      case Type::Ref:
      case Type::RefNull:
      case Type::NoneRef:
      case Type::FuncRef:
      case Type::ExternRef:
      case Type::Reference:
        return TypeVector(this, this + 1);

      default:
        WABT_UNREACHABLE;
    }
  }

  Enum enum_;
  Index type_index_;  // Only used for for Type::Reference
};

}  // namespace wabt

#endif  // WABT_TYPE_H_
