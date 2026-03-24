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

#ifndef WABT_COMPONENT_H_
#define WABT_COMPONENT_H_

#include "wabt/common.h"

namespace wabt {

// Contains data structures used by the component model.

enum class ComponentSection : uint8_t {
  Custom = 0,
  CoreModule = 1,
  CoreInstance = 2,
  CoreType = 3,
  Component = 4,
  Instance = 5,
  Alias = 6,
  Type = 7,
  Canon = 8,
  Import = 10,
  Export = 11,
};

class ComponentSort {
 public:
  enum Enum : uint8_t {
    CoreFunc,
    CoreTable,
    CoreMemory,
    CoreGlobal,
    CoreType,
    CoreModule,
    CoreInstance,
    Func,
    Value,
    Type,
    Component,
    Instance,
    Invalid,
  };

  ComponentSort(Enum sort)
      : sort_(sort) {}

  ComponentSort()
      : sort_(Invalid) {}

  constexpr operator Enum() const { return sort_; }

  const char* GetName() const;
  const char* GetCoreName() const;

 private:
  Enum sort_;
};

// Composite type definitions.
enum class ComponentTypeDef : uint8_t {
  ValueType = 0,

  // Types
  Record = 0x72,
  Variant = 0x71,
  List = 0x70,
  Tuple = 0x6f,
  Flags = 0x6e,
  Enum = 0x6d,
  Option = 0x6b,
  Result = 0x6a,
  Own = 0x69,
  Borrow = 0x68,
  Stream = 0x66,
  Future = 0x65,
  ListFixed = 0x67,
  AsyncFunc = 0x43,
  Instance = 0x42,
  Component = 0x41,
  Func = 0x40,
};

enum class ComponentExternalDesc : uint8_t {
  ValueEq = 0x00,
  ValueType = 0x01,
  TypeEq = 0x00,
  TypeSubRes = 0x01,
  Unused = 0xff,
};

struct ComponentNamedType {
  std::string_view name;
  ComponentType type;
};

struct ComponentNamedSort {
  std::string_view name;
  ComponentSort sort;
  Index index;
};

struct ComponentCanonOption {
  enum Option : uint8_t {
    StrEncUtf8 = 0x00,
    StrEncUtf16 = 0x01,
    StrEncLatin1Utf16 = 0x02,
    Memory = 0x03,
    Realloc = 0x04,
    PostReturn = 0x05,
    Async = 0x06,
    Callback = 0x07,
  };

  Option option;
  Index index;
};

struct ComponentExternalInfo {
  ComponentSort sort;
  ComponentExternalDesc external;
  Index index;
};

struct ComponentExportInfo {
  ComponentSort sort;
  Index index;
};

struct ComponentNamedExportInfo {
  std::string_view name;
  std::string_view version_suffix;
  ComponentSort sort;
  bool has_version_suffix;
  Index index;
};

}  // namespace wabt

#endif  // WABT_DECOMPILER_AST_H_
