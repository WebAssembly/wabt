/*
 * Copyright 2019 WebAssembly Community Group participants
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

#include "src/decompiler.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <iterator>
#include <map>
#include <numeric>
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

#define WABT_TRACING 0
#include "src/tracing.h"

namespace wabt {

namespace {

struct Decompiler : ModuleContext {
  Decompiler(Stream& stream, const Module& module,
             const DecompileOptions& options)
      : ModuleContext(module), stream(stream), options(options) {}

  struct Value {
    std::vector<std::string> v;
    // Lazily add bracketing only if the parent requires it.
    // TODO: replace with a system based on precedence?
    bool needs_bracketing;

    size_t width() {
      size_t w = 0;
      for (auto &s : v) {
        w = std::max(w, s.size());
      }
      return w;
    }
  };

  std::string Indent(size_t amount) {
    return std::string(amount, ' ');
  }

  string_view OpcodeToToken(Opcode opcode) {
    return opcode.GetDecomp();
  }

  void IndentValue(Value &val, size_t amount, string_view first_indent) {
    WABT_TRACE_ARGS(IndentValue, "\"" PRIstringview "\" - %d",
                    WABT_PRINTF_STRING_VIEW_ARG(val.v[0]), val.v.size());
    auto indent = Indent(amount);
    for (auto& s : val.v) {
      auto is = (&s != &val.v[0] || first_indent.empty())
                ? string_view(indent)
                : first_indent;
      s.insert(0, is.data(), is.size());
    }
  }

  void WrapChild(Value &child, string_view prefix, string_view postfix) {
    WABT_TRACE_ARGS(WrapChild, "\"" PRIstringview "\" - \"" PRIstringview "\"",
                    WABT_PRINTF_STRING_VIEW_ARG(prefix),
                    WABT_PRINTF_STRING_VIEW_ARG(postfix));
    auto width = prefix.size() + postfix.size() + child.width();
    auto &v = child.v;
    if (width < target_exp_width ||
        (prefix.size() <= indent_amount && postfix.size() <= indent_amount)) {
      if (v.size() == 1) {
        // Fits in a single line.
        v[0].insert(0, prefix.data(), prefix.size());
        v[0].append(postfix.data(), postfix.size());
      } else {
        // Multiline, but with prefix on same line.
        IndentValue(child, prefix.size(), prefix);
        v.back().append(postfix.data(), postfix.size());
      }
    } else {
      // Multiline with prefix on its own line.
      IndentValue(child, indent_amount, {});
      v.insert(v.begin(), std::string(prefix));
      v.back().append(postfix.data(), postfix.size());
    }
  }

  void BracketIfNeeded(Value &val) {
    if (!val.needs_bracketing) return;
    WrapChild(val, "(", ")");
    val.needs_bracketing = false;
  }

  void WrapBinary(string_view infix, bool indent_right) {
    auto right = std::move(stack.back());
    stack.pop_back();
    BracketIfNeeded(right);
    auto left = std::move(stack.back());
    stack.pop_back();
    BracketIfNeeded(left);
    auto width = infix.size() + left.width() + right.width();
    if (width < target_exp_width && left.v.size() == 1 && right.v.size() == 1) {
      stack.push_back({
        { left.v[0] + std::string(infix) + right.v[0] },
        true
      });
    } else {
      Value bin { {}, true };
      std::move(left.v.begin(), left.v.end(), std::back_inserter(bin.v));
      bin.v.back().append(infix.data(), infix.size());
      if (indent_right) IndentValue(right, indent_amount, {});
      std::move(right.v.begin(), right.v.end(), std::back_inserter(bin.v));
      stack.push_back(std::move(bin));
    }
  }

  void WrapNAry(Index nargs, string_view prefix, string_view postfix) {
    size_t total_width = 0;
    size_t max_width = 0;
    bool multiline = false;
    for (Index i = 0; i < nargs; i++) {
      auto& child = stack[stack.size() - i - 1];
      auto w = child.width();
      max_width = std::max(max_width, w);
      total_width += w;
      multiline = multiline || child.v.size() > 1;
    }
    if (!multiline &&
        total_width + prefix.size() + postfix.size() < target_exp_width) {
      // Single line.
      ss << prefix;
      if (nargs) {
        for (Index i = 0; i < nargs; i++) {
          auto& child = stack[stack.size() - nargs + i];
          if (i) ss << ", ";
          ss << child.v[0];
        }
        stack.resize(stack.size() - nargs);
      }
      ss << postfix;
      PushSStream();
      return;
    } else {
      // Multi-line.
      auto ident_with_name = max_width + prefix.size() < target_exp_width;
      for (Index i = 0; i < nargs; i++) {
        auto& child = stack[stack.size() - nargs + i];
        IndentValue(child, ident_with_name ? prefix.size() : indent_amount,
                    !i && ident_with_name ? prefix : string_view {});
        child.v.back() += i == nargs - 1 ? std::string(postfix) : ",";
        if (i) {
          std::move(child.v.begin(), child.v.end(),
                    std::back_inserter(stack[stack.size() - nargs].v));
        } else if (!ident_with_name) {
          child.v.insert(child.v.begin(), std::string(prefix));
        }
      }
      stack.erase(stack.end() - nargs + 1, stack.end());
      return;
    }
  }

  const char *GetDecompTypeName(Type t) {
    switch (t) {
      case Type::I32: return "int";
      case Type::I64: return "long";
      case Type::F32: return "float";
      case Type::F64: return "double";
      case Type::V128: return "simd";
      case Type::Anyref: return "anyref";
      case Type::Func: return "func";
      case Type::Funcref: return "funcref";
      case Type::Exnref: return "exceptionref";
      case Type::Void: return "void";
      default: return "illegal";
    }
  }

  Type GetTypeFromString(string_view name) {
    if (name == "i32") return Type::I32;
    if (name == "i64") return Type::I64;
    if (name == "f32") return Type::F32;
    if (name == "f64") return Type::F64;
    return Type::Any;
  }

  // Turns e.g. "i32.load8_u" -> "int_8_u"
  std::string TypeFromLoadStore(Opcode opcode, string_view name) {
    auto op = std::string(OpcodeToToken(opcode));
    auto load_pos = op.find(name.data(), 0, name.size());
    if (load_pos != std::string::npos) {
      auto t = GetTypeFromString(string_view(op.data(), load_pos));
      auto s = std::string(GetDecompTypeName(t));
      if (op.size() <= load_pos + name.size()) return s;
      return s + "_" + (op.data() + load_pos + name.size());
    }
    return op;
  }

  void PushSStream() {
    stack.push_back({ { ss.str() }, false });
    ss.str({});
  }

  template<ExprType T> void Get(const VarExpr<T>& ve) {
    ss << ve.var.name();
    PushSStream();
  }

  template<ExprType T> void Set(const VarExpr<T>& ve) {
    WrapChild(stack.back(), ve.var.name() + " = ", "");
    stack.back().needs_bracketing = true;
  }

  void DecompileExpr(const Expr& e) {
    switch (e.type()) {
      case ExprType::Const: {
        auto& c = cast<ConstExpr>(&e)->const_;
        switch (c.type) {
          case Type::I32:
            ss << static_cast<int32_t>(c.u32);
            break;
          case Type::I64:
            ss << static_cast<int64_t>(c.u64) << "L";
            break;
          case Type::F32:
            ss << *reinterpret_cast<const float *>(&c.f32_bits) << "f";
            break;
          case Type::F64:
            ss << *reinterpret_cast<const double *>(&c.f64_bits) << "d";
            break;
          case Type::V128:
            ss << "V128";  // FIXME
            break;
          default:
            WABT_UNREACHABLE;
        }
        PushSStream();
        return;
      }
      case ExprType::LocalGet: {
        Get(*cast<LocalGetExpr>(&e));
        return;
      }
      case ExprType::GlobalGet: {
        Get(*cast<GlobalGetExpr>(&e));
        return;
      }
      case ExprType::LocalSet: {
        Set(*cast<LocalSetExpr>(&e));
        return;
      }
      case ExprType::GlobalSet: {
        Set(*cast<GlobalSetExpr>(&e));
        return;
      }
      case ExprType::LocalTee: {
        auto& lt = *cast<LocalTeeExpr>(&e);
        Set(lt);
        if (stack_depth == 1) {  // Tee is the only thing on there.
          Get(lt);  // Now Set + Get instead.
        } else {
          // Things are above us on the stack so the Tee can't be eliminated.
          // The Set makes this work as a Tee when consumed by a parent.
        }
        return;
      }
      case ExprType::Binary: {
        auto& be = *cast<BinaryExpr>(&e);
        WrapBinary(" " + std::string(OpcodeToToken(be.opcode)) + " ", false);
        return;
      }
      case ExprType::Compare: {
        auto& ce = *cast<CompareExpr>(&e);
        WrapBinary(" " + std::string(OpcodeToToken(ce.opcode)) + " ", false);
        return;
      }
      case ExprType::Unary: {
        auto& ue = *cast<UnaryExpr>(&e);
        //BracketIfNeeded(stack.back());
        // TODO: also version without () depending on precedence.
        WrapChild(stack.back(),
                  std::string(OpcodeToToken(ue.opcode)) + "(", ")");
        return;
      }
      case ExprType::Load: {
        auto& le = *cast<LoadExpr>(&e);
        BracketIfNeeded(stack.back());
        std::string suffix = "[";
        suffix += std::to_string(le.offset);
        suffix += "]:";
        suffix += TypeFromLoadStore(le.opcode, string_view(".load"));
        stack.back().v.back() += suffix;
        // FIXME: align
        return;
      }
      case ExprType::Store: {
        auto& se = *cast<StoreExpr>(&e);
        BracketIfNeeded(stack[stack.size() - 2]);
        std::string suffix = "[";
        suffix += std::to_string(se.offset);
        suffix += "]:";
        suffix += TypeFromLoadStore(se.opcode, string_view(".store"));
        stack[stack.size() - 2].v.back() += suffix;
        WrapBinary(" = ", true);
        // FIXME: align
        return;
      }
      case ExprType::If: {
        auto ife = cast<IfExpr>(&e);
        auto ifs = std::move(stack.back());
        stack.pop_back();
        stack_depth--;  // Condition.
        DecompileExprs(ife->true_.exprs, ife->true_.decl.GetNumResults(), false);
        auto thenp = std::move(stack.back());
        stack.pop_back();
        bool multiline = ifs.v.size() > 1 || thenp.v.size() > 1;
        size_t width = ifs.width() + thenp.width();
        Value elsep { {}, false };
        if (!ife->false_.empty()) {
          DecompileExprs(ife->false_, ife->true_.decl.GetNumResults(), false);
          elsep = std::move(stack.back());
          width += elsep.width();
          stack.pop_back();
          multiline = multiline || elsep.v.size() > 1;
        }
        stack_depth++;  // Put Condition back.
        multiline = multiline || width > target_exp_width;
        if (multiline) {
          auto if_start = string_view("if (");
          ifs.v[0].insert(0, if_start.data(), if_start.size());
          ifs.v.back() += ") {";
          IndentValue(thenp, indent_amount, {});
          std::move(thenp.v.begin(), thenp.v.end(), std::back_inserter(ifs.v));
          if (!elsep.v.empty()) {
            ifs.v.push_back("} else {");
            IndentValue(elsep, indent_amount, {});
            std::move(elsep.v.begin(), elsep.v.end(), std::back_inserter(ifs.v));
          }
          ifs.v.push_back("}");
          stack.push_back(std::move(ifs));
          return;
        } else {
          ss << "if (" << ifs.v[0] << ") { " <<  thenp.v[0] << " }";
          if (!elsep.v.empty()) ss << " else { " << elsep.v[0] << " }";
          PushSStream();
          return;
        }
      }
      case ExprType::Block: {
        auto le = cast<BlockExpr>(&e);
        DecompileExprs(le->block.exprs, le->block.decl.GetNumResults(), false);
        auto &val = stack.back();
        IndentValue(val, indent_amount, {});
        val.v.insert(val.v.begin(), "block {");
        val.v.push_back("}");
        return;
      }
      case ExprType::Loop: {
        auto le = cast<LoopExpr>(&e);
        DecompileExprs(le->block.exprs, le->block.decl.GetNumResults(), false);
        auto &val = stack.back();
        IndentValue(val, indent_amount, {});
        val.v.insert(val.v.begin(), "loop {");
        val.v.push_back("}");
        return;
      }
      default: {
        std::string name;
        switch (e.type()) {
          case ExprType::Call:
            name = cast<CallExpr>(&e)->var.name();
            break;
          case ExprType::Convert:
            name = std::string(OpcodeToToken(cast<ConvertExpr>(&e)->opcode));
            break;
          default:
            name = GetExprTypeName(e.type());
            break;
        }
        WrapNAry(GetExprArity(e).nargs, name + "(", ")");
        return;
      }
    }
  }

  void DecompileExprs(const ExprList &es, Index nresults, bool return_results) {
    auto start = stack.size();
    auto stack_depth_start = stack_depth;
    bool unreachable = false;
    for (auto& e : es) {
      DecompileExpr(e);
      auto arity = GetExprArity(e);
      stack_depth -= arity.nargs;
      stack_depth += arity.nreturns;
      unreachable = unreachable || arity.unreachable;
      Assert(unreachable || stack_depth >= 0);
      if (arity.nreturns > 1) {
        // Multivalue: we "push" everything on to the stack.
        WrapChild(stack.back(), "push_all(", ")");
        // All values become pops.
        for (Index i = 0; i < arity.nreturns; i++)
          stack.insert(stack.end(), Value { { "pop()" }, false });
        // TODO: can also implement a push_all_but_one that returns the top,
        // then insert N-1 pops below it. Or have a function that returns N
        // values direcly to N arguments for when structs are passed on,
        // etc.
      }
    }
    Assert(unreachable ||
           stack_depth - stack_depth_start == static_cast<int>(nresults));
    // Undo any changes to stack_depth, since parent takes care of arity
    // changes.
    stack_depth = stack_depth_start;
    auto end = stack.size();
    Assert(end >= start);
    if (return_results) {
      // Combine nresults into a return statement, for when this is used as
      // a function body.
      // TODO: if this is some other kind of block and >1 value is being
      // returned, probably need some kind of syntax to make that clearer.
      WrapNAry(nresults, "return ", "");
    }
    if (end - start == 0) {
      stack.push_back({ { "{}" }, false });
    } else if (end - start > 1) {
      Value stats { {}, false };
      // FIXME: insert ; ?
      for (size_t i = start; i < end; i++) {
        auto &e = stack[i];
        std::move(e.v.begin(), e.v.end(), std::back_inserter(stats.v));
      }
      stack.resize(start);
      stack.push_back(std::move(stats));
    }
  }

  void Decompile() {
    Index func_index = 0;
    for(auto f : module.funcs) {
      BeginFunc(*f);
      stream.Writef("function %s(", f->name.c_str());
      for (Index i = 0; i < f->GetNumParams(); i++) {
        if (i) stream.Writef(", ");
        auto t = f->GetParamType(i);
        stream.Writef("%s:%s", IndexToAlphaName(i).c_str(),
                      GetDecompTypeName(t));
      }
      stream.Writef(")");
      if (f->GetNumResults()) {
        if (f->GetNumResults() == 1) {
          stream.Writef(":%s", GetDecompTypeName(f->GetResultType(0)));
        } else {
          stream.Writef(":(");
          for (Index i = 0; i < f->GetNumResults(); i++) {
            if (i) stream.Writef(", ");
            stream.Writef("%s", GetDecompTypeName(f->GetResultType(i)));
          }
          stream.Writef(")");
        }
      }
      if (module.IsImport(ExternalKind::Func, Var(func_index))) {
        stream.Writef(" = import;\n\n");
      } else {
        stream.Writef(" {\n");
        DecompileExprs(f->exprs, f->GetNumResults(), true);
        IndentValue(stack.back(), indent_amount, {});
        for (auto& s : stack[0].v) {
          stream.Writef("%s\n", s.c_str());
        }
        stream.Writef("}\n\n");
      }
      EndFunc();
      stack.clear();
      func_index++;
    }
  }

  void DumpInternalState() {
    // Dump the remainder of the stack.
    for (auto &se : stack) {
      for (auto& s : se.v) {
        stream.Writef("%s\n", s.c_str());
      }
    }
    stream.Flush();
  }

  // For debugging purposes, this assert, when it fails, first prints the
  // internal state that hasn't been printed yet.
  void Assert(bool ok) {
    if (ok) return;
    #ifndef NDEBUG
      DumpInternalState();
    #endif
    assert(false);
  }

  Stream& stream;
  const DecompileOptions& options;
  std::vector<Value> stack;
  std::ostringstream ss;
  size_t indent_amount = 2;
  size_t target_exp_width = 70;
  int stack_depth = 0;
};

}  // end anonymous namespace

Result Decompile(Stream& stream,
                 const Module& module,
                 const DecompileOptions& options) {
  Decompiler decompiler(stream, module, options);
  decompiler.Decompile();
  return Result::Ok;
}

}  // namespace wabt
