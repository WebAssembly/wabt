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

#include "common.h"

#define WABT_BINARY_MAGIC 0x6d736100
#define WABT_BINARY_VERSION 1
#define WABT_BINARY_LIMITS_HAS_MAX_FLAG 0x1

#define WABT_BINARY_SECTION_NAME "name"
#define WABT_BINARY_SECTION_RELOC "reloc"

#define WABT_FOREACH_BINARY_SECTION(V) \
  V(Custom, custom, 0)                 \
  V(Type, type, 1)                     \
  V(Import, import, 2)                 \
  V(Function, function, 3)             \
  V(Table, table, 4)                   \
  V(Memory, memory, 5)                 \
  V(Global, global, 6)                 \
  V(Export, export, 7)                 \
  V(Start, start, 8)                   \
  V(Elem, elem, 9)                     \
  V(Code, code, 10)                    \
  V(Data, data, 11)

/* clang-format off */
enum class WabtBinarySection {
#define V(Name, name, code) Name = code,
  WABT_FOREACH_BINARY_SECTION(V)
#undef V
  Invalid,

  First = Custom,
  Last = Data,
};
/* clang-format on */
static const int kWabtBinarySectionCount = WABT_ENUM_COUNT(WabtBinarySection);

WABT_EXTERN_C_BEGIN
extern const char* g_wabt_section_name[];

static WABT_INLINE const char* wabt_get_section_name(WabtBinarySection sec) {
  assert(static_cast<int>(sec) < kWabtBinarySectionCount);
  return g_wabt_section_name[static_cast<size_t>(sec)];
}
WABT_EXTERN_C_END

#endif /* WABT_BINARY_H_ */
