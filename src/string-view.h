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

#ifndef WABT_STRING_VIEW_H_
#define WABT_STRING_VIEW_H_

#include "src/macros.h"

#if WITH_EXCEPTIONS
#define nssv_throw(type, msg) throw type(msg)
#else
#define nssv_throw(type, msg) WABT_FATAL(msg)
#endif

#include "nonstd/string_view.hpp"

namespace wabt {

using nonstd::string_view;

}  // namespace wabt

#endif  // WABT_STRING_VIEW_H_
