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
#include "src/string-view.h"

#define INDENT_SIZE 2

namespace wabt {

namespace {

enum class NextChar {
  None,
  Space,
  Newline,
  ForceNewline,
};


class CWriter {
 public:
  CWriter(Stream* stream, const WriteCOptions* options)
      : options_(options), stream_(stream) {}

  Result WriteModule(const Module&);

 private:
  void Indent();
  void Dedent();
  void WriteIndent();
  void WriteNextChar();
  void WriteDataWithNextChar(const void* src, size_t size);
  void Writef(const char* format, ...);
  void WritefNewline(const char* format, ...);
  void WritePutc(char c);
  void WritePuts(const char* s);
  void WritePutsNewline(const char* s);
  void WriteNewline(bool force = false);

  void WriteName(const std::string&);
  void WriteVar(const Var&);
  void WriteTypeName(Type);
  void WriteResultType(const TypeVector&);
  void WriteConst(const Const&);
  void WriteInitExpr(const ExprList&);

  void WriteHeader();
  void WriteFuncTypes();
  void WriteFuncDeclarations();
  void WriteGlobals();
  void WriteMemories();
  void WriteTables();
  void WriteDataInitializers();
  void WriteElemInitializers();
  void WriteFuncs();
  void WriteFunc(const Func&);
  void WriteStackVariables(const Func&);
  void WriteLocals(const Func&);

  const WriteCOptions* options_ = nullptr;
  const Module* module_ = nullptr;
  Stream* stream_ = nullptr;
  Result result_ = Result::Ok;
  int indent_ = 0;
  NextChar next_char_ = NextChar::None;

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
u32 register_func_type(u32 params, u32 results, ...);
void init_data_segment(u32 addr, u8 *data, size_t len);
void init_elem_segment(u32 addr, Elem *data, size_t len);

)";

void CWriter::Indent() {
  indent_ += INDENT_SIZE;
}

void CWriter::Dedent() {
  indent_ -= INDENT_SIZE;
  assert(indent_ >= 0);
}

void CWriter::WriteIndent() {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t to_write = indent_;
  while (to_write >= s_indent_len) {
    stream_->WriteData(s_indent, s_indent_len);
    to_write -= s_indent_len;
  }
  if (to_write > 0) {
    stream_->WriteData(s_indent, to_write);
  }
}

void CWriter::WriteNextChar() {
  switch (next_char_) {
    case NextChar::Space:
      stream_->WriteChar(' ');
      break;
    case NextChar::Newline:
    case NextChar::ForceNewline:
      stream_->WriteChar('\n');
      WriteIndent();
      break;
    case NextChar::None:
      break;
  }
  next_char_ = NextChar::None;
}

void CWriter::WriteDataWithNextChar(const void* src, size_t size) {
  WriteNextChar();
  stream_->WriteData(src, size);
}

void WABT_PRINTF_FORMAT(2, 3) CWriter::Writef(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  WriteDataWithNextChar(buffer, length);
  next_char_ = NextChar::None;
}

void WABT_PRINTF_FORMAT(2, 3) CWriter::WritefNewline(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  WriteDataWithNextChar(buffer, length);
  next_char_ = NextChar::Newline;
}

void CWriter::WritePutc(char c) {
  stream_->WriteChar(c);
}

void CWriter::WritePuts(const char* s) {
  size_t len = strlen(s);
  WriteDataWithNextChar(s, len);
  next_char_ = NextChar::None;
}

void CWriter::WritePutsNewline(const char* s) {
  size_t len = strlen(s);
  WriteDataWithNextChar(s, len);
  next_char_ = NextChar::Newline;
}

void CWriter::WriteNewline(bool force) {
  if (next_char_ == NextChar::ForceNewline)
    WriteNextChar();
  next_char_ = force ? NextChar::ForceNewline : NextChar::Newline;
}

void CWriter::WriteName(const std::string& name) {
  Writef("%s", name.c_str() + 1);  // Skip $ prefix.
}

void CWriter::WriteVar(const Var& var) {
  assert(var.is_name());
  assert(var.name().size() >= 1);
  WriteName(var.name());
}

void CWriter::WriteTypeName(Type type) {
  switch (type) {
    case Type::I32: WritePuts("u32"); break;
    case Type::I64: WritePuts("u64"); break;
    case Type::F32: WritePuts("f32"); break;
    case Type::F64: WritePuts("f64"); break;
    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteResultType(const TypeVector& results) {
  if (!results.empty()) {
    WriteTypeName(results[0]);
  } else {
    WritePuts("void");
  }
}

void CWriter::WriteConst(const Const& const_) {
  switch (const_.type) {
    case Type::I32:
      Writef("%d", static_cast<int32_t>(const_.u32));
      break;

    case Type::I64:
      Writef("%" PRId64, static_cast<int64_t>(const_.u64));
      break;

    case Type::F32: {
      char buffer[128];
      WriteFloatHex(buffer, 128, const_.f32_bits);
      Writef("%s /*=%g*/", buffer, Bitcast<float>(const_.f32_bits));
      break;
    }

    case Type::F64: {
      char buffer[128];
      WriteDoubleHex(buffer, 128, const_.f64_bits);
      Writef("%s /*=%g*/", buffer, Bitcast<double>(const_.f64_bits));
      break;
    }

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteInitExpr(const ExprList& expr_list) {
  if (expr_list.empty())
    return;

  assert(expr_list.size() == 1);
  const Expr* expr = &expr_list.front();
  switch (expr_list.front().type()) {
    case ExprType::Const:
      WriteConst(cast<ConstExpr>(expr)->const_);
      break;

    case ExprType::GetGlobal:
      WriteVar(cast<GetGlobalExpr>(expr)->var);
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteHeader() {
  Writef(s_header);
}

void CWriter::WriteFuncTypes() {
  WritefNewline("static u32 func_types[%" PRIzd "];",
                module_->func_types.size());
  WriteNewline(true);
  WritefNewline("static void init_func_types(void) {");
  Index func_type_index = 0;
  for (FuncType* func_type : module_->func_types) {
    Index num_params = func_type->GetNumParams();
    Index num_results = func_type->GetNumResults();
    Writef("  func_types[%" PRIindex "] = register_func_type(%" PRIindex
           ", %" PRIindex "",
           func_type_index, num_params, num_results);
    for (Index i = 0; i < num_params; ++i)
      Writef(", %s", GetTypeName(func_type->GetParamType(i)));

    for (Index i = 0; i < num_results; ++i)
      Writef(", %s", GetTypeName(func_type->GetResultType(i)));

    WritefNewline(");");
    ++func_type_index;
  }
  WritePutsNewline("}");
}

void CWriter::WriteFuncDeclarations() {
  if (module_->funcs.empty())
    return;

  for (Func* func : module_->funcs) {
    WritePuts("static ");
    WriteResultType(func->decl.sig.result_types);
    WritePuts(" ");
    WriteName(func->name);
    WritePuts("(");
    for (Index i = 0; i < func->GetNumParams(); ++i) {
      if (i != 0)
        WritePuts(", ");
      WriteTypeName(func->GetParamType(i));
    }
    WritePuts(");");
    WriteNewline();
  }
}

void CWriter::WriteGlobals() {
  if (module_->globals.empty())
    return;

  for (Global* global : module_->globals) {
    WritePuts("static ");
    WriteTypeName(global->type);
    WritePuts(" ");
    WriteName(global->name);
    if (!global->init_expr.empty()) {
      WritePuts(" = ");
      WriteInitExpr(global->init_expr);
    }
    WritePuts(";");
    WriteNewline();
  }
}

void CWriter::WriteMemories() {
  assert(module_->memories.size() <= 1);
  for (Memory* memory : module_->memories) {
    WABT_USE(memory);
    WritePutsNewline("typedef struct Memory { u8* data; size_t len; } Memory;");
    WritePutsNewline(
        "typedef struct DataSegment { u32 offset; u8* data; size_t len; } "
        "DataSegment;");
    WritePutsNewline("static Memory mem;");
  }
}

void CWriter::WriteTables() {
  assert(module_->tables.size() <= 1);
  for (Table* table : module_->tables) {
    WABT_USE(table);
    WritePutsNewline("typedef void (*Anyfunc)();");
    WritePutsNewline("typedef struct Elem { u32 func_type; Anyfunc func; } Elem;");
    WritePutsNewline("typedef struct Table { Elem* data; size_t len; } Table;");
    WritePutsNewline("static Table table;");
  }
}

void CWriter::WriteDataInitializers() {
  if (module_->memories.empty())
    return;

  Index data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    Writef("static u8 data_segment_data_%" PRIindex "[] = {",
           data_segment_index);
    Indent();
    WriteNewline();
    size_t i = 0;
    for (uint8_t x : data_segment->data) {
      Writef("0x%02x, ", x);
      if ((++i % 16) == 0)
        WriteNewline();
    }
    Dedent();
    WriteNewline();
    WritePutsNewline("};");
    ++data_segment_index;
  }

  WritePutsNewline("static void init_memory(void) {");
  data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    WritePuts("  init_data_segment(");
    WriteInitExpr(data_segment->offset);
    WritefNewline(", data_segment_data_%" PRIindex ", %" PRIzd ");",
                  data_segment_index, data_segment->data.size());
    ++data_segment_index;
  }
  WritefNewline("}");
}

void CWriter::WriteElemInitializers() {
  if (module_->tables.empty())
    return;

  Index elem_segment_index = 0;
  for (ElemSegment* elem_segment : module_->elem_segments) {
    Writef("static Elem elem_segment_data_%" PRIindex "[] = {",
           elem_segment_index);
    Indent();
    WriteNewline();

    size_t i = 0;
    for (const Var& var : elem_segment->vars) {
      const Func* func = module_->GetFunc(var);
      Index func_type_index = module_->GetFuncTypeIndex(func->decl.type_var);

      Writef("{%" PRIindex ", ", func_type_index);
      WriteName(func->name);
      WritePuts("}, ");

      if ((++i % 8) == 0)
        WriteNewline();
    }
    Dedent();
    WriteNewline();
    WritePutsNewline("};");
    ++elem_segment_index;
  }

  WritePutsNewline("static void init_table(void) {");
  elem_segment_index = 0;
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    WritePuts("  init_elem_segment(");
    WriteInitExpr(elem_segment->offset);
    WritefNewline(", elem_segment_data_%" PRIindex ", %" PRIzd ");",
                  elem_segment_index, elem_segment->vars.size());
    ++elem_segment_index;
  }
  WritePutsNewline("}");
}

void CWriter::WriteFuncs() {
  for (const Func* func : module_->funcs)
    WriteFunc(*func);
}

void CWriter::WriteFunc(const Func& func) {
  WritePuts("static ");
  WriteResultType(func.decl.sig.result_types);
  WritePuts(" ");
  WriteName(func.name);
  WritePuts("(");
  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func.decl.sig.param_types, func.param_bindings,
                                &index_to_name);
  for (Index i = 0; i < func.GetNumParams(); ++i) {
    if (i != 0)
      WritePuts(", ");
    WriteTypeName(func.GetParamType(i));
    WritePuts(" ");
    WriteName(index_to_name[i]);
  }
  WritePuts(") {");
  Indent();
  WriteNewline();
  WriteStackVariables(func);
  WriteLocals(func);
  Dedent();
  WriteNewline();
  WritePutsNewline("}");
}

void CWriter::WriteStackVariables(const Func& func) {
}

void CWriter::WriteLocals(const Func& func) {
  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func.local_types, func.local_bindings,
                                &index_to_name);
  for (Type type : {Type::I32, Type::I64, Type::F32, Type::F64}) {
    Index local_index = 0;
    size_t count = 0;
    for (Type local_type : func.local_types) {
      if (local_type == type) {
        if (count++ == 0) {
          WriteTypeName(type);
          WritePuts(" ");
        } else {
          WritePuts(", ");
        }

        WriteName(index_to_name[local_index]);
      }
      ++local_index;
    }
    if (count != 0)
      WritePutsNewline(";");
  }
}

Result CWriter::WriteModule(const Module& module) {
  WABT_USE(indent_);
  WABT_USE(options_);
  WABT_USE(stream_);

  module_ = &module;
  WriteHeader();
  WriteFuncTypes();
  WriteFuncDeclarations();
  WriteGlobals();
  WriteMemories();
  WriteTables();
  WriteDataInitializers();
  WriteElemInitializers();
  WriteFuncs();

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
