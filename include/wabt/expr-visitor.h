/*
 * Copyright 2017 WebAssembly Community Group participants
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

#ifndef WABT_EXPR_VISITOR_H_
#define WABT_EXPR_VISITOR_H_

#include "wabt/common.h"
#include "wabt/ir.h"

namespace wabt {

class ExprVisitor {
 public:
  class Delegate;
  class DelegateNop;

  explicit ExprVisitor(Delegate* delegate);

  Result VisitExpr(Expr*);
  Result VisitExprList(ExprList&);
  Result VisitFunc(Func*);

 private:
  enum class State {
    Default,
    Block,
    IfTrue,
    IfFalse,
    Loop,
    Try,
    TryTable,
    Catch,
    OpcodeRaw
  };

  Result HandleDefaultState(Expr*);
  void PushDefault(Expr*);
  void PopDefault();
  void PushExprlist(State state, Expr*, ExprList&);
  void PopExprlist();
  void PushCatch(Expr*, Index catch_index, ExprList&);
  void PopCatch();

  Delegate* delegate_;

  // Use parallel arrays instead of array of structs so we can avoid allocating
  // unneeded objects. ExprList::iterator has no default constructor, so it
  // must only be allocated for states that use it.
  std::vector<State> state_stack_;
  std::vector<Expr*> expr_stack_;
  std::vector<ExprList::iterator> expr_iter_stack_;
  std::vector<Index> catch_index_stack_;
};

class ExprVisitor::Delegate {
 public:
  virtual ~Delegate() {}

  virtual Result OnBinaryExpr(BinaryExpr*) = 0;
  virtual Result BeginBlockExpr(BlockExpr*) = 0;
  virtual Result EndBlockExpr(BlockExpr*) = 0;
  virtual Result OnBrExpr(BrExpr*) = 0;
  virtual Result OnBrIfExpr(BrIfExpr*) = 0;
  virtual Result OnBrTableExpr(BrTableExpr*) = 0;
  virtual Result BeginTryTableExpr(TryTableExpr*) = 0;
  virtual Result EndTryTableExpr(TryTableExpr*) = 0;
  virtual Result OnCallExpr(CallExpr*) = 0;
  virtual Result OnCallIndirectExpr(CallIndirectExpr*) = 0;
  virtual Result OnCallRefExpr(CallRefExpr*) = 0;
  virtual Result OnCodeMetadataExpr(CodeMetadataExpr*) = 0;
  virtual Result OnCompareExpr(CompareExpr*) = 0;
  virtual Result OnConstExpr(ConstExpr*) = 0;
  virtual Result OnConvertExpr(ConvertExpr*) = 0;
  virtual Result OnDropExpr(DropExpr*) = 0;
  virtual Result OnGlobalGetExpr(GlobalGetExpr*) = 0;
  virtual Result OnGlobalSetExpr(GlobalSetExpr*) = 0;
  virtual Result BeginIfExpr(IfExpr*) = 0;
  virtual Result AfterIfTrueExpr(IfExpr*) = 0;
  virtual Result EndIfExpr(IfExpr*) = 0;
  virtual Result OnLoadExpr(LoadExpr*) = 0;
  virtual Result OnLocalGetExpr(LocalGetExpr*) = 0;
  virtual Result OnLocalSetExpr(LocalSetExpr*) = 0;
  virtual Result OnLocalTeeExpr(LocalTeeExpr*) = 0;
  virtual Result BeginLoopExpr(LoopExpr*) = 0;
  virtual Result EndLoopExpr(LoopExpr*) = 0;
  virtual Result OnMemoryCopyExpr(MemoryCopyExpr*) = 0;
  virtual Result OnDataDropExpr(DataDropExpr*) = 0;
  virtual Result OnMemoryFillExpr(MemoryFillExpr*) = 0;
  virtual Result OnMemoryGrowExpr(MemoryGrowExpr*) = 0;
  virtual Result OnMemoryInitExpr(MemoryInitExpr*) = 0;
  virtual Result OnMemorySizeExpr(MemorySizeExpr*) = 0;
  virtual Result OnTableCopyExpr(TableCopyExpr*) = 0;
  virtual Result OnElemDropExpr(ElemDropExpr*) = 0;
  virtual Result OnTableInitExpr(TableInitExpr*) = 0;
  virtual Result OnTableGetExpr(TableGetExpr*) = 0;
  virtual Result OnTableSetExpr(TableSetExpr*) = 0;
  virtual Result OnTableGrowExpr(TableGrowExpr*) = 0;
  virtual Result OnTableSizeExpr(TableSizeExpr*) = 0;
  virtual Result OnTableFillExpr(TableFillExpr*) = 0;
  virtual Result OnRefFuncExpr(RefFuncExpr*) = 0;
  virtual Result OnRefNullExpr(RefNullExpr*) = 0;
  virtual Result OnRefIsNullExpr(RefIsNullExpr*) = 0;
  virtual Result OnNopExpr(NopExpr*) = 0;
  virtual Result OnReturnExpr(ReturnExpr*) = 0;
  virtual Result OnReturnCallExpr(ReturnCallExpr*) = 0;
  virtual Result OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) = 0;
  virtual Result OnSelectExpr(SelectExpr*) = 0;
  virtual Result OnStoreExpr(StoreExpr*) = 0;
  virtual Result OnUnaryExpr(UnaryExpr*) = 0;
  virtual Result OnUnreachableExpr(UnreachableExpr*) = 0;
  virtual Result BeginTryExpr(TryExpr*) = 0;
  virtual Result OnCatchExpr(TryExpr*, Catch*) = 0;
  virtual Result OnDelegateExpr(TryExpr*) = 0;
  virtual Result EndTryExpr(TryExpr*) = 0;
  virtual Result OnThrowExpr(ThrowExpr*) = 0;
  virtual Result OnThrowRefExpr(ThrowRefExpr*) = 0;
  virtual Result OnRethrowExpr(RethrowExpr*) = 0;
  virtual Result OnAtomicWaitExpr(AtomicWaitExpr*) = 0;
  virtual Result OnAtomicFenceExpr(AtomicFenceExpr*) = 0;
  virtual Result OnAtomicNotifyExpr(AtomicNotifyExpr*) = 0;
  virtual Result OnAtomicLoadExpr(AtomicLoadExpr*) = 0;
  virtual Result OnAtomicStoreExpr(AtomicStoreExpr*) = 0;
  virtual Result OnAtomicRmwExpr(AtomicRmwExpr*) = 0;
  virtual Result OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) = 0;
  virtual Result OnTernaryExpr(TernaryExpr*) = 0;
  virtual Result OnSimdLaneOpExpr(SimdLaneOpExpr*) = 0;
  virtual Result OnSimdLoadLaneExpr(SimdLoadLaneExpr*) = 0;
  virtual Result OnSimdStoreLaneExpr(SimdStoreLaneExpr*) = 0;
  virtual Result OnSimdShuffleOpExpr(SimdShuffleOpExpr*) = 0;
  virtual Result OnLoadSplatExpr(LoadSplatExpr*) = 0;
  virtual Result OnLoadZeroExpr(LoadZeroExpr*) = 0;
  virtual Result OnOpcodeRawExpr(OpcodeRawExpr*) = 0;
};

class ExprVisitor::DelegateNop : public ExprVisitor::Delegate {
 public:
  Result OnBinaryExpr(BinaryExpr*) override { return Result::Ok; }
  Result BeginBlockExpr(BlockExpr*) override { return Result::Ok; }
  Result EndBlockExpr(BlockExpr*) override { return Result::Ok; }
  Result OnBrExpr(BrExpr*) override { return Result::Ok; }
  Result OnBrIfExpr(BrIfExpr*) override { return Result::Ok; }
  Result OnBrTableExpr(BrTableExpr*) override { return Result::Ok; }
  Result BeginTryTableExpr(TryTableExpr*) override { return Result::Ok; }
  Result EndTryTableExpr(TryTableExpr*) override { return Result::Ok; }
  Result OnCallExpr(CallExpr*) override { return Result::Ok; }
  Result OnCallIndirectExpr(CallIndirectExpr*) override { return Result::Ok; }
  Result OnCallRefExpr(CallRefExpr*) override { return Result::Ok; }
  Result OnCodeMetadataExpr(CodeMetadataExpr*) override { return Result::Ok; }
  Result OnCompareExpr(CompareExpr*) override { return Result::Ok; }
  Result OnConstExpr(ConstExpr*) override { return Result::Ok; }
  Result OnConvertExpr(ConvertExpr*) override { return Result::Ok; }
  Result OnDropExpr(DropExpr*) override { return Result::Ok; }
  Result OnGlobalGetExpr(GlobalGetExpr*) override { return Result::Ok; }
  Result OnGlobalSetExpr(GlobalSetExpr*) override { return Result::Ok; }
  Result BeginIfExpr(IfExpr*) override { return Result::Ok; }
  Result AfterIfTrueExpr(IfExpr*) override { return Result::Ok; }
  Result EndIfExpr(IfExpr*) override { return Result::Ok; }
  Result OnLoadExpr(LoadExpr*) override { return Result::Ok; }
  Result OnLocalGetExpr(LocalGetExpr*) override { return Result::Ok; }
  Result OnLocalSetExpr(LocalSetExpr*) override { return Result::Ok; }
  Result OnLocalTeeExpr(LocalTeeExpr*) override { return Result::Ok; }
  Result BeginLoopExpr(LoopExpr*) override { return Result::Ok; }
  Result EndLoopExpr(LoopExpr*) override { return Result::Ok; }
  Result OnMemoryCopyExpr(MemoryCopyExpr*) override { return Result::Ok; }
  Result OnDataDropExpr(DataDropExpr*) override { return Result::Ok; }
  Result OnMemoryFillExpr(MemoryFillExpr*) override { return Result::Ok; }
  Result OnMemoryGrowExpr(MemoryGrowExpr*) override { return Result::Ok; }
  Result OnMemoryInitExpr(MemoryInitExpr*) override { return Result::Ok; }
  Result OnMemorySizeExpr(MemorySizeExpr*) override { return Result::Ok; }
  Result OnTableCopyExpr(TableCopyExpr*) override { return Result::Ok; }
  Result OnElemDropExpr(ElemDropExpr*) override { return Result::Ok; }
  Result OnTableInitExpr(TableInitExpr*) override { return Result::Ok; }
  Result OnTableGetExpr(TableGetExpr*) override { return Result::Ok; }
  Result OnTableSetExpr(TableSetExpr*) override { return Result::Ok; }
  Result OnTableGrowExpr(TableGrowExpr*) override { return Result::Ok; }
  Result OnTableSizeExpr(TableSizeExpr*) override { return Result::Ok; }
  Result OnTableFillExpr(TableFillExpr*) override { return Result::Ok; }
  Result OnRefFuncExpr(RefFuncExpr*) override { return Result::Ok; }
  Result OnRefNullExpr(RefNullExpr*) override { return Result::Ok; }
  Result OnRefIsNullExpr(RefIsNullExpr*) override { return Result::Ok; }
  Result OnNopExpr(NopExpr*) override { return Result::Ok; }
  Result OnReturnExpr(ReturnExpr*) override { return Result::Ok; }
  Result OnReturnCallExpr(ReturnCallExpr*) override { return Result::Ok; }
  Result OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) override {
    return Result::Ok;
  }
  Result OnSelectExpr(SelectExpr*) override { return Result::Ok; }
  Result OnStoreExpr(StoreExpr*) override { return Result::Ok; }
  Result OnUnaryExpr(UnaryExpr*) override { return Result::Ok; }
  Result OnUnreachableExpr(UnreachableExpr*) override { return Result::Ok; }
  Result BeginTryExpr(TryExpr*) override { return Result::Ok; }
  Result OnCatchExpr(TryExpr*, Catch*) override { return Result::Ok; }
  Result OnDelegateExpr(TryExpr*) override { return Result::Ok; }
  Result EndTryExpr(TryExpr*) override { return Result::Ok; }
  Result OnThrowExpr(ThrowExpr*) override { return Result::Ok; }
  Result OnThrowRefExpr(ThrowRefExpr*) override { return Result::Ok; }
  Result OnRethrowExpr(RethrowExpr*) override { return Result::Ok; }
  Result OnAtomicWaitExpr(AtomicWaitExpr*) override { return Result::Ok; }
  Result OnAtomicFenceExpr(AtomicFenceExpr*) override { return Result::Ok; }
  Result OnAtomicNotifyExpr(AtomicNotifyExpr*) override { return Result::Ok; }
  Result OnAtomicLoadExpr(AtomicLoadExpr*) override { return Result::Ok; }
  Result OnAtomicStoreExpr(AtomicStoreExpr*) override { return Result::Ok; }
  Result OnAtomicRmwExpr(AtomicRmwExpr*) override { return Result::Ok; }
  Result OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) override {
    return Result::Ok;
  }
  Result OnTernaryExpr(TernaryExpr*) override { return Result::Ok; }
  Result OnSimdLaneOpExpr(SimdLaneOpExpr*) override { return Result::Ok; }
  Result OnSimdLoadLaneExpr(SimdLoadLaneExpr*) override { return Result::Ok; }
  Result OnSimdStoreLaneExpr(SimdStoreLaneExpr*) override { return Result::Ok; }
  Result OnSimdShuffleOpExpr(SimdShuffleOpExpr*) override { return Result::Ok; }
  Result OnLoadSplatExpr(LoadSplatExpr*) override { return Result::Ok; }
  Result OnLoadZeroExpr(LoadZeroExpr*) override { return Result::Ok; }
  Result OnOpcodeRawExpr(OpcodeRawExpr*) override { return Result::Ok; }
};

}  // namespace wabt

#endif  // WABT_EXPR_VISITOR_H_
