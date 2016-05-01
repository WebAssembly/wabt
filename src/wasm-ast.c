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

static size_t wasm_hash_name(const WasmStringSlice* name) {
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

static WasmBindingHashEntry* wasm_hash_main_entry(const WasmBindingHash* hash,
                                                  const WasmStringSlice* name) {
  return &hash->entries.data[wasm_hash_name(name) % hash->entries.capacity];
}

WasmBool wasm_hash_entry_is_free(const WasmBindingHashEntry* entry) {
  return !entry->binding.name.start;
}

static WasmBindingHashEntry* wasm_hash_new_entry(WasmBindingHash* hash,
                                                 const WasmStringSlice* name) {
  WasmBindingHashEntry* entry = wasm_hash_main_entry(hash, name);
  if (!wasm_hash_entry_is_free(entry)) {
    assert(hash->free_head);
    WasmBindingHashEntry* free_entry = hash->free_head;
    hash->free_head = free_entry->next;
    if (free_entry->next)
      free_entry->next->prev = NULL;

    /* our main position is already claimed. Check to see if the entry in that
     * position is in its main position */
    WasmBindingHashEntry* other_entry =
        wasm_hash_main_entry(hash, &entry->binding.name);
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

static WasmResult wasm_hash_resize(WasmAllocator* allocator,
                                   WasmBindingHash* hash,
                                   size_t desired_capacity) {
  WasmBindingHash new_hash;
  WASM_ZERO_MEMORY(new_hash);
  /* TODO(binji): better plural */
  if (WASM_FAILED(wasm_reserve_binding_hash_entrys(allocator, &new_hash.entries,
                                                   desired_capacity))) {
    return WASM_ERROR;
  }

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
    WasmBindingHashEntry* new_entry = wasm_hash_new_entry(&new_hash, name);
    new_entry->binding = old_entry->binding;
  }

  /* we are sharing the WasmStringSlices, so we only need to destroy the old
   * binding vector */
  wasm_destroy_binding_hash_entry_vector(allocator, &hash->entries);
  *hash = new_hash;
  return WASM_OK;
}

WasmBinding* wasm_insert_binding(WasmAllocator* allocator,
                                 WasmBindingHash* hash,
                                 const WasmStringSlice* name) {
  if (hash->entries.size == 0) {
    if (WASM_FAILED(wasm_hash_resize(allocator, hash, INITIAL_HASH_CAPACITY)))
      return NULL;
  }

  if (!hash->free_head) {
    /* no more free space, allocate more */
    if (WASM_FAILED(
            wasm_hash_resize(allocator, hash, hash->entries.capacity * 2))) {
      return NULL;
    }
  }

  WasmBindingHashEntry* entry = wasm_hash_new_entry(hash, name);
  assert(entry);
  hash->entries.size++;
  return &entry->binding;
}

static int find_binding_index_by_name(const WasmBindingHash* hash,
                                      const WasmStringSlice* name) {
  if (hash->entries.capacity == 0)
    return -1;

  WasmBindingHashEntry* entry = wasm_hash_main_entry(hash, name);
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
  return var->index;
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

int wasm_get_func_type_index_by_var(const WasmModule* module,
                                    const WasmVar* var) {
  return wasm_get_index_from_var(&module->func_type_bindings, var);
}

int wasm_get_import_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->import_bindings, var);
}

int wasm_get_local_index_by_var(const WasmFunc* func, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_INDEX)
    return var->index;

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

WasmFuncTypePtr wasm_get_func_type_by_var(const WasmModule* module,
                                          const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || (size_t)index >= module->func_types.size)
    return NULL;
  return module->func_types.data[index];
}

WasmImportPtr wasm_get_import_by_var(const WasmModule* module,
                                     const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->import_bindings, var);
  if (index < 0 || (size_t)index >= module->imports.size)
    return NULL;
  return module->imports.data[index];
}

WasmResult wasm_make_type_binding_reverse_mapping(
    struct WasmAllocator* allocator,
    const WasmTypeVector* types,
    const WasmBindingHash* bindings,
    WasmStringSliceVector* out_reverse_mapping) {
  uint32_t num_names = types->size;
  if (WASM_FAILED(wasm_reserve_string_slices(allocator, out_reverse_mapping,
                                             num_names))) {
    return WASM_ERROR;
  }
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
  return WASM_OK;
}

WasmModuleField* wasm_append_module_field(struct WasmAllocator* allocator,
                                          WasmModule* module) {
  WasmModuleField* result =
      wasm_alloc_zero(allocator, sizeof(WasmModuleField), WASM_DEFAULT_ALIGN);
  if (!result)
    return NULL;
  if (!module->first_field)
    module->first_field = result;
  else if (module->last_field)
    module->last_field->next = result;
  module->last_field = result;
  return result;
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
  V(WASM_EXPR_TYPE_CALL_IMPORT, call_import, call)              \
  V(WASM_EXPR_TYPE_CALL_INDIRECT, call_indirect, call_indirect) \
  V(WASM_EXPR_TYPE_COMPARE, compare, compare)                   \
  V(WASM_EXPR_TYPE_CONST, const, const_)                        \
  V(WASM_EXPR_TYPE_CONVERT, convert, convert)                   \
  V(WASM_EXPR_TYPE_GET_LOCAL, get_local, get_local)             \
  V(WASM_EXPR_TYPE_GROW_MEMORY, grow_memory, grow_memory)       \
  V(WASM_EXPR_TYPE_IF_ELSE, if_else, if_else)                   \
  V(WASM_EXPR_TYPE_IF, if, if_)                                 \
  V(WASM_EXPR_TYPE_LOAD, load, load)                            \
  V(WASM_EXPR_TYPE_LOOP, loop, loop)                            \
  V(WASM_EXPR_TYPE_RETURN, return, return_)                     \
  V(WASM_EXPR_TYPE_SELECT, select, select)                      \
  V(WASM_EXPR_TYPE_SET_LOCAL, set_local, set_local)             \
  V(WASM_EXPR_TYPE_STORE, store, store)                         \
  V(WASM_EXPR_TYPE_UNARY, unary, unary)

#define DEFINE_NEW_EXPR(type_, name, member)                    \
  WasmExpr* wasm_new_##name##_expr(WasmAllocator* allocator) {  \
    WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, member); \
    if (!result)                                                \
      return NULL;                                              \
    result->type = type_;                                       \
    return result;                                              \
  }
FOREACH_EXPR_TYPE(DEFINE_NEW_EXPR)
#undef DEFINE_NEW_EXPR

WasmExpr* wasm_new_empty_expr(struct WasmAllocator* allocator,
                              WasmExprType type) {
  WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, next);
  if (!result)
    return NULL;
  result->type = type;
  return result;
}

static void wasm_destroy_binding_hash_entry(WasmAllocator* allocator,
                                            WasmBindingHashEntry* entry) {
  wasm_destroy_string_slice(allocator, &entry->binding.name);
}

static void wasm_destroy_binding_hash(WasmAllocator* allocator,
                                      WasmBindingHash* hash) {
  /* Can't use WASM_DESTROY_VECTOR_AND_ELEMENTS, because it loops over size, not
   * capacity. */
  size_t i;
  for (i = 0; i < hash->entries.capacity; ++i)
    wasm_destroy_binding_hash_entry(allocator, &hash->entries.data[i]);
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
    case WASM_EXPR_TYPE_BINARY:
      wasm_destroy_expr(allocator, expr->binary.left);
      wasm_destroy_expr(allocator, expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      wasm_destroy_block(allocator, &expr->block);
      break;
    case WASM_EXPR_TYPE_BR:
      wasm_destroy_var(allocator, &expr->br.var);
      if (expr->br.expr)
        wasm_destroy_expr(allocator, expr->br.expr);
      break;
    case WASM_EXPR_TYPE_BR_IF:
      wasm_destroy_var(allocator, &expr->br_if.var);
      wasm_destroy_expr(allocator, expr->br_if.cond);
      if (expr->br_if.expr)
        wasm_destroy_expr(allocator, expr->br_if.expr);
      break;
    case WASM_EXPR_TYPE_CALL:
    case WASM_EXPR_TYPE_CALL_IMPORT:
      wasm_destroy_var(allocator, &expr->call.var);
      wasm_destroy_expr_list(allocator, expr->call.first_arg);
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      wasm_destroy_var(allocator, &expr->call_indirect.var);
      wasm_destroy_expr(allocator, expr->call_indirect.expr);
      wasm_destroy_expr_list(allocator, expr->call_indirect.first_arg);
      break;
    case WASM_EXPR_TYPE_COMPARE:
      wasm_destroy_expr(allocator, expr->compare.left);
      wasm_destroy_expr(allocator, expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONVERT:
      wasm_destroy_expr(allocator, expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      wasm_destroy_var(allocator, &expr->get_local.var);
      break;
    case WASM_EXPR_TYPE_GROW_MEMORY:
      wasm_destroy_expr(allocator, expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_IF:
      wasm_destroy_expr(allocator, expr->if_.cond);
      wasm_destroy_block(allocator, &expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      wasm_destroy_expr(allocator, expr->if_else.cond);
      wasm_destroy_block(allocator, &expr->if_else.true_);
      wasm_destroy_block(allocator, &expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LOAD:
      wasm_destroy_expr(allocator, expr->load.addr);
      break;
    case WASM_EXPR_TYPE_LOOP:
      wasm_destroy_string_slice(allocator, &expr->loop.inner);
      wasm_destroy_string_slice(allocator, &expr->loop.outer);
      wasm_destroy_expr_list(allocator, expr->loop.first);
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr)
        wasm_destroy_expr(allocator, expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      wasm_destroy_expr(allocator, expr->select.cond);
      wasm_destroy_expr(allocator, expr->select.true_);
      wasm_destroy_expr(allocator, expr->select.false_);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      wasm_destroy_var(allocator, &expr->set_local.var);
      wasm_destroy_expr(allocator, expr->set_local.expr);
      break;
    case WASM_EXPR_TYPE_STORE:
      wasm_destroy_expr(allocator, expr->store.addr);
      wasm_destroy_expr(allocator, expr->store.value);
      break;
    case WASM_EXPR_TYPE_BR_TABLE:
      wasm_destroy_expr(allocator, expr->br_table.key);
      if (expr->br_table.expr)
        wasm_destroy_expr(allocator, expr->br_table.expr);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->br_table.targets, var);
      wasm_destroy_var(allocator, &expr->br_table.default_target);
      break;
    case WASM_EXPR_TYPE_UNARY:
      wasm_destroy_expr(allocator, expr->unary.expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
    case WASM_EXPR_TYPE_CONST:
    case WASM_EXPR_TYPE_CURRENT_MEMORY:
    case WASM_EXPR_TYPE_NOP:
      break;
  }
  wasm_free(allocator, expr);
}

void wasm_destroy_func_declaration(WasmAllocator* allocator,
                                   WasmFuncDeclaration* decl) {
  wasm_destroy_var(allocator, &decl->type_var);
  wasm_destroy_func_signature(allocator, &decl->sig);
}

void wasm_destroy_func(WasmAllocator* allocator, WasmFunc* func) {
  wasm_destroy_string_slice(allocator, &func->name);
  wasm_destroy_func_declaration(allocator, &func->decl);
  wasm_destroy_type_vector(allocator, &func->local_types);
  wasm_destroy_binding_hash(allocator, &func->param_bindings);
  wasm_destroy_binding_hash(allocator, &func->local_bindings);
  wasm_destroy_expr_list(allocator, func->first_expr);
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

void wasm_destroy_func_fields(struct WasmAllocator* allocator,
                              WasmFuncField* func_field) {
  /* destroy the entire linked-list */
  while (func_field) {
    WasmFuncField* next_func_field = func_field->next;

    switch (func_field->type) {
      case WASM_FUNC_FIELD_TYPE_EXPRS:
        wasm_destroy_expr_list(allocator, func_field->first_expr);
        break;

      case WASM_FUNC_FIELD_TYPE_PARAM_TYPES:
      case WASM_FUNC_FIELD_TYPE_LOCAL_TYPES:
        wasm_destroy_type_vector(allocator, &func_field->types);
        break;

      case WASM_FUNC_FIELD_TYPE_BOUND_PARAM:
      case WASM_FUNC_FIELD_TYPE_BOUND_LOCAL:
        wasm_destroy_string_slice(allocator, &func_field->bound_type.name);
        break;

      case WASM_FUNC_FIELD_TYPE_RESULT_TYPE:
        /* nothing to free */
        break;
    }

    wasm_free(allocator, func_field);
    func_field = next_func_field;
  }
}

void wasm_destroy_segment(WasmAllocator* allocator, WasmSegment* segment) {
  wasm_free(allocator, segment->data);
}

void wasm_destroy_segment_vector_and_elements(WasmAllocator* allocator,
                                              WasmSegmentVector* segments) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *segments, segment);
}

void wasm_destroy_memory(WasmAllocator* allocator, WasmMemory* memory) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, memory->segments, segment);
}

static void wasm_destroy_module_field(WasmAllocator* allocator,
                                      WasmModuleField* field) {
  switch (field->type) {
    case WASM_MODULE_FIELD_TYPE_FUNC:
      wasm_destroy_func(allocator, &field->func);
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
    case WASM_MODULE_FIELD_TYPE_TABLE:
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, field->table, var);
      break;
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
      wasm_destroy_func_type(allocator, &field->func_type);
      break;
    case WASM_MODULE_FIELD_TYPE_MEMORY:
      wasm_destroy_memory(allocator, &field->memory);
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
    wasm_destroy_module_field(allocator, field);
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

static void wasm_destroy_invoke(WasmAllocator* allocator,
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
      wasm_destroy_invoke(allocator, &command->invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      wasm_destroy_module(allocator, &command->assert_invalid.module);
      wasm_destroy_string_slice(allocator, &command->assert_invalid.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      wasm_destroy_invoke(allocator, &command->assert_return.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      wasm_destroy_invoke(allocator, &command->assert_return_nan.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      wasm_destroy_invoke(allocator, &command->assert_trap.invoke);
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
      CALLBACK(begin_binary_expr);
      CHECK_RESULT(visit_expr(expr->binary.left, visitor));
      CHECK_RESULT(visit_expr(expr->binary.right, visitor));
      CALLBACK(end_binary_expr);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      CALLBACK(begin_block_expr);
      CHECK_RESULT(visit_expr_list(expr->block.first, visitor));
      CALLBACK(end_block_expr);
      break;

    case WASM_EXPR_TYPE_BR:
      CALLBACK(begin_br_expr);
      if (expr->br.expr)
        CHECK_RESULT(visit_expr(expr->br.expr, visitor));
      CALLBACK(end_br_expr);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      CALLBACK(begin_br_if_expr);
      if (expr->br_if.expr)
        CHECK_RESULT(visit_expr(expr->br_if.expr, visitor));
      CHECK_RESULT(visit_expr(expr->br_if.cond, visitor));
      CALLBACK(end_br_if_expr);
      break;

    case WASM_EXPR_TYPE_CALL:
      CALLBACK(begin_call_expr);
      CHECK_RESULT(visit_expr_list(expr->call.first_arg, visitor));
      CALLBACK(end_call_expr);
      break;

    case WASM_EXPR_TYPE_CALL_IMPORT:
      CALLBACK(begin_call_import_expr);
      CHECK_RESULT(visit_expr_list(expr->call.first_arg, visitor));
      CALLBACK(end_call_import_expr);
      break;

    case WASM_EXPR_TYPE_CALL_INDIRECT:
      CALLBACK(begin_call_indirect_expr);
      CHECK_RESULT(visit_expr(expr->call_indirect.expr, visitor));
      CHECK_RESULT(visit_expr_list(expr->call_indirect.first_arg, visitor));
      CALLBACK(end_call_indirect_expr);
      break;

    case WASM_EXPR_TYPE_COMPARE:
      CALLBACK(begin_compare_expr);
      CHECK_RESULT(visit_expr(expr->compare.left, visitor));
      CHECK_RESULT(visit_expr(expr->compare.right, visitor));
      CALLBACK(end_compare_expr);
      break;

    case WASM_EXPR_TYPE_CONST:
      CALLBACK(on_const_expr);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      CALLBACK(begin_convert_expr);
      CHECK_RESULT(visit_expr(expr->convert.expr, visitor));
      CALLBACK(end_convert_expr);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      CALLBACK(on_get_local_expr);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      CALLBACK(begin_grow_memory_expr);
      CHECK_RESULT(visit_expr(expr->grow_memory.expr, visitor));
      CALLBACK(end_grow_memory_expr);
      break;

    case WASM_EXPR_TYPE_IF:
      CALLBACK(begin_if_expr);
      CHECK_RESULT(visit_expr(expr->if_.cond, visitor));
      CHECK_RESULT(visit_expr_list(expr->if_.true_.first, visitor));
      CALLBACK(end_if_expr);
      break;

    case WASM_EXPR_TYPE_IF_ELSE:
      CALLBACK(begin_if_else_expr);
      CHECK_RESULT(visit_expr(expr->if_else.cond, visitor));
      CHECK_RESULT(visit_expr_list(expr->if_else.true_.first, visitor));
      CHECK_RESULT(visit_expr_list(expr->if_else.false_.first, visitor));
      CALLBACK(end_if_else_expr);
      break;

    case WASM_EXPR_TYPE_LOAD:
      CALLBACK(begin_load_expr);
      CHECK_RESULT(visit_expr(expr->load.addr, visitor));
      CALLBACK(end_load_expr);
      break;

    case WASM_EXPR_TYPE_LOOP:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(visit_expr_list(expr->loop.first, visitor));
      CALLBACK(end_loop_expr);
      break;

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      CALLBACK(on_current_memory_expr);
      break;

    case WASM_EXPR_TYPE_NOP:
      CALLBACK(on_nop_expr);
      break;

    case WASM_EXPR_TYPE_RETURN:
      CALLBACK(begin_return_expr);
      if (expr->return_.expr)
        CHECK_RESULT(visit_expr(expr->return_.expr, visitor));
      CALLBACK(end_return_expr);
      break;

    case WASM_EXPR_TYPE_SELECT:
      CALLBACK(begin_select_expr);
      CHECK_RESULT(visit_expr(expr->select.true_, visitor));
      CHECK_RESULT(visit_expr(expr->select.false_, visitor));
      CHECK_RESULT(visit_expr(expr->select.cond, visitor));
      CALLBACK(end_select_expr);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      CALLBACK(begin_set_local_expr);
      CHECK_RESULT(visit_expr(expr->set_local.expr, visitor));
      CALLBACK(end_set_local_expr);
      break;

    case WASM_EXPR_TYPE_STORE:
      CALLBACK(begin_store_expr);
      CHECK_RESULT(visit_expr(expr->store.addr, visitor));
      CHECK_RESULT(visit_expr(expr->store.value, visitor));
      CALLBACK(end_store_expr);
      break;

    case WASM_EXPR_TYPE_BR_TABLE:
      CALLBACK(begin_br_table_expr);
      CHECK_RESULT(visit_expr(expr->br_table.key, visitor));
      if (expr->br_table.expr)
        CHECK_RESULT(visit_expr(expr->br_table.expr, visitor));
      CALLBACK(end_br_table_expr);
      break;

    case WASM_EXPR_TYPE_UNARY:
      CALLBACK(begin_unary_expr);
      CHECK_RESULT(visit_expr(expr->unary.expr, visitor));
      CALLBACK(end_unary_expr);
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
