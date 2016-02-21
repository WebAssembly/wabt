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

#ifndef WASM_ALLOCATOR_H_
#define WASM_ALLOCATOR_H_

#include <stdlib.h>

#include "wasm-common.h"

#define WASM_DEFAULT_ALIGN sizeof(void*)

typedef struct WasmAllocator {
  void* (*alloc)(struct WasmAllocator*, size_t size, size_t align);
  void* (*realloc)(struct WasmAllocator*, void* p, size_t size, size_t align);
  void (*free)(struct WasmAllocator*, void* p);
} WasmAllocator;

extern WasmAllocator g_wasm_libc_allocator;

EXTERN_C_BEGIN
void* wasm_alloc(WasmAllocator*, size_t size, size_t align);
void* wasm_alloc_zero(WasmAllocator*, size_t size, size_t align);
void wasm_free(WasmAllocator*, void*);
char* wasm_strndup(WasmAllocator*, const char*, size_t);
EXTERN_C_END

#endif /* WASM_ALLOCATOR_H_ */
