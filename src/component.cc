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

#include "wabt/component.h"

namespace wabt {

const char* ComponentType::GetName() const {
  switch (type_) {
    case Bool:
      return "bool";
    case S8:
      return "s8";
    case U8:
      return "u8";
    case S16:
      return "s16";
    case U16:
      return "u16";
    case S32:
      return "s32";
    case U32:
      return "u32";
    case S64:
      return "s64";
    case U64:
      return "u64";
    case F32:
      return "f32";
    case F64:
      return "f64";
    case Char:
      return "char";
    case String:
      return "string";
    case ErrorContext:
      return "error-context";
    default:
      return "invalid";
  }
}

const char* ComponentSort::GetName() const {
  switch (sort_) {
    case CoreFunc:
      return "core func";
    case CoreTable:
      return "core table";
    case CoreMemory:
      return "core memory";
    case CoreGlobal:
      return "core global";
    case CoreType:
      return "core type";
    case CoreModule:
      return "core module";
    case CoreInstance:
      return "core instance";
    case Func:
      return "func";
    case Value:
      return "value";
    case Type:
      return "type";
    case Component:
      return "component";
    case Instance:
      return "instance";
    default:
      return "invalid";
  }
}

const char* ComponentSort::GetCoreName() const {
  switch (sort_) {
    case CoreFunc:
      return "func";
    case CoreTable:
      return "table";
    case CoreMemory:
      return "memory";
    case CoreGlobal:
      return "global";
    case CoreType:
      return "type";
    case CoreModule:
      return "module";
    case CoreInstance:
      return "instance";
    default:
      return "invalid";
  }
}

}  // namespace wabt
