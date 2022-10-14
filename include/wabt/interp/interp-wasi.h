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

#ifndef WABT_INTERP_WASI_H_
#define WABT_INTERP_WASI_H_

#include "wabt/common.h"
#include "wabt/error.h"
#include "wabt/interp/interp.h"

#ifdef WITH_WASI

struct uvwasi_s;

namespace wabt {
namespace interp {

Result WasiRegisterInstance(const Instance::Ptr& instance,
                            uvwasi_s* uvwasi,
                            Stream* stream,
                            Stream* trace_stream);

void WasiUnregisterInstance(const Instance::Ptr& instance);

Ref WasiGetImport(const Module::Ptr& module, const ImportDesc& import);

Result WasiRunStart(const Instance::Ptr& instance,
                    uvwasi_s* uvwasi,
                    Stream* stream,
                    Stream* trace_stream);

}  // namespace interp
}  // namespace wabt

#endif

#endif /* WABT_INTERP_WASI_H_ */
