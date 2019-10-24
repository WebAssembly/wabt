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

#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include <sstream>

#include "src/cast.h"
#include "src/common.h"
#include "src/expr-visitor.h"
#include "src/ir.h"
#include "src/ir-util.h"
#include "src/literal.h"
#include "src/generate-names.h"
#include "src/stream.h"

namespace wabt {

enum class NodeType {
  PushAll,
  Pop,
  Statements,
  EndReturn,
  Decl,
  DeclInit,
  Expr
};

// The AST we're going to convert the standard IR into.
struct Node {
  NodeType ntype;
  ExprType etype;  // Only if ntype == Expr.
  const Expr* e;
  std::vector<Node> children;
  // Node specific annotations.
  union {
    LabelType lt;  // br/br_if target.
    const Var* var;  // Decl/DeclInit.
  };

  Node()
    : ntype(NodeType::Expr), etype(ExprType::Nop), e(nullptr),
      var(nullptr) {}
  Node(NodeType ntype, ExprType etype, const Expr* e, const Var* v)
    : ntype(ntype), etype(etype), e(e), var(v) {}

  // This value should really never be copied, only moved.
  Node(Node&& rhs) = default;
  Node(const Node& rhs) = delete;
  Node& operator=(Node&& rhs) = default;
  Node& operator=(const Node& rhs) = delete;
};

struct AST {
  AST(ModuleContext& mc, const Func *f) : mc(mc), f(f) {
    if (f) {
      mc.BeginFunc(*f);
      for (Index i = 0; i < f->GetNumParams(); i++) {
        auto name = IndexToAlphaName(i);
        vars_defined.insert(name);
      }
    }
  }

  ~AST() {
    if (f) mc.EndFunc();
  }

  void NewNode(NodeType ntype, ExprType etype, const Expr* e, Index nargs) {
    assert(stack.size() >= nargs);
    Node n { ntype, etype, e, nullptr };
    n.children.reserve(nargs);
    std::move(stack.end() - nargs, stack.end(),
              std::back_inserter(n.children));
    stack.resize(stack.size() - nargs);
    stack.push_back(std::move(n));
  }

  template<ExprType T> void PreDecl(const VarExpr<T>& ve) {
    stack.emplace(stack.begin() + pre_decl_insertion_point++,
                  NodeType::Decl, ExprType::Nop, nullptr, &ve.var);
  }

  template<ExprType T> void Get(const VarExpr<T>& ve, bool local) {
    if (local && vars_defined.insert(ve.var.name()).second) {
      // Use before def, may happen since locals are guaranteed 0.
      PreDecl(ve);
    }
    NewNode(NodeType::Expr, T, &ve, 0);
  }

  template<ExprType T> void Set(const VarExpr<T>& ve, bool local) {
    // Seen this var before?
    if (local && vars_defined.insert(ve.var.name()).second) {
      if (stack_depth == 1) {
        // Top level, declare it here.
        NewNode(NodeType::DeclInit, ExprType::Nop, nullptr, 1);
        stack.back().var = &ve.var;
        return;
      } else {
        // Inside exp, better leave it as assignment exp and lift the decl out.
        PreDecl(ve);
      }
    }
    NewNode(NodeType::Expr, T, &ve, 1);
  }

  template<ExprType T> void Block(const BlockExprBase<T>& be, LabelType label) {
    mc.BeginBlock(label, be.block);
    Construct(be.block.exprs, be.block.decl.GetNumResults(), false);
    mc.EndBlock();
    NewNode(NodeType::Expr, T, &be, 1);
  }

  void Construct(const Expr& e) {
    auto arity = mc.GetExprArity(e);
    switch (e.type()) {
      case ExprType::LocalGet: {
        Get(*cast<LocalGetExpr>(&e), true);
        return;
      }
      case ExprType::GlobalGet: {
        Get(*cast<GlobalGetExpr>(&e), false);
        return;
      }
      case ExprType::LocalSet: {
        Set(*cast<LocalSetExpr>(&e), true);
        return;
      }
      case ExprType::GlobalSet: {
        Set(*cast<GlobalSetExpr>(&e), false);
        return;
      }
      case ExprType::LocalTee: {
        auto& lt = *cast<LocalTeeExpr>(&e);
        Set(lt, true);
        if (stack_depth == 1) {  // Tee is the only thing on there.
          Get(lt, true);  // Now Set + Get instead.
        } else {
          // Things are above us on the stack so the Tee can't be eliminated.
          // The Set makes this work as a Tee when consumed by a parent.
        }
        return;
      }
      case ExprType::If: {
        auto ife = cast<IfExpr>(&e);
        stack_depth--;  // Condition.
        mc.BeginBlock(LabelType::Block, ife->true_);
        Construct(ife->true_.exprs, ife->true_.decl.GetNumResults(), false);
        if (!ife->false_.empty()) {
          Construct(ife->false_, ife->true_.decl.GetNumResults(), false);
        }
        mc.EndBlock();
        stack_depth++;  // Put Condition back.
        NewNode(NodeType::Expr, ExprType::If, &e, ife->false_.empty() ? 2 : 3);
        return;
      }
      case ExprType::Block: {
        Block(*cast<BlockExpr>(&e), LabelType::Block);
        return;
      }
      case ExprType::Loop: {
        Block(*cast<LoopExpr>(&e), LabelType::Loop);
        return;
      }
      case ExprType::Br: {
        NewNode(NodeType::Expr, ExprType::Br, &e, 0);
        stack.back().lt = mc.GetLabel(cast<BrExpr>(&e)->var)->label_type;
        return;
      }
      case ExprType::BrIf: {
        NewNode(NodeType::Expr, ExprType::BrIf, &e, 1);
        stack.back().lt = mc.GetLabel(cast<BrIfExpr>(&e)->var)->label_type;
        return;
      }
      default: {
        NewNode(NodeType::Expr, e.type(), &e, mc.GetExprArity(e).nargs);
        return;
      }
    }
    NewNode(NodeType::Expr, e.type(), &e, arity.nargs);
  }

  void Construct(const ExprList& es, Index nresults, bool return_results) {
    auto start = stack.size();
    auto stack_depth_start = stack_depth;
    bool unreachable = false;
    for (auto& e : es) {
      Construct(e);
      auto arity = mc.GetExprArity(e);
      stack_depth -= arity.nargs;
      stack_depth += arity.nreturns;
      unreachable = unreachable || arity.unreachable;
      assert(unreachable || stack_depth >= 0);
      if (arity.nreturns > 1) {
        // Multivalue: we "push" everything on to the stack.
        NewNode(NodeType::PushAll, ExprType::Nop, nullptr, 1);
        // All values become pops.
        for (Index i = 0; i < arity.nreturns; i++)
          NewNode(NodeType::Pop, ExprType::Nop, nullptr, 0);
        // TODO: can also implement a push_all_but_one that returns the top,
        // then insert N-1 pops below it. Or have a function that returns N
        // values direcly to N arguments for when structs are passed on,
        // etc.
      }
    }
    assert(unreachable ||
           stack_depth - stack_depth_start == static_cast<int>(nresults));
    // Undo any changes to stack_depth, since parent takes care of arity
    // changes.
    stack_depth = stack_depth_start;
    auto end = stack.size();
    assert(end >= start);
    if (return_results && !stack.empty()) {
      if (stack.back().etype == ExprType::Return) {
        if (stack.back().children.empty()) {
          // Return statement at the end of a void function.
          stack.pop_back();
        }
      } else if (nresults) {
        // Combine nresults into a return statement, for when this is used as
        // a function body.
        // TODO: if this is some other kind of block and >1 value is being
        // returned, probably need some kind of syntax to make that clearer.
        NewNode(NodeType::EndReturn, ExprType::Nop, nullptr, nresults);
      }
    }
    end = stack.size();
    assert(end >= start);
    auto size = end - start;
    if (size != 1) {
      NewNode(NodeType::Statements, ExprType::Nop, nullptr, size);
    }
  }

  ModuleContext& mc;
  std::vector<Node> stack;
  const Func *f;
  size_t pre_decl_insertion_point = 0;
  int stack_depth = 0;
  std::set<std::string> vars_defined;
};

}  // namespace wabt
