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

#ifndef WASM_AST_CHECKER_H_
#define WASM_AST_CHECKER_H_

#include "wasm-common.h"
#include "wasm-lexer.h"

struct WasmAllocator;
struct WasmModule;
struct WasmScript;

WASM_EXTERN_C_BEGIN
WasmResult wasm_check_ast(WasmLexer*,
                          const struct WasmScript*,
                          WasmSourceErrorHandler*);

WasmResult wasm_check_assert_invalid(
    WasmLexer*,
    const struct WasmScript*,
    WasmSourceErrorHandler* assert_invalid_error_handler,
    WasmSourceErrorHandler* error_handler);
WASM_EXTERN_C_END

#endif /* WASM_AST_CHECKER_H_ */
