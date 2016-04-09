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
  return wasm_get_index_from_var(&func->params_and_locals.bindings, var);
}

WasmFuncPtr wasm_get_func_by_var(const WasmModule* module, const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_bindings, var);
  if (index < 0 || index >= module->funcs.size)
    return NULL;
  return module->funcs.data[index];
}

WasmFuncTypePtr wasm_get_func_type_by_var(const WasmModule* module,
                                          const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || index >= module->func_types.size)
    return NULL;
  return module->func_types.data[index];
}

WasmImportPtr wasm_get_import_by_var(const WasmModule* module,
                                     const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->import_bindings, var);
  if (index < 0 || index >= module->imports.size)
    return NULL;
  return module->imports.data[index];
}

WasmResult wasm_extend_type_bindings(WasmAllocator* allocator,
                                     WasmTypeBindings* dst,
                                     WasmTypeBindings* src) {
  size_t last_type = dst->types.size;
  if (WASM_FAILED(wasm_extend_types(allocator, &dst->types, &src->types)))
    return WASM_ERROR;

  size_t i;
  for (i = 0; i < src->bindings.entries.capacity; ++i) {
    WasmBindingHashEntry* src_entry = &src->bindings.entries.data[i];
    if (wasm_hash_entry_is_free(src_entry))
      continue;

    WasmBinding* dst_binding = wasm_insert_binding(allocator, &dst->bindings,
                                                   &src_entry->binding.name);
    if (!dst_binding)
      return WASM_ERROR;

    *dst_binding = src_entry->binding;
    dst_binding->index += last_type; /* fixup the binding index */
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
  WasmExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, type);
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

void wasm_destroy_type_bindings(WasmAllocator* allocator,
                                WasmTypeBindings* type_bindings) {
  wasm_destroy_type_vector(allocator, &type_bindings->types);
  wasm_destroy_binding_hash(allocator, &type_bindings->bindings);
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

void wasm_destroy_expr_ptr(WasmAllocator* allocator, WasmExpr** expr);

static void wasm_destroy_expr(WasmAllocator* allocator, WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      wasm_destroy_expr_ptr(allocator, &expr->binary.left);
      wasm_destroy_expr_ptr(allocator, &expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      wasm_destroy_string_slice(allocator, &expr->block.label);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->block.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_BR:
      wasm_destroy_var(allocator, &expr->br.var);
      if (expr->br.expr)
        wasm_destroy_expr_ptr(allocator, &expr->br.expr);
      break;
    case WASM_EXPR_TYPE_BR_IF:
      wasm_destroy_var(allocator, &expr->br_if.var);
      wasm_destroy_expr_ptr(allocator, &expr->br_if.cond);
      if (expr->br_if.expr)
        wasm_destroy_expr_ptr(allocator, &expr->br_if.expr);
      break;
    case WASM_EXPR_TYPE_CALL:
    case WASM_EXPR_TYPE_CALL_IMPORT:
      wasm_destroy_var(allocator, &expr->call.var);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->call.args, expr_ptr);
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      wasm_destroy_var(allocator, &expr->call_indirect.var);
      wasm_destroy_expr_ptr(allocator, &expr->call_indirect.expr);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->call_indirect.args,
                                       expr_ptr);
      break;
    case WASM_EXPR_TYPE_COMPARE:
      wasm_destroy_expr_ptr(allocator, &expr->compare.left);
      wasm_destroy_expr_ptr(allocator, &expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONVERT:
      wasm_destroy_expr_ptr(allocator, &expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      wasm_destroy_var(allocator, &expr->get_local.var);
      break;
    case WASM_EXPR_TYPE_GROW_MEMORY:
      wasm_destroy_expr_ptr(allocator, &expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_IF:
      wasm_destroy_expr_ptr(allocator, &expr->if_.cond);
      wasm_destroy_expr_ptr(allocator, &expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      wasm_destroy_expr_ptr(allocator, &expr->if_else.cond);
      wasm_destroy_expr_ptr(allocator, &expr->if_else.true_);
      wasm_destroy_expr_ptr(allocator, &expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LOAD:
      wasm_destroy_expr_ptr(allocator, &expr->load.addr);
      break;
    case WASM_EXPR_TYPE_LOOP:
      wasm_destroy_string_slice(allocator, &expr->loop.inner);
      wasm_destroy_string_slice(allocator, &expr->loop.outer);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->loop.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr)
        wasm_destroy_expr_ptr(allocator, &expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      wasm_destroy_expr_ptr(allocator, &expr->select.cond);
      wasm_destroy_expr_ptr(allocator, &expr->select.true_);
      wasm_destroy_expr_ptr(allocator, &expr->select.false_);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      wasm_destroy_var(allocator, &expr->set_local.var);
      wasm_destroy_expr_ptr(allocator, &expr->set_local.expr);
      break;
    case WASM_EXPR_TYPE_STORE:
      wasm_destroy_expr_ptr(allocator, &expr->store.addr);
      wasm_destroy_expr_ptr(allocator, &expr->store.value);
      break;
    case WASM_EXPR_TYPE_BR_TABLE:
      wasm_destroy_expr_ptr(allocator, &expr->br_table.expr);
      WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->br_table.targets, var);
      wasm_destroy_var(allocator, &expr->br_table.default_target);
      break;
    case WASM_EXPR_TYPE_UNARY:
      wasm_destroy_expr_ptr(allocator, &expr->unary.expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
    case WASM_EXPR_TYPE_CONST:
    case WASM_EXPR_TYPE_MEMORY_SIZE:
    case WASM_EXPR_TYPE_NOP:
      break;
  }
}

void wasm_destroy_expr_ptr(WasmAllocator* allocator, WasmExpr** expr) {
  wasm_destroy_expr(allocator, *expr);
  wasm_free(allocator, *expr);
}

void wasm_destroy_expr_ptr_vector_and_elements(WasmAllocator* allocator,
                                               WasmExprPtrVector* exprs) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *exprs, expr_ptr);
}

void wasm_destroy_func(WasmAllocator* allocator, WasmFunc* func) {
  wasm_destroy_string_slice(allocator, &func->name);
  wasm_destroy_var(allocator, &func->type_var);
  wasm_destroy_type_bindings(allocator, &func->params);
  wasm_destroy_type_bindings(allocator, &func->locals);
  /* params_and_locals shares binding data with params and locals */
  wasm_destroy_type_vector(allocator, &func->params_and_locals.types);
  wasm_destroy_binding_hash_entry_vector(
      allocator, &func->params_and_locals.bindings.entries);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, func->exprs, expr_ptr);
}

void wasm_destroy_import(WasmAllocator* allocator, WasmImport* import) {
  wasm_destroy_string_slice(allocator, &import->name);
  wasm_destroy_string_slice(allocator, &import->module_name);
  wasm_destroy_string_slice(allocator, &import->func_name);
  wasm_destroy_var(allocator, &import->type_var);
  wasm_destroy_func_signature(allocator, &import->func_sig);
}

void wasm_destroy_export(WasmAllocator* allocator, WasmExport* export) {
  wasm_destroy_string_slice(allocator, &export->name);
  wasm_destroy_var(allocator, &export->var);
}

void wasm_destroy_func_type(WasmAllocator* allocator, WasmFuncType* func_type) {
  wasm_destroy_string_slice(allocator, &func_type->name);
  wasm_destroy_func_signature(allocator, &func_type->sig);
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

#define CALLBACK(member)                                               \
  CHECK_RESULT((traverser)->member                                     \
                   ? (traverser)->member(expr, (traverser)->user_data) \
                   : WASM_OK)

static WasmResult traverse_expr(WasmExpr* expr, WasmExprTraverser* traverser);

static WasmResult traverse_exprs(WasmExprPtrVector* exprs,
                                 WasmExprTraverser* traverser) {
  size_t i;
  for (i = 0; i < exprs->size; ++i)
    CHECK_RESULT(traverse_expr(exprs->data[i], traverser));
  return WASM_OK;
}

static WasmResult traverse_expr(WasmExpr* expr, WasmExprTraverser* traverser) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      CALLBACK(begin_binary_expr);
      CHECK_RESULT(traverse_expr(expr->binary.left, traverser));
      CHECK_RESULT(traverse_expr(expr->binary.right, traverser));
      CALLBACK(end_binary_expr);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      CALLBACK(begin_block_expr);
      CHECK_RESULT(traverse_exprs(&expr->block.exprs, traverser));
      CALLBACK(end_block_expr);
      break;

    case WASM_EXPR_TYPE_BR:
      CALLBACK(begin_br_expr);
      if (expr->br.expr)
        CHECK_RESULT(traverse_expr(expr->br.expr, traverser));
      CALLBACK(end_br_expr);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      CALLBACK(begin_br_if_expr);
      if (expr->br_if.expr)
        CHECK_RESULT(traverse_expr(expr->br_if.expr, traverser));
      CHECK_RESULT(traverse_expr(expr->br_if.cond, traverser));
      CALLBACK(end_br_if_expr);
      break;

    case WASM_EXPR_TYPE_CALL:
      CALLBACK(begin_call_expr);
      CHECK_RESULT(traverse_exprs(&expr->call.args, traverser));
      CALLBACK(end_call_expr);
      break;

    case WASM_EXPR_TYPE_CALL_IMPORT:
      CALLBACK(begin_call_import_expr);
      CHECK_RESULT(traverse_exprs(&expr->call.args, traverser));
      CALLBACK(end_call_import_expr);
      break;

    case WASM_EXPR_TYPE_CALL_INDIRECT:
      CALLBACK(begin_call_indirect_expr);
      CHECK_RESULT(traverse_expr(expr->call_indirect.expr, traverser));
      CHECK_RESULT(traverse_exprs(&expr->call_indirect.args, traverser));
      CALLBACK(end_call_indirect_expr);
      break;

    case WASM_EXPR_TYPE_COMPARE:
      CALLBACK(begin_compare_expr);
      CHECK_RESULT(traverse_expr(expr->compare.left, traverser));
      CHECK_RESULT(traverse_expr(expr->compare.right, traverser));
      CALLBACK(end_compare_expr);
      break;

    case WASM_EXPR_TYPE_CONST:
      CALLBACK(on_const_expr);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      CALLBACK(begin_convert_expr);
      CHECK_RESULT(traverse_expr(expr->convert.expr, traverser));
      CALLBACK(end_convert_expr);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      CALLBACK(on_get_local_expr);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      CALLBACK(begin_grow_memory_expr);
      CHECK_RESULT(traverse_expr(expr->grow_memory.expr, traverser));
      CALLBACK(end_grow_memory_expr);
      break;

    case WASM_EXPR_TYPE_IF:
      CALLBACK(begin_if_expr);
      CHECK_RESULT(traverse_expr(expr->if_.cond, traverser));
      CHECK_RESULT(traverse_expr(expr->if_.true_, traverser));
      CALLBACK(end_if_expr);
      break;

    case WASM_EXPR_TYPE_IF_ELSE:
      CALLBACK(begin_if_else_expr);
      CHECK_RESULT(traverse_expr(expr->if_else.cond, traverser));
      CHECK_RESULT(traverse_expr(expr->if_else.true_, traverser));
      CHECK_RESULT(traverse_expr(expr->if_else.false_, traverser));
      CALLBACK(end_if_else_expr);
      break;

    case WASM_EXPR_TYPE_LOAD:
      CALLBACK(begin_load_expr);
      CHECK_RESULT(traverse_expr(expr->load.addr, traverser));
      CALLBACK(end_load_expr);
      break;

    case WASM_EXPR_TYPE_LOOP:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(traverse_exprs(&expr->loop.exprs, traverser));
      CALLBACK(end_loop_expr);
      break;

    case WASM_EXPR_TYPE_MEMORY_SIZE:
      CALLBACK(on_memory_size_expr);
      break;

    case WASM_EXPR_TYPE_NOP:
      CALLBACK(on_nop_expr);
      break;

    case WASM_EXPR_TYPE_RETURN:
      CALLBACK(begin_return_expr);
      if (expr->return_.expr)
        CHECK_RESULT(traverse_expr(expr->return_.expr, traverser));
      CALLBACK(end_return_expr);
      break;

    case WASM_EXPR_TYPE_SELECT:
      CALLBACK(begin_select_expr);
      CHECK_RESULT(traverse_expr(expr->select.true_, traverser));
      CHECK_RESULT(traverse_expr(expr->select.false_, traverser));
      CHECK_RESULT(traverse_expr(expr->select.cond, traverser));
      CALLBACK(end_select_expr);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      CALLBACK(begin_set_local_expr);
      CHECK_RESULT(traverse_expr(expr->set_local.expr, traverser));
      CALLBACK(end_set_local_expr);
      break;

    case WASM_EXPR_TYPE_STORE:
      CALLBACK(begin_store_expr);
      CHECK_RESULT(traverse_expr(expr->store.addr, traverser));
      CHECK_RESULT(traverse_expr(expr->store.value, traverser));
      CALLBACK(end_store_expr);
      break;

    case WASM_EXPR_TYPE_BR_TABLE:
      CALLBACK(begin_br_table_expr);
      CHECK_RESULT(traverse_expr(expr->br_table.expr, traverser));
      CALLBACK(end_br_table_expr);
      break;

    case WASM_EXPR_TYPE_UNARY:
      CALLBACK(begin_unary_expr);
      CHECK_RESULT(traverse_expr(expr->unary.expr, traverser));
      CALLBACK(end_unary_expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      CALLBACK(on_unreachable_expr);
      break;
  }

  return WASM_OK;
}

/* TODO(binji): make the traverser non-recursive */
WasmResult wasm_traverse_func(WasmFunc* func, WasmExprTraverser* traverser) {
  return traverse_exprs(&func->exprs, traverser);
}
