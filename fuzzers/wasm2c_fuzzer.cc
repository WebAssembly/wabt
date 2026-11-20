// Copyright 2026 WebAssembly Community Group participants
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "wabt/apply-names.h"
#include "wabt/binary-reader-ir.h"
#include "wabt/binary-reader.h"
#include "wabt/c-writer.h"
#include "wabt/expr-visitor.h"
#include "wabt/feature.h"
#include "wabt/generate-names.h"
#include "wabt/ir.h"
#include "wabt/stream.h"
#include "wabt/validator.h"

// The features the wasm2c tool accepts; see supported_features in
// src/tools/wasm2c.cc.
static bool IsWasm2cFeature(const char* flag) {
  static const char* const kSupported[] = {
      "multi-memory",      "multi-value",
      "sign-extension",    "saturating-float-to-int",
      "exceptions",        "memory64",
      "extended-const",    "simd",
      "threads",           "tail-call",
      "custom-page-sizes", "compact-imports"};
  for (const char* supported : kSupported) {
    if (strcmp(flag, supported) == 0) {
      return true;
    }
  }
  return false;
}

// wasm2c aborts with "unimplemented" on these, so skip modules that use them.
// The other unimplemented exprs (call_ref, return_call_ref) need
// function-references, which wasm2c doesn't support.
class UnimplementedExprFinder : public wabt::ExprVisitor::DelegateNop {
 public:
  wabt::Result OnAtomicWaitExpr(wabt::AtomicWaitExpr*) override {
    return wabt::Result::Error;
  }
  wabt::Result OnAtomicNotifyExpr(wabt::AtomicNotifyExpr*) override {
    return wabt::Result::Error;
  }
};

static bool UsesUnimplementedExpr(wabt::Module& module) {
  UnimplementedExprFinder finder;
  wabt::ExprVisitor visitor(&finder);
  for (wabt::Func* func : module.funcs) {
    if (wabt::Failed(visitor.VisitFunc(func))) {
      return true;
    }
  }
  return false;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // The first two bytes pick the options and the rest is the module, so a
  // seed is just two bytes followed by a .wasm file.
  if (size < 2) {
    return 0;
  }
  const unsigned config = data[0] | (data[1] << 8);
  wabt::ByteSpan wasm(data + 2, size - 2);

  // Most of these are on by default, so each bit turns its feature on or off
  // rather than only enabling it.
  wabt::WriteCOptions write_c_options;
  unsigned bit = 0;
#define WABT_FEATURE(variable, flag, default_, help)                         \
  if (IsWasm2cFeature(flag)) {                                               \
    write_c_options.features.set_##variable##_enabled(config & (1u << bit)); \
    bit++;                                                                   \
  }
#include "wabt/feature.def"
#undef WABT_FEATURE
  // relaxed-simd is on by default but the writer exits on its instructions.
  write_c_options.features.disable_relaxed_simd();
  const bool read_debug_names = config & (1u << 12);
  const size_t num_outputs = 1 + ((config >> 13) & 3);

  // Same pipeline as the wasm2c tool.
  wabt::Errors errors;
  wabt::Module module;
  wabt::ReadBinaryOptions options(write_c_options.features, nullptr,
                                  read_debug_names,
                                  /*stop_on_first_error=*/true,
                                  /*fail_on_custom_section_error=*/true);
  if (wabt::Failed(
          wabt::ReadBinaryIr("<fuzzer>", wasm, options, &errors, &module)) ||
      wabt::Failed(
          wabt::ValidateModule(&module, &errors, write_c_options.features)) ||
      UsesUnimplementedExpr(module) ||
      wabt::Failed(wabt::GenerateNames(&module))) {
    return 0;
  }
  (void)wabt::ApplyNames(&module);

  std::string module_name = module.name.empty() ? "fuzz" : module.name;
  write_c_options.module_name = module_name;

  std::vector<wabt::MemoryStream> c_streams(num_outputs);
  std::vector<wabt::Stream*> c_stream_ptrs;
  for (auto& stream : c_streams) {
    c_stream_ptrs.push_back(&stream);
  }
  wabt::MemoryStream h_stream;
  wabt::MemoryStream h_impl_stream;
  if (num_outputs == 1) {
    // A single output keeps the implementation header in the .c file.
    (void)wabt::WriteC(std::move(c_stream_ptrs), &h_stream, &c_streams[0],
                       "fuzz.h", "", &module, write_c_options);
  } else {
    (void)wabt::WriteC(std::move(c_stream_ptrs), &h_stream, &h_impl_stream,
                       "fuzz.h", "fuzz-impl.h", &module, write_c_options);
  }
  return 0;
}
