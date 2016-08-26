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

#include "wasm-ast.h"

#include <assert.h>
#include <stddef.h>

#include "wasm-allocator.h"

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

static int find_binding_index_by_name(const WasmBindingHash* hash,
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

int wasm_get_index_from_var(const WasmBindingHash* hash, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return find_binding_index_by_name(hash, &var->name);
  return (int)var->index;
}

WasmExportPtr wasm_get_export_by_name(const WasmModule* module,
                                      const WasmStringSlice* name) {
  int index = find_binding_index_by_name(&module->export_bindings, name);
  if (index == -1)
    return NULL;
  return module->exports.data[index];
}

int wasm_get_func_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->func_bindings, var);
}

int wasm_get_global_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->global_bindings, var);
}

int wasm_get_func_type_index_by_var(const WasmModule* module,
                                    const WasmVar* var) {
  return wasm_get_index_from_var(&module->func_type_bindings, var);
}

int wasm_get_import_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->import_bindings, var);
}

int wasm_get_local_index_by_var(const WasmFunc* func, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_INDEX)
    return (int)var->index;

  int result = find_binding_index_by_name(&func->param_bindings, &var->name);
  if (result != -1)
    return result;

  result = find_binding_index_by_name(&func->local_bindings, &var->name);
  if (result == -1)
    return result;

  /* the locals start after all the params */
  return func->decl.sig.param_types.size + result;
}

WasmFuncPtr wasm_get_func_by_var(const WasmModule* module, const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_bindings, var);
  if (index < 0 || (size_t)index >= module->funcs.size)
    return NULL;
  return module->funcs.data[index];
}

WasmGlobalPtr wasm_get_global_by_var(const WasmModule* module,
                                     const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->global_bindings, var);
  if (index < 0 || (size_t)index >= module->globals.size)
    return NULL;
  return module->globals.data[index];
}

WasmFuncTypePtr wasm_get_func_type_by_var(const WasmModule* module,
                                          const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || (size_t)index >= module->func_types.size)
    return NULL;
  return module->func_types.data[index];
}

int wasm_get_func_type_index_by_sig(const WasmModule* module,
                                    const WasmFuncSignature* sig) {
  size_t i;
  for (i = 0; i < module->func_types.size; ++i)
    if (wasm_signatures_are_equal(&module->func_types.data[i]->sig, sig))
      return i;
  return -1;
}

int wasm_get_func_type_index_by_decl(const WasmModule* module,
                                     const WasmFuncDeclaration* decl) {
  if (wasm_decl_has_func_type(decl)) {
    return wasm_get_func_type_index_by_var(module, &decl->type_var);
  } else {
    return wasm_get_func_type_index_by_sig(module, &decl->sig);
  }
}

WasmImportPtr wasm_get_import_by_var(const WasmModule* module,
                                     const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->import_bindings, var);
  if (index < 0 || (size_t)index >= module->imports.size)
    return NULL;
  return module->imports.data[index];
}

void wasm_make_type_binding_reverse_mapping(
    struct WasmAllocator* allocator,
    const WasmTypeVector* types,
    const WasmBindingHash* bindings,
    WasmStringSliceVector* out_reverse_mapping) {
  uint32_t num_names = types->size;
  wasm_reserve_string_slices(allocator, out_reverse_mapping, num_names);
  out_reverse_mapping->size = num_names;
  memset(out_reverse_mapping->data, 0, num_names * sizeof(WasmStringSlice));

  /* map index to name */
  size_t i;
  for (i = 0; i < bindings->entries.capacity; ++i) {
    const WasmBindingHashEntry* entry = &bindings->entries.data[i];
    if (wasm_hash_entry_is_free(entry))
      continue;

    uint32_t index = entry->binding.index;
    assert(index < out_reverse_mapping->size);
    out_reverse_mapping->data[index] = entry->binding.name;
  }
}

WasmModuleField* wasm_append_module_field(struct WasmAllocator* allocator,
                                          WasmModule* module) {
  WasmModuleField* result =
      wasm_alloc_zero(allocator, sizeof(WasmModuleField), WASM_DEFAULT_ALIGN);
  if (!module->first_field)
    module->first_field = result;
  else if (module->last_field)
    module->last_field->next = result;
  module->last_field = result;
  return result;
}

WasmFuncType* wasm_append_implicit_func_type(struct WasmAllocator* allocator,
                                             WasmLocation* loc,
                                             WasmModule* module,
                                             WasmFuncSignature* sig) {
  WasmModuleField* field = wasm_append_module_field(allocator, module);
  field->loc = *loc;
  field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;
  field->func_type.sig = *sig;

  WasmFuncType* func_type_ptr = &field->func_type;
  wasm_append_func_type_ptr_value(allocator, &module->func_types,
                                  &func_type_ptr);
  return func_type_ptr;
}

#define ALLOC_EXPR_TYPE_ZERO(allocator, member)                                \
  wasm_alloc_zero(allocator,                                                   \
                  offsetof(WasmExpr, member) + sizeof(((WasmExpr*)0)->member), \
                  WASM_DEFAULT_ALIGN)

#define FOREACH_EXPR_TYPE(V)                                    \
  V(WASM_EXPR_TYPE_BINARY, binary, binary)                      \
  V(WASM_EXPR_TYPE_BLOCK, block, block)                         \
  V(WASM_EXPR_TYPE_BR, br, br)                                  \
  V(WASM_EXPR_TYPE_BR_IF, br_if, br_if)                         \
  V(WASM_EXPR_TYPE_BR_TABLE, br_table, br_table)                \
  V(WASM_EXPR_TYPE_CALL, call, call)                            \
  V(WASM_EXPR_TYPE_CALL_IMPORT, call_import, call_import)       \
  V(WASM_EXPR_TYPE_CALL_INDIRECT, call_indirect, call_indirect) \
  V(WASM_EXPR_TYPE_COMPARE, compare, compare)                   \
  V(WASM_EXPR_TYPE_CONST, const, const_)                        \
  V(WASM_EXPR_TYPE_CONVERT, convert, convert)                   \
  V(WASM_EXPR_TYPE_GET_GLOBAL, get_global, get_global)          \
  V(WASM_EXPR_TYPE_GET_LOCAL, get_local, get_local)             \
  V(WASM_EXPR_TYPE_IF, if, if_)                                 \
  V(WASM_EXPR_TYPE_IF_ELSE, if_else, if_else)                   \
  V(WASM_EXPR_TYPE_LOAD, load, load)                            \
  V(WASM_EXPR_TYPE_LOOP, loop, loop)                            \
  V(WASM_EXPR_TYPE_SET_GLOBAL, set_global, set_global)          \
  V(WASM_EXPR_TYPE_SET_LOCAL, set_local, set_local)             \
  V(WASM_EXPR_TYPE_STORE, store, store)                         \
  V(WASM_EXPR_TYPE_TEE_LOCAL, tee_local, tee_local)             \
  V(WASM_EXPR_TYPE_UNARY, unary, unary)

#define DEFINE_NEW_EXPR(type_, name, member)                    \
  WasmExpr* wasm_new_##name##_expr(WasmAllocator* allocator) {  \
    WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, member); \
    result->type = type_;                                       \
    return result;                                              \
  }
FOREACH_EXPR_TYPE(DEFINE_NEW_EXPR)
#undef DEFINE_NEW_EXPR

#define FOREACH_EMPTY_EXPR_TYPE(V)                 \
  V(WASM_EXPR_TYPE_CURRENT_MEMORY, current_memory) \
  V(WASM_EXPR_TYPE_DROP, drop)                     \
  V(WASM_EXPR_TYPE_GROW_MEMORY, grow_memory)       \
  V(WASM_EXPR_TYPE_NOP, nop)                       \
  V(WASM_EXPR_TYPE_RETURN, return )                \
  V(WASM_EXPR_TYPE_SELECT, select)                 \
  V(WASM_EXPR_TYPE_UNREACHABLE, unreachable)

#define DEFINE_NEW_EMPTY_EXPR(type_, name)                     \
  WasmExpr* wasm_new_##name##_expr(WasmAllocator* allocator) { \
    WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, next);  \
    result->type = type_;                                      \
    return result;                                             \
  }
FOREACH_EMPTY_EXPR_TYPE(DEFINE_NEW_EMPTY_EXPR)
#undef DEFINE_NEW_EMPTY_EXPR

WasmExpr* wasm_new_empty_expr(struct WasmAllocator* allocator,
                              WasmExprType type) {
  WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, next);
  result->type = type;
  return result;
}

static void destroy_binding_hash_entry(WasmAllocator* allocator,
                                       WasmBindingHashEntry* entry) {
  wasm_destroy_string_slice(allocator, &entry->binding.name);
}

static void destroy_binding_hash(WasmAllocator* allocator,
                                 WasmBindingHash* hash) {
  /* Can't use WASM_DESTROY_VECTOR_AND_ELEMENTS, because it loops over size, not
   * capacity. */
  size_t i;
  for (i = 0; i < hash->entries.capacity; ++i)
    destroy_binding_hash_entry(allocator, &hash->entries.data[i]);
  wasm_destroy_binding_hash_entry_vector(allocator, &hash->entries);
}

void wasm_destroy_var(WasmAllocator* allocator, WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    wasm_destroy_string_slice(allocator, &var->name);
}

void wasm_destroy_var_vector_and_elements(WasmAllocator* allocator,
                                          WasmVarVector* vars) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *vars, var);
}

void wasm_destroy_func_signature(WasmAllocator* allocator,
                                 WasmFuncSignature* sig) {
  wasm_destroy_type_vector(allocator, &sig->param_types);
}

void wasm_destroy_expr_list(WasmAllocator* allocator, WasmExpr* first) {
  WasmExpr* expr = first;
  while (expr) {
    WasmExpr* next = expr->next;
    wasm_destroy_expr(allocator, expr);
    expr = next;
  }
}

void wasm_destroy_block(WasmAllocator* allocator, WasmBlock* block) {
  wasm_destroy_string_slice(allocator, &block->label);
  wasm_destroy_expr_list(allocator, block->first);
}

void wasm_destroy_expr(WasmAllocator* allocator, WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BLOCK:
      wasm_destroy_block(allocator, &expr->block);
      break;
    case WASM_EXPR_TYPE_BR:
      wasm_destroy_var(allocator, &expr->br.var);
      break;
    case WASM_EXPR_TYPE_BR_IF:
      wasm_destroy_var(allocator, &expr->br_if.var);
      break;
    case WASM_EXPR_TYPE_BR_TABLE:
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->br_table.targets, var);
      wasm_destroy_var(allocator, &expr->br_table.default_target);
      break;
    case WASM_EXPR_TYPE_CALL:
      wasm_destroy_var(allocator, &expr->call.var);
      break;
    case WASM_EXPR_TYPE_CALL_IMPORT:
      wasm_destroy_var(allocator, &expr->call_import.var);
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      wasm_destroy_var(allocator, &expr->call_indirect.var);
      break;
    case WASM_EXPR_TYPE_GET_GLOBAL:
      wasm_destroy_var(allocator, &expr->get_global.var);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      wasm_destroy_var(allocator, &expr->get_local.var);
      break;
    case WASM_EXPR_TYPE_IF:
      wasm_destroy_block(allocator, &expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      wasm_destroy_block(allocator, &expr->if_else.true_);
      wasm_destroy_block(allocator, &expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LOOP:
      wasm_destroy_block(allocator, &expr->loop);
      break;
    case WASM_EXPR_TYPE_SET_GLOBAL:
      wasm_destroy_var(allocator, &expr->set_global.var);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      wasm_destroy_var(allocator, &expr->set_local.var);
      break;
    case WASM_EXPR_TYPE_TEE_LOCAL:
      wasm_destroy_var(allocator, &expr->tee_local.var);
      break;

    case WASM_EXPR_TYPE_BINARY:
    case WASM_EXPR_TYPE_COMPARE:
    case WASM_EXPR_TYPE_CONST:
    case WASM_EXPR_TYPE_CONVERT:
    case WASM_EXPR_TYPE_DROP:
    case WASM_EXPR_TYPE_CURRENT_MEMORY:
    case WASM_EXPR_TYPE_GROW_MEMORY:
    case WASM_EXPR_TYPE_LOAD:
    case WASM_EXPR_TYPE_NOP:
    case WASM_EXPR_TYPE_RETURN:
    case WASM_EXPR_TYPE_SELECT:
    case WASM_EXPR_TYPE_STORE:
    case WASM_EXPR_TYPE_UNARY:
    case WASM_EXPR_TYPE_UNREACHABLE:
      break;
  }
  wasm_free(allocator, expr);
}

void wasm_destroy_func_declaration(WasmAllocator* allocator,
                                   WasmFuncDeclaration* decl) {
  wasm_destroy_var(allocator, &decl->type_var);
  if (!(decl->flags & WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE))
    wasm_destroy_func_signature(allocator, &decl->sig);
}

void wasm_destroy_func(WasmAllocator* allocator, WasmFunc* func) {
  wasm_destroy_string_slice(allocator, &func->name);
  wasm_destroy_func_declaration(allocator, &func->decl);
  wasm_destroy_type_vector(allocator, &func->local_types);
  destroy_binding_hash(allocator, &func->param_bindings);
  destroy_binding_hash(allocator, &func->local_bindings);
  wasm_destroy_expr_list(allocator, func->first_expr);
}

void wasm_destroy_global(WasmAllocator* allocator, WasmGlobal* global) {
  wasm_destroy_string_slice(allocator, &global->name);
  wasm_destroy_expr_list(allocator, global->init_expr);
}

void wasm_destroy_import(WasmAllocator* allocator, WasmImport* import) {
  wasm_destroy_string_slice(allocator, &import->name);
  wasm_destroy_string_slice(allocator, &import->module_name);
  wasm_destroy_string_slice(allocator, &import->func_name);
  wasm_destroy_func_declaration(allocator, &import->decl);
}

void wasm_destroy_export(WasmAllocator* allocator, WasmExport* export) {
  wasm_destroy_string_slice(allocator, &export->name);
  wasm_destroy_var(allocator, &export->var);
}

void wasm_destroy_func_type(WasmAllocator* allocator, WasmFuncType* func_type) {
  wasm_destroy_string_slice(allocator, &func_type->name);
  wasm_destroy_func_signature(allocator, &func_type->sig);
}

void wasm_destroy_data_segment(WasmAllocator* allocator,
                               WasmDataSegment* data) {
  wasm_free(allocator, data->data);
}

static void destroy_module_field(WasmAllocator* allocator,
                                 WasmModuleField* field) {
  switch (field->type) {
    case WASM_MODULE_FIELD_TYPE_FUNC:
      wasm_destroy_func(allocator, &field->func);
      break;
    case WASM_MODULE_FIELD_TYPE_GLOBAL:
      wasm_destroy_global(allocator, &field->global);
      break;
    case WASM_MODULE_FIELD_TYPE_IMPORT:
      wasm_destroy_import(allocator, &field->import);
      break;
    case WASM_MODULE_FIELD_TYPE_EXPORT:
      wasm_destroy_export(allocator, &field->export_);
      break;
    case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
      wasm_destroy_string_slice(allocator, &field->export_memory.name);
      break;
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
      wasm_destroy_func_type(allocator, &field->func_type);
      break;
    case WASM_MODULE_FIELD_TYPE_TABLE:
      /* nothing to destroy */
      break;
    case WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT:
      wasm_destroy_elem_segment(allocator, &field->elem_segment);
      break;
    case WASM_MODULE_FIELD_TYPE_MEMORY:
      /* nothing to destroy */
      break;
    case WASM_MODULE_FIELD_TYPE_DATA_SEGMENT:
      wasm_destroy_data_segment(allocator, &field->data_segment);
      break;
    case WASM_MODULE_FIELD_TYPE_START:
      wasm_destroy_var(allocator, &field->start);
      break;
  }
}

void wasm_destroy_module(WasmAllocator* allocator, WasmModule* module) {
  WasmModuleField* field = module->first_field;
  while (field != NULL) {
    WasmModuleField* next_field = field->next;
    destroy_module_field(allocator, field);
    wasm_free(allocator, field);
    field = next_field;
  }

  /* everything that follows shares data with the module fields above, so we
   only need to destroy the containing vectors */
  wasm_destroy_func_ptr_vector(allocator, &module->funcs);
  wasm_destroy_import_ptr_vector(allocator, &module->imports);
  wasm_destroy_export_ptr_vector(allocator, &module->exports);
  wasm_destroy_func_type_ptr_vector(allocator, &module->func_types);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->import_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->export_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_type_bindings.entries);
}

void wasm_destroy_raw_module(WasmAllocator* allocator, WasmRawModule* raw) {
  if (raw->type == WASM_RAW_MODULE_TYPE_TEXT) {
    wasm_destroy_module(allocator, raw->text);
    wasm_free(allocator, raw->text);
  } else {
    wasm_free(allocator, raw->binary.data);
  }
}

static void destroy_invoke(WasmAllocator* allocator,
                           WasmCommandInvoke* invoke) {
  wasm_destroy_string_slice(allocator, &invoke->name);
  wasm_destroy_const_vector(allocator, &invoke->args);
}

void wasm_destroy_command(WasmAllocator* allocator, WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      wasm_destroy_module(allocator, &command->module);
      break;
    case WASM_COMMAND_TYPE_INVOKE:
      destroy_invoke(allocator, &command->invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      wasm_destroy_raw_module(allocator, &command->assert_invalid.module);
      wasm_destroy_string_slice(allocator, &command->assert_invalid.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      destroy_invoke(allocator, &command->assert_return.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      destroy_invoke(allocator, &command->assert_return_nan.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      destroy_invoke(allocator, &command->assert_trap.invoke);
      wasm_destroy_string_slice(allocator, &command->assert_trap.text);
      break;
    default:
      assert(0);
      break;
  }
}

void wasm_destroy_command_vector_and_elements(WasmAllocator* allocator,
                                              WasmCommandVector* commands) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *commands, command);
}

void wasm_destroy_elem_segment(WasmAllocator* allocator, WasmElemSegment* segment) {
  wasm_destroy_expr_list(allocator, segment->offset);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, segment->vars, var);
}

void wasm_destroy_script(WasmScript* script) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(script->allocator, script->commands,
                                   command);
}

#define CHECK_RESULT(expr)   \
  do {                       \
    if (WASM_FAILED((expr))) \
      return WASM_ERROR;     \
  } while (0)

#define CALLBACK(member)                                           \
  CHECK_RESULT((visitor)->member                                   \
                   ? (visitor)->member(expr, (visitor)->user_data) \
                   : WASM_OK)

static WasmResult visit_expr(WasmExpr* expr, WasmExprVisitor* visitor);

static WasmResult visit_expr_list(WasmExpr* first, WasmExprVisitor* visitor) {
  WasmExpr* expr;
  for (expr = first; expr; expr = expr->next)
    CHECK_RESULT(visit_expr(expr, visitor));
  return WASM_OK;
}

static WasmResult visit_expr(WasmExpr* expr, WasmExprVisitor* visitor) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      CALLBACK(on_binary_expr);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      CALLBACK(begin_block_expr);
      CHECK_RESULT(visit_expr_list(expr->block.first, visitor));
      CALLBACK(end_block_expr);
      break;

    case WASM_EXPR_TYPE_BR:
      CALLBACK(on_br_expr);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      CALLBACK(on_br_if_expr);
      break;

    case WASM_EXPR_TYPE_BR_TABLE:
      CALLBACK(on_br_table_expr);
      break;

    case WASM_EXPR_TYPE_CALL:
      CALLBACK(on_call_expr);
      break;

    case WASM_EXPR_TYPE_CALL_IMPORT:
      CALLBACK(on_call_import_expr);
      break;

    case WASM_EXPR_TYPE_CALL_INDIRECT:
      CALLBACK(on_call_indirect_expr);
      break;

    case WASM_EXPR_TYPE_COMPARE:
      CALLBACK(on_compare_expr);
      break;

    case WASM_EXPR_TYPE_CONST:
      CALLBACK(on_const_expr);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      CALLBACK(on_convert_expr);
      break;

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      CALLBACK(on_current_memory_expr);
      break;

    case WASM_EXPR_TYPE_DROP:
      CALLBACK(on_drop_expr);
      break;

    case WASM_EXPR_TYPE_GET_GLOBAL:
      CALLBACK(on_get_global_expr);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      CALLBACK(on_get_local_expr);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      CALLBACK(on_grow_memory_expr);
      break;

    case WASM_EXPR_TYPE_IF:
      CALLBACK(begin_if_expr);
      CHECK_RESULT(visit_expr_list(expr->if_.true_.first, visitor));
      CALLBACK(end_if_expr);
      break;

    case WASM_EXPR_TYPE_IF_ELSE:
      CALLBACK(begin_if_else_expr);
      CHECK_RESULT(visit_expr_list(expr->if_else.true_.first, visitor));
      CALLBACK(after_if_else_true_expr);
      CHECK_RESULT(visit_expr_list(expr->if_else.false_.first, visitor));
      CALLBACK(end_if_else_expr);
      break;

    case WASM_EXPR_TYPE_LOAD:
      CALLBACK(on_load_expr);
      break;

    case WASM_EXPR_TYPE_LOOP:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(visit_expr_list(expr->loop.first, visitor));
      CALLBACK(end_loop_expr);
      break;

    case WASM_EXPR_TYPE_NOP:
      CALLBACK(on_nop_expr);
      break;

    case WASM_EXPR_TYPE_RETURN:
      CALLBACK(on_return_expr);
      break;

    case WASM_EXPR_TYPE_SELECT:
      CALLBACK(on_select_expr);
      break;

    case WASM_EXPR_TYPE_SET_GLOBAL:
      CALLBACK(on_set_global_expr);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      CALLBACK(on_set_local_expr);
      break;

    case WASM_EXPR_TYPE_STORE:
      CALLBACK(on_store_expr);
      break;

    case WASM_EXPR_TYPE_TEE_LOCAL:
      CALLBACK(on_tee_local_expr);
      break;

    case WASM_EXPR_TYPE_UNARY:
      CALLBACK(on_unary_expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      CALLBACK(on_unreachable_expr);
      break;
  }

  return WASM_OK;
}

/* TODO(binji): make the visitor non-recursive */
WasmResult wasm_visit_func(WasmFunc* func, WasmExprVisitor* visitor) {
  return visit_expr_list(func->first_expr, visitor);
}
