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

int wabt_get_index_from_var(const WabtBindingHash* hash, const WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME)
    return wabt_find_binding_index_by_name(hash, &var->name);
  return (int)var->index;
}

WabtExportPtr wabt_get_export_by_name(const WabtModule* module,
                                      const WabtStringSlice* name) {
  int index = wabt_find_binding_index_by_name(&module->export_bindings, name);
  if (index == -1)
    return nullptr;
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
    return nullptr;
  return module->funcs.data[index];
}

WabtGlobalPtr wabt_get_global_by_var(const WabtModule* module,
                                     const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->global_bindings, var);
  if (index < 0 || (size_t)index >= module->globals.size)
    return nullptr;
  return module->globals.data[index];
}

WabtTablePtr wabt_get_table_by_var(const WabtModule* module,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->table_bindings, var);
  if (index < 0 || (size_t)index >= module->tables.size)
    return nullptr;
  return module->tables.data[index];
}

WabtMemoryPtr wabt_get_memory_by_var(const WabtModule* module,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->memory_bindings, var);
  if (index < 0 || (size_t)index >= module->memories.size)
    return nullptr;
  return module->memories.data[index];
}

WabtFuncTypePtr wabt_get_func_type_by_var(const WabtModule* module,
                                          const WabtVar* var) {
  int index = wabt_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || (size_t)index >= module->func_types.size)
    return nullptr;
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
  return nullptr;
}

WabtModule* wabt_get_module_by_var(const WabtScript* script,
                                   const WabtVar* var) {
  int index = wabt_get_index_from_var(&script->module_bindings, var);
  if (index < 0 || (size_t)index >= script->commands.size)
    return nullptr;
  WabtCommand* command = &script->commands.data[index];
  assert(command->type == WABT_COMMAND_TYPE_MODULE);
  return &command->module;
}

void wabt_make_type_binding_reverse_mapping(
    const WabtTypeVector* types,
    const WabtBindingHash* bindings,
    WabtStringSliceVector* out_reverse_mapping) {
  uint32_t num_names = types->size;
  wabt_reserve_string_slices(out_reverse_mapping, num_names);
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
    if (entry->prev)
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

WabtModuleField* wabt_append_module_field(WabtModule* module) {
  WabtModuleField* result =
      (WabtModuleField*)wabt_alloc_zero(sizeof(WabtModuleField));
  if (!module->first_field)
    module->first_field = result;
  else if (module->last_field)
    module->last_field->next = result;
  module->last_field = result;
  return result;
}

WabtFuncType* wabt_append_implicit_func_type(WabtLocation* loc,
                                             WabtModule* module,
                                             WabtFuncSignature* sig) {
  WabtModuleField* field = wabt_append_module_field(module);
  field->loc = *loc;
  field->type = WABT_MODULE_FIELD_TYPE_FUNC_TYPE;
  field->func_type.sig = *sig;

  WabtFuncType* func_type_ptr = &field->func_type;
  wabt_append_func_type_ptr_value(&module->func_types, &func_type_ptr);
  return func_type_ptr;
}

#define FOREACH_EXPR_TYPE(V)                       \
  V(WABT_EXPR_TYPE_BINARY, binary)                 \
  V(WABT_EXPR_TYPE_BLOCK, block)                   \
  V(WABT_EXPR_TYPE_BR, br)                         \
  V(WABT_EXPR_TYPE_BR_IF, br_if)                   \
  V(WABT_EXPR_TYPE_BR_TABLE, br_table)             \
  V(WABT_EXPR_TYPE_CALL, call)                     \
  V(WABT_EXPR_TYPE_CALL_INDIRECT, call_indirect)   \
  V(WABT_EXPR_TYPE_COMPARE, compare)               \
  V(WABT_EXPR_TYPE_CONST, const)                   \
  V(WABT_EXPR_TYPE_CONVERT, convert)               \
  V(WABT_EXPR_TYPE_GET_GLOBAL, get_global)         \
  V(WABT_EXPR_TYPE_GET_LOCAL, get_local)           \
  V(WABT_EXPR_TYPE_IF, if)                         \
  V(WABT_EXPR_TYPE_LOAD, load)                     \
  V(WABT_EXPR_TYPE_LOOP, loop)                     \
  V(WABT_EXPR_TYPE_SET_GLOBAL, set_global)         \
  V(WABT_EXPR_TYPE_SET_LOCAL, set_local)           \
  V(WABT_EXPR_TYPE_STORE, store)                   \
  V(WABT_EXPR_TYPE_TEE_LOCAL, tee_local)           \
  V(WABT_EXPR_TYPE_UNARY, unary)                   \
  V(WABT_EXPR_TYPE_CURRENT_MEMORY, current_memory) \
  V(WABT_EXPR_TYPE_DROP, drop)                     \
  V(WABT_EXPR_TYPE_GROW_MEMORY, grow_memory)       \
  V(WABT_EXPR_TYPE_NOP, nop)                       \
  V(WABT_EXPR_TYPE_RETURN, return )                \
  V(WABT_EXPR_TYPE_SELECT, select)                 \
  V(WABT_EXPR_TYPE_UNREACHABLE, unreachable)

#define DEFINE_NEW_EXPR(type_, name)                                 \
  WabtExpr* wabt_new_##name##_expr(void) {                           \
    WabtExpr* result = (WabtExpr*)wabt_alloc_zero(sizeof(WabtExpr)); \
    result->type = type_;                                            \
    return result;                                                   \
  }
FOREACH_EXPR_TYPE(DEFINE_NEW_EXPR)
#undef DEFINE_NEW_EXPR

void wabt_destroy_var(WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_NAME)
    wabt_destroy_string_slice(&var->name);
}

void wabt_destroy_var_vector_and_elements(WabtVarVector* vars) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(*vars, var);
}

void wabt_destroy_func_signature(WabtFuncSignature* sig) {
  wabt_destroy_type_vector(&sig->param_types);
  wabt_destroy_type_vector(&sig->result_types);
}

void wabt_destroy_expr_list(WabtExpr* first) {
  WabtExpr* expr = first;
  while (expr) {
    WabtExpr* next = expr->next;
    wabt_destroy_expr(expr);
    expr = next;
  }
}

void wabt_destroy_block(WabtBlock* block) {
  wabt_destroy_string_slice(&block->label);
  wabt_destroy_type_vector(&block->sig);
  wabt_destroy_expr_list(block->first);
}

void wabt_destroy_expr(WabtExpr* expr) {
  switch (expr->type) {
    case WABT_EXPR_TYPE_BLOCK:
      wabt_destroy_block(&expr->block);
      break;
    case WABT_EXPR_TYPE_BR:
      wabt_destroy_var(&expr->br.var);
      break;
    case WABT_EXPR_TYPE_BR_IF:
      wabt_destroy_var(&expr->br_if.var);
      break;
    case WABT_EXPR_TYPE_BR_TABLE:
      WABT_DESTROY_VECTOR_AND_ELEMENTS(expr->br_table.targets, var);
      wabt_destroy_var(&expr->br_table.default_target);
      break;
    case WABT_EXPR_TYPE_CALL:
      wabt_destroy_var(&expr->call.var);
      break;
    case WABT_EXPR_TYPE_CALL_INDIRECT:
      wabt_destroy_var(&expr->call_indirect.var);
      break;
    case WABT_EXPR_TYPE_GET_GLOBAL:
      wabt_destroy_var(&expr->get_global.var);
      break;
    case WABT_EXPR_TYPE_GET_LOCAL:
      wabt_destroy_var(&expr->get_local.var);
      break;
    case WABT_EXPR_TYPE_IF:
      wabt_destroy_block(&expr->if_.true_);
      wabt_destroy_expr_list(expr->if_.false_);
      break;
    case WABT_EXPR_TYPE_LOOP:
      wabt_destroy_block(&expr->loop);
      break;
    case WABT_EXPR_TYPE_SET_GLOBAL:
      wabt_destroy_var(&expr->set_global.var);
      break;
    case WABT_EXPR_TYPE_SET_LOCAL:
      wabt_destroy_var(&expr->set_local.var);
      break;
    case WABT_EXPR_TYPE_TEE_LOCAL:
      wabt_destroy_var(&expr->tee_local.var);
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
  wabt_free(expr);
}

void wabt_destroy_func_declaration(WabtFuncDeclaration* decl) {
  wabt_destroy_var(&decl->type_var);
  if (!(decl->flags & WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE))
    wabt_destroy_func_signature(&decl->sig);
}

void wabt_destroy_func(WabtFunc* func) {
  wabt_destroy_string_slice(&func->name);
  wabt_destroy_func_declaration(&func->decl);
  wabt_destroy_type_vector(&func->local_types);
  wabt_destroy_binding_hash(&func->param_bindings);
  wabt_destroy_binding_hash(&func->local_bindings);
  wabt_destroy_expr_list(func->first_expr);
}

void wabt_destroy_global(WabtGlobal* global) {
  wabt_destroy_string_slice(&global->name);
  wabt_destroy_expr_list(global->init_expr);
}

void wabt_destroy_import(WabtImport* import) {
  wabt_destroy_string_slice(&import->module_name);
  wabt_destroy_string_slice(&import->field_name);
  switch (import->kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      wabt_destroy_func(&import->func);
      break;
    case WABT_EXTERNAL_KIND_TABLE:
      wabt_destroy_table(&import->table);
      break;
    case WABT_EXTERNAL_KIND_MEMORY:
      wabt_destroy_memory(&import->memory);
      break;
    case WABT_EXTERNAL_KIND_GLOBAL:
      wabt_destroy_global(&import->global);
      break;
    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

void wabt_destroy_export(WabtExport* export_) {
  wabt_destroy_string_slice(&export_->name);
  wabt_destroy_var(&export_->var);
}

void wabt_destroy_func_type(WabtFuncType* func_type) {
  wabt_destroy_string_slice(&func_type->name);
  wabt_destroy_func_signature(&func_type->sig);
}

void wabt_destroy_data_segment(WabtDataSegment* data) {
  wabt_destroy_var(&data->memory_var);
  wabt_destroy_expr_list(data->offset);
  wabt_free(data->data);
}

void wabt_destroy_memory(WabtMemory* memory) {
  wabt_destroy_string_slice(&memory->name);
}

void wabt_destroy_table(WabtTable* table) {
  wabt_destroy_string_slice(&table->name);
}

static void destroy_module_field(WabtModuleField* field) {
  switch (field->type) {
    case WABT_MODULE_FIELD_TYPE_FUNC:
      wabt_destroy_func(&field->func);
      break;
    case WABT_MODULE_FIELD_TYPE_GLOBAL:
      wabt_destroy_global(&field->global);
      break;
    case WABT_MODULE_FIELD_TYPE_IMPORT:
      wabt_destroy_import(&field->import);
      break;
    case WABT_MODULE_FIELD_TYPE_EXPORT:
      wabt_destroy_export(&field->export_);
      break;
    case WABT_MODULE_FIELD_TYPE_FUNC_TYPE:
      wabt_destroy_func_type(&field->func_type);
      break;
    case WABT_MODULE_FIELD_TYPE_TABLE:
      wabt_destroy_table(&field->table);
      break;
    case WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT:
      wabt_destroy_elem_segment(&field->elem_segment);
      break;
    case WABT_MODULE_FIELD_TYPE_MEMORY:
      wabt_destroy_memory(&field->memory);
      break;
    case WABT_MODULE_FIELD_TYPE_DATA_SEGMENT:
      wabt_destroy_data_segment(&field->data_segment);
      break;
    case WABT_MODULE_FIELD_TYPE_START:
      wabt_destroy_var(&field->start);
      break;
  }
}

void wabt_destroy_module(WabtModule* module) {
  wabt_destroy_string_slice(&module->name);

  WabtModuleField* field = module->first_field;
  while (field) {
    WabtModuleField* next_field = field->next;
    destroy_module_field(field);
    wabt_free(field);
    field = next_field;
  }

  /* everything that follows shares data with the module fields above, so we
   only need to destroy the containing vectors */
  wabt_destroy_func_ptr_vector(&module->funcs);
  wabt_destroy_global_ptr_vector(&module->globals);
  wabt_destroy_import_ptr_vector(&module->imports);
  wabt_destroy_export_ptr_vector(&module->exports);
  wabt_destroy_func_type_ptr_vector(&module->func_types);
  wabt_destroy_table_ptr_vector(&module->tables);
  wabt_destroy_elem_segment_ptr_vector(&module->elem_segments);
  wabt_destroy_memory_ptr_vector(&module->memories);
  wabt_destroy_data_segment_ptr_vector(&module->data_segments);
  wabt_destroy_binding_hash_entry_vector(&module->func_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(&module->global_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(&module->export_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(&module->func_type_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(&module->table_bindings.entries);
  wabt_destroy_binding_hash_entry_vector(&module->memory_bindings.entries);
}

void wabt_destroy_raw_module(WabtRawModule* raw) {
  if (raw->type == WABT_RAW_MODULE_TYPE_TEXT) {
    wabt_destroy_module(raw->text);
    wabt_free(raw->text);
  } else {
    wabt_destroy_string_slice(&raw->binary.name);
    wabt_free(raw->binary.data);
  }
}

void wabt_destroy_action(WabtAction* action) {
  wabt_destroy_var(&action->module_var);
  switch (action->type) {
    case WABT_ACTION_TYPE_INVOKE:
      wabt_destroy_string_slice(&action->invoke.name);
      wabt_destroy_const_vector(&action->invoke.args);
      break;
    case WABT_ACTION_TYPE_GET:
      wabt_destroy_string_slice(&action->get.name);
      break;
  }
}

void wabt_destroy_command(WabtCommand* command) {
  switch (command->type) {
    case WABT_COMMAND_TYPE_MODULE:
      wabt_destroy_module(&command->module);
      break;
    case WABT_COMMAND_TYPE_ACTION:
      wabt_destroy_action(&command->action);
      break;
    case WABT_COMMAND_TYPE_REGISTER:
      wabt_destroy_string_slice(&command->register_.module_name);
      wabt_destroy_var(&command->register_.var);
      break;
    case WABT_COMMAND_TYPE_ASSERT_MALFORMED:
      wabt_destroy_raw_module(&command->assert_malformed.module);
      wabt_destroy_string_slice(&command->assert_malformed.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_INVALID:
    case WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY:
      wabt_destroy_raw_module(&command->assert_invalid.module);
      wabt_destroy_string_slice(&command->assert_invalid.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_UNLINKABLE:
      wabt_destroy_raw_module(&command->assert_unlinkable.module);
      wabt_destroy_string_slice(&command->assert_unlinkable.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      wabt_destroy_raw_module(&command->assert_uninstantiable.module);
      wabt_destroy_string_slice(&command->assert_uninstantiable.text);
      break;
    case WABT_COMMAND_TYPE_ASSERT_RETURN:
      wabt_destroy_action(&command->assert_return.action);
      wabt_destroy_const_vector(&command->assert_return.expected);
      break;
    case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN:
      wabt_destroy_action(&command->assert_return_nan.action);
      break;
    case WABT_COMMAND_TYPE_ASSERT_TRAP:
    case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
      wabt_destroy_action(&command->assert_trap.action);
      wabt_destroy_string_slice(&command->assert_trap.text);
      break;
    case WABT_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

void wabt_destroy_command_vector_and_elements(WabtCommandVector* commands) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(*commands, command);
}

void wabt_destroy_elem_segment(WabtElemSegment* elem) {
  wabt_destroy_var(&elem->table_var);
  wabt_destroy_expr_list(elem->offset);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(elem->vars, var);
}

void wabt_destroy_script(WabtScript* script) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(script->commands, command);
  wabt_destroy_binding_hash(&script->module_bindings);
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
