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

#ifndef WABT_BINDING_HASH_H_
#define WABT_BINDING_HASH_H_

#include "common.h"
#include "vector.h"

struct WabtBinding {
  WabtLocation loc;
  WabtStringSlice name;
  int index;
};

struct WabtBindingHashEntry {
  WabtBinding binding;
  struct WabtBindingHashEntry* next;
  struct WabtBindingHashEntry* prev; /* only valid when this entry is unused */
};
WABT_DEFINE_VECTOR(binding_hash_entry, WabtBindingHashEntry);

struct WabtBindingHash {
  WabtBindingHashEntryVector entries;
  WabtBindingHashEntry* free_head;
};

WABT_EXTERN_C_BEGIN
WabtBinding* wabt_insert_binding(WabtBindingHash*, const WabtStringSlice*);
void wabt_remove_binding(WabtBindingHash*, const WabtStringSlice*);
bool wabt_hash_entry_is_free(const WabtBindingHashEntry*);
/* returns -1 if the name is not in the hash */
int wabt_find_binding_index_by_name(const WabtBindingHash*,
                                    const WabtStringSlice* name);
void wabt_destroy_binding_hash(WabtBindingHash*);
WABT_EXTERN_C_END

#endif /* WABT_BINDING_HASH_H_ */
