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

#include "wasm-rt-threads.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t not_impl() {
  printf(
      "Runtime support for Wasm thread/notify not included in this build. "
      "Rebuild WABT with -DBUILD_WASM2C_THREAD_WAIT_NOTIFY=ON\n");
  abort();
}

uint32_t wasm_rt_wait_on_address(uintptr_t address, int64_t timeout) {
  return not_impl();
}

uint32_t wasm_rt_notify_at_address(uintptr_t address, uint32_t count) {
  return not_impl();
}
