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

#include "gtest/gtest.h"

#include <memory>

#include "src/binary-reader.h"
#include "src/binary-reader-interp.h"
#include "src/cast.h"
#include "src/error-handler.h"
#include "src/interp.h"
#include "src/make-unique.h"

using namespace wabt;

namespace {

class TrapHostImportDelegate : public interp::HostImportDelegate {
 public:
  Result ImportFunc(interp::FuncImport* import,
                    interp::Func* func,
                    interp::FuncSignature* func_sig,
                    const ErrorCallback& callback) override {
    cast<interp::HostFunc>(func)->callback = TrapCallback;
    return Result::Ok;
  }

  Result ImportTable(interp::TableImport* import,
                     interp::Table* table,
                     const ErrorCallback& callback) override {
    return Result::Error;
  }

  Result ImportMemory(interp::MemoryImport* import,
                      interp::Memory* memory,
                      const ErrorCallback& callback) override {
    return Result::Error;
  }

  Result ImportGlobal(interp::GlobalImport* import,
                      interp::Global* global,
                      const ErrorCallback& callback) override {
    return Result::Error;
  }

 private:
  static interp::Result TrapCallback(const interp::HostFunc* func,
                                     const interp::FuncSignature* sig,
                                     Index num_args,
                                     interp::TypedValue* args,
                                     Index num_results,
                                     interp::TypedValue* out_results,
                                     void* user_data) {
    return interp::Result::TrapHostTrapped;
  }
};

class HostTrapTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    interp::HostModule* host_module = env_.AppendHostModule("host");
    host_module->import_delegate = MakeUnique<TrapHostImportDelegate>();
    executor_ = MakeUnique<interp::Executor>(&env_);
  }

  virtual void TearDown() {}

  interp::ExecResult LoadModuleAndRunStartFunction(
      const std::vector<uint8_t>& data) {
    ErrorHandlerFile error_handler(Location::Type::Binary);
    interp::DefinedModule* module = nullptr;
    ReadBinaryOptions options;
    Result result = ReadBinaryInterp(&env_, data.data(), data.size(), options,
                                     &error_handler, &module);
    EXPECT_EQ(Result::Ok, result);

    if (result == Result::Ok) {
      return executor_->RunStartFunction(module);
    } else {
      return {};
    }
  }

  interp::Environment env_;
  std::unique_ptr<interp::Executor> executor_;
};

}  // end of anonymous namespace

TEST_F(HostTrapTest, Call) {
  // (import "host" "a" (func $0))
  // (func $1 call $0)
  // (start $1)
  std::vector<uint8_t> data = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01,
      0x60, 0x00, 0x00, 0x02, 0x0a, 0x01, 0x04, 0x68, 0x6f, 0x73, 0x74,
      0x01, 0x61, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x08, 0x01, 0x01,
      0x0a, 0x06, 0x01, 0x04, 0x00, 0x10, 0x00, 0x0b,
  };
  ASSERT_EQ(interp::Result::TrapHostTrapped,
            LoadModuleAndRunStartFunction(data).result);
}

TEST_F(HostTrapTest, CallIndirect) {
  // (import "host" "a" (func $0))
  // (table anyfunc (elem $0))
  // (func $1 i32.const 0 call_indirect)
  // (start $1)
  std::vector<uint8_t> data = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
      0x00, 0x00, 0x02, 0x0a, 0x01, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x01, 0x61,
      0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x04, 0x05, 0x01, 0x70, 0x01, 0x01,
      0x01, 0x08, 0x01, 0x01, 0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01,
      0x00, 0x0a, 0x09, 0x01, 0x07, 0x00, 0x41, 0x00, 0x11, 0x00, 0x00, 0x0b,
  };
  ASSERT_EQ(interp::Result::TrapHostTrapped,
            LoadModuleAndRunStartFunction(data).result);
}
