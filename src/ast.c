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

int wabt_get_index_from_var(const WabtBindingHash* hash, const WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME)
    return wabt_find_binding_index_by_name(hash, &var->name);
  return (int)var->index;
}

WabtExportPtr wabt_get_export_by_name(const WabtModule* module,
                                      const WabtStringSlice* name) {
  int index = wabt_find_binding_index_by_name(&module->export_bindings, name);
  if (index == -1)
    return NULL;
  return module->exports.data[index];
}

int wabt_get_func_index_by_var(const WabtModule* module, const WabtVar* var) {
  return wabt_get_index_from_var(&module->func_bindings, var);
}

int wabt_get_global_index_by_var(const WabtModule* module, const WabtVar* var) {
  return wabt_get_index_from_var(&module->global_bindings, var);
}

int wabt_get_table_index_by_var(const WabtModule* module, const WabtVar* var) {
  return wabt_get_index_from_var(&module->table_bindings, var);
}

int wabt_get_memory_index_by_var(const WabtModule* module, const WabtVar* var) {
  return wabt_get_index_from_var(&module->memory_bindings, var);
}

int wabt_get_func_type_index_by_var(const WabtModule* module,
                                    const WabtVar* var) {
  return wabt_get_index_from_var(&module->func_type_bindings, var);
}

int wabt_get_local_index_by_var(const WabtFunc* func, const WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_INDEX)
    return (int)var->index;

  int result =
      wabt_find_binding_index_by_name(&func->param_bindings, &var->name);
  if (result != -1)
    return result;

  result = wabt_find_binding_index_by_name(&func->local_bindings, &var->name);
  if (result == -1)
    return result;

  /* the locals start after all the params */
  return func->decl.sig.param_types.size + result;
}

int wabt_get_module_index_by_var(const WabtScript* script, const WabtVar* var) {
  return wabt_get_index_from_var(&script->module_bindings, var);
}

WabtFuncPtr wabt_get_func_by_var(const WabtModule* module, const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->func_bindings, var);
  if (index < 0 || (size_t)index >= module->funcs.size)
    return NULL;
  return module->funcs.data[index];
}

WabtGlobalPtr wabt_get_global_by_var(const WabtModule* module,
                                     const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->global_bindings, var);
  if (index < 0 || (size_t)index >= module->globals.size)
    return NULL;
  return module->globals.data[index];
}

WabtTablePtr wabt_get_table_by_var(const WabtModule* module,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->table_bindings, var);
  if (index < 0 || (size_t)index >= module->tables.size)
    return NULL;
  return module->tables.data[index];
}

WabtMemoryPtr wabt_get_memory_by_var(const WabtModule* module,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->memory_bindings, var);
  if (index < 0 || (size_t)index >= module->memories.size)
    return NULL;
  return module->memories.data[index];
}

WabtFuncTypePtr wabt_get_func_type_by_var(const WabtModule* module,
                                          const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || (size_t)index >= module->func_types.size)
    return NULL;
  return module->func_types.data[index];
}

int wabt_get_func_type_index_by_sig(const WabtModule* module,
                                    const WabtFuncSignature* sig) {
  size_t i;
  for (i = 0; i < module->func_types.size; ++i)
    if (wabt_signatures_are_equal(&module->func_types.data[i]->sig, sig))
      return i;
  return -1;
}

int wabt_get_func_type_index_by_decl(const WabtModule* module,
                                     const WabtFuncDeclaration* decl) {
  if (wabt_decl_has_func_type(decl)) {
    return wabt_get_func_type_index_by_var(module, &decl->type_var);
  } else {
    return wabt_get_func_type_index_by_sig(module, &decl->sig);
  }
}

WabtModule* wabt_get_first_module(const WabtScript* script) {
  size_t i;
  for (i = 0; i < script->commands.size; ++i) {
    WabtCommand* command = &script->commands.data[i];
    if (command->type == WABT_COMMAND_TYPE_MODULE)
      return &command->module;
  }
  return NULL;
}

WabtModule* wabt_get_module_by_var(const WabtScript* script,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&script->module_bindings, var);
  if (index < 0 || (size_t)index >= script->commands.size)
    return NULL;
  WabtCommand* command = &script->commands.data[index];
  assert(command->type == WABT_COMMAND_TYPE_MODULE);
  return &command->module;
}

void wabt_make_type_binding_reverse_mapping(
    struct WabtAllocator* allocator,
    const WabtTypeVector* types,
    const WabtBindingHash* bindings,
    WabtStringSliceVector* out_reverse_mapping) {
  uint32_t num_names = types->size;
  wabt_reserve_string_slices(allocator, out_reverse_mapping, num_names);
  out_reverse_mapping->size = num_names;
  memset(out_reverse_mapping->data, 0, num_names * sizeof(WabtStringSlice));

  /* map index to name */
  size_t i;
  for (i = 0; i < bindings->entries.capacity; ++i) {
    const WabtBindingHashEntry* entry = &bindings->entries.data[i];
    if (wabt_hash_entry_is_free(entry))
      continue;

    uint32_t index = entry->binding.index;
    assert(index < out_reverse_mapping->size);
    out_reverse_mapping->data[index] = entry->binding.name;
  }
}

void wabt_find_duplicate_bindings(const WabtBindingHash* bindings,
                                  WabtDuplicateBindingCallback callback,
                                  void* user_data) {
  size_t i;
  for (i = 0; i < bindings->entries.capacity; ++i) {
    WabtBindingHashEntry* entry = &bindings->entries.data[i];
    if (wabt_hash_entry_is_free(entry))
      continue;

    /* only follow the chain if this is the first entry in the chain */
    if (entry->prev != NULL)
      continue;

    WabtBindingHashEntry* a = entry;
    for (; a; a = a->next) {
      WabtBindingHashEntry* b = a->next;
      for (; b; b = b->next) {
        if (wabt_string_slices_are_equal(&a->binding.name, &b->binding.name))
          callback(a, b, user_data);
      }
    }
  }
}

WabtModuleField* wabt_append_module_field(struct WabtAllocator* allocator,
                                          WabtModule* module) {
  WabtModuleField* result =
      wabt_alloc_zero(allocator, sizeof(WabtModuleField), WABT_DEFAULT_ALIGN);
  if (!module->first_field)
    module->first_field = result;
  else if (module->last_field)
    module->last_field->next = result;
  module->last_field = result;
  return result;
}

WabtFuncType* wabt_append_implicit_func_type(struct WabtAllocator* allocator,
                                             WabtLocation* loc,
                                             WabtModule* module,
                                             WabtFuncSignature* sig) {
  WabtModuleField* field = wabt_append_module_field(allocator, module);
  field->loc = *loc;
  field->type = WABT_MODULE_FIELD_TYPE_FUNC_TYPE;
  field->func_type.sig = *sig;

  WabtFuncType* func_type_ptr = &field->func_type;
  wabt_append_func_type_ptr_value(allocator, &module->func_types,
                                  &func_type_ptr);
  return func_type_ptr;
}

#define ALLOC_EXPR_TYPE_ZERO(allocator, member)                                \
  wabt_alloc_zero(allocator,                                                   \
                  offsetof(WabtExpr, member) + sizeof(((WabtExpr*)0)->member), \
                  WABT_DEFAULT_ALIGN)

#define FOREACH_EXPR_TYPE(V)                                    \
  V(WABT_EXPR_TYPE_BINARY, binary, binary)                      \
  V(WABT_EXPR_TYPE_BLOCK, block, block)                         \
  V(WABT_EXPR_TYPE_BR, br, br)                                  \
  V(WABT_EXPR_TYPE_BR_IF, br_if, br_if)                         \
  V(WABT_EXPR_TYPE_BR_TABLE, br_table, br_table)                \
  V(WABT_EXPR_TYPE_CALL, call, call)                            \
  V(WABT_EXPR_TYPE_CALL_INDIRECT, call_indirect, call_indirect) \
  V(WABT_EXPR_TYPE_COMPARE, compare, compare)                   \
  V(WABT_EXPR_TYPE_CONST, const, const_)                        \
  V(WABT_EXPR_TYPE_CONVERT, convert, convert)                   \
  V(WABT_EXPR_TYPE_GET_GLOBAL, get_global, get_global)          \
  V(WABT_EXPR_TYPE_GET_LOCAL, get_local, get_local)             \
  V(WABT_EXPR_TYPE_IF, if, if_)                                 \
  V(WABT_EXPR_TYPE_LOAD, load, load)                            \
  V(WABT_EXPR_TYPE_LOOP, loop, loop)                            \
  V(WABT_EXPR_TYPE_SET_GLOBAL, set_global, set_global)          \
  V(WABT_EXPR_TYPE_SET_LOCAL, set_local, set_local)             \
  V(WABT_EXPR_TYPE_STORE, store, store)                         \
  V(WABT_EXPR_TYPE_TEE_LOCAL, tee_local, tee_local)             \
  V(WABT_EXPR_TYPE_UNARY, unary, unary)

#define DEFINE_NEW_EXPR(type_, name, member)                    \
  WabtExpr* wabt_new_##name##_expr(WabtAllocator* allocator) {  \
    WabtExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, member); \
    result->type = type_;                                       \
    return result;                                              \
  }
FOREACH_EXPR_TYPE(DEFINE_NEW_EXPR)
#undef DEFINE_NEW_EXPR

#define FOREACH_EMPTY_EXPR_TYPE(V)                 \
  V(WABT_EXPR_TYPE_CURRENT_MEMORY, current_memory) \
  V(WABT_EXPR_TYPE_DROP, drop)                     \
  V(WABT_EXPR_TYPE_GROW_MEMORY, grow_memory)       \
  V(WABT_EXPR_TYPE_NOP, nop)                       \
  V(WABT_EXPR_TYPE_RETURN, return )                \
  V(WABT_EXPR_TYPE_SELECT, select)                 \
  V(WABT_EXPR_TYPE_UNREACHABLE, unreachable)

#define DEFINE_NEW_EMPTY_EXPR(type_, name)                     \
  WabtExpr* wabt_new_##name##_expr(WabtAllocator* allocator) { \
    WabtExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, next);  \
    result->type = type_;                                      \
    return result;                                             \
  }
FOREACH_EMPTY_EXPR_TYPE(DEFINE_NEW_EMPTY_EXPR)
#undef DEFINE_NEW_EMPTY_EXPR

WabtExpr* wabt_new_empty_expr(struct WabtAllocator* allocator,
                              WabtExprType type) {
  WabtExpr* result = ALLOC_EXPR_TYPE_ZERO(allocator, next);
  result->type = type;
  return result;
}

void wabt_destroy_var(WabtAllocator* allocator, WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME)
    wabt_destroy_string_slice(allocator, &var->name);
}

void wabt_destroy_var_vector_and_elements(WabtAllocator* allocator,
                                          WabtVarVector* vars) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(allocator, *vars, var);
}

void wabt_destroy_func_signature(WabtAllocator* allocator,
                                 WabtFuncSignature* sig) {
  wabt_destroy_type_vector(allocator, &sig->param_types);
  wabt_destroy_type_vector(allocator, &sig->result_types);
}

void wabt_destroy_expr_list(WabtAllocator* allocator, WabtExpr* first) {
  WabtExpr* expr = first;
  while (expr) {
    WabtExpr* next = expr->next;
    wabt_destroy_expr(allocator, expr);
    expr = next;
  }
}

void wabt_destroy_block(WabtAllocator* allocator, WabtBlock* block) {
  wabt_destroy_string_slice(allocator, &block->label);
  wabt_destroy_type_vector(allocator, &block->sig);
  wabt_destroy_expr_list(allocator, block->first);
}

void wabt_destroy_expr(WabtAllocator* allocator, WabtExpr* expr) {
  switch (expr->type) {
    case WABT_EXPR_TYPE_BLOCK:
      wabt_destroy_block(allocator, &expr->block);
      break;
    case WABT_EXPR_TYPE_BR:
      wabt_destroy_var(allocator, &expr->br.var);
      break;
    case WABT_EXPR_TYPE_BR_IF:
      wabt_destroy_var(allocator, &expr->br_if.var);
      break;
    case WABT_EXPR_TYPE_BR_TABLE:
      WABT_DESTROY_VECTOR_AND_ELEMENTS(allocator, expr->br_table.targets, var);
      wabt_destroy_var(allocator, &expr->br_table.default_target);
      break;
    case WABT_EXPR_TYPE_CALL:
      wabt_destroy_var(allocator, &expr->call.var);
      break;
    case WABT_EXPR_TYPE_CALL_INDIRECT:
      wabt_destroy_var(allocator, &expr->call_indirect.var);
      break;
    case WABT_EXPR_TYPE_GET_GLOBAL:
      wabt_destroy_var(allocator, &expr->get_global.var);
      break;
    case WABT_EXPR_TYPE_GET_LOCAL:
      wabt_destroy_var(allocator, &expr->get_local.var);
      break;
    case WABT_EXPR_TYPE_IF:
      wabt_destroy_block(allocator, &expr->if_.true_);
      wabt_destroy_expr_list(allocator, expr->if_.false_);
      break;
    case WABT_EXPR_TYPE_LOOP:
      wabt_destroy_block(allocator, &expr->loop);
      break;
    case WABT_EXPR_TYPE_SET_GLOBAL:
      wabt_destroy_var(allocator, &expr->set_global.var);
      break;
    case WABT_EXPR_TYPE_SET_LOCAL:
      wabt_destroy_var(allocator, &expr->set_local.var);
      break;
    case WABT_EXPR_TYPE_TEE_LOCAL:
      wabt_destroy_var(allocator, &expr->tee_local.var);
      break;

    case WABT_EXPR_TYPE_BINARY:
    case WABT_EXPR_TYPE_COMPARE:
    case WABT_EXPR_TYPE_CONST:
    case WABT_EXPR_TYPE_CONVERT:
    case WABT_EXPR_TYPE_DROP:
    case WABT_EXPR_TYPE_CURRENT_MEMORY:
    case WABT_EXPR_TYPE_GROW_MEMORY:
    case WABT_EXPR_TYPE_LOAD:
    case WABT_EXPR_TYPE_NOP:
    case WABT_EXPR_TYPE_RETURN:
    case WABT_EXPR_TYPE_SELECT:
    case WABT_EXPR_TYPE_STORE:
    case WABT_EXPR_TYPE_UNARY:
    case WABT_EXPR_TYPE_UNREACHABLE:
      break;
  }
  wabt_free(allocator, expr);
}

void wabt_destroy_func_declaration(WabtAllocator* allocator,
                                   WabtFuncDeclaration* decl) {
  wabt_destroy_var(allocator, &decl->type_var);
  if (!(decl->flags & WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE))
    wabt_destroy_func_signature(allocator, &decl->sig);
}

void wabt_destroy_func(WabtAllocator* allocator, WabtFunc* func) {
  wabt_destroy_string_slice(allocator, &func->name);
  wabt_destroy_func_declaration(allocator, &func->decl);
  wabt_destroy_type_vector(allocator, &func->local_types);
  wabt_destroy_binding_hash(allocator, &func->param_bindings);
  wabt_destroy_binding_hash(allocator, &func->local_bindings);
  wabt_destroy_expr_list(allocator, func->first_expr);
}

void wabt_destroy_global(WabtAllocator* allocator, WabtGlobal* global) {
  wabt_destroy_string_slice(allocator, &global->name);
  wabt_destroy_expr_list(allocator, global->init_expr);
}

void wabt_destroy_import(WabtAllocator* allocator, WabtImport* import) {
  wabt_destroy_string_slice(allocator, &import->module_name);
  wabt_destroy_string_slice(allocator, &import->field_name);
  switch (import->kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      wabt_destroy_func(allocator, &import->func);
      break;
    case WABT_EXTERNAL_KIND_TABLE:
      wabt_destroy_table(allocator, &import->table);
      break;
    case WABT_EXTERNAL_KIND_MEMORY:
      wabt_destroy_memory(allocator, &import->memory);
      break;
    case WABT_EXTERNAL_KIND_GLOBAL:
      wabt_destroy_global(allocator, &import->global);
      break;
    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

void wabt_destroy_export(WabtAllocator* allocator, WabtExport* export) {
  wabt_destroy_string_slice(allocator, &export->name);
  wabt_destroy_var(allocator, &export->var);
}

void wabt_destroy_func_type(WabtAllocator* allocator, WabtFuncType* func_type) {
  wabt_destroy_string_slice(allocator, &func_type->name);
  wabt_destroy_func_signature(allocator, &func_type->sig);
}

void wabt_destroy_data_segment(WabtAllocator* allocator,
                               WabtDataSegment* data) {
  wabt_destroy_var(allocator, &data->memory_var);
  wabt_destroy_expr_list(allocator, data->offset);
  wabt_free(allocator, data->data);
}

void wabt_destroy_memory(struct WabtAllocator* allocator, WabtMemory* memory) {
  wabt_destroy_string_slice(allocator, &memory->name);
}

void wabt_destroy_table(struct WabtAllocator* allocator, WabtTable* table) {
  wabt_destroy_string_slice(allocator, &table->name);
}

static void destroy_module_field(WabtAllocator* allocator,
                                 WabtModuleField* field) {
  switch (field->type) {
    case WABT_MODULE_FIELD_TYPE_FUNC:
      wabt_destroy_func(allocator, &field->func);
      break;
    case WABT_MODULE_FIELD_TYPE_GLOBAL:
      wabt_destroy_global(allocator, &field->global);
      break;
    case WABT_MODULE_FIELD_TYPE_IMPORT:
      wabt_destroy_import(allocator, &field->import);
      break;
    case WABT_MODULE_FIELD_TYPE_EXPORT:
      wabt_destroy_export(allocator, &field->export_);
      break;
    case WABT_MODULE_FIELD_TYPE_FUNC_TYPE:
      wabt_destroy_func_type(allocator, &field->func_type);
      break;
    case WABT_MODULE_FIELD_TYPE_TABLE:
      wabt_destroy_table(allocator, &field->table);
      break;
    case WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT:
      wabt_destroy_elem_segment(allocator, &field->elem_segment);
      break;
    case WABT_MODULE_FIELD_TYPE_MEMORY:
      wabt_destroy_memory(allocator, &field->memory);
      break;
    case WABT_MODULE_FIELD_TYPE_DATA_SEGMENT:
      wabt_destroy_data_segment(allocator, &field->data_segment);
      break;
    case WABT_MODULE_FIELD_TYPE_START:
      wabt_destroy_var(allocator, &field->start);
      break;
  }
}

void wabt_destroy_module(WabtAllocator* allocator, WabtModule* module) {
  wabt_destroy_string_slice(allocator, &module->name);

  WabtModuleField* field = module->first_field;
  while (field != NULL) {
    WabtModuleField* next_field = field->next;
    destroy_module_field(allocator, field);
    wabt_free(allocator, field);
    field = next_field;
  }

  /* everything that follows shares data with the module fields above, so we
   only need to destroy the containing vectors */
  wabt_destroy_func_ptr_vector(allocator, &module->funcs);
  wabt_destroy_global_ptr_vector(allocator, &module->globals);
  wabt_destroy_import_ptr_vector(allocator, &module->imports);
  wabt_destroy_export_ptr_vector(allocator, &module->exports);
  wabt_destroy_func_type_ptr_vector(allocator, &module->func_types);
  wabt_destroy_table_ptr_vector(allocator, &module->tables);
  wabt_destroy_elem_segment_ptr_vector(allocator, &module->elem_segments);
  wabt_destroy_memory_ptr_vector(allocator, &module->memories);
  wabt_destroy_data_segment_ptr_vector(allocator, &module->data_segments);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->global_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->export_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->func_type_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->table_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(allocator,
                                         &module->memory_bindings.entries);
}

void wabt_destroy_raw_module(WabtAllocator* allocator, WabtRawModule* raw) {
  if (raw->type == WABT_RAW_MODULE_TYPE_TEXT) {
    wabt_destroy_module(allocator, raw->text);
    wabt_free(allocator, raw->text);
  } else {
    wabt_destroy_string_slice(allocator, &raw->binary.name);
    wabt_free(allocator, raw->binary.data);
  }
}

void wabt_destroy_action(WabtAllocator* allocator, WabtAction* action) {
  wabt_destroy_var(allocator, &action->module_var);
  switch (action->type) {
    case WABT_ACTION_TYPE_INVOKE:
      wabt_destroy_string_slice(allocator, &action->invoke.name);
      wabt_destroy_const_vector(allocator, &action->invoke.args);
      break;
    case WABT_ACTION_TYPE_GET:
      wabt_destroy_string_slice(allocator, &action->get.name);
      break;
  }
}

void wabt_destroy_command(WabtAllocator* allocator, WabtCommand* command) {
  switch (command->type) {
    case WABT_COMMAND_TYPE_MODULE:
      wabt_destroy_module(allocator, &command->module);
      break;
    case WABT_COMMAND_TYPE_ACTION:
      wabt_destroy_action(allocator, &command->action);
      break;
    case WABT_COMMAND_TYPE_REGISTER:
      wabt_destroy_string_slice(allocator, &command->register_.module_name);
      wabt_destroy_var(allocator, &command->register_.var);
      break;
    case WABT_COMMAND_TYPE_ASSERT_MALFORMED:
      wabt_destroy_raw_module(allocator, &command->assert_malformed.module);
      wabt_destroy_string_slice(allocator, &command->assert_malformed.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_INVALID:
    case WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY:
      wabt_destroy_raw_module(allocator, &command->assert_invalid.module);
      wabt_destroy_string_slice(allocator, &command->assert_invalid.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_UNLINKABLE:
      wabt_destroy_raw_module(allocator, &command->assert_unlinkable.module);
      wabt_destroy_string_slice(allocator, &command->assert_unlinkable.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      wabt_destroy_raw_module(allocator,
                              &command->assert_uninstantiable.module);
      wabt_destroy_string_slice(allocator,
                                &command->assert_uninstantiable.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_RETURN:
      wabt_destroy_action(allocator, &command->assert_return.action);
      wabt_destroy_const_vector(allocator, &command->assert_return.expected);
      break;
    case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN:
      wabt_destroy_action(allocator, &command->assert_return_nan.action);
      break;
    case WABT_COMMAND_TYPE_ASSERT_TRAP:
    case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
      wabt_destroy_action(allocator, &command->assert_trap.action);
      wabt_destroy_string_slice(allocator, &command->assert_trap.text);
      break;
    case WABT_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

void wabt_destroy_command_vector_and_elements(WabtAllocator* allocator,
                                              WabtCommandVector* commands) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(allocator, *commands, command);
}

void wabt_destroy_elem_segment(WabtAllocator* allocator,
                               WabtElemSegment* elem) {
  wabt_destroy_var(allocator, &elem->table_var);
  wabt_destroy_expr_list(allocator, elem->offset);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(allocator, elem->vars, var);
}

void wabt_destroy_script(WabtScript* script) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(script->allocator, script->commands,
                                   command);
  wabt_destroy_binding_hash(script->allocator, &script->module_bindings);
}

#define CHECK_RESULT(expr)   \
  do {                       \
    if (WABT_FAILED((expr))) \
      return WABT_ERROR;     \
  } while (0)

#define CALLBACK(member)                                           \
  CHECK_RESULT((visitor)->member                                   \
                   ? (visitor)->member(expr, (visitor)->user_data) \
                   : WABT_OK)

static WabtResult visit_expr(WabtExpr* expr, WabtExprVisitor* visitor);

WabtResult wabt_visit_expr_list(WabtExpr* first, WabtExprVisitor* visitor) {
  WabtExpr* expr;
  for (expr = first; expr; expr = expr->next)
    CHECK_RESULT(visit_expr(expr, visitor));
  return WABT_OK;
}

static WabtResult visit_expr(WabtExpr* expr, WabtExprVisitor* visitor) {
  switch (expr->type) {
    case WABT_EXPR_TYPE_BINARY:
      CALLBACK(on_binary_expr);
      break;

    case WABT_EXPR_TYPE_BLOCK:
      CALLBACK(begin_block_expr);
      CHECK_RESULT(wabt_visit_expr_list(expr->block.first, visitor));
      CALLBACK(end_block_expr);
      break;

    case WABT_EXPR_TYPE_BR:
      CALLBACK(on_br_expr);
      break;

    case WABT_EXPR_TYPE_BR_IF:
      CALLBACK(on_br_if_expr);
      break;

    case WABT_EXPR_TYPE_BR_TABLE:
      CALLBACK(on_br_table_expr);
      break;

    case WABT_EXPR_TYPE_CALL:
      CALLBACK(on_call_expr);
      break;

    case WABT_EXPR_TYPE_CALL_INDIRECT:
      CALLBACK(on_call_indirect_expr);
      break;

    case WABT_EXPR_TYPE_COMPARE:
      CALLBACK(on_compare_expr);
      break;

    case WABT_EXPR_TYPE_CONST:
      CALLBACK(on_const_expr);
      break;

    case WABT_EXPR_TYPE_CONVERT:
      CALLBACK(on_convert_expr);
      break;

    case WABT_EXPR_TYPE_CURRENT_MEMORY:
      CALLBACK(on_current_memory_expr);
      break;

    case WABT_EXPR_TYPE_DROP:
      CALLBACK(on_drop_expr);
      break;

    case WABT_EXPR_TYPE_GET_GLOBAL:
      CALLBACK(on_get_global_expr);
      break;

    case WABT_EXPR_TYPE_GET_LOCAL:
      CALLBACK(on_get_local_expr);
      break;

    case WABT_EXPR_TYPE_GROW_MEMORY:
      CALLBACK(on_grow_memory_expr);
      break;

    case WABT_EXPR_TYPE_IF:
      CALLBACK(begin_if_expr);
      CHECK_RESULT(wabt_visit_expr_list(expr->if_.true_.first, visitor));
      CALLBACK(after_if_true_expr);
      CHECK_RESULT(wabt_visit_expr_list(expr->if_.false_, visitor));
      CALLBACK(end_if_expr);
      break;

    case WABT_EXPR_TYPE_LOAD:
      CALLBACK(on_load_expr);
      break;

    case WABT_EXPR_TYPE_LOOP:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(wabt_visit_expr_list(expr->loop.first, visitor));
      CALLBACK(end_loop_expr);
      break;

    case WABT_EXPR_TYPE_NOP:
      CALLBACK(on_nop_expr);
      break;

    case WABT_EXPR_TYPE_RETURN:
      CALLBACK(on_return_expr);
      break;

    case WABT_EXPR_TYPE_SELECT:
      CALLBACK(on_select_expr);
      break;

    case WABT_EXPR_TYPE_SET_GLOBAL:
      CALLBACK(on_set_global_expr);
      break;

    case WABT_EXPR_TYPE_SET_LOCAL:
      CALLBACK(on_set_local_expr);
      break;

    case WABT_EXPR_TYPE_STORE:
      CALLBACK(on_store_expr);
      break;

    case WABT_EXPR_TYPE_TEE_LOCAL:
      CALLBACK(on_tee_local_expr);
      break;

    case WABT_EXPR_TYPE_UNARY:
      CALLBACK(on_unary_expr);
      break;

    case WABT_EXPR_TYPE_UNREACHABLE:
      CALLBACK(on_unreachable_expr);
      break;
  }

  return WABT_OK;
}

/* TODO(binji): make the visitor non-recursive */
WabtResult wabt_visit_func(WabtFunc* func, WabtExprVisitor* visitor) {
  return wabt_visit_expr_list(func->first_expr, visitor);
}
