/*
 * Copyright 2018 WebAssembly Community Group participants
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

#include <map>
#include <vector>

#include "src/binary.h"
#include "src/binary-reader.h"
#include "src/binary-reader-ir.h"
#include "src/binary-writer.h"
#include "src/cast.h"
#include "src/error-formatter.h"
#include "src/expr-visitor.h"
#include "src/feature.h"
#include "src/filenames.h"
#include "src/ir.h"
#include "src/make-unique.h"
#include "src/option-parser.h"
#include "src/stream.h"
#include "src/validator.h"

using namespace wabt;

namespace names {

const char kModule[] = "wasm-hook";
const char kBeginFunc[] = "begin-func";  // [func idx:i32] -> []
const char kEndFunc[] = "end-func";      // [func idx:i32] -> []

// i32.load:     [addr:i32, align:i32, offset:i32, val:i32] -> []
// i64.load:     [addr:i32, align:i32, offset:i32, val:i64] -> []
// f32.load:     [addr:i32, align:i32, offset:i32, val:f32] -> []
// f64.load:     [addr:i32, align:i32, offset:i32, val:f64] -> []
// i32.load8_s:  [addr:i32, align:i32, offset:i32, val:i32] -> []
// i32.load8_u:  [addr:i32, align:i32, offset:i32, val:i32] -> []
// i32.load16_s: [addr:i32, align:i32, offset:i32, val:i32] -> []
// i32.load16_u: [addr:i32, align:i32, offset:i32, val:i32] -> []
// i64.load8_s:  [addr:i32, align:i32, offset:i32, val:i64] -> []
// i64.load8_u:  [addr:i32, align:i32, offset:i32, val:i64] -> []
// i64.load16_s: [addr:i32, align:i32, offset:i32, val:i64] -> []
// i64.load16_u: [addr:i32, align:i32, offset:i32, val:i64] -> []
// i64.load32_s: [addr:i32, align:i32, offset:i32, val:i64] -> []
// i64.load32_u: [addr:i32, align:i32, offset:i32, val:i64] -> []

} // namespace names.

static std::string s_infile;
static std::string s_outfile;
static bool s_hook_functions = false;
static bool s_hook_loads = false;

static const char s_description[] =
R"(  Instrument a WebAssembly module and add debugging hooks.

examples:
  # Add begin/end hooks for all defined functions.
  $ wasm-hook -f test.wasm -o test.hook.wasm

  # Add a hook after every load.
  $ wasm-hook -l test.wasm -o test.hook.wasm
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-hook", s_description);

  parser.AddHelpOption();
  parser.AddOption('o', "output", "FILE", "output wasm binary file",
                   [](const char* argument) {
                     s_outfile = argument;
                     ConvertBackslashToSlash(&s_infile);
                   });
  parser.AddOption('f', "function",
                   "Add begin/end hooks to all defined functions",
                   []() { s_hook_functions = true; });
  parser.AddOption('l', "load",
                   "Add begin/end hooks to all memory loads",
                   []() { s_hook_loads = true; });
  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) {
                       s_infile = argument;
                       ConvertBackslashToSlash(&s_infile);
                     });
  parser.Parse(argc, argv);
}

static std::string DefaultOuputName(string_view input_name) {
  // Strip existing extension and add .hook.wasm
  std::string result(StripExtension(GetBasename(input_name)));
  result += ".hook";
  result += kWasmExtension;

  return result;
}

void IncrementVarIndex(Var* var, Index min_index, Index delta = 1) {
  if (var->is_index() && var->index() >= min_index) {
    var->set_index(var->index() + delta);
  }
}

class IncrementFuncIndexDelegate : public ExprVisitor::DelegateNop {
 public:
  IncrementFuncIndexDelegate(Index min_index, Index delta)
      : min_index(min_index), delta(delta) {}

  Result OnCallExpr(CallExpr* expr) override {
    IncrementVarIndex(&expr->var, min_index, delta);
    return Result::Ok;
  }

  Result OnReturnCallExpr(ReturnCallExpr* expr) override {
    IncrementVarIndex(&expr->var, min_index, delta);
    return Result::Ok;
  }

  Index min_index;
  Index delta;
};

void SetFuncSignature(Module* module, Func* func, const FuncSignature& sig) {
  Index func_type_index = module->GetFuncTypeIndex(sig);
  if (func_type_index == kInvalidIndex) {
    auto func_type_field = MakeUnique<FuncTypeModuleField>();
    func_type_field->func_type.sig = sig;

    module->AppendField(std::move(func_type_field));
    func_type_index = module->func_types.size() - 1;
  }

  func->decl.has_func_type = true;
  func->decl.type_var.set_index(func_type_index);
  func->decl.sig = sig;
}

struct FuncImportInfo {
  std::string module;
  std::string field;
  FuncSignature sig;
};

static std::vector<Index> InsertFuncImports(
    Module* module,
    std::vector<FuncImportInfo> func_import_infos) {
  // Fix up all function references after this index.
  Index first_index = module->num_func_imports;
  Index delta = func_import_infos.size();

  // Fixup Func fields.
  for (Func* func: module->funcs) {
    IncrementFuncIndexDelegate delegate(first_index, delta);
    ExprVisitor visitor(&delegate);

    Result result = visitor.VisitFunc(func);
    assert(Succeeded(result));
  }

  // Fixup Export fields.
  for (Export* export_: module->exports) {
    if (export_->kind == ExternalKind::Func) {
      IncrementVarIndex(&export_->var, first_index, delta);
    }
  }

  // Fixup ElemSegment fields.
  for (ElemSegment* elem_segment: module->elem_segments) {
    for (Var& var: elem_segment->vars) {
      IncrementVarIndex(&var, first_index, delta);
    }
  }

  // Fixup Start field.
  for (Var* var: module->starts) {
    IncrementVarIndex(var, first_index, delta);
  }

  // Can't use AppendField since a function import has to come before defined
  // functions
  std::vector<Index> func_indexes;
  for (const FuncImportInfo& func_import_info: func_import_infos) {
    auto func_import = MakeUnique<FuncImport>();
    func_import->module_name = func_import_info.module;
    func_import->field_name = func_import_info.field;

    SetFuncSignature(module, &func_import->func, func_import_info.sig);

    module->imports.push_back(func_import.get());
    module->funcs.insert(module->funcs.begin() + module->num_func_imports,
                         &func_import->func);

    auto field = MakeUnique<ImportModuleField>();
    field->import = std::move(func_import);
    module->fields.push_back(std::move(field));

    func_indexes.push_back(module->num_func_imports);
    module->num_func_imports++;
  }

  return func_indexes;
}

template <typename F>
void ForEachDefinedFunc(Module* module, F&& callback) {
  Index func_index = 0;
  for (Func* func : module->funcs) {
    if (func_index >= module->num_func_imports) {
      callback(func, func_index);
    }

    ++func_index;
  }
}

template <typename F>
void RecurseExprs(ExprList* exprs, F&& callback) {
  for (auto iter = exprs->begin(), end = exprs->end(); iter != end;) {
    Expr& expr = *iter;
    auto next = iter;
    next++;

    callback(exprs, iter);

    switch (expr.type()) {
      case ExprType::Block:
        RecurseExprs(&cast<BlockExpr>(&expr)->block.exprs, callback);
        break;

      case ExprType::If:
        RecurseExprs(&cast<IfExpr>(&expr)->true_.exprs, callback);
        RecurseExprs(&cast<IfExpr>(&expr)->false_, callback);
        break;

      case ExprType::IfExcept:
        RecurseExprs(&cast<IfExceptExpr>(&expr)->true_.exprs, callback);
        RecurseExprs(&cast<IfExceptExpr>(&expr)->false_, callback);
        break;

      case ExprType::Try:
        RecurseExprs(&cast<TryExpr>(&expr)->block.exprs, callback);
        break;

      case ExprType::Loop:
        RecurseExprs(&cast<LoopExpr>(&expr)->block.exprs, callback);
        break;

      default:
        break;
    }

    iter = next;
  }
}

static Index AppendFuncWrapper(Module* module,
                               const Func& func,
                               Index func_index,
                               Index begin_hook_func_index,
                               Index end_hook_func_index) {
  auto field = MakeUnique<FuncModuleField>();
  Func& wrapper_func = field->func;
  SetFuncSignature(module, &wrapper_func, func.decl.sig);

  ExprList& exprs = wrapper_func.exprs;

  //   ;; Call the begin hook.
  //   i32.const func_index
  //   call $begin_hook
  exprs.push_back(MakeUnique<ConstExpr>(Const::I32(func_index)));
  exprs.push_back(MakeUnique<CallExpr>(Var(begin_hook_func_index)));

  //   ;; Call the original function, pushing the params on to the stack.
  //   get_local $p0
  //   get_local $p1
  //   ...
  //   call $orig
  for (Index i = 0; i < func.GetNumParams(); ++i) {
    exprs.push_back(MakeUnique<GetLocalExpr>(Var(i)));
  }
  exprs.push_back(MakeUnique<CallExpr>(Var(func_index)));

  //   ;; Call the end hook.
  //   i32.const func_index
  //   call $end_hook
  exprs.push_back(MakeUnique<ConstExpr>(Const::I32(func_index)));
  exprs.push_back(MakeUnique<CallExpr>(Var(end_hook_func_index)));

  module->AppendField(std::move(field));
  return module->funcs.size() - 1;
}

static void HookFuncs(Module* module) {
  const FuncSignature sig_i_v({Type::I32}, {});
  const std::vector<Index> indexes =
      InsertFuncImports(module, {{names::kModule, names::kBeginFunc, sig_i_v},
                                 {names::kModule, names::kEndFunc, sig_i_v}});
  assert(indexes.size() == 2);
  const Index begin_index = indexes[0];
  const Index end_index = indexes[1];

  Index last_non_wrapper_func_index = module->funcs.size();

  // Create a wrapper function for each original function.
  std::map<Index, Index> func_to_wrapper;
  ForEachDefinedFunc(module, [&](Func* func, Index func_index) {
    Index wrapper_func_index =
        AppendFuncWrapper(module, *func, func_index, begin_index, end_index);
    func_to_wrapper.insert(std::make_pair(func_index, wrapper_func_index));
  });

  auto&& fix_var = [&](Var* var) {
    Index old_index = var->index();
    auto iter = func_to_wrapper.find(old_index);
    if (iter != func_to_wrapper.end()) {
      var->set_index(iter->second);
    }
  };

  // Fixup Export fields.
  for (Export* export_: module->exports) {
    if (export_->kind == ExternalKind::Func) {
      fix_var(&export_->var);
    }
  }

  // Fixup ElemSegment fields.
  for (ElemSegment* elem_segment: module->elem_segments) {
    for (Var& var: elem_segment->vars) {
      fix_var(&var);
    }
  }

  // Fixup Start field.
  for (Var* var: module->starts) {
    fix_var(var);
  }

  ForEachDefinedFunc(module, [&](Func* func, Index func_index) {
    // Only wrap non-wrapper funcs.
    if (func_index >= last_non_wrapper_func_index) {
      return;
    }

    RecurseExprs(&func->exprs, [&](ExprList* exprs, ExprList::iterator iter) {
      auto* expr = dyn_cast<CallExpr>(&*iter);
      if (expr) {
        fix_var(&expr->var);
      }
    });
  });
}

static Index AppendLoadFuncWrapper(Module* module,
                                   const Opcode& op,
                                   Address align,
                                   Index hook_func_index) {
  // Create a wrapper function for any of the various load instructions. A load
  // instruction looks like:
  //
  // i32.load align=ALIGN offset=OFFSET: [addr:i32] -> [val:i32]
  //
  // We create a wrapper function as described in the comments below.
  //
  const Type result_type = op.GetResultType();
  auto field = MakeUnique<FuncModuleField>();
  Func& func = field->func;

  const Index addr = 0;
  const Index offset = 1;
  const Index fulladdr = 2;
  const Index val = 3;

  // (func (param $addr i32) (param $offset i32) (result i32)
  SetFuncSignature(module, &func,
                   FuncSignature({Type::I32, Type::I32}, {result_type}));

  //   (local $fulladdr i64)
  //   (local $val i32)
  func.local_types.AppendDecl(Type::I64, 1);
  func.local_types.AppendDecl(result_type, 1);

  ExprList& exprs = func.exprs;

  //   ;; Calculate addr + offset.
  //   get_local $addr
  //   i64.extend_u/i32
  //   get_local $offset
  //   i64.extend_u/i32
  //   i64.add
  //   tee_local $fulladdr
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(addr)));
  exprs.push_back(MakeUnique<ConvertExpr>(Opcode::I64ExtendUI32_Opcode));
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(offset)));
  exprs.push_back(MakeUnique<ConvertExpr>(Opcode::I64ExtendUI32_Opcode));
  exprs.push_back(MakeUnique<BinaryExpr>(Opcode::I64Add));
  exprs.push_back(MakeUnique<TeeLocalExpr>(Var(fulladdr)));

  //   ;; Trap if >= 32-bits.
  //   i64.const 0x100000000
  //   i64.ge_u
  //   if
  //     unreachable
  //   end
  exprs.push_back(MakeUnique<ConstExpr>(Const::I64(0x100000000)));
  exprs.push_back(MakeUnique<CompareExpr>(Opcode::I64GeU));
  auto if_expr = MakeUnique<IfExpr>();
  if_expr->true_.exprs.push_back(MakeUnique<UnreachableExpr>());
  exprs.push_back(std::move(if_expr));

  //   ;; Perform the load.
  //   get_local $fulladdr
  //   i32.wrap/i64
  //   i32.load offset=0 align=ALIGN
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(fulladdr)));
  exprs.push_back(MakeUnique<ConvertExpr>(Opcode::I32WrapI64_Opcode));
  exprs.push_back(MakeUnique<LoadExpr>(op, align, 0));

  //   ;; Call the hook.
  //   set_local $val
  //   get_local $addr
  //   i32.const ALIGN
  //   get_local $val
  //   call $hook
  exprs.push_back(MakeUnique<SetLocalExpr>(Var(val)));
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(addr)));
  exprs.push_back(MakeUnique<ConstExpr>(Const::I32(align)));
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(val)));
  exprs.push_back(MakeUnique<CallExpr>(Var(hook_func_index)));

  //   ;; Restore the result.
  //   get_local $val
  exprs.push_back(MakeUnique<GetLocalExpr>(Var(val)));

  module->AppendField(std::move(field));
  return module->funcs.size() - 1;
}

static void HookLoads(Module* module) {
  // Find which kinds of loads exist in the module. The key is a combination of
  // the opcode and its alignment.
  std::map<std::pair<Opcode::Enum, Address>, Index> opcode_aligns;
  ForEachDefinedFunc(module, [&](Func* func, Index func_index) {
    RecurseExprs(&func->exprs, [&](ExprList* exprs, ExprList::iterator iter) {
      auto* expr = dyn_cast<LoadExpr>(&*iter);
      if (expr) {
        opcode_aligns.emplace(std::make_pair(expr->opcode, expr->align),
                              kInvalidIndex);
      }
    });
  });

  // Create the function import hooks.
  std::vector<FuncImportInfo> func_import_infos;
  for (const auto& pair : opcode_aligns) {
    const Opcode op(pair.first.first);
    const Address align = pair.first.second;
    const Type result_type = op.GetResultType();
    const FuncSignature sig({Type::I32, Type::I32, result_type}, {});
    std::string name = op.GetName();
    if (!op.IsNaturallyAligned(align)) {
      name += StringPrintf(" align=%d", align);
    }
    func_import_infos.push_back({names::kModule, name, sig});
  }

  std::vector<Index> indexes = InsertFuncImports(module, func_import_infos);
  assert(indexes.size() == opcode_aligns.size());

  Index last_non_wrapper_func_index = module->funcs.size();

  // Create wrapper functions for each load/alignment combination.
  int i = 0;
  for (auto& pair : opcode_aligns) {
    const Opcode op(pair.first.first);
    pair.second =
        AppendLoadFuncWrapper(module, op, pair.first.second, indexes[i]);
    ++i;
  }

  // Replace all loads with their equivalent wrapper functions.
  ForEachDefinedFunc(module, [&](Func* func, Index func_index) {
    // Don't replace loads in the new functions.
    if (func_index >= last_non_wrapper_func_index) {
      return;
    }

    RecurseExprs(&func->exprs, [&](ExprList* exprs, ExprList::iterator iter) {
      auto* expr = dyn_cast<LoadExpr>(&*iter);
      if (!expr) {
        return;
      }

      auto oa_iter =
          opcode_aligns.find(std::make_pair(expr->opcode, expr->align));
      assert(oa_iter != opcode_aligns.end());
      const Index wrapper_func_index = oa_iter->second;

      // Push the immediate offset, and replace the load with a call to the
      // wrapper function.
      exprs->insert(iter, MakeUnique<ConstExpr>(Const::I32(expr->offset)));
      exprs->insert(iter, MakeUnique<CallExpr>(Var(wrapper_func_index)));
      exprs->erase(iter);
    });
  });
}

int ProgramMain(int argc, char** argv) {
  Result result;

  InitStdio();
  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  result = ReadFile(s_infile.c_str(), &file_data);
  if (Succeeded(result)) {
    Errors errors;
    Module module;
    Features features;
    features.EnableAll();

    const bool kReadDebugNames = true;
    const bool kStopOnFirstError = true;
    const bool kFailOnCustomSectionError = false;
    ReadBinaryOptions options(features, nullptr, kReadDebugNames,
                              kStopOnFirstError, kFailOnCustomSectionError);

    result = ReadBinaryIr(s_infile.c_str(), file_data.data(), file_data.size(),
                          options, &errors, &module);
    if (Succeeded(result)) {
      if (s_hook_functions) {
        HookFuncs(&module);
      }

      if (s_hook_loads) {
        HookLoads(&module);
      }

      result = ValidateModule(&module, &errors, ValidateOptions(features));

      if (Succeeded(result)) {
        WriteBinaryOptions write_binary_options;
        write_binary_options.write_debug_names = true;
        MemoryStream stream;
        result = WriteBinaryModule(&stream, &module, write_binary_options);

        if (Succeeded(result)) {
          if (s_outfile.empty()) {
            s_outfile = DefaultOuputName(s_infile);
          }

          stream.output_buffer().WriteToFile(s_outfile.c_str());
        }
      } else {
        fprintf(
            stderr,
            "Internal wasm-hook error: resulting module doesn't validate.\n");
      }
    }

    FormatErrorsToFile(errors, Location::Type::Binary);
  }
  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}
