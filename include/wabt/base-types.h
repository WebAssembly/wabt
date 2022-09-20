/*
 * Copyright 2021 WebAssembly Community Group participants
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

#ifndef WABT_BASE_TYPES_H_
#define WABT_BASE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

namespace wabt {

typedef uint32_t Index;    // An index into one of the many index spaces.
typedef uint64_t Address;  // An address or size in linear memory.
typedef size_t Offset;     // An offset into a host's file or memory buffer.

static const Address kInvalidAddress = ~0;
static const Index kInvalidIndex = ~0;
static const Offset kInvalidOffset = ~0;

}  // namespace wabt

#endif  // WABT_BASE_TYPES_H_
