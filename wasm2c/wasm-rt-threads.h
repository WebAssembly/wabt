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

#ifndef WASM_RT_THREADS_H_
#define WASM_RT_THREADS_H_

#include "wasm-rt.h"

#ifdef __cplusplus
extern "C" {
#endif

// Runtime support for wait/notify functions

/**
 * This function provides functionality for the Wasm atomic wait operation. It
 * take two operands: location of the runtime's internal datastructures, a
 * (global) address and a relative timeout in nanoseconds as an i64. A timeout <
 * 0 never expires. If 0 <= timeout <= maximum signed i64 value , this expires
 * after timeout nanoseconds. Return value is 0 if woken by the notify API, 2 if
 * this is not woken before the timeout expires.
 */
uint32_t wasm_rt_wait_on_address(uintptr_t address, int64_t timeout);

/**
 * This function provides functionality for the Wasm atomic notify operation. It
 * takes two operands: location of the runtime's internal datastructures, a
 * (global) address and a count. The operation will notify as many waiters
 * waiting on this address, up to the maximum as specified by count. The
 * operator returns the number of waiters that were woken up.
 */
uint32_t wasm_rt_notify_at_address(uintptr_t address, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
