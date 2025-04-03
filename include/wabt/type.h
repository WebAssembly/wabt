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
    I8 = -0x06,         // 0x7a  : packed-type only, used in gc and as v128 lane
    I16 = -0x07,        // 0x79  : packed-type only, used in gc and as v128 lane
    ExnRef = -0x17,     // 0x69
    FuncRef = -0x10,    // 0x70
    ExternRef = -0x11,  // 0x6f
    Reference = -0x15,  // 0x6b
    Func = -0x20,       // 0x60
    Struct = -0x21,     // 0x5f
    Array = -0x22,      // 0x5e
    Void = -0x40,       // 0x40
    ___ = Void,         // Convenient for the opcode table in opcode.h

    Any = 0,   // Not actually specified, but useful for type-checking
    I8U = 4,   // Not actually specified, but used internally with load/store
    I16U = 6,  // Not actually specified, but used internally with load/store
    I32U = 7,  // Not actually specified, but used internally with load/store
  };

  Type() = default;  // Provided so Type can be member of a union.
  Type(int32_t code)
      : enum_(static_cast<Enum>(code)), type_index_(0) {
    assert(!EnumIsReferenceWithIndex(enum_));
  }
  Type(Enum e) : enum_(e), type_index_(0) {
    assert(!EnumIsReferenceWithIndex(enum_));
  }
  Type(Enum e, Index type_index) : enum_(e), type_index_(type_index) {
    assert(EnumIsReferenceWithIndex(e));
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
    return enum_ == Type::ExternRef || enum_ == Type::FuncRef ||
           enum_ == Type::Reference || enum_ == Type::ExnRef;
  }

  bool IsReferenceWithIndex() const { return EnumIsReferenceWithIndex(enum_); }

  bool IsNullableRef() const {
    // Currently all reftypes are nullable
    return IsRef();
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
      case Type::FuncRef:   return "funcref";
      case Type::Func:      return "func";
      case Type::Void:      return "void";
      case Type::Any:       return "any";
      case Type::ExternRef: return "externref";
      case Type::Reference:
        return StringPrintf("(ref %d)", type_index_);
      default:
        return StringPrintf("<type_index[%d]>", enum_);
    }
  }

  const char* GetRefKindName() const {
    switch (enum_) {
      case Type::FuncRef:   return "func";
      case Type::ExternRef: return "extern";
      case Type::ExnRef:    return "exn";
      case Type::Struct:    return "struct";
      case Type::Array:     return "array";
      default:              return "<invalid>";
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
    assert(enum_ == Enum::Reference);
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
      case Type::FuncRef:
      case Type::ExnRef:
      case Type::ExternRef:
      case Type::Reference:
        return TypeVector(this, this + 1);

      default:
        WABT_UNREACHABLE;
    }
  }

 private:
  static bool EnumIsReferenceWithIndex(Enum value) {
    return value == Type::Reference;
  }

  Enum enum_;
  // This index is 0 for non-references, so a zeroed
  // memory area represents a valid Type::Any type.
  // It contains an index for references with type index.
  Index type_index_;
};

}  // namespace wabt

#endif  // WABT_TYPE_H_
