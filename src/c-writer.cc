/*
 * Copyright 2017 WebAssembly Community Group participants
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

#include "src/c-writer.h"

#include <cinttypes>

#include "src/cast.h"
#include "src/common.h"
#include "src/ir.h"
#include "src/literal.h"
#include "src/stream.h"

namespace wabt {

namespace {

class CWriter {
 public:
  CWriter(Stream* stream, const WriteCOptions* options)
      : options_(options), stream_(stream) {}

  Result WriteModule(const Module&);

 private:
  void WriteHeader();
  void WriteFuncTypes();
  void WriteMemory(const Memory&);
  void WriteTable(const Table&);
  void WriteDataInitializers();
  void WriteElemInitializers();
  void WriteInitExpr(const ExprList&);
  void WriteConst(const Const&);

  const WriteCOptions* options_ = nullptr;
  const Module* module_ = nullptr;
  Stream* stream_ = nullptr;
  Result result_ = Result::Ok;
  int indent_ = 0;

  // Index func_index_ = 0;
  // Index global_index_ = 0;
  // Index table_index_ = 0;
  // Index memory_index_ = 0;
  // Index func_type_index_ = 0;
};

static const char s_header[] =
R"(#include <math.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

void trap(int);
)";

void CWriter::WriteHeader() {
  stream_->Writef(s_header);
}

void CWriter::WriteFuncTypes() {
  stream_->Writef("\nstatic u32 func_types[%" PRIzd "];\n",
                  module_->func_types.size());
  stream_->Writef("u32 register_func_type(u32 params, u32 results, ...);\n\n");
  stream_->Writef("static void init_func_types(void) {\n");
  Index func_type_index = 0;
  for (FuncType* func_type : module_->func_types) {
    stream_->Writef("  func_types[%" PRIindex
                    "] = register_func_type(%" PRIindex ", %" PRIindex ");",
                    func_type_index, func_type->GetNumParams(),
                    func_type->GetNumResults());
    ++func_type_index;
  }
  stream_->Writef("}\n");
}

void CWriter::WriteMemory(const Memory& memory) {
  stream_->Writef("\ntypedef struct Memory { u8* data; size_t len; } Memory;\n");
  stream_->Writef("typedef struct DataSegment { u32 offset; u8* data; size_t len; } DataSegment;\n");
  stream_->Writef("static Memory mem;\n");
}

void CWriter::WriteTable(const Table& table) {
  stream_->Writef("\ntypedef void (*Anyfunc)();\n");
  stream_->Writef("typedef struct Elem { u32 func_type; Anyfunc func; } Elem;\n");
  stream_->Writef("typedef struct Table { Elem* data; size_t len; } Table;\n");
  stream_->Writef("static Table table;\n");
}

void CWriter::WriteDataInitializers() {
  if (module_->memories.empty())
    return;

  Index data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    stream_->Writef("\nstatic u8 data_segment_data_%" PRIindex "[] = {",
                    data_segment_index);
    size_t i = 0;
    for (uint8_t x : data_segment->data) {
      if ((i++ % 16) == 0)
        stream_->Writef("\n  ");
      stream_->Writef("0x%02x, ", x);
    }
    stream_->Writef("\n};\n");
    ++data_segment_index;
  }

  stream_->Writef("\nstatic void init_data(void) {\n");
  data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    stream_->Writef("  init_data_segment(");
    WriteInitExpr(data_segment->offset);
    stream_->Writef(", data_segment_data_%" PRIindex ", %" PRIzd ");\n",
                    data_segment_index, data_segment->data.size());
    ++data_segment_index;
  }
  stream_->Writef("}\n");
}

void CWriter::WriteElemInitializers() {
}

void CWriter::WriteInitExpr(const ExprList& expr_list) {
  assert(expr_list.size() == 1);
  const Expr* expr = &expr_list.front();
  switch (expr_list.front().type()) {
    case ExprType::Const: {
      WriteConst(cast<ConstExpr>(expr)->const_);
      break;
    }

    case ExprType::GetGlobal: {
      const Var& var = cast<GetGlobalExpr>(expr)->var;
      assert(var.is_name());
      stream_->Writef("%s", var.name().c_str());
      break;
    }

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteConst(const Const& const_) {
  switch (const_.type) {
    case Type::I32:
      stream_->Writef("%d", static_cast<int32_t>(const_.u32));
      break;

    case Type::I64:
      stream_->Writef("%" PRId64, static_cast<int64_t>(const_.u64));
      break;

    case Type::F32: {
      char buffer[128];
      WriteFloatHex(buffer, 128, const_.f32_bits);
      stream_->Writef("%s (;=%g)", buffer, Bitcast<float>(const_.f32_bits));
      break;
    }

    case Type::F64: {
      char buffer[128];
      WriteDoubleHex(buffer, 128, const_.f64_bits);
      stream_->Writef("%s (;=%g;)", buffer, Bitcast<double>(const_.f64_bits));
      break;
    }

    default:
      WABT_UNREACHABLE;
  }
}

Result CWriter::WriteModule(const Module& module) {
  WABT_USE(indent_);
  WABT_USE(options_);
  WABT_USE(stream_);

  module_ = &module;
  WriteHeader();

  assert(module_->memories.size() <= 1);
  for (Memory* memory: module_->memories)
    WriteMemory(*memory);

  assert(module_->tables.size() <= 1);
  for (Table* table: module_->tables)
    WriteTable(*table);

  WriteDataInitializers();

  return result_;
}


}  // end anonymous namespace

Result WriteC(Stream* stream,
              const Module* module,
              const WriteCOptions* options) {
  CWriter c_writer(stream, options);
  return c_writer.WriteModule(*module);
}

}  // namespace wabt
