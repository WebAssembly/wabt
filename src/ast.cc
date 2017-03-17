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

namespace wabt {

int get_index_from_var(const BindingHash* hash, const Var* var) {
  if (var->type == VarType::Name)
    return hash->find_index(var->name);
  return static_cast<int>(var->index);
}

Export* get_export_by_name(const Module* module, const StringSlice* name) {
  int index = module->export_bindings.find_index(*name);
  if (index == -1)
    return nullptr;
  return module->exports[index];
}

int get_func_index_by_var(const Module* module, const Var* var) {
  return get_index_from_var(&module->func_bindings, var);
}

int get_global_index_by_var(const Module* module, const Var* var) {
  return get_index_from_var(&module->global_bindings, var);
}

int get_table_index_by_var(const Module* module, const Var* var) {
  return get_index_from_var(&module->table_bindings, var);
}

int get_memory_index_by_var(const Module* module, const Var* var) {
  return get_index_from_var(&module->memory_bindings, var);
}

int get_func_type_index_by_var(const Module* module, const Var* var) {
  return get_index_from_var(&module->func_type_bindings, var);
}

int get_local_index_by_var(const Func* func, const Var* var) {
  if (var->type == VarType::Index)
    return static_cast<int>(var->index);

  int result = func->param_bindings.find_index(var->name);
  if (result != -1)
    return result;

  result = func->local_bindings.find_index(var->name);
  if (result == -1)
    return result;

  /* the locals start after all the params */
  return func->decl.sig.param_types.size() + result;
}

int get_module_index_by_var(const Script* script, const Var* var) {
  return get_index_from_var(&script->module_bindings, var);
}

Func* get_func_by_var(const Module* module, const Var* var) {
  int index = get_index_from_var(&module->func_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= module->funcs.size())
    return nullptr;
  return module->funcs[index];
}

Global* get_global_by_var(const Module* module, const Var* var) {
  int index = get_index_from_var(&module->global_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= module->globals.size())
    return nullptr;
  return module->globals[index];
}

Table* get_table_by_var(const Module* module, const Var* var) {
  int index = get_index_from_var(&module->table_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= module->tables.size())
    return nullptr;
  return module->tables[index];
}

Memory* get_memory_by_var(const Module* module, const Var* var) {
  int index = get_index_from_var(&module->memory_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= module->memories.size())
    return nullptr;
  return module->memories[index];
}

FuncType* get_func_type_by_var(const Module* module, const Var* var) {
  int index = get_index_from_var(&module->func_type_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= module->func_types.size())
    return nullptr;
  return module->func_types[index];
}

int get_func_type_index_by_sig(const Module* module, const FuncSignature* sig) {
  for (size_t i = 0; i < module->func_types.size(); ++i)
    if (signatures_are_equal(&module->func_types[i]->sig, sig))
      return i;
  return -1;
}

int get_func_type_index_by_decl(const Module* module,
                                const FuncDeclaration* decl) {
  if (decl_has_func_type(decl)) {
    return get_func_type_index_by_var(module, &decl->type_var);
  } else {
    return get_func_type_index_by_sig(module, &decl->sig);
  }
}

Module* get_first_module(const Script* script) {
  for (size_t i = 0; i < script->commands.size(); ++i) {
    const Command& command = *script->commands[i].get();
    if (command.type == CommandType::Module)
      return command.module;
  }
  return nullptr;
}

Module* get_module_by_var(const Script* script, const Var* var) {
  int index = get_index_from_var(&script->module_bindings, var);
  if (index < 0 || static_cast<size_t>(index) >= script->commands.size())
    return nullptr;
  const Command& command = *script->commands[index].get();
  assert(command.type == CommandType::Module);
  return command.module;
}

void make_type_binding_reverse_mapping(
    const TypeVector& types,
    const BindingHash& bindings,
    std::vector<std::string>* out_reverse_mapping) {
  out_reverse_mapping->clear();
  out_reverse_mapping->resize(types.size());
  for (auto iter: bindings) {
    assert(static_cast<size_t>(iter.second.index) <
           out_reverse_mapping->size());
    (*out_reverse_mapping)[iter.second.index] = iter.first;
  }
}

ModuleField* append_module_field(Module* module) {
  ModuleField* result = new ModuleField();
  if (!module->first_field)
    module->first_field = result;
  else if (module->last_field)
    module->last_field->next = result;
  module->last_field = result;
  return result;
}

FuncType* append_implicit_func_type(Location* loc,
                                    Module* module,
                                    FuncSignature* sig) {
  ModuleField* field = append_module_field(module);
  field->loc = *loc;
  field->type = ModuleFieldType::FuncType;
  field->func_type = new FuncType();
  field->func_type->sig = *sig;

  module->func_types.push_back(field->func_type);
  return field->func_type;
}

#define FOREACH_EXPR_TYPE(V)                 \
  V(ExprType::Binary, binary)                \
  V(ExprType::Block, block)                  \
  V(ExprType::Br, br)                        \
  V(ExprType::BrIf, br_if)                   \
  V(ExprType::BrTable, br_table)             \
  V(ExprType::Call, call)                    \
  V(ExprType::CallIndirect, call_indirect)   \
  V(ExprType::Compare, compare)              \
  V(ExprType::Const, const)                  \
  V(ExprType::Convert, convert)              \
  V(ExprType::GetGlobal, get_global)         \
  V(ExprType::GetLocal, get_local)           \
  V(ExprType::If, if)                        \
  V(ExprType::Load, load)                    \
  V(ExprType::Loop, loop)                    \
  V(ExprType::SetGlobal, set_global)         \
  V(ExprType::SetLocal, set_local)           \
  V(ExprType::Store, store)                  \
  V(ExprType::TeeLocal, tee_local)           \
  V(ExprType::Unary, unary)                  \
  V(ExprType::CurrentMemory, current_memory) \
  V(ExprType::Drop, drop)                    \
  V(ExprType::GrowMemory, grow_memory)       \
  V(ExprType::Nop, nop)                      \
  V(ExprType::Return, return )               \
  V(ExprType::Select, select)                \
  V(ExprType::Unreachable, unreachable)

#define DEFINE_NEW_EXPR(type_, name) \
  Expr* new_##name##_expr(void) {    \
    Expr* result = new Expr();       \
    result->type = type_;            \
    return result;                   \
  }
FOREACH_EXPR_TYPE(DEFINE_NEW_EXPR)
#undef DEFINE_NEW_EXPR

void destroy_var(Var* var) {
  if (var->type == VarType::Name)
    destroy_string_slice(&var->name);
}

void destroy_expr_list(Expr* first) {
  Expr* expr = first;
  while (expr) {
    Expr* next = expr->next;
    delete expr;
    expr = next;
  }
}

Block::Block(): first(nullptr) {
  WABT_ZERO_MEMORY(label);
}

Block::~Block() {
  destroy_string_slice(&label);
  destroy_expr_list(first);
}

Expr::Expr() : type(ExprType::Binary), next(nullptr) {
  WABT_ZERO_MEMORY(loc);
  binary.opcode = Opcode::Nop;
}

Expr::~Expr() {
  switch (type) {
    case ExprType::Block:
      delete block;
      break;
    case ExprType::Br:
      destroy_var(&br.var);
      break;
    case ExprType::BrIf:
      destroy_var(&br_if.var);
      break;
    case ExprType::BrTable:
      for (auto& var : *br_table.targets)
        destroy_var(&var);
      delete br_table.targets;
      destroy_var(&br_table.default_target);
      break;
    case ExprType::Call:
      destroy_var(&call.var);
      break;
    case ExprType::CallIndirect:
      destroy_var(&call_indirect.var);
      break;
    case ExprType::GetGlobal:
      destroy_var(&get_global.var);
      break;
    case ExprType::GetLocal:
      destroy_var(&get_local.var);
      break;
    case ExprType::If:
      delete if_.true_;
      destroy_expr_list(if_.false_);
      break;
    case ExprType::Loop:
      delete loop;
      break;
    case ExprType::SetGlobal:
      destroy_var(&set_global.var);
      break;
    case ExprType::SetLocal:
      destroy_var(&set_local.var);
      break;
    case ExprType::TeeLocal:
      destroy_var(&tee_local.var);
      break;

    case ExprType::Binary:
    case ExprType::Compare:
    case ExprType::Const:
    case ExprType::Convert:
    case ExprType::Drop:
    case ExprType::CurrentMemory:
    case ExprType::GrowMemory:
    case ExprType::Load:
    case ExprType::Nop:
    case ExprType::Return:
    case ExprType::Select:
    case ExprType::Store:
    case ExprType::Unary:
    case ExprType::Unreachable:
      break;
  }
}

FuncType::FuncType() {
  WABT_ZERO_MEMORY(name);
}

FuncType::~FuncType() {
  destroy_string_slice(&name);
}

FuncDeclaration::FuncDeclaration() : has_func_type(false) {
  WABT_ZERO_MEMORY(type_var);
}

FuncDeclaration::~FuncDeclaration() {
  destroy_var(&type_var);
}

Func::Func() : first_expr(nullptr) {
  WABT_ZERO_MEMORY(name);
}

Func::~Func() {
  destroy_string_slice(&name);
  destroy_expr_list(first_expr);
}

Global::Global() : type(Type::Void), mutable_(false), init_expr(nullptr) {
  WABT_ZERO_MEMORY(name);
}

Global::~Global() {
  destroy_string_slice(&name);
  destroy_expr_list(init_expr);
}

Table::Table() {
  WABT_ZERO_MEMORY(name);
  WABT_ZERO_MEMORY(elem_limits);
}

Table::~Table() {
  destroy_string_slice(&name);
}

ElemSegment::ElemSegment() : offset(nullptr) {
  WABT_ZERO_MEMORY(table_var);
}

ElemSegment::~ElemSegment() {
  destroy_var(&table_var);
  destroy_expr_list(offset);
  for (auto& var : vars)
    destroy_var(&var);
}

DataSegment::DataSegment() : offset(nullptr), data(nullptr), size(0) {
  WABT_ZERO_MEMORY(memory_var);
}

DataSegment::~DataSegment() {
  destroy_var(&memory_var);
  destroy_expr_list(offset);
  delete [] data;
}

Memory::Memory() {
  WABT_ZERO_MEMORY(name);
  WABT_ZERO_MEMORY(page_limits);
}

Memory::~Memory() {
  destroy_string_slice(&name);
}

Import::Import() : kind(ExternalKind::Func), func(nullptr) {
  WABT_ZERO_MEMORY(module_name);
  WABT_ZERO_MEMORY(field_name);
}

Import::~Import() {
  destroy_string_slice(&module_name);
  destroy_string_slice(&field_name);
  switch (kind) {
    case ExternalKind::Func:
      delete func;
      break;
    case ExternalKind::Table:
      delete table;
      break;
    case ExternalKind::Memory:
      delete memory;
      break;
    case ExternalKind::Global:
      delete global;
      break;
  }
}

Export::Export() {
  WABT_ZERO_MEMORY(name);
  WABT_ZERO_MEMORY(var);
}

Export::~Export() {
  destroy_string_slice(&name);
  destroy_var(&var);
}

void destroy_memory(Memory* memory) {
  destroy_string_slice(&memory->name);
}

void destroy_table(Table* table) {
  destroy_string_slice(&table->name);
}

ModuleField::ModuleField() : type(ModuleFieldType::Start), next(nullptr) {
  WABT_ZERO_MEMORY(loc);
  WABT_ZERO_MEMORY(start);
}

ModuleField::~ModuleField() {
  switch (type) {
    case ModuleFieldType::Func:
      delete func;
      break;
    case ModuleFieldType::Global:
      delete global;
      break;
    case ModuleFieldType::Import:
      delete import;
      break;
    case ModuleFieldType::Export:
      delete export_;
      break;
    case ModuleFieldType::FuncType:
      delete func_type;
      break;
    case ModuleFieldType::Table:
      delete table;
      break;
    case ModuleFieldType::ElemSegment:
      delete elem_segment;
      break;
    case ModuleFieldType::Memory:
      delete memory;
      break;
    case ModuleFieldType::DataSegment:
      delete data_segment;
      break;
    case ModuleFieldType::Start:
      destroy_var(&start);
      break;
  }
}

Module::Module()
    : first_field(nullptr),
      last_field(nullptr),
      num_func_imports(0),
      num_table_imports(0),
      num_memory_imports(0),
      num_global_imports(0),
      start(0) {
  WABT_ZERO_MEMORY(loc);
  WABT_ZERO_MEMORY(name);
}

Module::~Module() {
  destroy_string_slice(&name);

  ModuleField* field = first_field;
  while (field) {
    ModuleField* next_field = field->next;
    delete field;
    field = next_field;
  }
}

RawModule::RawModule() : type(RawModuleType::Text), text(nullptr) {}

RawModule::~RawModule() {
  if (type == RawModuleType::Text) {
    delete text;
  } else {
    destroy_string_slice(&binary.name);
    delete [] binary.data;
  }
}

ActionInvoke::ActionInvoke() {}

Action::Action() : type(ActionType::Get) {
  WABT_ZERO_MEMORY(loc);
  WABT_ZERO_MEMORY(module_var);
  WABT_ZERO_MEMORY(name);
}

Action::~Action() {
  destroy_var(&module_var);
  destroy_string_slice(&name);
  switch (type) {
    case ActionType::Invoke:
      delete invoke;
      break;
    case ActionType::Get:
      break;
  }
}

Command::Command() : type(CommandType::Module), module(nullptr) {}

Command::~Command() {
  switch (type) {
    case CommandType::Module:
      delete module;
      break;
    case CommandType::Action:
      delete action;
      break;
    case CommandType::Register:
      destroy_string_slice(&register_.module_name);
      destroy_var(&register_.var);
      break;
    case CommandType::AssertMalformed:
      delete assert_malformed.module;
      destroy_string_slice(&assert_malformed.text);
      break;
    case CommandType::AssertInvalid:
    case CommandType::AssertInvalidNonBinary:
      delete assert_invalid.module;
      destroy_string_slice(&assert_invalid.text);
      break;
    case CommandType::AssertUnlinkable:
      delete assert_unlinkable.module;
      destroy_string_slice(&assert_unlinkable.text);
      break;
    case CommandType::AssertUninstantiable:
      delete assert_uninstantiable.module;
      destroy_string_slice(&assert_uninstantiable.text);
      break;
    case CommandType::AssertReturn:
      delete assert_return.action;
      delete assert_return.expected;
      break;
    case CommandType::AssertReturnNan:
      delete assert_return_nan.action;
      break;
    case CommandType::AssertTrap:
    case CommandType::AssertExhaustion:
      delete assert_trap.action;
      destroy_string_slice(&assert_trap.text);
      break;
  }
}

Script::Script() {}

#define CHECK_RESULT(expr)   \
  do {                       \
    if (WABT_FAILED((expr))) \
      return Result::Error;  \
  } while (0)

#define CALLBACK(member)                                           \
  CHECK_RESULT((visitor)->member                                   \
                   ? (visitor)->member(expr, (visitor)->user_data) \
                   : Result::Ok)

static Result visit_expr(Expr* expr, ExprVisitor* visitor);

Result visit_expr_list(Expr* first, ExprVisitor* visitor) {
  for (Expr* expr = first; expr; expr = expr->next)
    CHECK_RESULT(visit_expr(expr, visitor));
  return Result::Ok;
}

static Result visit_expr(Expr* expr, ExprVisitor* visitor) {
  switch (expr->type) {
    case ExprType::Binary:
      CALLBACK(on_binary_expr);
      break;

    case ExprType::Block:
      CALLBACK(begin_block_expr);
      CHECK_RESULT(visit_expr_list(expr->block->first, visitor));
      CALLBACK(end_block_expr);
      break;

    case ExprType::Br:
      CALLBACK(on_br_expr);
      break;

    case ExprType::BrIf:
      CALLBACK(on_br_if_expr);
      break;

    case ExprType::BrTable:
      CALLBACK(on_br_table_expr);
      break;

    case ExprType::Call:
      CALLBACK(on_call_expr);
      break;

    case ExprType::CallIndirect:
      CALLBACK(on_call_indirect_expr);
      break;

    case ExprType::Compare:
      CALLBACK(on_compare_expr);
      break;

    case ExprType::Const:
      CALLBACK(on_const_expr);
      break;

    case ExprType::Convert:
      CALLBACK(on_convert_expr);
      break;

    case ExprType::CurrentMemory:
      CALLBACK(on_current_memory_expr);
      break;

    case ExprType::Drop:
      CALLBACK(on_drop_expr);
      break;

    case ExprType::GetGlobal:
      CALLBACK(on_get_global_expr);
      break;

    case ExprType::GetLocal:
      CALLBACK(on_get_local_expr);
      break;

    case ExprType::GrowMemory:
      CALLBACK(on_grow_memory_expr);
      break;

    case ExprType::If:
      CALLBACK(begin_if_expr);
      CHECK_RESULT(visit_expr_list(expr->if_.true_->first, visitor));
      CALLBACK(after_if_true_expr);
      CHECK_RESULT(visit_expr_list(expr->if_.false_, visitor));
      CALLBACK(end_if_expr);
      break;

    case ExprType::Load:
      CALLBACK(on_load_expr);
      break;

    case ExprType::Loop:
      CALLBACK(begin_loop_expr);
      CHECK_RESULT(visit_expr_list(expr->loop->first, visitor));
      CALLBACK(end_loop_expr);
      break;

    case ExprType::Nop:
      CALLBACK(on_nop_expr);
      break;

    case ExprType::Return:
      CALLBACK(on_return_expr);
      break;

    case ExprType::Select:
      CALLBACK(on_select_expr);
      break;

    case ExprType::SetGlobal:
      CALLBACK(on_set_global_expr);
      break;

    case ExprType::SetLocal:
      CALLBACK(on_set_local_expr);
      break;

    case ExprType::Store:
      CALLBACK(on_store_expr);
      break;

    case ExprType::TeeLocal:
      CALLBACK(on_tee_local_expr);
      break;

    case ExprType::Unary:
      CALLBACK(on_unary_expr);
      break;

    case ExprType::Unreachable:
      CALLBACK(on_unreachable_expr);
      break;
  }

  return Result::Ok;
}

/* TODO(binji): make the visitor non-recursive */
Result visit_func(Func* func, ExprVisitor* visitor) {
  return visit_expr_list(func->first_expr, visitor);
}

}  // namespace wabt
