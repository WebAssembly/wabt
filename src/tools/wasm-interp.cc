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
#include <unordered_map>
#include <vector>

#include "wabt/binary-reader.h"
#include "wabt/error-formatter.h"
#include "wabt/feature.h"
#include "wabt/filenames.h"
#include "wabt/interp/binary-reader-interp.h"
#include "wabt/interp/interp-util.h"
#include "wabt/interp/interp-wasi.h"
#include "wabt/interp/interp.h"
#include "wabt/option-parser.h"
#include "wabt/stream.h"

#ifdef WITH_WASI
#include "uvwasi.h"
#endif

using namespace wabt;
using namespace wabt::interp;

using ExportMap = std::unordered_map<std::string, Extern::Ptr>;
using Registry = std::unordered_map<std::string, ExportMap>;

static int s_verbose;
static const char* s_infile;
static Thread::Options s_thread_options;
static Stream* s_trace_stream;
static bool s_run_all_exports;
static bool s_continue_wasi_argv = false;
static bool s_host_print;
static bool s_dummy_import_func;
static Features s_features;
static bool s_wasi;
static std::vector<std::string> s_modules;
static std::vector<std::string> s_wasi_env;
static std::vector<std::string> s_wasi_argv;
static std::vector<std::string> s_wasi_dirs;

static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_stdout_stream;
static std::unique_ptr<FileStream> s_stderr_stream;

static Registry s_registry;

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
)";

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
      'm', "module", "[NAME:]FILE",
      "load module FILE and provide all exports under NAME for upcoming "
      "imports. if NAME is empty, the debug-name from the name section "
      "will be used if present, the filename elsewise.",
      [](const std::string& argument) { s_modules.push_back(argument); });
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
      ". arg", OptionParser::ArgumentCount::ZeroOrMore,
      [](const char* argument) { s_wasi_argv.push_back(argument); });
  parser.Parse(argc, argv);
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

static bool IsHostPrint(const ImportDesc& import) {
  return import.type.type->kind == ExternKind::Func &&
         (s_host_print && import.type.module == "host" &&
          import.type.name == "print");
}

#if WITH_WASI
static bool IsWasiImport(const ImportDesc& import) {
  return import.type.type->kind == ExternKind::Func &&
         (import.type.module == "wasi_snapshot_preview1" ||
          import.type.module == "wasi_unstable");
}

static bool HasWasiImport(const Module::Ptr module) {
  for (auto&& import : module->desc().imports) {
    if (IsWasiImport(import)) {
      return true;
    }
  }
  return false;
}
#endif

static Ref GenerateHostPrint(const ImportDesc& import) {
  auto* stream = s_stdout_stream.get();

  auto func_type = *cast<FuncType>(import.type.type.get());
  auto import_name = StringPrintf("%s.%s", import.type.module.c_str(),
                                  import.type.name.c_str());

  auto host_func = HostFunc::New(
      s_store, func_type,
      [=](Thread& thread, const Values& params, Values& results,
          Trap::Ptr* trap) -> Result {
        WriteCall(stream, import_name, func_type, params, results, *trap);
        return Result::Ok;
      });

  return host_func.ref();
}

static Extern::Ptr GetImport(const std::string& module,
                             const std::string& name) {
  auto mod_iter = s_registry.find(module);
  if (mod_iter != s_registry.end()) {
    auto extern_iter = mod_iter->second.find(name);
    if (extern_iter != mod_iter->second.end()) {
      return extern_iter->second;
    }
  }
  return {};
}

static void PopulateExports(const Instance::Ptr& instance,
                            Module::Ptr& module,
                            ExportMap& map) {
  map.clear();
  // Module::Ptr module{s_store, instance->module()};
  for (size_t i = 0; i < module->export_types().size(); ++i) {
    const ExportType& export_type = module->export_types()[i];
    auto export_ = s_store.UnsafeGet<Extern>(instance->exports()[i]);
    map[export_type.name] = export_;
  }
}

static std::string GetPathName(std::string module_arg) {
  size_t path_at = module_arg.find_first_of(':');

  path_at = path_at == std::string::npos ? 0 : path_at + 1;

  return module_arg.substr(path_at);
}

static std::string GetDebugName(const Module::Ptr& module) {
  // TODO: query debug_name
  return "";
}

static std::string GetRegistryName(std::string module_arg,
                                   const Module::Ptr& module) {
  size_t split_pos = module_arg.find_first_of(':');
  if (split_pos == std::string::npos) {
    split_pos = 0;
  }
  std::string override_name = module_arg.substr(0, split_pos);
  std::string path_name =
      module_arg.substr(split_pos ? split_pos + 1 : split_pos);
  std::string debug_name = GetDebugName(module);

  // use override_name if present
  if (!override_name.empty()) {
    return override_name;
  }

  // prefer debug_name
  if (!debug_name.empty()) {
    return debug_name;
  }

  // fall back to file-name
  return std::string(StripExtension(GetBasename(path_name)));
}

static void BindImports(const Module::Ptr& module, RefVec& imports) {
  for (auto&& import : module->desc().imports) {
    if (IsHostPrint(import)) {
      imports.push_back(GenerateHostPrint(import));
      continue;
    }

#if WITH_WASI
    // If there are wasi imports, we need to make a new instance for it
    if (IsWasiImport(import)) {
      imports.push_back(WasiGetImport(module, import));
      continue;
    }
#endif

    // By default, just push an null reference. This won't resolve, and
    // instantiation will fail.

    Extern::Ptr extern_ = GetImport(import.type.module, import.type.name);
    if (extern_) {
      imports.push_back(extern_.ref());
      continue;
    }

    // only populate missing imports for dummy-import-func
    imports.push_back(s_dummy_import_func ? GenerateHostPrint(import)
                                          : Ref::Null);
  }
}

static Result ReadModule(const char* module_filename,
                         Errors* errors,
                         Module::Ptr* out_module) {
  auto* stream = s_stdout_stream.get();
  std::vector<uint8_t> file_data;
  CHECK_RESULT(ReadFile(module_filename, &file_data));

  ModuleDesc module_desc;
  const bool kReadDebugNames = true;
  const bool kStopOnFirstError = true;
  const bool kFailOnCustomSectionError = true;
  ReadBinaryOptions options(s_features, s_log_stream.get(), kReadDebugNames,
                            kStopOnFirstError, kFailOnCustomSectionError);
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

static Result InitWasi(uvwasi_t* uvwasi) {
  uvwasi_errno_t err;
  uvwasi_options_t init_options;

  std::vector<const char*> argv;
  argv.push_back(s_infile);
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

  err = uvwasi_init(uvwasi, &init_options);
  if (err != UVWASI_ESUCCESS) {
    s_stderr_stream.get()->Writef("error initialiazing uvwasi: %d\n", err);
    return Result::Error;
  }
  return Result::Ok;
}

static Result ReadAndRunModule(const char* module_filename) {
  Errors errors;
  Result result;
#if WITH_WASI
  uvwasi_t uvwasi;
#endif

  if (s_wasi) {
#if WITH_WASI
    result = InitWasi(&uvwasi);
    if (result == Result::Error) {
      return result;
    }
#else
    s_stderr_stream.get()->Writef("wasi support not compiled in\n");
    return Result::Error;
#endif
  }

  // First PopulateExports
  // then reference all BindImports
  // if we only have dummy imports, we wont need to reghister anything
  // but we still want to load them.
  s_modules.push_back(s_infile);
  std::vector<Module::Ptr> modules_loaded(s_modules.size());
  std::vector<Instance::Ptr> instance_loaded(s_modules.size());
  for (auto import_module : s_modules) {
    ExportMap load_map;
    std::string module_path = GetPathName(import_module);

    Module::Ptr load_module;
    result = ReadModule(module_path.c_str(), &errors, &load_module);
    if (!Succeeded(result)) {
      FormatErrorsToFile(errors, Location::Type::Binary);
      return result;
    }

    RefVec load_imports;
    BindImports(load_module, load_imports);

    Instance::Ptr load_instance;
    CHECK_RESULT(InstantiateModule(load_imports, load_module, &load_instance));

    instance_loaded.push_back(load_instance);

#if WITH_WASI
    if (HasWasiImport(load_module)) {
      WasiRegisterInstance(load_instance, &uvwasi, s_stderr_stream.get(),
                           s_trace_stream);
    }
#endif

    std::string reg_name = GetRegistryName(import_module, load_module);
    PopulateExports(load_instance, load_module, load_map);
    s_registry[reg_name] = std::move(load_map);

    modules_loaded.push_back(load_module);
  }

  if (s_run_all_exports) {
    RunAllExports(instance_loaded.back(), &errors);
  }
#ifdef WITH_WASI
  if (s_wasi) {
    CHECK_RESULT(WasiRunStart(instance_loaded.back(), &uvwasi,
                              s_stderr_stream.get(), s_trace_stream));
  }
#endif
  // unregister all;
  for (auto& instance : instance_loaded) {
    WasiUnregisterInstance(instance);
  }

  return Result::Ok;
}

int ProgramMain(int argc, char** argv) {
  InitStdio();
  s_stdout_stream = FileStream::CreateStdout();
  s_stderr_stream = FileStream::CreateStderr();

  ParseOptions(argc, argv);
  s_store.setFeatures(s_features);

  wabt::Result result = ReadAndRunModule(s_infile);

  s_registry.clear();

  return result != wabt::Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}
