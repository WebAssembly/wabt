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
#include "intrusive-list.h"
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

  Const() : Const(I32(), 0, Location()) {}
  Const(I32, uint32_t val = 0, const Location& loc = Location());
  Const(I64, uint64_t val = 0, const Location& loc = Location());
  Const(F32, uint32_t val = 0, const Location& loc = Location());
  Const(F64, uint64_t val = 0, const Location& loc = Location());

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
  Unreachable,

  First = Binary,
  Last = Unreachable
};

const char* GetExprTypeName(ExprType type);

typedef TypeVector BlockSignature;

class Expr;
typedef intrusive_list<Expr> ExprList;

struct Block {
  WABT_DISALLOW_COPY_AND_ASSIGN(Block);
  Block();
  explicit Block(ExprList exprs);
  ~Block();

  Label label;
  BlockSignature sig;
  ExprList exprs;
};

struct Catch {
  WABT_DISALLOW_COPY_AND_ASSIGN(Catch);
  Catch() = delete;
  explicit Catch(ExprList exprs);
  Catch(Var var, ExprList exprs);
  Location loc;
  Var var;
  ExprList exprs;
  bool IsCatchAll() const {
    return var.type == VarType::Index && var.index == kInvalidIndex;
  }
};

typedef std::vector<Catch*> CatchVector;

class Expr : public intrusive_list_base<Expr> {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Expr);
  Expr() = delete;
  virtual ~Expr() = default;

  Location loc;
  ExprType type;

 protected:
  explicit Expr(ExprType);
};

const char* GetExprTypeName(const Expr& expr);

template <ExprType TypeEnum>
class ExprMixin : public Expr {
 public:
  static bool classof(const Expr* expr) { return expr->type == TypeEnum; }

  ExprMixin() : Expr(TypeEnum) {}
};

typedef ExprMixin<ExprType::CurrentMemory> CurrentMemoryExpr;
typedef ExprMixin<ExprType::Drop> DropExpr;
typedef ExprMixin<ExprType::GrowMemory> GrowMemoryExpr;
typedef ExprMixin<ExprType::Nop> NopExpr;
typedef ExprMixin<ExprType::Return> ReturnExpr;
typedef ExprMixin<ExprType::Select> SelectExpr;
typedef ExprMixin<ExprType::Unreachable> UnreachableExpr;

template <ExprType TypeEnum>
class OpcodeExpr : public ExprMixin<TypeEnum> {
 public:
  OpcodeExpr(Opcode opcode) : opcode(opcode) {}

  Opcode opcode;
};

typedef OpcodeExpr<ExprType::Binary> BinaryExpr;
typedef OpcodeExpr<ExprType::Compare> CompareExpr;
typedef OpcodeExpr<ExprType::Convert> ConvertExpr;
typedef OpcodeExpr<ExprType::Unary> UnaryExpr;

template <ExprType TypeEnum>
class VarExpr : public ExprMixin<TypeEnum> {
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

template <ExprType TypeEnum>
class BlockExprBase : public ExprMixin<TypeEnum> {
 public:
  explicit BlockExprBase(Block* block) : block(block) {}
  ~BlockExprBase() { delete block; }

  Block* block;
};

typedef BlockExprBase<ExprType::Block> BlockExpr;
typedef BlockExprBase<ExprType::Loop> LoopExpr;

class IfExpr : public ExprMixin<ExprType::If> {
 public:
  explicit IfExpr(Block* true_block, ExprList false_expr = ExprList())
      : true_(true_block), false_(std::move(false_expr)) {}
  ~IfExpr();

  Block* true_;
  ExprList false_;
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

template <ExprType TypeEnum>
class LoadStoreExpr : public ExprMixin<TypeEnum> {
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
  ExprList exprs;
};

struct Global {
  WABT_DISALLOW_COPY_AND_ASSIGN(Global);
  Global();
  ~Global();

  StringSlice name;
  Type type;
  bool mutable_;
  ExprList init_expr;
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

  Var table_var;
  ExprList offset;
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
  ExprList offset;
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

class ModuleField {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(ModuleField);
  ModuleField() = delete;
  virtual ~ModuleField() {}

  Location loc;
  ModuleFieldType type;
  ModuleField* next;

 protected:
  explicit ModuleField(ModuleFieldType, const Location& loc);
};

template <ModuleFieldType TypeEnum>
class ModuleFieldMixin : public ModuleField {
 public:
  static bool classof(const ModuleField* field) {
    return field->type == TypeEnum;
  }

  explicit ModuleFieldMixin(const Location& loc) : ModuleField(TypeEnum, loc) {}
};

class FuncModuleField : public ModuleFieldMixin<ModuleFieldType::Func> {
 public:
  explicit FuncModuleField(Func* func, const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Func>(loc), func(func) {}
  ~FuncModuleField() { delete func; }

  Func* func;
};

class GlobalModuleField : public ModuleFieldMixin<ModuleFieldType::Global> {
 public:
  explicit GlobalModuleField(Global* global,
                             const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Global>(loc), global(global) {}
  ~GlobalModuleField() { delete global; }

  Global* global;
};

class ImportModuleField : public ModuleFieldMixin<ModuleFieldType::Import> {
 public:
  explicit ImportModuleField(Import* import,
                             const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Import>(loc), import(import) {}
  ~ImportModuleField() { delete import; }

  Import* import;
};

class ExportModuleField : public ModuleFieldMixin<ModuleFieldType::Export> {
 public:
  explicit ExportModuleField(Export* export_,
                             const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Export>(loc), export_(export_) {}
  ~ExportModuleField() { delete export_; }

  Export* export_;
};

class FuncTypeModuleField : public ModuleFieldMixin<ModuleFieldType::FuncType> {
 public:
  explicit FuncTypeModuleField(FuncType* func_type,
                               const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::FuncType>(loc),
        func_type(func_type) {}
  ~FuncTypeModuleField() { delete func_type; }

  FuncType* func_type;
};

class TableModuleField : public ModuleFieldMixin<ModuleFieldType::Table> {
 public:
  explicit TableModuleField(Table* table, const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Table>(loc), table(table) {}
  ~TableModuleField() { delete table; }

  Table* table;
};

class ElemSegmentModuleField
    : public ModuleFieldMixin<ModuleFieldType::ElemSegment> {
 public:
  explicit ElemSegmentModuleField(ElemSegment* elem_segment,
                                  const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::ElemSegment>(loc),
        elem_segment(elem_segment) {}
  ~ElemSegmentModuleField() { delete elem_segment; }

  ElemSegment* elem_segment;
};

class MemoryModuleField : public ModuleFieldMixin<ModuleFieldType::Memory> {
 public:
  explicit MemoryModuleField(Memory* memory,
                             const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Memory>(loc), memory(memory) {}
  ~MemoryModuleField() { delete memory; }

  Memory* memory;
};

class DataSegmentModuleField
    : public ModuleFieldMixin<ModuleFieldType::DataSegment> {
 public:
  explicit DataSegmentModuleField(DataSegment* data_segment,
                                  const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::DataSegment>(loc),
        data_segment(data_segment) {}
  ~DataSegmentModuleField() { delete data_segment; }

  DataSegment* data_segment;
};

class ExceptionModuleField : public ModuleFieldMixin<ModuleFieldType::Except> {
 public:
  explicit ExceptionModuleField(Exception* except,
                                const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Except>(loc), except(except) {}
  ~ExceptionModuleField() { delete except; }

  Exception* except;
};

class StartModuleField : public ModuleFieldMixin<ModuleFieldType::Start> {
 public:
  explicit StartModuleField(Var start = Var(),
                            const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Start>(loc), start(start) {}

  Var start;
};

struct Module {
  WABT_DISALLOW_COPY_AND_ASSIGN(Module);
  Module();
  ~Module();

  void AppendField(ModuleField*);
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

class Command {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Command);
  Command() = delete;
  virtual ~Command() {}

  CommandType type;

 protected:
  explicit Command(CommandType type) : type(type) {}
};

template <CommandType TypeEnum>
class CommandMixin : public Command {
 public:
  static bool classof(const Command* cmd) { return cmd->type == TypeEnum; }
  CommandMixin() : Command(TypeEnum) {}
};

class ModuleCommand : public CommandMixin<CommandType::Module> {
 public:
  explicit ModuleCommand(Module* module) : module(module) {}
  ~ModuleCommand() { delete module; }

  Module* module;
};

template <CommandType TypeEnum>
class ActionCommandBase : public CommandMixin<TypeEnum> {
 public:
  explicit ActionCommandBase(Action* action) : action(action) {}
  ~ActionCommandBase() { delete action; }

  Action* action;
};

typedef ActionCommandBase<CommandType::Action> ActionCommand;
typedef ActionCommandBase<CommandType::AssertReturnCanonicalNan>
    AssertReturnCanonicalNanCommand;
typedef ActionCommandBase<CommandType::AssertReturnArithmeticNan>
    AssertReturnArithmeticNanCommand;

class RegisterCommand : public CommandMixin<CommandType::Register> {
 public:
  RegisterCommand(StringSlice module_name, const Var& var)
      : module_name(module_name), var(var) {}
  ~RegisterCommand() { destroy_string_slice(&module_name); }

  StringSlice module_name;
  Var var;
};

class AssertReturnCommand : public CommandMixin<CommandType::AssertReturn> {
 public:
  AssertReturnCommand(Action* action, ConstVector* expected)
      : action(action), expected(expected) {}
  ~AssertReturnCommand() {
    delete action;
    delete expected;
  }

  Action* action;
  ConstVector* expected;
};

template <CommandType TypeEnum>
class AssertTrapCommandBase : public CommandMixin<TypeEnum> {
 public:
  AssertTrapCommandBase(Action* action, StringSlice text)
      : action(action), text(text) {}
  ~AssertTrapCommandBase() {
    delete action;
    destroy_string_slice(&text);
  }

  Action* action;
  StringSlice text;
};

typedef AssertTrapCommandBase<CommandType::AssertTrap> AssertTrapCommand;
typedef AssertTrapCommandBase<CommandType::AssertExhaustion>
    AssertExhaustionCommand;

template <CommandType TypeEnum>
class AssertModuleCommand : public CommandMixin<TypeEnum> {
 public:
  AssertModuleCommand(ScriptModule* module, StringSlice text)
      : module(module), text(text) {}
  ~AssertModuleCommand() {
    delete module;
    destroy_string_slice(&text);
  }

  ScriptModule* module;
  StringSlice text;
};

typedef AssertModuleCommand<CommandType::AssertMalformed>
    AssertMalformedCommand;
typedef AssertModuleCommand<CommandType::AssertInvalid> AssertInvalidCommand;
typedef AssertModuleCommand<CommandType::AssertUnlinkable>
    AssertUnlinkableCommand;
typedef AssertModuleCommand<CommandType::AssertUninstantiable>
    AssertUninstantiableCommand;

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

void MakeTypeBindingReverseMapping(
    const TypeVector& types,
    const BindingHash&  bindings,
    std::vector<std::string>* out_reverse_mapping);

}  // namespace wabt

#endif /* WABT_IR_H_ */
