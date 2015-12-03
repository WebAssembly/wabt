#include "wasm.h"

#include <stdlib.h>

void wasm_destroy_string_slice(WasmStringSlice* str) {
  free((char*)str->start);
}

static void wasm_destroy_binding(WasmBinding* binding) {
  wasm_destroy_string_slice(&binding->name);
}

void wasm_destroy_type_bindings(WasmTypeBindings* type_bindings) {
  wasm_destroy_type_vector(&type_bindings->types);
  DESTROY_VECTOR_AND_ELEMENTS(type_bindings->bindings, binding);
}

void wasm_destroy_var(WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    wasm_destroy_string_slice(&var->name);
}

void wasm_destroy_var_vector_and_elements(WasmVarVector* vars) {
  DESTROY_VECTOR_AND_ELEMENTS(*vars, var);
}

void wasm_destroy_func_signature(WasmFuncSignature* sig) {
  wasm_destroy_type_vector(&sig->param_types);
}

void wasm_destroy_target(WasmTarget* target) {
  wasm_destroy_var(&target->var);
}

void wasm_destroy_target_vector_and_elements(WasmTargetVector* targets) {
  DESTROY_VECTOR_AND_ELEMENTS(*targets, target);
}

void wasm_destroy_expr_ptr(WasmExpr** expr);

void wasm_destroy_case(WasmCase* case_) {
  wasm_destroy_string_slice(&case_->label);
  DESTROY_VECTOR_AND_ELEMENTS(case_->exprs, expr_ptr);
}

void wasm_destroy_case_vector_and_elements(WasmCaseVector* cases) {
  DESTROY_VECTOR_AND_ELEMENTS(*cases, case);
}

static void wasm_destroy_expr(WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      wasm_destroy_expr_ptr(&expr->binary.left);
      wasm_destroy_expr_ptr(&expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      wasm_destroy_string_slice(&expr->block.label);
      DESTROY_VECTOR_AND_ELEMENTS(expr->block.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_BR:
      wasm_destroy_var(&expr->br.var);
      if (expr->br.expr)
        wasm_destroy_expr_ptr(&expr->br.expr);
      break;
    case WASM_EXPR_TYPE_BR_IF:
      wasm_destroy_var(&expr->br_if.var);
      wasm_destroy_expr_ptr(&expr->br_if.cond);
      break;
    case WASM_EXPR_TYPE_CALL:
    case WASM_EXPR_TYPE_CALL_IMPORT:
      wasm_destroy_var(&expr->call.var);
      DESTROY_VECTOR_AND_ELEMENTS(expr->call.args, expr_ptr);
      break;
    case WASM_EXPR_TYPE_CALL_INDIRECT:
      wasm_destroy_var(&expr->call_indirect.var);
      wasm_destroy_expr_ptr(&expr->call_indirect.expr);
      DESTROY_VECTOR_AND_ELEMENTS(expr->call_indirect.args, expr_ptr);
      break;
    case WASM_EXPR_TYPE_CAST:
      wasm_destroy_expr_ptr(&expr->cast.expr);
      break;
    case WASM_EXPR_TYPE_COMPARE:
      wasm_destroy_expr_ptr(&expr->compare.left);
      wasm_destroy_expr_ptr(&expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONVERT:
      wasm_destroy_expr_ptr(&expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL:
      wasm_destroy_var(&expr->get_local.var);
      break;
    case WASM_EXPR_TYPE_GROW_MEMORY:
      wasm_destroy_expr_ptr(&expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_HAS_FEATURE:
      wasm_destroy_string_slice(&expr->has_feature.text);
      break;
    case WASM_EXPR_TYPE_IF:
      wasm_destroy_expr_ptr(&expr->if_.cond);
      wasm_destroy_expr_ptr(&expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      wasm_destroy_expr_ptr(&expr->if_else.cond);
      wasm_destroy_expr_ptr(&expr->if_else.true_);
      wasm_destroy_expr_ptr(&expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LABEL:
      wasm_destroy_string_slice(&expr->label.label);
      wasm_destroy_expr_ptr(&expr->label.expr);
      break;
    case WASM_EXPR_TYPE_LOAD:
    case WASM_EXPR_TYPE_LOAD_EXTEND:
      wasm_destroy_expr_ptr(&expr->load.addr);
      break;
    case WASM_EXPR_TYPE_LOAD_GLOBAL:
      wasm_destroy_var(&expr->load_global.var);
      break;
    case WASM_EXPR_TYPE_LOOP:
      wasm_destroy_string_slice(&expr->loop.inner);
      wasm_destroy_string_slice(&expr->loop.outer);
      DESTROY_VECTOR_AND_ELEMENTS(expr->loop.exprs, expr_ptr);
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr)
        wasm_destroy_expr_ptr(&expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      wasm_destroy_expr_ptr(&expr->select.cond);
      wasm_destroy_expr_ptr(&expr->select.true_);
      wasm_destroy_expr_ptr(&expr->select.false_);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL:
      wasm_destroy_var(&expr->set_local.var);
      wasm_destroy_expr_ptr(&expr->set_local.expr);
      break;
    case WASM_EXPR_TYPE_STORE:
    case WASM_EXPR_TYPE_STORE_WRAP:
      wasm_destroy_expr_ptr(&expr->store.addr);
      wasm_destroy_expr_ptr(&expr->store.value);
      break;
    case WASM_EXPR_TYPE_STORE_GLOBAL:
      wasm_destroy_var(&expr->store_global.var);
      wasm_destroy_expr_ptr(&expr->store_global.expr);
      break;
    case WASM_EXPR_TYPE_TABLESWITCH:
      wasm_destroy_string_slice(&expr->tableswitch.label);
      wasm_destroy_expr_ptr(&expr->tableswitch.expr);
      DESTROY_VECTOR_AND_ELEMENTS(expr->tableswitch.targets, target);
      wasm_destroy_target(&expr->tableswitch.default_target);
      /* the binding name memory is shared, so just destroy the vector */
      wasm_destroy_binding_vector(&expr->tableswitch.case_bindings);
      DESTROY_VECTOR_AND_ELEMENTS(expr->tableswitch.cases, case);
      break;
    case WASM_EXPR_TYPE_UNARY:
      wasm_destroy_expr_ptr(&expr->unary.expr);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
    case WASM_EXPR_TYPE_CONST:
    case WASM_EXPR_TYPE_MEMORY_SIZE:
    case WASM_EXPR_TYPE_NOP:
    case WASM_EXPR_TYPE_PAGE_SIZE:
      break;
  }
}

void wasm_destroy_expr_ptr(WasmExpr** expr) {
  wasm_destroy_expr(*expr);
  free(*expr);
}

void wasm_destroy_expr_ptr_vector_and_elements(WasmExprPtrVector* exprs) {
  DESTROY_VECTOR_AND_ELEMENTS(*exprs, expr_ptr);
}

void wasm_destroy_func(WasmFunc* func) {
  wasm_destroy_string_slice(&func->name);
  wasm_destroy_var(&func->type_var);
  wasm_destroy_type_bindings(&func->params);
  wasm_destroy_type_bindings(&func->locals);
  /* params_and_locals shares binding data with params and locals */
  wasm_destroy_type_vector(&func->params_and_locals.types);
  wasm_destroy_binding_vector(&func->params_and_locals.bindings);
  DESTROY_VECTOR_AND_ELEMENTS(func->exprs, expr_ptr);
}

void wasm_destroy_import(WasmImport* import) {
  wasm_destroy_string_slice(&import->name);
  wasm_destroy_string_slice(&import->module_name);
  wasm_destroy_string_slice(&import->func_name);
  wasm_destroy_var(&import->type_var);
  wasm_destroy_func_signature(&import->func_sig);
}

void wasm_destroy_export(WasmExport* export) {
  wasm_destroy_string_slice(&export->name);
  wasm_destroy_var(&export->var);
}

void wasm_destroy_func_type(WasmFuncType* func_type) {
  wasm_destroy_string_slice(&func_type->name);
  wasm_destroy_func_signature(&func_type->sig);
}

void wasm_destroy_segment(WasmSegment* segment) {
  free(segment->data);
}

void wasm_destroy_segment_vector_and_elements(WasmSegmentVector* segments) {
  DESTROY_VECTOR_AND_ELEMENTS(*segments, segment);
}

void wasm_destroy_memory(WasmMemory* memory) {
  DESTROY_VECTOR_AND_ELEMENTS(memory->segments, segment);
}

static void wasm_destroy_module_field(WasmModuleField* field) {
  switch (field->type) {
    case WASM_MODULE_FIELD_TYPE_FUNC:
      wasm_destroy_func(&field->func);
      break;
    case WASM_MODULE_FIELD_TYPE_IMPORT:
      wasm_destroy_import(&field->import);
      break;
    case WASM_MODULE_FIELD_TYPE_EXPORT:
      wasm_destroy_export(&field->export);
      break;
    case WASM_MODULE_FIELD_TYPE_TABLE:
      DESTROY_VECTOR_AND_ELEMENTS(field->table, var);
      break;
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
      wasm_destroy_func_type(&field->func_type);
      break;
    case WASM_MODULE_FIELD_TYPE_MEMORY:
      wasm_destroy_memory(&field->memory);
      break;
    case WASM_MODULE_FIELD_TYPE_GLOBAL:
      wasm_destroy_type_bindings(&field->global);
      break;
  }
}

void wasm_destroy_module_field_vector_and_elements(
    WasmModuleFieldVector* module_fields) {
  DESTROY_VECTOR_AND_ELEMENTS(*module_fields, module_field);
}

void wasm_destroy_module(WasmModule* module) {
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

static void wasm_destroy_invoke(WasmCommandInvoke* invoke) {
  wasm_destroy_string_slice(&invoke->name);
  wasm_destroy_const_vector(&invoke->args);
}

void wasm_destroy_command(WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      wasm_destroy_module(&command->module);
      break;
    case WASM_COMMAND_TYPE_INVOKE:
      wasm_destroy_invoke(&command->invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      wasm_destroy_module(&command->assert_invalid.module);
      wasm_destroy_string_slice(&command->assert_invalid.text);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      wasm_destroy_invoke(&command->assert_return.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      wasm_destroy_invoke(&command->assert_return_nan.invoke);
      break;
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      wasm_destroy_invoke(&command->assert_trap.invoke);
      wasm_destroy_string_slice(&command->assert_trap.text);
      break;
  }
}

void wasm_destroy_command_vector_and_elements(WasmCommandVector* commands) {
  DESTROY_VECTOR_AND_ELEMENTS(*commands, command);
}

void wasm_destroy_script(WasmScript* script) {
  DESTROY_VECTOR_AND_ELEMENTS(script->commands, command);
}
