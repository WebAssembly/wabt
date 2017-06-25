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

#ifndef WABT_IR_H_
#define WABT_IR_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#include "binding-hash.h"
#include "common.h"
#include "opcode.h"

namespace wabt {

enum class VarType {
  Index,
  Name,
};

struct Var {
  explicit Var(Index index = kInvalidIndex);
  explicit Var(const StringSlice& name);
  Var(Index index, const Location& loc);
  Var(const StringSlice& name, const Location& loc);
  Var(Var&&);
  Var(const Var&);
  Var& operator =(const Var&);
  Var& operator =(Var&&);
  ~Var();

  Location loc;
  VarType type;
  union {
    Index index;
    StringSlice name;
  };
};
typedef std::vector<Var> VarVector;

typedef StringSlice Label;

struct Const {
  // Struct tags to differentiate constructors.
  struct I32 {};
  struct I64 {};
  struct F32 {};
  struct F64 {};

  // Keep the default constructor trivial so it can be used as a union member.
  Const() = default;
  Const(I32, uint32_t);
  Const(I64, uint64_t);
  Const(F32, uint32_t);
  Const(F64, uint64_t);
  // TODO(binji): Make these default arguments on the constructors above when
  // Location has a constructor.
  Const(I32, uint32_t, const Location& loc);
  Const(I64, uint64_t, const Location& loc);
  Const(F32, uint32_t, const Location& loc);
  Const(F64, uint64_t, const Location& loc);

  Location loc;
  Type type;
  union {
    uint32_t u32;
    uint64_t u64;
    uint32_t f32_bits;
    uint64_t f64_bits;
  };
};
typedef std::vector<Const> ConstVector;

enum class ExprType {
  Binary,
  Block,
  Br,
  BrIf,
  BrTable,
  Call,
  CallIndirect,
  Compare,
  Const,
  Convert,
  CurrentMemory,
  Drop,
  GetGlobal,
  GetLocal,
  GrowMemory,
  If,
  Load,
  Loop,
  Nop,
  Rethrow,
  Return,
  Select,
  SetGlobal,
  SetLocal,
  Store,
  TeeLocal,
  Throw,
  TryBlock,
  Unary,
  Unreachable,  // Unreachable must appear last
};

const char* GetExprTypeName(ExprType type);

typedef TypeVector BlockSignature;

class Expr;

struct Block {
  WABT_DISALLOW_COPY_AND_ASSIGN(Block);
  Block();
  explicit Block(Expr* first);
  ~Block();

  Label label;
  BlockSignature sig;
  Expr* first;
};

struct Catch {
  WABT_DISALLOW_COPY_AND_ASSIGN(Catch);
  Catch() = delete;
  explicit Catch(Expr* first);
  Catch(Var var, Expr* first);
  ~Catch();
  Location loc;
  Var var;
  Expr* first;
  bool IsCatchAll() const {
    return var.type == VarType::Index && var.index == kInvalidIndex;
  }
};

typedef std::vector<Catch*> CatchVector;

class Expr {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Expr);
  Expr() = delete;
  virtual ~Expr() {}

  template <typename T>
  bool Is() const {
    WABT_STATIC_ASSERT((std::is_base_of<Expr, T>::value));
    return type == T::kStaticType;
  };

  template <typename T>
  const T* As() const {
    assert(Is<T>());
    return static_cast<const T*>(this);
  }

  template <typename T>
  T* As() {
    assert(Is<T>());
    return static_cast<T*>(this);
  }

  Location loc;
  ExprType type;
  Expr* next;

 protected:
  explicit Expr(ExprType);
};

const char* GetExprTypeName(const Expr& expr);

template <ExprType T>
class ExprMixin : public Expr {
 public:
  static const ExprType kStaticType = T;
  ExprMixin() : Expr(T) {}
};

typedef ExprMixin<ExprType::CurrentMemory> CurrentMemoryExpr;
typedef ExprMixin<ExprType::Drop> DropExpr;
typedef ExprMixin<ExprType::GrowMemory> GrowMemoryExpr;
typedef ExprMixin<ExprType::Nop> NopExpr;
typedef ExprMixin<ExprType::Return> ReturnExpr;
typedef ExprMixin<ExprType::Select> SelectExpr;
typedef ExprMixin<ExprType::Unreachable> UnreachableExpr;

template <ExprType T>
class OpcodeExpr : public ExprMixin<T> {
 public:
  OpcodeExpr(Opcode opcode) : opcode(opcode) {}

  Opcode opcode;
};

typedef OpcodeExpr<ExprType::Binary> BinaryExpr;
typedef OpcodeExpr<ExprType::Compare> CompareExpr;
typedef OpcodeExpr<ExprType::Convert> ConvertExpr;
typedef OpcodeExpr<ExprType::Unary> UnaryExpr;

template <ExprType T>
class VarExpr : public ExprMixin<T> {
 public:
  VarExpr(const Var& var) : var(var) {}

  Var var;
};

typedef VarExpr<ExprType::Br> BrExpr;
typedef VarExpr<ExprType::BrIf> BrIfExpr;
typedef VarExpr<ExprType::Call> CallExpr;
typedef VarExpr<ExprType::CallIndirect> CallIndirectExpr;
typedef VarExpr<ExprType::GetGlobal> GetGlobalExpr;
typedef VarExpr<ExprType::GetLocal> GetLocalExpr;
typedef VarExpr<ExprType::Rethrow> RethrowExpr;
typedef VarExpr<ExprType::SetGlobal> SetGlobalExpr;
typedef VarExpr<ExprType::SetLocal> SetLocalExpr;
typedef VarExpr<ExprType::TeeLocal> TeeLocalExpr;
typedef VarExpr<ExprType::Throw> ThrowExpr;

template <ExprType type>
class BlockExprBase : public ExprMixin<type> {
 public:
  explicit BlockExprBase(Block* block) : block(block) {}
  ~BlockExprBase() { delete block; }

  Block* block;
};

typedef BlockExprBase<ExprType::Block> BlockExpr;
typedef BlockExprBase<ExprType::Loop> LoopExpr;

class IfExpr : public ExprMixin<ExprType::If> {
 public:
  explicit IfExpr(Block* true_block, Expr* false_expr = nullptr)
      : true_(true_block), false_(false_expr) {}
  ~IfExpr();

  Block* true_;
  Expr* false_;
};

class TryExpr : public ExprMixin<ExprType::TryBlock> {
 public:
  TryExpr() : block(nullptr) {}
  ~TryExpr();

  Block* block;
  CatchVector catches;
};

class BrTableExpr : public ExprMixin<ExprType::BrTable> {
 public:
  BrTableExpr(VarVector* targets, Var default_target)
      : targets(targets), default_target(default_target) {}
  ~BrTableExpr() { delete targets; }

  VarVector* targets;
  Var default_target;
};

class ConstExpr : public ExprMixin<ExprType::Const> {
 public:
  ConstExpr(const Const& c) : const_(c) {}

  Const const_;
};

template <ExprType T>
class LoadStoreExpr : public ExprMixin<T> {
 public:
  LoadStoreExpr(Opcode opcode, Address align, uint32_t offset)
      : opcode(opcode), align(align), offset(offset) {}

  Opcode opcode;
  Address align;
  uint32_t offset;
};

typedef LoadStoreExpr<ExprType::Load> LoadExpr;
typedef LoadStoreExpr<ExprType::Store> StoreExpr;

struct Exception {
  StringSlice name;
  TypeVector sig;
  ~Exception() { destroy_string_slice(&name); }
};

struct FuncSignature {
  TypeVector param_types;
  TypeVector result_types;

  Index GetNumParams() const { return param_types.size(); }
  Index GetNumResults() const { return result_types.size(); }
  Type GetParamType(Index index) const { return param_types[index]; }
  Type GetResultType(Index index) const { return result_types[index]; }

  bool operator==(const FuncSignature&) const;
};

struct FuncType {
  WABT_DISALLOW_COPY_AND_ASSIGN(FuncType);
  FuncType();
  ~FuncType();

  Index GetNumParams() const { return sig.GetNumParams(); }
  Index GetNumResults() const { return sig.GetNumResults(); }
  Type GetParamType(Index index) const { return sig.GetParamType(index); }
  Type GetResultType(Index index) const { return sig.GetResultType(index); }

  StringSlice name;
  FuncSignature sig;
};

struct FuncDeclaration {
  WABT_DISALLOW_COPY_AND_ASSIGN(FuncDeclaration);
  FuncDeclaration();
  ~FuncDeclaration();

  Index GetNumParams() const { return sig.GetNumParams(); }
  Index GetNumResults() const { return sig.GetNumResults(); }
  Type GetParamType(Index index) const { return sig.GetParamType(index); }
  Type GetResultType(Index index) const { return sig.GetResultType(index); }

  bool has_func_type;
  Var type_var;
  FuncSignature sig;
};

struct Func {
  WABT_DISALLOW_COPY_AND_ASSIGN(Func);
  Func();
  ~Func();

  Type GetParamType(Index index) const { return decl.GetParamType(index); }
  Type GetResultType(Index index) const { return decl.GetResultType(index); }
  Index GetNumParams() const { return decl.GetNumParams(); }
  Index GetNumLocals() const { return local_types.size(); }
  Index GetNumParamsAndLocals() const {
    return GetNumParams() + GetNumLocals();
  }
  Index GetNumResults() const { return decl.GetNumResults(); }
  Index GetLocalIndex(const Var&) const;

  StringSlice name;
  FuncDeclaration decl;
  TypeVector local_types;
  BindingHash param_bindings;
  BindingHash local_bindings;
  Expr* first_expr;
};

struct Global {
  WABT_DISALLOW_COPY_AND_ASSIGN(Global);
  Global();
  ~Global();

  StringSlice name;
  Type type;
  bool mutable_;
  Expr* init_expr;
};

struct Table {
  WABT_DISALLOW_COPY_AND_ASSIGN(Table);
  Table();
  ~Table();

  StringSlice name;
  Limits elem_limits;
};

struct ElemSegment {
  WABT_DISALLOW_COPY_AND_ASSIGN(ElemSegment);
  ElemSegment();
  ~ElemSegment();

  Var table_var;
  Expr* offset;
  VarVector vars;
};

struct Memory {
  WABT_DISALLOW_COPY_AND_ASSIGN(Memory);
  Memory();
  ~Memory();

  StringSlice name;
  Limits page_limits;
};

struct DataSegment {
  WABT_DISALLOW_COPY_AND_ASSIGN(DataSegment);
  DataSegment();
  ~DataSegment();

  Var memory_var;
  Expr* offset;
  char* data;
  size_t size;
};

struct Import {
  WABT_DISALLOW_COPY_AND_ASSIGN(Import);
  Import();
  ~Import();

  StringSlice module_name;
  StringSlice field_name;
  ExternalKind kind;
  union {
    // An imported func has the type Func so it can be more easily included in
    // the Module's vector of funcs, but only the FuncDeclaration will have any
    // useful information.
    Func* func;
    Table* table;
    Memory* memory;
    Global* global;
    Exception* except;
  };
};

struct Export {
  WABT_DISALLOW_COPY_AND_ASSIGN(Export);
  Export();
  ~Export();

  StringSlice name;
  ExternalKind kind;
  Var var;
};

enum class ModuleFieldType {
  Func,
  Global,
  Import,
  Export,
  FuncType,
  Table,
  ElemSegment,
  Memory,
  DataSegment,
  Start,
  Except
};

struct ModuleField {
  WABT_DISALLOW_COPY_AND_ASSIGN(ModuleField);
  ModuleField();
  explicit ModuleField(ModuleFieldType);
  ~ModuleField();

  Location loc;
  ModuleFieldType type;
  ModuleField* next;
  union {
    Func* func;
    Global* global;
    Import* import;
    Export* export_;
    FuncType* func_type;
    Table* table;
    ElemSegment* elem_segment;
    Memory* memory;
    DataSegment* data_segment;
    Exception* except;
    Var start;
  };
};

struct Module {
  WABT_DISALLOW_COPY_AND_ASSIGN(Module);
  Module();
  ~Module();

  ModuleField* AppendField();
  FuncType* AppendImplicitFuncType(const Location&, const FuncSignature&);

  Index GetFuncTypeIndex(const Var&) const;
  Index GetFuncTypeIndex(const FuncDeclaration&) const;
  Index GetFuncTypeIndex(const FuncSignature&) const;
  const FuncType* GetFuncType(const Var&) const;
  FuncType* GetFuncType(const Var&);
  Index GetFuncIndex(const Var&) const;
  const Func* GetFunc(const Var&) const;
  Func* GetFunc(const Var&);
  Index GetTableIndex(const Var&) const;
  Table* GetTable(const Var&);
  Index GetMemoryIndex(const Var&) const;
  Memory* GetMemory(const Var&);
  Index GetGlobalIndex(const Var&) const;
  const Global* GetGlobal(const Var&) const;
  Global* GetGlobal(const Var&);
  const Export* GetExport(const StringSlice&) const;
  Exception* GetExcept(const Var&) const;
  Index GetExceptIndex(const Var&) const;

  Location loc;
  StringSlice name;
  ModuleField* first_field;
  ModuleField* last_field;

  Index num_except_imports;
  Index num_func_imports;
  Index num_table_imports;
  Index num_memory_imports;
  Index num_global_imports;

  // Cached for convenience; the pointers are shared with values that are
  // stored in either ModuleField or Import.
  std::vector<Exception*> excepts;
  std::vector<Func*> funcs;
  std::vector<Global*> globals;
  std::vector<Import*> imports;
  std::vector<Export*> exports;
  std::vector<FuncType*> func_types;
  std::vector<Table*> tables;
  std::vector<ElemSegment*> elem_segments;
  std::vector<Memory*> memories;
  std::vector<DataSegment*> data_segments;
  Var* start;

  BindingHash except_bindings;
  BindingHash func_bindings;
  BindingHash global_bindings;
  BindingHash export_bindings;
  BindingHash func_type_bindings;
  BindingHash table_bindings;
  BindingHash memory_bindings;
};

// A ScriptModule is a module that may not yet be decoded. This allows for text
// and binary parsing errors to be deferred until validation time.
struct ScriptModule {
  enum class Type {
    Text,
    Binary,
    Quoted,
  };

  WABT_DISALLOW_COPY_AND_ASSIGN(ScriptModule);
  ScriptModule();
  ~ScriptModule();

  const Location& GetLocation() const {
    switch (type) {
      case Type::Binary: return binary.loc;
      case Type::Quoted: return quoted.loc;
      default: assert(0); // Fallthrough.
      case Type::Text: return text->loc;
    }
  }

  Type type;

  union {
    Module* text;
    struct {
      Location loc;
      StringSlice name;
      char* data;
      size_t size;
    } binary, quoted;
  };
};

enum class ActionType {
  Invoke,
  Get,
};

struct ActionInvoke {
  WABT_DISALLOW_COPY_AND_ASSIGN(ActionInvoke);
  ActionInvoke();

  ConstVector args;
};

struct Action {
  WABT_DISALLOW_COPY_AND_ASSIGN(Action);
  Action();
  ~Action();

  Location loc;
  ActionType type;
  Var module_var;
  StringSlice name;
  union {
    ActionInvoke* invoke;
    struct {} get;
  };
};

enum class CommandType {
  Module,
  Action,
  Register,
  AssertMalformed,
  AssertInvalid,
  /* This is a module that is invalid, but cannot be written as a binary module
   * (e.g. it has unresolvable names.) */
  AssertInvalidNonBinary,
  AssertUnlinkable,
  AssertUninstantiable,
  AssertReturn,
  AssertReturnCanonicalNan,
  AssertReturnArithmeticNan,
  AssertTrap,
  AssertExhaustion,

  First = Module,
  Last = AssertExhaustion,
};
static const int kCommandTypeCount = WABT_ENUM_COUNT(CommandType);

struct Command {
  WABT_DISALLOW_COPY_AND_ASSIGN(Command);
  Command();
  ~Command();

  CommandType type;
  union {
    Module* module;
    Action* action;
    struct { StringSlice module_name; Var var; } register_;
    struct { Action* action; ConstVector* expected; } assert_return;
    struct {
      Action* action;
    } assert_return_canonical_nan, assert_return_arithmetic_nan;
    struct { Action* action; StringSlice text; } assert_trap;
    struct {
      ScriptModule* module;
      StringSlice text;
    } assert_malformed, assert_invalid, assert_unlinkable,
        assert_uninstantiable;
  };
};
typedef std::vector<std::unique_ptr<Command>> CommandPtrVector;

struct Script {
  WABT_DISALLOW_COPY_AND_ASSIGN(Script);
  Script();

  const Module* GetFirstModule() const;
  Module* GetFirstModule();
  const Module* GetModule(const Var&) const;

  CommandPtrVector commands;
  BindingHash module_bindings;
};

void DestroyExprList(Expr*);

void MakeTypeBindingReverseMapping(
    const TypeVector&,
    const BindingHash&,
    std::vector<std::string>* out_reverse_mapping);

}  // namespace wabt

#endif /* WABT_IR_H_ */
