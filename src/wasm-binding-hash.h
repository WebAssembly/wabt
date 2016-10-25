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

#ifndef WASM_BINDING_HASH_H_
#define WASM_BINDING_HASH_H_

#include "wasm-common.h"
#include "wasm-vector.h"

struct WasmAllocator;

typedef struct WasmBinding {
  WasmLocation loc;
  WasmStringSlice name;
  int index;
} WasmBinding;

typedef struct WasmBindingHashEntry {
  WasmBinding binding;
  struct WasmBindingHashEntry* next;
  struct WasmBindingHashEntry* prev; /* only valid when this entry is unused */
} WasmBindingHashEntry;
WASM_DEFINE_VECTOR(binding_hash_entry, WasmBindingHashEntry);

typedef struct WasmBindingHash {
  WasmBindingHashEntryVector entries;
  WasmBindingHashEntry* free_head;
} WasmBindingHash;

WASM_EXTERN_C_BEGIN
WasmBinding* wasm_insert_binding(struct WasmAllocator*,
                                 WasmBindingHash*,
                                 const WasmStringSlice*);
WasmBool wasm_hash_entry_is_free(const WasmBindingHashEntry*);
/* returns -1 if the name is not in the hash */
int wasm_find_binding_index_by_name(const WasmBindingHash*,
                                    const WasmStringSlice* name);
void wasm_destroy_binding_hash(struct WasmAllocator*, WasmBindingHash*);
WASM_EXTERN_C_END

#endif /* WASM_BINDING_HASH_H_ */
