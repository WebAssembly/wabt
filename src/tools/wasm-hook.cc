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

static std::string s_infile;
static std::string s_outfile;
static bool s_hook_functions = false;

static const char s_description[] =
R"(  Instrument a WebAssembly module and add debugging hooks.

examples:
  # Add debugging hooks for all function calls.
  $ wasm-hook -f test.wasm -o test.hook.wasm
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-hook", s_description);

  parser.AddHelpOption();
  parser.AddOption('o', "output", "FILE", "output wasm binary file",
                   [](const char* argument) {
                     s_outfile = argument;
                     ConvertBackslashToSlash(&s_infile);
                   });
  parser.AddOption('f', "function", "Add hook for all function calls",
                   []() { s_hook_functions = true; });
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

struct FuncImportInfo {
  std::string module;
  std::string field;
  FuncSignature sig;
};

static std::vector<Index> AppendFuncImports(
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

    Index func_type_index = module->GetFuncTypeIndex(func_import_info.sig);
    if (func_type_index == kInvalidIndex) {
      auto func_type_field = MakeUnique<FuncTypeModuleField>();
      func_type_field->func_type.sig = func_import_info.sig;

      module->AppendField(std::move(func_type_field));
      func_type_index = module->func_types.size() - 1;
    }

    func_import->func.decl.has_func_type = true;
    func_import->func.decl.type_var.set_index(func_type_index);
    func_import->func.decl.sig = func_import_info.sig;

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
void RecurseExprs(ExprList* exprs, F&& func) {
  for (auto iter = exprs->begin(), end = exprs->end(); iter != end; ++iter) {
    Expr& expr = *iter;

    func(exprs, iter);

    switch (expr.type()) {
      case ExprType::Block:
        RecurseExprs(&cast<BlockExpr>(&expr)->block.exprs, func);
        break;

      case ExprType::If:
        RecurseExprs(&cast<IfExpr>(&expr)->true_.exprs, func);
        RecurseExprs(&cast<IfExpr>(&expr)->false_, func);
        break;

      case ExprType::IfExcept:
        RecurseExprs(&cast<IfExceptExpr>(&expr)->true_.exprs, func);
        RecurseExprs(&cast<IfExceptExpr>(&expr)->false_, func);
        break;

      case ExprType::Try:
        RecurseExprs(&cast<TryExpr>(&expr)->block.exprs, func);
        break;

      case ExprType::Loop:
        RecurseExprs(&cast<LoopExpr>(&expr)->block.exprs, func);
        break;

      default:
        break;
    }
  }
}

static void HookFuncs(Module* module,
                      Index begin_func_index,
                      Index end_func_index) {
  Index func_index = 0;
  for (Func* func : module->funcs) {
    if (func_index >= module->num_func_imports) {
      auto block = MakeUnique<BlockExpr>();
      block->block.decl.sig.result_types = func->decl.sig.result_types;
      block->block.exprs = std::move(func->exprs);

      // Insert endFunc hook before all return instructions.
      RecurseExprs(
          &block->block.exprs, [=](ExprList* exprs, ExprList::iterator iter) {
            auto* expr = dyn_cast<ReturnExpr>(&*iter);
            if (!expr) {
              return;
            }

            exprs->insert(iter, MakeUnique<ConstExpr>(Const::I32(func_index)));
            exprs->insert(iter, MakeUnique<CallExpr>(Var(end_func_index)));
          });

      func->exprs.push_back(MakeUnique<ConstExpr>(Const::I32(func_index)));
      func->exprs.push_back(MakeUnique<CallExpr>(Var(begin_func_index)));
      func->exprs.push_back(std::move(block));
      func->exprs.push_back(MakeUnique<ConstExpr>(Const::I32(func_index)));
      func->exprs.push_back(MakeUnique<CallExpr>(Var(end_func_index)));
    }

    ++func_index;
  }
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
        FuncSignature sig_i_v({Type::I32}, {});
        std::vector<Index> indexes =
            AppendFuncImports(&module, {{"wasm-hook", "beginFunc", sig_i_v},
                                        {"wasm-hook", "endFunc", sig_i_v}});
        assert(indexes.size() == 2);

        HookFuncs(&module, indexes[0], indexes[1]);
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
