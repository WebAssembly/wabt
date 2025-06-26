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

#include "wabt/ir.h"

#include <cassert>
#include <cstddef>
#include <numeric>

#include "wabt/cast.h"

namespace {

const char* ExprTypeName[] = {
    "AtomicFence",
    "AtomicLoad",
    "AtomicRmw",
    "AtomicRmwCmpxchg",
    "AtomicStore",
    "AtomicNotify",
    "AtomicWait",
    "Binary",
    "Block",
    "Br",
    "BrIf",
    "BrTable",
    "Call",
    "CallIndirect",
    "CallRef",
    "CodeMetadata",
    "Compare",
    "Const",
    "Convert",
    "Drop",
    "GlobalGet",
    "GlobalSet",
    "If",
    "Load",
    "LocalGet",
    "LocalSet",
    "LocalTee",
    "Loop",
    "MemoryCopy",
    "DataDrop",
    "MemoryFill",
    "MemoryGrow",
    "MemoryInit",
    "MemorySize",
    "Nop",
    "RefIsNull",
    "RefFunc",
    "RefNull",
    "Rethrow",
    "Return",
    "ReturnCall",
    "ReturnCallIndirect",
    "Select",
    "SimdLaneOp",
    "SimdLoadLane",
    "SimdStoreLane",
    "SimdShuffleOp",
    "LoadSplat",
    "LoadZero",
    "Store",
    "TableCopy",
    "ElemDrop",
    "TableInit",
    "TableGet",
    "TableGrow",
    "TableSize",
    "TableSet",
    "TableFill",
    "Ternary",
    "Throw",
    "ThrowRef",
    "Try",
    "TryTable",
    "Unary",
    "Unreachable",
};

}  // end of anonymous namespace

namespace wabt {

const char* GetExprTypeName(ExprType type) {
  static_assert(WABT_ENUM_COUNT(ExprType) == WABT_ARRAY_SIZE(ExprTypeName),
                "Malformed ExprTypeName array");
  return ExprTypeName[size_t(type)];
}

const char* GetExprTypeName(const Expr& expr) {
  return GetExprTypeName(expr.type());
}

bool FuncSignature::operator==(const FuncSignature& rhs) const {
  if (param_types.size() != rhs.param_types.size() ||
      result_types.size() != rhs.result_types.size()) {
    return false;
  }

  if (param_types.size() > 0 &&
      memcmp(param_types.data(), rhs.param_types.data(),
             param_types.size() * sizeof(Type)) != 0) {
    return false;
  }

  if (result_types.size() > 0 &&
      memcmp(result_types.data(), rhs.result_types.data(),
             result_types.size() * sizeof(Type)) != 0) {
    return false;
  }

  return true;
}

const Export* Module::GetExport(std::string_view name) const {
  Index index = export_bindings.FindIndex(name);
  if (index >= exports.size()) {
    return nullptr;
  }
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
  return type_bindings.FindIndex(var);
}

Index Module::GetTagIndex(const Var& var) const {
  return tag_bindings.FindIndex(var);
}

Index Module::GetDataSegmentIndex(const Var& var) const {
  return data_segment_bindings.FindIndex(var);
}

Index Module::GetElemSegmentIndex(const Var& var) const {
  return elem_segment_bindings.FindIndex(var);
}

bool Module::IsImport(ExternalKind kind, const Var& var) const {
  switch (kind) {
    case ExternalKind::Func:
      return GetFuncIndex(var) < num_func_imports;

    case ExternalKind::Global:
      return GetGlobalIndex(var) < num_global_imports;

    case ExternalKind::Memory:
      return GetMemoryIndex(var) < num_memory_imports;

    case ExternalKind::Table:
      return GetTableIndex(var) < num_table_imports;

    case ExternalKind::Tag:
      return GetTagIndex(var) < num_tag_imports;

    default:
      return false;
  }
}

void LocalTypes::Set(const TypeVector& types) {
  decls_.clear();
  if (types.empty()) {
    return;
  }

  Type type = types[0];
  Index count = 1;
  for (Index i = 1; i < types.size(); ++i) {
    if (types[i] != type) {
      decls_.emplace_back(type, count);
      type = types[i];
      count = 1;
    } else {
      ++count;
    }
  }
  decls_.emplace_back(type, count);
}

Index LocalTypes::size() const {
  return std::accumulate(
      decls_.begin(), decls_.end(), 0,
      [](Index sum, const Decl& decl) { return sum + decl.second; });
}

Type LocalTypes::operator[](Index i) const {
  Index count = 0;
  for (auto decl : decls_) {
    if (i < count + decl.second) {
      return decl.first;
    }
    count += decl.second;
  }
  assert(i < count);
  return Type::Any;
}

Type Func::GetLocalType(Index index) const {
  Index num_params = decl.GetNumParams();
  if (index < num_params) {
    return GetParamType(index);
  } else {
    index -= num_params;
    assert(index < local_types.size());
    return local_types[index];
  }
}

Type Func::GetLocalType(const Var& var) const {
  return GetLocalType(GetLocalIndex(var));
}

Index Func::GetLocalIndex(const Var& var) const {
  if (var.is_index()) {
    return var.index();
  }
  return bindings.FindIndex(var);
}

const Func* Module::GetFunc(const Var& var) const {
  return const_cast<Module*>(this)->GetFunc(var);
}

Func* Module::GetFunc(const Var& var) {
  Index index = func_bindings.FindIndex(var);
  if (index >= funcs.size()) {
    return nullptr;
  }
  return funcs[index];
}

const Global* Module::GetGlobal(const Var& var) const {
  return const_cast<Module*>(this)->GetGlobal(var);
}

Global* Module::GetGlobal(const Var& var) {
  Index index = global_bindings.FindIndex(var);
  if (index >= globals.size()) {
    return nullptr;
  }
  return globals[index];
}

const Table* Module::GetTable(const Var& var) const {
  return const_cast<Module*>(this)->GetTable(var);
}

Table* Module::GetTable(const Var& var) {
  Index index = table_bindings.FindIndex(var);
  if (index >= tables.size()) {
    return nullptr;
  }
  return tables[index];
}

const Memory* Module::GetMemory(const Var& var) const {
  return const_cast<Module*>(this)->GetMemory(var);
}

Memory* Module::GetMemory(const Var& var) {
  Index index = memory_bindings.FindIndex(var);
  if (index >= memories.size()) {
    return nullptr;
  }
  return memories[index];
}

Tag* Module::GetTag(const Var& var) const {
  Index index = GetTagIndex(var);
  if (index >= tags.size()) {
    return nullptr;
  }
  return tags[index];
}

const DataSegment* Module::GetDataSegment(const Var& var) const {
  return const_cast<Module*>(this)->GetDataSegment(var);
}

DataSegment* Module::GetDataSegment(const Var& var) {
  Index index = data_segment_bindings.FindIndex(var);
  if (index >= data_segments.size()) {
    return nullptr;
  }
  return data_segments[index];
}

const ElemSegment* Module::GetElemSegment(const Var& var) const {
  return const_cast<Module*>(this)->GetElemSegment(var);
}

ElemSegment* Module::GetElemSegment(const Var& var) {
  Index index = elem_segment_bindings.FindIndex(var);
  if (index >= elem_segments.size()) {
    return nullptr;
  }
  return elem_segments[index];
}

const FuncType* Module::GetFuncType(const Var& var) const {
  return const_cast<Module*>(this)->GetFuncType(var);
}

FuncType* Module::GetFuncType(const Var& var) {
  Index index = type_bindings.FindIndex(var);
  if (index >= types.size()) {
    return nullptr;
  }
  return dyn_cast<FuncType>(types[index]);
}

Index Module::GetFuncTypeIndex(const FuncSignature& sig) const {
  for (size_t i = 0; i < types.size(); ++i) {
    if (auto* func_type = dyn_cast<FuncType>(types[i])) {
      if (func_type->sig == sig) {
        return i;
      }
    }
  }
  return kInvalidIndex;
}

Index Module::GetFuncTypeIndex(const FuncDeclaration& decl) const {
  if (decl.has_func_type) {
    return GetFuncTypeIndex(decl.type_var);
  } else {
    return GetFuncTypeIndex(decl.sig);
  }
}

void Module::AppendField(std::unique_ptr<DataSegmentModuleField> field) {
  DataSegment& data_segment = field->data_segment;
  if (!data_segment.name.empty()) {
    data_segment_bindings.emplace(data_segment.name,
                                  Binding(field->loc, data_segments.size()));
  }
  data_segments.push_back(&data_segment);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<ElemSegmentModuleField> field) {
  ElemSegment& elem_segment = field->elem_segment;
  if (!elem_segment.name.empty()) {
    elem_segment_bindings.emplace(elem_segment.name,
                                  Binding(field->loc, elem_segments.size()));
  }
  elem_segments.push_back(&elem_segment);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<TagModuleField> field) {
  Tag& tag = field->tag;
  if (!tag.name.empty()) {
    tag_bindings.emplace(tag.name, Binding(field->loc, tags.size()));
  }
  tags.push_back(&tag);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<ExportModuleField> field) {
  // Exported names are allowed to be empty.
  Export& export_ = field->export_;
  export_bindings.emplace(export_.name, Binding(field->loc, exports.size()));
  exports.push_back(&export_);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<FuncModuleField> field) {
  Func& func = field->func;
  if (!func.name.empty()) {
    func_bindings.emplace(func.name, Binding(field->loc, funcs.size()));
  }
  funcs.push_back(&func);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<TypeModuleField> field) {
  TypeEntry& type = *field->type;
  if (!type.name.empty()) {
    type_bindings.emplace(type.name, Binding(field->loc, types.size()));
  }
  types.push_back(&type);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<GlobalModuleField> field) {
  Global& global = field->global;
  if (!global.name.empty()) {
    global_bindings.emplace(global.name, Binding(field->loc, globals.size()));
  }
  globals.push_back(&global);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<ImportModuleField> field) {
  Import* import = field->import.get();
  const std::string* name = nullptr;
  BindingHash* bindings = nullptr;
  Index index = kInvalidIndex;

  switch (import->kind()) {
    case ExternalKind::Func: {
      Func& func = cast<FuncImport>(import)->func;
      name = &func.name;
      bindings = &func_bindings;
      index = funcs.size();
      funcs.push_back(&func);
      ++num_func_imports;
      break;
    }

    case ExternalKind::Table: {
      Table& table = cast<TableImport>(import)->table;
      name = &table.name;
      bindings = &table_bindings;
      index = tables.size();
      tables.push_back(&table);
      ++num_table_imports;
      break;
    }

    case ExternalKind::Memory: {
      Memory& memory = cast<MemoryImport>(import)->memory;
      name = &memory.name;
      bindings = &memory_bindings;
      index = memories.size();
      memories.push_back(&memory);
      ++num_memory_imports;
      break;
    }

    case ExternalKind::Global: {
      Global& global = cast<GlobalImport>(import)->global;
      name = &global.name;
      bindings = &global_bindings;
      index = globals.size();
      globals.push_back(&global);
      ++num_global_imports;
      break;
    }

    case ExternalKind::Tag: {
      Tag& tag = cast<TagImport>(import)->tag;
      name = &tag.name;
      bindings = &tag_bindings;
      index = tags.size();
      tags.push_back(&tag);
      ++num_tag_imports;
      break;
    }
  }

  assert(name && bindings && index != kInvalidIndex);
  if (!name->empty()) {
    bindings->emplace(*name, Binding(field->loc, index));
  }
  imports.push_back(import);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<MemoryModuleField> field) {
  Memory& memory = field->memory;
  if (!memory.name.empty()) {
    memory_bindings.emplace(memory.name, Binding(field->loc, memories.size()));
  }
  memories.push_back(&memory);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<StartModuleField> field) {
  starts.push_back(&field->start);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<TableModuleField> field) {
  Table& table = field->table;
  if (!table.name.empty()) {
    table_bindings.emplace(table.name, Binding(field->loc, tables.size()));
  }
  tables.push_back(&table);
  fields.push_back(std::move(field));
}

void Module::AppendField(std::unique_ptr<ModuleField> field) {
  switch (field->type()) {
    case ModuleFieldType::Func:
      AppendField(cast<FuncModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Global:
      AppendField(cast<GlobalModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Import:
      AppendField(cast<ImportModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Export:
      AppendField(cast<ExportModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Type:
      AppendField(cast<TypeModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Table:
      AppendField(cast<TableModuleField>(std::move(field)));
      break;

    case ModuleFieldType::ElemSegment:
      AppendField(cast<ElemSegmentModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Memory:
      AppendField(cast<MemoryModuleField>(std::move(field)));
      break;

    case ModuleFieldType::DataSegment:
      AppendField(cast<DataSegmentModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Start:
      AppendField(cast<StartModuleField>(std::move(field)));
      break;

    case ModuleFieldType::Tag:
      AppendField(cast<TagModuleField>(std::move(field)));
      break;
  }
}

void Module::AppendFields(ModuleFieldList* fields) {
  while (!fields->empty())
    AppendField(std::unique_ptr<ModuleField>(fields->extract_front()));
}

const Module* Script::GetFirstModule() const {
  return const_cast<Script*>(this)->GetFirstModule();
}

Module* Script::GetFirstModule() {
  for (const std::unique_ptr<Command>& command : commands) {
    if (auto* module_command = dyn_cast<ModuleCommand>(command.get())) {
      return &module_command->module;
    }
  }
  return nullptr;
}

const Module* Script::GetModule(const Var& var) const {
  Index index = module_bindings.FindIndex(var);
  if (index >= commands.size()) {
    return nullptr;
  }
  auto* command = commands[index].get();
  if (isa<ModuleCommand>(command)) {
    return &cast<ModuleCommand>(command)->module;
  } else if (isa<ScriptModuleCommand>(command)) {
    return &cast<ScriptModuleCommand>(command)->module;
  }
  return nullptr;
}

void MakeTypeBindingReverseMapping(
    size_t num_types,
    const BindingHash& bindings,
    std::vector<std::string>* out_reverse_mapping) {
  out_reverse_mapping->clear();
  out_reverse_mapping->resize(num_types);
  for (const auto& [name, binding] : bindings) {
    assert(static_cast<size_t>(binding.index) < out_reverse_mapping->size());
    (*out_reverse_mapping)[binding.index] = name;
  }
}

Var::Var() : Var(kInvalidIndex, Location()) {}

Var::Var(Index index, const Location& loc)
    : loc(loc), type_(VarType::Index), opt_type_(0), index_(index) {}

Var::Var(std::string_view name, const Location& loc)
    : loc(loc), type_(VarType::Name), opt_type_(0), name_(name) {}

Var::Var(Type type, const Location& loc)
    : loc(loc), type_(VarType::Index), index_(0) {
  assert(static_cast<int32_t>(type) < 0 &&
         static_cast<int32_t>(type) >= INT16_MIN);
  opt_type_ = static_cast<int16_t>(type);

  if (type.IsReferenceWithIndex()) {
    index_ = type.GetReferenceIndex();
  } else if (type.IsNonTypedRef()) {
    index_ = type.IsNullableNonTypedRef() ? Type::ReferenceOrNull
                                          : Type::ReferenceNonNull;
  }
}

Var::Var(Var&& rhs) : Var() {
  *this = std::move(rhs);
}

Var::Var(const Var& rhs) : Var() {
  *this = rhs;
}

Var& Var::operator=(Var&& rhs) {
  loc = rhs.loc;
  opt_type_ = rhs.opt_type_;
  if (rhs.is_index()) {
    set_index(rhs.index_);
  } else {
    set_name(rhs.name_);
  }
  return *this;
}

Var& Var::operator=(const Var& rhs) {
  loc = rhs.loc;
  opt_type_ = rhs.opt_type_;
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

void Var::set_name(std::string_view name) {
  set_name(std::string(name));
}

void Var::set_opt_type(Type::Enum type) {
  assert(static_cast<int32_t>(type) < 0 &&
         static_cast<int32_t>(type) >= INT16_MIN);
  opt_type_ = static_cast<int16_t>(type);
}

Type Var::to_type() const {
  Type::Enum type = static_cast<Type::Enum>(opt_type_);

  if (Type::EnumIsReferenceWithIndex(type) || Type::EnumIsNonTypedRef(type)) {
    return Type(type, index());
  }

  return Type(type);
}

void Var::Destroy() {
  if (is_name()) {
    Destruct(name_);
  }
}

uint8_t ElemSegment::GetFlags(const Module* module) const {
  uint8_t flags = 0;

  switch (kind) {
    case SegmentKind::Active: {
      Index table_index = module->GetTableIndex(table_var);
      if (elem_type != Type::FuncRef || table_index != 0) {
        flags |= SegExplicitIndex;
      }
      break;
    }

    case SegmentKind::Passive:
      flags |= SegPassive;
      break;

    case SegmentKind::Declared:
      flags |= SegDeclared;
      break;
  }

  bool all_ref_func =
      elem_type == Type::FuncRef &&
      std::all_of(elem_exprs.begin(), elem_exprs.end(),
                  [](const ExprList& elem_expr) {
                    return elem_expr.front().type() == ExprType::RefFunc;
                  });

  if (!all_ref_func) {
    flags |= SegUseElemExprs;
  }

  return flags;
}

uint8_t DataSegment::GetFlags(const Module* module) const {
  uint8_t flags = 0;

  if (kind == SegmentKind::Passive) {
    flags |= SegPassive;
  }

  Index memory_index = module->GetMemoryIndex(memory_var);
  if (memory_index != 0) {
    flags |= SegExplicitIndex;
  }

  return flags;
}

}  // namespace wabt
