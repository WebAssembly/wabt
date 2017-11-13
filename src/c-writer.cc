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

#include <cctype>
#include <cinttypes>
#include <map>
#include <set>

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

struct Label {
  Label(const Block& block, size_t type_stack_size)
      : block(block), type_stack_size(type_stack_size) {}
  const Block& block;
  size_t type_stack_size;
};

template <bool is_global>
struct Name {
  explicit Name(const std::string& name) : name(name) {}
  const std::string& name;
};

typedef Name<false> LocalName;
typedef Name<true> GlobalName;

struct GlobalVar {
  explicit GlobalVar(const Var& var) : var(var) {}
  const Var& var;
};

struct StackVar {
  explicit StackVar(Index index, Type type = Type::Any)
      : index(index), type(type) {}
  Index index;
  Type type;
};

struct CopyLabelVar {
  explicit CopyLabelVar(const Label& label) : label(label) {}
  const Label& label;
};

struct TypeEnum {
  explicit TypeEnum(Type type) : type(type) {}
  Type type;
};

struct SignedType {
  explicit SignedType(Type type) : type(type) {}
  Type type;
};

struct ResultType {
  explicit ResultType(const TypeVector& types) : types(types) {}
  const TypeVector& types;
};

struct Newline {
  explicit Newline(bool force = false) : force(force) {}
  bool force = false;
};

struct OpenBrace {};
struct CloseBrace {};

int GetShiftMask(Type type) {
  switch (type) {
    case Type::I32: return 31;
    case Type::I64: return 63;
    default: WABT_UNREACHABLE; return 0;
  }
}

class CWriter {
 public:
  CWriter(Stream* stream, const WriteCOptions* options)
      : options_(options), stream_(stream), module_stream_(stream) {}

  Result WriteModule(const Module&);

 private:
  typedef std::set<std::string> SymbolSet;
  typedef std::map<std::string, std::string> SymbolMap;
  typedef std::pair<Index, Type> StackTypePair;
  typedef std::map<StackTypePair, std::string> StackVarSymbolMap;

  size_t MarkTypeStack() const;
  void ResetTypeStack(size_t mark);
  void PushType(Type);
  void PushTypes(const TypeVector&);
  void DropTypes(size_t count);

  void PushLabel(const Block& block);
  const Label* FindLabel(const Var& var);
  void PopLabel();

  std::string LegalizeName(string_view);
  std::string DefineName(SymbolSet*, string_view);
  std::string DefineGlobalName(const std::string&);
  std::string DefineLocalName(const std::string&);
  std::string DefineStackVarName(Index, Type, string_view);

  void Indent(int size = INDENT_SIZE);
  void Dedent(int size = INDENT_SIZE);
  void WriteIndent();
  void WriteNextChar();
  void WriteDataWithNextChar(const void* src, size_t size);
  void Writef(const char* format, ...);

  template <typename T, typename U, typename... Args>
  void Write(T&& t, U&& u, Args&&... args) {
    Write(std::forward<T>(t));
    Write(std::forward<U>(u));
    Write(std::forward<Args>(args)...);
  }

  void Write() {}
  void Write(Newline);
  void Write(OpenBrace);
  void Write(CloseBrace);
  void Write(Index);
  void Write(const char*);
  void Write(const std::string&);
  void Write(const LocalName&);
  void Write(const GlobalName&);
  void Write(Type);
  void Write(SignedType);
  void Write(TypeEnum);
  void Write(const Var&);
  void Write(const GlobalVar&);
  void Write(const StackVar&);
  void Write(const CopyLabelVar&);
  void Write(const ResultType&);
  void Write(const Const&);
  void WriteInitExpr(const ExprList&);
  void WriteHeader();
  void WriteFuncTypes();
  void WriteFuncDeclarations();
  void WriteGlobals();
  void WriteMemories();
  void WriteTables();
  void WriteDataInitializers();
  void WriteElemInitializers();
  void WriteInit();
  void WriteFuncs();
  void Write(const Func&);
  void WriteParams();
  void WriteLocals();
  void WriteStackVarDeclarations();
  void Write(const Block&);
  void Write(const ExprList&);

  enum class AssignOp {
    Disallowed,
    Allowed,
  };

  void WriteSimpleUnaryExpr(const char* op, Type result_type);
  void WriteSimpleBinaryExpr(const char* op, AssignOp = AssignOp::Allowed);
  void WriteSignedBinaryExpr(const char* op, Type);
  void Write(const BinaryExpr&);
  void Write(const CompareExpr&);
  void Write(const ConvertExpr&);
  void Write(const LoadExpr&);
  void Write(const StoreExpr&);
  void Write(const UnaryExpr&);

  const WriteCOptions* options_ = nullptr;
  const Module* module_ = nullptr;
  const Func* func_ = nullptr;
  Stream* stream_ = nullptr;
  Stream* module_stream_ = nullptr;
  MemoryStream func_stream_;
  Result result_ = Result::Ok;
  int indent_ = 0;
  NextChar next_char_ = NextChar::None;

  SymbolMap global_sym_map_;
  SymbolMap local_sym_map_;
  StackVarSymbolMap stack_var_sym_map_;
  SymbolSet global_syms_;
  SymbolSet local_syms_;
  TypeVector type_stack_;
  std::vector<Label> label_stack_;

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

typedef enum Type { I32, I64, F32, F64 } Type;
typedef void (*Anyfunc)();
typedef struct Elem { u32 func_type; Anyfunc func; } Elem;
typedef struct Memory { u8* data; size_t len; } Memory;
typedef struct Table { Elem* data; size_t len; } Table;

u32 register_func_type(u32 params, u32 results, ...);
void init_data_segment(u32 addr, u8 *data, size_t len);
void init_elem_segment(u32 addr, Elem *data, size_t len);
void init(void);

#define I32LOAD(o) 0
#define I64LOAD(o) 0
#define F32LOAD(o) 0
#define F64LOAD(o) 0
#define I32LOAD8S(o) 0
#define I64LOAD8S(o) 0
#define I32LOAD8U(o) 0
#define i64LOAD8U(o) 0
#define I32LOAD16S(o) 0
#define I64LOAD16S(o) 0
#define I32LOAD16U(o) 0
#define I32LOAD16U(o) 0
#define I64LOAD32S(o) 0
#define I64LOAD32U(o) 0
#define I32STORE(o, v)
#define I64STORE(o, v)
#define F32STORE(o, v)
#define F64STORE(o, v)
#define I32STORE8(o, v)
#define I64STORE8(o, v)
#define I32STORE16(o, v)
#define I64STORE16(o, v)
#define I64STORE32(o, v)
#define CALL_INDIRECT(x, ...) 0
#define UNREACHABLE

)";

size_t CWriter::MarkTypeStack() const {
  return type_stack_.size();
}

void CWriter::ResetTypeStack(size_t mark) {
  assert(mark <= type_stack_.size());
  type_stack_.erase(type_stack_.begin() + mark, type_stack_.end());
}

void CWriter::PushType(Type type) {
  type_stack_.push_back(type);
}

void CWriter::PushTypes(const TypeVector& types) {
  type_stack_.insert(type_stack_.end(), types.begin(), types.end());
}

void CWriter::DropTypes(size_t count) {
  assert(count <= type_stack_.size());
  type_stack_.erase(type_stack_.end() - count, type_stack_.end());
}

void CWriter::PushLabel(const Block& block) {
  label_stack_.emplace_back(block, type_stack_.size());
}

const Label* CWriter::FindLabel(const Var& var) {
  assert(var.is_name());
  for (Index i = label_stack_.size(); i > 0; --i) {
    const Label& label = label_stack_[i - 1];
    if (label.block.label == var.name())
      return &label;
  }
  assert(0);
  return nullptr;
}

void CWriter::PopLabel() {
  label_stack_.pop_back();
}

std::string CWriter::LegalizeName(string_view name) {
  if (name.empty())
    return "_";

  std::string result;
  result = isalpha(name[0]) ? name[0] : '_';
  for (size_t i = 1; i < name.size(); ++i)
    result += isalnum(name[i]) ? name[i] : '_';

  return result;
}

std::string CWriter::DefineName(SymbolSet* set, string_view name) {
  std::string legal = LegalizeName(name);
  if (set->find(legal) != set->end()) {
    std::string base = legal + "_";;
    size_t count = 0;
    do {
      legal = base + std::to_string(count++);
    } while (set->find(legal) != set->end());
  }
  set->insert(legal);
  return legal;
}

string_view StripLeadingDollar(string_view name) {
  assert(!name.empty() && name[0] == '$');
  name.remove_prefix(1);
  return name;
}

std::string CWriter::DefineGlobalName(const std::string& name) {
  std::string unique = DefineName(&global_syms_, StripLeadingDollar(name));
  global_sym_map_.insert(SymbolMap::value_type(name, unique));
  return unique;
}

std::string CWriter::DefineLocalName(const std::string& name) {
  std::string unique = DefineName(&local_syms_, StripLeadingDollar(name));
  local_sym_map_.insert(SymbolMap::value_type(name, unique));
  return unique;
}

std::string CWriter::DefineStackVarName(Index index,
                                        Type type,
                                        string_view name) {
  std::string unique = DefineName(&local_syms_, name);
  StackTypePair stp = {index, type};
  stack_var_sym_map_.insert(StackVarSymbolMap::value_type(stp, unique));
  return unique;
}

void CWriter::Indent(int size) {
  indent_ += size;
}

void CWriter::Dedent(int size) {
  indent_ -= size;
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

void CWriter::Write(Newline newline) {
  if (next_char_ == NextChar::ForceNewline)
    WriteNextChar();
  next_char_ = newline.force ? NextChar::ForceNewline : NextChar::Newline;
}

void CWriter::Write(OpenBrace) {
  Write("{");
  Indent();
  Write(Newline());
}

void CWriter::Write(CloseBrace) {
  Dedent();
  Write(Newline(), "}");
}

void CWriter::Write(Index index) {
  Writef("%" PRIindex, index);
}

void CWriter::Write(const char* s) {
  size_t len = strlen(s);
  WriteDataWithNextChar(s, len);
  next_char_ = NextChar::None;
}

void CWriter::Write(const std::string& s) {
  WriteDataWithNextChar(s.data(), s.size());
  next_char_ = NextChar::None;
}

void CWriter::Write(const LocalName& name) {
  assert(local_sym_map_.count(name.name) == 1);
  Write(local_sym_map_[name.name]);
}

void CWriter::Write(const GlobalName& name) {
  assert(global_sym_map_.count(name.name) == 1);
  Write(global_sym_map_[name.name]);
}

void CWriter::Write(const Var& var) {
  assert(var.is_name());
  Write(LocalName(var.name()));
}

void CWriter::Write(const GlobalVar& var) {
  assert(var.var.is_name());
  Write(GlobalName(var.var.name()));
}

void CWriter::Write(const StackVar& sv) {
  Index index = type_stack_.size() - 1 - sv.index;
  Type type = sv.type;
  if (type == Type::Any) {
    assert(index < type_stack_.size());
    type = type_stack_[index];
  }

  StackTypePair stp = {index, type};
  auto iter = stack_var_sym_map_.find(stp);
  if (iter == stack_var_sym_map_.end()) {
    std::string name;
    switch (type) {
      case Type::I32: name = 'i'; break;
      case Type::I64: name = 'j'; break;
      case Type::F32: name = 'f'; break;
      case Type::F64: name = 'd'; break;
      default: WABT_UNREACHABLE;
    }
    name += std::to_string(index);
    Write(DefineStackVarName(index, type, name));
  } else {
    Write(iter->second);
  }
}

void CWriter::Write(const CopyLabelVar& clv) {
  assert(clv.label.block.sig.size() == 1);
  assert(type_stack_.size() >= clv.label.type_stack_size);
  Write(StackVar(type_stack_.size() - clv.label.type_stack_size - 1,
                 clv.label.block.sig[0]),
        " = ", StackVar(0), ";");
}

void CWriter::Write(Type type) {
  switch (type) {
    case Type::I32: Write("u32"); break;
    case Type::I64: Write("u64"); break;
    case Type::F32: Write("f32"); break;
    case Type::F64: Write("f64"); break;
    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(TypeEnum type) {
  switch (type.type) {
    case Type::I32: Write("I32"); break;
    case Type::I64: Write("I64"); break;
    case Type::F32: Write("F32"); break;
    case Type::F64: Write("F64"); break;
    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(SignedType type) {
  switch (type.type) {
    case Type::I32: Write("s32"); break;
    case Type::I64: Write("s64"); break;
    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const ResultType& rt) {
  if (!rt.types.empty()) {
    Write(rt.types[0]);
  } else {
    Write("void");
  }
}

void CWriter::Write(const Const& const_) {
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
      Write(cast<ConstExpr>(expr)->const_);
      break;

    case ExprType::GetGlobal:
      Write(GlobalVar(cast<GetGlobalExpr>(expr)->var));
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteHeader() {
  Write(s_header);
}

void CWriter::WriteFuncTypes() {
  Writef("static u32 func_types[%" PRIzd "];", module_->func_types.size());
  Write(Newline(), Newline(true));
  Write("static void init_func_types(void) {", Newline());
  Index func_type_index = 0;
  for (FuncType* func_type : module_->func_types) {
    Index num_params = func_type->GetNumParams();
    Index num_results = func_type->GetNumResults();
    Write("  func_types[", func_type_index, "] = register_func_type(",
          num_params, ", ", num_results);
    for (Index i = 0; i < num_params; ++i)
      Write(", ", TypeEnum(func_type->GetParamType(i)));

    for (Index i = 0; i < num_results; ++i)
      Write(", ", TypeEnum(func_type->GetResultType(i)));

    Write(");", Newline());
    ++func_type_index;
  }
  Write("}", Newline(), Newline(true), Newline(true));
}

void CWriter::WriteFuncDeclarations() {
  if (module_->funcs.empty())
    return;

  Index func_index = 0;
  for (Func* func : module_->funcs) {
    bool is_import = func_index < module_->num_func_imports;
    if (!is_import)
      Write("static ");

    Write(ResultType(func->decl.sig.result_types), " ",
          DefineGlobalName(func->name), "(");
    for (Index i = 0; i < func->GetNumParams(); ++i) {
      if (i != 0)
        Write(", ");
      Write(func->GetParamType(i));
    }
    Write(");", Newline());
    ++func_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteGlobals() {
  if (module_->globals.empty())
    return;

  Index global_index = 0;
  for (Global* global : module_->globals) {
    bool is_import = global_index < module_->num_global_imports;
    Write(is_import ? "extern " : "static ");
    Write(global->type, " ", DefineGlobalName(global->name));
    Write(";", Newline());
    ++global_index;
  }
  Write(Newline(true), Newline(true));

  Write("static void init_globals(void) ", OpenBrace());
  global_index = 0;
  for (Global* global : module_->globals) {
    bool is_import = global_index < module_->num_global_imports;
    if (!is_import) {
      assert(!global->init_expr.empty());
      Write(GlobalName(global->name), " = ");
      WriteInitExpr(global->init_expr);
      Write(";", Newline());
    }
    ++global_index;
  }
  Write(CloseBrace(), Newline(), Newline(true), Newline(true));
}

void CWriter::WriteMemories() {
  assert(module_->memories.size() <= 1);
  Index memory_index = 0;
  for (Memory* memory : module_->memories) {
    bool is_import = memory_index < module_->num_memory_imports;
    WABT_USE(memory);
    Write(is_import ? "extern " : "static ");
    Write("Memory mem;", Newline());
    ++memory_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteTables() {
  assert(module_->tables.size() <= 1);
  Index table_index = 0;
  for (Table* table : module_->tables) {
    bool is_import = table_index < module_->num_table_imports;
    WABT_USE(table);
    Write(is_import ? "extern " : "static ");
    Write("Table table;", Newline());
    ++table_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteDataInitializers() {
  if (module_->memories.empty())
    return;

  Index data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    Write("static u8 data_segment_data_", data_segment_index,
          "[] = ", OpenBrace());
    size_t i = 0;
    for (uint8_t x : data_segment->data) {
      Writef("0x%02x, ", x);
      if ((++i % 12) == 0)
        Write(Newline());
    }
    Write(CloseBrace(), ";", Newline(), Newline(true), Newline(true));
    ++data_segment_index;
  }

  Write("static void init_memory(void) ", OpenBrace());
  data_segment_index = 0;
  for (DataSegment* data_segment : module_->data_segments) {
    Write("init_data_segment(");
    WriteInitExpr(data_segment->offset);
    Write(", data_segment_data_", data_segment_index, ", ",
          data_segment->data.size(), ");", Newline());
    ++data_segment_index;
  }
  Write(CloseBrace(), Newline(), Newline(true), Newline(true));
}

void CWriter::WriteElemInitializers() {
  if (module_->tables.empty())
    return;

  Index elem_segment_index = 0;
  for (ElemSegment* elem_segment : module_->elem_segments) {
    Write("static Elem elem_segment_data_", elem_segment_index,
          "[] = ", OpenBrace());

    size_t i = 0;
    for (const Var& var : elem_segment->vars) {
      const Func* func = module_->GetFunc(var);
      Index func_type_index = module_->GetFuncTypeIndex(func->decl.type_var);

      Write("{", func_type_index, ", (Anyfunc)", GlobalName(func->name), "}, ");

      if ((++i % 4) == 0)
        Write(Newline());
    }
    Write(CloseBrace(), ";", Newline(), Newline(true), Newline(true));
    ++elem_segment_index;
  }

  Write("static void init_table(void) ", OpenBrace());
  elem_segment_index = 0;
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    Write("init_elem_segment(");
    WriteInitExpr(elem_segment->offset);
    Write(", elem_segment_data_", elem_segment_index, ", ",
          elem_segment->vars.size(), ");", Newline());
    ++elem_segment_index;
  }
  Write(CloseBrace(), Newline(), Newline(true), Newline(true));
}

void CWriter::WriteInit() {
  Write("void init(void) ", OpenBrace());
  Write("init_func_types();", Newline());
  Write("init_globals();", Newline());
  Write("init_memory();", Newline());
  Write("init_table();", Newline());
  for (Var* var : module_->starts) {
    Write(GlobalName(module_->GetFunc(*var)->name), ";", Newline());
  }
  Write(CloseBrace(), Newline(), Newline(true), Newline(true));
}

void CWriter::WriteFuncs() {
  Index func_index = 0;
  for (const Func* func : module_->funcs) {
    bool is_import = func_index < module_->num_func_imports;
    if (!is_import)
      Write(*func);
    ++func_index;
  }
}

void CWriter::Write(const Func& func) {
  func_ = &func;
  local_syms_.clear();
  local_sym_map_.clear();
  stack_var_sym_map_.clear();

  Write("static ", ResultType(func.decl.sig.result_types), " ",
        GlobalName(func.name), "(");
  WriteParams();
  WriteLocals();

  stream_ = &func_stream_;
  stream_->ClearOffset();

  ResetTypeStack(0);
  Write(func.exprs);
  ResetTypeStack(0);
  PushTypes(func.decl.sig.result_types);

  if (!func.decl.sig.result_types.empty()) {
    // Return the top of the stack implicitly.
    Write("return ", StackVar(0), ";", Newline());
  }

  stream_ = module_stream_;
  WriteStackVarDeclarations();

  std::unique_ptr<OutputBuffer> buf = func_stream_.ReleaseOutputBuffer();
  stream_->WriteData(buf->data.data(), buf->data.size());

  Write(CloseBrace(), Newline(true), Newline(true));

  func_stream_.Clear();
  func_ = nullptr;
}

void CWriter::WriteParams() {
  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func_->decl.sig.param_types,
                                func_->param_bindings, &index_to_name);
  Indent(4);
  for (Index i = 0; i < func_->GetNumParams(); ++i) {
    if (i != 0) {
      Write(", ");
      if ((i % 8) == 0)
        Write(Newline());
    }
    Write(func_->GetParamType(i), " ", DefineLocalName(index_to_name[i]));
  }
  Dedent(4);
  Write(") ", OpenBrace());
}

void CWriter::WriteLocals() {
  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func_->local_types, func_->local_bindings,
                                &index_to_name);
  for (Type type : {Type::I32, Type::I64, Type::F32, Type::F64}) {
    Index local_index = 0;
    size_t count = 0;
    for (Type local_type : func_->local_types) {
      if (local_type == type) {
        if (count == 0) {
          Write(type, " ");
          Indent(4);
        } else {
          Write(", ");
          if ((count % 8) == 0)
            Write(Newline());
        }

        Write(DefineLocalName(index_to_name[local_index]), " = 0");
        ++count;
      }
      ++local_index;
    }
    if (count != 0) {
      Dedent(4);
      Write(";", Newline());
    }
  }
}

void CWriter::WriteStackVarDeclarations() {
  for (Type type : {Type::I32, Type::I64, Type::F32, Type::F64}) {
    size_t count = 0;
    for (const auto& pair: stack_var_sym_map_) {
      Type stp_type = pair.first.second;
      const std::string& name = pair.second;

      if (stp_type == type) {
        if (count == 0) {
          Write(type, " ");
          Indent(4);
        } else {
          Write(", ");
          if ((count % 8) == 0)
            Write(Newline());
        }

        Write(name);
        ++count;
      }
    }
    if (count != 0) {
      Dedent(4);
      Write(";", Newline());
    }
  }
}

void CWriter::Write(const Block& block) {
  size_t mark = MarkTypeStack();
  PushLabel(block);
  Write(OpenBrace(), block.exprs, CloseBrace());
  ResetTypeStack(mark);
  PopLabel();
  PushTypes(block.sig);
}

void CWriter::Write(const ExprList& exprs) {
  for (const Expr& expr: exprs) {
    switch (expr.type()) {
      case ExprType::Binary:
        Write(*cast<BinaryExpr>(&expr));
        break;

      case ExprType::Block: {
        const Block& block = cast<BlockExpr>(&expr)->block;
        std::string label = DefineLocalName(block.label);
        Write(block, " ", label, ":;", Newline());
        break;
      }

      case ExprType::Br: {
        const Var& var = cast<BrExpr>(&expr)->var;
        const Label* label = FindLabel(var);
        if (!label->block.sig.empty())
          Write(CopyLabelVar(*label), " ");
        Write("goto ", var, ";", Newline());
        // Stop processing this ExprList, since the following are unreachable.
        return;
      }

      case ExprType::BrIf: {
        const Var& var = cast<BrIfExpr>(&expr)->var;
        const Label* label = FindLabel(var);
        Write("if (", StackVar(0), ") ");
        if (!label->block.sig.empty()) {
          Write("{", CopyLabelVar(*label), " goto ", var, ";}", Newline());
        } else {
          Write("goto ", var, ";", Newline());
        }
        DropTypes(1);
        break;
      }

      case ExprType::BrTable: {
        const auto* bt_expr = cast<BrTableExpr>(&expr);
        Write("switch (", StackVar(0), ") ", OpenBrace());
        DropTypes(1);
        Index i = 0;
        const Label* label;
        for (const Var& var : bt_expr->targets) {
          label = FindLabel(var);
          Write(Newline(), "case ", i, ": ");
          if (!label->block.sig.empty())
            Write(CopyLabelVar(*label), "; ");
          Write("goto ", var, ";");
          ++i;
        }

        label = FindLabel(bt_expr->default_target);
        Write(Newline(), "default: ");
        if (!label->block.sig.empty())
          Write(CopyLabelVar(*label), "; ");
        Write("goto ", bt_expr->default_target, ";", CloseBrace(), Newline());
        break;
      }

      case ExprType::Call: {
        const Var& var = cast<CallExpr>(&expr)->var;
        const Func& func = *module_->GetFunc(var);
        Index num_params = func.GetNumParams();
        Index num_results = func.GetNumResults();
        assert(type_stack_.size() >= num_params);
        if (num_results > 0) {
          assert(num_results == 1);
          Write(StackVar(num_params - 1, func.GetResultType(0)), " = ");
        }

        Write(GlobalVar(var), "(");
        for (Index i = 0; i < num_params; ++i) {
          if (i != 0)
            Write(", ");
          Write(StackVar(num_params - i - 1));
        }
        Write(");", Newline());
        DropTypes(num_params);
        PushTypes(func.decl.sig.result_types);
        break;
      }

      case ExprType::CallIndirect: {
        const FuncDeclaration& decl = cast<CallIndirectExpr>(&expr)->decl;
        Index num_params = decl.GetNumParams();
        Index num_results = decl.GetNumResults();
        assert(type_stack_.size() > num_params);
        if (num_results > 0) {
          assert(num_results == 1);
          Write(StackVar(num_params, decl.GetResultType(0)), " = ");
        }

        Write("CALL_INDIRECT(", StackVar(num_params));
        for (Index i = 0; i < num_params; ++i) {
          Write(", ", StackVar(num_params - i - 1));
        }
        Write(");", Newline());
        DropTypes(num_params + 1);
        PushTypes(decl.sig.result_types);
        break;
      }

      case ExprType::Compare:
        Write(*cast<CompareExpr>(&expr));
        break;

      case ExprType::Const: {
        const Const& const_ = cast<ConstExpr>(&expr)->const_;
        PushType(const_.type);
        Write(StackVar(0), " = ", const_, ";", Newline());
        break;
      }

      case ExprType::Convert:
        Write(*cast<ConvertExpr>(&expr));
        break;

      case ExprType::CurrentMemory:
        assert(0);
        break;

      case ExprType::Drop:
        DropTypes(1);
        break;

      case ExprType::GetGlobal: {
        const Var& var = cast<GetGlobalExpr>(&expr)->var;
        PushType(module_->GetGlobal(var)->type);
        Write(StackVar(0), " = ", GlobalVar(var), ";", Newline());
        break;
      }

      case ExprType::GetLocal: {
        const Var& var = cast<GetLocalExpr>(&expr)->var;
        PushType(func_->GetLocalType(var));
        Write(StackVar(0), " = ", var, ";", Newline());
        break;
      }

      case ExprType::GrowMemory:
        assert(0);
        break;

      case ExprType::If: {
        const IfExpr& if_ = *cast<IfExpr>(&expr);
        Write("if (", StackVar(0), ") ", OpenBrace());
        DropTypes(1);
        size_t mark = MarkTypeStack();
        PushLabel(if_.true_);
        Write(if_.true_.exprs, CloseBrace());
        if (!if_.false_.empty()) {
          ResetTypeStack(mark);
          Write(" else ", OpenBrace(), if_.false_, CloseBrace());
        }
        ResetTypeStack(mark);
        PopLabel();
        Write(Newline());
        PushTypes(if_.true_.sig);
        break;
      }

      case ExprType::Load:
        Write(*cast<LoadExpr>(&expr));
        break;

      case ExprType::Loop: {
        const Block& block = cast<LoopExpr>(&expr)->block;
        Write(DefineLocalName(block.label), ": ", block, Newline());
        break;
      }

      case ExprType::Nop:
        break;

      case ExprType::Return:
        Write("return");
        if (!func_->decl.sig.result_types.empty())
          Write(" ", StackVar(0));
        Write(";", Newline());
        return;

      case ExprType::Select:
        assert(0);
        break;

      case ExprType::SetGlobal: {
        const Var& var = cast<SetGlobalExpr>(&expr)->var;
        Write(GlobalVar(var), " = ", StackVar(0), ";", Newline());
        DropTypes(1);
        break;
      }

      case ExprType::SetLocal: {
        const Var& var = cast<SetLocalExpr>(&expr)->var;
        Write(var, " = ", StackVar(0), ";", Newline());
        DropTypes(1);
        break;
      }

      case ExprType::Store:
        Write(*cast<StoreExpr>(&expr));
        break;

      case ExprType::TeeLocal: {
        const Var& var = cast<TeeLocalExpr>(&expr)->var;
        Write(var, " = ", StackVar(0), ";", Newline());
        break;
      }

      case ExprType::Unary:
        Write(*cast<UnaryExpr>(&expr));
        break;

      case ExprType::Unreachable:
        Write("UNREACHABLE;", Newline());
        return;

      case ExprType::AtomicLoad:
      case ExprType::AtomicRmw:
      case ExprType::AtomicRmwCmpxchg:
      case ExprType::AtomicStore:
      case ExprType::Rethrow:
      case ExprType::Throw:
      case ExprType::TryBlock:
      case ExprType::Wait:
      case ExprType::Wake:
        assert(0);
        break;
    }
  }
}

void CWriter::WriteSimpleUnaryExpr(const char* op, Type result_type) {
  Write(StackVar(0, result_type), " = ", op, "(", StackVar(0), ");", Newline());
}

void CWriter::WriteSimpleBinaryExpr(const char* op, AssignOp assign_op) {
  Write(StackVar(1));
  if (assign_op == AssignOp::Allowed) {
    Write(" ", op, "= ", StackVar(0));
  } else {
    Write(" = ", StackVar(1), " ", op, " ", StackVar(0));
  }
  Write(";", Newline());
  DropTypes(1);
}

void CWriter::WriteSignedBinaryExpr(const char* op, Type type) {
  const char *utype, *stype;
  if (type == Type::I32) {
    utype = "u32";
    stype = "s32";
  } else {
    utype = "u64";
    stype = "s64";
  }
  Write(StackVar(1), " = (", utype, ")((", stype, ")", StackVar(1), " ", op,
        " (", stype, ")", StackVar(0), ");", Newline());
  DropTypes(1);
}

void CWriter::Write(const BinaryExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Add:
    case Opcode::I64Add:
    case Opcode::F32Add:
    case Opcode::F64Add:
      WriteSimpleBinaryExpr("+");
      break;

    case Opcode::I32Sub:
    case Opcode::I64Sub:
    case Opcode::F32Sub:
    case Opcode::F64Sub:
      WriteSimpleBinaryExpr("-");
      break;

    case Opcode::I32Mul:
    case Opcode::I64Mul:
    case Opcode::F32Mul:
    case Opcode::F64Mul:
      WriteSimpleBinaryExpr("*");
      break;

    case Opcode::I32DivS:
    case Opcode::I64DivS:
      assert(0);
      break;

    case Opcode::I32DivU:
    case Opcode::I64DivU:
    case Opcode::F32Div:
    case Opcode::F64Div:
      assert(0);
      break;

    case Opcode::I32RemS:
    case Opcode::I64RemS:
      assert(0);
      break;

    case Opcode::I32RemU:
    case Opcode::I64RemU:
      assert(0);
      break;

    case Opcode::I32And:
    case Opcode::I64And:
      WriteSimpleBinaryExpr("&");
      break;

    case Opcode::I32Or:
    case Opcode::I64Or:
      WriteSimpleBinaryExpr("|");
      break;

    case Opcode::I32Xor:
    case Opcode::I64Xor:
      WriteSimpleBinaryExpr("^");
      break;

    case Opcode::I32Shl:
    case Opcode::I64Shl:
      Write(StackVar(1), " <<= (", StackVar(0), " & ",
            GetShiftMask(expr.opcode.GetResultType()), ");", Newline());
      DropTypes(1);
      break;

    case Opcode::I32ShrS:
    case Opcode::I64ShrS: {
      Type type = expr.opcode.GetResultType();
      Write(StackVar(1), " = (", type, ")((", SignedType(type), ")",
            StackVar(1), " >> (", StackVar(0), " & ", GetShiftMask(type), "));",
            Newline());
      DropTypes(1);
      break;
    }

    case Opcode::I32ShrU:
    case Opcode::I64ShrU:
      Write(StackVar(1), " >>= (", StackVar(0), " & ",
            GetShiftMask(expr.opcode.GetResultType()), ");", Newline());
      DropTypes(1);
      break;

    case Opcode::I32Rotl:
    case Opcode::I64Rotl:
      assert(0);
      break;

    case Opcode::I32Rotr:
    case Opcode::I64Rotr:
      assert(0);
      break;

    case Opcode::F32Min:
    case Opcode::F64Min:
      assert(0);
      break;

    case Opcode::F32Max:
    case Opcode::F64Max:
      assert(0);
      break;

    case Opcode::F32Copysign:
    case Opcode::F64Copysign:
      assert(0);
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const CompareExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Eq:
    case Opcode::I64Eq:
    case Opcode::F32Eq:
    case Opcode::F64Eq:
      WriteSimpleBinaryExpr("==", AssignOp::Disallowed);
      break;

    case Opcode::I32Ne:
    case Opcode::I64Ne:
    case Opcode::F32Ne:
    case Opcode::F64Ne:
      WriteSimpleBinaryExpr("!=", AssignOp::Disallowed);
      break;

    case Opcode::I32LtS:
    case Opcode::I64LtS:
      WriteSignedBinaryExpr("<", expr.opcode.GetResultType());
      break;

    case Opcode::I32LtU:
    case Opcode::I64LtU:
    case Opcode::F32Lt:
    case Opcode::F64Lt:
      WriteSimpleBinaryExpr("<", AssignOp::Disallowed);
      break;

    case Opcode::I32LeS:
    case Opcode::I64LeS:
      WriteSignedBinaryExpr("<=", expr.opcode.GetResultType());
      break;

    case Opcode::I32LeU:
    case Opcode::I64LeU:
    case Opcode::F32Le:
    case Opcode::F64Le:
      WriteSimpleBinaryExpr("<=", AssignOp::Disallowed);
      break;

    case Opcode::I32GtS:
    case Opcode::I64GtS:
      WriteSignedBinaryExpr(">", expr.opcode.GetResultType());
      break;

    case Opcode::I32GtU:
    case Opcode::I64GtU:
    case Opcode::F32Gt:
    case Opcode::F64Gt:
      WriteSimpleBinaryExpr(">", AssignOp::Disallowed);
      break;

    case Opcode::I32GeS:
    case Opcode::I64GeS:
      WriteSignedBinaryExpr(">=", expr.opcode.GetResultType());
      break;

    case Opcode::I32GeU:
    case Opcode::I64GeU:
    case Opcode::F32Ge:
    case Opcode::F64Ge:
      WriteSimpleBinaryExpr(">=", AssignOp::Disallowed);
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const ConvertExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Eqz:
    case Opcode::I64Eqz:
      WriteSimpleUnaryExpr("!", expr.opcode.GetResultType());
      break;

    case Opcode::I64ExtendSI32:
    case Opcode::I64ExtendUI32:
    case Opcode::I32WrapI64:
    case Opcode::I32TruncSF32:
    case Opcode::I64TruncSF32:
    case Opcode::I32TruncSF64:
    case Opcode::I64TruncSF64:
    case Opcode::I32TruncUF32:
    case Opcode::I64TruncUF32:
    case Opcode::I32TruncUF64:
    case Opcode::I64TruncUF64:
    case Opcode::I32TruncSSatF32:
    case Opcode::I64TruncSSatF32:
    case Opcode::I32TruncSSatF64:
    case Opcode::I64TruncSSatF64:
    case Opcode::I32TruncUSatF32:
    case Opcode::I64TruncUSatF32:
    case Opcode::I32TruncUSatF64:
    case Opcode::I64TruncUSatF64:
    case Opcode::F32ConvertSI32:
    case Opcode::F64ConvertSI32:
    case Opcode::F32ConvertSI64:
    case Opcode::F64ConvertSI64:
    case Opcode::F32ConvertUI32:
    case Opcode::F64ConvertUI32:
    case Opcode::F32ConvertUI64:
    case Opcode::F64ConvertUI64:
    case Opcode::F64PromoteF32:
    case Opcode::F32DemoteF64:
    case Opcode::F32ReinterpretI32:
    case Opcode::I32ReinterpretF32:
    case Opcode::F64ReinterpretI64:
    case Opcode::I64ReinterpretF64:
      assert(0);
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const LoadExpr& expr) {
  const char* macro = nullptr;
  switch (expr.opcode) {
    case Opcode::I32Load: macro = "I32LOAD"; break;
    case Opcode::I64Load: macro = "I64LOAD"; break;
    case Opcode::F32Load: macro = "F32LOAD"; break;
    case Opcode::F64Load: macro = "F64LOAD"; break;
    case Opcode::I32Load8S: macro = "I32LOAD8S"; break;
    case Opcode::I64Load8S: macro = "I64LOAD8S"; break;
    case Opcode::I32Load8U: macro = "I32LOAD8U"; break;
    case Opcode::I64Load8U: macro = "i64LOAD8U"; break;
    case Opcode::I32Load16S: macro = "I32LOAD16S"; break;
    case Opcode::I64Load16S: macro = "I64LOAD16S"; break;
    case Opcode::I32Load16U: macro = "I32LOAD16U"; break;
    case Opcode::I64Load16U: macro = "I32LOAD16U"; break;
    case Opcode::I64Load32S: macro = "I64LOAD32S"; break;
    case Opcode::I64Load32U: macro = "I64LOAD32U"; break;

    default:
      WABT_UNREACHABLE;
  }

  Write(StackVar(0, expr.opcode.GetResultType()), " = ", macro, "(",
        StackVar(0));
  if (expr.offset != 0)
    Write(" + ", expr.offset);
  Write(");", Newline());
}

void CWriter::Write(const StoreExpr& expr) {
  const char* macro = nullptr;
  switch (expr.opcode) {
    case Opcode::I32Store: macro = "I32STORE"; break;
    case Opcode::I64Store: macro = "I64STORE"; break;
    case Opcode::F32Store: macro = "F32STORE"; break;
    case Opcode::F64Store: macro = "F64STORE"; break;
    case Opcode::I32Store8: macro = "I32STORE8"; break;
    case Opcode::I64Store8: macro = "I64STORE8"; break;
    case Opcode::I32Store16: macro = "I32STORE16"; break;
    case Opcode::I64Store16: macro = "I64STORE16"; break;
    case Opcode::I64Store32: macro = "I64STORE32"; break;

    default:
      WABT_UNREACHABLE;
  }

  Write(macro, "(", StackVar(1));
  if (expr.offset != 0)
    Write(" + ", expr.offset);
  Write(", ", StackVar(0), ");", Newline());
  DropTypes(2);
}

void CWriter::Write(const UnaryExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Clz:
    case Opcode::I64Clz:
    case Opcode::I32Ctz:
    case Opcode::I64Ctz:
    case Opcode::I32Popcnt:
    case Opcode::I64Popcnt:
    case Opcode::F32Neg:
    case Opcode::F64Neg:
    case Opcode::F32Abs:
    case Opcode::F64Abs:
    case Opcode::F32Sqrt:
    case Opcode::F64Sqrt:
    case Opcode::F32Ceil:
    case Opcode::F64Ceil:
    case Opcode::F32Floor:
    case Opcode::F64Floor:
    case Opcode::F32Trunc:
    case Opcode::F64Trunc:
    case Opcode::F32Nearest:
    case Opcode::F64Nearest:
    case Opcode::I32Extend8S:
    case Opcode::I32Extend16S:
    case Opcode::I64Extend8S:
    case Opcode::I64Extend16S:
    case Opcode::I64Extend32S:
      assert(0);
      break;

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
  WriteFuncTypes();
  WriteFuncDeclarations();
  WriteGlobals();
  WriteMemories();
  WriteTables();
  WriteFuncs();
  WriteDataInitializers();
  WriteElemInitializers();
  WriteInit();

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
