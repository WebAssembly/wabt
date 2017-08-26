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

#include "binary-reader-ir.h"

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "binary-reader-nop.h"
#include "cast.h"
#include "common.h"
#include "error-handler.h"
#include "ir.h"

#define CHECK_RESULT(expr)  \
  do {                      \
    if (Failed(expr))       \
      return Result::Error; \
  } while (0)

namespace wabt {

namespace {

struct LabelNode {
  LabelNode(LabelType, ExprList* exprs, Expr* context = nullptr);

  LabelType label_type;
  ExprList* exprs;
  Expr* context;
};

LabelNode::LabelNode(LabelType label_type, ExprList* exprs, Expr* context)
    : label_type(label_type), exprs(exprs), context(context) {}

class BinaryReaderIR : public BinaryReaderNop {
 public:
  BinaryReaderIR(Module* out_module,
                 const char* filename,
                 ErrorHandler* error_handler);

  bool OnError(const char* message) override;

  Result OnTypeCount(Index count) override;
  Result OnType(Index index,
                Index param_count,
                Type* param_types,
                Index result_count,
                Type* result_types) override;

  Result OnImportCount(Index count) override;
  Result OnImportFunc(Index import_index,
                      string_view module_name,
                      string_view field_name,
                      Index func_index,
                      Index sig_index) override;
  Result OnImportTable(Index import_index,
                       string_view module_name,
                       string_view field_name,
                       Index table_index,
                       Type elem_type,
                       const Limits* elem_limits) override;
  Result OnImportMemory(Index import_index,
                        string_view module_name,
                        string_view field_name,
                        Index memory_index,
                        const Limits* page_limits) override;
  Result OnImportGlobal(Index import_index,
                        string_view module_name,
                        string_view field_name,
                        Index global_index,
                        Type type,
                        bool mutable_) override;
  Result OnImportException(Index import_index,
                           string_view module_name,
                           string_view field_name,
                           Index except_index,
                           TypeVector& sig) override;

  Result OnFunctionCount(Index count) override;
  Result OnFunction(Index index, Index sig_index) override;

  Result OnTableCount(Index count) override;
  Result OnTable(Index index,
                 Type elem_type,
                 const Limits* elem_limits) override;

  Result OnMemoryCount(Index count) override;
  Result OnMemory(Index index, const Limits* limits) override;

  Result OnGlobalCount(Index count) override;
  Result BeginGlobal(Index index, Type type, bool mutable_) override;
  Result BeginGlobalInitExpr(Index index) override;
  Result EndGlobalInitExpr(Index index) override;

  Result OnExportCount(Index count) override;
  Result OnExport(Index index,
                  ExternalKind kind,
                  Index item_index,
                  string_view name) override;

  Result OnStartFunction(Index func_index) override;

  Result OnFunctionBodyCount(Index count) override;
  Result BeginFunctionBody(Index index) override;
  Result OnLocalDecl(Index decl_index, Index count, Type type) override;

  Result OnBinaryExpr(Opcode opcode) override;
  Result OnBlockExpr(Index num_types, Type* sig_types) override;
  Result OnBrExpr(Index depth) override;
  Result OnBrIfExpr(Index depth) override;
  Result OnBrTableExpr(Index num_targets,
                       Index* target_depths,
                       Index default_target_depth) override;
  Result OnCallExpr(Index func_index) override;
  Result OnCatchExpr(Index except_index) override;
  Result OnCatchAllExpr() override;
  Result OnCallIndirectExpr(Index sig_index) override;
  Result OnCompareExpr(Opcode opcode) override;
  Result OnConvertExpr(Opcode opcode) override;
  Result OnDropExpr() override;
  Result OnElseExpr() override;
  Result OnEndExpr() override;
  Result OnF32ConstExpr(uint32_t value_bits) override;
  Result OnF64ConstExpr(uint64_t value_bits) override;
  Result OnGetGlobalExpr(Index global_index) override;
  Result OnGetLocalExpr(Index local_index) override;
  Result OnGrowMemoryExpr() override;
  Result OnI32ConstExpr(uint32_t value) override;
  Result OnI64ConstExpr(uint64_t value) override;
  Result OnIfExpr(Index num_types, Type* sig_types) override;
  Result OnLoadExpr(Opcode opcode,
                    uint32_t alignment_log2,
                    Address offset) override;
  Result OnLoopExpr(Index num_types, Type* sig_types) override;
  Result OnCurrentMemoryExpr() override;
  Result OnNopExpr() override;
  Result OnRethrowExpr(Index depth) override;
  Result OnReturnExpr() override;
  Result OnSelectExpr() override;
  Result OnSetGlobalExpr(Index global_index) override;
  Result OnSetLocalExpr(Index local_index) override;
  Result OnStoreExpr(Opcode opcode,
                     uint32_t alignment_log2,
                     Address offset) override;
  Result OnThrowExpr(Index except_index) override;
  Result OnTeeLocalExpr(Index local_index) override;
  Result OnTryExpr(Index num_types, Type* sig_types) override;
  Result OnUnaryExpr(Opcode opcode) override;
  Result OnUnreachableExpr() override;
  Result EndFunctionBody(Index index) override;

  Result OnElemSegmentCount(Index count) override;
  Result BeginElemSegment(Index index, Index table_index) override;
  Result BeginElemSegmentInitExpr(Index index) override;
  Result EndElemSegmentInitExpr(Index index) override;
  Result OnElemSegmentFunctionIndexCount(Index index,
                                         Index count) override;
  Result OnElemSegmentFunctionIndex(Index index,
                                    Index func_index) override;

  Result OnDataSegmentCount(Index count) override;
  Result BeginDataSegment(Index index, Index memory_index) override;
  Result BeginDataSegmentInitExpr(Index index) override;
  Result EndDataSegmentInitExpr(Index index) override;
  Result OnDataSegmentData(Index index,
                           const void* data,
                           Address size) override;

  Result OnFunctionNamesCount(Index num_functions) override;
  Result OnFunctionName(Index function_index,
                        string_view function_name) override;
  Result OnLocalNameLocalCount(Index function_index,
                               Index num_locals) override;
  Result OnLocalName(Index function_index,
                     Index local_index,
                     string_view local_name) override;

  Result BeginExceptionSection(Offset size) override { return Result::Ok; }
  Result OnExceptionCount(Index count) override { return Result::Ok; }
  Result OnExceptionType(Index index, TypeVector& types) override;
  Result EndExceptionSection() override { return Result::Ok; }

  Result OnInitExprF32ConstExpr(Index index, uint32_t value) override;
  Result OnInitExprF64ConstExpr(Index index, uint64_t value) override;
  Result OnInitExprGetGlobalExpr(Index index, Index global_index) override;
  Result OnInitExprI32ConstExpr(Index index, uint32_t value) override;
  Result OnInitExprI64ConstExpr(Index index, uint64_t value) override;

 private:
  bool HandleError(Offset offset, const char* message);
  Location GetLocation() const;
  void PrintError(const char* format, ...);
  void PushLabel(LabelType label_type,
                 ExprList* first,
                 Expr* context = nullptr);
  Result PopLabel();
  Result GetLabelAt(LabelNode** label, Index depth);
  Result TopLabel(LabelNode** label);
  Result AppendExpr(std::unique_ptr<Expr> expr);
  Result AppendCatch(Catch&& catch_);

  ErrorHandler* error_handler = nullptr;
  Module* module = nullptr;

  Func* current_func = nullptr;
  std::vector<LabelNode> label_stack;
  ExprList* current_init_expr = nullptr;
  const char* filename_;
};

BinaryReaderIR::BinaryReaderIR(Module* out_module,
                               const char* filename,
                               ErrorHandler* error_handler)
    : error_handler(error_handler), module(out_module), filename_(filename) {}

Location BinaryReaderIR::GetLocation() const {
  Location loc;
  loc.filename = filename_;
  loc.offset = state->offset;
  return loc;
}

void WABT_PRINTF_FORMAT(2, 3) BinaryReaderIR::PrintError(const char* format,
                                                          ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  HandleError(kInvalidOffset, buffer);
}

void BinaryReaderIR::PushLabel(LabelType label_type,
                               ExprList* first,
                               Expr* context) {
  label_stack.emplace_back(label_type, first, context);
}

Result BinaryReaderIR::PopLabel() {
  if (label_stack.size() == 0) {
    PrintError("popping empty label stack");
    return Result::Error;
  }

  label_stack.pop_back();
  return Result::Ok;
}

Result BinaryReaderIR::GetLabelAt(LabelNode** label, Index depth) {
  if (depth >= label_stack.size()) {
    PrintError("accessing stack depth: %" PRIindex " >= max: %" PRIzd, depth,
               label_stack.size());
    return Result::Error;
  }

  *label = &label_stack[label_stack.size() - depth - 1];
  return Result::Ok;
}

Result BinaryReaderIR::TopLabel(LabelNode** label) {
  return GetLabelAt(label, 0);
}

Result BinaryReaderIR::AppendExpr(std::unique_ptr<Expr> expr) {
  LabelNode* label;
  if (Failed(TopLabel(&label))) {
    return Result::Error;
  }
  label->exprs->push_back(expr.release());
  return Result::Ok;
}

bool BinaryReaderIR::HandleError(Offset offset, const char* message) {
  return error_handler->OnError(offset, message);
}

bool BinaryReaderIR::OnError(const char* message) {
  return HandleError(state->offset, message);
}

Result BinaryReaderIR::OnTypeCount(Index count) {
  module->func_types.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::OnType(Index index,
                               Index param_count,
                               Type* param_types,
                               Index result_count,
                               Type* result_types) {
  auto field = make_unique<FuncTypeModuleField>(GetLocation());
  auto&& func_type = field->func_type;
  func_type.sig.param_types.assign(param_types, param_types + param_count);
  func_type.sig.result_types.assign(result_types, result_types + result_count);
  module->func_types.push_back(&func_type);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnImportCount(Index count) {
  module->imports.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::OnImportFunc(Index import_index,
                                    string_view module_name,
                                    string_view field_name,
                                    Index func_index,
                                    Index sig_index) {
  auto import = make_unique<FuncImport>();
  import->module_name = module_name.to_string();
  import->field_name = field_name.to_string();
  import->func.decl.has_func_type = true;
  import->func.decl.type_var = Var(sig_index, GetLocation());
  import->func.decl.sig = module->func_types[sig_index]->sig;
  module->funcs.push_back(&import->func);
  module->imports.push_back(import.get());

  auto field = make_unique<ImportModuleField>(std::move(import), GetLocation());
  module->fields.push_back(field.release());
  module->num_func_imports++;
  return Result::Ok;
}

Result BinaryReaderIR::OnImportTable(Index import_index,
                                     string_view module_name,
                                     string_view field_name,
                                     Index table_index,
                                     Type elem_type,
                                     const Limits* elem_limits) {
  auto import = make_unique<TableImport>();
  import->module_name = module_name.to_string();
  import->field_name = field_name.to_string();
  import->table.elem_limits = *elem_limits;
  module->tables.push_back(&import->table);
  module->imports.push_back(import.get());

  auto field = make_unique<ImportModuleField>(std::move(import), GetLocation());
  module->fields.push_back(field.release());
  module->num_table_imports++;
  return Result::Ok;
}

Result BinaryReaderIR::OnImportMemory(Index import_index,
                                      string_view module_name,
                                      string_view field_name,
                                      Index memory_index,
                                      const Limits* page_limits) {
  auto import = make_unique<MemoryImport>();
  import->module_name = module_name.to_string();
  import->field_name = field_name.to_string();
  import->memory.page_limits = *page_limits;
  module->memories.push_back(&import->memory);
  module->imports.push_back(import.get());

  auto field = make_unique<ImportModuleField>(std::move(import), GetLocation());
  module->fields.push_back(field.release());
  module->num_memory_imports++;
  return Result::Ok;
}

Result BinaryReaderIR::OnImportGlobal(Index import_index,
                                      string_view module_name,
                                      string_view field_name,
                                      Index global_index,
                                      Type type,
                                      bool mutable_) {
  auto import = make_unique<GlobalImport>();
  import->module_name = module_name.to_string();
  import->field_name = field_name.to_string();
  import->global.type = type;
  import->global.mutable_ = mutable_;
  module->globals.push_back(&import->global);
  module->imports.push_back(import.get());

  auto field = make_unique<ImportModuleField>(std::move(import), GetLocation());
  module->fields.push_back(field.release());
  module->num_global_imports++;
  return Result::Ok;
}

Result BinaryReaderIR::OnImportException(Index import_index,
                                         string_view module_name,
                                         string_view field_name,
                                         Index except_index,
                                         TypeVector& sig) {
  auto import = make_unique<ExceptionImport>();
  import->module_name = module_name.to_string();
  import->field_name = field_name.to_string();
  import->except.sig = sig;
  module->excepts.push_back(&import->except);
  module->imports.push_back(import.get());

  auto field = make_unique<ImportModuleField>(std::move(import), GetLocation());
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnFunctionCount(Index count) {
  module->funcs.reserve(module->num_func_imports + count);
  return Result::Ok;
}

Result BinaryReaderIR::OnFunction(Index index, Index sig_index) {
  auto field = make_unique<FuncModuleField>(GetLocation());
  auto&& func = field->func;
  func.decl.has_func_type = true;
  func.decl.type_var = Var(sig_index, GetLocation());
  func.decl.sig = module->func_types[sig_index]->sig;

  module->funcs.push_back(&func);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnTableCount(Index count) {
  module->tables.reserve(module->num_table_imports + count);
  return Result::Ok;
}

Result BinaryReaderIR::OnTable(Index index,
                               Type elem_type,
                               const Limits* elem_limits) {
  auto field = make_unique<TableModuleField>(GetLocation());
  auto&& table = field->table;
  table.elem_limits = *elem_limits;
  module->tables.push_back(&table);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnMemoryCount(Index count) {
  module->memories.reserve(module->num_memory_imports + count);
  return Result::Ok;
}

Result BinaryReaderIR::OnMemory(Index index, const Limits* page_limits) {
  auto field = make_unique<MemoryModuleField>(GetLocation());
  auto&& memory = field->memory;
  memory.page_limits = *page_limits;
  module->memories.push_back(&memory);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnGlobalCount(Index count) {
  module->globals.reserve(module->num_global_imports + count);
  return Result::Ok;
}

Result BinaryReaderIR::BeginGlobal(Index index, Type type, bool mutable_) {
  auto field = make_unique<GlobalModuleField>(GetLocation());
  auto&& global = field->global;
  global.type = type;
  global.mutable_ = mutable_;
  module->globals.push_back(&global);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::BeginGlobalInitExpr(Index index) {
  assert(index == module->globals.size() - 1);
  Global* global = module->globals[index];
  current_init_expr = &global->init_expr;
  return Result::Ok;
}

Result BinaryReaderIR::EndGlobalInitExpr(Index index) {
  current_init_expr = nullptr;
  return Result::Ok;
}

Result BinaryReaderIR::OnExportCount(Index count) {
  module->exports.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::OnExport(Index index,
                                ExternalKind kind,
                                Index item_index,
                                string_view name) {
  auto field = make_unique<ExportModuleField>(GetLocation());
  auto&& export_ = field->export_;
  export_.name = name.to_string();
  switch (kind) {
    case ExternalKind::Func:
      assert(item_index < module->funcs.size());
      break;
    case ExternalKind::Table:
      assert(item_index < module->tables.size());
      break;
    case ExternalKind::Memory:
      assert(item_index < module->memories.size());
      break;
    case ExternalKind::Global:
      assert(item_index < module->globals.size());
      break;
    case ExternalKind::Except:
      // Note: Can't check if index valid, exceptions section comes later.
      break;
  }
  export_.var = Var(item_index, GetLocation());
  export_.kind = kind;
  module->exports.push_back(&export_);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::OnStartFunction(Index func_index) {
  assert(func_index < module->funcs.size());
  Var start(func_index, GetLocation());
  module->fields.push_back(new StartModuleField(start, GetLocation()));
  return Result::Ok;
}

Result BinaryReaderIR::OnFunctionBodyCount(Index count) {
  assert(module->num_func_imports + count == module->funcs.size());
  return Result::Ok;
}

Result BinaryReaderIR::BeginFunctionBody(Index index) {
  current_func = module->funcs[index];
  PushLabel(LabelType::Func, &current_func->exprs);
  return Result::Ok;
}

Result BinaryReaderIR::OnLocalDecl(Index decl_index, Index count, Type type) {
  TypeVector& types = current_func->local_types;
  types.reserve(types.size() + count);
  for (size_t i = 0; i < count; ++i)
    types.push_back(type);
  return Result::Ok;
}

Result BinaryReaderIR::OnBinaryExpr(Opcode opcode) {
  return AppendExpr(make_unique<BinaryExpr>(opcode, GetLocation()));
}

Result BinaryReaderIR::OnBlockExpr(Index num_types, Type* sig_types) {
  auto expr = make_unique<BlockExpr>(GetLocation());
  expr->block.sig.assign(sig_types, sig_types + num_types);
  auto* expr_list = &expr->block.exprs;
  CHECK_RESULT(AppendExpr(std::move(expr)));
  PushLabel(LabelType::Block, expr_list);
  return Result::Ok;
}

Result BinaryReaderIR::OnBrExpr(Index depth) {
  return AppendExpr(make_unique<BrExpr>(Var(depth, GetLocation())));
}

Result BinaryReaderIR::OnBrIfExpr(Index depth) {
  return AppendExpr(make_unique<BrIfExpr>(Var(depth, GetLocation())));
}

Result BinaryReaderIR::OnBrTableExpr(Index num_targets,
                                     Index* target_depths,
                                     Index default_target_depth) {
  auto expr = make_unique<BrTableExpr>(GetLocation());
  expr->default_target = Var(default_target_depth);
  expr->targets.resize(num_targets);
  for (Index i = 0; i < num_targets; ++i) {
    expr->targets[i] = Var(target_depths[i]);
  }
  return AppendExpr(std::move(expr));
}

Result BinaryReaderIR::OnCallExpr(Index func_index) {
  assert(func_index < module->funcs.size());
  return AppendExpr(make_unique<CallExpr>(Var(func_index, GetLocation())));
}

Result BinaryReaderIR::OnCallIndirectExpr(Index sig_index) {
  assert(sig_index < module->func_types.size());
  return AppendExpr(
      make_unique<CallIndirectExpr>(Var(sig_index, GetLocation())));
}

Result BinaryReaderIR::OnCompareExpr(Opcode opcode) {
  return AppendExpr(make_unique<CompareExpr>(opcode, GetLocation()));
}

Result BinaryReaderIR::OnConvertExpr(Opcode opcode) {
  return AppendExpr(make_unique<ConvertExpr>(opcode, GetLocation()));
}

Result BinaryReaderIR::OnCurrentMemoryExpr() {
  return AppendExpr(make_unique<CurrentMemoryExpr>(GetLocation()));
}

Result BinaryReaderIR::OnDropExpr() {
  return AppendExpr(make_unique<DropExpr>(GetLocation()));
}

Result BinaryReaderIR::OnElseExpr() {
  LabelNode* label;
  CHECK_RESULT(TopLabel(&label));
  if (label->label_type != LabelType::If) {
    PrintError("else expression without matching if");
    return Result::Error;
  }

  LabelNode* parent_label;
  CHECK_RESULT(GetLabelAt(&parent_label, 1));

  label->label_type = LabelType::Else;
  label->exprs = &cast<IfExpr>(&parent_label->exprs->back())->false_;
  return Result::Ok;
}

Result BinaryReaderIR::OnEndExpr() {
  return PopLabel();
}

Result BinaryReaderIR::OnF32ConstExpr(uint32_t value_bits) {
  return AppendExpr(
      make_unique<ConstExpr>(Const::F32(value_bits, GetLocation())));
}

Result BinaryReaderIR::OnF64ConstExpr(uint64_t value_bits) {
  return AppendExpr(
      make_unique<ConstExpr>(Const::F64(value_bits, GetLocation())));
}

Result BinaryReaderIR::OnGetGlobalExpr(Index global_index) {
  return AppendExpr(
      make_unique<GetGlobalExpr>(Var(global_index, GetLocation())));
}

Result BinaryReaderIR::OnGetLocalExpr(Index local_index) {
  return AppendExpr(make_unique<GetLocalExpr>(Var(local_index, GetLocation())));
}

Result BinaryReaderIR::OnGrowMemoryExpr() {
  return AppendExpr(make_unique<GrowMemoryExpr>());
}

Result BinaryReaderIR::OnI32ConstExpr(uint32_t value) {
  return AppendExpr(make_unique<ConstExpr>(Const::I32(value, GetLocation())));
}

Result BinaryReaderIR::OnI64ConstExpr(uint64_t value) {
  return AppendExpr(make_unique<ConstExpr>(Const::I64(value, GetLocation())));
}

Result BinaryReaderIR::OnIfExpr(Index num_types, Type* sig_types) {
  auto expr = make_unique<IfExpr>(GetLocation());
  expr->true_.sig.assign(sig_types, sig_types + num_types);
  auto* expr_list = &expr->true_.exprs;
  CHECK_RESULT(AppendExpr(std::move(expr)));
  PushLabel(LabelType::If, expr_list);
  return Result::Ok;
}

Result BinaryReaderIR::OnLoadExpr(Opcode opcode,
                                  uint32_t alignment_log2,
                                  Address offset) {
  return AppendExpr(make_unique<LoadExpr>(opcode, 1 << alignment_log2, offset));
}

Result BinaryReaderIR::OnLoopExpr(Index num_types, Type* sig_types) {
  auto expr = make_unique<LoopExpr>();
  expr->block.sig.assign(sig_types, sig_types + num_types);
  auto* expr_list = &expr->block.exprs;
  CHECK_RESULT(AppendExpr(std::move(expr)));
  PushLabel(LabelType::Loop, expr_list);
  return Result::Ok;
}

Result BinaryReaderIR::OnNopExpr() {
  return AppendExpr(make_unique<NopExpr>(GetLocation()));
}

Result BinaryReaderIR::OnRethrowExpr(Index depth) {
  return AppendExpr(make_unique<RethrowExpr>(Var(depth, GetLocation())));
}

Result BinaryReaderIR::OnReturnExpr() {
  return AppendExpr(make_unique<ReturnExpr>());
}

Result BinaryReaderIR::OnSelectExpr() {
  return AppendExpr(make_unique<SelectExpr>());
}

Result BinaryReaderIR::OnSetGlobalExpr(Index global_index) {
  return AppendExpr(
      make_unique<SetGlobalExpr>(Var(global_index, GetLocation())));
}

Result BinaryReaderIR::OnSetLocalExpr(Index local_index) {
  return AppendExpr(make_unique<SetLocalExpr>(Var(local_index, GetLocation())));
}

Result BinaryReaderIR::OnStoreExpr(Opcode opcode,
                                   uint32_t alignment_log2,
                                   Address offset) {
  return AppendExpr(
      make_unique<StoreExpr>(opcode, 1 << alignment_log2, offset));
}

Result BinaryReaderIR::OnThrowExpr(Index except_index) {
  return AppendExpr(make_unique<ThrowExpr>(Var(except_index, GetLocation())));
}

Result BinaryReaderIR::OnTeeLocalExpr(Index local_index) {
  return AppendExpr(make_unique<TeeLocalExpr>(Var(local_index, GetLocation())));
}

Result BinaryReaderIR::OnTryExpr(Index num_types, Type* sig_types) {
  auto expr_ptr = make_unique<TryExpr>();
  // Save expr so it can be used below, after expr_ptr has been moved.
  auto* expr = expr_ptr.get();
  auto* expr_list = &expr->block.exprs;
  expr->block.sig.assign(sig_types, sig_types + num_types);
  CHECK_RESULT(AppendExpr(std::move(expr_ptr)));
  PushLabel(LabelType::Try, expr_list, expr);
  return Result::Ok;
}

Result BinaryReaderIR::AppendCatch(Catch&& catch_) {
  LabelNode* label = nullptr;
  CHECK_RESULT(TopLabel(&label));

  if (label->label_type != LabelType::Try) {
    PrintError("catch not inside try block");
    return Result::Error;
  }

  auto try_ = cast<TryExpr>(label->context);
  try_->catches.push_back(std::move(catch_));
  label->exprs = &try_->catches.back().exprs;
  return Result::Ok;
}

Result BinaryReaderIR::OnCatchExpr(Index except_index) {
  return AppendCatch(Catch(Var(except_index, GetLocation())));
}

Result BinaryReaderIR::OnCatchAllExpr() {
  return AppendCatch(Catch(GetLocation()));
}

Result BinaryReaderIR::OnUnaryExpr(Opcode opcode) {
  return AppendExpr(make_unique<UnaryExpr>(opcode));
}

Result BinaryReaderIR::OnUnreachableExpr() {
  return AppendExpr(make_unique<UnreachableExpr>());
}

Result BinaryReaderIR::EndFunctionBody(Index index) {
  CHECK_RESULT(PopLabel());
  current_func = nullptr;
  return Result::Ok;
}

Result BinaryReaderIR::OnElemSegmentCount(Index count) {
  module->elem_segments.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::BeginElemSegment(Index index, Index table_index) {
  auto field = make_unique<ElemSegmentModuleField>(GetLocation());
  auto&& elem_segment = field->elem_segment;
  elem_segment.table_var = Var(table_index, GetLocation());
  module->elem_segments.push_back(&elem_segment);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::BeginElemSegmentInitExpr(Index index) {
  assert(index == module->elem_segments.size() - 1);
  ElemSegment* segment = module->elem_segments[index];
  current_init_expr = &segment->offset;
  return Result::Ok;
}

Result BinaryReaderIR::EndElemSegmentInitExpr(Index index) {
  current_init_expr = nullptr;
  return Result::Ok;
}

Result BinaryReaderIR::OnElemSegmentFunctionIndexCount(Index index,
                                                       Index count) {
  assert(index == module->elem_segments.size() - 1);
  ElemSegment* segment = module->elem_segments[index];
  segment->vars.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::OnElemSegmentFunctionIndex(Index segment_index,
                                                  Index func_index) {
  assert(segment_index == module->elem_segments.size() - 1);
  ElemSegment* segment = module->elem_segments[segment_index];
  segment->vars.emplace_back();
  Var* var = &segment->vars.back();
  *var = Var(func_index, GetLocation());
  return Result::Ok;
}

Result BinaryReaderIR::OnDataSegmentCount(Index count) {
  module->data_segments.reserve(count);
  return Result::Ok;
}

Result BinaryReaderIR::BeginDataSegment(Index index, Index memory_index) {
  auto field = make_unique<DataSegmentModuleField>(GetLocation());
  auto&& data_segment = field->data_segment;
  data_segment.memory_var = Var(memory_index, GetLocation());
  module->data_segments.push_back(&data_segment);
  module->fields.push_back(field.release());
  return Result::Ok;
}

Result BinaryReaderIR::BeginDataSegmentInitExpr(Index index) {
  assert(index == module->data_segments.size() - 1);
  DataSegment* segment = module->data_segments[index];
  current_init_expr = &segment->offset;
  return Result::Ok;
}

Result BinaryReaderIR::EndDataSegmentInitExpr(Index index) {
  current_init_expr = nullptr;
  return Result::Ok;
}

Result BinaryReaderIR::OnDataSegmentData(Index index,
                                         const void* data,
                                         Address size) {
  assert(index == module->data_segments.size() - 1);
  DataSegment* segment = module->data_segments[index];
  segment->data.resize(size);
  if (size > 0)
    memcpy(segment->data.data(), data, size);
  return Result::Ok;
}

Result BinaryReaderIR::OnFunctionNamesCount(Index count) {
  if (count > module->funcs.size()) {
    PrintError("expected function name count (%" PRIindex
               ") <= function count (%" PRIzd ")",
               count, module->funcs.size());
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderIR::OnFunctionName(Index index, string_view name) {
  if (name.empty())
    return Result::Ok;

  Func* func = module->funcs[index];
  std::string dollar_name = std::string("$") + name.to_string();
  int counter = 1;
  std::string orig_name = dollar_name;
  while (module->func_bindings.count(dollar_name) != 0) {
    dollar_name = orig_name + "." + std::to_string(counter++);
  }
  func->name = dollar_name;
  module->func_bindings.emplace(dollar_name, Binding(index));
  return Result::Ok;
}

Result BinaryReaderIR::OnLocalNameLocalCount(Index index, Index count) {
  assert(index < module->funcs.size());
  Func* func = module->funcs[index];
  Index num_params_and_locals = func->GetNumParamsAndLocals();
  if (count > num_params_and_locals) {
    PrintError("expected local name count (%" PRIindex
               ") <= local count (%" PRIindex ")",
               count, num_params_and_locals);
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderIR::OnInitExprF32ConstExpr(Index index, uint32_t value) {
  auto expr = new ConstExpr(Const::F32(value, GetLocation()));
  expr->loc = GetLocation();
  current_init_expr->push_back(expr);
  return Result::Ok;
}

Result BinaryReaderIR::OnInitExprF64ConstExpr(Index index, uint64_t value) {
  auto expr = new ConstExpr(Const::F64(value, GetLocation()));
  expr->loc = GetLocation();
  current_init_expr->push_back(expr);
  return Result::Ok;
}

Result BinaryReaderIR::OnInitExprGetGlobalExpr(Index index,
                                               Index global_index) {
  auto expr = new GetGlobalExpr(Var(global_index, GetLocation()));
  expr->loc = GetLocation();
  current_init_expr->push_back(expr);
  return Result::Ok;
}

Result BinaryReaderIR::OnInitExprI32ConstExpr(Index index, uint32_t value) {
  auto expr = new ConstExpr(Const::I32(value, GetLocation()));
  expr->loc = GetLocation();
  current_init_expr->push_back(expr);
  return Result::Ok;
}

Result BinaryReaderIR::OnInitExprI64ConstExpr(Index index, uint64_t value) {
  auto expr = new ConstExpr(Const::I64(value, GetLocation()));
  expr->loc = GetLocation();
  current_init_expr->push_back(expr);
  return Result::Ok;
}

Result BinaryReaderIR::OnLocalName(Index func_index,
                                   Index local_index,
                                   string_view name) {
  if (name.empty())
    return Result::Ok;

  Func* func = module->funcs[func_index];
  Index num_params = func->GetNumParams();
  BindingHash* bindings;
  Index index;
  if (local_index < num_params) {
    /* param name */
    bindings = &func->param_bindings;
    index = local_index;
  } else {
    /* local name */
    bindings = &func->local_bindings;
    index = local_index - num_params;
  }
  bindings->emplace(std::string("$") + name.to_string(), Binding(index));
  return Result::Ok;
}

Result BinaryReaderIR::OnExceptionType(Index index, TypeVector& sig) {
  auto field = make_unique<ExceptionModuleField>(GetLocation());
  auto&& except = field->except;
  except.sig = sig;
  module->excepts.push_back(&except);
  module->fields.push_back(field.release());
  return Result::Ok;
}

}  // end anonymous namespace

Result ReadBinaryIr(const char* filename,
                    const void* data,
                    size_t size,
                    const ReadBinaryOptions* options,
                    ErrorHandler* error_handler,
                    struct Module* out_module) {
  BinaryReaderIR reader(out_module, filename, error_handler);
  Result result = ReadBinary(data, size, &reader, options);
  return result;
}

}  // namespace wabt
