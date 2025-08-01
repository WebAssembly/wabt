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

#include "wabt/config.h"
#include "wabt/base-types.h"
#include "wabt/string-format.h"

namespace wabt {

class Type;

using TypeVector = std::vector<Type>;

class Type {
 public:
  // Matches binary format, do not change.
  enum Enum : int32_t {
    I32 = -0x01,        // 0x7f
    I64 = -0x02,        // 0x7e
    F32 = -0x03,        // 0x7d
    F64 = -0x04,        // 0x7c
    V128 = -0x05,       // 0x7b
    I8 = -0x08,         // 0x78  : packed-type only, used in gc and as v128 lane
    I16 = -0x09,        // 0x77  : packed-type only, used in gc and as v128 lane
    NullFuncRef = -0x0d, // 0x73
    NullExternRef = -0x0e, // 0x72
    NullRef = -0x0f,    // 0x71
    FuncRef = -0x10,    // 0x70
    ExternRef = -0x11,  // 0x6f
    AnyRef = -0x12,     // 0x6e
    EqRef = -0x13,      // 0x6d
    I31Ref = -0x14,     // 0x6c
    StructRef = -0x15,  // 0x6b
    ArrayRef = -0x16,   // 0x6a
    ExnRef = -0x17,     // 0x69
    Ref = -0x1c,        // 0x64
    RefNull = -0x1d,    // 0x63
    Func = -0x20,       // 0x60
    Struct = -0x21,     // 0x5f
    Array = -0x22,      // 0x5e
    Sub = -0x30,        // 0x50
    SubFinal = -0x31,   // 0x4f
    Rec = -0x32,        // 0x4e
    Void = -0x40,       // 0x40
    ___ = Void,         // Convenient for the opcode table in opcode.h

    Any = 0,   // Not actually specified, but useful for type-checking
    I8U = 4,   // Not actually specified, but used internally with load/store
    I16U = 6,  // Not actually specified, but used internally with load/store
    I32U = 7,  // Not actually specified, but used internally with load/store
  };

  // Used by FuncRef / ExternRef
  enum GenericReferenceType : uint32_t {
    ReferenceOrNull = 0,
    ReferenceNonNull = 1,
  };

  Type() = default;  // Provided so Type can be member of a union.
  Type(int32_t code)
      : enum_(static_cast<Enum>(code)), type_index_(0) {
    assert(!IsReferenceWithIndex());
  }
  Type(Enum e) : enum_(e), type_index_(0) {
    assert(!IsReferenceWithIndex());
  }
  Type(Enum e, Index type_index) : enum_(e), type_index_(type_index) {
    assert(IsReferenceWithIndex() ||
           (IsNonTypedRef() && (type_index_ == ReferenceOrNull || type_index_ == ReferenceNonNull)));
  }
  Type(Enum e, bool is_nullable)
      : enum_(e), type_index_(is_nullable ? ReferenceOrNull : ReferenceNonNull) {
    assert(IsNonTypedRef());
  }

  constexpr operator Enum() const { return enum_; }

  friend constexpr bool operator==(const Type a, const Type b) {
    return a.enum_ == b.enum_ && a.type_index_ == b.type_index_;
  }
  friend constexpr bool operator!=(const Type a, const Type b) {
    return !(a == b);
  }
  friend constexpr bool operator==(const Type ty, const Enum code) {
    return ty.enum_ == code;
  }
  friend constexpr bool operator!=(const Type ty, const Enum code) {
    return !(ty == code);
  }
  friend constexpr bool operator<(const Type a, const Type b) {
    return a.enum_ == b.enum_ ? a.type_index_ < b.type_index_
                              : a.enum_ < b.enum_;
  }

  bool IsRef() const {
    return enum_ == Type::NullFuncRef || enum_ == Type::NullExternRef ||
           enum_ == Type::NullRef || enum_ == Type::FuncRef ||
           enum_ == Type::ExternRef || enum_ == Type::AnyRef ||
           enum_ == Type::EqRef || enum_ == Type::I31Ref ||
           enum_ == Type::StructRef || enum_ == Type::ArrayRef ||
           enum_ == Type::ExnRef || enum_ == Type::Ref ||
           enum_ == Type::RefNull;
  }

  bool IsNullableRef() const {
    return enum_ == Type::ExnRef || enum_ == Type::RefNull ||
           (EnumIsNonTypedRef(enum_) && type_index_ == ReferenceOrNull);
  }

  bool IsNonNullableRef() const {
    return enum_ == Type::Ref ||
           (EnumIsNonTypedRef(enum_) && type_index_ != ReferenceOrNull);
  }

  bool IsReferenceWithIndex() const { return EnumIsReferenceWithIndex(enum_); }

  bool IsNonTypedRef() const {
    return EnumIsNonTypedRef(enum_);
  }

  bool IsNullableNonTypedRef() const {
    assert(EnumIsNonTypedRef(enum_));
    return type_index_ == ReferenceOrNull;
  }

  std::string GetName() const {
    switch (enum_) {
      case Type::I32:       return "i32";
      case Type::I64:       return "i64";
      case Type::F32:       return "f32";
      case Type::F64:       return "f64";
      case Type::V128:      return "v128";
      case Type::I8:        return "i8";
      case Type::I16:       return "i16";
      case Type::ExnRef:    return "exnref";
      case Type::Func:      return "func";
      case Type::Struct:    return "struct";
      case Type::Array:     return "array";
      case Type::Void:      return "void";
      case Type::Any:       return "any";
      case Type::NullFuncRef:
        return type_index_ == ReferenceOrNull ? "nullfuncref" : "(ref nofunc)";
      case Type::NullExternRef:
        return type_index_ == ReferenceOrNull ? "nullexternref" : "(ref noextern)";
      case Type::NullRef:
        if (type_index_ == kBottomRef) {
          return "(ref something)";
        }
        return type_index_ == ReferenceOrNull ? "nullref" : "(ref none)";
      case Type::FuncRef:
        return type_index_ == ReferenceOrNull ? "funcref" : "(ref func)";
      case Type::ExternRef:
        return type_index_ == ReferenceOrNull ? "externref" : "(ref extern)";
      case Type::AnyRef:
        return type_index_ == ReferenceOrNull ? "anyref" : "(ref any)";
      case Type::EqRef:
        return type_index_ == ReferenceOrNull ? "eqref" : "(ref eq)";
      case Type::I31Ref:
        return type_index_ == ReferenceOrNull ? "i31ref" : "(ref i31)";
      case Type::StructRef:
        return type_index_ == ReferenceOrNull ? "structref" : "(ref struct)";
      case Type::ArrayRef:
        return type_index_ == ReferenceOrNull ? "arrayref" : "(ref array)";
      case Type::Ref:
        return StringPrintf("(ref %d)", type_index_);
      case Type::RefNull:
        return StringPrintf("(ref null %d)", type_index_);
      default:
        return StringPrintf("<type_index[%d]>", enum_);
    }
  }

  const char* GetRefKindName() const {
    switch (enum_) {
      case Type::NullFuncRef:   return "nofunc";
      case Type::NullExternRef: return "noextern";
      case Type::NullRef:
        return (type_index_ == kBottomRef) ? "something" : "none";
      case Type::FuncRef:       return "func";
      case Type::ExternRef:     return "extern";
      case Type::ExnRef:        return "exn";
      case Type::AnyRef:        return "any";
      case Type::EqRef:         return "eq";
      case Type::I31Ref:        return "i31";
      case Type::Struct:        return "struct";
      case Type::Array:         return "array";
      default:                  return "<invalid>";
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

  Index GetIndex() const {
    assert(IsIndex());
    return static_cast<Index>(enum_);
  }

  Index GetReferenceIndex() const {
    assert(IsReferenceWithIndex());
    return type_index_;
  }

  bool IsPackedType() const {
    return enum_ == Type::I8 || enum_ == Type::I16;
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
      case Type::NullFuncRef:
      case Type::NullExternRef:
      case Type::NullRef:
      case Type::FuncRef:
      case Type::ExternRef:
      case Type::AnyRef:
      case Type::EqRef:
      case Type::I31Ref:
      case Type::StructRef:
      case Type::ArrayRef:
      case Type::ExnRef:
      case Type::Ref:
      case Type::RefNull:
        return TypeVector(this, this + 1);

      default:
        WABT_UNREACHABLE;
    }
  }

  static bool EnumIsReferenceWithIndex(Enum value) {
    return value == Type::Ref || value == Type::RefNull;
  }

  static bool EnumIsNonTypedGCRef(Enum value) {
    return value == Type::NullFuncRef || value == Type::NullExternRef ||
           value == Type::NullRef || value == Type::AnyRef ||
           value == Type::EqRef || value == Type::I31Ref ||
           value == Type::StructRef || value == Type::ArrayRef;
  }

  static bool EnumIsNonTypedRef(Enum value) {
    return value == Type::ExternRef || value == Type::FuncRef ||
           value == Type::ExnRef || EnumIsNonTypedGCRef(value);
  }

  // Bottom references are only used by the shared
  // validator. It represents an unknown reference.
  // Nullable property is not defined for this type.
  static Type BottomRef() {
    Type type(NullRef);
    type.type_index_ = kBottomRef;
    return type;
  }

  bool IsBottomRef() const {
    return enum_ == NullRef && type_index_ == kBottomRef;
  }

  void ConvertRefNullToRef() {
    if (IsReferenceWithIndex()) {
      enum_ = Type::Ref;
    } else {
      assert(IsNonTypedRef());
      type_index_ |= ReferenceNonNull;
    }
  }

 private:
  // Special value representing an unknown reference.
  static const uint32_t kBottomRef = 0x2 | ReferenceNonNull;

  Enum enum_;
  // This index is 0 for non-references, so a zeroed
  // memory area represents a valid Type::Any type.
  // It contains an index for references with type index.
  Index type_index_;
};

}  // namespace wabt

#endif  // WABT_TYPE_H_
