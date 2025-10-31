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
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "wabt/binary-reader.h"
#include "wabt/error-formatter.h"
#include "wabt/feature.h"
#include "wabt/interp/binary-reader-interp.h"
#include "wabt/interp/interp-util.h"
#include "wabt/interp/interp-wasi.h"
#include "wabt/interp/interp.h"
#include "wabt/literal.h"
#include "wabt/option-parser.h"
#include "wabt/stream.h"

#ifdef WITH_WASI
#include "uvwasi.h"
#endif

using namespace wabt;
using namespace wabt::interp;

struct FunctionCall {
  std::string name;
  Values args;
};

#define ERROR_EXIT_UNLESS(cond, ...) \
  do {                               \
    if (!(cond)) {                   \
      printf(__VA_ARGS__);           \
      exit(1);                       \
    }                                \
  } while (0);

static int s_verbose;
static const char* s_infile;
static Thread::Options s_thread_options;
static Stream* s_trace_stream;
static bool s_run_all_exports;
static bool s_host_print;
static bool s_dummy_import_func;
static Features s_features;
static bool s_wasi;
static std::vector<FunctionCall> s_run_exports;
static std::vector<std::string> s_wasi_env;
static std::vector<std::string> s_wasi_argv;
static std::vector<std::string> s_wasi_dirs;

static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_stdout_stream;
static std::unique_ptr<FileStream> s_stderr_stream;

static Store s_store;

static const char s_description[] =
    R"(  read a file in the wasm binary format, and run in it a stack-based
  interpreter.

examples:
  # parse binary file test.wasm, and type-check it
  $ wasm-interp test.wasm

  # parse test.wasm and run all its exported functions
  $ wasm-interp test.wasm --run-all-exports

  # parse test.wasm, run the exported functions and trace the output
  $ wasm-interp test.wasm --run-all-exports --trace

  # parse test.wasm and run all its exported functions, setting the
  # value stack size to 100 elements
  $ wasm-interp test.wasm -V 100 --run-all-exports

  # parse test.wasm, run specific exported function by name with argument
  $ wasm-interp test.wasm -r "func_sum" -a "i32:8" -a "i32:5"
)";

Result ParseWasmValue(std::string argument, Value& val) {
  Result result = Result::Ok;

  size_t cindex;
  if (argument.empty() || (cindex = argument.find(':')) == std::string::npos) {
    return wabt::Result::Error;
  }

  argument[cindex] = '\0';
  const char* ptype = argument.c_str();
  const char* pval = argument.c_str() + cindex + 1;
  const char* pval_end = ptype + argument.length();

  if (strcmp(ptype, "i32") == 0) {
    uint32_t parsed_value;
    result |=
        ParseInt32(pval, pval_end, &parsed_value, ParseIntType::UnsignedOnly);
    val.Set(parsed_value);
  }
  if (strcmp(ptype, "i64") == 0) {
    uint64_t parsed_value;
    result |=
        ParseInt64(pval, pval_end, &parsed_value, ParseIntType::UnsignedOnly);
    val.Set(parsed_value);
  }
  if (strcmp(ptype, "f32") == 0) {
    uint32_t parsed_value;
    result |= ParseFloat(LiteralType::Float, pval, pval_end, &parsed_value);
    val.Set(parsed_value);
  }
  if (strcmp(ptype, "f64") == 0) {
    uint64_t parsed_value;
    result |= ParseDouble(LiteralType::Float, pval, pval_end, &parsed_value);
    val.Set(parsed_value);
  }
  return result;
}

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-interp", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStderr();
  });
  s_features.AddOptions(&parser);
  parser.AddOption('V', "value-stack-size", "SIZE",
                   "Size in elements of the value stack",
                   [](const std::string& argument) {
                     // TODO(binji): validate.
                     s_thread_options.value_stack_size = atoi(argument.c_str());
                   });
  parser.AddOption('C', "call-stack-size", "SIZE",
                   "Size in elements of the call stack",
                   [](const std::string& argument) {
                     // TODO(binji): validate.
                     s_thread_options.call_stack_size = atoi(argument.c_str());
                   });
  parser.AddOption('t', "trace", "Trace execution",
                   []() { s_trace_stream = s_stdout_stream.get(); });
  parser.AddOption('r', "run-export", "FUNCTION",
                   "Run exported function by name",
                   [](const std::string& argument) {
                     FunctionCall func;
                     func.name = argument;
                     s_run_exports.push_back(func);
                   });
  parser.AddOption('a', "argument", "ARGUMENT",
                   "Add argument to an exported function execution",
                   [](const std::string& argument) {
                     Value val;
                     ERROR_EXIT_UNLESS(
                         !s_run_exports.empty(),
                         "Cannot find a function execution for argument '%s'\n",
                         argument.c_str());
                     ERROR_EXIT_UNLESS(Succeeded(ParseWasmValue(argument, val)),
                                       "Failed to parse argument '%s'\n",
                                       argument.c_str());
                     s_run_exports.back().args.push_back(val);
                   });
  parser.AddOption("wasi",
                   "Assume input module is WASI compliant (Export "
                   " WASI API the the module and invoke _start function)",
                   []() { s_wasi = true; });
  parser.AddOption(
      'e', "env", "ENV",
      "Pass the given environment string in the WASI runtime",
      [](const std::string& argument) { s_wasi_env.push_back(argument); });
  parser.AddOption(
      'd', "dir", "DIR", "Pass the given directory the the WASI runtime",
      [](const std::string& argument) { s_wasi_dirs.push_back(argument); });
  parser.AddOption(
      "run-all-exports",
      "Run all the exported functions, in order. Useful for testing",
      []() { s_run_all_exports = true; });
  parser.AddOption("host-print",
                   "Include an importable function named \"host.print\" for "
                   "printing to stdout",
                   []() { s_host_print = true; });
  parser.AddOption(
      "dummy-import-func",
      "Provide a dummy implementation of all imported functions. The function "
      "will log the call and return an appropriate zero value.",
      []() { s_dummy_import_func = true; });

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });
  parser.AddArgument(
      "arg", OptionParser::ArgumentCount::ZeroOrMore,
      [](const char* argument) { s_wasi_argv.push_back(argument); });
  parser.Parse(argc, argv);
}

Result RunSpecificExports(const Instance::Ptr& instance,
                          Errors* errors,
                          std::vector<FunctionCall>& calls) {
  Result result = Result::Ok;

  auto module = s_store.UnsafeGet<Module>(instance->module());
  auto&& module_desc = module->desc();

  for (auto&& export_ : module_desc.exports) {
    for (auto& call_ : calls) {
      if (export_.type.name == call_.name) {
        ERROR_EXIT_UNLESS(export_.type.type->kind == ExternalKind::Func,
                          "Export '%s' is not a function\n",
                          export_.type.name.c_str());

        if (s_trace_stream) {
          s_trace_stream->Writef(">>> running export \"%s\":\n",
                                 call_.name.c_str());
        }
        auto* func_type = cast<FuncType>(export_.type.type.get());
        int params_size = func_type->params.size();
        int args_size = call_.args.size();
        ERROR_EXIT_UNLESS(params_size == args_size,
                          "Exported function '%s' expects %d arguments, but %d "
                          "were provided\n",
                          export_.type.name.c_str(), params_size, args_size);

        auto func = s_store.UnsafeGet<Func>(instance->funcs()[export_.index]);
        Values results;
        Trap::Ptr trap;
        result |=
            func->Call(s_store, call_.args, results, &trap, s_trace_stream);
        WriteCall(s_stdout_stream.get(), export_.type.name, *func_type,
                  call_.args, results, trap);
      }
    }
  }

  return result;
}

Result RunAllExports(const Instance::Ptr& instance, Errors* errors) {
  Result result = Result::Ok;

  auto module = s_store.UnsafeGet<Module>(instance->module());
  auto&& module_desc = module->desc();

  for (auto&& export_ : module_desc.exports) {
    if (export_.type.type->kind != ExternalKind::Func) {
      continue;
    }
    auto* func_type = cast<FuncType>(export_.type.type.get());
    if (func_type->params.empty()) {
      if (s_trace_stream) {
        s_trace_stream->Writef(">>> running export \"%s\":\n",
                               export_.type.name.c_str());
      }
      auto func = s_store.UnsafeGet<Func>(instance->funcs()[export_.index]);
      Values params;
      Values results;
      Trap::Ptr trap;
      result |= func->Call(s_store, params, results, &trap, s_trace_stream);
      WriteCall(s_stdout_stream.get(), export_.type.name, *func_type, params,
                results, trap);
    }
  }

  return result;
}

static void BindImports(const Module::Ptr& module, RefVec& imports) {
  auto* stream = s_stdout_stream.get();

  for (auto&& import : module->desc().imports) {
    if (import.type.type->kind == ExternKind::Func &&
        ((s_host_print && import.type.module == "host" &&
          import.type.name == "print") ||
         s_dummy_import_func)) {
      auto func_type = *cast<FuncType>(import.type.type.get());
      auto import_name = StringPrintf("%s.%s", import.type.module.c_str(),
                                      import.type.name.c_str());

      auto host_func = HostFunc::New(
          s_store, func_type,
          [=](Thread& thread, const Values& params, Values& results,
              Trap::Ptr* trap) -> Result {
            printf("called host ");
            WriteCall(stream, import_name, func_type, params, results, *trap);
            return Result::Ok;
          });
      imports.push_back(host_func.ref());
      continue;
    }

    // By default, just push an null reference. This won't resolve, and
    // instantiation will fail.
    imports.push_back(Ref::Null);
  }
}

static Result ReadModule(const char* module_filename,
                         Errors* errors,
                         Module::Ptr* out_module) {
  auto* stream = s_stdout_stream.get();
  std::vector<uint8_t> file_data;
  CHECK_RESULT(ReadFile(module_filename, &file_data));

  ModuleDesc module_desc;
  ReadBinaryInterpOptions options(s_features, s_log_stream.get());
  CHECK_RESULT(ReadBinaryInterp(module_filename, file_data.data(),
                                file_data.size(), options, errors,
                                &module_desc));

  if (s_verbose) {
    module_desc.istream.Disassemble(stream);
  }

  *out_module = Module::New(s_store, module_desc);
  return Result::Ok;
}

static Result InstantiateModule(RefVec& imports,
                                const Module::Ptr& module,
                                Instance::Ptr* out_instance) {
  RefPtr<Trap> trap;
  *out_instance = Instance::Instantiate(s_store, module.ref(), imports, &trap);
  if (!*out_instance) {
    WriteTrap(s_stderr_stream.get(), "error initializing module", trap);
    return Result::Error;
  }
  return Result::Ok;
}

static Result ReadAndRunModule(const char* module_filename) {
  Errors errors;
  Module::Ptr module;
  Result result = ReadModule(module_filename, &errors, &module);
  if (!Succeeded(result)) {
    FormatErrorsToFile(errors, Location::Type::Binary);
    return result;
  }

  RefVec imports;

#if WITH_WASI
  uvwasi_t uvwasi;
#endif

  if (s_wasi) {
#if WITH_WASI
    uvwasi_errno_t err;
    uvwasi_options_t init_options;

    std::vector<const char*> argv;
    argv.push_back(module_filename);
    for (auto& s : s_wasi_argv) {
      if (s_trace_stream) {
        s_trace_stream->Writef("wasi: arg: \"%s\"\n", s.c_str());
      }
      argv.push_back(s.c_str());
    }
    argv.push_back(nullptr);

    std::vector<const char*> envp;
    for (auto& s : s_wasi_env) {
      if (s_trace_stream) {
        s_trace_stream->Writef("wasi: env: \"%s\"\n", s.c_str());
      }
      envp.push_back(s.c_str());
    }
    envp.push_back(nullptr);

    std::vector<uvwasi_preopen_t> dirs;
    for (auto& dir : s_wasi_dirs) {
      if (s_trace_stream) {
        s_trace_stream->Writef("wasi: dir: \"%s\"\n", dir.c_str());
      }
      dirs.push_back({dir.c_str(), dir.c_str()});
    }

    /* Setup the initialization options. */
    init_options.in = 0;
    init_options.out = 1;
    init_options.err = 2;
    init_options.fd_table_size = 3;
    init_options.argc = argv.size() - 1;
    init_options.argv = argv.data();
    init_options.envp = envp.data();
    init_options.preopenc = dirs.size();
    init_options.preopens = dirs.data();
    init_options.allocator = NULL;

    err = uvwasi_init(&uvwasi, &init_options);
    if (err != UVWASI_ESUCCESS) {
      s_stderr_stream.get()->Writef("error initialiazing uvwasi: %d\n", err);
      return Result::Error;
    }
    CHECK_RESULT(WasiBindImports(module, imports, s_stderr_stream.get(),
                                 s_trace_stream));
#else
    s_stderr_stream.get()->Writef("wasi support not compiled in\n");
    return Result::Error;
#endif
  } else {
    BindImports(module, imports);
  }

  Instance::Ptr instance;
  CHECK_RESULT(InstantiateModule(imports, module, &instance));

  if (s_run_all_exports) {
    RunAllExports(instance, &errors);
  }

  if (!s_run_exports.empty()) {
    RunSpecificExports(instance, &errors, s_run_exports);
  }
#ifdef WITH_WASI
  if (s_wasi) {
    CHECK_RESULT(
        WasiRunStart(instance, &uvwasi, s_stderr_stream.get(), s_trace_stream));
  }
#endif

  return Result::Ok;
}

int ProgramMain(int argc, char** argv) {
  InitStdio();
  s_stdout_stream = FileStream::CreateStdout();
  s_stderr_stream = FileStream::CreateStderr();

  ParseOptions(argc, argv);
  s_store.setFeatures(s_features);

  wabt::Result result = ReadAndRunModule(s_infile);
  return result != wabt::Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}
