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

#include "expr-visitor.h"

#include "ir.h"

#define CHECK_RESULT(expr)   \
  do {                       \
    if (WABT_FAILED((expr))) \
      return Result::Error;  \
  } while (0)

namespace wabt {

ExprVisitor::ExprVisitor(Delegate* delegate) : delegate_(delegate) {}

Result ExprVisitor::VisitExpr(Expr* expr) {
  switch (expr->type) {
    case ExprType::Binary:
      CHECK_RESULT(delegate_->OnBinaryExpr(expr->As<BinaryExpr>()));
      break;

    case ExprType::Block: {
      auto block_expr = expr->As<BlockExpr>();
      CHECK_RESULT(delegate_->BeginBlockExpr(block_expr));
      CHECK_RESULT(VisitExprList(block_expr->block->first));
      CHECK_RESULT(delegate_->EndBlockExpr(block_expr));
      break;
    }

    case ExprType::Br:
      CHECK_RESULT(delegate_->OnBrExpr(expr->As<BrExpr>()));
      break;

    case ExprType::BrIf:
      CHECK_RESULT(delegate_->OnBrIfExpr(expr->As<BrIfExpr>()));
      break;

    case ExprType::BrTable:
      CHECK_RESULT(delegate_->OnBrTableExpr(expr->As<BrTableExpr>()));
      break;

    case ExprType::Call:
      CHECK_RESULT(delegate_->OnCallExpr(expr->As<CallExpr>()));
      break;

    case ExprType::CallIndirect:
      CHECK_RESULT(delegate_->OnCallIndirectExpr(expr->As<CallIndirectExpr>()));
      break;

    case ExprType::Compare:
      CHECK_RESULT(delegate_->OnCompareExpr(expr->As<CompareExpr>()));
      break;

    case ExprType::Const:
      CHECK_RESULT(delegate_->OnConstExpr(expr->As<ConstExpr>()));
      break;

    case ExprType::Convert:
      CHECK_RESULT(delegate_->OnConvertExpr(expr->As<ConvertExpr>()));
      break;

    case ExprType::CurrentMemory:
      CHECK_RESULT(
          delegate_->OnCurrentMemoryExpr(expr->As<CurrentMemoryExpr>()));
      break;

    case ExprType::Drop:
      CHECK_RESULT(delegate_->OnDropExpr(expr->As<DropExpr>()));
      break;

    case ExprType::GetGlobal:
      CHECK_RESULT(delegate_->OnGetGlobalExpr(expr->As<GetGlobalExpr>()));
      break;

    case ExprType::GetLocal:
      CHECK_RESULT(delegate_->OnGetLocalExpr(expr->As<GetLocalExpr>()));
      break;

    case ExprType::GrowMemory:
      CHECK_RESULT(delegate_->OnGrowMemoryExpr(expr->As<GrowMemoryExpr>()));
      break;

    case ExprType::If: {
      auto if_expr = expr->As<IfExpr>();
      CHECK_RESULT(delegate_->BeginIfExpr(if_expr));
      CHECK_RESULT(VisitExprList(if_expr->true_->first));
      CHECK_RESULT(delegate_->AfterIfTrueExpr(if_expr));
      CHECK_RESULT(VisitExprList(if_expr->false_));
      CHECK_RESULT(delegate_->EndIfExpr(if_expr));
      break;
    }

    case ExprType::Load:
      CHECK_RESULT(delegate_->OnLoadExpr(expr->As<LoadExpr>()));
      break;

    case ExprType::Loop: {
      auto loop_expr = expr->As<LoopExpr>();
      CHECK_RESULT(delegate_->BeginLoopExpr(loop_expr));
      CHECK_RESULT(VisitExprList(loop_expr->block->first));
      CHECK_RESULT(delegate_->EndLoopExpr(loop_expr));
      break;
    }

    case ExprType::Nop:
      CHECK_RESULT(delegate_->OnNopExpr(expr->As<NopExpr>()));
      break;

    case ExprType::Rethrow:
      CHECK_RESULT(delegate_->OnRethrowExpr(expr->As<RethrowExpr>()));
      break;

    case ExprType::Return:
      CHECK_RESULT(delegate_->OnReturnExpr(expr->As<ReturnExpr>()));
      break;

    case ExprType::Select:
      CHECK_RESULT(delegate_->OnSelectExpr(expr->As<SelectExpr>()));
      break;

    case ExprType::SetGlobal:
      CHECK_RESULT(delegate_->OnSetGlobalExpr(expr->As<SetGlobalExpr>()));
      break;

    case ExprType::SetLocal:
      CHECK_RESULT(delegate_->OnSetLocalExpr(expr->As<SetLocalExpr>()));
      break;

    case ExprType::Store:
      CHECK_RESULT(delegate_->OnStoreExpr(expr->As<StoreExpr>()));
      break;

    case ExprType::TeeLocal:
      CHECK_RESULT(delegate_->OnTeeLocalExpr(expr->As<TeeLocalExpr>()));
      break;

    case ExprType::Throw:
      CHECK_RESULT(delegate_->OnThrowExpr(expr->As<ThrowExpr>()));
      break;

    case ExprType::TryBlock: {
      auto try_expr = expr->As<TryExpr>();
      CHECK_RESULT(delegate_->BeginTryExpr(try_expr));
      CHECK_RESULT(VisitExprList(try_expr->block->first));
      for (Catch* catch_ : try_expr->catches) {
        CHECK_RESULT(delegate_->OnCatchExpr(try_expr, catch_));
        CHECK_RESULT(VisitExprList(catch_->first));
      }
      CHECK_RESULT(delegate_->EndTryExpr(try_expr));
      break;
    }

    case ExprType::Unary:
      CHECK_RESULT(delegate_->OnUnaryExpr(expr->As<UnaryExpr>()));
      break;

    case ExprType::Unreachable:
      CHECK_RESULT(delegate_->OnUnreachableExpr(expr->As<UnreachableExpr>()));
      break;
  }

  return Result::Ok;
}

Result ExprVisitor::VisitExprList(Expr* first) {
  for (Expr* expr = first; expr; expr = expr->next)
    CHECK_RESULT(VisitExpr(expr));
  return Result::Ok;
}

Result ExprVisitor::VisitFunc(Func* func) {
  return VisitExprList(func->first_expr);
}

}  // namespace wabt
