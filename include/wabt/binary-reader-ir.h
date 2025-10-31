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

#ifndef WABT_BINARY_READER_IR_H_
#define WABT_BINARY_READER_IR_H_

#include "wabt/common.h"
#include "wabt/error.h"

namespace wabt {

struct Module;
struct ReadBinaryOptions;

Result ReadBinaryIr(const char* filename,
                    const void* data,
                    size_t size,
                    const ReadBinaryOptions& options,
                    Errors*,
                    Module* out_module);

class OpcodeRawExpr;
Result ExtractOpcodeRawExpr(OpcodeRawExpr& expr,
                            Module& module,
                            const ReadBinaryOptions& options);

} // namespace wabt

#endif /* WABT_BINARY_READER_IR_H_ */
