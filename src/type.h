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

#include <cstdint>
#include <vector>

namespace wabt {

class Type;

using Index = uint32_t;
using TypeVector = std::vector<Type>;

class Type {
 public:
  // Matches binary format, do not change.
  enum Enum : int32_t {
    I32 = -0x01,      // 0x7f
    I64 = -0x02,      // 0x7e
    F32 = -0x03,      // 0x7d
    F64 = -0x04,      // 0x7c
    V128 = -0x05,     // 0x7b
    I8 = -0x06,       // 0x7a  : packed-type only, used in gc and as v128 lane
    I16 = -0x07,      // 0x79  : packed-type only, used in gc and as v128 lane
    Funcref = -0x10,  // 0x70
    Anyref = -0x11,   // 0x6f
    Nullref = -0x12,  // 0x6e
    RefT = -0x13,     // 0x6d
    Exnref = -0x18,   // 0x68
    Func = -0x20,     // 0x60
    Struct = -0x21,   // 0x5f
    Array = -0x22,    // 0x5e
    Void = -0x40,     // 0x40
    ___ = Void,       // Convenient for the opcode table in opcode.h

    Any = 0,          // Not actually specified, but useful for type-checking
    Hostref = 2,      // Not actually specified, but used in testing and type-checking
    I8U = 4,   // Not actually specified, but used internally with load/store
    I16U = 6,  // Not actually specified, but used internally with load/store
    I32U = 7,  // Not actually specified, but used internally with load/store
  };

  Type() = default;  // Provided so Type can be member of a union.
  Type(int32_t code) : enum_(static_cast<Enum>(code)) {
    assert(enum_ != RefT);  // Use MakeRefT below instead!
  }
  Type(Enum e) : enum_(e) {
    assert(e != RefT);  // Use MakeRefT below instead!
  }
  operator Enum() const { return IsRefT() ? RefT : enum_; }

  bool IsRef() const {
    return enum_ == Type::Anyref || enum_ == Type::Funcref ||
           enum_ == Type::Nullref || enum_ == Type::Exnref ||
           enum_ == Type::Hostref || IsRefT();
  }

  bool IsNullableRef() const {
    // Currently all reftypes are nullable
    return IsRef();
  }

  const char* GetName() const {
    switch (operator Enum()) {
      case Type::I32:     return "i32";
      case Type::I64:     return "i64";
      case Type::F32:     return "f32";
      case Type::F64:     return "f64";
      case Type::V128:    return "v128";
      case Type::I8:      return "i8";
      case Type::I16:     return "i16";
      case Type::Funcref: return "funcref";
      case Type::Anyref:  return "anyref";
      case Type::Nullref: return "nullref";
      case Type::RefT:    return "ref T";
      case Type::Exnref:  return "exnref";
      case Type::Func:    return "func";
      case Type::Struct:  return "struct";
      case Type::Array:   return "array";
      case Type::Void:    return "void";
      case Type::Any:     return "any";
      default:            return "<type_index>";
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

  TypeVector GetInlineVector() const {
    assert(!IsIndex());
    switch (operator Enum()) {
      case Type::Void:
        return TypeVector();

      case Type::I32:
      case Type::I64:
      case Type::F32:
      case Type::F64:
      case Type::V128:
      case Type::Funcref:
      case Type::Anyref:
      case Type::Nullref:
      case Type::RefT:
      case Type::Exnref:
        return TypeVector(this, this + 1);

      default:
        WABT_UNREACHABLE;
    }
  }

  // Functions for handling types of the form (ref $T), where $T is an index
  // into the type section. Rather than storing an additional field in the Type
  // struct, it's more efficient to use the extra bits in the enum to encode
  // the index. For example, RefT is encoded as -0x13 (0xffffffff6d):
  //
  //    1111 1111 1111 1111 1111 1111 0110 1101
  //       f    f    f    f    f    f    6    d
  //
  // We can store the index in the top bits (making sure to keep the MSB set so
  // the number remains negative), giving us 23 bits to use for the type index.
  //
  //    1nnn nnnn nnnn nnnn nnnn nnnn 0110 1101
  //
  static const uint32_t kRefTMask = 0x800000ff;
  static const uint32_t kRefTSignature =
      static_cast<uint32_t>(RefT) & kRefTMask;

  static const uint32_t kRefT_IndexMask = 0x7fffff00;
  static const uint32_t kRefT_IndexShift = 8;

  static Type MakeRefT(Index index) {
    return static_cast<Enum>(kRefTSignature | (index << kRefT_IndexShift));
  }

  bool IsRefT() const {
    return (static_cast<uint32_t>(enum_) & kRefTMask) == kRefTSignature;
  }

  static bool IsRefT(uint32_t code) {
    return code == static_cast<uint32_t>(RefT);
  }

  Index GetRefTIndex() const {
    assert(IsRefT());
    return (static_cast<uint32_t>(enum_) & kRefT_IndexMask) >> kRefT_IndexShift;
  }

  friend bool operator==(Type lhs, Type rhs) { return lhs.enum_ == rhs.enum_; }
  friend bool operator==(Type lhs, Type::Enum rhs) { return lhs.enum_ == rhs; }
  friend bool operator==(Type::Enum lhs, Type rhs) { return lhs == rhs.enum_; }

  friend bool operator!=(Type lhs, Type rhs) { return lhs.enum_ != rhs.enum_; }
  friend bool operator!=(Type lhs, Type::Enum rhs) { return lhs.enum_ != rhs; }
  friend bool operator!=(Type::Enum lhs, Type rhs) { return lhs != rhs.enum_; }

 private:
  Enum enum_;
};

}  // namespace wabt

#endif  // WABT_TYPE_H_
