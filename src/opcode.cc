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

#include "opcode.h"

#include <algorithm>

namespace wabt {

// static
Opcode::Info Opcode::infos_[] = {
#define WABT_OPCODE(rtype, type1, type2, mem_size, code, Name, text) \
  {text, Type::rtype, Type::type1, Type::type2, mem_size, code},
#include "opcode.def"
#undef WABT_OPCODE
};

#define WABT_OPCODE(rtype, type1, type2, mem_size, code, Name, text) \
  /* static */ Opcode Opcode::Name##_Opcode(Opcode::Name);
#include "opcode.def"
#undef WABT_OPCODE

// static
Opcode::Info Opcode::invalid_info_ = {
    "<invalid>", Type::Void, Type::Void, Type::Void, 0, 0,
};

// static
Opcode Opcode::FromCode(uint32_t code) {
  auto iter = std::lower_bound(
      infos_, infos_ + WABT_ARRAY_SIZE(infos_), code,
      [](const Info& info, uint32_t code) { return info.code < code; });

  if (iter->code != code)
    return Opcode(Invalid);

  return Opcode(static_cast<Enum>(iter - infos_));
}

Opcode::Info Opcode::GetInfo() const {
  return enum_ < Invalid ? infos_[enum_] : invalid_info_;
}

bool Opcode::IsNaturallyAligned(Address alignment) const {
  Address opcode_align = GetMemorySize();
  return alignment == WABT_USE_NATURAL_ALIGNMENT || alignment == opcode_align;
}

Address Opcode::GetAlignment(Address alignment) const {
  if (alignment == WABT_USE_NATURAL_ALIGNMENT)
    return GetMemorySize();
  return alignment;
}

}  // namespace
