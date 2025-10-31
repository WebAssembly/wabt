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

#ifndef WABT_BINARY_READER_OBJDUMP_H_
#define WABT_BINARY_READER_OBJDUMP_H_

#include <map>
#include <string>

#include "wabt/binary-reader-options.h"
#include "wabt/common.h"
#include "wabt/feature.h"
#include "wabt/stream.h"

namespace wabt {

struct Module;

enum class ObjdumpMode {
  Prepass,
  Headers,
  Details,
  Disassemble,
  RawData,
};

struct ObjdumpOptions {
  Stream* log_stream;
  bool headers;
  bool details;
  bool raw;
  bool disassemble;
  bool debug;
  bool relocs;
  bool section_offsets;
  ObjdumpMode mode;
  const char* filename;
  const char* section_name;
};

struct ObjdumpSymbol {
  wabt::SymbolType kind;
  std::string name;
  Index index;
};

struct ObjdumpNames {
  std::string_view Get(Index index) const;
  void Set(Index index, std::string_view name);

  std::map<Index, std::string> names;
};

struct ObjdumpLocalNames {
  std::string_view Get(Index function_index, Index local_index) const;
  void Set(Index function_index, Index local_index, std::string_view name);

  std::map<std::pair<Index, Index>, std::string> names;
};

// read_binary_objdump uses this state to store information from previous runs
// and use it to display more useful information.
struct ObjdumpState {
  std::vector<Reloc> code_relocations;
  std::vector<Reloc> data_relocations;
  ObjdumpNames type_names;
  ObjdumpNames function_names;
  ObjdumpNames global_names;
  ObjdumpNames section_names;
  ObjdumpNames tag_names;
  ObjdumpNames segment_names;
  ObjdumpNames table_names;
  ObjdumpLocalNames local_names;
  std::vector<ObjdumpSymbol> symtab;
  std::map<Index, Index> function_param_counts;
  std::map<Index, Index> function_types;
};

Result ReadBinaryObjdump(const uint8_t* data,
                         size_t size,
                         ObjdumpOptions* options,
                         ObjdumpState* state);

}  // namespace wabt

#endif /* WABT_BINARY_READER_OBJDUMP_H_ */
