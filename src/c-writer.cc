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
#include <string_view>

#include "src/cast.h"
#include "src/common.h"
#include "src/ir.h"
#include "src/literal.h"
#include "src/stream.h"
#include "src/string-util.h"

#define INDENT_SIZE 2

#define UNIMPLEMENTED(x) printf("unimplemented: %s\n", (x)), abort()

namespace wabt {

namespace {

struct Label {
  Label(LabelType label_type,
        const std::string& name,
        const TypeVector& sig,
        size_t type_stack_size,
        bool used = false)
      : label_type(label_type),
        name(name),
        sig(sig),
        type_stack_size(type_stack_size),
        used(used) {}

  bool HasValue() const { return !sig.empty(); }

  LabelType label_type;
  const std::string& name;
  const TypeVector& sig;
  size_t type_stack_size;
  bool used = false;
};

template <int>
struct Name {
  explicit Name(const std::string& name) : name(name) {}
  const std::string& name;
};

typedef Name<0> LocalName;
typedef Name<1> GlobalName;
typedef Name<2> ExternalPtr;
typedef Name<3> ExternalRef;
typedef Name<4> ExternalInstancePtr;
typedef Name<5> ExternalInstanceRef;

struct GotoLabel {
  explicit GotoLabel(const Var& var) : var(var) {}
  const Var& var;
};

struct LabelDecl {
  explicit LabelDecl(const std::string& name) : name(name) {}
  std::string name;
};

struct GlobalVar {
  explicit GlobalVar(const Var& var) : var(var) {}
  const Var& var;
};

struct GlobalInstanceVar {
  explicit GlobalInstanceVar(const Var& var) : var(var) {}
  const Var& var;
};

struct StackVar {
  explicit StackVar(Index index, Type type = Type::Any)
      : index(index), type(type) {}
  Index index;
  Type type;
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

struct Newline {};
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
  CWriter(Stream* c_stream,
          Stream* h_stream,
          const char* header_name,
          const WriteCOptions& options)
      : options_(options),
        c_stream_(c_stream),
        h_stream_(h_stream),
        header_name_(header_name),
        module_name_(header_name_) {
    if ((module_name_.size() > 2) &&
        (module_name_.substr(module_name_.size() - 2) == ".h")) {
      module_name_.erase(module_name_.size() - 2);
    }
  }

  Result WriteModule(const Module&);

 private:
  typedef std::set<std::string> SymbolSet;
  typedef std::map<std::string, std::string> SymbolMap;
  typedef std::pair<Index, Type> StackTypePair;
  typedef std::map<StackTypePair, std::string> StackVarSymbolMap;

  void UseStream(Stream*);

  void WriteCHeader();
  void WriteCSource();

  size_t MarkTypeStack() const;
  void ResetTypeStack(size_t mark);
  Type StackType(Index) const;
  void PushType(Type);
  void PushTypes(const TypeVector&);
  void DropTypes(size_t count);

  void PushLabel(LabelType,
                 const std::string& name,
                 const FuncSignature&,
                 bool used = false);
  const Label* FindLabel(const Var& var);
  bool IsTopLabelUsed() const;
  void PopLabel();

  static std::string AddressOf(const std::string&);
  static std::string Deref(const std::string&);

  static char MangleType(Type);
  static std::string MangleMultivalueTypes(const TypeVector&);
  static std::string MangleName(std::string_view);
  static std::string MangleModuleInstanceName(const std::string& module_name);
  static std::string MangleModuleInstanceTypeName(
      const std::string& module_name);
  static std::string LegalizeName(std::string_view);
  static std::string ExternalName(std::string_view module_name,
                                  std::string_view mangled_name);
  std::string DefineName(SymbolSet*, std::string_view);
  std::string DefineImportName(const std::string& name,
                               std::string_view module_name,
                               std::string_view mangled_field_name);
  std::string DefineImportInstanceName(const std::string& name,
                                       std::string_view module_name,
                                       std::string_view mangled_field_name);
  std::string DefineGlobalScopeName(const std::string&);
  std::string DefineLocalScopeName(const std::string&);
  std::string DefineStackVarName(Index, Type, std::string_view);

  void Indent(int size = INDENT_SIZE);
  void Dedent(int size = INDENT_SIZE);
  void WriteIndent();
  void WriteData(const void* src, size_t size);
  void Writef(const char* format, ...);

  template <typename T, typename U, typename... Args>
  void Write(T&& t, U&& u, Args&&... args) {
    Write(std::forward<T>(t));
    Write(std::forward<U>(u));
    Write(std::forward<Args>(args)...);
  }

  std::string GetGlobalName(const std::string&) const;

  enum class WriteExportsKind {
    Declarations,
    Definitions,
  };

  void Write() {}
  void Write(Newline);
  void Write(OpenBrace);
  void Write(CloseBrace);
  void Write(Index);
  void Write(std::string_view);
  void Write(const LocalName&);
  void Write(const GlobalName&);
  void Write(const ExternalPtr&);
  void Write(const ExternalRef&);
  void Write(const ExternalInstancePtr&);
  void Write(const ExternalInstanceRef&);
  void Write(Type);
  void Write(SignedType);
  void Write(TypeEnum);
  void Write(const Var&);
  void Write(const GotoLabel&);
  void Write(const LabelDecl&);
  void Write(const GlobalVar&);
  void Write(const GlobalInstanceVar&);
  void Write(const StackVar&);
  void Write(const ResultType&);
  void Write(const Const&);
  void WriteInitExpr(const ExprList&);
  std::string GenerateHeaderGuard() const;
  void WriteSourceTop();
  void WriteMultivalueTypes();
  void WriteFuncTypes();
  void ComputeUniqueImports();
  void BeginInstance();
  void WriteImports();
  void WriteFuncDeclarations();
  void WriteFuncDeclaration(const FuncDeclaration&, const std::string&);
  void WriteImportFuncDeclaration(const FuncDeclaration&,
                                  const std::string& module_name,
                                  const std::string&);
  void WriteCallIndirectFuncDeclaration(const FuncDeclaration&,
                                        const std::string&);
  void WriteModuleInstance();
  void WriteGlobals();
  void WriteGlobal(const Global&, const std::string&);
  void WriteGlobalExportDecl(const Global&, const std::string&);
  void WriteMemories();
  void WriteMemory(const std::string&);
  void WriteMemoryExportDecl(const std::string&);
  void WriteTables();
  void WriteTable(const std::string&);
  void WriteTableExportDecl(const std::string&);
  void WriteDataInstances();
  void WriteElemInstances();
  void WriteGlobalInitializers();
  void WriteDataInitializers();
  void WriteElemInitializers();
  void WriteExports(WriteExportsKind);
  void WriteInitDecl();
  void WriteInit();
  void WriteModuleInstanceFreeDecl();
  void WriteModuleInstanceFree();
  void WriteInitInstanceImport();
  void WriteFuncs();
  void Write(const Func&);
  void WriteParamsAndLocals();
  void WriteParams(const std::vector<std::string>& index_to_name);
  void WriteParamSymbols(const std::vector<std::string>& index_to_name);
  void WriteParamTypes(const FuncDeclaration& decl);
  void WriteLocals(const std::vector<std::string>& index_to_name);
  void WriteStackVarDeclarations();
  void Write(const ExprList&);

  enum class AssignOp {
    Disallowed,
    Allowed,
  };

  void WriteSimpleUnaryExpr(Opcode, const char* op);
  void WriteInfixBinaryExpr(Opcode,
                            const char* op,
                            AssignOp = AssignOp::Allowed);
  void WritePrefixBinaryExpr(Opcode, const char* op);
  void WriteSignedBinaryExpr(Opcode, const char* op);
  void Write(const BinaryExpr&);
  void Write(const CompareExpr&);
  void Write(const ConvertExpr&);
  void Write(const LoadExpr&);
  void Write(const StoreExpr&);
  void Write(const UnaryExpr&);
  void Write(const TernaryExpr&);
  void Write(const SimdLaneOpExpr&);
  void Write(const SimdLoadLaneExpr&);
  void Write(const SimdStoreLaneExpr&);
  void Write(const SimdShuffleOpExpr&);
  void Write(const LoadSplatExpr&);
  void Write(const LoadZeroExpr&);

  const WriteCOptions& options_;
  const Module* module_ = nullptr;
  const Func* func_ = nullptr;
  Stream* stream_ = nullptr;
  MemoryStream func_stream_;
  Stream* c_stream_ = nullptr;
  Stream* h_stream_ = nullptr;
  std::string header_name_;
  std::string module_name_;
  Result result_ = Result::Ok;
  int indent_ = 0;
  bool should_write_indent_next_ = false;

  SymbolMap global_sym_map_;
  SymbolMap local_sym_map_;
  SymbolMap import_module_sym_map_;
  StackVarSymbolMap stack_var_sym_map_;
  SymbolSet global_syms_;
  SymbolSet local_syms_;
  SymbolSet import_syms_;
  TypeVector type_stack_;
  std::vector<Label> label_stack_;

  std::vector<const Import*> unique_imports_;
  SymbolSet import_module_set_;       // modules that are imported from
  SymbolSet import_func_module_set_;  // modules that funcs are imported from
};

static const char kImplicitFuncLabel[] = "$Bfunc";

#define SECTION_NAME(x) s_header_##x
#include "src/prebuilt/wasm2c.include.h"
#undef SECTION_NAME

#define SECTION_NAME(x) s_source_##x
#include "src/prebuilt/wasm2c.include.c"
#undef SECTION_NAME

size_t CWriter::MarkTypeStack() const {
  return type_stack_.size();
}

void CWriter::ResetTypeStack(size_t mark) {
  assert(mark <= type_stack_.size());
  type_stack_.erase(type_stack_.begin() + mark, type_stack_.end());
}

Type CWriter::StackType(Index index) const {
  assert(index < type_stack_.size());
  return *(type_stack_.rbegin() + index);
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

void CWriter::PushLabel(LabelType label_type,
                        const std::string& name,
                        const FuncSignature& sig,
                        bool used) {
  if (label_type == LabelType::Loop)
    label_stack_.emplace_back(label_type, name, sig.param_types,
                              type_stack_.size(), used);
  else
    label_stack_.emplace_back(label_type, name, sig.result_types,
                              type_stack_.size(), used);
}

const Label* CWriter::FindLabel(const Var& var) {
  Label* label = nullptr;

  if (var.is_index()) {
    // We've generated names for all labels, so we should only be using an
    // index when branching to the implicit function label, which can't be
    // named.
    assert(var.index() + 1 == label_stack_.size());
    label = &label_stack_[0];
  } else {
    assert(var.is_name());
    for (Index i = label_stack_.size(); i > 0; --i) {
      label = &label_stack_[i - 1];
      if (label->name == var.name())
        break;
    }
  }

  assert(label);
  label->used = true;
  return label;
}

bool CWriter::IsTopLabelUsed() const {
  assert(!label_stack_.empty());
  return label_stack_.back().used;
}

void CWriter::PopLabel() {
  label_stack_.pop_back();
}

// static
std::string CWriter::AddressOf(const std::string& s) {
  return "(&" + s + ")";
}

// static
std::string CWriter::Deref(const std::string& s) {
  return "(*" + s + ")";
}

// static
char CWriter::MangleType(Type type) {
  switch (type) {
    case Type::I32: return 'i';
    case Type::I64: return 'j';
    case Type::F32: return 'f';
    case Type::F64: return 'd';
    default: WABT_UNREACHABLE;
  }
}

// static
std::string CWriter::MangleMultivalueTypes(const TypeVector& types) {
  assert(types.size() >= 2);
  std::string result = "wasm_multi_";
  for (auto type : types) {
    result += MangleType(type);
  }
  return result;
}

// static
std::string CWriter::MangleName(std::string_view name) {
  const char kPrefix = 'Z';
  std::string result = "Z_";

  if (!name.empty()) {
    for (unsigned char c : name) {
      if ((isalnum(c) && c != kPrefix) || c == '_') {
        result += c;
      } else {
        result += kPrefix;
        result += StringPrintf("%02X", static_cast<uint8_t>(c));
      }
    }
  }

  return result;
}

// static
std::string CWriter::MangleModuleInstanceName(const std::string& module_name) {
  return MangleName(module_name) + "_module_instance";
}

// static
std::string CWriter::MangleModuleInstanceTypeName(
    const std::string& module_name) {
  return MangleName(module_name) + "_module_instance_t";
}

// static
std::string CWriter::ExternalName(std::string_view module_name,
                                  std::string_view name) {
  return MangleName(module_name) + "_" + MangleName(name);
}

// static
std::string CWriter::LegalizeName(std::string_view name) {
  if (name.empty())
    return "_";

  std::string result;
  result = isalpha(static_cast<unsigned char>(name[0])) ? name[0] : '_';
  for (size_t i = 1; i < name.size(); ++i)
    result += isalnum(static_cast<unsigned char>(name[i])) ? name[i] : '_';

  // In addition to containing valid characters for C, we must also avoid
  // colliding with things C cares about, such as reserved words (e.g. "void")
  // or a function name like main() (which a compiler will  complain about if we
  // define it with another type). To avoid such problems, prefix.
  result = "w2c_" + result;

  return result;
}

std::string CWriter::DefineName(SymbolSet* set, std::string_view name) {
  std::string legal = LegalizeName(name);
  if (set->find(legal) != set->end()) {
    std::string base = legal + "_";
    size_t count = 0;
    do {
      legal = base + std::to_string(count++);
    } while (set->find(legal) != set->end());
  }
  set->insert(legal);
  return legal;
}

std::string_view StripLeadingDollar(std::string_view name) {
  if (!name.empty() && name[0] == '$') {
    name.remove_prefix(1);
  }
  return name;
}

std::string CWriter::DefineImportName(const std::string& name,
                                      std::string_view module,
                                      std::string_view field_name) {
  std::string mangled = ExternalName(module, field_name);
  mangled = MangleName(module_name_) + "_" + mangled;
  import_syms_.insert(name);
  global_syms_.insert(mangled);
  global_sym_map_.insert(SymbolMap::value_type(name, mangled));
  return mangled;
}

std::string CWriter::DefineImportInstanceName(const std::string& name,
                                              std::string_view module,
                                              std::string_view field_name) {
  std::string mangled = ExternalName(module, field_name);
  import_syms_.insert(name);
  global_syms_.insert(mangled);
  global_sym_map_.insert(SymbolMap::value_type(name, mangled));
  return "(*" + mangled + ")";
}

std::string CWriter::DefineGlobalScopeName(const std::string& name) {
  std::string unique = DefineName(&global_syms_, StripLeadingDollar(name));
  global_sym_map_.insert(SymbolMap::value_type(name, unique));
  return unique;
}

std::string CWriter::DefineLocalScopeName(const std::string& name) {
  std::string unique = DefineName(&local_syms_, StripLeadingDollar(name));
  local_sym_map_.insert(SymbolMap::value_type(name, unique));
  return unique;
}

std::string CWriter::DefineStackVarName(Index index,
                                        Type type,
                                        std::string_view name) {
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

void CWriter::WriteData(const void* src, size_t size) {
  if (should_write_indent_next_) {
    WriteIndent();
    should_write_indent_next_ = false;
  }
  stream_->WriteData(src, size);
}

void WABT_PRINTF_FORMAT(2, 3) CWriter::Writef(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  WriteData(buffer, length);
}

void CWriter::Write(Newline) {
  Write("\n");
  should_write_indent_next_ = true;
}

void CWriter::Write(OpenBrace) {
  Write("{");
  Indent();
  Write(Newline());
}

void CWriter::Write(CloseBrace) {
  Dedent();
  Write("}");
}

void CWriter::Write(Index index) {
  Writef("%" PRIindex, index);
}

void CWriter::Write(std::string_view s) {
  WriteData(s.data(), s.size());
}

void CWriter::Write(const LocalName& name) {
  assert(local_sym_map_.count(name.name) == 1);
  Write(local_sym_map_[name.name]);
}

std::string CWriter::GetGlobalName(const std::string& name) const {
  assert(global_sym_map_.count(name) == 1);
  auto iter = global_sym_map_.find(name);
  assert(iter != global_sym_map_.end());
  return iter->second;
}

void CWriter::Write(const GlobalName& name) {
  Write(GetGlobalName(name.name));
}

void CWriter::Write(const ExternalPtr& name) {
  bool is_import = import_syms_.count(name.name) != 0;
  if (is_import) {
    Write(GetGlobalName(name.name));
  } else {
    Write(AddressOf(GetGlobalName(name.name)));
  }
}

void CWriter::Write(const ExternalInstancePtr& name) {
  bool is_import = import_syms_.count(name.name) != 0;
  if (is_import) {
    Write("module_instance->", GetGlobalName(name.name));
  } else {
    Write("&module_instance->", GetGlobalName(name.name));
  }
}

void CWriter::Write(const ExternalRef& name) {
  Write(GetGlobalName(name.name));
}

void CWriter::Write(const ExternalInstanceRef& name) {
  bool is_import = import_syms_.count(name.name) != 0;
  if (is_import) {
    Write(Deref("module_instance->" + GetGlobalName(name.name)));
  } else {
    Write("module_instance->", GetGlobalName(name.name));
  }
}

void CWriter::Write(const Var& var) {
  assert(var.is_name());
  Write(LocalName(var.name()));
}

void CWriter::Write(const GotoLabel& goto_label) {
  const Label* label = FindLabel(goto_label.var);
  if (label->HasValue()) {
    size_t amount = label->sig.size();
    assert(type_stack_.size() >= label->type_stack_size);
    assert(type_stack_.size() >= amount);
    assert(type_stack_.size() - amount >= label->type_stack_size);
    Index offset = type_stack_.size() - label->type_stack_size - amount;
    if (offset != 0) {
      for (Index i = 0; i < amount; ++i) {
        Write(StackVar(amount - i - 1 + offset, label->sig[i]), " = ",
              StackVar(amount - i - 1), "; ");
      }
    }
  }

  if (goto_label.var.is_name()) {
    Write("goto ", goto_label.var, ";");
  } else {
    // We've generated names for all labels, so we should only be using an
    // index when branching to the implicit function label, which can't be
    // named.
    Write("goto ", Var(kImplicitFuncLabel), ";");
  }
}

void CWriter::Write(const LabelDecl& label) {
  if (IsTopLabelUsed())
    Write(label.name, ":;", Newline());
}

void CWriter::Write(const GlobalVar& var) {
  assert(var.var.is_name());
  Write(ExternalRef(var.var.name()));
}

void CWriter::Write(const GlobalInstanceVar& var) {
  assert(var.var.is_name());
  Write(ExternalInstanceRef(var.var.name()));
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
    std::string name = MangleType(type) + std::to_string(index);
    Write(DefineStackVarName(index, type, name));
  } else {
    Write(iter->second);
  }
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
    case Type::I32: Write("WASM_RT_I32"); break;
    case Type::I64: Write("WASM_RT_I64"); break;
    case Type::F32: Write("WASM_RT_F32"); break;
    case Type::F64: Write("WASM_RT_F64"); break;
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
  if (rt.types.empty()) {
    Write("void");
  } else if (rt.types.size() == 1) {
    Write(rt.types[0]);
  } else {
    Write("struct ", MangleMultivalueTypes(rt.types));
  }
}

void CWriter::Write(const Const& const_) {
  switch (const_.type()) {
    case Type::I32:
      Writef("%uu", static_cast<int32_t>(const_.u32()));
      break;

    case Type::I64:
      Writef("%" PRIu64 "ull", static_cast<int64_t>(const_.u64()));
      break;

    case Type::F32: {
      uint32_t f32_bits = const_.f32_bits();
      // TODO(binji): Share with similar float info in interp.cc and literal.cc
      if ((f32_bits & 0x7f800000u) == 0x7f800000u) {
        const char* sign = (f32_bits & 0x80000000) ? "-" : "";
        uint32_t significand = f32_bits & 0x7fffffu;
        if (significand == 0) {
          // Infinity.
          Writef("%sINFINITY", sign);
        } else {
          // Nan.
          Writef("f32_reinterpret_i32(0x%08x) /* %snan:0x%06x */", f32_bits,
                 sign, significand);
        }
      } else if (f32_bits == 0x80000000) {
        // Negative zero. Special-cased so it isn't written as -0 below.
        Writef("-0.f");
      } else {
        Writef("%.9g", Bitcast<float>(f32_bits));
      }
      break;
    }

    case Type::F64: {
      uint64_t f64_bits = const_.f64_bits();
      // TODO(binji): Share with similar float info in interp.cc and literal.cc
      if ((f64_bits & 0x7ff0000000000000ull) == 0x7ff0000000000000ull) {
        const char* sign = (f64_bits & 0x8000000000000000ull) ? "-" : "";
        uint64_t significand = f64_bits & 0xfffffffffffffull;
        if (significand == 0) {
          // Infinity.
          Writef("%sINFINITY", sign);
        } else {
          // Nan.
          Writef("f64_reinterpret_i64(0x%016" PRIx64 ") /* %snan:0x%013" PRIx64
                 " */",
                 f64_bits, sign, significand);
        }
      } else if (f64_bits == 0x8000000000000000ull) {
        // Negative zero. Special-cased so it isn't written as -0 below.
        Writef("-0.0");
      } else {
        Writef("%.17g", Bitcast<double>(f64_bits));
      }
      break;
    }

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::WriteInitDecl() {
  Write("extern void " + MangleName(module_name_) + "_init_module(void);",
        Newline());
  Write("extern void " + MangleName(module_name_) + "_init(",
        MangleModuleInstanceTypeName(module_name_), "*");
  for (auto import_module_name : import_module_set_) {
    Write(", struct ", MangleModuleInstanceTypeName(import_module_name), "*");
  }
  Write(");", Newline());
}

void CWriter::WriteModuleInstanceFreeDecl() {
  Write("extern void " + MangleName(module_name_) + "_free(",
        MangleModuleInstanceTypeName(module_name_), "*);");
  Write(Newline());
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

    case ExprType::GlobalGet:
      Write(GlobalInstanceVar(cast<GlobalGetExpr>(expr)->var));
      break;

    default:
      WABT_UNREACHABLE;
  }
}

std::string CWriter::GenerateHeaderGuard() const {
  std::string result;
  for (char c : header_name_) {
    if (isalnum(c) || c == '_') {
      result += toupper(c);
    } else {
      result += '_';
    }
  }
  result += "_GENERATED_";
  return result;
}

void CWriter::WriteSourceTop() {
  Write(s_source_includes);
  Write(Newline(), "#include \"", header_name_, "\"", Newline());
  Write(s_source_declarations);
}

void CWriter::WriteMultivalueTypes() {
  for (TypeEntry* type : module_->types) {
    FuncType* func_type = cast<FuncType>(type);
    Index num_results = func_type->GetNumResults();
    if (num_results <= 1) {
      continue;
    }
    std::string name = MangleMultivalueTypes(func_type->sig.result_types);
    // these ifndefs are actually to support importing multiple modules
    // incidentally they also mean we don't have to bother with deduplication
    Write("#ifndef ", name, Newline());
    Write("#define ", name, " ", name, Newline());
    Write("struct ", name, " ", OpenBrace());
    for (Index i = 0; i < num_results; ++i) {
      Type type = func_type->GetResultType(i);
      Write(type);
      Writef(" %c%d;", MangleType(type), i);
      Write(Newline());
    }
    Write(CloseBrace(), ";", Newline(), "#endif  /* ", name, " */", Newline());
  }
}

void CWriter::WriteFuncTypes() {
  if (module_->types.size()) {
    Writef("static u32 func_types[%" PRIzd "];", module_->types.size());
    Write(Newline());
  }
  Write(Newline());
  Write("static void init_func_types(void) ", OpenBrace());
  if (!module_->types.size()) {
    Write(CloseBrace(), Newline());
    return;
  }
  Index func_type_index = 0;
  for (TypeEntry* type : module_->types) {
    FuncType* func_type = cast<FuncType>(type);
    Index num_params = func_type->GetNumParams();
    Index num_results = func_type->GetNumResults();
    Write("func_types[", func_type_index, "] = wasm_rt_register_func_type(",
          num_params, ", ", num_results);
    for (Index i = 0; i < num_params; ++i) {
      Write(", ", TypeEnum(func_type->GetParamType(i)));
    }

    for (Index i = 0; i < num_results; ++i) {
      Write(", ", TypeEnum(func_type->GetResultType(i)));
    }

    Write(");", Newline());
    ++func_type_index;
  }
  Write(CloseBrace(), Newline());
}

void CWriter::ComputeUniqueImports() {
  std::map<std::pair<std::string, std::string>, const Import*> import_map;
  for (const Import* import : module_->imports) {
    auto key = make_pair(import->module_name, import->field_name);
    auto insertion_result = import_map.emplace(key, import);
    if (!insertion_result.second) {
      if (insertion_result.first->second->kind() != import->kind()) {
        UNIMPLEMENTED("contradictory import declaration");
      } else {
        fprintf(stderr, "warning: duplicate import declaration \"%s\" \"%s\"\n",
                import->module_name.c_str(), import->field_name.c_str());
      }
    }
    import_module_set_.insert(import->module_name);
    if (import->kind() == ExternalKind::Func) {
      import_func_module_set_.insert(import->module_name);
    }
  }

  for (const auto& node : import_map) {
    unique_imports_.push_back(node.second);
  }
}

void CWriter::BeginInstance() {
  if (module_->imports.empty()) {
    Write("typedef struct ", MangleModuleInstanceTypeName(module_name_), " ",
          OpenBrace());
    return;
  }

  ComputeUniqueImports();

  // define names of per-instance imports
  for (const Import* import : module_->imports) {
    switch (import->kind()) {
      case ExternalKind::Func: {
        const Func& func = cast<FuncImport>(import)->func;
        DefineImportName(func.name, import->module_name, import->field_name);
        import_module_sym_map_.emplace(func.name, import->module_name);
      } break;

      case ExternalKind::Global:
        DefineImportInstanceName(cast<GlobalImport>(import)->global.name,
                                 import->module_name, import->field_name);
        break;

      case ExternalKind::Memory:
        DefineImportInstanceName(cast<MemoryImport>(import)->memory.name,
                                 import->module_name, import->field_name);
        break;

      case ExternalKind::Table:
        DefineImportInstanceName(cast<TableImport>(import)->table.name,
                                 import->module_name, import->field_name);
        break;

      default:
        WABT_UNREACHABLE;
    }
  }

  // Forward declaring module instance types
  for (auto import_module : import_module_set_) {
    Write("struct ", MangleModuleInstanceTypeName(import_module), ";",
          Newline());
  }

  // Forward declaring module imports
  for (const Import* import : unique_imports_) {
    if (import->kind() == ExternalKind::Func) {
      continue;
    }

    Write("extern ");
    switch (import->kind()) {
      case ExternalKind::Global: {
        const Global& global = cast<GlobalImport>(import)->global;
        Write(global.type);
        break;
      }

      case ExternalKind::Memory: {
        Write("wasm_rt_memory_t");
        break;
      }

      case ExternalKind::Table: {
        Write("wasm_rt_table_t");
        break;
      }

      default:
        WABT_UNREACHABLE;
    }
    Write("* ", ExternalName(import->module_name, import->field_name),
          "(struct ", MangleModuleInstanceTypeName(import->module_name), "*);",
          Newline());
  }
  Write(Newline());

  // Add pointers to module instances that any func is imported from,
  // so that imported functions can be given their own module instances
  // when invoked
  Write("typedef struct ", MangleModuleInstanceTypeName(module_name_), " ",
        OpenBrace());
  for (auto import_module : import_func_module_set_) {
    Write("struct ", MangleModuleInstanceTypeName(import_module), "* ",
          MangleModuleInstanceName(import_module) + ";", Newline());
  }

  for (const Import* import : unique_imports_) {
    if (import->kind() == ExternalKind::Func) {
      continue;
    }

    Write("/* import: '", import->module_name, "' '", import->field_name,
          "' */", Newline());

    switch (import->kind()) {
      case ExternalKind::Global:
        WriteGlobal(
            cast<GlobalImport>(import)->global,
            "*" + ExternalName(import->module_name, import->field_name));
        break;

      case ExternalKind::Memory:
        WriteMemory("*" +
                    ExternalName(import->module_name, import->field_name));
        break;

      case ExternalKind::Table:
        WriteTable("*" + ExternalName(import->module_name, import->field_name));
        break;

      default:
        WABT_UNREACHABLE;
    }

    Write(Newline());
  }
}

void CWriter::WriteImports() {
  if (module_->imports.empty())
    return;

  Write(Newline());

  for (const Import* import : unique_imports_) {
    if (import->kind() == ExternalKind::Func) {
      Write("/* import: '", import->module_name, "' '", import->field_name,
            "' */", Newline());
      Write("extern ");
      const Func& func = cast<FuncImport>(import)->func;
      WriteImportFuncDeclaration(
          func.decl, import->module_name,
          MangleName(module_name_) + "_" +
              ExternalName(import->module_name, import->field_name));
      Write(";");
      Write(Newline());
    }
  }
}

void CWriter::WriteFuncDeclarations() {
  if (module_->funcs.size() == module_->num_func_imports)
    return;

  Write(Newline());

  Index func_index = 0;
  for (const Func* func : module_->funcs) {
    bool is_import = func_index < module_->num_func_imports;
    if (!is_import) {
      Write("static ");
      WriteFuncDeclaration(func->decl, DefineGlobalScopeName(func->name));
      Write(";", Newline());
    }
    ++func_index;
  }
}

void CWriter::WriteFuncDeclaration(const FuncDeclaration& decl,
                                   const std::string& name) {
  Write(ResultType(decl.sig.result_types), " ", name, "(");
  Write(MangleModuleInstanceTypeName(module_name_), "*");
  WriteParamTypes(decl);
  Write(")");
}

void CWriter::WriteImportFuncDeclaration(const FuncDeclaration& decl,
                                         const std::string& module_name,
                                         const std::string& name) {
  Write(ResultType(decl.sig.result_types), " ", name, "(");
  Write("struct ", MangleModuleInstanceTypeName(module_name), "*");
  WriteParamTypes(decl);
  Write(")");
}

void CWriter::WriteCallIndirectFuncDeclaration(const FuncDeclaration& decl,
                                               const std::string& name) {
  Write(ResultType(decl.sig.result_types), " ", name, "(void*");
  WriteParamTypes(decl);
  Write(")");
}

void CWriter::WriteModuleInstance() {
  BeginInstance();
  WriteGlobals();
  WriteMemories();
  WriteTables();
  WriteDataInstances();
  WriteElemInstances();

  // C forbids an empty struct
  if (module_->imports.empty() && module_->globals.empty() &&
      module_->memories.empty() && module_->tables.empty()) {
    Write("char dummy_member;", Newline());
  }

  Write(CloseBrace(), " ", MangleModuleInstanceTypeName(module_name_), ";",
        Newline());
  Write(Newline());
}

void CWriter::WriteGlobals() {
  Index global_index = 0;
  if (module_->globals.size() != module_->num_global_imports) {

    for (const Global* global : module_->globals) {
      bool is_import = global_index < module_->num_global_imports;
      if (!is_import) {
        WriteGlobal(*global, DefineGlobalScopeName(global->name));
        Write(Newline());
      }
      ++global_index;
    }
  }
}

void CWriter::WriteGlobal(const Global& global, const std::string& name) {
  Write(global.type, " ", name, ";");
}

void CWriter::WriteGlobalExportDecl(const Global& global,
                                    const std::string& mangled_name) {
  Write(global.type, "* ", mangled_name, "(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance)");
}

void CWriter::WriteMemories() {
  if (module_->memories.size() == module_->num_memory_imports)
    return;

  Index memory_index = 0;
  for (const Memory* memory : module_->memories) {
    bool is_import = memory_index < module_->num_memory_imports;
    if (!is_import) {
      WriteMemory(DefineGlobalScopeName(memory->name));
      Write(Newline());
    }
    ++memory_index;
  }
}

void CWriter::WriteMemory(const std::string& name) {
  Write("wasm_rt_memory_t ", name, ";");
}

void CWriter::WriteMemoryExportDecl(const std::string& mangled_name) {
  Write("wasm_rt_memory_t* ", mangled_name, "(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance)");
}

void CWriter::WriteTables() {
  if (module_->tables.size() == module_->num_table_imports) {
    return;
  }

  assert(module_->tables.size() <= 1);
  Index table_index = 0;
  for (const Table* table : module_->tables) {
    bool is_import = table_index < module_->num_table_imports;
    if (!is_import) {
      WriteTable(DefineGlobalScopeName(table->name));
      Write(Newline());
    }
    ++table_index;
  }
}

void CWriter::WriteTable(const std::string& name) {
  Write("wasm_rt_table_t ", name, ";");
}

void CWriter::WriteTableExportDecl(const std::string& mangled_name) {
  Write("wasm_rt_table_t* ", mangled_name, "(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance)");
}

void CWriter::WriteGlobalInitializers() {
  Write(Newline(), "static void init_globals(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance) ",
        OpenBrace());
  Index global_index = 0;
  for (const Global* global : module_->globals) {
    bool is_import = global_index < module_->num_global_imports;
    if (!is_import) {
      assert(!global->init_expr.empty());
      Write(ExternalInstanceRef(global->name), " = ");
      WriteInitExpr(global->init_expr);
      Write(";", Newline());
    }
    ++global_index;
  }
  Write(CloseBrace(), Newline());
}

static inline bool is_droppable(const DataSegment* data_segment) {
  return (data_segment->kind == SegmentKind::Passive) &&
         (!data_segment->data.empty());
}

static inline bool is_droppable(const ElemSegment* elem_segment) {
  return (elem_segment->kind == SegmentKind::Passive) &&
         (!elem_segment->elem_exprs.empty());
}

void CWriter::WriteDataInstances() {
  for (const DataSegment* data_segment : module_->data_segments) {
    DefineGlobalScopeName(data_segment->name);
    if (is_droppable(data_segment)) {
      Write("bool ", "data_segment_dropped_", GetGlobalName(data_segment->name),
            " : 1;", Newline());
    }
  }
}

void CWriter::WriteDataInitializers() {
  if (!module_->memories.empty()) {
    if (module_->data_segments.empty()) {
      Write(Newline());
    } else {
      for (const DataSegment* data_segment : module_->data_segments) {
        if (data_segment->data.size()) {
          Write(Newline(), "static const u8 data_segment_data_",
                GetGlobalName(data_segment->name), "[] = ", OpenBrace());
          size_t i = 0;
          for (uint8_t x : data_segment->data) {
            Writef("0x%02x, ", x);
            if ((++i % 12) == 0)
              Write(Newline());
          }
          if (i > 0)
            Write(Newline());
          Write(CloseBrace(), ";", Newline());
        }
      }
    }
  }

  Write("static void init_memory(", MangleModuleInstanceTypeName(module_name_),
        "* module_instance) ", OpenBrace());
  if (module_->memories.size() > module_->num_memory_imports) {
    Index memory_idx = module_->num_memory_imports;
    for (Index i = memory_idx; i < module_->memories.size(); i++) {
      const Memory* memory = module_->memories[i];
      uint32_t max =
          memory->page_limits.has_max ? memory->page_limits.max : 65536;
      Write("wasm_rt_allocate_memory(", ExternalInstancePtr(memory->name), ", ",
            memory->page_limits.initial, ", ", max, ");", Newline());
    }
  }
  for (const DataSegment* data_segment : module_->data_segments) {
    if (data_segment->kind == SegmentKind::Active) {
      const Memory* memory =
          module_->memories[module_->GetMemoryIndex(data_segment->memory_var)];
      Write("LOAD_DATA(", ExternalInstanceRef(memory->name), ", ");
      WriteInitExpr(data_segment->offset);
      if (data_segment->data.empty()) {
        Write(", NULL, 0");
      } else {
        Write(", data_segment_data_", GetGlobalName(data_segment->name), ", ",
              data_segment->data.size());
      }
      Write(");", Newline());
    }
  }

  Write(CloseBrace(), Newline());

  Write(Newline(), "static void init_data_instances(",
        MangleModuleInstanceTypeName(module_name_), " *module_instance) ",
        OpenBrace());

  for (const DataSegment* data_segment : module_->data_segments) {
    if (is_droppable(data_segment)) {
      Write("module_instance->data_segment_dropped_",
            GetGlobalName(data_segment->name), " = false;", Newline());
    }
  }

  Write(CloseBrace(), Newline());
}

void CWriter::WriteElemInstances() {
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    DefineGlobalScopeName(elem_segment->name);
    if (is_droppable(elem_segment)) {
      Write("bool ", "elem_segment_dropped_", GetGlobalName(elem_segment->name),
            " : 1;", Newline());
    }
  }
}

void CWriter::WriteElemInitializers() {
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    if (elem_segment->elem_exprs.empty()) {
      continue;
    }

    Write(Newline(),
          "static const wasm_elem_segment_expr_t elem_segment_exprs_",
          GetGlobalName(elem_segment->name), "[] = ", OpenBrace());

    for (const ExprList& elem_expr : elem_segment->elem_exprs) {
      assert(elem_expr.size() == 1);
      const Expr& expr = elem_expr.front();
      switch (expr.type()) {
        case ExprType::RefFunc: {
          const Func* func = module_->GetFunc(cast<RefFuncExpr>(&expr)->var);
          const Index func_type_index =
              module_->GetFuncTypeIndex(func->decl.type_var);
          Write("{", func_type_index, ", ");
          Write("(wasm_rt_funcref_t)", ExternalPtr(func->name), ", ");
          const bool is_import = import_module_sym_map_.count(func->name) != 0;
          if (is_import) {
            Write("offsetof(", MangleModuleInstanceTypeName(module_name_), ", ",
                  MangleModuleInstanceName(import_module_sym_map_[func->name]),
                  ")");
          } else {
            Write("0");
          }
          Write("}, ", Newline());
        } break;
        case ExprType::RefNull:
          Write("{0, NULL, 0}, ", Newline());
          break;
        default:
          WABT_UNREACHABLE;
      }
    }
    Write(CloseBrace(), ";", Newline());
  }

  Write(Newline(), "static void init_table(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance) ",
        OpenBrace());

  const Table* table = module_->tables.empty() ? nullptr : module_->tables[0];

  if (table && module_->num_table_imports == 0) {
    uint32_t max =
        table->elem_limits.has_max ? table->elem_limits.max : UINT32_MAX;
    Write("wasm_rt_allocate_table(", ExternalInstancePtr(table->name), ", ",
          table->elem_limits.initial, ", ", max, ");", Newline());
  }
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    if (elem_segment->kind != SegmentKind::Active) {
      continue;
    }

    Write("table_init(", ExternalInstancePtr(table->name), ", ");
    if (elem_segment->elem_exprs.empty()) {
      Write("NULL, 0, ");
    } else {
      Write("elem_segment_exprs_", GetGlobalName(elem_segment->name), ", ",
            elem_segment->elem_exprs.size(), ", ");
    }
    WriteInitExpr(elem_segment->offset);
    if (elem_segment->elem_exprs.empty()) {
      // It's mandatory to handle the case of a zero-length elem segment
      // (even in a module with no types). This must trap if the offset
      // is out of bounds.
      Write(", 0, 0, module_instance, NULL);", Newline());
    } else {
      Write(", 0, ", elem_segment->elem_exprs.size(),
            ", module_instance, func_types);", Newline());
    }
  }

  Write(CloseBrace(), Newline());

  Write(Newline(), "static void init_elem_instances(",
        MangleModuleInstanceTypeName(module_name_), " *module_instance) ",
        OpenBrace());

  for (const ElemSegment* elem_segment : module_->elem_segments) {
    if (is_droppable(elem_segment)) {
      Write("module_instance->elem_segment_dropped_",
            GetGlobalName(elem_segment->name), " = false;", Newline());
    }
  }

  Write(CloseBrace(), Newline());
}

void CWriter::WriteExports(WriteExportsKind kind) {
  if (module_->exports.empty())
    return;

  for (const Export* export_ : module_->exports) {
    Write(Newline(), "/* export: '", export_->name, "' */", Newline());
    if (kind == WriteExportsKind::Declarations) {
      Write("extern ");
    }

    std::string mangled_name;
    std::string internal_name;

    switch (export_->kind) {
      case ExternalKind::Func: {
        const Func* func = module_->GetFunc(export_->var);
        mangled_name = ExternalName(module_name_, export_->name);
        internal_name = func->name;
        if (kind == WriteExportsKind::Definitions) {
          func_ = func;
          local_syms_ = global_syms_;
          local_sym_map_.clear();
          stack_var_sym_map_.clear();

          Write(ResultType(func->decl.sig.result_types), " ", mangled_name,
                "(");
          std::vector<std::string> index_to_name;
          MakeTypeBindingReverseMapping(func_->GetNumParamsAndLocals(),
                                        func_->bindings, &index_to_name);
          WriteParams(index_to_name);
          Write("return ", ExternalRef(internal_name), "(");

          bool is_import = import_module_sym_map_.count(internal_name) != 0;
          if (is_import) {
            Write("module_instance->",
                  MangleModuleInstanceName(
                      import_module_sym_map_[internal_name]));
          } else {
            Write("module_instance");
          }
          WriteParamSymbols(index_to_name);
          Write(CloseBrace(), Newline());

          local_sym_map_.clear();
          stack_var_sym_map_.clear();
        } else if (kind == WriteExportsKind::Declarations) {
          WriteFuncDeclaration(func->decl, mangled_name);
        }
        break;
      }

      case ExternalKind::Global: {
        const Global* global = module_->GetGlobal(export_->var);
        mangled_name = ExternalName(module_name_, export_->name);
        internal_name = global->name;
        WriteGlobalExportDecl(*global, mangled_name);
        break;
      }

      case ExternalKind::Memory: {
        const Memory* memory = module_->GetMemory(export_->var);
        mangled_name = ExternalName(module_name_, export_->name);
        internal_name = memory->name;
        WriteMemoryExportDecl(mangled_name);
        break;
      }

      case ExternalKind::Table: {
        const Table* table = module_->GetTable(export_->var);
        mangled_name = ExternalName(module_name_, export_->name);
        internal_name = table->name;
        WriteTableExportDecl(mangled_name);
        break;
      }

      default:
        WABT_UNREACHABLE;
    }

    if (kind == WriteExportsKind::Declarations) {
      Write(";");
    }

    if (kind == WriteExportsKind::Definitions) {
      switch (export_->kind) {
        case ExternalKind::Func: {
          break;
        }

        case ExternalKind::Global:
        case ExternalKind::Memory:
        case ExternalKind::Table: {
          Write(OpenBrace());
          Write("return ", ExternalInstancePtr(internal_name), ";", Newline());
          Write(CloseBrace(), Newline());
          break;
        }

        default:
          WABT_UNREACHABLE;
      }
    }
  }
}

void CWriter::WriteInit() {
  Write(Newline(), "void " + MangleName(module_name_) + "_init_module(void)",
        OpenBrace());
  Write("wasm_rt_init();", Newline());
  Write("init_func_types();", Newline());
  Write(CloseBrace(), Newline());

  Write(Newline(), "void " + MangleName(module_name_) + "_init(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance");
  for (auto import_module_name : import_module_set_) {
    Write(", struct ", MangleModuleInstanceTypeName(import_module_name), "* ",
          MangleModuleInstanceName(import_module_name));
  }
  Write(") ", OpenBrace());

  if (!module_->imports.empty()) {
    Write("init_instance_import(module_instance");
    for (auto import_module_name : import_module_set_) {
      Write(", ", MangleModuleInstanceName(import_module_name));
    }
    Write(");", Newline());
  }

  Write("init_globals(module_instance);", Newline());
  Write("init_memory(module_instance);", Newline());
  Write("init_table(module_instance);", Newline());
  Write("init_data_instances(module_instance);", Newline());
  Write("init_elem_instances(module_instance);", Newline());
  for (Var* var : module_->starts) {
    Write(ExternalRef(module_->GetFunc(*var)->name));
    bool is_import =
        import_module_sym_map_.count(module_->GetFunc(*var)->name) != 0;
    if (is_import) {
      Write("(module_instance->",
            MangleModuleInstanceName(
                import_module_sym_map_[module_->GetFunc(*var)->name]) +
                ");");
    } else {
      Write("(module_instance);");
    }
    Write(Newline());
  }
  Write(CloseBrace(), Newline());
}

void CWriter::WriteModuleInstanceFree() {
  Write(Newline(), "void " + MangleName(module_name_) + "_free(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance) ",
        OpenBrace());

  {
    assert(module_->tables.size() <= 1);
    Index table_index = 0;
    for (const Table* table : module_->tables) {
      bool is_import = table_index < module_->num_table_imports;
      if (!is_import) {
        Write("wasm_rt_free_table(", ExternalInstancePtr(table->name), ");",
              Newline());
      }
      ++table_index;
    }
  }

  {
    Index memory_index = 0;
    for (const Memory* memory : module_->memories) {
      bool is_import = memory_index < module_->num_memory_imports;
      if (!is_import) {
        Write("wasm_rt_free_memory(", ExternalInstancePtr(memory->name), ");",
              Newline());
      }
      ++memory_index;
    }
  }

  Write(CloseBrace(), Newline());
}

void CWriter::WriteInitInstanceImport() {
  if (module_->imports.empty())
    return;

  Write(Newline(), "static void init_instance_import(",
        MangleModuleInstanceTypeName(module_name_), "* module_instance");
  for (auto import_module_name : import_module_set_) {
    Write(", struct ", MangleModuleInstanceTypeName(import_module_name), "* ",
          MangleModuleInstanceName(import_module_name));
  }
  Write(")", OpenBrace());

  for (auto import_module : import_func_module_set_) {
    Write("module_instance->", MangleModuleInstanceName(import_module), " = ",
          MangleModuleInstanceName(import_module), ";", Newline());
  }

  for (const Import* import : unique_imports_) {
    switch (import->kind()) {
      case ExternalKind::Func: {
        break;
      }

      case ExternalKind::Global:
      case ExternalKind::Memory:
      case ExternalKind::Table: {
        Write("module_instance->",
              ExternalName(import->module_name, import->field_name), " = ",
              ExternalName(import->module_name, import->field_name), "(",
              MangleModuleInstanceName(import->module_name), ");", Newline());
        break;
      }

      default:
        WABT_UNREACHABLE;
    }
  }
  Write(CloseBrace(), Newline());
}

void CWriter::WriteFuncs() {
  Index func_index = 0;
  for (const Func* func : module_->funcs) {
    bool is_import = func_index < module_->num_func_imports;
    if (!is_import)
      Write(Newline(), *func, Newline());
    ++func_index;
  }
}

void CWriter::Write(const Func& func) {
  func_ = &func;
  // Copy symbols from global symbol table so we don't shadow them.
  local_syms_ = global_syms_;
  local_sym_map_.clear();
  stack_var_sym_map_.clear();

  Write("static ", ResultType(func.decl.sig.result_types), " ",
        GlobalName(func.name), "(");
  WriteParamsAndLocals();
  Write("FUNC_PROLOGUE;", Newline());

  stream_ = &func_stream_;
  stream_->ClearOffset();

  std::string label = DefineLocalScopeName(kImplicitFuncLabel);
  ResetTypeStack(0);
  std::string empty;  // Must not be temporary, since address is taken by Label.
  PushLabel(LabelType::Func, empty, func.decl.sig);
  Write(func.exprs, LabelDecl(label));
  PopLabel();
  ResetTypeStack(0);
  PushTypes(func.decl.sig.result_types);
  Write("FUNC_EPILOGUE;", Newline());

  // Return the top of the stack implicitly.
  Index num_results = func.GetNumResults();
  if (num_results == 1) {
    Write("return ", StackVar(0), ";", Newline());
  } else if (num_results >= 2) {
    Write(OpenBrace());
    Write(ResultType(func.decl.sig.result_types), " tmp;", Newline());
    for (Index i = 0; i < num_results; ++i) {
      Type type = func.GetResultType(i);
      Writef("tmp.%c%d = ", MangleType(type), i);
      Write(StackVar(num_results - i - 1), ";", Newline());
    }
    Write("return tmp;", Newline());
    Write(CloseBrace(), Newline());
  }

  stream_ = c_stream_;

  WriteStackVarDeclarations();

  std::unique_ptr<OutputBuffer> buf = func_stream_.ReleaseOutputBuffer();
  stream_->WriteData(buf->data.data(), buf->data.size());

  Write(CloseBrace());

  func_stream_.Clear();
  func_ = nullptr;
}

void CWriter::WriteParamsAndLocals() {
  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func_->GetNumParamsAndLocals(), func_->bindings,
                                &index_to_name);
  WriteParams(index_to_name);
  WriteLocals(index_to_name);
}

void CWriter::WriteParams(const std::vector<std::string>& index_to_name) {
  Write(MangleModuleInstanceTypeName(module_name_), "* module_instance");
  if (func_->GetNumParams() != 0) {
    Indent(4);
    for (Index i = 0; i < func_->GetNumParams(); ++i) {
      Write(", ");
      if (i != 0 && (i % 8) == 0) {
        Write(Newline());
      }
      Write(func_->GetParamType(i), " ",
            DefineLocalScopeName(index_to_name[i]));
    }
    Dedent(4);
  }
  Write(") ", OpenBrace());
}

void CWriter::WriteParamSymbols(const std::vector<std::string>& index_to_name) {
  if (func_->GetNumParams() != 0) {
    Indent(4);
    for (Index i = 0; i < func_->GetNumParams(); ++i) {
      Write(", ");
      if (i != 0 && (i % 8) == 0) {
        Write(Newline());
      }
      Write(local_sym_map_[index_to_name[i]]);
    }
    Dedent(4);
  }
  Write(");", Newline());
}

void CWriter::WriteParamTypes(const FuncDeclaration& decl) {
  if (decl.GetNumParams() != 0) {
    for (Index i = 0; i < decl.GetNumParams(); ++i) {
      Write(", ");
      Write(decl.GetParamType(i));
    }
  }
}

void CWriter::WriteLocals(const std::vector<std::string>& index_to_name) {
  Index num_params = func_->GetNumParams();
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

        Write(DefineLocalScopeName(index_to_name[num_params + local_index]),
              " = 0");
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
    for (const auto& [pair, name] : stack_var_sym_map_) {
      Type stp_type = pair.second;

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

void CWriter::Write(const ExprList& exprs) {
  for (const Expr& expr : exprs) {
    switch (expr.type()) {
      case ExprType::Binary:
        Write(*cast<BinaryExpr>(&expr));
        break;

      case ExprType::Block: {
        const Block& block = cast<BlockExpr>(&expr)->block;
        std::string label = DefineLocalScopeName(block.label);
        DropTypes(block.decl.GetNumParams());
        size_t mark = MarkTypeStack();
        PushLabel(LabelType::Block, block.label, block.decl.sig);
        PushTypes(block.decl.sig.param_types);
        Write(block.exprs, LabelDecl(label));
        ResetTypeStack(mark);
        PopLabel();
        PushTypes(block.decl.sig.result_types);
        break;
      }

      case ExprType::Br:
        Write(GotoLabel(cast<BrExpr>(&expr)->var), Newline());
        // Stop processing this ExprList, since the following are unreachable.
        return;

      case ExprType::BrIf:
        Write("if (", StackVar(0), ") {");
        DropTypes(1);
        Write(GotoLabel(cast<BrIfExpr>(&expr)->var), "}", Newline());
        break;

      case ExprType::BrTable: {
        const auto* bt_expr = cast<BrTableExpr>(&expr);
        Write("switch (", StackVar(0), ") ", OpenBrace());
        DropTypes(1);
        Index i = 0;
        for (const Var& var : bt_expr->targets) {
          Write("case ", i++, ": ", GotoLabel(var), Newline());
        }
        Write("default: ");
        Write(GotoLabel(bt_expr->default_target), Newline(), CloseBrace(),
              Newline());
        // Stop processing this ExprList, since the following are unreachable.
        return;
      }

      case ExprType::Call: {
        const Var& var = cast<CallExpr>(&expr)->var;
        const Func& func = *module_->GetFunc(var);
        Index num_params = func.GetNumParams();
        Index num_results = func.GetNumResults();
        assert(type_stack_.size() >= num_params);
        if (num_results > 1) {
          Write(OpenBrace());
          Write("struct ", MangleMultivalueTypes(func.decl.sig.result_types));
          Write(" tmp = ");
        } else if (num_results == 1) {
          Write(StackVar(num_params - 1, func.GetResultType(0)), " = ");
        }

        Write(GlobalVar(var), "(");
        bool is_import = import_module_sym_map_.count(func.name) != 0;
        if (is_import) {
          Write("module_instance->",
                MangleModuleInstanceName(import_module_sym_map_[func.name]));
        } else {
          Write("module_instance");
        }
        for (Index i = 0; i < num_params; ++i) {
          Write(", ");
          Write(StackVar(num_params - i - 1));
        }
        Write(");", Newline());
        DropTypes(num_params);
        if (num_results > 1) {
          for (Index i = 0; i < num_results; ++i) {
            Type type = func.GetResultType(i);
            PushType(type);
            Write(StackVar(0));
            Writef(" = tmp.%c%d;", MangleType(type), i);
            Write(Newline());
          }
          Write(CloseBrace(), Newline());
        } else {
          PushTypes(func.decl.sig.result_types);
        }
        break;
      }

      case ExprType::CallIndirect: {
        const FuncDeclaration& decl = cast<CallIndirectExpr>(&expr)->decl;
        Index num_params = decl.GetNumParams();
        Index num_results = decl.GetNumResults();
        assert(type_stack_.size() > num_params);
        if (num_results > 1) {
          Write(OpenBrace());
          Write("struct ", MangleMultivalueTypes(decl.sig.result_types));
          Write(" tmp = ");
        } else if (num_results == 1) {
          Write(StackVar(num_params, decl.GetResultType(0)), " = ");
        }

        assert(module_->tables.size() == 1);
        const Table* table = module_->tables[0];

        assert(decl.has_func_type);
        Index func_type_index = module_->GetFuncTypeIndex(decl.type_var);

        Write("CALL_INDIRECT(", ExternalInstanceRef(table->name), ", ");
        WriteCallIndirectFuncDeclaration(decl, "(*)");
        Write(", ", func_type_index, ", ", StackVar(0));
        Write(", ", ExternalInstanceRef(table->name), ".data[", StackVar(0),
              "].module_instance");
        for (Index i = 0; i < num_params; ++i) {
          Write(", ", StackVar(num_params - i));
        }
        Write(");", Newline());
        DropTypes(num_params + 1);
        if (num_results > 1) {
          for (Index i = 0; i < num_results; ++i) {
            Type type = decl.GetResultType(i);
            PushType(type);
            Write(StackVar(0));
            Writef(" = tmp.%c%d;", MangleType(type), i);
            Write(Newline());
          }
          Write(CloseBrace(), Newline());
        } else {
          PushTypes(decl.sig.result_types);
        }
        break;
      }

      case ExprType::CodeMetadata:
        break;

      case ExprType::Compare:
        Write(*cast<CompareExpr>(&expr));
        break;

      case ExprType::Const: {
        const Const& const_ = cast<ConstExpr>(&expr)->const_;
        PushType(const_.type());
        Write(StackVar(0), " = ", const_, ";", Newline());
        break;
      }

      case ExprType::Convert:
        Write(*cast<ConvertExpr>(&expr));
        break;

      case ExprType::Drop:
        DropTypes(1);
        break;

      case ExprType::GlobalGet: {
        const Var& var = cast<GlobalGetExpr>(&expr)->var;
        PushType(module_->GetGlobal(var)->type);
        Write(StackVar(0), " = ", GlobalInstanceVar(var), ";", Newline());
        break;
      }

      case ExprType::GlobalSet: {
        const Var& var = cast<GlobalSetExpr>(&expr)->var;
        Write(GlobalInstanceVar(var), " = ", StackVar(0), ";", Newline());
        DropTypes(1);
        break;
      }

      case ExprType::If: {
        const IfExpr& if_ = *cast<IfExpr>(&expr);
        Write("if (", StackVar(0), ") ", OpenBrace());
        DropTypes(1);
        std::string label = DefineLocalScopeName(if_.true_.label);
        DropTypes(if_.true_.decl.GetNumParams());
        size_t mark = MarkTypeStack();
        PushLabel(LabelType::If, if_.true_.label, if_.true_.decl.sig);
        PushTypes(if_.true_.decl.sig.param_types);
        Write(if_.true_.exprs, CloseBrace());
        if (!if_.false_.empty()) {
          ResetTypeStack(mark);
          PushTypes(if_.true_.decl.sig.param_types);
          Write(" else ", OpenBrace(), if_.false_, CloseBrace());
        }
        ResetTypeStack(mark);
        Write(Newline(), LabelDecl(label));
        PopLabel();
        PushTypes(if_.true_.decl.sig.result_types);
        break;
      }

      case ExprType::Load:
        Write(*cast<LoadExpr>(&expr));
        break;

      case ExprType::LocalGet: {
        const Var& var = cast<LocalGetExpr>(&expr)->var;
        PushType(func_->GetLocalType(var));
        Write(StackVar(0), " = ", var, ";", Newline());
        break;
      }

      case ExprType::LocalSet: {
        const Var& var = cast<LocalSetExpr>(&expr)->var;
        Write(var, " = ", StackVar(0), ";", Newline());
        DropTypes(1);
        break;
      }

      case ExprType::LocalTee: {
        const Var& var = cast<LocalTeeExpr>(&expr)->var;
        Write(var, " = ", StackVar(0), ";", Newline());
        break;
      }

      case ExprType::Loop: {
        const Block& block = cast<LoopExpr>(&expr)->block;
        if (!block.exprs.empty()) {
          Write(DefineLocalScopeName(block.label), ": ");
          Indent();
          DropTypes(block.decl.GetNumParams());
          size_t mark = MarkTypeStack();
          PushLabel(LabelType::Loop, block.label, block.decl.sig);
          PushTypes(block.decl.sig.param_types);
          Write(Newline(), block.exprs);
          ResetTypeStack(mark);
          PopLabel();
          PushTypes(block.decl.sig.result_types);
          Dedent();
        }
        break;
      }

      case ExprType::MemoryFill: {
        const auto inst = cast<MemoryFillExpr>(&expr);
        Memory* memory =
            module_->memories[module_->GetMemoryIndex(inst->memidx)];
        Write("memory_fill(", ExternalInstancePtr(memory->name), ", ",
              StackVar(2), ", ", StackVar(1), ", ", StackVar(0), ");",
              Newline());
        DropTypes(3);
      } break;

      case ExprType::MemoryCopy: {
        const auto inst = cast<MemoryCopyExpr>(&expr);
        Memory* dest_memory =
            module_->memories[module_->GetMemoryIndex(inst->destmemidx)];
        const Memory* src_memory = module_->GetMemory(inst->srcmemidx);
        Write("memory_copy(", ExternalInstancePtr(dest_memory->name), ", ",
              ExternalInstancePtr(src_memory->name), ", ", StackVar(2), ", ",
              StackVar(1), ", ", StackVar(0), ");", Newline());
        DropTypes(3);
      } break;

      case ExprType::MemoryInit: {
        const auto inst = cast<MemoryInitExpr>(&expr);
        Memory* dest_memory =
            module_->memories[module_->GetMemoryIndex(inst->memidx)];
        const DataSegment* src_data = module_->GetDataSegment(inst->var);
        Write("memory_init(", ExternalInstancePtr(dest_memory->name), ", ");
        if (src_data->data.empty()) {
          Write("NULL, 0");
        } else {
          Write("data_segment_data_", GetGlobalName(src_data->name), ", ");
          if (is_droppable(src_data)) {
            Write("(", "module_instance->data_segment_dropped_",
                  GetGlobalName(src_data->name),
                  " ? 0 : ", src_data->data.size(), ")");
          } else {
            Write("0");
          }
        }

        Write(", ", StackVar(2), ", ", StackVar(1), ", ", StackVar(0), ");",
              Newline());
        DropTypes(3);
      } break;

      case ExprType::TableInit: {
        const auto inst = cast<TableInitExpr>(&expr);
        Table* dest_table =
            module_->tables[module_->GetTableIndex(inst->table_index)];
        const ElemSegment* src_segment =
            module_->GetElemSegment(inst->segment_index);
        Write("table_init(", ExternalInstancePtr(dest_table->name), ", ");
        if (src_segment->elem_exprs.empty()) {
          Write("NULL, 0");
        } else {
          Write("elem_segment_exprs_", GetGlobalName(src_segment->name), ", ");
          if (is_droppable(src_segment)) {
            Write("(module_instance->elem_segment_dropped_",
                  GetGlobalName(src_segment->name),
                  " ? 0 : ", src_segment->elem_exprs.size(), ")");
          } else {
            Write("0");
          }
        }

        Write(", ", StackVar(2), ", ", StackVar(1), ", ", StackVar(0));
        Write(", module_instance, func_types);", Newline());
        DropTypes(3);
      } break;

      case ExprType::DataDrop: {
        const auto inst = cast<DataDropExpr>(&expr);
        const DataSegment* data = module_->GetDataSegment(inst->var);
        if (is_droppable(data)) {
          Write("module_instance->data_segment_dropped_",
                GetGlobalName(data->name), " = true;", Newline());
        }
      } break;

      case ExprType::ElemDrop: {
        const auto inst = cast<ElemDropExpr>(&expr);
        const ElemSegment* seg = module_->GetElemSegment(inst->var);
        if (is_droppable(seg)) {
          Write("module_instance->elem_segment_dropped_",
                GetGlobalName(seg->name), " = true;", Newline());
        }
      } break;

      case ExprType::TableCopy: {
        const auto inst = cast<TableCopyExpr>(&expr);
        Table* dest_table =
            module_->tables[module_->GetTableIndex(inst->dst_table)];
        const Table* src_table = module_->GetTable(inst->src_table);
        Write("table_copy(", ExternalInstancePtr(dest_table->name), ", ",
              ExternalInstancePtr(src_table->name), ", ", StackVar(2), ", ",
              StackVar(1), ", ", StackVar(0), ");", Newline());
      } break;

      case ExprType::TableGet:
      case ExprType::TableSet:
      case ExprType::TableGrow:
      case ExprType::TableSize:
      case ExprType::TableFill:
      case ExprType::RefFunc:
      case ExprType::RefNull:
      case ExprType::RefIsNull:
        UNIMPLEMENTED("...");
        break;

      case ExprType::MemoryGrow: {
        Memory* memory = module_->memories[module_->GetMemoryIndex(
            cast<MemoryGrowExpr>(&expr)->memidx)];

        Write(StackVar(0), " = wasm_rt_grow_memory(",
              ExternalInstancePtr(memory->name), ", ", StackVar(0), ");",
              Newline());
        break;
      }

      case ExprType::MemorySize: {
        Memory* memory = module_->memories[module_->GetMemoryIndex(
            cast<MemorySizeExpr>(&expr)->memidx)];

        PushType(Type::I32);
        Write(StackVar(0), " = ", ExternalInstanceRef(memory->name), ".pages;",
              Newline());
        break;
      }

      case ExprType::Nop:
        break;

      case ExprType::Return:
        // Goto the function label instead; this way we can do shared function
        // cleanup code in one place.
        Write(GotoLabel(Var(label_stack_.size() - 1)), Newline());
        // Stop processing this ExprList, since the following are unreachable.
        return;

      case ExprType::Select: {
        Type type = StackType(1);
        Write(StackVar(2), " = ", StackVar(0), " ? ", StackVar(2), " : ",
              StackVar(1), ";", Newline());
        DropTypes(3);
        PushType(type);
        break;
      }

      case ExprType::Store:
        Write(*cast<StoreExpr>(&expr));
        break;

      case ExprType::Unary:
        Write(*cast<UnaryExpr>(&expr));
        break;

      case ExprType::Ternary:
        Write(*cast<TernaryExpr>(&expr));
        break;

      case ExprType::SimdLaneOp: {
        Write(*cast<SimdLaneOpExpr>(&expr));
        break;
      }

      case ExprType::SimdLoadLane: {
        Write(*cast<SimdLoadLaneExpr>(&expr));
        break;
      }

      case ExprType::SimdStoreLane: {
        Write(*cast<SimdStoreLaneExpr>(&expr));
        break;
      }

      case ExprType::SimdShuffleOp: {
        Write(*cast<SimdShuffleOpExpr>(&expr));
        break;
      }

      case ExprType::LoadSplat:
        Write(*cast<LoadSplatExpr>(&expr));
        break;

      case ExprType::LoadZero:
        Write(*cast<LoadZeroExpr>(&expr));
        break;

      case ExprType::Unreachable:
        Write("UNREACHABLE;", Newline());
        return;

      case ExprType::AtomicLoad:
      case ExprType::AtomicRmw:
      case ExprType::AtomicRmwCmpxchg:
      case ExprType::AtomicStore:
      case ExprType::AtomicWait:
      case ExprType::AtomicFence:
      case ExprType::AtomicNotify:
      case ExprType::Rethrow:
      case ExprType::ReturnCall:
      case ExprType::ReturnCallIndirect:
      case ExprType::Throw:
      case ExprType::Try:
      case ExprType::CallRef:
        UNIMPLEMENTED("...");
        break;
    }
  }
}

void CWriter::WriteSimpleUnaryExpr(Opcode opcode, const char* op) {
  Type result_type = opcode.GetResultType();
  Write(StackVar(0, result_type), " = ", op, "(", StackVar(0), ");", Newline());
  DropTypes(1);
  PushType(opcode.GetResultType());
}

void CWriter::WriteInfixBinaryExpr(Opcode opcode,
                                   const char* op,
                                   AssignOp assign_op) {
  Type result_type = opcode.GetResultType();
  Write(StackVar(1, result_type));
  if (assign_op == AssignOp::Allowed) {
    Write(" ", op, "= ", StackVar(0));
  } else {
    Write(" = ", StackVar(1), " ", op, " ", StackVar(0));
  }
  Write(";", Newline());
  DropTypes(2);
  PushType(result_type);
}

void CWriter::WritePrefixBinaryExpr(Opcode opcode, const char* op) {
  Type result_type = opcode.GetResultType();
  Write(StackVar(1, result_type), " = ", op, "(", StackVar(1), ", ",
        StackVar(0), ");", Newline());
  DropTypes(2);
  PushType(result_type);
}

void CWriter::WriteSignedBinaryExpr(Opcode opcode, const char* op) {
  Type result_type = opcode.GetResultType();
  Type type = opcode.GetParamType1();
  assert(opcode.GetParamType2() == type);
  Write(StackVar(1, result_type), " = (", type, ")((", SignedType(type), ")",
        StackVar(1), " ", op, " (", SignedType(type), ")", StackVar(0), ");",
        Newline());
  DropTypes(2);
  PushType(result_type);
}

void CWriter::Write(const BinaryExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Add:
    case Opcode::I64Add:
    case Opcode::F32Add:
    case Opcode::F64Add:
      WriteInfixBinaryExpr(expr.opcode, "+");
      break;

    case Opcode::I32Sub:
    case Opcode::I64Sub:
    case Opcode::F32Sub:
    case Opcode::F64Sub:
      WriteInfixBinaryExpr(expr.opcode, "-");
      break;

    case Opcode::I32Mul:
    case Opcode::I64Mul:
    case Opcode::F32Mul:
    case Opcode::F64Mul:
      WriteInfixBinaryExpr(expr.opcode, "*");
      break;

    case Opcode::I32DivS:
      WritePrefixBinaryExpr(expr.opcode, "I32_DIV_S");
      break;

    case Opcode::I64DivS:
      WritePrefixBinaryExpr(expr.opcode, "I64_DIV_S");
      break;

    case Opcode::I32DivU:
    case Opcode::I64DivU:
      WritePrefixBinaryExpr(expr.opcode, "DIV_U");
      break;

    case Opcode::F32Div:
    case Opcode::F64Div:
      WriteInfixBinaryExpr(expr.opcode, "/");
      break;

    case Opcode::I32RemS:
      WritePrefixBinaryExpr(expr.opcode, "I32_REM_S");
      break;

    case Opcode::I64RemS:
      WritePrefixBinaryExpr(expr.opcode, "I64_REM_S");
      break;

    case Opcode::I32RemU:
    case Opcode::I64RemU:
      WritePrefixBinaryExpr(expr.opcode, "REM_U");
      break;

    case Opcode::I32And:
    case Opcode::I64And:
      WriteInfixBinaryExpr(expr.opcode, "&");
      break;

    case Opcode::I32Or:
    case Opcode::I64Or:
      WriteInfixBinaryExpr(expr.opcode, "|");
      break;

    case Opcode::I32Xor:
    case Opcode::I64Xor:
      WriteInfixBinaryExpr(expr.opcode, "^");
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
      WritePrefixBinaryExpr(expr.opcode, "I32_ROTL");
      break;

    case Opcode::I64Rotl:
      WritePrefixBinaryExpr(expr.opcode, "I64_ROTL");
      break;

    case Opcode::I32Rotr:
      WritePrefixBinaryExpr(expr.opcode, "I32_ROTR");
      break;

    case Opcode::I64Rotr:
      WritePrefixBinaryExpr(expr.opcode, "I64_ROTR");
      break;

    case Opcode::F32Min:
    case Opcode::F64Min:
      WritePrefixBinaryExpr(expr.opcode, "FMIN");
      break;

    case Opcode::F32Max:
    case Opcode::F64Max:
      WritePrefixBinaryExpr(expr.opcode, "FMAX");
      break;

    case Opcode::F32Copysign:
      WritePrefixBinaryExpr(expr.opcode, "copysignf");
      break;

    case Opcode::F64Copysign:
      WritePrefixBinaryExpr(expr.opcode, "copysign");
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
      WriteInfixBinaryExpr(expr.opcode, "==", AssignOp::Disallowed);
      break;

    case Opcode::I32Ne:
    case Opcode::I64Ne:
    case Opcode::F32Ne:
    case Opcode::F64Ne:
      WriteInfixBinaryExpr(expr.opcode, "!=", AssignOp::Disallowed);
      break;

    case Opcode::I32LtS:
    case Opcode::I64LtS:
      WriteSignedBinaryExpr(expr.opcode, "<");
      break;

    case Opcode::I32LtU:
    case Opcode::I64LtU:
    case Opcode::F32Lt:
    case Opcode::F64Lt:
      WriteInfixBinaryExpr(expr.opcode, "<", AssignOp::Disallowed);
      break;

    case Opcode::I32LeS:
    case Opcode::I64LeS:
      WriteSignedBinaryExpr(expr.opcode, "<=");
      break;

    case Opcode::I32LeU:
    case Opcode::I64LeU:
    case Opcode::F32Le:
    case Opcode::F64Le:
      WriteInfixBinaryExpr(expr.opcode, "<=", AssignOp::Disallowed);
      break;

    case Opcode::I32GtS:
    case Opcode::I64GtS:
      WriteSignedBinaryExpr(expr.opcode, ">");
      break;

    case Opcode::I32GtU:
    case Opcode::I64GtU:
    case Opcode::F32Gt:
    case Opcode::F64Gt:
      WriteInfixBinaryExpr(expr.opcode, ">", AssignOp::Disallowed);
      break;

    case Opcode::I32GeS:
    case Opcode::I64GeS:
      WriteSignedBinaryExpr(expr.opcode, ">=");
      break;

    case Opcode::I32GeU:
    case Opcode::I64GeU:
    case Opcode::F32Ge:
    case Opcode::F64Ge:
      WriteInfixBinaryExpr(expr.opcode, ">=", AssignOp::Disallowed);
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const ConvertExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Eqz:
    case Opcode::I64Eqz:
      WriteSimpleUnaryExpr(expr.opcode, "!");
      break;

    case Opcode::I64ExtendI32S:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)(s64)(s32)");
      break;

    case Opcode::I64ExtendI32U:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)");
      break;

    case Opcode::I32WrapI64:
      WriteSimpleUnaryExpr(expr.opcode, "(u32)");
      break;

    case Opcode::I32TruncF32S:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_S_F32");
      break;

    case Opcode::I64TruncF32S:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_S_F32");
      break;

    case Opcode::I32TruncF64S:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_S_F64");
      break;

    case Opcode::I64TruncF64S:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_S_F64");
      break;

    case Opcode::I32TruncF32U:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_U_F32");
      break;

    case Opcode::I64TruncF32U:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_U_F32");
      break;

    case Opcode::I32TruncF64U:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_U_F64");
      break;

    case Opcode::I64TruncF64U:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_U_F64");
      break;

    case Opcode::I32TruncSatF32S:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_SAT_S_F32");
      break;

    case Opcode::I64TruncSatF32S:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_SAT_S_F32");
      break;

    case Opcode::I32TruncSatF64S:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_SAT_S_F64");
      break;

    case Opcode::I64TruncSatF64S:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_SAT_S_F64");
      break;

    case Opcode::I32TruncSatF32U:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_SAT_U_F32");
      break;

    case Opcode::I64TruncSatF32U:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_SAT_U_F32");
      break;

    case Opcode::I32TruncSatF64U:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_SAT_U_F64");
      break;

    case Opcode::I64TruncSatF64U:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_SAT_U_F64");
      break;

    case Opcode::F32ConvertI32S:
      WriteSimpleUnaryExpr(expr.opcode, "(f32)(s32)");
      break;

    case Opcode::F32ConvertI64S:
      WriteSimpleUnaryExpr(expr.opcode, "(f32)(s64)");
      break;

    case Opcode::F32ConvertI32U:
    case Opcode::F32DemoteF64:
      WriteSimpleUnaryExpr(expr.opcode, "(f32)");
      break;

    case Opcode::F32ConvertI64U:
      // TODO(binji): This needs to be handled specially (see
      // wabt_convert_uint64_to_float).
      WriteSimpleUnaryExpr(expr.opcode, "(f32)");
      break;

    case Opcode::F64ConvertI32S:
      WriteSimpleUnaryExpr(expr.opcode, "(f64)(s32)");
      break;

    case Opcode::F64ConvertI64S:
      WriteSimpleUnaryExpr(expr.opcode, "(f64)(s64)");
      break;

    case Opcode::F64ConvertI32U:
    case Opcode::F64PromoteF32:
      WriteSimpleUnaryExpr(expr.opcode, "(f64)");
      break;

    case Opcode::F64ConvertI64U:
      // TODO(binji): This needs to be handled specially (see
      // wabt_convert_uint64_to_double).
      WriteSimpleUnaryExpr(expr.opcode, "(f64)");
      break;

    case Opcode::F32ReinterpretI32:
      WriteSimpleUnaryExpr(expr.opcode, "f32_reinterpret_i32");
      break;

    case Opcode::I32ReinterpretF32:
      WriteSimpleUnaryExpr(expr.opcode, "i32_reinterpret_f32");
      break;

    case Opcode::F64ReinterpretI64:
      WriteSimpleUnaryExpr(expr.opcode, "f64_reinterpret_i64");
      break;

    case Opcode::I64ReinterpretF64:
      WriteSimpleUnaryExpr(expr.opcode, "i64_reinterpret_f64");
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const LoadExpr& expr) {
  const char* func = nullptr;
  switch (expr.opcode) {
    case Opcode::I32Load: func = "i32_load"; break;
    case Opcode::I64Load: func = "i64_load"; break;
    case Opcode::F32Load: func = "f32_load"; break;
    case Opcode::F64Load: func = "f64_load"; break;
    case Opcode::I32Load8S: func = "i32_load8_s"; break;
    case Opcode::I64Load8S: func = "i64_load8_s"; break;
    case Opcode::I32Load8U: func = "i32_load8_u"; break;
    case Opcode::I64Load8U: func = "i64_load8_u"; break;
    case Opcode::I32Load16S: func = "i32_load16_s"; break;
    case Opcode::I64Load16S: func = "i64_load16_s"; break;
    case Opcode::I32Load16U: func = "i32_load16_u"; break;
    case Opcode::I64Load16U: func = "i64_load16_u"; break;
    case Opcode::I64Load32S: func = "i64_load32_s"; break;
    case Opcode::I64Load32U: func = "i64_load32_u"; break;

    default:
      WABT_UNREACHABLE;
  }

  Memory* memory = module_->memories[module_->GetMemoryIndex(expr.memidx)];

  Type result_type = expr.opcode.GetResultType();
  Write(StackVar(0, result_type), " = ", func, "(",
        ExternalInstancePtr(memory->name), ", (u64)(", StackVar(0), ")");
  if (expr.offset != 0)
    Write(" + ", expr.offset, "u");
  Write(");", Newline());
  DropTypes(1);
  PushType(result_type);
}

void CWriter::Write(const StoreExpr& expr) {
  const char* func = nullptr;
  switch (expr.opcode) {
    case Opcode::I32Store: func = "i32_store"; break;
    case Opcode::I64Store: func = "i64_store"; break;
    case Opcode::F32Store: func = "f32_store"; break;
    case Opcode::F64Store: func = "f64_store"; break;
    case Opcode::I32Store8: func = "i32_store8"; break;
    case Opcode::I64Store8: func = "i64_store8"; break;
    case Opcode::I32Store16: func = "i32_store16"; break;
    case Opcode::I64Store16: func = "i64_store16"; break;
    case Opcode::I64Store32: func = "i64_store32"; break;

    default:
      WABT_UNREACHABLE;
  }

  Memory* memory = module_->memories[module_->GetMemoryIndex(expr.memidx)];

  Write(func, "(", ExternalInstancePtr(memory->name), ", (u64)(", StackVar(1),
        ")");
  if (expr.offset != 0)
    Write(" + ", expr.offset);
  Write(", ", StackVar(0), ");", Newline());
  DropTypes(2);
}

void CWriter::Write(const UnaryExpr& expr) {
  switch (expr.opcode) {
    case Opcode::I32Clz:
      WriteSimpleUnaryExpr(expr.opcode, "I32_CLZ");
      break;

    case Opcode::I64Clz:
      WriteSimpleUnaryExpr(expr.opcode, "I64_CLZ");
      break;

    case Opcode::I32Ctz:
      WriteSimpleUnaryExpr(expr.opcode, "I32_CTZ");
      break;

    case Opcode::I64Ctz:
      WriteSimpleUnaryExpr(expr.opcode, "I64_CTZ");
      break;

    case Opcode::I32Popcnt:
      WriteSimpleUnaryExpr(expr.opcode, "I32_POPCNT");
      break;

    case Opcode::I64Popcnt:
      WriteSimpleUnaryExpr(expr.opcode, "I64_POPCNT");
      break;

    case Opcode::F32Neg:
    case Opcode::F64Neg:
      WriteSimpleUnaryExpr(expr.opcode, "-");
      break;

    case Opcode::F32Abs:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_fabsf");
      break;

    case Opcode::F64Abs:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_fabs");
      break;

    case Opcode::F32Sqrt:
      WriteSimpleUnaryExpr(expr.opcode, "sqrtf");
      break;

    case Opcode::F64Sqrt:
      WriteSimpleUnaryExpr(expr.opcode, "sqrt");
      break;

    case Opcode::F32Ceil:
      WriteSimpleUnaryExpr(expr.opcode, "ceilf");
      break;

    case Opcode::F64Ceil:
      WriteSimpleUnaryExpr(expr.opcode, "ceil");
      break;

    case Opcode::F32Floor:
      WriteSimpleUnaryExpr(expr.opcode, "floorf");
      break;

    case Opcode::F64Floor:
      WriteSimpleUnaryExpr(expr.opcode, "floor");
      break;

    case Opcode::F32Trunc:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_truncf");
      break;

    case Opcode::F64Trunc:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_trunc");
      break;

    case Opcode::F32Nearest:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_nearbyintf");
      break;

    case Opcode::F64Nearest:
      WriteSimpleUnaryExpr(expr.opcode, "wasm_rt_nearbyint");
      break;

    case Opcode::I32Extend8S:
      WriteSimpleUnaryExpr(expr.opcode, "(u32)(s32)(s8)(u8)");
      break;

    case Opcode::I32Extend16S:
      WriteSimpleUnaryExpr(expr.opcode, "(u32)(s32)(s16)(u16)");
      break;

    case Opcode::I64Extend8S:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)(s64)(s8)(u8)");
      break;

    case Opcode::I64Extend16S:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)(s64)(s16)(u16)");
      break;

    case Opcode::I64Extend32S:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)(s64)(s32)(u32)");
      break;

    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const TernaryExpr& expr) {
  switch (expr.opcode) {
    case Opcode::V128BitSelect: {
      Type result_type = expr.opcode.GetResultType();
      Write(StackVar(2, result_type), " = ", "v128.bitselect", "(", StackVar(0),
            ", ", StackVar(1), ", ", StackVar(2), ");", Newline());
      DropTypes(3);
      PushType(result_type);
      break;
    }
    default:
      WABT_UNREACHABLE;
  }
}

void CWriter::Write(const SimdLaneOpExpr& expr) {
  Type result_type = expr.opcode.GetResultType();

  switch (expr.opcode) {
    case Opcode::I8X16ExtractLaneS:
    case Opcode::I8X16ExtractLaneU:
    case Opcode::I16X8ExtractLaneS:
    case Opcode::I16X8ExtractLaneU:
    case Opcode::I32X4ExtractLane:
    case Opcode::I64X2ExtractLane:
    case Opcode::F32X4ExtractLane:
    case Opcode::F64X2ExtractLane: {
      Write(StackVar(0, result_type), " = ", expr.opcode.GetName(), "(",
            StackVar(0), ", lane Imm: ", expr.val, ");", Newline());
      DropTypes(1);
      break;
    }
    case Opcode::I8X16ReplaceLane:
    case Opcode::I16X8ReplaceLane:
    case Opcode::I32X4ReplaceLane:
    case Opcode::I64X2ReplaceLane:
    case Opcode::F32X4ReplaceLane:
    case Opcode::F64X2ReplaceLane: {
      Write(StackVar(1, result_type), " = ", expr.opcode.GetName(), "(",
            StackVar(0), ", ", StackVar(1), ", lane Imm: ", expr.val, ");",
            Newline());
      DropTypes(2);
      break;
    }
    default:
      WABT_UNREACHABLE;
  }

  PushType(result_type);
}

void CWriter::Write(const SimdLoadLaneExpr& expr) {
  UNIMPLEMENTED("SIMD support");
}

void CWriter::Write(const SimdStoreLaneExpr& expr) {
  UNIMPLEMENTED("SIMD support");
}

void CWriter::Write(const SimdShuffleOpExpr& expr) {
  Type result_type = expr.opcode.GetResultType();
  Write(StackVar(1, result_type), " = ", expr.opcode.GetName(), "(",
        StackVar(1), " ", StackVar(0), ", lane Imm: $0x%08x %08x %08x %08x",
        expr.val.u32(0), expr.val.u32(1), expr.val.u32(2), expr.val.u32(3), ")",
        Newline());
  DropTypes(2);
  PushType(result_type);
}

void CWriter::Write(const LoadSplatExpr& expr) {
  assert(module_->memories.size() == 1);
  Memory* memory = module_->memories[0];

  Type result_type = expr.opcode.GetResultType();
  Write(StackVar(0, result_type), " = ", expr.opcode.GetName(), "(",
        ExternalInstancePtr(memory->name), ", (u64)(", StackVar(0));
  if (expr.offset != 0)
    Write(" + ", expr.offset);
  Write("));", Newline());
  DropTypes(1);
  PushType(result_type);
}

void CWriter::Write(const LoadZeroExpr& expr) {
  UNIMPLEMENTED("SIMD support");
}

void CWriter::WriteCHeader() {
  stream_ = h_stream_;
  std::string guard = GenerateHeaderGuard();
  Write("#ifndef ", guard, Newline());
  Write("#define ", guard, Newline());
  Write(s_header_top);
  WriteModuleInstance();
  WriteInitDecl();
  WriteModuleInstanceFreeDecl();
  WriteMultivalueTypes();
  WriteImports();
  WriteExports(WriteExportsKind::Declarations);
  Write(Newline(), s_header_bottom);
  Write(Newline(), "#endif  /* ", guard, " */", Newline());
}

void CWriter::WriteCSource() {
  stream_ = c_stream_;
  WriteSourceTop();
  WriteFuncTypes();
  WriteFuncDeclarations();
  WriteGlobalInitializers();
  WriteDataInitializers();
  WriteElemInitializers();
  WriteFuncs();
  WriteExports(WriteExportsKind::Definitions);
  WriteInitInstanceImport();
  WriteInit();
  WriteModuleInstanceFree();
}

Result CWriter::WriteModule(const Module& module) {
  WABT_USE(options_);
  module_ = &module;
  WriteCHeader();
  WriteCSource();
  return result_;
}

}  // end anonymous namespace

Result WriteC(Stream* c_stream,
              Stream* h_stream,
              const char* header_name,
              const Module* module,
              const WriteCOptions& options) {
  CWriter c_writer(c_stream, h_stream, header_name, options);
  return c_writer.WriteModule(*module);
}

}  // namespace wabt
