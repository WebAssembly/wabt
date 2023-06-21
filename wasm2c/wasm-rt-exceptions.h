/*
 * Copyright 2018 WebAssembly Community Group participants
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

#ifndef WASM_RT_EXCEPTIONS_H_
#define WASM_RT_EXCEPTIONS_H_

#include "wasm-rt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A tag is represented as an arbitrary pointer.
 */
typedef const void* wasm_rt_tag_t;

/**
 * Set the active exception to given tag, size, and contents.
 */
void wasm_rt_load_exception(const wasm_rt_tag_t tag,
                            uint32_t size,
                            const void* values);

/**
 * Throw the active exception.
 */
WASM_RT_NO_RETURN void wasm_rt_throw(void);

/**
 * The type of an unwind target if an exception is thrown and caught.
 */
#define WASM_RT_UNWIND_TARGET wasm_rt_jmp_buf

/**
 * Get the current unwind target if an exception is thrown.
 */
WASM_RT_UNWIND_TARGET* wasm_rt_get_unwind_target(void);

/**
 * Set the unwind target if an exception is thrown.
 */
void wasm_rt_set_unwind_target(WASM_RT_UNWIND_TARGET* target);

/**
 * Tag of the active exception.
 */
wasm_rt_tag_t wasm_rt_exception_tag(void);

/**
 * Size of the active exception.
 */
uint32_t wasm_rt_exception_size(void);

/**
 * Contents of the active exception.
 */
void* wasm_rt_exception(void);

#ifdef __cplusplus
}
#endif

#endif
