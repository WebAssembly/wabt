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

#ifndef WABT_BINARY_H_
#define WABT_BINARY_H_

#include "wabt/common.h"

#define WABT_BINARY_MAGIC 0x6d736100
#define WABT_BINARY_VERSION 1
#define WABT_BINARY_COMPONENT_VERSION 0xd
#define WABT_BINARY_LAYER_MODULE 0
#define WABT_BINARY_LAYER_COMPONENT 1
#define WABT_BINARY_LIMITS_HAS_MAX_FLAG 0x1
#define WABT_BINARY_LIMITS_IS_SHARED_FLAG 0x2
#define WABT_BINARY_LIMITS_IS_64_FLAG 0x4
#define WABT_BINARY_LIMITS_HAS_CUSTOM_PAGE_SIZE_FLAG 0x8
#define WABT_BINARY_LIMITS_ALL_MEMORY_FLAGS                              \
  (WABT_BINARY_LIMITS_HAS_MAX_FLAG | WABT_BINARY_LIMITS_IS_SHARED_FLAG | \
   WABT_BINARY_LIMITS_IS_64_FLAG |                                       \
   WABT_BINARY_LIMITS_HAS_CUSTOM_PAGE_SIZE_FLAG)
#define WABT_BINARY_LIMITS_ALL_TABLE_FLAGS                               \
  (WABT_BINARY_LIMITS_HAS_MAX_FLAG | WABT_BINARY_LIMITS_IS_SHARED_FLAG | \
   WABT_BINARY_LIMITS_IS_64_FLAG)

#define WABT_BINARY_SECTION_NAME "name"
#define WABT_BINARY_SECTION_RELOC "reloc"
#define WABT_BINARY_SECTION_LINKING "linking"
#define WABT_BINARY_SECTION_TARGET_FEATURES "target_features"
#define WABT_BINARY_SECTION_DYLINK "dylink"
#define WABT_BINARY_SECTION_DYLINK0 "dylink.0"
#define WABT_BINARY_SECTION_CODE_METADATA "metadata.code."

#define WABT_FOREACH_BINARY_SECTION(V) \
  V(Custom, custom, 0)                 \
  V(Type, type, 1)                     \
  V(Import, import, 2)                 \
  V(Function, function, 3)             \
  V(Table, table, 4)                   \
  V(Memory, memory, 5)                 \
  V(Tag, tag, 13)                      \
  V(Global, global, 6)                 \
  V(Export, export, 7)                 \
  V(Start, start, 8)                   \
  V(Elem, elem, 9)                     \
  V(DataCount, data_count, 12)         \
  V(Code, code, 10)                    \
  V(Data, data, 11)

namespace wabt {

/* clang-format off */
enum class BinarySection {
#define V(Name, name, code) Name = code,
  WABT_FOREACH_BINARY_SECTION(V)
#undef V
  Invalid = ~0,

  First = Custom,
  Last = Tag,
};
/* clang-format on */
constexpr int kBinarySectionCount = WABT_ENUM_COUNT(BinarySection);

enum class BinarySectionOrder {
#define V(Name, name, code) Name,
  WABT_FOREACH_BINARY_SECTION(V)
#undef V
};

BinarySectionOrder GetSectionOrder(BinarySection);
const char* GetSectionName(BinarySection);

// See
// https://github.com/WebAssembly/extended-name-section/blob/main/proposals/extended-name-section/Overview.md
enum class NameSectionSubsection {
  Module = 0,
  Function = 1,
  Local = 2,
  Label = 3,
  Type = 4,
  Table = 5,
  Memory = 6,
  Global = 7,
  ElemSegment = 8,
  DataSegment = 9,
  Field = 10,
  Tag = 11,

  First = Module,
  Last = Tag,
};
const char* GetNameSectionSubsectionName(NameSectionSubsection subsec);

enum class ComponentBinarySection : uint8_t {
  // Same as ComponentDef::Type in ir.h.
  CoreModule = 1,
  CoreInstance = 2,
  CoreType = 3,
  Component = 4,
  Instance = 5,
  Alias = 6,
  Type = 7,
  Import = 10,
  Export = 11,
};

enum class ComponentBinarySort : uint8_t {
  Core = 0x00,
  Func = 0x01,
  Value = 0x02,
  Type = 0x03,
  Component = 0x04,
  Instance = 0x05,
};

enum class ComponentBinaryCoreSort : uint8_t {
  Func = 0x00,
  Table = 0x01,
  Memory = 0x02,
  Global = 0x03,
  Type = 0x10,
  Module = 0x11,
  Instance = 0x12,
  NonCore = 0xff,
};

enum class ComponentBinaryAlias : uint8_t {
  Export = 0x00,
  CoreExport = 0x01,
  Outer = 0x02,
};

enum class ComponentBinaryType : uint8_t {
  None = 0,
  Some = 1,

  ResultSome = 0,
  ResultNone = 1,

  // Types
  Record = 0x72,
  Variant = 0x71,
  List = 0x70,
  Option = 0x6b,
  Result = 0x6a,
  Own = 0x69,
  Borrow = 0x68,
  ListFixed = 0x67,
  AsyncFunc = 0x43,
  Instance = 0x42,
  Component = 0x41,
  Func = 0x40,
};

enum class ComponentBinaryInterface : uint8_t {
  CoreType = 0,
  Type = 1,
  Alias = 2,
  Import = 3,
  Export = 4,
};

enum class ComponentBinaryExternal : uint8_t {
  ValueEq = 0x00,
  ValueType = 0x01,
  TypeEq = 0x00,
  TypeSubRes = 0x01,
  Unused = 0xff,
};

}  // namespace wabt

#endif /* WABT_BINARY_H_ */
