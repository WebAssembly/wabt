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

#include "binding-hash.h"

#define INITIAL_HASH_CAPACITY 8

static size_t hash_name(const WasmStringSlice* name) {
  // FNV-1a hash
  const uint32_t fnv_prime = 0x01000193;
  const uint8_t* bp = (const uint8_t*)name->start;
  const uint8_t* be = bp + name->length;
  uint32_t hval = 0x811c9dc5;
  while (bp < be) {
    hval ^= (uint32_t)*bp++;
    hval *= fnv_prime;
  }
  return hval;
}

static WasmBindingHashEntry* hash_main_entry(const WasmBindingHash* hash,
                                             const WasmStringSlice* name) {
  return &hash->entries.data[hash_name(name) % hash->entries.capacity];
}

WasmBool wasm_hash_entry_is_free(const WasmBindingHashEntry* entry) {
  return !entry->binding.name.start;
}

static WasmBindingHashEntry* hash_new_entry(WasmBindingHash* hash,
                                            const WasmStringSlice* name) {
  WasmBindingHashEntry* entry = hash_main_entry(hash, name);
  if (!wasm_hash_entry_is_free(entry)) {
    assert(hash->free_head);
    WasmBindingHashEntry* free_entry = hash->free_head;
    hash->free_head = free_entry->next;
    if (free_entry->next)
      free_entry->next->prev = NULL;

    /* our main position is already claimed. Check to see if the entry in that
     * position is in its main position */
    WasmBindingHashEntry* other_entry =
        hash_main_entry(hash, &entry->binding.name);
    if (other_entry == entry) {
      /* yes, so add this new entry to the chain, even if it is already there */
      /* add as the second entry in the chain */
      free_entry->next = entry->next;
      entry->next = free_entry;
      entry = free_entry;
    } else {
      /* no, move the entry to the free entry */
      assert(!wasm_hash_entry_is_free(other_entry));
      while (other_entry->next != entry)
        other_entry = other_entry->next;

      other_entry->next = free_entry;
      *free_entry = *entry;
      entry->next = NULL;
    }
  } else {
    /* remove from the free list */
    if (entry->next)
      entry->next->prev = entry->prev;
    if (entry->prev)
      entry->prev->next = entry->next;
    else
      hash->free_head = entry->next;
    entry->next = NULL;
  }

  WASM_ZERO_MEMORY(entry->binding);
  entry->binding.name = *name;
  entry->prev = NULL;
  /* entry->next is set above */
  return entry;
}

static void hash_resize(WasmAllocator* allocator,
                        WasmBindingHash* hash,
                        size_t desired_capacity) {
  WasmBindingHash new_hash;
  WASM_ZERO_MEMORY(new_hash);
  /* TODO(binji): better plural */
  wasm_reserve_binding_hash_entrys(allocator, &new_hash.entries,
                                   desired_capacity);

  /* update the free list */
  size_t i;
  for (i = 0; i < new_hash.entries.capacity; ++i) {
    WasmBindingHashEntry* entry = &new_hash.entries.data[i];
    if (new_hash.free_head)
      new_hash.free_head->prev = entry;

    WASM_ZERO_MEMORY(entry->binding.name);
    entry->next = new_hash.free_head;
    new_hash.free_head = entry;
  }
  new_hash.free_head->prev = NULL;

  /* copy from the old hash to the new hash */
  for (i = 0; i < hash->entries.capacity; ++i) {
    WasmBindingHashEntry* old_entry = &hash->entries.data[i];
    if (wasm_hash_entry_is_free(old_entry))
      continue;

    WasmStringSlice* name = &old_entry->binding.name;
    WasmBindingHashEntry* new_entry = hash_new_entry(&new_hash, name);
    new_entry->binding = old_entry->binding;
  }

  /* we are sharing the WasmStringSlices, so we only need to destroy the old
   * binding vector */
  wasm_destroy_binding_hash_entry_vector(allocator, &hash->entries);
  *hash = new_hash;
}

WasmBinding* wasm_insert_binding(WasmAllocator* allocator,
                                 WasmBindingHash* hash,
                                 const WasmStringSlice* name) {
  if (hash->entries.size == 0)
    hash_resize(allocator, hash, INITIAL_HASH_CAPACITY);

  if (!hash->free_head) {
    /* no more free space, allocate more */
    hash_resize(allocator, hash, hash->entries.capacity * 2);
  }

  WasmBindingHashEntry* entry = hash_new_entry(hash, name);
  assert(entry);
  hash->entries.size++;
  return &entry->binding;
}

int wasm_find_binding_index_by_name(const WasmBindingHash* hash,
                                    const WasmStringSlice* name) {
  if (hash->entries.capacity == 0)
    return -1;

  WasmBindingHashEntry* entry = hash_main_entry(hash, name);
  do {
    if (wasm_string_slices_are_equal(&entry->binding.name, name))
      return entry->binding.index;

    entry = entry->next;
  } while (entry && !wasm_hash_entry_is_free(entry));
  return -1;
}

static void destroy_binding_hash_entry(WasmAllocator* allocator,
                                       WasmBindingHashEntry* entry) {
  wasm_destroy_string_slice(allocator, &entry->binding.name);
}

void wasm_destroy_binding_hash(WasmAllocator* allocator,
                               WasmBindingHash* hash) {
  /* Can't use WASM_DESTROY_VECTOR_AND_ELEMENTS, because it loops over size, not
   * capacity. */
  size_t i;
  for (i = 0; i < hash->entries.capacity; ++i)
    destroy_binding_hash_entry(allocator, &hash->entries.data[i]);
  wasm_destroy_binding_hash_entry_vector(allocator, &hash->entries);
}
