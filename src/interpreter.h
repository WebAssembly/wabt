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

#ifndef WABT_INTERPRETER_H_
#define WABT_INTERPRETER_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "common.h"
#include "binding-hash.h"
#include "opcode.h"
#include "writer.h"

namespace wabt {

class Stream;

namespace interpreter {

#define FOREACH_INTERPRETER_RESULT(V)                                       \
  V(Ok, "ok")                                                               \
  /* returned from the top-most function */                                 \
  V(Returned, "returned")                                                   \
  /* memory access is out of bounds */                                      \
  V(TrapMemoryAccessOutOfBounds, "out of bounds memory access")             \
  /* converting from float -> int would overflow int */                     \
  V(TrapIntegerOverflow, "integer overflow")                                \
  /* dividend is zero in integer divide */                                  \
  V(TrapIntegerDivideByZero, "integer divide by zero")                      \
  /* converting from float -> int where float is nan */                     \
  V(TrapInvalidConversionToInteger, "invalid conversion to integer")        \
  /* function table index is out of bounds */                               \
  V(TrapUndefinedTableIndex, "undefined table index")                       \
  /* function table element is uninitialized */                             \
  V(TrapUninitializedTableElement, "uninitialized table element")           \
  /* unreachable instruction executed */                                    \
  V(TrapUnreachable, "unreachable executed")                                \
  /* call indirect signature doesn't match function table signature */      \
  V(TrapIndirectCallSignatureMismatch, "indirect call signature mismatch")  \
  /* ran out of call stack frames (probably infinite recursion) */          \
  V(TrapCallStackExhausted, "call stack exhausted")                         \
  /* ran out of value stack space */                                        \
  V(TrapValueStackExhausted, "value stack exhausted")                       \
  /* we called a host function, but the return value didn't match the */    \
  /* expected type */                                                       \
  V(TrapHostResultTypeMismatch, "host result type mismatch")                \
  /* we called an import function, but it didn't complete succesfully */    \
  V(TrapHostTrapped, "host function trapped")                               \
  /* we attempted to call a function with the an argument list that doesn't \
   * match the function signature */                                        \
  V(ArgumentTypeMismatch, "argument type mismatch")                         \
  /* we tried to get an export by name that doesn't exist */                \
  V(UnknownExport, "unknown export")                                        \
  /* the expected export kind doesn't match. */                             \
  V(ExportKindMismatch, "export kind mismatch")

enum class Result {
#define V(Name, str) Name,
  FOREACH_INTERPRETER_RESULT(V)
#undef V
};

typedef uint32_t IstreamOffset;
static const IstreamOffset kInvalidIstreamOffset = ~0;

// A table entry has the following packed layout:
//
//   struct {
//     IstreamOffset offset;
//     uint32_t drop_count;
//     uint8_t keep_count;
//   };
#define WABT_TABLE_ENTRY_SIZE \
  (sizeof(IstreamOffset) + sizeof(uint32_t) + sizeof(uint8_t))
#define WABT_TABLE_ENTRY_OFFSET_OFFSET 0
#define WABT_TABLE_ENTRY_DROP_OFFSET sizeof(uint32_t)
#define WABT_TABLE_ENTRY_KEEP_OFFSET (sizeof(IstreamOffset) + sizeof(uint32_t))

enum class Opcode {
/* push space on the value stack for N entries */
#define WABT_OPCODE(rtype, type1, type2, mem_size, code, Name, text) \
  Name = code,
#include "interpreter-opcode.def"
#undef WABT_OPCODE

  First = static_cast<int>(::wabt::Opcode::First),
  Last = DropKeep,
};
static const int kOpcodeCount = WABT_ENUM_COUNT(Opcode);

struct FuncSignature {
  std::vector<Type> param_types;
  std::vector<Type> result_types;
};

struct Table {
  explicit Table(const Limits& limits)
      : limits(limits), func_indexes(limits.initial, kInvalidIndex) {}

  Limits limits;
  std::vector<Index> func_indexes;
};

struct Memory {
  Memory() { WABT_ZERO_MEMORY(page_limits); }
  explicit Memory(const Limits& limits)
      : page_limits(limits), data(limits.initial * WABT_PAGE_SIZE) {}

  Limits page_limits;
  std::vector<char> data;
};

union Value {
  uint32_t i32;
  uint64_t i64;
  uint32_t f32_bits;
  uint64_t f64_bits;
};

struct TypedValue {
  TypedValue() {}
  explicit TypedValue(Type type) : type(type) {}
  TypedValue(Type type, const Value& value) : type(type), value(value) {}

  Type type;
  Value value;
};

struct Global {
  Global() : mutable_(false), import_index(kInvalidIndex) {}
  Global(const TypedValue& typed_value, bool mutable_)
      : typed_value(typed_value), mutable_(mutable_) {}

  TypedValue typed_value;
  bool mutable_;
  Index import_index; /* or INVALID_INDEX if not imported */
};

struct Import {
  Import();
  Import(Import&&);
  Import& operator=(Import&&);
  ~Import();

  StringSlice module_name;
  StringSlice field_name;
  ExternalKind kind;
  union {
    struct {
      Index sig_index;
    } func;
    struct {
      Limits limits;
    } table, memory;
    struct {
      Type type;
      bool mutable_;
    } global;
  };
};

struct Func;

typedef Result (*HostFuncCallback)(const struct HostFunc* func,
                                   const FuncSignature* sig,
                                   Index num_args,
                                   TypedValue* args,
                                   Index num_results,
                                   TypedValue* out_results,
                                   void* user_data);

struct Func {
  WABT_DISALLOW_COPY_AND_ASSIGN(Func);
  Func(Index sig_index, bool is_host)
      : sig_index(sig_index), is_host(is_host) {}
  virtual ~Func() {}

  inline struct DefinedFunc* as_defined();
  inline struct HostFunc* as_host();

  Index sig_index;
  bool is_host;
};

struct DefinedFunc : Func {
  DefinedFunc(Index sig_index)
      : Func(sig_index, false),
        offset(kInvalidIstreamOffset),
        local_decl_count(0),
        local_count(0) {}

  IstreamOffset offset;
  Index local_decl_count;
  Index local_count;
  std::vector<Type> param_and_local_types;
};

struct HostFunc : Func {
  HostFunc(const StringSlice& module_name,
           const StringSlice& field_name,
           Index sig_index)
      : Func(sig_index, true),
        module_name(module_name),
        field_name(field_name) {}

  StringSlice module_name;
  StringSlice field_name;
  HostFuncCallback callback;
  void* user_data;
};

DefinedFunc* Func::as_defined() {
  assert(!is_host);
  return static_cast<DefinedFunc*>(this);
}

HostFunc* Func::as_host() {
  assert(is_host);
  return static_cast<HostFunc*>(this);
}

struct Export {
  Export(const StringSlice& name, ExternalKind kind, Index index)
      : name(name), kind(kind), index(index) {}
  Export(Export&&);
  Export& operator=(Export&&);
  ~Export();

  StringSlice name;
  ExternalKind kind;
  Index index;
};

struct PrintErrorCallback {
  void* user_data;
  void (*print_error)(const char* msg, void* user_data);
};

struct HostImportDelegate {
  void* user_data;
  ::wabt::Result (*import_func)(Import*,
                        Func*,
                        FuncSignature*,
                        PrintErrorCallback,
                        void* user_data);
  ::wabt::Result (*import_table)(Import*,
                                 Table*,
                                 PrintErrorCallback,
                                 void* user_data);
  ::wabt::Result (*import_memory)(Import*,
                                  Memory*,
                                  PrintErrorCallback,
                                  void* user_data);
  ::wabt::Result (*import_global)(Import*,
                                  Global*,
                                  PrintErrorCallback,
                                  void* user_data);
};

struct Module {
  WABT_DISALLOW_COPY_AND_ASSIGN(Module);
  explicit Module(bool is_host);
  Module(const StringSlice& name, bool is_host);
  virtual ~Module();

  inline struct DefinedModule* as_defined();
  inline struct HostModule* as_host();

  StringSlice name;
  std::vector<Export> exports;
  BindingHash export_bindings;
  Index memory_index; /* kInvalidIndex if not defined */
  Index table_index;  /* kInvalidIndex if not defined */
  bool is_host;
};

struct DefinedModule : Module {
  explicit DefinedModule(size_t istream_start);

  std::vector<Import> imports;
  Index start_func_index; /* kInvalidIndex if not defined */
  size_t istream_start;
  size_t istream_end;
};

struct HostModule : Module {
  HostModule(const StringSlice& name);

  HostImportDelegate import_delegate;
};

DefinedModule* Module::as_defined() {
  assert(!is_host);
  return static_cast<DefinedModule*>(this);
}

HostModule* Module::as_host() {
  assert(is_host);
  return static_cast<HostModule*>(this);
}

/* Used to track and reset the state of the environment. */
struct EnvironmentMark {
  size_t modules_size;
  size_t sigs_size;
  size_t funcs_size;
  size_t memories_size;
  size_t tables_size;
  size_t globals_size;
  size_t istream_size;
};

struct Environment {
  Environment();

  std::vector<std::unique_ptr<Module>> modules;
  std::vector<FuncSignature> sigs;
  std::vector<std::unique_ptr<Func>> funcs;
  std::vector<Memory> memories;
  std::vector<Table> tables;
  std::vector<Global> globals;
  std::unique_ptr<OutputBuffer> istream;
  BindingHash module_bindings;
  BindingHash registered_module_bindings;
};

struct Thread {
  Thread();

  Environment* env;
  std::vector<Value> value_stack;
  std::vector<IstreamOffset> call_stack;
  Value* value_stack_top;
  Value* value_stack_end;
  IstreamOffset* call_stack_top;
  IstreamOffset* call_stack_end;
  IstreamOffset pc;
};

// TODO(binji): Remove and use default constructor.
#define WABT_INTERPRETER_THREAD_OPTIONS_DEFAULT                 \
  {                                                             \
    512 * 1024 / sizeof(::wabt::interpreter::Value), 64 * 1024, \
        ::wabt::interpreter::kInvalidIstreamOffset              \
  }

struct ThreadOptions {
  uint32_t value_stack_size;
  uint32_t call_stack_size;
  IstreamOffset pc;
};

bool is_canonical_nan_f32(uint32_t f32_bits);
bool is_canonical_nan_f64(uint64_t f64_bits);
bool is_arithmetic_nan_f32(uint32_t f32_bits);
bool is_arithmetic_nan_f64(uint64_t f64_bits);
bool func_signatures_are_equal(Environment* env,
                               Index sig_index_0,
                               Index sig_index_1);

// TODO(binji): Use methods on Environment and Thread instead.
void destroy_environment(Environment* env);
EnvironmentMark mark_environment(Environment* env);
void reset_environment_to_mark(Environment* env, EnvironmentMark mark);
HostModule* append_host_module(Environment* env, StringSlice name);
void init_thread(Environment* env, Thread* thread, ThreadOptions* options);
Result push_thread_value(Thread* thread, Value value);
void destroy_thread(Thread* thread);
Result call_host(Thread* thread, HostFunc* func);
Result run_interpreter(Thread* thread,
                       int num_instructions,
                       IstreamOffset* call_stack_return_top);
void trace_pc(Thread* thread, Stream* stream);
void disassemble(Environment* env,
                 Stream* stream,
                 IstreamOffset from,
                 IstreamOffset to);
void disassemble_module(Environment* env, Stream* stream, Module* module);

Export* get_export_by_name(Module* module, const StringSlice* name);

}  // namespace interpreter
}  // namespace wabt

#endif /* WABT_INTERPRETER_H_ */
