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

#ifndef WABT_INTERP_UTIL_H_
#define WABT_INTERP_UTIL_H_

#include <string>
#include <vector>

#include "src/interp/interp.h"
#include "src/string-view.h"

namespace wabt {

class Stream;

namespace interp {

std::string TypedValueToString(const TypedValue&);

void WriteValue(Stream* stream, const TypedValue&);

void WriteValues(Stream* stream, const ValueTypes&, const Values&);

void WriteTrap(Stream* stream, const char* desc, const Trap::Ptr&);

void WriteCall(Stream* stream,
               string_view name,
               const FuncType& func_type,
               const Values& params,
               const Values& results,
               const Trap::Ptr& trap);

}  // namespace interp
}  // namespace wabt

#endif  // WABT_INTERP_UTIL_H_
