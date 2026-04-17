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
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "wabt/binding-hash.h"
#include "wabt/common.h"
#include "wabt/component.h"
#include "wabt/intrusive-list.h"
#include "wabt/opcode.h"

namespace wabt {

struct Module;

// VarType (16 bit) and the opt_type_ (16 bit)
// fields of Var forms a 32 bit field.
enum class VarType : uint16_t {
  Index,
  Name,
};

struct Var {
  // Var can represent variables or types.

  // Represent a variable:
  //   has_opt_type() is false
  //   Only used by wast-parser

  // Represent a type:
  //   has_opt_type() is true, is_index() is true
  //   type can be get by to_type()
  //   Binary reader only constructs this variant

  // Represent both a variable and a type:
  //   has_opt_type() is true, is_name() is true
  //   A reference, which index is unknown
  //   Only used by wast-parser

  explicit Var();
  explicit Var(Index index, const Location& loc);
  explicit Var(std::string_view name, const Location& loc);
  explicit Var(Type type, const Location& loc);
  Var(Var&&);
  Var(const Var&);
  Var& operator=(const Var&);
  Var& operator=(Var&&);
  ~Var();

  bool is_index() const { return type_ == VarType::Index; }
  bool is_name() const { return type_ == VarType::Name; }
  bool has_opt_type() const { return opt_type_ < 0; }

  Index index() const {
    assert(is_index());
    return index_;
  }
  const std::string& name() const {
    assert(is_name());
    return name_;
  }
  Type::Enum opt_type() const {
    assert(has_opt_type());
    return static_cast<Type::Enum>(opt_type_);
  }

  void set_index(Index);
  void set_name(std::string&&);
  void set_name(std::string_view);
  void set_opt_type(Type::Enum);
  Type to_type() const;

  Location loc;

 private:
  void Destroy();

  VarType type_;
  // Can be set to Type::Enum types, Type::Any represent no optional type.
  int16_t opt_type_;
  union {
    Index index_;
    std::string name_;
  };
};
using VarVector = std::vector<Var>;

struct Const {
  static constexpr uintptr_t kRefNullBits = ~uintptr_t(0);
  static constexpr uintptr_t kRefAnyValueBits = ~uintptr_t(1);

  Const() : Const(Type::I32, uint32_t(0)) {}

  static Const I32(uint32_t val = 0, const Location& loc = Location()) {
    return Const(Type::I32, val, loc);
  }
  static Const I64(uint64_t val = 0, const Location& loc = Location()) {
    return Const(Type::I64, val, loc);
  }
  static Const F32(uint32_t val = 0, const Location& loc = Location()) {
    return Const(Type::F32, val, loc);
  }
  static Const F64(uint64_t val = 0, const Location& loc = Location()) {
    return Const(Type::F64, val, loc);
  }
  static Const V128(v128 val, const Location& loc = Location()) {
    return Const(Type::V128, val, loc);
  }

  Type type() const { return type_; }
  Type lane_type() const {
    assert(type_ == Type::V128);
    return lane_type_;
  }

  int lane_count() const {
    switch (lane_type()) {
      case Type::I8:  return 16;
      case Type::I16: return 8;
      case Type::I32: return 4;
      case Type::I64: return 2;
      case Type::F32: return 4;
      case Type::F64: return 2;
      default: WABT_UNREACHABLE;
    }
  }

  uint32_t u32() const { return data_.u32(0); }
  uint64_t u64() const { return data_.u64(0); }
  uint32_t f32_bits() const { return data_.f32_bits(0); }
  uint64_t f64_bits() const { return data_.f64_bits(0); }
  uintptr_t ref_bits() const { return data_.To<uintptr_t>(0); }
  v128 vec128() const { return data_; }

  template <typename T>
  T v128_lane(int lane) const {
    return data_.To<T>(lane);
  }

  void set_u32(uint32_t x) { From(Type::I32, x); }
  void set_u64(uint64_t x) { From(Type::I64, x); }
  void set_f32(uint32_t x) { From(Type::F32, x); }
  void set_f64(uint64_t x) { From(Type::F64, x); }

  void set_v128_u8(int lane, uint8_t x) { set_v128_lane(lane, Type::I8, x); }
  void set_v128_u16(int lane, uint16_t x) { set_v128_lane(lane, Type::I16, x); }
  void set_v128_u32(int lane, uint32_t x) { set_v128_lane(lane, Type::I32, x); }
  void set_v128_u64(int lane, uint64_t x) { set_v128_lane(lane, Type::I64, x); }
  void set_v128_f32(int lane, uint32_t x) { set_v128_lane(lane, Type::F32, x); }
  void set_v128_f64(int lane, uint64_t x) { set_v128_lane(lane, Type::F64, x); }

  // Only used for expectations. (e.g. wast assertions)
  void set_f32(ExpectedNan nan) {
    set_f32(0);
    set_expected_nan(0, nan);
  }
  void set_f64(ExpectedNan nan) {
    set_f64(0);
    set_expected_nan(0, nan);
  }
  void set_arrayref() { From<uintptr_t>(Type(Type::ArrayRef, Type::ReferenceNonNull), 0); }
  // AnyRef represents ref.host.
  void set_anyref(uintptr_t x) { From(Type::AnyRef, x); }
  void set_any(uintptr_t x) { From(Type(Type::AnyRef, Type::ReferenceNonNull), x); }
  void set_eqref() { From<uintptr_t>(Type(Type::EqRef, Type::ReferenceNonNull), 0); }
  void set_externref(uintptr_t x) { From(Type::ExternRef, x); }
  void set_extern(uintptr_t x) { From(Type(Type::ExternRef, Type::ReferenceNonNull), x); }
  void set_funcref() { From<uintptr_t>(Type::FuncRef, 0); }
  void set_i31ref() { From<uintptr_t>(Type(Type::I31Ref, Type::ReferenceNonNull), 0); }
  void set_structref() { From<uintptr_t>(Type(Type::StructRef, Type::ReferenceNonNull), 0); }
  void set_null(Type type) { From<uintptr_t>(type, kRefNullBits); }

  bool is_expected_nan(int lane = 0) const {
    return expected_nan(lane) != ExpectedNan::None;
  }

  ExpectedNan expected_nan(int lane = 0) const {
    return lane < 4 ? nan_[lane] : ExpectedNan::None;
  }

  void set_expected_nan(int lane, ExpectedNan nan) {
    if (lane < 4) {
      nan_[lane] = nan;
    }
  }

  // v128 support
  Location loc;

 private:
  template <typename T>
  void set_v128_lane(int lane, Type lane_type, T x) {
    lane_type_ = lane_type;
    From(Type::V128, x, lane);
    set_expected_nan(lane, ExpectedNan::None);
  }

  template <typename T>
  Const(Type type, T data, const Location& loc = Location()) : loc(loc) {
    From<T>(type, data);
  }

  template <typename T>
  void From(Type type, T data, int lane = 0) {
    static_assert(sizeof(T) <= sizeof(data_), "Invalid cast!");
    assert((lane + 1) * sizeof(T) <= sizeof(data_));
    type_ = type;
    data_.From<T>(lane, data);
    set_expected_nan(lane, ExpectedNan::None);
  }

  Type type_;
  Type lane_type_;  // Only valid if type_ == Type::V128.
  v128 data_;
  ExpectedNan nan_[4];
};
using ConstVector = std::vector<Const>;

enum class ExpectationType {
  Values,
  Either,
};

class Expectation {
 public:
  Expectation() = delete;
  virtual ~Expectation() = default;
  ExpectationType type() const { return type_; }

  Location loc;

  ConstVector expected;

 protected:
  explicit Expectation(ExpectationType type, const Location& loc = Location())
      : loc(loc), type_(type) {}

 private:
  ExpectationType type_;
};

template <ExpectationType TypeEnum>
class ExpectationMixin : public Expectation {
 public:
  static bool classof(const Expectation* expectation) {
    return expectation->type() == TypeEnum;
  }

  explicit ExpectationMixin(const Location& loc = Location())
      : Expectation(TypeEnum, loc) {}
};

class ValueExpectation : public ExpectationMixin<ExpectationType::Values> {
 public:
  explicit ValueExpectation(const Location& loc = Location())
      : ExpectationMixin<ExpectationType::Values>(loc) {}
};

struct EitherExpectation : public ExpectationMixin<ExpectationType::Either> {
 public:
  explicit EitherExpectation(const Location& loc = Location())
      : ExpectationMixin<ExpectationType::Either>(loc) {}
};

typedef std::unique_ptr<Expectation> ExpectationPtr;

struct FuncSignature {
  TypeVector param_types;
  TypeVector result_types;

  Index GetNumParams() const { return param_types.size(); }
  Index GetNumResults() const { return result_types.size(); }
  Type GetParamType(Index index) const { return param_types[index]; }
  Type GetResultType(Index index) const { return result_types[index]; }

  bool operator==(const FuncSignature&) const;
};

enum class TypeEntryKind {
  Func,
  Struct,
  Array,
};

struct TypeEntrySupertypesInfo {
  TypeEntrySupertypesInfo(bool is_final_sub_type)
      : is_final_sub_type(is_final_sub_type) {}

  void InitSubTypes(Index* sub_type_list, Index sub_type_count);

  bool is_final_sub_type;
  // The binary/text format allows any number of subtypes.
  // Currently, validator rejects lists which size is greater
  // than 1, but this might be changed in the future.
  VarVector sub_types;
};

class TypeEntry {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(TypeEntry);

  virtual ~TypeEntry() = default;

  TypeEntryKind kind() const { return kind_; }

  Location loc;
  std::string name;
  TypeEntrySupertypesInfo supertypes;

 protected:
  explicit TypeEntry(TypeEntryKind kind,
                     bool is_final_sub_type,
                     std::string_view name = std::string_view(),
                     const Location& loc = Location())
      : loc(loc),
        name(name),
        supertypes(is_final_sub_type),
        kind_(kind) {}

  TypeEntryKind kind_;
};

class FuncType : public TypeEntry {
 public:
  static bool classof(const TypeEntry* entry) {
    return entry->kind() == TypeEntryKind::Func;
  }

  explicit FuncType(bool is_final_sub_type, std::string_view name = std::string_view())
      : TypeEntry(TypeEntryKind::Func, is_final_sub_type, name) {}

  Index GetNumParams() const { return sig.GetNumParams(); }
  Index GetNumResults() const { return sig.GetNumResults(); }
  Type GetParamType(Index index) const { return sig.GetParamType(index); }
  Type GetResultType(Index index) const { return sig.GetResultType(index); }

  FuncSignature sig;

  // The BinaryReaderIR tracks whether a FuncType is the target of a tailcall
  // (via a return_call_indirect). wasm2c (CWriter) uses this information to
  // limit its output in some cases.
  struct {
    bool tailcall = false;
  } features_used;
};

struct Field {
  std::string name;
  Type type = Type::Void;
  bool mutable_ = false;
};

class StructType : public TypeEntry {
 public:
  static bool classof(const TypeEntry* entry) {
    return entry->kind() == TypeEntryKind::Struct;
  }

  explicit StructType(bool is_final_sub_type, std::string_view name = std::string_view())
      : TypeEntry(TypeEntryKind::Struct, is_final_sub_type, name) {}

  std::vector<Field> fields;
};

class ArrayType : public TypeEntry {
 public:
  static bool classof(const TypeEntry* entry) {
    return entry->kind() == TypeEntryKind::Array;
  }

  explicit ArrayType(bool is_final_sub_type, std::string_view name = std::string_view())
      : TypeEntry(TypeEntryKind::Array, is_final_sub_type, name) {}

  Field field;
};

struct RecGroupRange {
  Index first_type_index;
  Index type_count;

  Index EndTypeIndex() const { return first_type_index + type_count; }
};

struct FuncDeclaration {
  Index GetNumParams() const { return sig.GetNumParams(); }
  Index GetNumResults() const { return sig.GetNumResults(); }
  Type GetParamType(Index index) const { return sig.GetParamType(index); }
  Type GetResultType(Index index) const { return sig.GetResultType(index); }

  bool has_func_type = false;
  Var type_var;
  FuncSignature sig;
};

enum class ExprType {
  ArrayCopy,
  ArrayFill,
  ArrayGet,
  ArrayInitData,
  ArrayInitElem,
  ArrayNew,
  ArrayNewData,
  ArrayNewDefault,
  ArrayNewElem,
  ArrayNewFixed,
  ArraySet,
  AtomicLoad,
  AtomicRmw,
  AtomicRmwCmpxchg,
  AtomicStore,
  AtomicNotify,
  AtomicFence,
  AtomicWait,
  Binary,
  Quaternary,
  Block,
  Br,
  BrIf,
  BrOnCast,
  BrOnNonNull,
  BrOnNull,
  BrTable,
  Call,
  CallIndirect,
  CallRef,
  CodeMetadata,
  Compare,
  Const,
  Convert,
  Drop,
  GCUnary,
  GlobalGet,
  GlobalSet,
  If,
  Load,
  LocalGet,
  LocalSet,
  LocalTee,
  Loop,
  MemoryCopy,
  DataDrop,
  MemoryFill,
  MemoryGrow,
  MemoryInit,
  MemorySize,
  Nop,
  RefAsNonNull,
  RefCast,
  RefIsNull,
  RefFunc,
  RefNull,
  RefTest,
  Rethrow,
  Return,
  ReturnCall,
  ReturnCallIndirect,
  ReturnCallRef,
  Select,
  SimdLaneOp,
  SimdLoadLane,
  SimdStoreLane,
  SimdShuffleOp,
  StructGet,
  StructNew,
  StructNewDefault,
  StructSet,
  LoadSplat,
  LoadZero,
  Store,
  TableCopy,
  ElemDrop,
  TableInit,
  TableGet,
  TableGrow,
  TableSize,
  TableSet,
  TableFill,
  Ternary,
  Throw,
  ThrowRef,
  Try,
  TryTable,
  Unary,
  Unreachable,

  First = ArrayCopy,
  Last = Unreachable
};

const char* GetExprTypeName(ExprType type);

class Expr;
using ExprList = intrusive_list<Expr>;

using BlockDeclaration = FuncDeclaration;

struct Block {
  Block() = default;
  explicit Block(ExprList exprs) : exprs(std::move(exprs)) {}

  std::string label;
  BlockDeclaration decl;
  ExprList exprs;
  Location end_loc;
};

struct Catch {
  explicit Catch(const Location& loc = Location()) : loc(loc) {}
  explicit Catch(const Var& var, const Location& loc = Location())
      : loc(loc), var(var) {}
  Location loc;
  Var var;
  ExprList exprs;
  bool IsCatchAll() const {
    return var.is_index() && var.index() == kInvalidIndex;
  }
};
using CatchVector = std::vector<Catch>;

struct TableCatch {
  explicit TableCatch(const Location& loc = Location()) : loc(loc) {}
  Location loc;
  Var tag;
  Var target;
  CatchKind kind;
  bool IsCatchAll() const {
    return kind == CatchKind::CatchAll || kind == CatchKind::CatchAllRef;
  }
  bool IsRef() const {
    return kind == CatchKind::CatchRef || kind == CatchKind::CatchAllRef;
  }
};
using TryTableVector = std::vector<TableCatch>;

enum class TryKind { Plain, Catch, Delegate };

class Expr : public intrusive_list_base<Expr> {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Expr);
  Expr() = delete;
  virtual ~Expr() = default;

  ExprType type() const { return type_; }

  Location loc;

 protected:
  explicit Expr(ExprType type, const Location& loc = Location())
      : loc(loc), type_(type) {}

  ExprType type_;
};

const char* GetExprTypeName(const Expr& expr);

template <ExprType TypeEnum>
class ExprMixin : public Expr {
 public:
  static bool classof(const Expr* expr) { return expr->type() == TypeEnum; }

  explicit ExprMixin(const Location& loc = Location()) : Expr(TypeEnum, loc) {}
};

template <ExprType TypeEnum>
class MemoryExpr : public ExprMixin<TypeEnum> {
 public:
  MemoryExpr(Var memidx, const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc), memidx(memidx) {}

  Var memidx;
};

template <ExprType TypeEnum>
class MemoryBinaryExpr : public ExprMixin<TypeEnum> {
 public:
  MemoryBinaryExpr(Var destmemidx,
                   Var srcmemidx,
                   const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc),
        destmemidx(destmemidx),
        srcmemidx(srcmemidx) {}

  Var destmemidx;
  Var srcmemidx;
};

using DropExpr = ExprMixin<ExprType::Drop>;
using NopExpr = ExprMixin<ExprType::Nop>;
using ReturnExpr = ExprMixin<ExprType::Return>;
using UnreachableExpr = ExprMixin<ExprType::Unreachable>;
using ThrowRefExpr = ExprMixin<ExprType::ThrowRef>;

using MemoryGrowExpr = MemoryExpr<ExprType::MemoryGrow>;
using MemorySizeExpr = MemoryExpr<ExprType::MemorySize>;
using MemoryFillExpr = MemoryExpr<ExprType::MemoryFill>;

using MemoryCopyExpr = MemoryBinaryExpr<ExprType::MemoryCopy>;

template <ExprType TypeEnum>
class RefTypeExpr : public ExprMixin<TypeEnum> {
 public:
  RefTypeExpr(Var type, const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc), type(type) {}

  Var type;
};

using RefNullExpr = RefTypeExpr<ExprType::RefNull>;
using RefIsNullExpr = ExprMixin<ExprType::RefIsNull>;

template <ExprType TypeEnum>
class OpcodeExpr : public ExprMixin<TypeEnum> {
 public:
  OpcodeExpr(Opcode opcode, const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc), opcode(opcode) {}

  Opcode opcode;
};

using BinaryExpr = OpcodeExpr<ExprType::Binary>;
using QuaternaryExpr = OpcodeExpr<ExprType::Quaternary>;
using CompareExpr = OpcodeExpr<ExprType::Compare>;
using ConvertExpr = OpcodeExpr<ExprType::Convert>;
using GCUnaryExpr = OpcodeExpr<ExprType::GCUnary>;
using UnaryExpr = OpcodeExpr<ExprType::Unary>;
using TernaryExpr = OpcodeExpr<ExprType::Ternary>;
using RefAsNonNullExpr = OpcodeExpr<ExprType::RefAsNonNull>;

class SimdLaneOpExpr : public ExprMixin<ExprType::SimdLaneOp> {
 public:
  SimdLaneOpExpr(Opcode opcode, uint64_t val, const Location& loc = Location())
      : ExprMixin<ExprType::SimdLaneOp>(loc), opcode(opcode), val(val) {}

  Opcode opcode;
  uint64_t val;
};

class SimdLoadLaneExpr : public MemoryExpr<ExprType::SimdLoadLane> {
 public:
  SimdLoadLaneExpr(Opcode opcode,
                   Var memidx,
                   Address align,
                   Address offset,
                   uint64_t val,
                   const Location& loc = Location())
      : MemoryExpr<ExprType::SimdLoadLane>(memidx, loc),
        opcode(opcode),
        align(align),
        offset(offset),
        val(val) {}

  Opcode opcode;
  Address align;
  Address offset;
  uint64_t val;
};

class SimdStoreLaneExpr : public MemoryExpr<ExprType::SimdStoreLane> {
 public:
  SimdStoreLaneExpr(Opcode opcode,
                    Var memidx,
                    Address align,
                    Address offset,
                    uint64_t val,
                    const Location& loc = Location())
      : MemoryExpr<ExprType::SimdStoreLane>(memidx, loc),
        opcode(opcode),
        align(align),
        offset(offset),
        val(val) {}

  Opcode opcode;
  Address align;
  Address offset;
  uint64_t val;
};

class SimdShuffleOpExpr : public ExprMixin<ExprType::SimdShuffleOp> {
 public:
  SimdShuffleOpExpr(Opcode opcode, v128 val, const Location& loc = Location())
      : ExprMixin<ExprType::SimdShuffleOp>(loc), opcode(opcode), val(val) {}

  Opcode opcode;
  v128 val;
};

template <ExprType TypeEnum>
class VarExpr : public ExprMixin<TypeEnum> {
 public:
  VarExpr(const Var& var, const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc), var(var) {}

  Var var;
};

template <ExprType TypeEnum>
class MemoryVarExpr : public MemoryExpr<TypeEnum> {
 public:
  MemoryVarExpr(const Var& var, Var memidx, const Location& loc = Location())
      : MemoryExpr<TypeEnum>(memidx, loc), var(var) {}

  Var var;
};

using BrExpr = VarExpr<ExprType::Br>;
using BrIfExpr = VarExpr<ExprType::BrIf>;
using BrOnNonNullExpr = VarExpr<ExprType::BrOnNonNull>;
using BrOnNullExpr = VarExpr<ExprType::BrOnNull>;
using CallExpr = VarExpr<ExprType::Call>;
using RefFuncExpr = VarExpr<ExprType::RefFunc>;
using GlobalGetExpr = VarExpr<ExprType::GlobalGet>;
using GlobalSetExpr = VarExpr<ExprType::GlobalSet>;
using LocalGetExpr = VarExpr<ExprType::LocalGet>;
using LocalSetExpr = VarExpr<ExprType::LocalSet>;
using LocalTeeExpr = VarExpr<ExprType::LocalTee>;
using ReturnCallExpr = VarExpr<ExprType::ReturnCall>;
using ThrowExpr = VarExpr<ExprType::Throw>;
using RethrowExpr = VarExpr<ExprType::Rethrow>;

using DataDropExpr = VarExpr<ExprType::DataDrop>;
using ElemDropExpr = VarExpr<ExprType::ElemDrop>;
using TableGetExpr = VarExpr<ExprType::TableGet>;
using TableSetExpr = VarExpr<ExprType::TableSet>;
using TableGrowExpr = VarExpr<ExprType::TableGrow>;
using TableSizeExpr = VarExpr<ExprType::TableSize>;
using TableFillExpr = VarExpr<ExprType::TableFill>;

using MemoryInitExpr = MemoryVarExpr<ExprType::MemoryInit>;

using ArrayFillExpr = VarExpr<ExprType::ArrayFill>;
using ArrayNewExpr = VarExpr<ExprType::ArrayNew>;
using ArrayNewDefaultExpr = VarExpr<ExprType::ArrayNewDefault>;
using ArraySetExpr = VarExpr<ExprType::ArraySet>;
using RefCastExpr = VarExpr<ExprType::RefCast>;
using RefTestExpr = VarExpr<ExprType::RefTest>;
using StructNewExpr = VarExpr<ExprType::StructNew>;
using StructNewDefaultExpr = VarExpr<ExprType::StructNewDefault>;

class SelectExpr : public ExprMixin<ExprType::Select> {
 public:
  SelectExpr(const Location& loc = Location())
      : ExprMixin<ExprType::Select>(loc) {}
  TypeVector result_type;
};

class TableInitExpr : public ExprMixin<ExprType::TableInit> {
 public:
  TableInitExpr(const Var& segment_index,
                const Var& table_index,
                const Location& loc = Location())
      : ExprMixin<ExprType::TableInit>(loc),
        segment_index(segment_index),
        table_index(table_index) {}

  Var segment_index;
  Var table_index;
};

class TableCopyExpr : public ExprMixin<ExprType::TableCopy> {
 public:
  TableCopyExpr(const Var& dst,
                const Var& src,
                const Location& loc = Location())
      : ExprMixin<ExprType::TableCopy>(loc), dst_table(dst), src_table(src) {}

  Var dst_table;
  Var src_table;
};

class CallIndirectExpr : public ExprMixin<ExprType::CallIndirect> {
 public:
  explicit CallIndirectExpr(const Location& loc = Location())
      : ExprMixin<ExprType::CallIndirect>(loc) {}

  FuncDeclaration decl;
  Var table;
};

class CodeMetadataExpr : public ExprMixin<ExprType::CodeMetadata> {
 public:
  explicit CodeMetadataExpr(std::string_view name,
                            std::vector<uint8_t> data,
                            const Location& loc = Location())
      : ExprMixin<ExprType::CodeMetadata>(loc),
        name(std::move(name)),
        data(std::move(data)) {}

  std::string_view name;
  std::vector<uint8_t> data;
};

class ReturnCallIndirectExpr : public ExprMixin<ExprType::ReturnCallIndirect> {
 public:
  explicit ReturnCallIndirectExpr(const Location& loc = Location())
      : ExprMixin<ExprType::ReturnCallIndirect>(loc) {}

  FuncDeclaration decl;
  Var table;
};

class CallRefExpr : public ExprMixin<ExprType::CallRef> {
 public:
  explicit CallRefExpr(const Location& loc = Location())
      : ExprMixin<ExprType::CallRef>(loc) {}

  Var sig_type;
};

class ReturnCallRefExpr : public ExprMixin<ExprType::ReturnCallRef> {
 public:
  explicit ReturnCallRefExpr(const Location& loc = Location())
      : ExprMixin<ExprType::ReturnCallRef>(loc) {}

  Var sig_type;
};

template <ExprType TypeEnum>
class BlockExprBase : public ExprMixin<TypeEnum> {
 public:
  explicit BlockExprBase(const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc) {}

  Block block;
};

using BlockExpr = BlockExprBase<ExprType::Block>;
using LoopExpr = BlockExprBase<ExprType::Loop>;

class IfExpr : public ExprMixin<ExprType::If> {
 public:
  explicit IfExpr(const Location& loc = Location())
      : ExprMixin<ExprType::If>(loc) {}

  Block true_;
  ExprList false_;
  Location false_end_loc;
};

class TryTableExpr : public ExprMixin<ExprType::TryTable> {
 public:
  explicit TryTableExpr(const Location& loc = Location())
      : ExprMixin<ExprType::TryTable>(loc) {}

  Block block;
  TryTableVector catches;
};

class TryExpr : public ExprMixin<ExprType::Try> {
 public:
  explicit TryExpr(const Location& loc = Location())
      : ExprMixin<ExprType::Try>(loc), kind(TryKind::Plain) {}

  TryKind kind;
  Block block;
  CatchVector catches;
  Var delegate_target;
};

class BrTableExpr : public ExprMixin<ExprType::BrTable> {
 public:
  BrTableExpr(const Location& loc = Location())
      : ExprMixin<ExprType::BrTable>(loc) {}

  VarVector targets;
  Var default_target;
};

class ConstExpr : public ExprMixin<ExprType::Const> {
 public:
  ConstExpr(const Const& c, const Location& loc = Location())
      : ExprMixin<ExprType::Const>(loc), const_(c) {}

  Const const_;
};

// TODO(binji): Rename this, it is used for more than loads/stores now.
template <ExprType TypeEnum>
class LoadStoreExpr : public MemoryExpr<TypeEnum> {
 public:
  LoadStoreExpr(Opcode opcode,
                Var memidx,
                Address align,
                Address offset,
                const Location& loc = Location())
      : MemoryExpr<TypeEnum>(memidx, loc),
        opcode(opcode),
        align(align),
        offset(offset) {}

  Opcode opcode;
  Address align;
  Address offset;
};

using LoadExpr = LoadStoreExpr<ExprType::Load>;
using StoreExpr = LoadStoreExpr<ExprType::Store>;

using AtomicLoadExpr = LoadStoreExpr<ExprType::AtomicLoad>;
using AtomicStoreExpr = LoadStoreExpr<ExprType::AtomicStore>;
using AtomicRmwExpr = LoadStoreExpr<ExprType::AtomicRmw>;
using AtomicRmwCmpxchgExpr = LoadStoreExpr<ExprType::AtomicRmwCmpxchg>;
using AtomicWaitExpr = LoadStoreExpr<ExprType::AtomicWait>;
using AtomicNotifyExpr = LoadStoreExpr<ExprType::AtomicNotify>;
using LoadSplatExpr = LoadStoreExpr<ExprType::LoadSplat>;
using LoadZeroExpr = LoadStoreExpr<ExprType::LoadZero>;

class AtomicFenceExpr : public ExprMixin<ExprType::AtomicFence> {
 public:
  explicit AtomicFenceExpr(uint32_t consistency_model,
                           const Location& loc = Location())
      : ExprMixin<ExprType::AtomicFence>(loc),
        consistency_model(consistency_model) {}

  uint32_t consistency_model;
};

class ArrayGetExpr : public ExprMixin<ExprType::ArrayGet> {
 public:
  ArrayGetExpr(Opcode opcode, const Var& type_var, const Location& loc = Location())
      : ExprMixin<ExprType::ArrayGet>(loc), opcode(opcode), type_var(type_var) {}

  Opcode opcode;
  Var type_var;
};

template <ExprType TypeEnum>
class TypeVarExpr : public ExprMixin<TypeEnum> {
 public:
  TypeVarExpr(const Var& type_var, const Var& var, const Location& loc = Location())
      : ExprMixin<TypeEnum>(loc), type_var(type_var), var(var) {}

  Var type_var;
  Var var;
};

using ArrayCopyExpr = TypeVarExpr<ExprType::ArrayCopy>;
using ArrayInitDataExpr = TypeVarExpr<ExprType::ArrayInitData>;
using ArrayInitElemExpr = TypeVarExpr<ExprType::ArrayInitElem>;
using ArrayNewDataExpr = TypeVarExpr<ExprType::ArrayNewData>;
using ArrayNewElemExpr = TypeVarExpr<ExprType::ArrayNewElem>;
using StructSetExpr = TypeVarExpr<ExprType::StructSet>;

class ArrayNewFixedExpr : public ExprMixin<ExprType::ArrayNewFixed> {
 public:
  ArrayNewFixedExpr(const Var& type_var, Index count, const Location& loc = Location())
      : ExprMixin<ExprType::ArrayNewFixed>(loc), type_var(type_var), count(count) {}

  Var type_var;
  Index count;
};

class StructGetExpr : public ExprMixin<ExprType::StructGet> {
 public:
  StructGetExpr(Opcode opcode, const Var& type_var, const Var& var, const Location& loc = Location())
      : ExprMixin<ExprType::StructGet>(loc), opcode(opcode), type_var(type_var), var(var) {}

  Opcode opcode;
  Var type_var;
  Var var;
};

class BrOnCastExpr : public ExprMixin<ExprType::BrOnCast> {
 public:
  BrOnCastExpr(Opcode opcode, const Var& label_var, const Var& type1_var, const Var& type2_var, const Location& loc = Location())
      : ExprMixin<ExprType::BrOnCast>(loc), opcode(opcode), label_var(label_var), type1_var(type1_var), type2_var(type2_var) {}

  Opcode opcode;
  Var label_var;
  Var type1_var;
  Var type2_var;
};

struct Tag {
  explicit Tag(std::string_view name) : name(name) {}

  std::string name;
  FuncDeclaration decl;
};

class LocalTypes {
 public:
  using Decl = std::pair<Type, Index>;
  using Decls = std::vector<Decl>;

  struct const_iterator {
    const_iterator(Decls::const_iterator decl, Index index)
        : decl(decl), index(index) {}
    Type operator*() const { return decl->first; }
    const_iterator& operator++();
    const_iterator operator++(int);

    Decls::const_iterator decl;
    Index index;
  };

  void Set(const TypeVector&);

  const Decls& decls() const { return decls_; }

  void AppendDecl(Type type, Index count) {
    if (count != 0) {
      decls_.emplace_back(type, count);
    }
  }

  Index size() const;
  Type operator[](Index) const;

  const_iterator begin() const { return {decls_.begin(), 0}; }
  const_iterator end() const { return {decls_.end(), 0}; }

 private:
  Decls decls_;
};

inline LocalTypes::const_iterator& LocalTypes::const_iterator::operator++() {
  ++index;
  if (index >= decl->second) {
    ++decl;
    index = 0;
  }
  return *this;
}

inline LocalTypes::const_iterator LocalTypes::const_iterator::operator++(int) {
  const_iterator result = *this;
  operator++();
  return result;
}

inline bool operator==(const LocalTypes::const_iterator& lhs,
                       const LocalTypes::const_iterator& rhs) {
  return lhs.decl == rhs.decl && lhs.index == rhs.index;
}

inline bool operator!=(const LocalTypes::const_iterator& lhs,
                       const LocalTypes::const_iterator& rhs) {
  return !operator==(lhs, rhs);
}

struct Func {
  explicit Func(std::string_view name) : name(name) {}

  Type GetParamType(Index index) const { return decl.GetParamType(index); }
  Type GetResultType(Index index) const { return decl.GetResultType(index); }
  Type GetLocalType(Index index) const;
  Type GetLocalType(const Var& var) const;
  Index GetNumParams() const { return decl.GetNumParams(); }
  Index GetNumLocals() const { return local_types.size(); }
  Index GetNumParamsAndLocals() const {
    return GetNumParams() + GetNumLocals();
  }
  Index GetNumResults() const { return decl.GetNumResults(); }
  Index GetLocalIndex(const Var&) const;

  std::string name;
  FuncDeclaration decl;
  LocalTypes local_types;
  BindingHash bindings;
  ExprList exprs;
  Location loc;

  // For a subset of features, the BinaryReaderIR tracks whether they are
  // actually used by the function. wasm2c (CWriter) uses this information to
  // limit its output in some cases.
  struct {
    bool tailcall = false;
  } features_used;
};

struct Global {
  explicit Global(std::string_view name) : name(name) {}

  std::string name;
  Type type = Type::Void;
  bool mutable_ = false;
  ExprList init_expr;
};

struct Table {
  explicit Table(std::string_view name)
      : name(name), elem_type(Type::FuncRef) {}

  std::string name;
  Limits elem_limits;
  Type elem_type;
  ExprList init_expr;
};

using ExprListVector = std::vector<ExprList>;

struct ElemSegment {
  explicit ElemSegment(std::string_view name) : name(name) {}
  uint8_t GetFlags(const Module*, bool function_references_enabled) const;

  SegmentKind kind = SegmentKind::Active;
  std::string name;
  Var table_var;
  Type elem_type;
  ExprList offset;
  ExprListVector elem_exprs;
};

struct Memory {
  explicit Memory(std::string_view name) : name(name) {}

  std::string name;
  Limits page_limits;
  uint32_t page_size;
};

struct DataSegment {
  explicit DataSegment(std::string_view name) : name(name) {}
  uint8_t GetFlags(const Module*) const;

  SegmentKind kind = SegmentKind::Active;
  std::string name;
  Var memory_var;
  ExprList offset;
  std::vector<uint8_t> data;
};

class Import {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Import);
  Import() = delete;
  virtual ~Import() = default;

  ExternalKind kind() const { return kind_; }

  std::string module_name;
  std::string field_name;

 protected:
  Import(ExternalKind kind) : kind_(kind) {}

  ExternalKind kind_;
};

template <ExternalKind TypeEnum>
class ImportMixin : public Import {
 public:
  static bool classof(const Import* import) {
    return import->kind() == TypeEnum;
  }

  ImportMixin() : Import(TypeEnum) {}
};

class FuncImport : public ImportMixin<ExternalKind::Func> {
 public:
  explicit FuncImport(std::string_view name = std::string_view())
      : ImportMixin<ExternalKind::Func>(), func(name) {}

  Func func;
};

class TableImport : public ImportMixin<ExternalKind::Table> {
 public:
  explicit TableImport(std::string_view name = std::string_view())
      : ImportMixin<ExternalKind::Table>(), table(name) {}

  Table table;
};

class MemoryImport : public ImportMixin<ExternalKind::Memory> {
 public:
  explicit MemoryImport(std::string_view name = std::string_view())
      : ImportMixin<ExternalKind::Memory>(), memory(name) {}

  Memory memory;
};

class GlobalImport : public ImportMixin<ExternalKind::Global> {
 public:
  explicit GlobalImport(std::string_view name = std::string_view())
      : ImportMixin<ExternalKind::Global>(), global(name) {}

  Global global;
};

class TagImport : public ImportMixin<ExternalKind::Tag> {
 public:
  explicit TagImport(std::string_view name = std::string_view())
      : ImportMixin<ExternalKind::Tag>(), tag(name) {}

  Tag tag;
};

struct Export {
  std::string name;
  ExternalKind kind;
  Var var;
};

enum class ModuleFieldType {
  Func,
  Global,
  Import,
  Export,
  Type,
  Table,
  ElemSegment,
  Memory,
  DataSegment,
  Start,
  Tag
};

class ModuleField : public intrusive_list_base<ModuleField> {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(ModuleField);
  ModuleField() = delete;
  virtual ~ModuleField() = default;

  ModuleFieldType type() const { return type_; }

  Location loc;

 protected:
  ModuleField(ModuleFieldType type, const Location& loc)
      : loc(loc), type_(type) {}

  ModuleFieldType type_;
};

using ModuleFieldList = intrusive_list<ModuleField>;

template <ModuleFieldType TypeEnum>
class ModuleFieldMixin : public ModuleField {
 public:
  static bool classof(const ModuleField* field) {
    return field->type() == TypeEnum;
  }

  explicit ModuleFieldMixin(const Location& loc) : ModuleField(TypeEnum, loc) {}
};

class FuncModuleField : public ModuleFieldMixin<ModuleFieldType::Func> {
 public:
  explicit FuncModuleField(const Location& loc = Location(),
                           std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::Func>(loc), func(name) {}

  Func func;
};

class GlobalModuleField : public ModuleFieldMixin<ModuleFieldType::Global> {
 public:
  explicit GlobalModuleField(const Location& loc = Location(),
                             std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::Global>(loc), global(name) {}

  Global global;
};

class ImportModuleField : public ModuleFieldMixin<ModuleFieldType::Import> {
 public:
  explicit ImportModuleField(const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Import>(loc) {}
  explicit ImportModuleField(std::unique_ptr<Import> import,
                             const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Import>(loc),
        import(std::move(import)) {}

  std::unique_ptr<Import> import;
};

class ExportModuleField : public ModuleFieldMixin<ModuleFieldType::Export> {
 public:
  explicit ExportModuleField(const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Export>(loc) {}

  Export export_;
};

class TypeModuleField : public ModuleFieldMixin<ModuleFieldType::Type> {
 public:
  explicit TypeModuleField(const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Type>(loc) {}

  std::unique_ptr<TypeEntry> type;
};

class TableModuleField : public ModuleFieldMixin<ModuleFieldType::Table> {
 public:
  explicit TableModuleField(const Location& loc = Location(),
                            std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::Table>(loc), table(name) {}

  Table table;
};

class ElemSegmentModuleField
    : public ModuleFieldMixin<ModuleFieldType::ElemSegment> {
 public:
  explicit ElemSegmentModuleField(const Location& loc = Location(),
                                  std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::ElemSegment>(loc),
        elem_segment(name) {}

  ElemSegment elem_segment;
};

class MemoryModuleField : public ModuleFieldMixin<ModuleFieldType::Memory> {
 public:
  explicit MemoryModuleField(const Location& loc = Location(),
                             std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::Memory>(loc), memory(name) {}

  Memory memory;
};

class DataSegmentModuleField
    : public ModuleFieldMixin<ModuleFieldType::DataSegment> {
 public:
  explicit DataSegmentModuleField(const Location& loc = Location(),
                                  std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::DataSegment>(loc),
        data_segment(name) {}

  DataSegment data_segment;
};

class TagModuleField : public ModuleFieldMixin<ModuleFieldType::Tag> {
 public:
  explicit TagModuleField(const Location& loc = Location(),
                          std::string_view name = std::string_view())
      : ModuleFieldMixin<ModuleFieldType::Tag>(loc), tag(name) {}

  Tag tag;
};

class StartModuleField : public ModuleFieldMixin<ModuleFieldType::Start> {
 public:
  explicit StartModuleField(Var start = Var(), const Location& loc = Location())
      : ModuleFieldMixin<ModuleFieldType::Start>(loc), start(start) {}

  Var start;
};

struct Custom {
  explicit Custom(const Location& loc = Location(),
                  std::string_view name = std::string_view(),
                  std::vector<uint8_t> data = std::vector<uint8_t>())
      : name(name), data(data), loc(loc) {}

  std::string name;
  std::vector<uint8_t> data;
  Location loc;
};

struct Module {
  Index GetFuncTypeIndex(const Var&) const;
  Index GetFuncTypeIndex(const FuncDeclaration&) const;
  Index GetFuncTypeIndex(const FuncSignature&) const;
  const FuncType* GetFuncType(const Var&) const;
  FuncType* GetFuncType(const Var&);
  const StructType* GetStructType(const Var&) const;
  StructType* GetStructType(const Var&);
  const ArrayType* GetArrayType(const Var&) const;
  ArrayType* GetArrayType(const Var&);
  Index GetFuncIndex(const Var&) const;
  const Func* GetFunc(const Var&) const;
  Func* GetFunc(const Var&);
  Index GetTableIndex(const Var&) const;
  const Table* GetTable(const Var&) const;
  Table* GetTable(const Var&);
  Index GetMemoryIndex(const Var&) const;
  const Memory* GetMemory(const Var&) const;
  Memory* GetMemory(const Var&);
  Index GetGlobalIndex(const Var&) const;
  const Global* GetGlobal(const Var&) const;
  Global* GetGlobal(const Var&);
  const Export* GetExport(std::string_view) const;
  Tag* GetTag(const Var&) const;
  Index GetTagIndex(const Var&) const;
  const DataSegment* GetDataSegment(const Var&) const;
  DataSegment* GetDataSegment(const Var&);
  Index GetDataSegmentIndex(const Var&) const;
  const ElemSegment* GetElemSegment(const Var&) const;
  ElemSegment* GetElemSegment(const Var&);
  Index GetElemSegmentIndex(const Var&) const;

  bool IsImport(ExternalKind kind, const Var&) const;
  bool IsImport(const Export& export_) const {
    return IsImport(export_.kind, export_.var);
  }

  // TODO(binji): move this into a builder class?
  void AppendField(std::unique_ptr<DataSegmentModuleField>);
  void AppendField(std::unique_ptr<ElemSegmentModuleField>);
  void AppendField(std::unique_ptr<TagModuleField>);
  void AppendField(std::unique_ptr<ExportModuleField>);
  void AppendField(std::unique_ptr<FuncModuleField>);
  void AppendField(std::unique_ptr<TypeModuleField>);
  void AppendField(std::unique_ptr<GlobalModuleField>);
  void AppendField(std::unique_ptr<ImportModuleField>);
  void AppendField(std::unique_ptr<MemoryModuleField>);
  void AppendField(std::unique_ptr<StartModuleField>);
  void AppendField(std::unique_ptr<TableModuleField>);
  void AppendField(std::unique_ptr<ModuleField>);
  void AppendFields(ModuleFieldList*);

  Location loc;
  std::string name;
  std::string_view filename;
  ModuleFieldList fields;

  Index num_tag_imports = 0;
  Index num_func_imports = 0;
  Index num_table_imports = 0;
  Index num_memory_imports = 0;
  Index num_global_imports = 0;

  // Cached for convenience; the pointers are shared with values that are
  // stored in either ModuleField or Import.
  std::vector<Tag*> tags;
  std::vector<Func*> funcs;
  std::vector<Global*> globals;
  std::vector<Import*> imports;
  std::vector<Export*> exports;
  std::vector<TypeEntry*> types;
  // Ordered list of recursive group type ranges.
  std::vector<RecGroupRange> rec_group_ranges;
  std::vector<Table*> tables;
  std::vector<ElemSegment*> elem_segments;
  std::vector<Memory*> memories;
  std::vector<DataSegment*> data_segments;
  std::vector<Var*> starts;
  std::vector<Custom> customs;

  BindingHash tag_bindings;
  BindingHash func_bindings;
  BindingHash global_bindings;
  BindingHash export_bindings;
  BindingHash type_bindings;
  BindingHash table_bindings;
  BindingHash memory_bindings;
  BindingHash data_segment_bindings;
  BindingHash elem_segment_bindings;

  // For a subset of features, the BinaryReaderIR tracks whether they are
  // actually used by the module. wasm2c (CWriter) uses this information to
  // limit its output in some cases.
  struct {
    bool simd = false;
    bool exceptions = false;
    bool threads = false;
  } features_used;

  // The BinaryReaderIR tracks function references used by the module, whether
  // in element segment initializers, global initializers, or functions. wasm2c
  // needs to emit wrappers for any functions that might get used as function
  // references, and uses this information to limit its output.
  std::set<Index> used_func_refs;
};

enum class ScriptModuleType {
  Text,
  Binary,
  Quoted,
};

// A ScriptModule is a module that may not yet be decoded. This allows for text
// and binary parsing errors to be deferred until validation time.
class ScriptModule {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(ScriptModule);
  ScriptModule() = delete;
  virtual ~ScriptModule() = default;

  ScriptModuleType type() const { return type_; }
  virtual const Location& location() const = 0;

 protected:
  explicit ScriptModule(ScriptModuleType type) : type_(type) {}

  ScriptModuleType type_;
};

template <ScriptModuleType TypeEnum>
class ScriptModuleMixin : public ScriptModule {
 public:
  static bool classof(const ScriptModule* script_module) {
    return script_module->type() == TypeEnum;
  }

  ScriptModuleMixin() : ScriptModule(TypeEnum) {}
};

class TextScriptModule : public ScriptModuleMixin<ScriptModuleType::Text> {
 public:
  const Location& location() const override { return module.loc; }

  Module module;
};

template <ScriptModuleType TypeEnum>
class DataScriptModule : public ScriptModuleMixin<TypeEnum> {
 public:
  const Location& location() const override { return loc; }

  Location loc;
  std::string name;
  std::vector<uint8_t> data;
};

using BinaryScriptModule = DataScriptModule<ScriptModuleType::Binary>;
using QuotedScriptModule = DataScriptModule<ScriptModuleType::Quoted>;

enum class ActionType {
  Invoke,
  Get,
};

class Action {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Action);
  Action() = delete;
  virtual ~Action() = default;

  ActionType type() const { return type_; }

  Location loc;
  Var module_var;
  std::string name;

 protected:
  explicit Action(ActionType type, const Location& loc = Location())
      : loc(loc), type_(type) {}

  ActionType type_;
};

using ActionPtr = std::unique_ptr<Action>;

template <ActionType TypeEnum>
class ActionMixin : public Action {
 public:
  static bool classof(const Action* action) {
    return action->type() == TypeEnum;
  }

  explicit ActionMixin(const Location& loc = Location())
      : Action(TypeEnum, loc) {}
};

class GetAction : public ActionMixin<ActionType::Get> {
 public:
  explicit GetAction(const Location& loc = Location())
      : ActionMixin<ActionType::Get>(loc) {}
};

class InvokeAction : public ActionMixin<ActionType::Invoke> {
 public:
  explicit InvokeAction(const Location& loc = Location())
      : ActionMixin<ActionType::Invoke>(loc) {}

  ConstVector args;
};

enum class CommandType {
  Module,
  ScriptModule,
  Action,
  Register,
  AssertMalformed,
  AssertInvalid,
  AssertUnlinkable,
  AssertUninstantiable,
  AssertReturn,
  AssertTrap,
  AssertExhaustion,
  AssertException,

  First = Module,
  Last = AssertException,
};
constexpr int kCommandTypeCount = WABT_ENUM_COUNT(CommandType);

class Command {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Command);
  Command() = delete;
  virtual ~Command() = default;

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
  Module module;
};

class ScriptModuleCommand : public CommandMixin<CommandType::ScriptModule> {
 public:
  // Both the module and the script_module need to be stored since the module
  // has the parsed information about the module, but the script_module has the
  // original contents (binary or quoted).
  Module module;
  std::unique_ptr<ScriptModule> script_module;
};

template <CommandType TypeEnum>
class ActionCommandBase : public CommandMixin<TypeEnum> {
 public:
  ActionPtr action;
};

using ActionCommand = ActionCommandBase<CommandType::Action>;

class RegisterCommand : public CommandMixin<CommandType::Register> {
 public:
  RegisterCommand(std::string_view module_name, const Var& var)
      : module_name(module_name), var(var) {}

  std::string module_name;
  Var var;
};

class AssertReturnCommand : public CommandMixin<CommandType::AssertReturn> {
 public:
  ActionPtr action;
  ExpectationPtr expected;
};

template <CommandType TypeEnum>
class AssertTrapCommandBase : public CommandMixin<TypeEnum> {
 public:
  ActionPtr action;
  std::string text;
};

using AssertTrapCommand = AssertTrapCommandBase<CommandType::AssertTrap>;
using AssertExhaustionCommand =
    AssertTrapCommandBase<CommandType::AssertExhaustion>;

template <CommandType TypeEnum>
class AssertModuleCommand : public CommandMixin<TypeEnum> {
 public:
  std::unique_ptr<ScriptModule> module;
  std::string text;
};

using AssertMalformedCommand =
    AssertModuleCommand<CommandType::AssertMalformed>;
using AssertInvalidCommand = AssertModuleCommand<CommandType::AssertInvalid>;
using AssertUnlinkableCommand =
    AssertModuleCommand<CommandType::AssertUnlinkable>;
using AssertUninstantiableCommand =
    AssertModuleCommand<CommandType::AssertUninstantiable>;

class AssertExceptionCommand
    : public CommandMixin<CommandType::AssertException> {
 public:
  ActionPtr action;
};

using CommandPtr = std::unique_ptr<Command>;
using CommandPtrVector = std::vector<CommandPtr>;

struct Script {
  WABT_DISALLOW_COPY_AND_ASSIGN(Script);
  Script() = default;

  const Module* GetFirstModule() const;
  Module* GetFirstModule();
  const Module* GetModule(const Var&) const;

  CommandPtrVector commands;
  BindingHash module_bindings;
  std::string_view filename;
};

class ComponentData;
class ComponentAliasExport;
class ComponentAliasOuter;
class ComponentValueType;
class ComponentTypeIndex;
class ComponentTypeItems;
class ComponentTypeListFixed;
class ComponentTypeTuple;
class ComponentTypeLabels;
class ComponentTypeResult;
class ComponentTypeFunc;
class ComponentTypeResource;
class ComponentInterfaceType;
class ComponentCanonLift;
class ComponentCanonLower;
class ComponentCanonType;
class ComponentExternal;
class ComponentCoreModule;
class ComponentInstance;
class ComponentInlineInstance;

// Base class of all component definitions.
class ComponentDef {
 public:
  using Section = ComponentSection;
  using Type = ComponentTypeDef;

  // Subtype for Section::CoreInstance and Section::Instance
  enum class Instance : uint8_t {
    Reference,
    Inline,
  };

  // Subtype for Section::Alias
  enum class Alias : uint8_t {
    Export,
    CoreExport,
    Outer,
  };

  // Subtype for Section::Import and Section::Export
  enum class ExternalDescriptor : uint8_t {
    CoreModule,
    Func,
    ValueEq,
    ValueType,
    TypeEq,
    TypeSubResource,
    Component,
    Instance,
    // Only allowed for Component Exports,
    // where the descriptor is optional.
    Unused,
  };

  struct StringLoc {
    StringLoc()
        : str(nullptr) {}
    StringLoc(const std::string* str, Location loc)
        : str(str), loc(loc) {}

    const ComponentStringLoc ToStringLoc() const {
      return ComponentStringLoc(*str, loc);
    }

    const std::string* str;
    Location loc;
  };

  virtual ~ComponentDef() {}

  Section section() const {
    return section_;
  }

  ComponentSort sort() const {
    return sort_;
  }

  Instance instance() const {
    assert(section() == Section::CoreInstance ||
           section() == Section::Instance);
    return static_cast<Instance>(sub_type_);
  }

  Alias alias() const {
    assert(section() == Section::Alias);
    return static_cast<Alias>(sub_type_);
  }

  Type type() const {
    assert(section() == Section::Type);
    return static_cast<Type>(sub_type_);
  }

  ComponentCanon canon() const {
    assert(section() == Section::Canon);
    return static_cast<ComponentCanon>(sub_type_);
  }

  ExternalDescriptor external() const {
    assert(section() == Section::Import || section() == Section::Export);
    return static_cast<ExternalDescriptor>(sub_type_);
  }

  const std::string* Name() const { return name_; }

  void SetName(const std::string* name) {
    name_ = name;
  }

  ComponentData* AsComponent() {
    assert(section() == Section::Component);
    return reinterpret_cast<ComponentData*>(this);
  }

  const ComponentData* AsComponent() const {
    assert(section() == Section::Component);
    return reinterpret_cast<const ComponentData*>(this);
  }

  const ComponentAliasExport* AsAliasExport() const {
    assert(alias() == Alias::Export || alias() == Alias::CoreExport);
    return reinterpret_cast<const ComponentAliasExport*>(this);
  }

  const ComponentAliasOuter* AsAliasOuter() const {
    assert(alias() == Alias::Outer);
    return reinterpret_cast<const ComponentAliasOuter*>(this);
  }

  const ComponentValueType* AsValueType() const {
    assert(type() == Type::ValueType || type() == Type::List ||
           type() == Type::Option || type() == Type::Stream ||
           type() == Type::Future);
    return reinterpret_cast<const ComponentValueType*>(this);
  }

  const ComponentTypeIndex* AsTypeIndex() const {
    assert(type() == Type::Own || type() == Type::Borrow ||
           type() == Type::Resource);
    return reinterpret_cast<const ComponentTypeIndex*>(this);
  }

  bool IsTypeItems() const {
    return section() == Section::Type &&
           (type() == Type::Record || type() == Type::Variant);
  }

  ComponentTypeItems* AsTypeItems() {
    assert(IsTypeItems());
    return reinterpret_cast<ComponentTypeItems*>(this);
  }

  const ComponentTypeItems* AsTypeItems() const {
    assert(IsTypeItems());
    return reinterpret_cast<const ComponentTypeItems*>(this);
  }

  const ComponentTypeListFixed* AsTypeListFixed() const {
    assert(type() == Type::ListFixed);
    return reinterpret_cast<const ComponentTypeListFixed*>(this);
  }

  bool IsTypeTuple() const {
    return section() == Section::Type && type() == Type::Tuple;
  }

  ComponentTypeTuple* AsTypeTuple() {
    assert(IsTypeTuple());
    return reinterpret_cast<ComponentTypeTuple*>(this);
  }

  const ComponentTypeTuple* AsTypeTuple() const {
    assert(IsTypeTuple());
    return reinterpret_cast<const ComponentTypeTuple*>(this);
  }

  bool IsTypeLabels() const {
    return section() == Section::Type &&
           (type() == Type::Flags || type() == Type::Enum);
  }

  ComponentTypeLabels* AsTypeLabels() {
    assert(IsTypeLabels());
    return reinterpret_cast<ComponentTypeLabels*>(this);
  }

  const ComponentTypeLabels* AsTypeLabels() const {
    assert(IsTypeLabels());
    return reinterpret_cast<const ComponentTypeLabels*>(this);
  }

  const ComponentTypeResult* AsTypeResult() const {
    assert(type() == Type::Result);
    return reinterpret_cast<const ComponentTypeResult*>(this);
  }

  bool IsTypeFunc() const {
    return section() == Section::Type &&
           (type() == Type::Func || type() == Type::AsyncFunc);
  }

  ComponentTypeFunc* AsTypeFunc() {
    assert(IsTypeFunc());
    return reinterpret_cast<ComponentTypeFunc*>(this);
  }

  const ComponentTypeFunc* AsTypeFunc() const {
    assert(IsTypeFunc());
    return reinterpret_cast<const ComponentTypeFunc*>(this);
  }

  const ComponentTypeResource* AsTypeResource() const {
    assert(type() == Type::Resource || type() == Type::ResourceAsync);
    return reinterpret_cast<const ComponentTypeResource*>(this);
  }

  const ComponentInterfaceType* AsInterfaceType() const {
    assert(type() == Type::Instance || type() == Type::Component);
    return reinterpret_cast<const ComponentInterfaceType*>(this);
  }

  const ComponentCanonLift* AsCanonLift() const {
    assert(canon() == ComponentCanon::Lift);
    return reinterpret_cast<const ComponentCanonLift*>(this);
  }

  const ComponentCanonLower* AsCanonLower() const {
    assert(canon() == ComponentCanon::Lower);
    return reinterpret_cast<const ComponentCanonLower*>(this);
  }

  bool IsCanonType() const {
    return section() == Section::Canon &&
           (canon() == ComponentCanon::ResourceNew ||
            canon() == ComponentCanon::ResourceDrop ||
            canon() == ComponentCanon::ResourceRep);
  }

  const ComponentCanonType* AsCanonType() const {
    assert(IsCanonType());
    return reinterpret_cast<const ComponentCanonType*>(this);
  }

  const ComponentExternal* AsExternal() const {
    assert(section() == Section::Import || section() == Section::Export);
    return reinterpret_cast<const ComponentExternal*>(this);
  }

  const ComponentCoreModule* AsCoreModule() const {
    assert(section() == Section::CoreModule);
    return reinterpret_cast<const ComponentCoreModule*>(this);
  }

  bool IsInstance() const {
    return section() == Section::CoreInstance ||
           (section() == Section::Instance &&
            instance() == Instance::Reference);
  }

  ComponentInstance* AsInstance() {
    assert(IsInstance());
    return reinterpret_cast<ComponentInstance*>(this);
  }

  const ComponentInstance* AsInstance() const {
    assert(IsInstance());
    return reinterpret_cast<const ComponentInstance*>(this);
  }

  bool IsInlineInstance() const {
    return section() == Section::Instance && instance() == Instance::Inline;
  }

  ComponentInlineInstance* AsInlineInstance() {
    assert(IsInlineInstance());
    return reinterpret_cast<ComponentInlineInstance*>(this);
  }

  const ComponentInlineInstance* AsInlineInstance() const {
    assert(IsInlineInstance());
    return reinterpret_cast<const ComponentInlineInstance*>(this);
  }

 protected:
  ComponentDef(Section section, ComponentSort sort)
      : section_(section), sort_(sort), sub_type_(0) {
    assert(section != Section::Type);
  }

  ComponentDef(Section section, Instance instance)
      : section_(section),
        sort_(section == Section::Instance ? ComponentSort::Instance
                                           : ComponentSort::CoreInstance),
        sub_type_(static_cast<uint8_t>(instance)) {
    assert(section == Section::CoreInstance || section == Section::Instance);
  }

  ComponentDef(Alias alias, ComponentSort sort)
      : section_(Section::Alias), sort_(sort),
        sub_type_(static_cast<uint8_t>(alias)) {}

  ComponentDef(Type type)
      : section_(Section::Type), sort_(ComponentSort::Type),
        sub_type_(static_cast<uint8_t>(type)) {}

  ComponentDef(ComponentCanon canon)
      : section_(Section::Canon),
        sort_(canon == ComponentCanon::Lift ? ComponentSort::Func
                                            : ComponentSort::CoreFunc),
        sub_type_(static_cast<uint8_t>(canon)) {}

  ComponentDef(Section section,
               ExternalDescriptor external,
               ComponentSort sort)
      : section_(section), sort_(sort),
        sub_type_(static_cast<uint8_t>(external)) {
    assert(section == Section::Import || section == Section::Export);
  }

 private:
  Section section_;
  ComponentSort sort_;
  // Secondary, optional type field.
  uint8_t sub_type_;
  const std::string* name_ = nullptr;
};

class ComponentDefList : public ComponentDef {
 public:
  ComponentDefList* GetParent() { return parent_; }
  const ComponentDefList* GetParent() const { return parent_; }

  bool IsComponent() const {
    return section() == Section::Component;
  }

  bool IsInstanceType() const {
    return section() == Section::Type && type() == Type::Instance;
  }

  bool IsComponentType() const {
    return section() == Section::Type && type() == Type::Component;
  }

  uint32_t Size() const {
    return static_cast<uint32_t>(list_.size());
  }

  const ComponentDef* Get(size_t idx) const {
    return list_[idx].get();
  }

  void Append(std::unique_ptr<ComponentDef> type) {
    assert(type->sort() != ComponentSort::Type);
    list_.push_back(std::move(type));
  }

  void AppendType(std::unique_ptr<ComponentDef> type) {
    assert(type->sort() == ComponentSort::Type);
    type_list_.push_back(type.get());
    list_.push_back(std::move(type));
  }

  void AppendAny(std::unique_ptr<ComponentDef> type) {
    if (type->sort() == ComponentSort::Type) {
      type_list_.push_back(type.get());
    }
    list_.push_back(std::move(type));
  }

  void SetLastTypeName(const std::string* name) {
    assert(type_list_.size() > 0 && list_.back().get() == type_list_.back());
    list_.back().get()->SetName(name);
  }

  Index TypeSize() const {
    return static_cast<Index>(type_list_.size());
  }

  const ComponentDef* Find(ComponentSort sort,
                           const std::string* name,
                           Index* out_index = nullptr) const;
  const ComponentDef* Find(ComponentSort sort, Index index) const;
  Index SortSize(ComponentSort sort) const;

 protected:
  ComponentDefList(ComponentDefList* parent)
      : ComponentDef(Section::Component, ComponentSort::Component),
        parent_(parent) {}

  ComponentDefList(ComponentDefList* parent, Type type)
      : ComponentDef(type), parent_(parent) {
    assert(IsInstanceType() || IsComponentType());
  }

 private:
  ComponentDefList* parent_;
  std::vector<std::unique_ptr<ComponentDef>> list_;
  // Types and core types are frequently used.
  std::vector<const ComponentDef*> type_list_;
};

class ComponentAliasExport : public ComponentDef {
 public:
  ComponentAliasExport(bool is_core,
                       const ComponentIndexLoc& instance_index,
                       const StringLoc& export_name,
                       ComponentSort sort)
      : ComponentDef(is_core ? Alias::CoreExport : Alias::Export, sort),
        instance_index_(instance_index), export_name_(export_name) {}

  Index InstanceIndex() const { return instance_index_.index; }
  const ComponentIndexLoc& InstanceIndexLoc() const { return instance_index_; }
  const std::string* ExportName() const { return export_name_.str; }
  const StringLoc& ExportNameLoc() const { return export_name_; }

 private:
  ComponentIndexLoc instance_index_;
  StringLoc export_name_;
};

class ComponentAliasOuter : public ComponentDef {
 public:
  ComponentAliasOuter(Index counter,
                      Index index,
                      ComponentSort sort,
                      Location loc)
      : ComponentDef(Alias::Outer, sort), counter_(counter),
        index_(index), loc_(loc) {
    assert(sort == ComponentSort::CoreModule ||
           sort == ComponentSort::Component ||
           sort == ComponentSort::Type);
  }

  Index GetCounter() const { return counter_; }
  Index GetIndex() const { return index_; }
  const Location& GetLocation() const { return loc_; }

 private:
  Index counter_;
  Index index_;
  Location loc_;
};

class ComponentValueType : public ComponentDef {
 public:
  ComponentValueType(Type type, const ComponentTypeLoc& value_type)
      : ComponentDef(type), value_type_(value_type) {
    assert(type == Type::Stream || type == Type::Future ||
           ((type == Type::ValueType || type == Type::List ||
             type == Type::Option) &&
            !value_type.type.IsNone()));
  }

  const ComponentType& ValueType() const { return value_type_.type; }
  const ComponentTypeLoc& ValueTypeLoc() const { return value_type_; }

 private:
  ComponentTypeLoc value_type_;
};

class ComponentTypeIndex : public ComponentDef {
 public:
  ComponentTypeIndex(Type type, const ComponentIndexLoc& index)
      : ComponentDef(type), index_(index) {
    assert(type == Type::Resource ||
           ((type == Type::Own || type == Type::Borrow) &&
            index.index != kInvalidIndex));
  }

  Index GetIndex() const { return index_.index; }
  const ComponentIndexLoc& GetIndexLoc() const { return index_; }

 private:
  ComponentIndexLoc index_;
};

class ComponentTypeItems : public ComponentDef {
 public:
  struct Item {
    StringLoc name;
    ComponentTypeLoc type;
  };

  using ItemVector = std::vector<Item>;

  ComponentTypeItems(Type type, Location loc)
      : ComponentDef(type), loc_(loc) {
    assert(IsTypeItems());
  }

  const ItemVector& Items() const { return items_; }
  const Location& GetLocation() const { return loc_; }

  void Append(const StringLoc& name, const ComponentTypeLoc& type) {
    items_.push_back(Item{name, type});
  }

 private:
  ItemVector items_;
  Location loc_;
};

class ComponentTypeListFixed : public ComponentDef {
 public:
  ComponentTypeListFixed(const ComponentTypeLoc& value_type,
                         uint32_t size,
                         Location loc)
      : ComponentDef(Type::ListFixed), value_type_(value_type), size_(size),
        loc_(loc) {
    assert(!value_type.type.IsNone());
  }

  const ComponentType& ValueType() const { return value_type_.type; }
  const ComponentTypeLoc& ValueTypeLoc() const { return value_type_; }
  uint32_t Size() const { return size_; }
  const Location& GetLocation() const { return loc_; }

 private:
  ComponentTypeLoc value_type_;
  uint32_t size_;
  Location loc_;
};

class ComponentTypeTuple : public ComponentDef {
 public:
  using ItemVector = std::vector<ComponentTypeLoc>;

  ComponentTypeTuple(Location loc)
      : ComponentDef(Type::Tuple), loc_(loc) {
  }

  const ItemVector& Items() const { return items_; }
  const Location& GetLocation() const { return loc_; }

  void Append(const ComponentTypeLoc& type) {
    items_.push_back(type);
  }

 private:
  ItemVector items_;
  Location loc_;
};

class ComponentTypeLabels : public ComponentDef {
 public:
  using ItemVector = std::vector<StringLoc>;

  ComponentTypeLabels(Type type, Location loc)
      : ComponentDef(type), loc_(loc) {
    assert(IsTypeLabels());
  }

  const ItemVector& Labels() const { return items_; }
  const Location& GetLocation() const { return loc_; }

  void Append(const StringLoc& label) {
    items_.push_back(label);
  }

 private:
  ItemVector items_;
  Location loc_;
};

class ComponentTypeResult : public ComponentDef {
 public:
  ComponentTypeResult(const ComponentTypeLoc& result,
                      const ComponentTypeLoc& error)
      : ComponentDef(Type::Result), result_(result), error_(error) {}

  const ComponentType& Result() const { return result_.type; }
  const ComponentTypeLoc& ResultLoc() const { return result_; }
  const ComponentType& Error() const { return error_.type; }
  const ComponentTypeLoc& ErrorLoc() const { return error_; }

 private:
  ComponentTypeLoc result_;
  ComponentTypeLoc error_;
};

class ComponentTypeFunc : public ComponentDef {
 public:
  struct Param {
    StringLoc name;
    ComponentTypeLoc type;
  };

  using ParamVector = std::vector<Param>;

  ComponentTypeFunc(ComponentTypeDef type)
      : ComponentDef(type) {
    assert(IsTypeFunc());
  }

  const ParamVector& Params() const { return params_; }
  const ComponentType& Result() const { return result_.type; }
  const ComponentTypeLoc& ResultLoc() const { return result_; }

  void AppendParam(const StringLoc& name, const ComponentTypeLoc& type) {
    params_.push_back(Param{name, type});
  }
  void SetResult(const ComponentTypeLoc& result_type) {
    result_ = result_type;
  }

 private:
  ParamVector params_;
  ComponentTypeLoc result_;
};

class ComponentTypeResource : public ComponentDef {
 public:
  ComponentTypeResource(ComponentResourceRep rep,
                        const ComponentIndexLoc& dtor,
                        const Location& loc)
      : ComponentDef(Type::Resource), rep_(rep), dtor_(dtor), loc_(loc) {}

  ComponentTypeResource(ComponentResourceRep rep,
                        const ComponentIndexLoc& dtor,
                        const ComponentIndexLoc& callback,
                        const Location& loc)
      : ComponentDef(Type::ResourceAsync), rep_(rep), dtor_(dtor),
        callback_(callback), loc_(loc) {
    assert(dtor.index != kInvalidIndex);
  }

  ComponentResourceRep Rep() const { return rep_; }
  Index Dtor() const { return dtor_.index; }
  const ComponentIndexLoc& DtorLoc() const { return dtor_; }
  Index Callback() const { return callback_.index; }
  const ComponentIndexLoc& CallbackLoc() const { return callback_; }
  const Location& GetLocation() const { return loc_; }

 private:
  ComponentResourceRep rep_;
  ComponentIndexLoc dtor_;
  ComponentIndexLoc callback_;
  Location loc_;
};

class ComponentInterfaceType : public ComponentDefList {
 public:
  ComponentInterfaceType(ComponentDefList* parent, Type type)
      : ComponentDefList(parent, type) {
    assert(type == Type::Instance || type == Type::Component);
  }

  ComponentInterfaceType(ComponentDefList* parent, bool is_instance)
      : ComponentDefList(parent,
                         is_instance ? Type::Instance : Type::Component) {}
};

class ComponentCanonOpts : public ComponentDef {
 public:
  using OptionVector = std::vector<ComponentCanonOption>;

  uint32_t OptionsSize() const {
    return static_cast<uint32_t>(options_.size());
  }
  const OptionVector& Options() const { return options_; }

 protected:
  ComponentCanonOpts(ComponentCanon canon, OptionVector* options)
      : ComponentDef(canon) {
    options_ = std::move(*options);
  }

 private:
  OptionVector options_;
};

class ComponentCanonLift : public ComponentCanonOpts {
 public:
  ComponentCanonLift(const ComponentIndexLoc& core_func_index,
                     OptionVector* options,
                     const ComponentIndexLoc& type_index)
      : ComponentCanonOpts(ComponentCanon::Lift, options),
        core_func_index_(core_func_index), type_index_(type_index) {}

  Index CoreFuncIndex() const { return core_func_index_.index; }
  const ComponentIndexLoc& CoreFuncIndexLoc() const { return core_func_index_; }
  Index TypeIndex() const { return type_index_.index; }
  const ComponentIndexLoc& TypeIndexLoc() const { return type_index_; }

 private:
  ComponentIndexLoc core_func_index_;
  ComponentIndexLoc type_index_;
};

class ComponentCanonLower : public ComponentCanonOpts {
 public:
  ComponentCanonLower(const ComponentIndexLoc& func_index,
                      OptionVector* options)
      : ComponentCanonOpts(ComponentCanon::Lower, options),
        func_index_(func_index) {}

  Index FuncIndex() const { return func_index_.index; }
  const ComponentIndexLoc& FuncIndexLoc() const { return func_index_; }

 private:
  ComponentIndexLoc func_index_;
};

class ComponentCanonType : public ComponentDef {
 public:
  ComponentCanonType(ComponentCanon canon, const ComponentIndexLoc& type_index)
      : ComponentDef(canon), type_index_(type_index) {
    assert(IsCanonType());
  }

  Index TypeIndex() const { return type_index_.index; }
  const ComponentIndexLoc& TypeIndexLoc() const { return type_index_; }

 private:
  ComponentIndexLoc type_index_;
};

class ComponentExternal : public ComponentDef {
 public:
  ComponentExternal(bool is_import,
                    StringLoc& external_name,
                    const std::string* version_suffix,
                    ExternalDescriptor external,
                    ComponentSort sort,
                    const ComponentIndexLoc& type_index,
                    ComponentSort export_sort,
                    const ComponentIndexLoc& export_index)
      : ComponentDef(is_import ? Section::Import : Section::Export, external,
                     sort),
        external_name_(external_name), version_suffix_(version_suffix),
        type_index_(type_index), export_sort_(export_sort),
        export_index_(export_index) {
    assert(external_name.str != nullptr &&
           (export_index.index == kInvalidIndex || !is_import));
  }

  const std::string* ExternalName() const { return external_name_.str; }
  const StringLoc& ExternalNameLoc() const { return external_name_; }
  // Optional suffix (can be nullptr).
  const std::string* VersionSuffix() const { return version_suffix_; }
  // Optional for component (not component type) exports.
  Index TypeIndex() const { return type_index_.index; }
  const ComponentIndexLoc& TypeIndexLoc() const { return type_index_; }
  // Only used by component (not component type) exports.
  ComponentSort ExportSort() const { return export_sort_; }
  Index ExportIndex() const { return export_index_.index; }
  const ComponentIndexLoc& ExportIndexLoc() const { return export_index_; }

 private:
  StringLoc external_name_;
  const std::string* version_suffix_;
  ComponentIndexLoc type_index_;
  ComponentSort export_sort_;
  ComponentIndexLoc export_index_;
};

class ComponentCoreModule : public ComponentDef {
 public:
  ComponentCoreModule()
      : ComponentDef(Section::CoreModule, ComponentSort::CoreModule) {}

  const Module* module() const {
    return &module_;
  }

  Module* module() {
    return &module_;
  }

 private:
  Module module_;
};

class ComponentInstance : public ComponentDef {
 public:
  struct Argument {
    StringLoc name;
    ComponentSort sort;
    ComponentIndexLoc index;
  };

  using ArgumentVector = std::vector<Argument>;

  ComponentInstance(bool is_core,
                    const ComponentIndexLoc& from_index,
                    uint32_t argument_count)
      : ComponentDef(is_core ? Section::CoreInstance : Section::Instance,
                     Instance::Reference),
        from_index_(from_index) {
    arguments_.reserve(argument_count);
  }

  ComponentInstance(uint32_t argument_count)
      : ComponentDef(Section::CoreInstance, Instance::Inline),
        from_index_(ComponentIndexLoc()) {
    arguments_.reserve(argument_count);
  }

  Index FromIndex() const { return from_index_.index; }
  const ComponentIndexLoc& FromIndexLoc() const { return from_index_; }
  const ArgumentVector& Arguments() const { return arguments_; }

  void Append(const StringLoc& name,
              ComponentSort sort,
              const ComponentIndexLoc& index) {
    arguments_.push_back(Argument{name, sort, index});
  }

 private:
  ArgumentVector arguments_;
  ComponentIndexLoc from_index_;
};

class ComponentInlineInstance : public ComponentDef {
 public:
  struct Argument {
    StringLoc name;
    // Optional suffix (can be nullptr)
    const std::string* version_suffix;
    ComponentSort sort;
    ComponentIndexLoc index;
  };

  using ArgumentVector = std::vector<Argument>;

  ComponentInlineInstance(uint32_t argument_count)
      : ComponentDef(Section::Instance, Instance::Inline) {
    arguments_.reserve(argument_count);
  }

  const ArgumentVector& Arguments() const { return arguments_; }

  void Append(const StringLoc& name,
              const std::string* version_suffix,
              ComponentSort sort,
              const ComponentIndexLoc& index) {
    arguments_.push_back(Argument{name, version_suffix, sort, index});
  }

 private:
  ArgumentVector arguments_;
};

class ComponentData : public ComponentDefList {
 public:
  ComponentData(ComponentData* parent)
      : ComponentDefList(parent) {}

  const ComponentData* GetParentComponent() const {
    return GetParent()->AsComponent();
  }
};

class Component : public ComponentData {
 public:
  // Helper class used by parsers.
  class StringTable {
   public:
    StringTable(Component* owner)
        : string_map_(Less), string_table_(&owner->string_table_) {
      assert(string_table_->empty());
    }

    StringTable(std::vector<std::unique_ptr<std::string>>* string_table_)
        : string_map_(Less), string_table_(string_table_) {
      assert(string_table_->empty());
    }

    const std::string* Find(const std::string_view& name) const;
    const std::string* Append(const std::string_view& name);

    StringLoc Append(const ComponentStringLoc& name) {
      return StringLoc{Append(name.str), name.loc};
    }

   private:
    static bool Less(std::string* first, std::string* second) {
      assert(first != nullptr && second != nullptr);
      return *first < *second;
    }

    std::set<std::string*, decltype(Less)*> string_map_;
    std::vector<std::unique_ptr<std::string>>* string_table_;
  };

  Component(std::string_view filename)
      : ComponentData(nullptr), filename(filename) {}

  std::string_view Filename() const { return filename; }

 private:
  std::string_view filename;
  // Only these strings are used by the component
  std::vector<std::unique_ptr<std::string>> string_table_;
};

void MakeTypeBindingReverseMapping(
    size_t num_types,
    const BindingHash& bindings,
    std::vector<std::string>* out_reverse_mapping);

}  // namespace wabt

#endif /* WABT_IR_H_ */
