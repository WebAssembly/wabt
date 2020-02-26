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

using Index = uint32_t;

// Matches binary format, do not change.
enum class Type : int32_t {
  I32 = -0x01,      // 0x7f
  I64 = -0x02,      // 0x7e
  F32 = -0x03,      // 0x7d
  F64 = -0x04,      // 0x7c
  V128 = -0x05,     // 0x7b
  Funcref = -0x10,  // 0x70
  Anyref = -0x11,   // 0x6f
  Nullref = -0x12,  // 0x6e
  Exnref = -0x18,   // 0x68
  Func = -0x20,     // 0x60
  Void = -0x40,     // 0x40
  ___ = Void,       // Convenient for the opcode table in opcode.h
  Any = 0,          // Not actually specified, but useful for type-checking
  Hostref = 2,  // Not actually specified, but used in testing and type-checking
  I8 = 3,       // Not actually specified, but used internally with load/store
  I8U = 4,      // Not actually specified, but used internally with load/store
  I16 = 5,      // Not actually specified, but used internally with load/store
  I16U = 6,     // Not actually specified, but used internally with load/store
  I32U = 7,     // Not actually specified, but used internally with load/store
};
typedef std::vector<Type> TypeVector;

inline bool IsRefType(Type t) {
  return t == Type::Anyref || t == Type::Funcref || t == Type::Nullref ||
         t == Type::Exnref || t == Type::Hostref;
}

inline bool IsNullableRefType(Type t) {
  /* Currently all reftypes are nullable */
  return IsRefType(t);
}

inline const char* GetTypeName(Type type) {
  switch (type) {
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
    case Type::Funcref:
      return "funcref";
    case Type::Func:
      return "func";
    case Type::Exnref:
      return "exnref";
    case Type::Void:
      return "void";
    case Type::Any:
      return "any";
    case Type::Anyref:
      return "anyref";
    case Type::Nullref:
      return "nullref";
    default:
      return "<type_index>";
  }
}

inline bool IsTypeIndex(Type type) {
  return static_cast<int32_t>(type) >= 0;
}

inline Index GetTypeIndex(Type type) {
  assert(IsTypeIndex(type));
  return static_cast<Index>(type);
}

inline TypeVector GetInlineTypeVector(Type type) {
  assert(!IsTypeIndex(type));
  switch (type) {
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
    case Type::Exnref:
      return TypeVector(&type, &type + 1);

    default:
      WABT_UNREACHABLE;
  }
}

}  // namespace wabt

#endif  // WABT_TYPE_H_
