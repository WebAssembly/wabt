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

#ifndef WASM_VALIDATOR_H_
#define WASM_VALIDATOR_H_

#include "ast-lexer.h"
#include "common.h"

struct WasmAllocator;
struct WasmModule;
struct WasmScript;

WASM_EXTERN_C_BEGIN
/* perform all checks on the AST; the module is valid if and only if this
 * function succeeds. */
WasmResult wasm_validate_script(struct WasmAllocator*,
                                WasmAstLexer*,
                                const struct WasmScript*,
                                WasmSourceErrorHandler*);

#endif /* WASM_VALIDATOR_H_ */
