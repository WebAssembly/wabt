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
#ifndef WABT_COMPILER_SUPPORT_H_
#define WABT_COMPILER_SUPPORT_H_

// Taken from binaryen's src/compiler-support.h
// The code might contain TODOs or stubs that read some values but do nothing
// with them. The compiler might fail with [-Werror,-Wunused-variable].
// The WASM_UNUSED(varible) is a wrapper that helps to suppress the error.
#define WASM_UNUSED(expr)                                                      \
  do {                                                                         \
    if (sizeof expr) {                                                         \
      (void)0;                                                                 \
    }                                                                          \
  } while (0)

#endif  // WABT_COMPILER_SUPPORT_H_
