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

#ifndef WASM_TYPE_VECTOR_H_
#define WASM_TYPE_VECTOR_H_

#include "wasm-common.h"
#include "wasm-vector.h"

WASM_DEFINE_VECTOR(type, WasmType)

WASM_EXTERN_C_BEGIN

static WASM_INLINE WasmBool
wasm_type_vectors_are_equal(const WasmTypeVector* types1,
                            const WasmTypeVector* types2) {
  if (types1->size != types2->size)
    return WASM_FALSE;
  size_t i;
  for (i = 0; i < types1->size; ++i) {
    if (types1->data[i] != types2->data[i])
      return WASM_FALSE;
  }
  return WASM_TRUE;
}

WASM_EXTERN_C_END

#endif /* WASM_TYPE_VECTOR_H_ */
