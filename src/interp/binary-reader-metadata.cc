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

#include "src/interp/binary-reader-metadata.h"

#include "src/binary-reader-nop.h"
#include "src/binary-reader.h"
#include "src/interp/interp.h"
#include "src/string-view.h"

namespace wabt {

using namespace interp;

namespace {

class BinaryReaderMetadata : public BinaryReaderNop {
 public:
  BinaryReaderMetadata(ModuleMetadata* metadata,
                       Errors* errors,
                       const Features& features)
      : metadata_(metadata) {}
  wabt::Result OnImport(Index index,
                        ExternalKind kind,
                        string_view module_name,
                        string_view field_name) override {
    metadata_->imports.emplace_back(kind, module_name, field_name);
    return wabt::Result::Ok;
  }

  wabt::Result OnExport(Index index,
                        ExternalKind kind,
                        Index item_index,
                        string_view name) override {
    metadata_->exports.emplace_back(name, kind, item_index);
    return wabt::Result::Ok;
  }

 private:
  ModuleMetadata* metadata_;
};

}  // end anonymous namespace

wabt::Result ReadBinaryMetadata(const void* data,
                                size_t size,
                                const ReadBinaryOptions& options,
                                Errors* errors,
                                ModuleMetadata** out_metadata) {
  ModuleMetadata* metadata = new ModuleMetadata();
  BinaryReaderMetadata reader(metadata, errors, options.features);
  wabt::Result result = ReadBinary(data, size, &reader, options);
  if (!Succeeded(result)) {
    delete metadata;
    return result;
  }
  *out_metadata = metadata;
  return result;
}

}  // namespace wabt
