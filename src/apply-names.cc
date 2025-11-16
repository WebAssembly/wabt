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

#include "wabt/apply-names.h"

#include <cassert>
#include <cstdio>
#include <string_view>
#include <vector>

#include "wabt/cast.h"
#include "wabt/expr-visitor.h"
#include "wabt/ir.h"

namespace wabt {

namespace {

class NameApplier : public ExprVisitor::DelegateNop {
 public:
  NameApplier();

  Result VisitModule(Module* module);

  // Implementation of ExprVisitor::DelegateNop.
  Result BeginBlockExpr(BlockExpr*) override;
  Result EndBlockExpr(BlockExpr*) override;
  Result OnBrExpr(BrExpr*) override;
  Result OnBrIfExpr(BrIfExpr*) override;
  Result OnBrOnNonNullExpr(BrOnNonNullExpr*) override;
  Result OnBrOnNullExpr(BrOnNullExpr*) override;
  Result OnBrTableExpr(BrTableExpr*) override;
  Result OnCallExpr(CallExpr*) override;
  Result OnRefFuncExpr(RefFuncExpr*) override;
  Result OnCallIndirectExpr(CallIndirectExpr*) override;
  Result OnReturnCallExpr(ReturnCallExpr*) override;
  Result OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) override;
  Result OnGlobalGetExpr(GlobalGetExpr*) override;
  Result OnGlobalSetExpr(GlobalSetExpr*) override;
  Result BeginIfExpr(IfExpr*) override;
  Result EndIfExpr(IfExpr*) override;
  Result OnLoadExpr(LoadExpr*) override;
  Result OnLocalGetExpr(LocalGetExpr*) override;
  Result OnLocalSetExpr(LocalSetExpr*) override;
  Result OnLocalTeeExpr(LocalTeeExpr*) override;
  Result BeginLoopExpr(LoopExpr*) override;
  Result EndLoopExpr(LoopExpr*) override;
  Result OnMemoryCopyExpr(MemoryCopyExpr*) override;
  Result OnDataDropExpr(DataDropExpr*) override;
  Result OnMemoryFillExpr(MemoryFillExpr*) override;
  Result OnMemoryGrowExpr(MemoryGrowExpr*) override;
  Result OnMemoryInitExpr(MemoryInitExpr*) override;
  Result OnMemorySizeExpr(MemorySizeExpr*) override;
  Result OnElemDropExpr(ElemDropExpr*) override;
  Result OnTableCopyExpr(TableCopyExpr*) override;
  Result OnTableInitExpr(TableInitExpr*) override;
  Result OnTableGetExpr(TableGetExpr*) override;
  Result OnTableSetExpr(TableSetExpr*) override;
  Result OnTableGrowExpr(TableGrowExpr*) override;
  Result OnTableSizeExpr(TableSizeExpr*) override;
  Result OnTableFillExpr(TableFillExpr*) override;
  Result OnStoreExpr(StoreExpr*) override;
  Result BeginTryExpr(TryExpr*) override;
  Result EndTryExpr(TryExpr*) override;
  Result BeginTryTableExpr(TryTableExpr*) override;
  Result EndTryTableExpr(TryTableExpr*) override;
  Result OnCatchExpr(TryExpr*, Catch*) override;
  Result OnDelegateExpr(TryExpr*) override;
  Result OnThrowExpr(ThrowExpr*) override;
  Result OnRethrowExpr(RethrowExpr*) override;
  Result OnSimdLoadLaneExpr(SimdLoadLaneExpr*) override;
  Result OnSimdStoreLaneExpr(SimdStoreLaneExpr*) override;

 private:
  void PushLabel(const std::string& label);
  void PopLabel();
  std::string_view FindLabelByVar(Var* var);
  void UseNameForVar(std::string_view name, Var* var);
  Result UseNameForFuncTypeVar(Var* var);
  Result UseNameForFuncVar(Var* var);
  Result UseNameForGlobalVar(Var* var);
  Result UseNameForTableVar(Var* var);
  Result UseNameForMemoryVar(Var* var);
  Result UseNameForTagVar(Var* var);
  Result UseNameForDataSegmentVar(Var* var);
  Result UseNameForElemSegmentVar(Var* var);
  Result UseNameForParamAndLocalVar(Func* func, Var* var);
  Result UseNameForLabelVar(Var* var);
  Result VisitFunc(Index func_index, Func* func);
  Result VisitGlobal(Global* global);
  Result VisitTag(Tag* tag);
  Result VisitExport(Index export_index, Export* export_);
  Result VisitTable(Table* table);
  Result VisitElemSegment(Index elem_segment_index, ElemSegment* segment);
  Result VisitDataSegment(Index data_segment_index, DataSegment* segment);
  Result VisitStart(Var* start_var);

  Module* module_ = nullptr;
  Func* current_func_ = nullptr;
  ExprVisitor visitor_;
  std::vector<std::string> param_and_local_index_to_name_;
  std::vector<std::string> labels_;
};

NameApplier::NameApplier() : visitor_(this) {}

void NameApplier::PushLabel(const std::string& label) {
  labels_.push_back(label);
}

void NameApplier::PopLabel() {
  labels_.pop_back();
}

std::string_view NameApplier::FindLabelByVar(Var* var) {
  if (var->is_name()) {
    for (int i = labels_.size() - 1; i >= 0; --i) {
      const std::string& label = labels_[i];
      if (label == var->name()) {
        return label;
      }
    }
    return std::string_view();
  } else {
    if (var->index() >= labels_.size()) {
      return std::string_view();
    }
    return labels_[labels_.size() - 1 - var->index()];
  }
}

void NameApplier::UseNameForVar(std::string_view name, Var* var) {
  if (var->is_name()) {
    assert(name == var->name());
    return;
  }

  if (!name.empty()) {
    var->set_name(name);
  }
}

Result NameApplier::UseNameForFuncTypeVar(Var* var) {
  FuncType* func_type = module_->GetFuncType(*var);
  if (!func_type) {
    return Result::Error;
  }
  UseNameForVar(func_type->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForFuncVar(Var* var) {
  Func* func = module_->GetFunc(*var);
  if (!func) {
    return Result::Error;
  }
  UseNameForVar(func->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForGlobalVar(Var* var) {
  Global* global = module_->GetGlobal(*var);
  if (!global) {
    return Result::Error;
  }
  UseNameForVar(global->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForTableVar(Var* var) {
  Table* table = module_->GetTable(*var);
  if (!table) {
    return Result::Error;
  }
  UseNameForVar(table->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForMemoryVar(Var* var) {
  Memory* memory = module_->GetMemory(*var);
  if (!memory) {
    return Result::Error;
  }
  UseNameForVar(memory->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForTagVar(Var* var) {
  Tag* tag = module_->GetTag(*var);
  if (!tag) {
    return Result::Error;
  }
  UseNameForVar(tag->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForDataSegmentVar(Var* var) {
  DataSegment* data_segment = module_->GetDataSegment(*var);
  if (!data_segment) {
    return Result::Error;
  }
  UseNameForVar(data_segment->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForElemSegmentVar(Var* var) {
  ElemSegment* elem_segment = module_->GetElemSegment(*var);
  if (!elem_segment) {
    return Result::Error;
  }
  UseNameForVar(elem_segment->name, var);
  return Result::Ok;
}

Result NameApplier::UseNameForParamAndLocalVar(Func* func, Var* var) {
  Index local_index = func->GetLocalIndex(*var);
  if (local_index >= func->GetNumParamsAndLocals()) {
    return Result::Error;
  }

  std::string name = param_and_local_index_to_name_[local_index];
  if (var->is_name()) {
    assert(name == var->name());
    return Result::Ok;
  }

  if (!name.empty()) {
    var->set_name(name);
  }
  return Result::Ok;
}

Result NameApplier::UseNameForLabelVar(Var* var) {
  std::string_view label = FindLabelByVar(var);
  UseNameForVar(label, var);
  return Result::Ok;
}

Result NameApplier::BeginBlockExpr(BlockExpr* expr) {
  PushLabel(expr->block.label);
  return Result::Ok;
}

Result NameApplier::EndBlockExpr(BlockExpr* expr) {
  PopLabel();
  return Result::Ok;
}

Result NameApplier::BeginLoopExpr(LoopExpr* expr) {
  PushLabel(expr->block.label);
  return Result::Ok;
}

Result NameApplier::EndLoopExpr(LoopExpr* expr) {
  PopLabel();
  return Result::Ok;
}

Result NameApplier::OnDataDropExpr(DataDropExpr* expr) {
  return UseNameForDataSegmentVar(&expr->var);
}

Result NameApplier::OnMemoryCopyExpr(MemoryCopyExpr* expr) {
  CHECK_RESULT(UseNameForMemoryVar(&expr->destmemidx));
  return UseNameForMemoryVar(&expr->srcmemidx);
}

Result NameApplier::OnMemoryFillExpr(MemoryFillExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnMemoryGrowExpr(MemoryGrowExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnMemoryInitExpr(MemoryInitExpr* expr) {
  CHECK_RESULT(UseNameForDataSegmentVar(&expr->var));
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnMemorySizeExpr(MemorySizeExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnElemDropExpr(ElemDropExpr* expr) {
  return UseNameForElemSegmentVar(&expr->var);
}

Result NameApplier::OnTableCopyExpr(TableCopyExpr* expr) {
  CHECK_RESULT(UseNameForTableVar(&expr->dst_table));
  return UseNameForTableVar(&expr->src_table);
}

Result NameApplier::OnTableInitExpr(TableInitExpr* expr) {
  CHECK_RESULT(UseNameForElemSegmentVar(&expr->segment_index));
  return UseNameForTableVar(&expr->table_index);
}

Result NameApplier::OnTableGetExpr(TableGetExpr* expr) {
  return UseNameForTableVar(&expr->var);
}

Result NameApplier::OnTableSetExpr(TableSetExpr* expr) {
  return UseNameForTableVar(&expr->var);
}

Result NameApplier::OnTableGrowExpr(TableGrowExpr* expr) {
  return UseNameForTableVar(&expr->var);
}

Result NameApplier::OnTableSizeExpr(TableSizeExpr* expr) {
  return UseNameForTableVar(&expr->var);
}

Result NameApplier::OnTableFillExpr(TableFillExpr* expr) {
  return UseNameForTableVar(&expr->var);
}

Result NameApplier::OnStoreExpr(StoreExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnBrExpr(BrExpr* expr) {
  return UseNameForLabelVar(&expr->var);
}

Result NameApplier::OnBrIfExpr(BrIfExpr* expr) {
  return UseNameForLabelVar(&expr->var);
}

Result NameApplier::OnBrOnNonNullExpr(BrOnNonNullExpr* expr) {
  return UseNameForLabelVar(&expr->var);
}

Result NameApplier::OnBrOnNullExpr(BrOnNullExpr* expr) {
  return UseNameForLabelVar(&expr->var);
}

Result NameApplier::OnBrTableExpr(BrTableExpr* expr) {
  for (Var& target : expr->targets) {
    UseNameForLabelVar(&target);
  }

  return UseNameForLabelVar(&expr->default_target);
}

Result NameApplier::BeginTryExpr(TryExpr* expr) {
  PushLabel(expr->block.label);
  return Result::Ok;
}

Result NameApplier::EndTryExpr(TryExpr*) {
  PopLabel();
  return Result::Ok;
}

Result NameApplier::BeginTryTableExpr(TryTableExpr* expr) {
  for (TableCatch& catch_ : expr->catches) {
    if (!catch_.IsCatchAll()) {
      CHECK_RESULT(UseNameForTagVar(&catch_.tag));
    }
    UseNameForLabelVar(&catch_.target);
  }
  PushLabel(expr->block.label);
  return Result::Ok;
}

Result NameApplier::EndTryTableExpr(TryTableExpr*) {
  PopLabel();
  return Result::Ok;
}

Result NameApplier::OnCatchExpr(TryExpr*, Catch* expr) {
  if (!expr->IsCatchAll()) {
    CHECK_RESULT(UseNameForTagVar(&expr->var));
  }
  return Result::Ok;
}

Result NameApplier::OnDelegateExpr(TryExpr* expr) {
  PopLabel();
  return UseNameForLabelVar(&expr->delegate_target);
}

Result NameApplier::OnThrowExpr(ThrowExpr* expr) {
  return UseNameForTagVar(&expr->var);
}

Result NameApplier::OnRethrowExpr(RethrowExpr* expr) {
  return UseNameForLabelVar(&expr->var);
}

Result NameApplier::OnCallExpr(CallExpr* expr) {
  return UseNameForFuncVar(&expr->var);
}

Result NameApplier::OnRefFuncExpr(RefFuncExpr* expr) {
  return UseNameForFuncVar(&expr->var);
}

Result NameApplier::OnCallIndirectExpr(CallIndirectExpr* expr) {
  if (expr->decl.has_func_type) {
    CHECK_RESULT(UseNameForFuncTypeVar(&expr->decl.type_var));
  }
  return UseNameForTableVar(&expr->table);
}

Result NameApplier::OnReturnCallExpr(ReturnCallExpr* expr) {
  return UseNameForFuncVar(&expr->var);
}

Result NameApplier::OnReturnCallIndirectExpr(ReturnCallIndirectExpr* expr) {
  if (expr->decl.has_func_type) {
    CHECK_RESULT(UseNameForFuncTypeVar(&expr->decl.type_var));
  }
  return UseNameForTableVar(&expr->table);
}

Result NameApplier::OnGlobalGetExpr(GlobalGetExpr* expr) {
  return UseNameForGlobalVar(&expr->var);
}

Result NameApplier::OnLocalGetExpr(LocalGetExpr* expr) {
  return UseNameForParamAndLocalVar(current_func_, &expr->var);
}

Result NameApplier::BeginIfExpr(IfExpr* expr) {
  PushLabel(expr->true_.label);
  return Result::Ok;
}

Result NameApplier::EndIfExpr(IfExpr* expr) {
  PopLabel();
  return Result::Ok;
}

Result NameApplier::OnLoadExpr(LoadExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnGlobalSetExpr(GlobalSetExpr* expr) {
  return UseNameForGlobalVar(&expr->var);
}

Result NameApplier::OnLocalSetExpr(LocalSetExpr* expr) {
  return UseNameForParamAndLocalVar(current_func_, &expr->var);
}

Result NameApplier::OnLocalTeeExpr(LocalTeeExpr* expr) {
  return UseNameForParamAndLocalVar(current_func_, &expr->var);
}

Result NameApplier::OnSimdLoadLaneExpr(SimdLoadLaneExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::OnSimdStoreLaneExpr(SimdStoreLaneExpr* expr) {
  return UseNameForMemoryVar(&expr->memidx);
}

Result NameApplier::VisitFunc(Index func_index, Func* func) {
  current_func_ = func;
  if (func->decl.has_func_type) {
    CHECK_RESULT(UseNameForFuncTypeVar(&func->decl.type_var));
  }

  MakeTypeBindingReverseMapping(func->GetNumParamsAndLocals(), func->bindings,
                                &param_and_local_index_to_name_);

  CHECK_RESULT(visitor_.VisitFunc(func));
  current_func_ = nullptr;
  return Result::Ok;
}

Result NameApplier::VisitGlobal(Global* global) {
  return visitor_.VisitExprList(global->init_expr);
}

Result NameApplier::VisitTag(Tag* tag) {
  if (tag->decl.has_func_type) {
    CHECK_RESULT(UseNameForFuncTypeVar(&tag->decl.type_var));
  }
  return Result::Ok;
}

Result NameApplier::VisitExport(Index export_index, Export* export_) {
  switch (export_->kind) {
    case ExternalKind::Func:
      UseNameForFuncVar(&export_->var);
      break;

    case ExternalKind::Table:
      UseNameForTableVar(&export_->var);
      break;

    case ExternalKind::Memory:
      UseNameForMemoryVar(&export_->var);
      break;

    case ExternalKind::Global:
      UseNameForGlobalVar(&export_->var);
      break;

    case ExternalKind::Tag:
      UseNameForTagVar(&export_->var);
      break;
  }
  return Result::Ok;
}

Result NameApplier::VisitTable(Table* table) {
  return visitor_.VisitExprList(table->init_expr);
}

Result NameApplier::VisitElemSegment(Index elem_segment_index,
                                     ElemSegment* segment) {
  CHECK_RESULT(UseNameForTableVar(&segment->table_var));
  CHECK_RESULT(visitor_.VisitExprList(segment->offset));
  for (ExprList& elem_expr : segment->elem_exprs) {
    Expr* expr = &elem_expr.front();
    if (expr->type() == ExprType::RefFunc) {
      CHECK_RESULT(UseNameForFuncVar(&cast<RefFuncExpr>(expr)->var));
    }
  }
  return Result::Ok;
}

Result NameApplier::VisitDataSegment(Index data_segment_index,
                                     DataSegment* segment) {
  CHECK_RESULT(UseNameForMemoryVar(&segment->memory_var));
  return visitor_.VisitExprList(segment->offset);
}

Result NameApplier::VisitStart(Var* start_var) {
  return UseNameForFuncVar(start_var);
}

Result NameApplier::VisitModule(Module* module) {
  module_ = module;
  for (size_t i = 0; i < module->funcs.size(); ++i)
    CHECK_RESULT(VisitFunc(i, module->funcs[i]));
  for (size_t i = 0; i < module->globals.size(); ++i)
    CHECK_RESULT(VisitGlobal(module->globals[i]));
  for (size_t i = 0; i < module->tags.size(); ++i)
    CHECK_RESULT(VisitTag(module->tags[i]));
  for (size_t i = 0; i < module->exports.size(); ++i)
    CHECK_RESULT(VisitExport(i, module->exports[i]));
  for (size_t i = 0; i < module->tables.size(); ++i)
    CHECK_RESULT(VisitTable(module->tables[i]));
  for (size_t i = 0; i < module->elem_segments.size(); ++i)
    CHECK_RESULT(VisitElemSegment(i, module->elem_segments[i]));
  for (size_t i = 0; i < module->data_segments.size(); ++i)
    CHECK_RESULT(VisitDataSegment(i, module->data_segments[i]));
  for (size_t i = 0; i < module->starts.size(); ++i)
    CHECK_RESULT(VisitStart(module->starts[i]));
  module_ = nullptr;
  return Result::Ok;
}

}  // end anonymous namespace

Result ApplyNames(Module* module) {
  NameApplier applier;
  return applier.VisitModule(module);
}

}  // namespace wabt
