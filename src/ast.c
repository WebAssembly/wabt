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

#include "ast.h"

#include <assert.h>
#include <stddef.h>

#include "allocator.h"

int wasm_get_index_from_var(const WasmBindingHash* hash, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return wasm_find_binding_index_by_name(hash, &var->name);
  return (int)var->index;
}

WasmExportPtr wasm_get_export_by_name(const WasmModule* module,
                                      const WasmStringSlice* name) {
  int index = wasm_find_binding_index_by_name(&module->export_bindings, name);
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

int wasm_get_table_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->table_bindings, var);
}

int wasm_get_memory_index_by_var(const WasmModule* module, const WasmVar* var) {
  return wasm_get_index_from_var(&module->memory_bindings, var);
}

int wasm_get_func_type_index_by_var(const WasmModule* module,
                                    const WasmVar* var) {
  return wasm_get_index_from_var(&module->func_type_bindings, var);
}

int wasm_get_local_index_by_var(const WasmFunc* func, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_INDEX)
    return (int)var->index;

  int result =
      wasm_find_binding_index_by_name(&func->param_bindings, &var->name);
  if (result != -1)
    return result;

  result = wasm_find_binding_index_by_name(&func->local_bindings, &var->name);
  if (result == -1)
    return result;

  /* the locals start after all the params */
  return func->decl.sig.param_types.size + result;
}

int wasm_get_module_index_by_var(const WasmScript* script, const WasmVar* var) {
  return wasm_get_index_from_var(&script->module_bindings, var);
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

WasmTablePtr wasm_get_table_by_var(const WasmModule* module,
                                   const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->table_bindings, var);
  if (index < 0 || (size_t)index >= module->tables.size)
    return NULL;
  return module->tables.data[index];
}

WasmMemoryPtr wasm_get_memory_by_var(const WasmModule* module,
                                   const WasmVar* var) {
  int index = wasm_get_index_from_var(&module->memory_bindings, var);
  if (index < 0 || (size_t)index >= module->memories.size)
    return NULL;
  return module->memories.data[index];
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

WasmModule* wasm_get_first_module(const WasmScript* script) {
  size_t i;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type == WASM_COMMAND_TYPE_MODULE)
      return &command->module;
  }
  return NULL;
}

WasmModule* wasm_get_module_by_var(const WasmScript* script,
                                   const WasmVar* var) {
  int index = wasm_get_index_from_var(&script->module_bindings, var);
  if (index < 0 || (size_t)index >= script->commands.size)
    return NULL;
  WasmCommand* command = &script->commands.data[index];
  assert(command->type == WASM_COMMAND_TYPE_MODULE);
  return &command->module;
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
  V(WASM_EXPR_TYPE_CALL_INDIRECT, call_indirect, call_indirect) \
  V(WASM_EXPR_TYPE_COMPARE, compare, compare)                   \
  V(WASM_EXPR_TYPE_CONST, const, const_)                        \
  V(WASM_EXPR_TYPE_CONVERT, convert, convert)                   \
  V(WASM_EXPR_TYPE_GET_GLOBAL, get_global, get_global)          \
  V(WASM_EXPR_TYPE_GET_LOCAL, get_local, get_local)             \
  V(WASM_EXPR_TYPE_IF, if, if_)                                 \
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
  wasm_destroy_type_vector(allocator, &sig->result_types);
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
  wasm_destroy_type_vector(allocator, &block->sig);
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
      wasm_destroy_expr_list(allocator, expr->if_.false_);
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
  wasm_destroy_binding_hash(allocator, &func->param_bindings);
  wasm_destroy_binding_hash(allocator, &func->local_bindings);
  wasm_destroy_expr_list(allocator, func->first_expr);
}

void wasm_destroy_global(WasmAllocator* allocator, WasmGlobal* global) {
  wasm_destroy_string_slice(allocator, &global->name);
  wasm_destroy_expr_list(allocator, global->init_expr);
}

void wasm_destroy_import(WasmAllocator* allocator, WasmImport* import) {
  wasm_destroy_string_slice(allocator, &import->module_name);
  wasm_destroy_string_slice(allocator, &import->field_name);
  switch (import->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      wasm_destroy_func(allocator, &import->func);
      break;
    case WASM_EXTERNAL_KIND_TABLE:
      wasm_destroy_table(allocator, &import->table);
      break;
    case WASM_EXTERNAL_KIND_MEMORY:
      wasm_destroy_memory(allocator, &import->memory);
      break;
    case WASM_EXTERNAL_KIND_GLOBAL:
      wasm_destroy_global(allocator, &import->global);
      break;
    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
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
  wasm_destroy_var(allocator, &data->memory_var);
  wasm_destroy_expr_list(allocator, data->offset);
  wasm_free(allocator, data->data);
}

void wasm_destroy_memory(struct WasmAllocator* allocator, WasmMemory* memory) {
  wasm_destroy_string_slice(allocator, &memory->name);
}

void wasm_destroy_table(struct WasmAllocator* allocator, WasmTable* table) {
  wasm_destroy_string_slice(allocator, &table->name);
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
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
      wasm_destroy_func_type(allocator, &field->func_type);
      break;
    case WASM_MODULE_FIELD_TYPE_TABLE:
      wasm_destroy_table(allocator, &field->table);
      break;
    case WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT:
      wasm_destroy_elem_segment(allocator, &field->elem_segment);
      break;
    case WASM_MODULE_FIELD_TYPE_MEMORY:
      wasm_destroy_memory(allocator, &field->memory);
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
  wasm_destroy_string_slice(allocator, &module->name);

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
  wasm_destroy_global_ptr_vector(allocator, &module->globals);
  wasm_destroy_import_ptr_vector(allocator, &module->imports);
  wasm_destroy_export_ptr_vector(allocator, &module->exports);
  wasm_destroy_func_type_ptr_vector(allocator, &module->func_types);
  wasm_destroy_table_ptr_vector(allocator, &module->tables);
  wasm_destroy_elem_segment_ptr_vector(allocator, &module->elem_segments);
  wasm_destroy_memory_ptr_vector(allocator, &module->memories);
  wasm_destroy_data_segment_ptr_vector(allocator, &module->data_segments);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->global_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->export_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_type_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->table_bindings.entries);
  wasm_destroy_binding_hash_entry_vector(allocator,
                                         &module->memory_bindings.entries);
}

void wasm_destroy_raw_module(WasmAllocator* allocator, WasmRawModule* raw) {
  if (raw->type == WASM_RAW_MODULE_TYPE_TEXT) {
    wasm_destroy_module(allocator, raw->text);
    wasm_free(allocator, raw->text);
  } else {
    wasm_destroy_string_slice(allocator, &raw->binary.name);
    wasm_free(allocator, raw->binary.data);
  }
}

void wasm_destroy_action(WasmAllocator* allocator, WasmAction* action) {
  wasm_destroy_var(allocator, &action->module_var);
  switch (action->type) {
    case WASM_ACTION_TYPE_INVOKE:
      wasm_destroy_string_slice(allocator, &action->invoke.name);
      wasm_destroy_const_vector(allocator, &action->invoke.args);
      break;
    case WASM_ACTION_TYPE_GET:
      wasm_destroy_string_slice(allocator, &action->get.name);
      break;
  }
}

void wasm_destroy_command(WasmAllocator* allocator, WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      wasm_destroy_module(allocator, &command->module);
      break;
    case WASM_COMMAND_TYPE_ACTION:
      wasm_destroy_action(allocator, &command->action);
      break;
    case WASM_COMMAND_TYPE_REGISTER:
      wasm_destroy_string_slice(allocator, &command->register_.module_name);
      wasm_destroy_var(allocator, &command->register_.var);
      break;
    case WASM_COMMAND_TYPE_ASSERT_MALFORMED:
      wasm_destroy_raw_module(allocator, &command->assert_malformed.module);
      wasm_destroy_string_slice(allocator, &command->assert_malformed.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      wasm_destroy_raw_module(allocator, &command->assert_invalid.module);
      wasm_destroy_string_slice(allocator, &command->assert_invalid.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_UNLINKABLE:
      wasm_destroy_raw_module(allocator, &command->assert_unlinkable.module);
      wasm_destroy_string_slice(allocator, &command->assert_unlinkable.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      wasm_destroy_raw_module(allocator,
                              &command->assert_uninstantiable.module);
      wasm_destroy_string_slice(allocator,
                                &command->assert_uninstantiable.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      wasm_destroy_action(allocator, &command->assert_return.action);
      wasm_destroy_const_vector(allocator, &command->assert_return.expected);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      wasm_destroy_action(allocator, &command->assert_return_nan.action);
      break;
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      wasm_destroy_action(allocator, &command->assert_trap.action);
      wasm_destroy_string_slice(allocator, &command->assert_trap.text);
      break;
    case WASM_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

void wasm_destroy_command_vector_and_elements(WasmAllocator* allocator,
                                              WasmCommandVector* commands) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *commands, command);
}

void wasm_destroy_elem_segment(WasmAllocator* allocator,
                               WasmElemSegment* elem) {
  wasm_destroy_var(allocator, &elem->table_var);
  wasm_destroy_expr_list(allocator, elem->offset);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, elem->vars, var);
}

void wasm_destroy_script(WasmScript* script) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(script->allocator, script->commands,
                                   command);
  wasm_destroy_binding_hash(script->allocator, &script->module_bindings);
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

WasmResult wasm_visit_expr_list(WasmExpr* first, WasmExprVisitor* visitor) {
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
      CHECK_RESULT(wasm_visit_expr_list(expr->block.first, visitor));
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
      CHECK_RESULT(wasm_visit_expr_list(expr->if_.true_.first, visitor));
      CALLBACK(after_if_true_expr);
      CHECK_RESULT(wasm_visit_expr_list(expr->if_.false_, visitor));
      CALLBACK(end_if_expr);
      break;

    case WASM_EXPR_TYPE_LOAD:
      CALLBACK(on_load_expr);
      break;

    case WASM_EXPR_TYPE_LOOP:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(wasm_visit_expr_list(expr->loop.first, visitor));
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
  return wasm_visit_expr_list(func->first_expr, visitor);
}
