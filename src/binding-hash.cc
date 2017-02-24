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

static size_t hash_name(const WabtStringSlice* name) {
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

static WabtBindingHashEntry* hash_main_entry(const WabtBindingHash* hash,
                                             const WabtStringSlice* name) {
  return &hash->entries.data[hash_name(name) % hash->entries.capacity];
}

bool wabt_hash_entry_is_free(const WabtBindingHashEntry* entry) {
  return !entry->binding.name.start;
}

static WabtBindingHashEntry* hash_new_entry(WabtBindingHash* hash,
                                            const WabtStringSlice* name) {
  WabtBindingHashEntry* entry = hash_main_entry(hash, name);
  if (!wabt_hash_entry_is_free(entry)) {
    assert(hash->free_head);
    WabtBindingHashEntry* free_entry = hash->free_head;
    hash->free_head = free_entry->next;
    if (free_entry->next)
      free_entry->next->prev = nullptr;

    /* our main position is already claimed. Check to see if the entry in that
     * position is in its main position */
    WabtBindingHashEntry* other_entry =
        hash_main_entry(hash, &entry->binding.name);
    if (other_entry == entry) {
      /* yes, so add this new entry to the chain, even if it is already there */
      /* add as the second entry in the chain */
      free_entry->next = entry->next;
      entry->next = free_entry;
      entry = free_entry;
    } else {
      /* no, move the entry to the free entry */
      assert(!wabt_hash_entry_is_free(other_entry));
      while (other_entry->next != entry)
        other_entry = other_entry->next;

      other_entry->next = free_entry;
      *free_entry = *entry;
      entry->next = nullptr;
    }
  } else {
    /* remove from the free list */
    if (entry->next)
      entry->next->prev = entry->prev;
    if (entry->prev)
      entry->prev->next = entry->next;
    else
      hash->free_head = entry->next;
    entry->next = nullptr;
  }

  WABT_ZERO_MEMORY(entry->binding);
  entry->binding.name = *name;
  entry->prev = nullptr;
  /* entry->next is set above */
  return entry;
}

static void hash_resize(WabtBindingHash* hash, size_t desired_capacity) {
  WabtBindingHash new_hash;
  WABT_ZERO_MEMORY(new_hash);
  /* TODO(binji): better plural */
  wabt_reserve_binding_hash_entrys(&new_hash.entries, desired_capacity);

  /* update the free list */
  size_t i;
  for (i = 0; i < new_hash.entries.capacity; ++i) {
    WabtBindingHashEntry* entry = &new_hash.entries.data[i];
    if (new_hash.free_head)
      new_hash.free_head->prev = entry;

    WABT_ZERO_MEMORY(entry->binding.name);
    entry->next = new_hash.free_head;
    new_hash.free_head = entry;
  }
  new_hash.free_head->prev = nullptr;

  /* copy from the old hash to the new hash */
  for (i = 0; i < hash->entries.capacity; ++i) {
    WabtBindingHashEntry* old_entry = &hash->entries.data[i];
    if (wabt_hash_entry_is_free(old_entry))
      continue;

    WabtStringSlice* name = &old_entry->binding.name;
    WabtBindingHashEntry* new_entry = hash_new_entry(&new_hash, name);
    new_entry->binding = old_entry->binding;
  }

  /* we are sharing the WabtStringSlices, so we only need to destroy the old
   * binding vector */
  wabt_destroy_binding_hash_entry_vector(&hash->entries);
  *hash = new_hash;
}

WabtBinding* wabt_insert_binding(WabtBindingHash* hash,
                                 const WabtStringSlice* name) {
  if (hash->entries.size == 0)
    hash_resize(hash, INITIAL_HASH_CAPACITY);

  if (!hash->free_head) {
    /* no more free space, allocate more */
    hash_resize(hash, hash->entries.capacity * 2);
  }

  WabtBindingHashEntry* entry = hash_new_entry(hash, name);
  assert(entry);
  hash->entries.size++;
  return &entry->binding;
}

int wabt_find_binding_index_by_name(const WabtBindingHash* hash,
                                    const WabtStringSlice* name) {
  if (hash->entries.capacity == 0)
    return -1;

  WabtBindingHashEntry* entry = hash_main_entry(hash, name);
  do {
    if (wabt_string_slices_are_equal(&entry->binding.name, name))
      return entry->binding.index;

    entry = entry->next;
  } while (entry && !wabt_hash_entry_is_free(entry));
  return -1;
}

void wabt_remove_binding(WabtBindingHash* hash, const WabtStringSlice* name) {
  int index = wabt_find_binding_index_by_name(hash, name);
  if (index == -1)
    return;

  WabtBindingHashEntry* entry = &hash->entries.data[index];
  wabt_destroy_string_slice(&entry->binding.name);
  WABT_ZERO_MEMORY(*entry);
}

static void destroy_binding_hash_entry(WabtBindingHashEntry* entry) {
  wabt_destroy_string_slice(&entry->binding.name);
}

void wabt_destroy_binding_hash(WabtBindingHash* hash) {
  /* Can't use WABT_DESTROY_VECTOR_AND_ELEMENTS, because it loops over size, not
   * capacity. */
  size_t i;
  for (i = 0; i < hash->entries.capacity; ++i)
    destroy_binding_hash_entry(&hash->entries.data[i]);
  wabt_destroy_binding_hash_entry_vector(&hash->entries);
}
