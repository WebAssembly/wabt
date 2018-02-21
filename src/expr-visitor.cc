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

#include "src/expr-visitor.h"

#include "src/cast.h"
#include "src/ir.h"

namespace wabt {

ExprVisitor::ExprVisitor(Delegate* delegate) : delegate_(delegate) {}

Result ExprVisitor::VisitExpr(Expr* root_expr) {
  enum class State {
    Default,
    Block,
    IfTrue,
    IfFalse,
    Loop,
    Try,
    TryCatch,
  };

  // Use parallel arrays instead of array of structs so we can avoid allocating
  // unneeded objects. ExprList::iterator has no default constructor, so it
  // must only be allocated for states that use it.
  //
  // NOTE(binji): This can be simplified when the new exception proposal is
  // implemented, as it only allows for one catch block per try block.
  std::vector<State> state_stack;
  std::vector<Expr*> expr_stack;
  std::vector<ExprList::iterator> expr_iter_stack;
  std::vector<Index> catch_index_stack;

  auto push_default = [&](Expr* expr) {
    state_stack.emplace_back(State::Default);
    expr_stack.emplace_back(expr);
  };

  auto pop_default = [&]() {
    state_stack.pop_back();
    expr_stack.pop_back();
  };

  auto push_exprlist = [&](State state, Expr* expr, ExprList& expr_list) {
    state_stack.emplace_back(state);
    expr_stack.emplace_back(expr);
    expr_iter_stack.emplace_back(expr_list.begin());
  };

  auto pop_exprlist = [&]() {
    state_stack.pop_back();
    expr_stack.pop_back();
    expr_iter_stack.pop_back();
  };

  auto push_catch = [&](Expr* expr, Index catch_index, ExprList& expr_list) {
    state_stack.emplace_back(State::TryCatch);
    expr_stack.emplace_back(expr);
    expr_iter_stack.emplace_back(expr_list.begin());
    catch_index_stack.emplace_back(catch_index);
  };

  auto pop_catch = [&]() {
    state_stack.pop_back();
    expr_stack.pop_back();
    expr_iter_stack.pop_back();
    catch_index_stack.pop_back();
  };

  push_default(root_expr);

  while (!state_stack.empty()) {
    State state = state_stack.back();
    auto* expr = expr_stack.back();

    switch (state) {
      case State::Default:
        pop_default();

        switch (expr->type()) {
          case ExprType::AtomicLoad:
            CHECK_RESULT(
                delegate_->OnAtomicLoadExpr(cast<AtomicLoadExpr>(expr)));
            break;

          case ExprType::AtomicStore:
            CHECK_RESULT(
                delegate_->OnAtomicStoreExpr(cast<AtomicStoreExpr>(expr)));
            break;

          case ExprType::AtomicRmw:
            CHECK_RESULT(delegate_->OnAtomicRmwExpr(cast<AtomicRmwExpr>(expr)));
            break;

          case ExprType::AtomicRmwCmpxchg:
            CHECK_RESULT(delegate_->OnAtomicRmwCmpxchgExpr(
                cast<AtomicRmwCmpxchgExpr>(expr)));
            break;

          case ExprType::AtomicWait:
            CHECK_RESULT(
                delegate_->OnAtomicWaitExpr(cast<AtomicWaitExpr>(expr)));
            break;

          case ExprType::AtomicWake:
            CHECK_RESULT(
                delegate_->OnAtomicWakeExpr(cast<AtomicWakeExpr>(expr)));
            break;

          case ExprType::Binary:
            CHECK_RESULT(delegate_->OnBinaryExpr(cast<BinaryExpr>(expr)));
            break;

          case ExprType::Block: {
            auto block_expr = cast<BlockExpr>(expr);
            CHECK_RESULT(delegate_->BeginBlockExpr(block_expr));
            push_exprlist(State::Block, expr, block_expr->block.exprs);
            break;
          }

          case ExprType::Br:
            CHECK_RESULT(delegate_->OnBrExpr(cast<BrExpr>(expr)));
            break;

          case ExprType::BrIf:
            CHECK_RESULT(delegate_->OnBrIfExpr(cast<BrIfExpr>(expr)));
            break;

          case ExprType::BrTable:
            CHECK_RESULT(delegate_->OnBrTableExpr(cast<BrTableExpr>(expr)));
            break;

          case ExprType::Call:
            CHECK_RESULT(delegate_->OnCallExpr(cast<CallExpr>(expr)));
            break;

          case ExprType::CallIndirect:
            CHECK_RESULT(
                delegate_->OnCallIndirectExpr(cast<CallIndirectExpr>(expr)));
            break;

          case ExprType::Compare:
            CHECK_RESULT(delegate_->OnCompareExpr(cast<CompareExpr>(expr)));
            break;

          case ExprType::Const:
            CHECK_RESULT(delegate_->OnConstExpr(cast<ConstExpr>(expr)));
            break;

          case ExprType::Convert:
            CHECK_RESULT(delegate_->OnConvertExpr(cast<ConvertExpr>(expr)));
            break;

          case ExprType::CurrentMemory:
            CHECK_RESULT(
                delegate_->OnCurrentMemoryExpr(cast<CurrentMemoryExpr>(expr)));
            break;

          case ExprType::Drop:
            CHECK_RESULT(delegate_->OnDropExpr(cast<DropExpr>(expr)));
            break;

          case ExprType::GetGlobal:
            CHECK_RESULT(delegate_->OnGetGlobalExpr(cast<GetGlobalExpr>(expr)));
            break;

          case ExprType::GetLocal:
            CHECK_RESULT(delegate_->OnGetLocalExpr(cast<GetLocalExpr>(expr)));
            break;

          case ExprType::GrowMemory:
            CHECK_RESULT(
                delegate_->OnGrowMemoryExpr(cast<GrowMemoryExpr>(expr)));
            break;

          case ExprType::If: {
            auto if_expr = cast<IfExpr>(expr);
            CHECK_RESULT(delegate_->BeginIfExpr(if_expr));
            push_exprlist(State::IfTrue, expr, if_expr->true_.exprs);
            break;
          }

          case ExprType::Load:
            CHECK_RESULT(delegate_->OnLoadExpr(cast<LoadExpr>(expr)));
            break;

          case ExprType::Loop: {
            auto loop_expr = cast<LoopExpr>(expr);
            CHECK_RESULT(delegate_->BeginLoopExpr(loop_expr));
            push_exprlist(State::Loop, expr, loop_expr->block.exprs);
            break;
          }

          case ExprType::Nop:
            CHECK_RESULT(delegate_->OnNopExpr(cast<NopExpr>(expr)));
            break;

          case ExprType::Rethrow:
            CHECK_RESULT(delegate_->OnRethrowExpr(cast<RethrowExpr>(expr)));
            break;

          case ExprType::Return:
            CHECK_RESULT(delegate_->OnReturnExpr(cast<ReturnExpr>(expr)));
            break;

          case ExprType::Select:
            CHECK_RESULT(delegate_->OnSelectExpr(cast<SelectExpr>(expr)));
            break;

          case ExprType::SetGlobal:
            CHECK_RESULT(delegate_->OnSetGlobalExpr(cast<SetGlobalExpr>(expr)));
            break;

          case ExprType::SetLocal:
            CHECK_RESULT(delegate_->OnSetLocalExpr(cast<SetLocalExpr>(expr)));
            break;

          case ExprType::Store:
            CHECK_RESULT(delegate_->OnStoreExpr(cast<StoreExpr>(expr)));
            break;

          case ExprType::TeeLocal:
            CHECK_RESULT(delegate_->OnTeeLocalExpr(cast<TeeLocalExpr>(expr)));
            break;

          case ExprType::Throw:
            CHECK_RESULT(delegate_->OnThrowExpr(cast<ThrowExpr>(expr)));
            break;

          case ExprType::TryBlock: {
            auto try_expr = cast<TryExpr>(expr);
            CHECK_RESULT(delegate_->BeginTryExpr(try_expr));
            push_exprlist(State::Try, expr, try_expr->block.exprs);
            break;
          }

          case ExprType::Unary:
            CHECK_RESULT(delegate_->OnUnaryExpr(cast<UnaryExpr>(expr)));
            break;

          case ExprType::Ternary:
            CHECK_RESULT(delegate_->OnTernaryExpr(cast<TernaryExpr>(expr)));
            break;

          case ExprType::Unreachable:
            CHECK_RESULT(
                delegate_->OnUnreachableExpr(cast<UnreachableExpr>(expr)));
            break;
        }
        break;

      case State::Block: {
        auto block_expr = cast<BlockExpr>(expr);
        auto& iter = expr_iter_stack.back();
        if (iter != block_expr->block.exprs.end()) {
          push_default(&*iter++);
        } else {
          CHECK_RESULT(delegate_->EndBlockExpr(block_expr));
          pop_exprlist();
        }
        break;
      }

      case State::IfTrue: {
        auto if_expr = cast<IfExpr>(expr);
        auto& iter = expr_iter_stack.back();
        if (iter != if_expr->true_.exprs.end()) {
          push_default(&*iter++);
        } else {
          CHECK_RESULT(delegate_->AfterIfTrueExpr(if_expr));
          pop_exprlist();
          push_exprlist(State::IfFalse, expr, if_expr->false_);
        }
        break;
      }

      case State::IfFalse: {
        auto if_expr = cast<IfExpr>(expr);
        auto& iter = expr_iter_stack.back();
        if (iter != if_expr->false_.end()) {
          push_default(&*iter++);
        } else {
          CHECK_RESULT(delegate_->EndIfExpr(if_expr));
          pop_exprlist();
        }
        break;
      }

      case State::Loop: {
        auto loop_expr = cast<LoopExpr>(expr);
        auto& iter = expr_iter_stack.back();
        if (iter != loop_expr->block.exprs.end()) {
          push_default(&*iter++);
        } else {
          CHECK_RESULT(delegate_->EndLoopExpr(loop_expr));
          pop_exprlist();
        }
        break;
      }

      case State::Try: {
        auto try_expr = cast<TryExpr>(expr);
        auto& iter = expr_iter_stack.back();
        if (iter != try_expr->block.exprs.end()) {
          push_default(&*iter++);
        } else {
          pop_exprlist();
          if (!try_expr->catches.empty()) {
            Catch& catch_ = try_expr->catches[0];
            CHECK_RESULT(delegate_->OnCatchExpr(try_expr, &catch_));
            push_catch(expr, 0, catch_.exprs);
          } else {
            CHECK_RESULT(delegate_->EndTryExpr(try_expr));
          }
        }

        break;
      }

      case State::TryCatch: {
        auto try_expr = cast<TryExpr>(expr);
        Index catch_index = catch_index_stack.back();
        auto& iter = expr_iter_stack.back();
        if (iter != try_expr->catches[catch_index].exprs.end()) {
          push_default(&*iter++);
        } else {
          pop_catch();
          catch_index++;
          if (catch_index < try_expr->catches.size()) {
            Catch& catch_ = try_expr->catches[catch_index];
            CHECK_RESULT(delegate_->OnCatchExpr(try_expr, &catch_));
            push_catch(expr, catch_index, catch_.exprs);
          } else {
            CHECK_RESULT(delegate_->EndTryExpr(try_expr));
          }
        }
      }
    }
  }

  return Result::Ok;
}

Result ExprVisitor::VisitExprList(ExprList& exprs) {
  for (Expr& expr : exprs)
    CHECK_RESULT(VisitExpr(&expr));
  return Result::Ok;
}

Result ExprVisitor::VisitFunc(Func* func) {
  return VisitExprList(func->exprs);
}

}  // namespace wabt
