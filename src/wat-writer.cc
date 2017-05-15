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

#include "wat-writer.h"

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include "common.h"
#include "ir.h"
#include "literal.h"
#include "stream.h"
#include "writer.h"

#define INDENT_SIZE 2
#define NO_FORCE_NEWLINE 0
#define FORCE_NEWLINE 1

namespace wabt {

namespace {

static const uint8_t s_is_char_escaped[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

enum class NextChar {
  None,
  Space,
  Newline,
  ForceNewline,
};

class WatWriter {
 public:
  WatWriter(Writer* writer) : stream_(writer) {}

  Result WriteModule(const Module* module);

 private:
  void Indent();
  void Dedent();
  void WriteIndent();
  void WriteNextChar();
  void WriteDataWithNextChar(const void* src, size_t size);
  void Writef(const char* format, ...);
  void WritePutc(char c);
  void WritePuts(const char* s, NextChar next_char);
  void WritePutsSpace(const char* s);
  void WritePutsNewline(const char* s);
  void WriteNewline(bool force);
  void WriteOpen(const char* name, NextChar next_char);
  void WriteOpenNewline(const char* name);
  void WriteOpenSpace(const char* name);
  void WriteClose(NextChar next_char);
  void WriteCloseNewline();
  void WriteCloseSpace();
  void WriteString(const std::string& str, NextChar next_char);
  void WriteStringSlice(const StringSlice* str, NextChar next_char);
  bool WriteStringSliceOpt(const StringSlice* str, NextChar next_char);
  void WriteStringSliceOrIndex(const StringSlice* str,
                               Index index,
                               NextChar next_char);
  void WriteQuotedData(const void* data, size_t length);
  void WriteQuotedStringSlice(const StringSlice* str, NextChar next_char);
  void WriteVar(const Var* var, NextChar next_char);
  void WriteBrVar(const Var* var, NextChar next_char);
  void WriteType(Type type, NextChar next_char);
  void WriteTypes(const TypeVector& types, const char* name);
  void WriteFuncSigSpace(const FuncSignature* func_sig);
  void WriteBeginBlock(const Block* block, const char* text);
  void WriteEndBlock();
  void WriteBlock(const Block* block, const char* start_text);
  void WriteConst(const Const* const_);
  void WriteExpr(const Expr* expr);
  void WriteExprList(const Expr* first);
  void WriteInitExpr(const Expr* expr);
  void WriteTypeBindings(const char* prefix,
                         const Func* func,
                         const TypeVector& types,
                         const BindingHash& bindings);
  void WriteFunc(const Module* module, const Func* func);
  void WriteBeginGlobal(const Global* global);
  void WriteGlobal(const Global* global);
  void WriteLimits(const Limits* limits);
  void WriteTable(const Table* table);
  void WriteElemSegment(const ElemSegment* segment);
  void WriteMemory(const Memory* memory);
  void WriteDataSegment(const DataSegment* segment);
  void WriteImport(const Import* import);
  void WriteExport(const Export* export_);
  void WriteFuncType(const FuncType* func_type);
  void WriteStartFunction(const Var* start);

  Stream stream_;
  Result result_ = Result::Ok;
  int indent_ = 0;
  NextChar next_char_ = NextChar::None;
  Index depth_ = 0;
  std::vector<std::string> index_to_name_;

  Index func_index_ = 0;
  Index global_index_ = 0;
  Index table_index_ = 0;
  Index memory_index_ = 0;
  Index func_type_index_ = 0;
};

}  // namespace

void WatWriter::Indent() {
  indent_ += INDENT_SIZE;
}

void WatWriter::Dedent() {
  indent_ -= INDENT_SIZE;
  assert(indent_ >= 0);
}

void WatWriter::WriteIndent() {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t indent = indent_;
  while (static_cast<size_t>(indent_) > s_indent_len) {
    stream_.WriteData(s_indent, s_indent_len);
    indent -= s_indent_len;
  }
  if (indent > 0) {
    stream_.WriteData(s_indent, indent);
  }
}

void WatWriter::WriteNextChar() {
  switch (next_char_) {
    case NextChar::Space:
      stream_.WriteChar(' ');
      break;
    case NextChar::Newline:
    case NextChar::ForceNewline:
      stream_.WriteChar('\n');
      WriteIndent();
      break;

    default:
    case NextChar::None:
      break;
  }
  next_char_ = NextChar::None;
}

void WatWriter::WriteDataWithNextChar(const void* src, size_t size) {
  WriteNextChar();
  stream_.WriteData(src, size);
}

void WABT_PRINTF_FORMAT(2, 3) WatWriter::Writef(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  /* default to following space */
  WriteDataWithNextChar(buffer, length);
  next_char_ = NextChar::Space;
}

void WatWriter::WritePutc(char c) {
  stream_.WriteChar(c);
}

void WatWriter::WritePuts(const char* s, NextChar next_char) {
  size_t len = strlen(s);
  WriteDataWithNextChar(s, len);
  next_char_ = next_char;
}

void WatWriter::WritePutsSpace(const char* s) {
  WritePuts(s, NextChar::Space);
}

void WatWriter::WritePutsNewline(const char* s) {
  WritePuts(s, NextChar::Newline);
}

void WatWriter::WriteNewline(bool force) {
  if (next_char_ == NextChar::ForceNewline)
    WriteNextChar();
  next_char_ = force ? NextChar::ForceNewline : NextChar::Newline;
}

void WatWriter::WriteOpen(const char* name, NextChar next_char) {
  WritePuts("(", NextChar::None);
  WritePuts(name, next_char);
  Indent();
}

void WatWriter::WriteOpenNewline(const char* name) {
  WriteOpen(name, NextChar::Newline);
}

void WatWriter::WriteOpenSpace(const char* name) {
  WriteOpen(name, NextChar::Space);
}

void WatWriter::WriteClose(NextChar next_char) {
  if (next_char_ != NextChar::ForceNewline)
    next_char_ = NextChar::None;
  Dedent();
  WritePuts(")", next_char);
}

void WatWriter::WriteCloseNewline() {
  WriteClose(NextChar::Newline);
}

void WatWriter::WriteCloseSpace() {
  WriteClose(NextChar::Space);
}

void WatWriter::WriteString(const std::string& str, NextChar next_char) {
  WritePuts(str.c_str(), next_char);
}

void WatWriter::WriteStringSlice(const StringSlice* str, NextChar next_char) {
  Writef(PRIstringslice, WABT_PRINTF_STRING_SLICE_ARG(*str));
  next_char_ = next_char;
}

bool WatWriter::WriteStringSliceOpt(const StringSlice* str,
                                    NextChar next_char) {
  if (str->start)
    WriteStringSlice(str, next_char);
  return !!str->start;
}

void WatWriter::WriteStringSliceOrIndex(const StringSlice* str,
                                        Index index,
                                        NextChar next_char) {
  if (str->start)
    WriteStringSlice(str, next_char);
  else
    Writef("(;%u;)", index);
}

void WatWriter::WriteQuotedData(const void* data, size_t length) {
  const uint8_t* u8_data = static_cast<const uint8_t*>(data);
  static const char s_hexdigits[] = "0123456789abcdef";
  WriteNextChar();
  WritePutc('\"');
  for (size_t i = 0; i < length; ++i) {
    uint8_t c = u8_data[i];
    if (s_is_char_escaped[c]) {
      WritePutc('\\');
      WritePutc(s_hexdigits[c >> 4]);
      WritePutc(s_hexdigits[c & 0xf]);
    } else {
      WritePutc(c);
    }
  }
  WritePutc('\"');
  next_char_ = NextChar::Space;
}

void WatWriter::WriteQuotedStringSlice(const StringSlice* str,
                                       NextChar next_char) {
  WriteQuotedData(str->start, str->length);
  next_char_ = next_char;
}

void WatWriter::WriteVar(const Var* var, NextChar next_char) {
  if (var->type == VarType::Index) {
    Writef("%" PRIindex, var->index);
    next_char_ = next_char;
  } else {
    WriteStringSlice(&var->name, next_char);
  }
}

void WatWriter::WriteBrVar(const Var* var, NextChar next_char) {
  if (var->type == VarType::Index) {
    assert(var->index < depth_);
    Writef("%" PRIindex " (;@%" PRIindex ";)", var->index,
           depth_ - var->index - 1);
    next_char_ = next_char;
  } else {
    WriteStringSlice(&var->name, next_char);
  }
}

void WatWriter::WriteType(Type type, NextChar next_char) {
  const char* type_name = get_type_name(type);
  assert(type_name);
  WritePuts(type_name, next_char);
}

void WatWriter::WriteTypes(const TypeVector& types, const char* name) {
  if (types.size()) {
    if (name)
      WriteOpenSpace(name);
    for (Type type: types)
      WriteType(type, NextChar::Space);
    if (name)
      WriteCloseSpace();
  }
}

void WatWriter::WriteFuncSigSpace(const FuncSignature* func_sig) {
  WriteTypes(func_sig->param_types, "param");
  WriteTypes(func_sig->result_types, "result");
}

void WatWriter::WriteBeginBlock(const Block* block, const char* text) {
  WritePutsSpace(text);
  bool has_label = WriteStringSliceOpt(&block->label, NextChar::Space);
  WriteTypes(block->sig, nullptr);
  if (!has_label)
    Writef(" ;; label = @%" PRIindex, depth_);
  WriteNewline(FORCE_NEWLINE);
  depth_++;
  Indent();
}

void WatWriter::WriteEndBlock() {
  Dedent();
  depth_--;
  WritePutsNewline(get_opcode_name(Opcode::End));
}

void WatWriter::WriteBlock(const Block* block, const char* start_text) {
  WriteBeginBlock(block, start_text);
  WriteExprList(block->first);
  WriteEndBlock();
}

void WatWriter::WriteConst(const Const* const_) {
  switch (const_->type) {
    case Type::I32:
      WritePutsSpace(get_opcode_name(Opcode::I32Const));
      Writef("%d", static_cast<int32_t>(const_->u32));
      WriteNewline(NO_FORCE_NEWLINE);
      break;

    case Type::I64:
      WritePutsSpace(get_opcode_name(Opcode::I64Const));
      Writef("%" PRId64, static_cast<int64_t>(const_->u64));
      WriteNewline(NO_FORCE_NEWLINE);
      break;

    case Type::F32: {
      WritePutsSpace(get_opcode_name(Opcode::F32Const));
      char buffer[128];
      write_float_hex(buffer, 128, const_->f32_bits);
      WritePutsSpace(buffer);
      float f32;
      memcpy(&f32, &const_->f32_bits, sizeof(f32));
      Writef("(;=%g;)", f32);
      WriteNewline(NO_FORCE_NEWLINE);
      break;
    }

    case Type::F64: {
      WritePutsSpace(get_opcode_name(Opcode::F64Const));
      char buffer[128];
      write_double_hex(buffer, 128, const_->f64_bits);
      WritePutsSpace(buffer);
      double f64;
      memcpy(&f64, &const_->f64_bits, sizeof(f64));
      Writef("(;=%g;)", f64);
      WriteNewline(NO_FORCE_NEWLINE);
      break;
    }

    default:
      assert(0);
      break;
  }
}

void WatWriter::WriteExpr(const Expr* expr) {
  switch (expr->type) {
    case ExprType::Binary:
      WritePutsNewline(get_opcode_name(expr->binary.opcode));
      break;

    case ExprType::Block:
      WriteBlock(expr->block, get_opcode_name(Opcode::Block));
      break;

    case ExprType::Br:
      WritePutsSpace(get_opcode_name(Opcode::Br));
      WriteBrVar(&expr->br.var, NextChar::Newline);
      break;

    case ExprType::BrIf:
      WritePutsSpace(get_opcode_name(Opcode::BrIf));
      WriteBrVar(&expr->br_if.var, NextChar::Newline);
      break;

    case ExprType::BrTable: {
      WritePutsSpace(get_opcode_name(Opcode::BrTable));
      for (const Var& var : *expr->br_table.targets)
        WriteBrVar(&var, NextChar::Space);
      WriteBrVar(&expr->br_table.default_target, NextChar::Newline);
      break;
    }

    case ExprType::Call:
      WritePutsSpace(get_opcode_name(Opcode::Call));
      WriteVar(&expr->call.var, NextChar::Newline);
      break;

    case ExprType::CallIndirect:
      WritePutsSpace(get_opcode_name(Opcode::CallIndirect));
      WriteVar(&expr->call_indirect.var, NextChar::Newline);
      break;

    case ExprType::Compare:
      WritePutsNewline(get_opcode_name(expr->compare.opcode));
      break;

    case ExprType::Const:
      WriteConst(&expr->const_);
      break;

    case ExprType::Convert:
      WritePutsNewline(get_opcode_name(expr->convert.opcode));
      break;

    case ExprType::Drop:
      WritePutsNewline(get_opcode_name(Opcode::Drop));
      break;

    case ExprType::GetGlobal:
      WritePutsSpace(get_opcode_name(Opcode::GetGlobal));
      WriteVar(&expr->get_global.var, NextChar::Newline);
      break;

    case ExprType::GetLocal:
      WritePutsSpace(get_opcode_name(Opcode::GetLocal));
      WriteVar(&expr->get_local.var, NextChar::Newline);
      break;

    case ExprType::GrowMemory:
      WritePutsNewline(get_opcode_name(Opcode::GrowMemory));
      break;

    case ExprType::If:
      WriteBeginBlock(expr->if_.true_, get_opcode_name(Opcode::If));
      WriteExprList(expr->if_.true_->first);
      if (expr->if_.false_) {
        Dedent();
        WritePutsSpace(get_opcode_name(Opcode::Else));
        Indent();
        WriteNewline(FORCE_NEWLINE);
        WriteExprList(expr->if_.false_);
      }
      WriteEndBlock();
      break;

    case ExprType::Load:
      WritePutsSpace(get_opcode_name(expr->load.opcode));
      if (expr->load.offset)
        Writef("offset=%" PRIu64, expr->load.offset);
      if (!is_naturally_aligned(expr->load.opcode, expr->load.align))
        Writef("align=%u", expr->load.align);
      WriteNewline(NO_FORCE_NEWLINE);
      break;

    case ExprType::Loop:
      WriteBlock(expr->loop, get_opcode_name(Opcode::Loop));
      break;

    case ExprType::CurrentMemory:
      WritePutsNewline(get_opcode_name(Opcode::CurrentMemory));
      break;

    case ExprType::Nop:
      WritePutsNewline(get_opcode_name(Opcode::Nop));
      break;

    case ExprType::Return:
      WritePutsNewline(get_opcode_name(Opcode::Return));
      break;

    case ExprType::Select:
      WritePutsNewline(get_opcode_name(Opcode::Select));
      break;

    case ExprType::SetGlobal:
      WritePutsSpace(get_opcode_name(Opcode::SetGlobal));
      WriteVar(&expr->set_global.var, NextChar::Newline);
      break;

    case ExprType::SetLocal:
      WritePutsSpace(get_opcode_name(Opcode::SetLocal));
      WriteVar(&expr->set_local.var, NextChar::Newline);
      break;

    case ExprType::Store:
      WritePutsSpace(get_opcode_name(expr->store.opcode));
      if (expr->store.offset)
        Writef("offset=%" PRIu64, expr->store.offset);
      if (!is_naturally_aligned(expr->store.opcode, expr->store.align))
        Writef("align=%u", expr->store.align);
      WriteNewline(NO_FORCE_NEWLINE);
      break;

    case ExprType::TeeLocal:
      WritePutsSpace(get_opcode_name(Opcode::TeeLocal));
      WriteVar(&expr->tee_local.var, NextChar::Newline);
      break;

    case ExprType::Unary:
      WritePutsNewline(get_opcode_name(expr->unary.opcode));
      break;

    case ExprType::Unreachable:
      WritePutsNewline(get_opcode_name(Opcode::Unreachable));
      break;

    default:
      fprintf(stderr, "bad expr type: %d\n", static_cast<int>(expr->type));
      assert(0);
      break;
  }
}

void WatWriter::WriteExprList(const Expr* first) {
  for (const Expr* expr = first; expr; expr = expr->next)
    WriteExpr(expr);
}

void WatWriter::WriteInitExpr(const Expr* expr) {
  if (expr) {
    WritePuts("(", NextChar::None);
    WriteExpr(expr);
    /* clear the next char, so we don't write a newline after the expr */
    next_char_ = NextChar::None;
    WritePuts(")", NextChar::Space);
  }
}

void WatWriter::WriteTypeBindings(const char* prefix,
                                  const Func* func,
                                  const TypeVector& types,
                                  const BindingHash& bindings) {
  make_type_binding_reverse_mapping(types, bindings, &index_to_name_);

  /* named params/locals must be specified by themselves, but nameless
   * params/locals can be compressed, e.g.:
   *   (param $foo i32)
   *   (param i32 i64 f32)
   */
  bool is_open = false;
  for (size_t i = 0; i < types.size(); ++i) {
    if (!is_open) {
      WriteOpenSpace(prefix);
      is_open = true;
    }

    const std::string& name = index_to_name_[i];
    if (!name.empty())
      WriteString(name, NextChar::Space);
    WriteType(types[i], NextChar::Space);
    if (!name.empty()) {
      WriteCloseSpace();
      is_open = false;
    }
  }
  if (is_open)
    WriteCloseSpace();
}

void WatWriter::WriteFunc(const Module* module, const Func* func) {
  WriteOpenSpace("func");
  WriteStringSliceOrIndex(&func->name, func_index_++, NextChar::Space);
  if (decl_has_func_type(&func->decl)) {
    WriteOpenSpace("type");
    WriteVar(&func->decl.type_var, NextChar::None);
    WriteCloseSpace();
  }
  WriteTypeBindings("param", func, func->decl.sig.param_types,
                    func->param_bindings);
  WriteTypes(func->decl.sig.result_types, "result");
  WriteNewline(NO_FORCE_NEWLINE);
  if (func->local_types.size()) {
    WriteTypeBindings("local", func, func->local_types, func->local_bindings);
  }
  WriteNewline(NO_FORCE_NEWLINE);
  depth_ = 1; /* for the implicit "return" label */
  WriteExprList(func->first_expr);
  WriteCloseNewline();
}

void WatWriter::WriteBeginGlobal(const Global* global) {
  WriteOpenSpace("global");
  WriteStringSliceOrIndex(&global->name, global_index_++,
                              NextChar::Space);
  if (global->mutable_) {
    WriteOpenSpace("mut");
    WriteType(global->type, NextChar::Space);
    WriteCloseSpace();
  } else {
    WriteType(global->type, NextChar::Space);
  }
}

void WatWriter::WriteGlobal(const Global* global) {
  WriteBeginGlobal(global);
  WriteInitExpr(global->init_expr);
  WriteCloseNewline();
}

void WatWriter::WriteLimits(const Limits* limits) {
  Writef("%" PRIu64, limits->initial);
  if (limits->has_max)
    Writef("%" PRIu64, limits->max);
}

void WatWriter::WriteTable(const Table* table) {
  WriteOpenSpace("table");
  WriteStringSliceOrIndex(&table->name, table_index_++,
                              NextChar::Space);
  WriteLimits(&table->elem_limits);
  WritePutsSpace("anyfunc");
  WriteCloseNewline();
}

void WatWriter::WriteElemSegment(const ElemSegment* segment) {
  WriteOpenSpace("elem");
  WriteInitExpr(segment->offset);
  for (const Var& var : segment->vars)
    WriteVar(&var, NextChar::Space);
  WriteCloseNewline();
}

void WatWriter::WriteMemory(const Memory* memory) {
  WriteOpenSpace("memory");
  WriteStringSliceOrIndex(&memory->name, memory_index_++,
                              NextChar::Space);
  WriteLimits(&memory->page_limits);
  WriteCloseNewline();
}

void WatWriter::WriteDataSegment(const DataSegment* segment) {
  WriteOpenSpace("data");
  WriteInitExpr(segment->offset);
  WriteQuotedData(segment->data, segment->size);
  WriteCloseNewline();
}

void WatWriter::WriteImport(const Import* import) {
  WriteOpenSpace("import");
  WriteQuotedStringSlice(&import->module_name, NextChar::Space);
  WriteQuotedStringSlice(&import->field_name, NextChar::Space);
  switch (import->kind) {
    case ExternalKind::Func:
      WriteOpenSpace("func");
      WriteStringSliceOrIndex(&import->func->name, func_index_++,
                                  NextChar::Space);
      if (decl_has_func_type(&import->func->decl)) {
        WriteOpenSpace("type");
        WriteVar(&import->func->decl.type_var, NextChar::None);
        WriteCloseSpace();
      } else {
        WriteFuncSigSpace(&import->func->decl.sig);
      }
      WriteCloseSpace();
      break;

    case ExternalKind::Table:
      WriteTable(import->table);
      break;

    case ExternalKind::Memory:
      WriteMemory(import->memory);
      break;

    case ExternalKind::Global:
      WriteBeginGlobal(import->global);
      WriteCloseSpace();
      break;
  }
  WriteCloseNewline();
}

void WatWriter::WriteExport(const Export* export_) {
  static const char* s_kind_names[] = {"func", "table", "memory", "global"};
  WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(s_kind_names) == kExternalKindCount);
  WriteOpenSpace("export");
  WriteQuotedStringSlice(&export_->name, NextChar::Space);
  assert(static_cast<size_t>(export_->kind) < WABT_ARRAY_SIZE(s_kind_names));
  WriteOpenSpace(s_kind_names[static_cast<size_t>(export_->kind)]);
  WriteVar(&export_->var, NextChar::Space);
  WriteCloseSpace();
  WriteCloseNewline();
}

void WatWriter::WriteFuncType(const FuncType* func_type) {
  WriteOpenSpace("type");
  WriteStringSliceOrIndex(&func_type->name, func_type_index_++,
                          NextChar::Space);
  WriteOpenSpace("func");
  WriteFuncSigSpace(&func_type->sig);
  WriteCloseSpace();
  WriteCloseNewline();
}

void WatWriter::WriteStartFunction(const Var* start) {
  WriteOpenSpace("start");
  WriteVar(start, NextChar::None);
  WriteCloseNewline();
}

Result WatWriter::WriteModule(const Module* module) {
  WriteOpenNewline("module");
  for (const ModuleField* field = module->first_field; field;
       field = field->next) {
    switch (field->type) {
      case ModuleFieldType::Func:
        WriteFunc(module, field->func);
        break;
      case ModuleFieldType::Global:
        WriteGlobal(field->global);
        break;
      case ModuleFieldType::Import:
        WriteImport(field->import);
        break;
      case ModuleFieldType::Export:
        WriteExport(field->export_);
        break;
      case ModuleFieldType::Table:
        WriteTable(field->table);
        break;
      case ModuleFieldType::ElemSegment:
        WriteElemSegment(field->elem_segment);
        break;
      case ModuleFieldType::Memory:
        WriteMemory(field->memory);
        break;
      case ModuleFieldType::DataSegment:
        WriteDataSegment(field->data_segment);
        break;
      case ModuleFieldType::FuncType:
        WriteFuncType(field->func_type);
        break;
      case ModuleFieldType::Start:
        WriteStartFunction(&field->start);
        break;
    }
  }
  WriteCloseNewline();
  /* force the newline to be written */
  WriteNextChar();
  return result_;
}

Result write_wat(Writer* writer, const Module* module) {
  WatWriter wat_writer(writer);
  return wat_writer.WriteModule(module);
}

}  // namespace wabt
