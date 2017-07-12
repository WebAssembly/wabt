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

#include "cast.h"

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

const Export* Module::GetExport(string_view name) const {
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
  if (var.is_index())
    return var.index();

  Index result = param_bindings.FindIndex(var);
  if (result != kInvalidIndex)
    return result;

  result = local_bindings.FindIndex(var);
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
    if (auto* module_command = dyn_cast<ModuleCommand>(command.get()))
      return module_command->module;
  }
  return nullptr;
}

const Module* Script::GetModule(const Var& var) const {
  Index index = module_bindings.FindIndex(var);
  if (index >= commands.size())
    return nullptr;
  auto* command = cast<ModuleCommand>(commands[index].get());
  return command->module;
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

FuncType* Module::AppendImplicitFuncType(const Location& loc,
                                         const FuncSignature& sig) {
  FuncType* func_type = new FuncType();
  func_type->sig = sig;
  func_types.push_back(func_type);
  fields.push_back(new FuncTypeModuleField(func_type, loc));
  return func_type;
}

Var::Var(Index index, const Location& loc)
    : loc(loc), type_(VarType::Index), index_(index) {}

Var::Var(string_view name, const Location& loc)
    : loc(loc), type_(VarType::Name), name_(name) {}

Var::Var(Var&& rhs) : Var(kInvalidIndex) {
  *this = std::move(rhs);
}

Var::Var(const Var& rhs) : Var(kInvalidIndex) {
  *this = rhs;
}

Var& Var::operator=(Var&& rhs) {
  loc = rhs.loc;
  if (rhs.is_index()) {
    set_index(rhs.index_);
  } else {
    set_name(rhs.name_);
  }
  return *this;
}

Var& Var::operator=(const Var& rhs) {
  loc = rhs.loc;
  if (rhs.is_index()) {
    set_index(rhs.index_);
  } else {
    set_name(rhs.name_);
  }
  return *this;
}

Var::~Var() {
  Destroy();
}

void Var::set_index(Index index) {
  Destroy();
  type_ = VarType::Index;
  index_ = index;
}

void Var::set_name(std::string&& name) {
  Destroy();
  type_ = VarType::Name;
  Construct(name_, std::move(name));
}

void Var::set_name(string_view name) {
  set_name(name.to_string());
}

void Var::Destroy() {
  if (is_name())
    Destruct(name_);
}

Const::Const(I32, uint32_t value, const Location& loc_)
    : loc(loc_), type(Type::I32), u32(value) {
}

Const::Const(I64, uint64_t value, const Location& loc_)
    : loc(loc_), type(Type::I64), u64(value) {
}

Const::Const(F32, uint32_t value, const Location& loc_)
    : loc(loc_), type(Type::F32), f32_bits(value) {
}

Const::Const(F64, uint64_t value, const Location& loc_)
    : loc(loc_), type(Type::F64), f64_bits(value) {
}

Block::Block(ExprList exprs) : exprs(std::move(exprs)) {}

Catch::Catch() {}

Catch::Catch(const Var& var) : var(var) {}

Catch::Catch(ExprList exprs) : exprs(std::move(exprs)) {}

Catch::Catch(const Var& var, ExprList exprs)
    : var(var), exprs(std::move(exprs)) {}

IfExpr::~IfExpr() {
  delete true_;
}

TryExpr::~TryExpr() {
  delete block;
  for (Catch* catch_ : catches)
    delete catch_;
}

Expr::Expr(ExprType type) : type(type) {}

Table::Table() {
  ZeroMemory(elem_limits);
}
DataSegment::DataSegment() : data(nullptr), size(0) {}

DataSegment::~DataSegment() {
  delete[] data;
}

Memory::Memory() {
  ZeroMemory(page_limits);
}

Import::Import() : kind(ExternalKind::Func), func(nullptr) {}

Import::~Import() {
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

ModuleField::ModuleField(ModuleFieldType type, const Location& loc)
    : loc(loc), type(type) {}

ScriptModule::ScriptModule(Type type) : type(type) {
  switch (type) {
    case ScriptModule::Type::Text:
      text = nullptr;
      break;

    case ScriptModule::Type::Binary:
      Construct(binary.loc);
      Construct(binary.name);
      binary.data = nullptr;
      binary.size = 0;
      break;

    case ScriptModule::Type::Quoted:
      Construct(quoted.loc);
      Construct(quoted.name);
      quoted.data = nullptr;
      quoted.size = 0;
      break;
  }
}

ScriptModule::~ScriptModule() {
  switch (type) {
    case ScriptModule::Type::Text:
      delete text;
      break;
    case ScriptModule::Type::Binary:
      Destruct(binary.loc);
      Destruct(binary.name);
      delete [] binary.data;
      break;
    case ScriptModule::Type::Quoted:
      Destruct(quoted.loc);
      Destruct(quoted.name);
      delete [] binary.data;
      break;
  }
}

Action::Action() : type(ActionType::Get), module_var(kInvalidIndex) {}

Action::~Action() {
  switch (type) {
    case ActionType::Invoke:
      delete invoke;
      break;
    case ActionType::Get:
      break;
  }
}

}  // namespace wabt
