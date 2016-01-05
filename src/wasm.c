#include "wasm.h"

#include <assert.h>
#include <ctype.h>
#include <memory.h>
#include <stdlib.h>

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

DEFINE_VECTOR(type, WasmType)
DEFINE_VECTOR(var, WasmVar);
DEFINE_VECTOR(expr_ptr, WasmExprPtr);
DEFINE_VECTOR(target, WasmTarget);
DEFINE_VECTOR(case, WasmCase);
DEFINE_VECTOR(binding, WasmBinding);
DEFINE_VECTOR(func_ptr, WasmFuncPtr);
DEFINE_VECTOR(segment, WasmSegment);
DEFINE_VECTOR(func_type_ptr, WasmFuncTypePtr);
DEFINE_VECTOR(import_ptr, WasmImportPtr);
DEFINE_VECTOR(export_ptr, WasmExportPtr);
DEFINE_VECTOR(module_field, WasmModuleField);
DEFINE_VECTOR(const, WasmConst);
DEFINE_VECTOR(command, WasmCommand);

int wasm_string_slices_are_equal(WasmStringSlice* a, WasmStringSlice* b) {
  return a->start && b->start && a->length == b->length &&
         memcmp(a->start, b->start, a->length) == 0;
}

static int find_binding_index_by_name(WasmBindingVector* bindings,
                                      WasmStringSlice* name) {
  int i;
  for (i = 0; i < bindings->size; ++i)
    if (wasm_string_slices_are_equal(name, &bindings->data[i].name))
      return bindings->data[i].index;
  return -1;
}

int wasm_get_index_from_var(WasmBindingVector* bindings, WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return find_binding_index_by_name(bindings, &var->name);
  return var->index;
}

WasmExportPtr wasm_get_export_by_name(WasmModule* module,
                                      WasmStringSlice* name) {
  int index = find_binding_index_by_name(&module->export_bindings, name);
  if (index == -1)
    return NULL;
  return module->exports.data[index];
}

int wasm_func_is_exported(WasmModule* module, WasmFunc* func) {
  int i;
  for (i = 0; i < module->exports.size; ++i) {
    WasmExport* export = module->exports.data[i];
    if (export->var.type == WASM_VAR_TYPE_NAME) {
      if (wasm_string_slices_are_equal(&export->var.name, &func->name))
        return 1;
    } else {
      assert(export->var.type == WASM_VAR_TYPE_INDEX);
      int index = export->var.index;
      if (index >= 0 && index < module->funcs.size &&
          module->funcs.data[index] == func)
        return 1;
    }
  }
  return 0;
}

int wasm_get_func_index_by_var(WasmModule* module, WasmVar* var) {
  return wasm_get_index_from_var(&module->func_bindings, var);
}

int wasm_get_func_type_index_by_var(WasmModule* module, WasmVar* var) {
  return wasm_get_index_from_var(&module->func_type_bindings, var);
}

int wasm_get_global_index_by_var(WasmModule* module, WasmVar* var) {
  return wasm_get_index_from_var(&module->globals.bindings, var);
}

int wasm_get_import_index_by_var(WasmModule* module, WasmVar* var) {
  return wasm_get_index_from_var(&module->import_bindings, var);
}

int wasm_get_local_index_by_var(WasmFunc* func, WasmVar* var) {
  return wasm_get_index_from_var(&func->params_and_locals.bindings, var);
}

WasmFuncPtr wasm_get_func_by_var(WasmModule* module, WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_bindings, var);
  if (index < 0 || index >= module->funcs.size)
    return NULL;
  return module->funcs.data[index];
}

WasmFuncTypePtr wasm_get_func_type_by_var(WasmModule* module, WasmVar* var) {
  int index = wasm_get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || index >= module->func_types.size)
    return NULL;
  return module->func_types.data[index];
}

WasmImportPtr wasm_get_import_by_var(WasmModule* module, WasmVar* var) {
  int index = wasm_get_index_from_var(&module->import_bindings, var);
  if (index < 0 || index >= module->imports.size)
    return NULL;
  return module->imports.data[index];
}

WasmResult wasm_extend_type_bindings(WasmTypeBindings* dst,
                                     WasmTypeBindings* src) {
  WasmResult result = WASM_OK;
  int last_type = dst->types.size;
  int last_binding = dst->bindings.size;
  result = wasm_extend_types(&dst->types, &src->types);
  if (result != WASM_OK)
    return result;
  result = wasm_extend_bindings(&dst->bindings, &src->bindings);
  if (result != WASM_OK)
    return result;
  /* fixup the binding indexes */
  int i;
  for (i = last_binding; i < dst->bindings.size; ++i)
    dst->bindings.data[i].index += last_type;
  return result;
}

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
      if (expr->br_if.expr)
        wasm_destroy_expr_ptr(&expr->br_if.expr);
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
    case WASM_EXPR_TYPE_LOAD:
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

void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc) {
  /* mimic xxd output */
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    printf("%07x: ", (int)((void*)p - start + offset));
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          printf("%02x", *p);
        } else {
          putchar(' ');
          putchar(' ');
        }
      }
      putchar(' ');
    }

    putchar(' ');
    p = line;
    int i;
    for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
      if (print_chars)
        printf("%c", isprint(*p) ? *p : '.');
    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      printf("  ; %s", desc);
    putchar('\n');
  }
}
