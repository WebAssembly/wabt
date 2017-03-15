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

#include <string>
#include <unordered_map>

namespace wabt {

struct Binding {
  explicit Binding(int index) : index(index) {
    WABT_ZERO_MEMORY(loc);
  }
  Binding(const Location& loc, int index) : loc(loc), index(index) {}

  Location loc;
  int index;
};

typedef std::unordered_multimap<std::string, Binding> BindingHash;

typedef void (*DuplicateBindingCallback)(const BindingHash::value_type& a,
                                         const BindingHash::value_type& b,
                                         void* user_data);
void find_duplicate_bindings(const BindingHash&,
                             DuplicateBindingCallback callback,
                             void* user_data);

inline int find_binding_index_by_name(const BindingHash& hash,
                                      const StringSlice& name) {
  auto iter = hash.find(string_slice_to_string(name));
  if (iter != hash.end())
    return iter->second.index;
  return -1;
}

}  // namespace wabt

#endif /* WABT_BINDING_HASH_H_ */
