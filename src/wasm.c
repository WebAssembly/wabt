#include "wasm.h"

#include <stdlib.h>

#define DESTROY_VECTOR_AND_ELEMENTS(V, name) \
  {                                          \
    int i;                                   \
    for (i = 0; i < (V).size; ++i)           \
      destroy_##name(&((V).data[i]));        \
    wasm_destroy_##name##_vector(&(V));      \
  }

static void destroy_string_slice(WasmStringSlice* str) {
  free((char*)str->start);
}

static void destroy_binding(WasmBinding* binding) {
  destroy_string_slice(&binding->name);
}

static void destroy_type_bindings(WasmTypeBindings* type_bindings) {
  wasm_destroy_type_vector(&type_bindings->types);
  DESTROY_VECTOR_AND_ELEMENTS(type_bindings->bindings, binding);
}

static void destroy_var(WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    destroy_string_slice(&var->name);
}

static void destroy_func_signature(WasmFuncSignature* sig) {
  wasm_destroy_type_vector(&sig->param_types);
}

static void destroy_target(WasmTarget* target) {
  destroy_var(&target->var);
}

static void destroy_expr_ptr(WasmExpr** expr);

static void destroy_case(WasmCase* case_) {
  destroy_string_slice(&case_->label);
  DESTROY_VECTOR_AND_ELEMENTS(case_->exprs, expr_ptr);
}

static void destroy_expr(WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      destroy_expr_ptr(&expr->binary.left);
      destroy_expr_ptr(&expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      destroy_string_slice(&expr->block.label);
      DESTROY_VECTOR_AND_ELEMENTS(expr->block.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_BR:
      destroy_var(&expr->br.var);
      if (expr->br.expr)
        destroy_expr_ptr(&expr->br.expr);
      break;
    case WASM_EXPR_TYPE_BR_IF:
      destroy_var(&expr->br_if.var);
      destroy_expr_ptr(&expr->br_if.cond);
      break;
    case WASM_EXPR_TYPE_CALL:
    case WASM_EXPR_TYPE_CALL_IMPORT:
      destroy_var(&expr->call.var);
      DESTROY_VECTOR_AND_ELEMENTS(expr->call.args, expr_ptr);
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      destroy_var(&expr->call_indirect.var);
      destroy_expr_ptr(&expr->call_indirect.expr);
      DESTROY_VECTOR_AND_ELEMENTS(expr->call_indirect.args, expr_ptr);
      break;
    case WASM_EXPR_TYPE_CAST:
      destroy_expr_ptr(&expr->cast.expr);
      break;
    case WASM_EXPR_TYPE_COMPARE:
      destroy_expr_ptr(&expr->compare.left);
      destroy_expr_ptr(&expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONVERT:
      destroy_expr_ptr(&expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      destroy_var(&expr->get_local.var);
      break;
    case WASM_EXPR_TYPE_GROW_MEMORY:
      destroy_expr_ptr(&expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_HAS_FEATURE:
      destroy_string_slice(&expr->has_feature.text);
      break;
    case WASM_EXPR_TYPE_IF:
      destroy_expr_ptr(&expr->if_.cond);
      destroy_expr_ptr(&expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      destroy_expr_ptr(&expr->if_else.cond);
      destroy_expr_ptr(&expr->if_else.true_);
      destroy_expr_ptr(&expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LABEL:
      destroy_string_slice(&expr->label.label);
      destroy_expr_ptr(&expr->label.expr);
      break;
    case WASM_EXPR_TYPE_LOAD:
    case WASM_EXPR_TYPE_LOAD_EXTEND:
      destroy_expr_ptr(&expr->load.addr);
      break;
    case WASM_EXPR_TYPE_LOAD_GLOBAL:
      destroy_var(&expr->load_global.var);
      break;
    case WASM_EXPR_TYPE_LOOP:
      destroy_string_slice(&expr->loop.inner);
      destroy_string_slice(&expr->loop.outer);
      DESTROY_VECTOR_AND_ELEMENTS(expr->loop.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr)
        destroy_expr_ptr(&expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      destroy_expr_ptr(&expr->select.cond);
      destroy_expr_ptr(&expr->select.true_);
      destroy_expr_ptr(&expr->select.false_);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      destroy_var(&expr->set_local.var);
      destroy_expr_ptr(&expr->set_local.expr);
      break;
    case WASM_EXPR_TYPE_STORE:
    case WASM_EXPR_TYPE_STORE_WRAP:
      destroy_expr_ptr(&expr->store.addr);
      destroy_expr_ptr(&expr->store.value);
      break;
    case WASM_EXPR_TYPE_STORE_GLOBAL:
      destroy_var(&expr->store_global.var);
      destroy_expr_ptr(&expr->store_global.expr);
      break;
    case WASM_EXPR_TYPE_TABLESWITCH:
      destroy_string_slice(&expr->tableswitch.label);
      destroy_expr_ptr(&expr->tableswitch.expr);
      DESTROY_VECTOR_AND_ELEMENTS(expr->tableswitch.targets, target);
      destroy_target(&expr->tableswitch.default_target);
      /* the binding name memory is shared, so just destroy the vector */
      wasm_destroy_binding_vector(&expr->tableswitch.case_bindings);
      DESTROY_VECTOR_AND_ELEMENTS(expr->tableswitch.cases, case);
      break;
    case WASM_EXPR_TYPE_UNARY:
      destroy_expr_ptr(&expr->unary.expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
    case WASM_EXPR_TYPE_CONST:
    case WASM_EXPR_TYPE_MEMORY_SIZE:
    case WASM_EXPR_TYPE_NOP:
    case WASM_EXPR_TYPE_PAGE_SIZE:
      break;
  }
}

static void destroy_expr_ptr(WasmExpr** expr) {
  destroy_expr(*expr);
  free(*expr);
}

static void destroy_func(WasmFunc* func) {
  destroy_string_slice(&func->name);
  destroy_var(&func->type_var);
  destroy_type_bindings(&func->params);
  destroy_type_bindings(&func->locals);
  /* params_and_locals shares binding data with params and locals */
  wasm_destroy_type_vector(&func->params_and_locals.types);
  wasm_destroy_binding_vector(&func->params_and_locals.bindings);
  DESTROY_VECTOR_AND_ELEMENTS(func->exprs, expr_ptr);
}

static void destroy_import(WasmImport* import) {
  destroy_string_slice(&import->name);
  destroy_string_slice(&import->module_name);
  destroy_string_slice(&import->func_name);
  destroy_var(&import->type_var);
  destroy_func_signature(&import->func_sig);
}

static void destroy_export(WasmExport* export) {
  destroy_string_slice(&export->name);
  destroy_var(&export->var);
}

static void destroy_func_type(WasmFuncType* func_type) {
  destroy_string_slice(&func_type->name);
  destroy_func_signature(&func_type->sig);
}

static void destroy_segment(WasmSegment* segment) {
  free(segment->data);
}

static void destroy_memory(WasmMemory* memory) {
  DESTROY_VECTOR_AND_ELEMENTS(memory->segments, segment);
}

static void destroy_module_field(WasmModuleField* field) {
  switch (field->type) {
    case WASM_MODULE_FIELD_TYPE_FUNC:
      destroy_func(&field->func);
      break;
    case WASM_MODULE_FIELD_TYPE_IMPORT:
      destroy_import(&field->import);
      break;
    case WASM_MODULE_FIELD_TYPE_EXPORT:
      destroy_export(&field->export);
      break;
    case WASM_MODULE_FIELD_TYPE_TABLE:
      DESTROY_VECTOR_AND_ELEMENTS(field->table, var);
      break;
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
      destroy_func_type(&field->func_type);
      break;
    case WASM_MODULE_FIELD_TYPE_MEMORY:
      destroy_memory(&field->memory);
      break;
    case WASM_MODULE_FIELD_TYPE_GLOBAL:
      destroy_type_bindings(&field->global);
      break;
  }
}

static void destroy_module(WasmModule* module) {
  DESTROY_VECTOR_AND_ELEMENTS(module->fields, module_field);
  /* everything that follows shares data with the module_fields above, so we
   only need to destroy the containing vectors */
  wasm_destroy_func_ptr_vector(&module->funcs);
  wasm_destroy_import_ptr_vector(&module->imports);
  wasm_destroy_export_ptr_vector(&module->exports);
  wasm_destroy_func_type_ptr_vector(&module->func_types);
  wasm_destroy_type_vector(&module->globals.types);
  wasm_destroy_binding_vector(&module->globals.bindings);
  wasm_destroy_binding_vector(&module->func_bindings);
  wasm_destroy_binding_vector(&module->import_bindings);
  wasm_destroy_binding_vector(&module->export_bindings);
  wasm_destroy_binding_vector(&module->func_type_bindings);
}

static void destroy_invoke(WasmCommandInvoke* invoke) {
  destroy_string_slice(&invoke->name);
  wasm_destroy_const_vector(&invoke->args);
}

static void destroy_command(WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      destroy_module(&command->module);
      break;
    case WASM_COMMAND_TYPE_INVOKE:
      destroy_invoke(&command->invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      destroy_module(&command->assert_invalid.module);
      destroy_string_slice(&command->assert_invalid.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      destroy_invoke(&command->assert_return.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      destroy_invoke(&command->assert_return_nan.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      destroy_invoke(&command->assert_trap.invoke);
      destroy_string_slice(&command->assert_trap.text);
      break;
  }
}

void wasm_destroy_script(WasmScript* script) {
  DESTROY_VECTOR_AND_ELEMENTS(script->commands, command);
}
