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

#define UNIMPLEMENTED(x) printf("unimplemented: %s\n", (x)), abort()

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
  Type StackType(Index) const;
  void PushType(Type);
  void PushTypes(const TypeVector&);
  void DropTypes(size_t count);

  void PushLabel(const Block& block);
  const Label* FindLabel(const Var& var);
  void PopLabel();

  std::string MangleName(string_view);
  std::string MangleName(string_view, string_view);
  std::string LegalizeName(string_view);
  std::string DefineName(SymbolSet*, string_view);
  std::string DefineImportName(const std::string&, string_view, string_view);
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
  void Write(string_view);
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
  void InitGlobalSymbols();
  void WriteHeader();
  void WriteFuncTypes();
  void WriteImports();
  void WriteFuncDeclarations();
  void WriteFuncDeclaration(const FuncDeclaration&, const std::string&);
  void WriteGlobals();
  void WriteGlobal(const Global&, const std::string&);
  void WriteMemories();
  void WriteMemory(const Memory&, const std::string&);
  void WriteDefineLoad(string_view name, string_view rest);
  void WriteDefineStore(string_view name, string_view rest);
  void WriteTables();
  void WriteTable(const Table&, const std::string&);
  void WriteDataInitializers();
  void WriteElemInitializers();
  void WriteInit();
  void WriteExports();
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

static const char* s_global_symbols[] = {
  // keywords
  "_Alignas", "_Alignof", "asm", "_Atomic", "auto", "_Bool", "break", "case",
  "char", "_Complex", "const", "continue", "default", "do", "double", "else",
  "enum", "extern", "float", "for", "_Generic", "goto", "if", "_Imaginary",
  "inline", "int", "long", "_Noreturn", "_Pragma", "register", "restrict",
  "return", "short", "signed", "sizeof", "static", "_Static_assert", "struct",
  "switch", "_Thread_local", "typedef", "union", "unsigned", "void", "volatile",
  "while",

  // math.h
  "abs", "acos", "acosh", "asin", "asinh", "atan", "atan2", "atanh", "cbrt",
  "ceil", "copysign", "cos", "cosh", "double_t", "erf", "erfc", "exp", "exp2",
  "expm1", "fabs", "fdim", "float_t", "floor", "fma", "fmax", "fmin", "fmod",
  "fpclassify", "FP_FAST_FMA", "FP_FAST_FMAF", "FP_FAST_FMAL", "FP_ILOGB0",
  "FP_ILOGBNAN", "FP_INFINITE", "FP_NAN", "FP_NORMAL", "FP_SUBNORMAL",
  "FP_ZERO", "frexp", "HUGE_VAL", "HUGE_VALF", "HUGE_VALL", "hypot", "ilogb",
  "INFINITY", "isfinite", "isgreater", "isgreaterequal", "isinf", "isless",
  "islessequal", "islessgreater", "isnan", "isnormal", "isunordered", "ldexp",
  "lgamma", "llrint", "llround", "log", "log10", "log1p", "log2", "logb",
  "lrint", "lround", "MATH_ERREXCEPT", "math_errhandling", "MATH_ERRNO", "modf",
  "nan", "NAN", "nanf", "nanl", "nearbyint", "nextafter", "nexttoward", "pow",
  "remainder", "remquo", "rint", "round", "scalbln", "scalbn", "signbit", "sin",
  "sinh", "sqrt", "tan", "tanh", "tgamma", "trunc",

  // stdint.h
  "INT16_C", "INT16_MAX", "INT16_MIN", "int16_t", "INT32_MAX", "INT32_MIN",
  "int32_t", "INT64_C", "INT64_MAX", "INT64_MIN", "int64_t", "INT8_C",
  "INT8_MAX", "INT8_MIN", "int8_t", "INT_FAST16_MAX", "INT_FAST16_MIN",
  "int_fast16_t", "INT_FAST32_MAX", "INT_FAST32_MIN", "int_fast32_t",
  "INT_FAST64_MAX", "INT_FAST64_MIN", "int_fast64_t", "INT_FAST8_MAX",
  "INT_FAST8_MIN", "int_fast8_t", "INT_LEAST16_MAX", "INT_LEAST16_MIN",
  "int_least16_t", "INT_LEAST32_MAX", "INT_LEAST32_MIN", "int_least32_t",
  "INT_LEAST64_MAX", "INT_LEAST64_MIN", "int_least64_t", "INT_LEAST8_MAX",
  "INT_LEAST8_MIN", "int_least8_t", "INTMAX_C", "INTMAX_MAX", "INTMAX_MIN",
  "intmax_t", "INTPTR_MAX", "INTPTR_MIN", "intptr_t", "PTRDIFF_MAX",
  "PTRDIFF_MIN", "SIG_ATOMIC_MAX", "SIG_ATOMIC_MIN", "SIZE_MAX", "UINT16_C",
  "UINT16_MAX", "uint16_t", "UINT32_C", "UINT32_MAX", "uint32_t", "UINT64_C",
  "UINT64_MAX", "uint64_t", "UINT8_MAX", "uint8_t", "UINT_FAST16_MAX",
  "uint_fast16_t", "UINT_FAST32_MAX", "uint_fast32_t", "UINT_FAST64_MAX",
  "uint_fast64_t", "UINT_FAST8_MAX", "uint_fast8_t", "UINT_LEAST16_MAX",
  "uint_least16_t", "UINT_LEAST32_MAX", "uint_least32_t", "UINT_LEAST64_MAX",
  "uint_least64_t", "UINT_LEAST8_MAX", "uint_least8_t", "UINTMAX_C",
  "UINTMAX_MAX", "uintmax_t", "UINTPTR_MAX", "uintptr_t", "UNT8_C", "WCHAR_MAX",
  "WCHAR_MIN", "WINT_MAX", "WINT_MIN",

  // stdlib.h
  "abort", "abs", "atexit", "atof", "atoi", "atol", "atoll", "at_quick_exit",
  "bsearch", "calloc", "div", "div_t", "exit", "_Exit", "EXIT_FAILURE",
  "EXIT_SUCCESS", "free", "getenv", "labs", "ldiv", "ldiv_t", "llabs", "lldiv",
  "lldiv_t", "malloc", "MB_CUR_MAX", "mblen", "mbstowcs", "mbtowc", "qsort",
  "quick_exit", "rand", "RAND_MAX", "realloc", "size_t", "srand", "strtod",
  "strtof", "strtol", "strtold", "strtoll", "strtoul", "strtoull", "system",
  "wcstombs", "wctomb",

  // string.h
  "memchr", "memcmp", "memcpy", "memmove", "memset", "NULL", "size_t", "strcat",
  "strchr", "strcmp", "strcoll", "strcpy", "strcspn", "strerror", "strlen",
  "strncat", "strncmp", "strncpy", "strpbrk", "strrchr", "strspn", "strstr",
  "strtok", "strxfrm",

  // defined
  "Anyfunc",
  "CALL_INDIRECT",
  "DEFINE_LOAD",
  "DEFINE_STORE",
  "Elem",
  "EXPORT_FUNC",
  "EXPORT_GLOBAL",
  "EXPORT_MEMORY",
  "EXPORT_TABLE",
  "f32",
  "F32",
  "f64",
  "F64",
  "I32",
  "I64",
  "init",
  "init_data_segment",
  "init_elem_segment",
  "MEMCHECK",
  "Memory",
  "register_func_type",
  "s16",
  "s32",
  "s64",
  "s8",
  "Table",
  "trap",
  "Trap",
  "TRAP_OOB",
  "Type",
  "u16",
  "u32",
  "u64",
  "u8",
  "UNREACHABLE",

};

static const char s_header[] =
    R"(#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;

typedef enum Trap {
  TRAP_OOB, TRAP_INT_OVERFLOW, TRAP_DIV_BY_ZERO, TRAP_INVALID_CONVERSION,
  TRAP_UNREACHABLE, TRAP_CALL_INDIRECT } Trap;
typedef enum Type { I32, I64, F32, F64 } Type;
typedef void (*Anyfunc)();
typedef struct Elem { u32 func_type; Anyfunc func; } Elem;
typedef struct Memory { u8* data; size_t len; } Memory;
typedef struct Table { Elem* data; size_t len; } Table;

extern void trap(Trap) __attribute__((noreturn));
extern u32 register_func_type(u32 params, u32 results, ...);
extern void init_data_segment(u32 addr, const u8 *data, size_t len);
extern void init_elem_segment(u32 idx, const Elem *data, size_t len);

void init(void);

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define EXPORT_FUNC(sym, name) void sym(void) __attribute__((alias(#name)))
#define EXPORT_GLOBAL(sym, name) extern int sym __attribute__((alias(#name)))
#define EXPORT_MEMORY(sym, name) extern int sym __attribute__((alias(#name)))
#define EXPORT_TABLE(sym, name) extern int sym __attribute__((alias(#name)))

#define TRAP(x) (trap(TRAP_##x), 0)

#define MEMCHECK(mem, a, t)  \
  if (UNLIKELY((a) + sizeof(t) > mem.len)) TRAP(OOB)

#define DEFINE_LOAD(mem, name, t1, t2, t3)        \
  static inline t3 name(u32 addr) {               \
    MEMCHECK(mem, addr, t1);                      \
    t1 result;                                    \
    memcpy(&result, &mem.data[addr], sizeof(t1)); \
    return (t3)(t2)result;                        \
  }

#define DEFINE_STORE(mem, name, t1, t2)            \
  static inline void name(u32 addr, t2 value) {    \
    MEMCHECK(mem, addr, t1);                       \
    t1 wrapped = (t1)value;                        \
    memcpy(&mem.data[addr], &wrapped, sizeof(t1)); \
  }

#define CALL_INDIRECT(table, t, ft, x, ...)                                 \
  (((x) < table.len && table.data[x].func && table.data[x].func_type == ft) \
       ? ((t)table.data[x].func)(__VA_ARGS__)                               \
       : TRAP(CALL_INDIRECT))

#define DIVREM_S(op, ut, st, min, x, y)                      \
   ((UNLIKELY((y) == 0)) ?                TRAP(DIV_BY_ZERO)  \
  : (UNLIKELY((x) == min && (y) == -1)) ? TRAP(INT_OVERFLOW) \
  : (ut)((x) op (y)))

#define DIVREM_U(op, x, y) \
  ((UNLIKELY((y) == 0)) ? TRAP(DIV_BY_ZERO) : ((x) op (y)))

#define I32_DIV_S(x, y) DIVREM_S(/, u32, s32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIVREM_S(/, u64, s64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) DIVREM_S(%, u32, s32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) DIVREM_S(%, u64, s64, INT64_MIN, (s64)x, (s64)y)

#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define TRUNC_S(ut, st, min, max, x)                            \
   ((UNLIKELY((x) != (x))) ? TRAP(INVALID_CONVERSION)           \
  : (UNLIKELY((x) < (min) || (x) > (max))) ? TRAP(INT_OVERFLOW) \
  : (ut)(x))

#define I32_TRUNC_S_F32(x) TRUNC_S(u32, s32, (f32)INT32_MIN, (f32)INT32_MAX, (s32)x)
#define I64_TRUNC_S_F32(x) TRUNC_S(u64, s64, (f32)INT64_MIN, (f32)INT64_MAX, (s64)x)
#define I32_TRUNC_S_F64(x) TRUNC_S(u32, s32, (f64)INT32_MIN, (f64)INT32_MAX, (s32)x)
#define I64_TRUNC_S_F64(x) TRUNC_S(u64, s64, (f64)INT64_MIN, (f64)INT64_MAX, (s64)x)

#define DEFINE_REINTERPRET(name, t1, t2)  \
  static inline t2 name(t1 x) {           \
    t2 result;                            \
    memcpy(&result, &x, sizeof(result));  \
    return result;                        \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)

#define UNREACHABLE TRAP(UNREACHABLE)

)";

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

std::string CWriter::MangleName(string_view name) {
  const char kPrefix = 'Z';
  std::string result = "Z_";

  if (!name.empty()) {
    for (char c: name) {
      if ((isalnum(c) && c != kPrefix) || c == '_') {
        result += c;
      } else {
        result += kPrefix;
        result += StringPrintf("%02X", c);
      }
    }
  }

  return result;
}

std::string CWriter::MangleName(string_view module, string_view field) {
  return MangleName(module) + MangleName(field);
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

std::string CWriter::DefineImportName(const std::string& name,
                                      string_view module,
                                      string_view field) {
  std::string mangled = MangleName(module, field);
  assert(global_syms_.count(mangled) == 0);
  global_syms_.insert(mangled);
  global_sym_map_.insert(SymbolMap::value_type(name, mangled));
  return mangled;
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

void CWriter::Write(string_view s) {
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
      Writef("%u" "u", static_cast<int32_t>(const_.u32));
      break;

    case Type::I64:
      Writef("%" PRIu64 "ull", static_cast<int64_t>(const_.u64));
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

void CWriter::InitGlobalSymbols() {
  for (const char* symbol : s_global_symbols)
    global_syms_.insert(symbol);
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

void CWriter::WriteImports() {
  for (const Import* import : module_->imports) {
    Write("extern ");
    switch (import->kind()) {
      case ExternalKind::Func: {
        const Func& func = cast<FuncImport>(import)->func;
        WriteFuncDeclaration(func.decl,
                             DefineImportName(func.name, import->module_name,
                                              import->field_name));
        Write(";", Newline());
        break;
      }

      case ExternalKind::Global: {
        const Global& global = cast<GlobalImport>(import)->global;
        WriteGlobal(global, DefineImportName(global.name, import->module_name,
                                             import->field_name));
        break;
      }

      case ExternalKind::Memory: {
        const Memory& memory = cast<MemoryImport>(import)->memory;
        WriteMemory(memory, DefineImportName(memory.name, import->module_name,
                                             import->field_name));
        break;
      }

      case ExternalKind::Table: {
        const Table& table = cast<TableImport>(import)->table;
        WriteTable(table, DefineImportName(table.name, import->module_name,
                                           import->field_name));
        break;
      }

      default:
        WABT_UNREACHABLE;
    }
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteFuncDeclarations() {
  if (module_->funcs.size() == module_->num_func_imports)
    return;

  Index func_index = 0;
  for (const Func* func : module_->funcs) {
    bool is_import = func_index < module_->num_func_imports;
    if (!is_import) {
      Write("static ");
      WriteFuncDeclaration(func->decl, DefineGlobalName(func->name));
      Write(";", Newline());
    }
    ++func_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteFuncDeclaration(const FuncDeclaration& decl,
                                   const std::string& name) {
  Write(ResultType(decl.sig.result_types), " ", name, "(");
  for (Index i = 0; i < decl.GetNumParams(); ++i) {
    if (i != 0)
      Write(", ");
    Write(decl.GetParamType(i));
  }
  Write(")");
}

void CWriter::WriteGlobals() {
  if (module_->globals.size() == module_->num_global_imports)
    return;

  Index global_index = 0;
  for (const Global* global : module_->globals) {
    bool is_import = global_index < module_->num_global_imports;
    if (!is_import) {
      Write("static ");
      WriteGlobal(*global, DefineGlobalName(global->name));
    }
    ++global_index;
  }
  Write(Newline(true), Newline(true));

  Write("static void init_globals(void) ", OpenBrace());
  global_index = 0;
  for (const Global* global : module_->globals) {
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

void CWriter::WriteGlobal(const Global& global, const std::string& name) {
  Write(global.type, " ", name, ";", Newline());
}

void CWriter::WriteMemories() {
  if (module_->memories.size() == module_->num_memory_imports)
    return;

  assert(module_->memories.size() <= 1);
  Index memory_index = 0;
  for (const Memory* memory : module_->memories) {
    bool is_import = memory_index < module_->num_memory_imports;
    if (!is_import)
      WriteMemory(*memory, DefineGlobalName(memory->name));
    ++memory_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteMemory(const Memory& memory, const std::string& name) {
  Write("Memory ", name, ";", Newline(), Newline(true), Newline(true));
  WriteDefineLoad(name, "i32_load, u32, u32, u32");
  WriteDefineLoad(name, "i64_load, u64, u64, u64");
  WriteDefineLoad(name, "f32_load, f32, f32, f32");
  WriteDefineLoad(name, "f64_load, f64, f64, f64");
  WriteDefineLoad(name, "i32_load8_s, s8, s32, u32");
  WriteDefineLoad(name, "i64_load8_s, s8, s64, u64");
  WriteDefineLoad(name, "i32_load8_u, u8, u32, u32");
  WriteDefineLoad(name, "i64_load8_u, u8, u64, u64");
  WriteDefineLoad(name, "i32_load16_s, s16, s32, u32");
  WriteDefineLoad(name, "i64_load16_s, s16, s64, u64");
  WriteDefineLoad(name, "i32_load16_u, u16, u32, u32");
  WriteDefineLoad(name, "i64_load16_u, u16, u64, u64");
  WriteDefineLoad(name, "i64_load32_s, s32, s64, u64");
  WriteDefineLoad(name, "i64_load32_u, u32, u64, u64");

  WriteDefineStore(name, "i32_store, u32, u32");
  WriteDefineStore(name, "i64_store, u64, u64");
  WriteDefineStore(name, "f32_store, f32, f32");
  WriteDefineStore(name, "f64_store, f64, f64");
  WriteDefineStore(name, "i32_store8, u8, u32");
  WriteDefineStore(name, "i32_store16, u16, u32");
  WriteDefineStore(name, "i64_store8, u8, u64");
  WriteDefineStore(name, "i64_store16, u16, u64");
  WriteDefineStore(name, "i64_store32, u32, u64");
  Write(Newline(true), Newline(true));
}

void CWriter::WriteDefineLoad(string_view name, string_view rest) {
  Write("DEFINE_LOAD(", name, ", ", rest, ")", Newline());
}

void CWriter::WriteDefineStore(string_view name, string_view rest) {
  Write("DEFINE_STORE(", name, ", ", rest, ")", Newline());
}

void CWriter::WriteTables() {
  if (module_->tables.size() == module_->num_table_imports)
    return;

  assert(module_->tables.size() <= 1);
  Index table_index = 0;
  for (const Table* table : module_->tables) {
    bool is_import = table_index < module_->num_table_imports;
    if (!is_import)
      WriteTable(*table, DefineGlobalName(table->name));
    ++table_index;
  }
  Write(Newline(true), Newline(true));
}

void CWriter::WriteTable(const Table& table, const std::string& name) {
  Write("Table ", name, ";", Newline());
}

void CWriter::WriteDataInitializers() {
  if (module_->memories.empty())
    return;

  Index data_segment_index = 0;
  for (const DataSegment* data_segment : module_->data_segments) {
    Write("static const u8 data_segment_data_", data_segment_index,
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
  for (const DataSegment* data_segment : module_->data_segments) {
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
  for (const ElemSegment* elem_segment : module_->elem_segments) {
    Write("static const Elem elem_segment_data_", elem_segment_index,
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

void CWriter::WriteExports() {
  for (const Export* export_ : module_->exports) {
    const char* macro;
    std::string name;

    switch (export_->kind) {
      case ExternalKind::Func:
        macro = "EXPORT_FUNC";
        name = module_->GetFunc(export_->var)->name;
        break;

      case ExternalKind::Global:
        macro = "EXPORT_GLOBAL";
        name = module_->GetGlobal(export_->var)->name;
        break;

      case ExternalKind::Memory:
        macro = "EXPORT_MEMORY";
        name = module_->GetMemory(export_->var)->name;
        break;

      case ExternalKind::Table:
        macro = "EXPORT_TABLE";
        name = module_->GetTable(export_->var)->name;
        break;

      default: WABT_UNREACHABLE;
    }

    Write(macro, "(", MangleName(export_->name), ", ", GlobalName(name), ");",
          Newline());
  }
  Write(Newline(true), Newline(true));
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
  Write(Newline(), block.exprs);
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
        Write(block, label, ":;", Newline());
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

        assert(module_->tables.size() == 1);
        const Table* table = module_->tables[0];

        assert(decl.has_func_type);
        Index func_type_index = module_->GetFuncTypeIndex(decl.type_var);

        Write("CALL_INDIRECT(", GlobalName(table->name), ", ");
        WriteFuncDeclaration(decl, "(*)");
        Write(", ", func_type_index, ", ", StackVar(0));
        for (Index i = 0; i < num_params; ++i) {
          Write(", ", StackVar(num_params - i));
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
        UNIMPLEMENTED("current_memory");
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
        UNIMPLEMENTED("grow_memory");
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
        Write(DefineLocalName(block.label), ": ");
        Indent();
        Write(block);
        Dedent();
        Write(Newline());
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

      case ExprType::Select: {
        Type type = StackType(1);
        Write(StackVar(2), " = ", StackVar(0), " ? ", StackVar(2), " : ",
              StackVar(1), ";", Newline());
        DropTypes(3);
        PushType(type);
        break;
      }

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
    case Opcode::I64Rotl:
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::I32Rotr:
    case Opcode::I64Rotr:
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::F32Min:
    case Opcode::F64Min:
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::F32Max:
    case Opcode::F64Max:
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::F32Copysign:
    case Opcode::F64Copysign:
      UNIMPLEMENTED(expr.opcode.GetName());
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

    case Opcode::I64ExtendSI32:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)(s64)(s32)");
      break;

    case Opcode::I64ExtendUI32:
      WriteSimpleUnaryExpr(expr.opcode, "(u64)");
      break;

    case Opcode::I32WrapI64:
      WriteSimpleUnaryExpr(expr.opcode, "(u32)");
      break;

    case Opcode::I32TruncSF32:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_S_F32");
      break;

    case Opcode::I64TruncSF32:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_S_F32");
      break;

    case Opcode::I32TruncSF64:
      WriteSimpleUnaryExpr(expr.opcode, "I32_TRUNC_S_F64");
      break;

    case Opcode::I64TruncSF64:
      WriteSimpleUnaryExpr(expr.opcode, "I64_TRUNC_S_F64");
      break;

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
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::F32ConvertSI32:
    case Opcode::F32ConvertUI32:
    case Opcode::F32ConvertSI64:
    case Opcode::F32DemoteF64:
      WriteSimpleUnaryExpr(expr.opcode, "(f32)");
      break;

    case Opcode::F32ConvertUI64:
      // TODO(binji): This needs to be handled specially (see
      // wabt_convert_uint64_to_float).
      WriteSimpleUnaryExpr(expr.opcode, "(f32)");
      break;

    case Opcode::F64ConvertSI32:
    case Opcode::F64ConvertUI32:
    case Opcode::F64ConvertSI64:
    case Opcode::F64PromoteF32:
      WriteSimpleUnaryExpr(expr.opcode, "(f64)");
      break;

    case Opcode::F64ConvertUI64:
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
    case Opcode::I64Load16U: func = "i32_load16_u"; break;
    case Opcode::I64Load32S: func = "i64_load32_s"; break;
    case Opcode::I64Load32U: func = "i64_load32_u"; break;

    default:
      WABT_UNREACHABLE;
  }

  Type result_type = expr.opcode.GetResultType();
  Write(StackVar(0, result_type), " = ", func, "(", StackVar(0));
  if (expr.offset != 0)
    Write(" + ", expr.offset);
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

  Write(func, "(", StackVar(1));
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
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    case Opcode::F32Neg:
    case Opcode::F64Neg:
      WriteSimpleUnaryExpr(expr.opcode, "-");
      break;

    case Opcode::F32Abs:
      WriteSimpleUnaryExpr(expr.opcode, "fabsf");
      break;

    case Opcode::F64Abs:
      WriteSimpleUnaryExpr(expr.opcode, "fabs");
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
      WriteSimpleUnaryExpr(expr.opcode, "truncf");
      break;

    case Opcode::F64Trunc:
      WriteSimpleUnaryExpr(expr.opcode, "trunc");
      break;

    case Opcode::F32Nearest:
      WriteSimpleUnaryExpr(expr.opcode, "nearbyintf");
      break;

    case Opcode::F64Nearest:
      WriteSimpleUnaryExpr(expr.opcode, "nearbyint");
      break;

    case Opcode::I32Extend8S:
    case Opcode::I32Extend16S:
    case Opcode::I64Extend8S:
    case Opcode::I64Extend16S:
    case Opcode::I64Extend32S:
      UNIMPLEMENTED(expr.opcode.GetName());
      break;

    default:
      WABT_UNREACHABLE;
  }
}

Result CWriter::WriteModule(const Module& module) {
  WABT_USE(options_);

  module_ = &module;
  InitGlobalSymbols();
  WriteHeader();
  WriteFuncTypes();
  WriteImports();
  WriteFuncDeclarations();
  WriteGlobals();
  WriteMemories();
  WriteTables();
  WriteFuncs();
  WriteDataInitializers();
  WriteElemInitializers();
  WriteInit();
  WriteExports();

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
