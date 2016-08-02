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

#include "wasm-ast-lexer.h"
#include "wasm-common.h"

struct WasmAllocator;
struct WasmModule;
struct WasmScript;

WASM_EXTERN_C_BEGIN
/* Only check that names are valid; this is useful if you want to generate
 * invalid binaries (so they can be validated by the consumer). */
WasmResult wasm_check_names(struct WasmAllocator*,
                            WasmAstLexer*,
                            const struct WasmScript*,
                            WasmSourceErrorHandler*);

/* perform all checks on the AST; the module is valid if and only if this
 * function succeeds. */
WasmResult wasm_check_ast(struct WasmAllocator*,
                          WasmAstLexer*,
                          const struct WasmScript*,
                          WasmSourceErrorHandler*);

/* Run the assert_invalid spec tests. These always contain an invalid module.
 * This function succeeds if and only if the contained module is invalid. */
WasmResult wasm_check_assert_invalid(
    struct WasmAllocator*,
    WasmAstLexer*,
    const struct WasmScript*,
    WasmSourceErrorHandler* assert_invalid_error_handler,
    WasmSourceErrorHandler* error_handler);
WASM_EXTERN_C_END

#endif /* WASM_AST_CHECKER_H_ */
