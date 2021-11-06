/*
 * Copyright 2020 WebAssembly Community Group participants
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

#ifndef WABT_INTERP_H_
#define WABT_INTERP_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "src/cast.h"
#include "src/common.h"
#include "src/feature.h"
#include "src/opcode.h"
#include "src/result.h"
#include "src/string-view.h"

#include "src/interp/istream.h"

namespace wabt {
namespace interp {

class Store;
class Object;
class Trap;
class DataSegment;
class ElemSegment;
class Module;
class Instance;
class Thread;
template <typename T> class RefPtr;

using s8 = int8_t;
using u8 = uint8_t;
using s16 = int16_t;
using u16 = uint16_t;
using s32 = int32_t;
using u32 = uint32_t;
using Index = uint32_t;
using s64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using Buffer = std::vector<u8>;

using ValueType = wabt::Type;
using ValueTypes = std::vector<ValueType>;

template <typename T> bool HasType(ValueType);
template <typename T> void RequireType(ValueType);
bool IsReference(ValueType);
bool TypesMatch(ValueType expected, ValueType actual);

using ExternKind = ExternalKind;
enum class Mutability { Const, Var };
enum class TagAttr { Exception };
using SegmentMode = SegmentKind;
enum class ElemKind { RefNull, RefFunc };

enum class ObjectKind {
  Null,
  Foreign,
  Trap,
  DefinedFunc,
  HostFunc,
  Table,
  Memory,
  Global,
  Tag,
  Module,
  Instance,
  Thread,
};

const char* GetName(Mutability);
const char* GetName(ValueType);
const char* GetName(ExternKind);
const char* GetName(ObjectKind);

enum class InitExprKind {
  None,
  I32,
  I64,
  F32,
  F64,
  V128,
  GlobalGet,
  RefNull,
  RefFunc
};

struct InitExpr {
  InitExprKind kind;
  union {
    u32 i32_;
    u64 i64_;
    f32 f32_;
    f64 f64_;
    v128 v128_;
    Index index_;
    Type type_;
  };
};

struct Ref {
  static const Ref Null;

  Ref() = default;
  explicit Ref(size_t index);

  friend bool operator==(Ref, Ref);
  friend bool operator!=(Ref, Ref);

  size_t index;
};
using RefVec = std::vector<Ref>;

template <typename T, u8 L>
struct Simd {
  using LaneType = T;
  static const u8 lanes = L;

  T v[L];

  inline T& operator[](u8 idx) {
#if WABT_BIG_ENDIAN
    idx = (~idx) & (L-1);
#endif
    return v[idx];
  }
  inline T operator[](u8 idx) const {
#if WABT_BIG_ENDIAN
    idx = (~idx) & (L-1);
#endif
    return v[idx];
  }
};
using s8x16 = Simd<s8, 16>;
using u8x16 = Simd<u8, 16>;
using s16x8 = Simd<s16, 8>;
using u16x8 = Simd<u16, 8>;
using s32x4 = Simd<s32, 4>;
using u32x4 = Simd<u32, 4>;
using s64x2 = Simd<s64, 2>;
using u64x2 = Simd<u64, 2>;
using f32x4 = Simd<f32, 4>;
using f64x2 = Simd<f64, 2>;

// Used for load extend instructions.
using s8x8 = Simd<s8, 8>;
using u8x8 = Simd<u8, 8>;
using s16x4 = Simd<s16, 4>;
using u16x4 = Simd<u16, 4>;
using s32x2 = Simd<s32, 2>;
using u32x2 = Simd<u32, 2>;

//// Types ////

bool CanGrow(const Limits&, u32 old_size, u32 delta, u32* new_size);
Result Match(const Limits& expected,
             const Limits& actual,
             std::string* out_msg);

struct ExternType {
  explicit ExternType(ExternKind);
  virtual ~ExternType() {}
  virtual std::unique_ptr<ExternType> Clone() const = 0;

  ExternKind kind;
};

struct FuncType : ExternType {
  static const ExternKind skind = ExternKind::Func;
  static bool classof(const ExternType* type);

  explicit FuncType(ValueTypes params, ValueTypes results);

  std::unique_ptr<ExternType> Clone() const override;

  friend Result Match(const FuncType& expected,
                      const FuncType& actual,
                      std::string* out_msg);

  ValueTypes params;
  ValueTypes results;
};

struct TableType : ExternType {
  static const ExternKind skind = ExternKind::Table;
  static bool classof(const ExternType* type);

  explicit TableType(ValueType, Limits);

  std::unique_ptr<ExternType> Clone() const override;

  friend Result Match(const TableType& expected,
                      const TableType& actual,
                      std::string* out_msg);

  ValueType element;
  Limits limits;
};

struct MemoryType : ExternType {
  static const ExternKind skind = ExternKind::Memory;
  static bool classof(const ExternType* type);

  explicit MemoryType(Limits);

  std::unique_ptr<ExternType> Clone() const override;

  friend Result Match(const MemoryType& expected,
                      const MemoryType& actual,
                      std::string* out_msg);

  Limits limits;
};

struct GlobalType : ExternType {
  static const ExternKind skind = ExternKind::Global;
  static bool classof(const ExternType* type);

  explicit GlobalType(ValueType, Mutability);

  std::unique_ptr<ExternType> Clone() const override;

  friend Result Match(const GlobalType& expected,
                      const GlobalType& actual,
                      std::string* out_msg);

  ValueType type;
  Mutability mut;
};

struct TagType : ExternType {
  static const ExternKind skind = ExternKind::Tag;
  static bool classof(const ExternType* type);

  explicit TagType(TagAttr, const ValueTypes&);

  std::unique_ptr<ExternType> Clone() const override;

  friend Result Match(const TagType& expected,
                      const TagType& actual,
                      std::string* out_msg);

  TagAttr attr;
  ValueTypes signature;
};

struct ImportType {
  explicit ImportType(std::string module,
                      std::string name,
                      std::unique_ptr<ExternType>);
  ImportType(const ImportType&);
  ImportType& operator=(const ImportType&);

  std::string module;
  std::string name;
  std::unique_ptr<ExternType> type;
};

struct ExportType {
  explicit ExportType(std::string name, std::unique_ptr<ExternType>);
  ExportType(const ExportType&);
  ExportType& operator=(const ExportType&);

  std::string name;
  std::unique_ptr<ExternType> type;
};

//// Structure ////

struct ImportDesc {
  ImportType type;
};

struct LocalDesc {
  ValueType type;
  u32 count;
  // One past the last local index that has this type. For example, a vector of
  // LocalDesc might look like:
  //
  //   {{I32, 2, 2}, {I64, 3, 5}, {F32, 1, 6}, ...}
  //
  // This makes it possible to use a binary search to find the type of a local
  // at a given index.
  u32 end;
};

struct FuncDesc {
  // Includes params.
  ValueType GetLocalType(Index) const;

  FuncType type;
  std::vector<LocalDesc> locals;
  u32 code_offset;
};

struct TableDesc {
  TableType type;
};

struct MemoryDesc {
  MemoryType type;
};

struct GlobalDesc {
  GlobalType type;
  InitExpr init;
};

struct TagDesc {
  TagType type;
};

struct ExportDesc {
  ExportType type;
  Index index;
};

struct StartDesc {
  Index func_index;
};

struct DataDesc {
  Buffer data;
  SegmentMode mode;
  Index memory_index;
  InitExpr offset;
};

struct ElemExpr {
  ElemKind kind;
  Index index;
};

struct ElemDesc {
  std::vector<ElemExpr> elements;
  ValueType type;
  SegmentMode mode;
  Index table_index;
  InitExpr offset;
};

struct ModuleDesc {
  std::vector<FuncType> func_types;
  std::vector<ImportDesc> imports;
  std::vector<FuncDesc> funcs;
  std::vector<TableDesc> tables;
  std::vector<MemoryDesc> memories;
  std::vector<GlobalDesc> globals;
  std::vector<TagDesc> tags;
  std::vector<ExportDesc> exports;
  std::vector<StartDesc> starts;
  std::vector<ElemDesc> elems;
  std::vector<DataDesc> datas;
  Istream istream;
};

//// Runtime ////

struct Frame {
  explicit Frame(Ref func, u32 values, u32 offset, Instance*, Module*);

  void Mark(Store&);

  Ref func;
  u32 values;  // Height of the value stack at this activation.
  u32 offset;  // Istream offset; either the return PC, or the current PC.

  // Cached for convenience. Both are null if func is a HostFunc.
  Instance* inst;
  Module* mod;
};

template <typename T>
class FreeList {
 public:
  using Index = size_t;

  template <typename... Args>
  Index New(Args&&...);
  void Delete(Index);

  bool IsUsed(Index) const;

  const T& Get(Index) const;
  T& Get(Index);

  Index size() const;  // 1 greater than the maximum index.
  Index count() const; // The number of used elements.

 private:
  // TODO: Optimize memory layout? We could probably store all of this
  // information in one uintptr_t.
  //
  // For example, when T is a pointer to an Object (e.g. Store::ObjectList), we
  // can assume alignment to 4 bytes at least. Given a 32-bit pointer, we can
  // expect the following layout:
  //
  //   nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnn0 f
  //
  // where:
  //   f: "is_free": 0 when the payload is of type T
  //                 1 when the payload is the index of the next free object
  //   n: the payload
  //
  // When T is a Ref (see Store::RootList below), we'd need to store the
  // "is_free" bit in most-significant bit instead.
  //
  std::vector<T> list_;
  std::vector<size_t> free_;
  std::vector<bool> is_free_;
};

class Store {
 public:
  using ObjectList = FreeList<std::unique_ptr<Object>>;
  using RootList = FreeList<Ref>;

  explicit Store(const Features& = Features{});

  bool IsValid(Ref) const;
  bool HasValueType(Ref, ValueType) const;
  template <typename T>
  bool Is(Ref) const;

  template <typename T, typename... Args>
  RefPtr<T> Alloc(Args&&...);
  template <typename T>
  Result Get(Ref, RefPtr<T>* out);
  template <typename T>
  RefPtr<T> UnsafeGet(Ref);

  RootList::Index NewRoot(Ref);
  RootList::Index CopyRoot(RootList::Index);
  void DeleteRoot(RootList::Index);

  void Collect();
  void Mark(Ref);
  void Mark(const RefVec&);

  ObjectList::Index object_count() const;

  const Features& features() const;
  void setFeatures(const Features& features) { features_ = features; }

 private:
  template <typename T>
  friend class RefPtr;

  Features features_;
  ObjectList objects_;
  RootList roots_;
  std::vector<bool> marks_;
};

template <typename T>
class RefPtr {
 public:
  RefPtr();
  RefPtr(Store&, Ref);
  RefPtr(const RefPtr&);
  RefPtr& operator=(const RefPtr&);
  RefPtr(RefPtr&&);
  RefPtr& operator=(RefPtr&&);
  ~RefPtr();

  template <typename U>
  RefPtr(const RefPtr<U>&);
  template <typename U>
  RefPtr& operator=(const RefPtr<U>&);
  template <typename U>
  RefPtr(RefPtr&&);
  template <typename U>
  RefPtr& operator=(RefPtr&&);

  template <typename U>
  RefPtr<U> As();

  bool empty() const;
  void reset();

  T* get() const;
  T* operator->() const;
  T& operator*() const;
  explicit operator bool() const;

  Ref ref() const;
  Store* store() const;

  template <typename U, typename V>
  friend bool operator==(const RefPtr<U>& lhs, const RefPtr<V>& rhs);
  template <typename U, typename V>
  friend bool operator!=(const RefPtr<U>& lhs, const RefPtr<V>& rhs);

 private:
  template <typename U>
  friend class RefPtr;

  T* obj_;
  Store* store_;
  Store::RootList::Index root_index_;
};

struct Value {
  static Value WABT_VECTORCALL Make(s32);
  static Value WABT_VECTORCALL Make(u32);
  static Value WABT_VECTORCALL Make(s64);
  static Value WABT_VECTORCALL Make(u64);
  static Value WABT_VECTORCALL Make(f32);
  static Value WABT_VECTORCALL Make(f64);
  static Value WABT_VECTORCALL Make(v128);
  static Value WABT_VECTORCALL Make(Ref);
  template <typename T, u8 L>
  static Value WABT_VECTORCALL Make(Simd<T, L>);

  template <typename T>
  T WABT_VECTORCALL Get() const;
  template <typename T>
  void WABT_VECTORCALL Set(T);

 private:
  union {
    u32 i32_;
    u64 i64_;
    f32 f32_;
    f64 f64_;
    v128 v128_;
    Ref ref_;
  };

 public:
#ifndef NDEBUG
  Value() : v128_(0, 0, 0, 0), type(ValueType::Any) {}
  void SetType(ValueType t) { type = t; }
  void CheckType(ValueType t) const {
    // Sadly we must allow Any here, since locals may be uninitialized.
    // Alternatively we could modify InterpAlloca to set the type.
    assert(t == type || type == ValueType::Any);
  }
  ValueType type;
#else
  Value() : v128_(0, 0, 0, 0) {}
  void SetType(ValueType) {}
  void CheckType(ValueType) const {}
#endif
};
using Values = std::vector<Value>;

struct TypedValue {
  ValueType type;
  Value value;
};
using TypedValues = std::vector<TypedValue>;

using Finalizer = std::function<void(Object*)>;

class Object {
 public:
  static bool classof(const Object* obj);
  static const char* GetTypeName() { return "Object"; }
  using Ptr = RefPtr<Object>;

  Object(const Object&) = delete;
  Object& operator=(const Object&) = delete;

  virtual ~Object();

  ObjectKind kind() const;
  Ref self() const;

  void* host_info() const;
  void set_host_info(void*);

  Finalizer get_finalizer() const;
  void set_finalizer(Finalizer);

 protected:
  friend Store;
  explicit Object(ObjectKind);
  virtual void Mark(Store&) {}

  ObjectKind kind_;
  Finalizer finalizer_ = nullptr;
  void* host_info_ = nullptr;
  Ref self_ = Ref::Null;
};

class Foreign : public Object {
 public:
  static const ObjectKind skind = ObjectKind::Foreign;
  static bool classof(const Object* obj);
  static const char* GetTypeName() { return "Foreign"; }
  using Ptr = RefPtr<Foreign>;

  static Foreign::Ptr New(Store&, void*);

  void* ptr();

 private:
  friend Store;
  explicit Foreign(Store&, void*);
  void Mark(Store&) override;

  void* ptr_;
};

class Trap : public Object {
 public:
  static const ObjectKind skind = ObjectKind::Trap;
  static bool classof(const Object* obj);
  using Ptr = RefPtr<Trap>;

  static Trap::Ptr New(Store&,
                       const std::string& msg,
                       const std::vector<Frame>& trace = std::vector<Frame>());

  std::string message() const;

 private:
  friend Store;
  explicit Trap(Store&,
                const std::string& msg,
                const std::vector<Frame>& trace = std::vector<Frame>());
  void Mark(Store&) override;

  std::string message_;
  std::vector<Frame> trace_;
};

class Extern : public Object {
 public:
  static bool classof(const Object* obj);
  static const char* GetTypeName() { return "Foreign"; }
  using Ptr = RefPtr<Extern>;

  virtual Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) = 0;
  virtual const ExternType& extern_type() = 0;

 protected:
  friend Store;
  explicit Extern(ObjectKind);

  template <typename T>
  Result MatchImpl(Store&,
                   const ImportType&,
                   const T& actual,
                   Trap::Ptr* out_trap);
};

class Func : public Extern {
 public:
  static bool classof(const Object* obj);
  using Ptr = RefPtr<Func>;

  Result Call(Thread& thread,
              const Values& params,
              Values& results,
              Trap::Ptr* out_trap);

  // Convenience function that creates new Thread.
  Result Call(Store&,
              const Values& params,
              Values& results,
              Trap::Ptr* out_trap,
              Stream* = nullptr);

  const ExternType& extern_type() override;
  const FuncType& type() const;

 protected:
  explicit Func(ObjectKind, FuncType);
  virtual Result DoCall(Thread& thread,
                        const Values& params,
                        Values& results,
                        Trap::Ptr* out_trap) = 0;

  FuncType type_;
};

class DefinedFunc : public Func {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::DefinedFunc;
  static const char* GetTypeName() { return "DefinedFunc"; }
  using Ptr = RefPtr<DefinedFunc>;

  static DefinedFunc::Ptr New(Store&, Ref instance, FuncDesc);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

  Ref instance() const;
  const FuncDesc& desc() const;

 protected:
  Result DoCall(Thread& thread,
                const Values& params,
                Values& results,
                Trap::Ptr* out_trap) override;

 private:
  friend Store;
  explicit DefinedFunc(Store&, Ref instance, FuncDesc);
  void Mark(Store&) override;

  Ref instance_;
  FuncDesc desc_;
};

class HostFunc : public Func {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::HostFunc;
  static const char* GetTypeName() { return "HostFunc"; }
  using Ptr = RefPtr<HostFunc>;

  using Callback = std::function<Result(Thread& thread,
                                        const Values& params,
                                        Values& results,
                                        Trap::Ptr* out_trap)>;

  static HostFunc::Ptr New(Store&, FuncType, Callback);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

 protected:
  Result DoCall(Thread& thread,
                const Values& params,
                Values& results,
                Trap::Ptr* out_trap) override;

 private:
  friend Store;
  friend Thread;
  explicit HostFunc(Store&, FuncType, Callback);
  void Mark(Store&) override;

  Callback callback_;
};

class Table : public Extern {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Table;
  static const char* GetTypeName() { return "Table"; }
  using Ptr = RefPtr<Table>;

  static Table::Ptr New(Store&, TableType);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

  bool IsValidRange(u32 offset, u32 size) const;

  Result Get(u32 offset, Ref* out) const;
  Result Set(Store&, u32 offset, Ref);
  Result Grow(Store&, u32 count, Ref);
  Result Fill(Store&, u32 offset, Ref, u32 size);
  Result Init(Store&,
              u32 dst_offset,
              const ElemSegment&,
              u32 src_offset,
              u32 size);
  static Result Copy(Store&,
                     Table& dst,
                     u32 dst_offset,
                     const Table& src,
                     u32 src_offset,
                     u32 size);

  // Unsafe API.
  Ref UnsafeGet(u32 offset) const;

  const ExternType& extern_type() override;
  const TableType& type() const;
  const RefVec& elements() const;
  u32 size() const;

 private:
  friend Store;
  explicit Table(Store&, TableType);
  void Mark(Store&) override;

  TableType type_;
  RefVec elements_;
};

class Memory : public Extern {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Memory;
  static const char* GetTypeName() { return "Memory"; }
  using Ptr = RefPtr<Memory>;

  static Memory::Ptr New(Store&, MemoryType);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

  bool IsValidAccess(u64 offset, u64 addend, u64 size) const;
  bool IsValidAtomicAccess(u64 offset, u64 addend, u64 size) const;

  template <typename T>
  Result Load(u64 offset, u64 addend, T* out) const;
  template <typename T>
  Result WABT_VECTORCALL Store(u64 offset, u64 addend, T);
  Result Grow(u64 pages);
  Result Fill(u64 offset, u8 value, u64 size);
  Result Init(u64 dst_offset, const DataSegment&, u64 src_offset, u64 size);
  static Result Copy(Memory& dst,
                     u64 dst_offset,
                     const Memory& src,
                     u64 src_offset,
                     u64 size);

  // Fake atomics; just checks alignment.
  template <typename T>
  Result AtomicLoad(u64 offset, u64 addend, T* out) const;
  template <typename T>
  Result AtomicStore(u64 offset, u64 addend, T);
  template <typename T, typename F>
  Result AtomicRmw(u64 offset, u64 addend, T, F&& func, T* out);
  template <typename T>
  Result AtomicRmwCmpxchg(u64 offset, u64 addend, T expect, T replace, T* out);

  u64 ByteSize() const;
  u64 PageSize() const;

  // Unsafe API.
  template <typename T>
  T WABT_VECTORCALL UnsafeLoad(u64 offset, u64 addend) const;
  u8* UnsafeData();

  const ExternType& extern_type() override;
  const MemoryType& type() const;

 private:
  friend class Store;
  explicit Memory(class Store&, MemoryType);
  void Mark(class Store&) override;

  MemoryType type_;
  Buffer data_;
  u64 pages_;
};

class Global : public Extern {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Global;
  static const char* GetTypeName() { return "Global"; }
  using Ptr = RefPtr<Global>;

  static Global::Ptr New(Store&, GlobalType, Value);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

  Value Get() const;
  template <typename T>
  Result Get(T* out) const;
  template <typename T>
  Result WABT_VECTORCALL Set(T);
  Result Set(Store&, Ref);

  template <typename T>
  T WABT_VECTORCALL UnsafeGet() const;
  void UnsafeSet(Value);

  const ExternType& extern_type() override;
  const GlobalType& type() const;

 private:
  friend Store;
  explicit Global(Store&, GlobalType, Value);
  void Mark(Store&) override;

  GlobalType type_;
  Value value_;
};

class Tag : public Extern {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Tag;
  static const char* GetTypeName() { return "Tag"; }
  using Ptr = RefPtr<Tag>;

  static Tag::Ptr New(Store&, TagType);

  Result Match(Store&, const ImportType&, Trap::Ptr* out_trap) override;

  const ExternType& extern_type() override;
  const TagType& type() const;

 private:
  friend Store;
  explicit Tag(Store&, TagType);
  void Mark(Store&) override;

  TagType type_;
};

class ElemSegment {
 public:
  explicit ElemSegment(const ElemDesc*, RefPtr<Instance>&);

  bool IsValidRange(u32 offset, u32 size) const;
  void Drop();

  const ElemDesc& desc() const;
  const RefVec& elements() const;
  u32 size() const;

 private:
  friend Instance;
  void Mark(Store&);

  const ElemDesc* desc_;  // Borrowed from the Module.
  RefVec elements_;
};

class DataSegment {
 public:
  explicit DataSegment(const DataDesc*);

  bool IsValidRange(u64 offset, u64 size) const;
  void Drop();

  const DataDesc& desc() const;
  u64 size() const;

 private:
  const DataDesc* desc_;  // Borrowed from the Module.
  u64 size_;
};

class Module : public Object {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Module;
  static const char* GetTypeName() { return "Module"; }
  using Ptr = RefPtr<Module>;

  static Module::Ptr New(Store&, ModuleDesc);

  const ModuleDesc& desc() const;
  const std::vector<ImportType>& import_types() const;
  const std::vector<ExportType>& export_types() const;

 private:
  friend Store;
  friend Instance;
  explicit Module(Store&, ModuleDesc);
  void Mark(Store&) override;

  ModuleDesc desc_;
  std::vector<ImportType> import_types_;
  std::vector<ExportType> export_types_;
};

class Instance : public Object {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Instance;
  static const char* GetTypeName() { return "Instance"; }
  using Ptr = RefPtr<Instance>;

  static Instance::Ptr Instantiate(Store&,
                                   Ref module,
                                   const RefVec& imports,
                                   Trap::Ptr* out_trap);

  Ref module() const;
  const RefVec& imports() const;
  const RefVec& funcs() const;
  const RefVec& tables() const;
  const RefVec& memories() const;
  const RefVec& globals() const;
  const RefVec& tags() const;
  const RefVec& exports() const;
  const std::vector<ElemSegment>& elems() const;
  std::vector<ElemSegment>& elems();
  const std::vector<DataSegment>& datas() const;
  std::vector<DataSegment>& datas();

 private:
  friend Store;
  friend ElemSegment;
  friend DataSegment;
  explicit Instance(Store&, Ref module);
  void Mark(Store&) override;

  Value ResolveInitExpr(Store&, InitExpr);

  Ref module_;
  RefVec imports_;
  RefVec funcs_;
  RefVec tables_;
  RefVec memories_;
  RefVec globals_;
  RefVec tags_;
  RefVec exports_;
  std::vector<ElemSegment> elems_;
  std::vector<DataSegment> datas_;
};

enum class RunResult {
  Ok,
  Return,
  Trap,
};

// TODO: Kinda weird to have a thread as an object, but it makes reference
// marking simpler.
class Thread : public Object {
 public:
  static bool classof(const Object* obj);
  static const ObjectKind skind = ObjectKind::Thread;
  static const char* GetTypeName() { return "Thread"; }
  using Ptr = RefPtr<Thread>;

  struct Options {
    static const u32 kDefaultValueStackSize = 64 * 1024 / sizeof(Value);
    static const u32 kDefaultCallStackSize = 64 * 1024 / sizeof(Frame);

    u32 value_stack_size = kDefaultValueStackSize;
    u32 call_stack_size = kDefaultCallStackSize;
    Stream* trace_stream = nullptr;
  };

  static Thread::Ptr New(Store&, const Options&);

  RunResult Run(Trap::Ptr* out_trap);
  RunResult Run(int num_instructions, Trap::Ptr* out_trap);
  RunResult Step(Trap::Ptr* out_trap);

  Store& store();

  Instance* GetCallerInstance();

 private:
  friend Store;
  friend DefinedFunc;

  struct TraceSource;

  explicit Thread(Store&, const Options&);
  void Mark(Store&) override;

  RunResult PushCall(Ref func, u32 offset, Trap::Ptr* out_trap);
  RunResult PushCall(const DefinedFunc&, Trap::Ptr* out_trap);
  RunResult PushCall(const HostFunc&, Trap::Ptr* out_trap);
  RunResult PopCall();
  RunResult DoCall(const Func::Ptr&, Trap::Ptr* out_trap);
  RunResult DoReturnCall(const Func::Ptr&, Trap::Ptr* out_trap);

  void PushValues(const ValueTypes&, const Values&);
  void PopValues(const ValueTypes&, Values*);

  Value& Pick(Index);

  template <typename T>
  T WABT_VECTORCALL Pop();
  Value Pop();
  u64 PopPtr(const Memory::Ptr& memory);

  template <typename T>
  void WABT_VECTORCALL Push(T);
  void Push(Value);
  void Push(Ref);

  template <typename R, typename T>
  using UnopFunc = R WABT_VECTORCALL(T);
  template <typename R, typename T>
  using UnopTrapFunc = RunResult WABT_VECTORCALL(T, R*, std::string*);
  template <typename R, typename T>
  using BinopFunc = R WABT_VECTORCALL(T, T);
  template <typename R, typename T>
  using BinopTrapFunc = RunResult WABT_VECTORCALL(T, T, R*, std::string*);

  template <typename R, typename T>
  RunResult DoUnop(UnopFunc<R, T>);
  template <typename R, typename T>
  RunResult DoUnop(UnopTrapFunc<R, T>, Trap::Ptr* out_trap);
  template <typename R, typename T>
  RunResult DoBinop(BinopFunc<R, T>);
  template <typename R, typename T>
  RunResult DoBinop(BinopTrapFunc<R, T>, Trap::Ptr* out_trap);

  template <typename R, typename T>
  RunResult DoConvert(Trap::Ptr* out_trap);
  template <typename R, typename T>
  RunResult DoReinterpret();

  template <typename T>
  RunResult Load(Instr, T* out, Trap::Ptr* out_trap);
  template <typename T, typename V = T>
  RunResult DoLoad(Instr, Trap::Ptr* out_trap);
  template <typename T, typename V = T>
  RunResult DoStore(Instr, Trap::Ptr* out_trap);

  RunResult DoMemoryInit(Instr, Trap::Ptr* out_trap);
  RunResult DoDataDrop(Instr);
  RunResult DoMemoryCopy(Instr, Trap::Ptr* out_trap);
  RunResult DoMemoryFill(Instr, Trap::Ptr* out_trap);

  RunResult DoTableInit(Instr, Trap::Ptr* out_trap);
  RunResult DoElemDrop(Instr);
  RunResult DoTableCopy(Instr, Trap::Ptr* out_trap);
  RunResult DoTableGet(Instr, Trap::Ptr* out_trap);
  RunResult DoTableSet(Instr, Trap::Ptr* out_trap);
  RunResult DoTableGrow(Instr, Trap::Ptr* out_trap);
  RunResult DoTableSize(Instr);
  RunResult DoTableFill(Instr, Trap::Ptr* out_trap);

  template <typename R, typename T>
  RunResult DoSimdSplat();
  template <typename R, typename T>
  RunResult DoSimdExtract(Instr);
  template <typename R, typename T>
  RunResult DoSimdReplace(Instr);

  template <typename R, typename T>
  RunResult DoSimdUnop(UnopFunc<R, T>);
  // Like DoSimdUnop but zeroes top half.
  template <typename R, typename T>
  RunResult DoSimdUnopZero(UnopFunc<R, T>);
  template <typename R, typename T>
  RunResult DoSimdBinop(BinopFunc<R, T>);
  RunResult DoSimdBitSelect();
  template <typename S, u8 count>
  RunResult DoSimdIsTrue();
  template <typename S>
  RunResult DoSimdBitmask();
  template <typename R, typename T>
  RunResult DoSimdShift(BinopFunc<R, T>);
  template <typename S>
  RunResult DoSimdLoadSplat(Instr, Trap::Ptr* out_trap);
  template <typename S>
  RunResult DoSimdLoadLane(Instr, Trap::Ptr* out_trap);
  template <typename S>
  RunResult DoSimdStoreLane(Instr, Trap::Ptr* out_trap);
  template <typename S, typename T>
  RunResult DoSimdLoadZero(Instr, Trap::Ptr* out_trap);
  RunResult DoSimdSwizzle();
  RunResult DoSimdShuffle(Instr);
  template <typename S, typename T>
  RunResult DoSimdNarrow();
  template <typename S, typename T, bool low>
  RunResult DoSimdConvert();
  template <typename S, typename T>
  RunResult DoSimdDot();
  template <typename S, typename T>
  RunResult DoSimdLoadExtend(Instr, Trap::Ptr* out_trap);
  template <typename S, typename T>
  RunResult DoSimdExtaddPairwise();
  template <typename S, typename T, bool low>
  RunResult DoSimdExtmul();

  template <typename T, typename V = T>
  RunResult DoAtomicLoad(Instr, Trap::Ptr* out_trap);
  template <typename T, typename V = T>
  RunResult DoAtomicStore(Instr, Trap::Ptr* out_trap);
  template <typename R, typename T>
  RunResult DoAtomicRmw(BinopFunc<T, T>, Instr, Trap::Ptr* out_trap);
  template <typename T, typename V = T>
  RunResult DoAtomicRmwCmpxchg(Instr, Trap::Ptr* out_trap);

  RunResult StepInternal(Trap::Ptr* out_trap);

  std::vector<Frame> frames_;
  std::vector<Value> values_;
  std::vector<u32> refs_;  // Index into values_.

  // Cached for convenience.
  Store& store_;
  Instance* inst_ = nullptr;
  Module* mod_ = nullptr;

  // Tracing.
  Stream* trace_stream_;
  std::unique_ptr<TraceSource> trace_source_;
};

struct Thread::TraceSource : Istream::TraceSource {
 public:
  explicit TraceSource(Thread*);
  std::string Header(Istream::Offset) override;
  std::string Pick(Index, Instr) override;

 private:
  ValueType GetLocalType(Index);
  ValueType GetGlobalType(Index);
  ValueType GetTableElementType(Index);

  Thread* thread_;
};

}  // namespace interp
}  // namespace wabt

#include "src/interp/interp-inl.h"

#endif  // WABT_INTERP_H_
