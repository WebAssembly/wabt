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
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "src/binary-reader-interpreter.h"
#include "src/binary-reader.h"
#include "src/cast.h"
#include "src/error-handler.h"
#include "src/feature.h"
#include "src/interpreter.h"
#include "src/literal.h"
#include "src/option-parser.h"
#include "src/resolve-names.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-lexer.h"
#include "src/wast-parser.h"

using namespace wabt;
using namespace wabt::interpreter;

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static int s_verbose;
static const char* s_infile;
static Thread::Options s_thread_options;
static bool s_trace;
static bool s_run_all_exports;
static bool s_host_print;
static Features s_features;

static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_stdout_stream;

enum class RunVerbosity {
  Quiet = 0,
  Verbose = 1,
};

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
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-interp", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();
  });
  parser.AddHelpOption();
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
  parser.AddOption('t', "trace", "Trace execution", []() { s_trace = true; });
  parser.AddOption(
      "run-all-exports",
      "Run all the exported functions, in order. Useful for testing",
      []() { s_run_all_exports = true; });
  parser.AddOption("host-print",
                   "Include an importable function named \"host.print\" for "
                   "printing to stdout",
                   []() { s_host_print = true; });

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });
  parser.Parse(argc, argv);
}

// TODO(binji): Share these helper functions w/ spectest-interp as well.

/* Not sure, but 100 chars is probably safe */
#define MAX_TYPED_VALUE_CHARS 100

static void SPrintTypedValue(char* buffer, size_t size, const TypedValue* tv) {
  switch (tv->type) {
    case Type::I32:
      snprintf(buffer, size, "i32:%u", tv->value.i32);
      break;

    case Type::I64:
      snprintf(buffer, size, "i64:%" PRIu64, tv->value.i64);
      break;

    case Type::F32: {
      float value;
      memcpy(&value, &tv->value.f32_bits, sizeof(float));
      snprintf(buffer, size, "f32:%f", value);
      break;
    }

    case Type::F64: {
      double value;
      memcpy(&value, &tv->value.f64_bits, sizeof(double));
      snprintf(buffer, size, "f64:%f", value);
      break;
    }

    default:
      WABT_UNREACHABLE;
  }
}

static void PrintTypedValue(const TypedValue* tv) {
  char buffer[MAX_TYPED_VALUE_CHARS];
  SPrintTypedValue(buffer, sizeof(buffer), tv);
  printf("%s", buffer);
}

static void PrintTypedValueVector(const std::vector<TypedValue>& values) {
  for (size_t i = 0; i < values.size(); ++i) {
    PrintTypedValue(&values[i]);
    if (i != values.size() - 1)
      printf(", ");
  }
}

static void PrintInterpreterResult(const char* desc,
                                   interpreter::Result iresult) {
  printf("%s: %s\n", desc, s_trap_strings[static_cast<size_t>(iresult)]);
}

static void PrintCall(string_view module_name,
                      string_view func_name,
                      const std::vector<TypedValue>& args,
                      const std::vector<TypedValue>& results,
                      interpreter::Result iresult) {
  if (!module_name.empty())
    printf(PRIstringview ".", WABT_PRINTF_STRING_VIEW_ARG(module_name));
  printf(PRIstringview "(", WABT_PRINTF_STRING_VIEW_ARG(func_name));
  PrintTypedValueVector(args);
  printf(") =>");
  if (iresult == interpreter::Result::Ok) {
    if (results.size() > 0) {
      printf(" ");
      PrintTypedValueVector(results);
    }
    printf("\n");
  } else {
    PrintInterpreterResult(" error", iresult);
  }
}

static interpreter::Result RunFunction(Thread* thread,
                                       Index func_index,
                                       const std::vector<TypedValue>& args,
                                       std::vector<TypedValue>* out_results) {
  return s_trace ? thread->TraceFunction(func_index, s_stdout_stream.get(),
                                         args, out_results)
                 : thread->RunFunction(func_index, args, out_results);
}

static interpreter::Result RunStartFunction(Thread* thread,
                                            DefinedModule* module) {
  if (module->start_func_index == kInvalidIndex)
    return interpreter::Result::Ok;

  if (s_trace)
    printf(">>> running start function:\n");
  std::vector<TypedValue> args;
  std::vector<TypedValue> results;
  interpreter::Result iresult =
      RunFunction(thread, module->start_func_index, args, &results);
  assert(results.size() == 0);
  return iresult;
}

static interpreter::Result RunExport(Thread* thread,
                                     const interpreter::Export* export_,
                                     const std::vector<TypedValue>& args,
                                     std::vector<TypedValue>* out_results) {
  if (s_trace) {
    printf(">>> running export \"" PRIstringview "\":\n",
           WABT_PRINTF_STRING_VIEW_ARG(export_->name));
  }

  assert(export_->kind == ExternalKind::Func);
  return RunFunction(thread, export_->index, args, out_results);
}

static void RunAllExports(interpreter::Module* module,
                          Thread* thread,
                          RunVerbosity verbose) {
  std::vector<TypedValue> args;
  std::vector<TypedValue> results;
  for (const interpreter::Export& export_ : module->exports) {
    interpreter::Result iresult = RunExport(thread, &export_, args, &results);
    if (verbose == RunVerbosity::Verbose) {
      PrintCall(string_view(), export_.name, args, results, iresult);
    }
  }
}

static wabt::Result ReadModule(const char* module_filename,
                               Environment* env,
                               ErrorHandler* error_handler,
                               DefinedModule** out_module) {
  wabt::Result result;
  std::vector<uint8_t> file_data;

  *out_module = nullptr;

  result = ReadFile(module_filename, &file_data);
  if (Succeeded(result)) {
    const bool kReadDebugNames = true;
    const bool kStopOnFirstError = true;
    ReadBinaryOptions options(s_features, s_log_stream.get(), kReadDebugNames,
                              kStopOnFirstError);
    result = ReadBinaryInterpreter(env, DataOrNull(file_data), file_data.size(),
                                   &options, error_handler, out_module);

    if (Succeeded(result)) {
      if (s_verbose)
        env->DisassembleModule(s_stdout_stream.get(), *out_module);
    }
  }
  return result;
}

#define PRIimport "\"" PRIstringview "." PRIstringview "\""
#define PRINTF_IMPORT_ARG(x)                    \
  WABT_PRINTF_STRING_VIEW_ARG((x).module_name) \
  , WABT_PRINTF_STRING_VIEW_ARG((x).field_name)

class WasmInterpHostImportDelegate : public HostImportDelegate {
 public:
  wabt::Result ImportFunc(interpreter::FuncImport* import,
                          interpreter::Func* func,
                          interpreter::FuncSignature* func_sig,
                          const ErrorCallback& callback) override {
    if (import->field_name == "print") {
      cast<HostFunc>(func)->callback = PrintCallback;
      return wabt::Result::Ok;
    } else {
      PrintError(callback, "unknown host function import " PRIimport,
                 PRINTF_IMPORT_ARG(*import));
      return wabt::Result::Error;
    }
  }

  wabt::Result ImportTable(interpreter::TableImport* import,
                           interpreter::Table* table,
                           const ErrorCallback& callback) override {
    return wabt::Result::Error;
  }

  wabt::Result ImportMemory(interpreter::MemoryImport* import,
                            interpreter::Memory* memory,
                            const ErrorCallback& callback) override {
    return wabt::Result::Error;
  }

  wabt::Result ImportGlobal(interpreter::GlobalImport* import,
                            interpreter::Global* global,
                            const ErrorCallback& callback) override {
    return wabt::Result::Error;
  }

 private:
  static interpreter::Result PrintCallback(
      const HostFunc* func,
      const interpreter::FuncSignature* sig,
      Index num_args,
      TypedValue* args,
      Index num_results,
      TypedValue* out_results,
      void* user_data) {
    memset(out_results, 0, sizeof(TypedValue) * num_results);
    for (Index i = 0; i < num_results; ++i)
      out_results[i].type = sig->result_types[i];

    std::vector<TypedValue> vec_args(args, args + num_args);
    std::vector<TypedValue> vec_results(out_results, out_results + num_results);

    printf("called host ");
    PrintCall(func->module_name, func->field_name, vec_args, vec_results,
              interpreter::Result::Ok);
    return interpreter::Result::Ok;
  }

  void PrintError(const ErrorCallback& callback, const char* format, ...) {
    WABT_SNPRINTF_ALLOCA(buffer, length, format);
    callback(buffer);
  }
};

static void InitEnvironment(Environment* env) {
  if (s_host_print) {
    HostModule* host_module = env->AppendHostModule("host");
    host_module->import_delegate.reset(new WasmInterpHostImportDelegate());
  }
}

static wabt::Result ReadAndRunModule(const char* module_filename) {
  wabt::Result result;
  Environment env;
  InitEnvironment(&env);

  ErrorHandlerFile error_handler(Location::Type::Binary);
  DefinedModule* module = nullptr;
  result = ReadModule(module_filename, &env, &error_handler, &module);
  if (Succeeded(result)) {
    Thread thread(&env, s_thread_options);
    interpreter::Result iresult = RunStartFunction(&thread, module);
    if (iresult == interpreter::Result::Ok) {
      if (s_run_all_exports)
        RunAllExports(module, &thread, RunVerbosity::Verbose);
    } else {
      PrintInterpreterResult("error running start function", iresult);
    }
  }
  return result;
}

int ProgramMain(int argc, char** argv) {
  InitStdio();
  ParseOptions(argc, argv);

  s_stdout_stream = FileStream::CreateStdout();

  wabt::Result result = ReadAndRunModule(s_infile);
  return result != wabt::Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}
