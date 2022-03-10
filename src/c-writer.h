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

#ifndef WABT_C_WRITER_H_
#define WABT_C_WRITER_H_

#include "src/common.h"

#include <string>

namespace wabt {

struct Module;
class Stream;

struct WriteCOptions {
    // Unique name for the module being generated. Each wasm sandboxed module in a single application should have a unique name.
    // By default the module name is empty and need not be set if an application is only using one Wasm module or is using Wasm modules in shared libraries.
    // However, if an application wants to statically link more than one Wasm module, it should assign each module a unique name.
    std::string mod_name;
};

Result WriteC(Stream* c_stream,
              Stream* h_stream,
              const char* header_name,
              const Module*,
              const WriteCOptions&);

}  // namespace wabt

#endif /* WABT_C_WRITER_H_ */
