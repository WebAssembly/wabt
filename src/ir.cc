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

#include "ir.h"

#include <cassert>
#include <cstddef>

namespace {

const char* ExprTypeName[] = {
  "Binary",
  "Block",
  "Br",
  "BrIf",
  "BrTable",
  "Call",
  "CallIndirect",
  "Compare",
  "Const",
  "Convert",
  "CurrentMemory",
  "Drop",
  "GetGlobal",
  "GetLocal",
  "GrowMemory",
  "If",
  "Load",
  "Loop",
  "Nop",
  "Rethrow",
  "Return",
  "Select",
  "SetGlobal",
  "SetLocal",
  "Store",
  "TeeLocal",
  "Throw",
  "TryBlock",
  "Unary",
  "Unreachable"
};

}  // end of anonymous namespace

namespace wabt {

const char* GetExprTypeName(ExprType type) {
  static_assert(WABT_ENUM_COUNT(ExprType) == WABT_ARRAY_SIZE(ExprTypeName),
                "Malformed ExprTypeName array");
  return ExprTypeName[size_t(type)];
}

const char* GetExprTypeName(const Expr& expr) {
  return GetExprTypeName(expr.type);
}

bool FuncSignature::operator==(const FuncSignature& rhs) const {
  return param_types == rhs.param_types && result_types == rhs.result_types;
}

const Export* Module::GetExport(const StringSlice& name) const {
  Index index = export_bindings.FindIndex(name);
  if (index >= exports.size())
    return nullptr;
  return exports[index];
}

Index Module::GetFuncIndex(const Var& var) const {
  return func_bindings.FindIndex(var);
}

Index Module::GetGlobalIndex(const Var& var) const {
  return global_bindings.FindIndex(var);
}

Index Module::GetTableIndex(const Var& var) const {
  return table_bindings.FindIndex(var);
}

Index Module::GetMemoryIndex(const Var& var) const {
  return memory_bindings.FindIndex(var);
}

Index Module::GetFuncTypeIndex(const Var& var) const {
  return func_type_bindings.FindIndex(var);
}

Index Module::GetExceptIndex(const Var& var) const {
  return except_bindings.FindIndex(var);
}

Index Func::GetLocalIndex(const Var& var) const {
  if (var.type == VarType::Index)
    return var.index;

  Index result = param_bindings.FindIndex(var.name);
  if (result != kInvalidIndex)
    return result;

  result = local_bindings.FindIndex(var.name);
  if (result == kInvalidIndex)
    return result;

  // The locals start after all the params.
  return decl.GetNumParams() + result;
}

const Func* Module::GetFunc(const Var& var) const {
  return const_cast<Module*>(this)->GetFunc(var);
}

Func* Module::GetFunc(const Var& var) {
  Index index = func_bindings.FindIndex(var);
  if (index >= funcs.size())
    return nullptr;
  return funcs[index];
}

const Global* Module::GetGlobal(const Var& var) const {
  return const_cast<Module*>(this)->GetGlobal(var);
}

Global* Module::GetGlobal(const Var& var) {
  Index index = global_bindings.FindIndex(var);
  if (index >= globals.size())
    return nullptr;
  return globals[index];
}

Table* Module::GetTable(const Var& var) {
  Index index = table_bindings.FindIndex(var);
  if (index >= tables.size())
    return nullptr;
  return tables[index];
}

Memory* Module::GetMemory(const Var& var) {
  Index index = memory_bindings.FindIndex(var);
  if (index >= memories.size())
    return nullptr;
  return memories[index];
}

Exception* Module::GetExcept(const Var& var) const {
  Index index = GetExceptIndex(var);
  if (index >= excepts.size())
    return nullptr;
  return excepts[index];
}

const FuncType* Module::GetFuncType(const Var& var) const {
  return const_cast<Module*>(this)->GetFuncType(var);
}

FuncType* Module::GetFuncType(const Var& var) {
  Index index = func_type_bindings.FindIndex(var);
  if (index >= func_types.size())
    return nullptr;
  return func_types[index];
}


Index Module::GetFuncTypeIndex(const FuncSignature& sig) const {
  for (size_t i = 0; i < func_types.size(); ++i)
    if (func_types[i]->sig == sig)
      return i;
  return kInvalidIndex;
}

Index Module::GetFuncTypeIndex(const FuncDeclaration& decl) const {
  if (decl.has_func_type) {
    return GetFuncTypeIndex(decl.type_var);
  } else {
    return GetFuncTypeIndex(decl.sig);
  }
}

const Module* Script::GetFirstModule() const {
  return const_cast<Script*>(this)->GetFirstModule();
}

Module* Script::GetFirstModule() {
  for (const std::unique_ptr<Command>& command : commands) {
    if (command->type == CommandType::Module)
      return command->module;
  }
  return nullptr;
}

const Module* Script::GetModule(const Var& var) const {
  Index index = module_bindings.FindIndex(var);
  if (index >= commands.size())
    return nullptr;
  const Command& command = *commands[index].get();
  assert(command.type == CommandType::Module);
  return command.module;
}

void MakeTypeBindingReverseMapping(
    const TypeVector& types,
    const BindingHash& bindings,
    std::vector<std::string>* out_reverse_mapping) {
  out_reverse_mapping->clear();
  out_reverse_mapping->resize(types.size());
  for (const auto& pair : bindings) {
    assert(static_cast<size_t>(pair.second.index) <
           out_reverse_mapping->size());
    (*out_reverse_mapping)[pair.second.index] = pair.first;
  }
}

void Module::AppendField(ModuleField* field) {
  if (!first_field)
    first_field = field;
  else if (last_field)
    last_field->next = field;
  last_field = field;
}

FuncType* Module::AppendImplicitFuncType(const Location& loc,
                                         const FuncSignature& sig) {
  FuncType* func_type = new FuncType();
  func_type->sig = sig;
  func_types.push_back(func_type);
  AppendField(new FuncTypeModuleField(func_type, loc));
  return func_type;
}

void DestroyExprList(Expr* first) {
  Expr* expr = first;
  while (expr) {
    Expr* next = expr->next;
    delete expr;
    expr = next;
  }
}

Var::Var(Index index) : type(VarType::Index), index(index) {
  WABT_ZERO_MEMORY(loc);
}

Var::Var(const StringSlice& name) : type(VarType::Name), name(name) {
  WABT_ZERO_MEMORY(loc);
}

Var::Var(Index index, const Location& loc_) : Var(index) {
  loc = loc;
}

Var::Var(const StringSlice& name, const Location& loc_) : Var(name) {
  loc = loc;
}

Var::Var(Var&& rhs) : loc(rhs.loc), type(rhs.type) {
  if (rhs.type == VarType::Index) {
    index = rhs.index;
  } else {
    name = rhs.name;
    rhs = Var(kInvalidIndex);
  }
}

Var::Var(const Var& rhs) : loc(rhs.loc), type(rhs.type) {
  if (rhs.type == VarType::Index) {
    index = rhs.index;
  } else {
    name = dup_string_slice(rhs.name);
  }
}

Var& Var::operator =(Var&& rhs) {
  loc = rhs.loc;
  type = rhs.type;
  if (rhs.type == VarType::Index) {
    index = rhs.index;
  } else {
    name = rhs.name;
    rhs = Var(kInvalidIndex);
  }
  return *this;
}

Var& Var::operator =(const Var& rhs) {
  loc = rhs.loc;
  type = rhs.type;
  if (rhs.type == VarType::Index) {
    index = rhs.index;
  } else {
    name = dup_string_slice(rhs.name);
  }
  return *this;
}

Var::~Var() {
  if (type == VarType::Name)
    destroy_string_slice(&name);
}

Const::Const(I32, uint32_t value) : type(Type::I32), u32(value) {
  WABT_ZERO_MEMORY(loc);
}

Const::Const(I64, uint64_t value) : type(Type::I64), u64(value) {
  WABT_ZERO_MEMORY(loc);
}

Const::Const(F32, uint32_t value) : type(Type::F32), f32_bits(value) {
  WABT_ZERO_MEMORY(loc);
}

Const::Const(F64, uint64_t value) : type(Type::F64), f64_bits(value) {
  WABT_ZERO_MEMORY(loc);
}

Const::Const(I32 tag, uint32_t value, const Location& loc_)
    : Const(tag, value) {
  loc = loc_;
}

Const::Const(I64 tag, uint64_t value, const Location& loc_)
    : Const(tag, value) {
  loc = loc_;
}

Const::Const(F32 tag, uint32_t value, const Location& loc_)
    : Const(tag, value) {
  loc = loc_;
}

Const::Const(F64 tag, uint64_t value, const Location& loc_)
    : Const(tag, value) {
  loc = loc_;
}


Block::Block(): first(nullptr) {
  WABT_ZERO_MEMORY(label);
}

Block::Block(Expr* first) : first(first) {
  WABT_ZERO_MEMORY(label);
}

Block::~Block() {
  destroy_string_slice(&label);
  DestroyExprList(first);
}

Catch::Catch(Expr* first) : first(first) {
  WABT_ZERO_MEMORY(loc);
}

Catch::Catch(Var var, Expr* first) : var(var), first(first) {
  WABT_ZERO_MEMORY(loc);
}

Catch::~Catch() {
  delete first;
}

IfExpr::~IfExpr() {
  delete true_;
  DestroyExprList(false_);
}

TryExpr::~TryExpr() {
  delete block;
  for (Catch* catch_ : catches)
    delete catch_;
}

Expr::Expr(ExprType type) : type(type), next(nullptr) {
  WABT_ZERO_MEMORY(loc);
}

FuncType::FuncType() {
  WABT_ZERO_MEMORY(name);
}

FuncType::~FuncType() {
  destroy_string_slice(&name);
}

FuncDeclaration::FuncDeclaration()
    : has_func_type(false), type_var(kInvalidIndex) {}

FuncDeclaration::~FuncDeclaration() {}

Func::Func() : first_expr(nullptr) {
  WABT_ZERO_MEMORY(name);
}

Func::~Func() {
  destroy_string_slice(&name);
  DestroyExprList(first_expr);
}

Global::Global() : type(Type::Void), mutable_(false), init_expr(nullptr) {
  WABT_ZERO_MEMORY(name);
}

Global::~Global() {
  destroy_string_slice(&name);
  DestroyExprList(init_expr);
}

Table::Table() {
  WABT_ZERO_MEMORY(name);
  WABT_ZERO_MEMORY(elem_limits);
}

Table::~Table() {
  destroy_string_slice(&name);
}

ElemSegment::ElemSegment() : table_var(kInvalidIndex), offset(nullptr) {}

ElemSegment::~ElemSegment() {
  DestroyExprList(offset);
}

DataSegment::DataSegment() : offset(nullptr), data(nullptr), size(0) {}

DataSegment::~DataSegment() {
  DestroyExprList(offset);
  delete[] data;
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
    case ExternalKind::Except:
      delete except;
      break;
  }
}

Export::Export() {
  WABT_ZERO_MEMORY(name);
}

Export::~Export() {
  destroy_string_slice(&name);
}

ModuleField::ModuleField(ModuleFieldType type, const Location& loc)
    : loc(loc), type(type), next(nullptr) {}

Module::Module()
    : first_field(nullptr),
      last_field(nullptr),
      num_except_imports(0),
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

ScriptModule::ScriptModule() : type(ScriptModule::Type::Text), text(nullptr) {}

ScriptModule::~ScriptModule() {
  switch (type) {
    case ScriptModule::Type::Text:
      delete text;
      break;
    case ScriptModule::Type::Binary:
      destroy_string_slice(&binary.name);
      delete [] binary.data;
      break;
    case ScriptModule::Type::Quoted:
      destroy_string_slice(&quoted.name);
      delete [] binary.data;
      break;
  }
}

ActionInvoke::ActionInvoke() {}

Action::Action() : type(ActionType::Get), module_var(kInvalidIndex) {
  WABT_ZERO_MEMORY(loc);
  WABT_ZERO_MEMORY(name);
}

Action::~Action() {
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
      register_.var.~Var();
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
    case CommandType::AssertReturnCanonicalNan:
      delete assert_return_arithmetic_nan.action;
      break;
    case CommandType::AssertReturnArithmeticNan:
      delete assert_return_canonical_nan.action;
      break;
    case CommandType::AssertTrap:
    case CommandType::AssertExhaustion:
      delete assert_trap.action;
      destroy_string_slice(&assert_trap.text);
      break;
  }
}

Script::Script() {}

}  // namespace wabt
