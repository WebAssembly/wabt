// Copyright 2019 Google LLC
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
//
// This file is copied from the oss-fuzz project:
//
// https://github.com/google/oss-fuzz/blob/master/projects/wabt/wasm2wat_fuzzer.cc

#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/common.h"
#include "src/ir.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  wabt::ReadBinaryOptions options;
  wabt::Errors errors;
  wabt::Module module;
  wabt::ReadBinaryIr("dummy filename", data, size, options, &errors, &module);
  return 0;
}
