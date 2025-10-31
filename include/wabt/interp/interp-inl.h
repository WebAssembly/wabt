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

#include <cassert>
#include <limits>
#include <string>

namespace wabt {
namespace interp {

//// Ref ////
inline Ref::Ref(size_t index) : index(index) {}

inline bool operator==(Ref lhs, Ref rhs) {
  return lhs.index == rhs.index;
}

inline bool operator!=(Ref lhs, Ref rhs) {
  return lhs.index != rhs.index;
}

//// ExternType ////
inline ExternType::ExternType(ExternKind kind) : kind(kind) {}

//// FuncType ////
// static
inline bool FuncType::classof(const ExternType* type) {
  return type->kind == skind;
}

inline FuncType::FuncType(ValueTypes params, ValueTypes results)
    : ExternType(ExternKind::Func), params(params), results(results), func_types(nullptr) {}

//// TableType ////
// static
inline bool TableType::classof(const ExternType* type) {
  return type->kind == skind;
}

inline TableType::TableType(ValueType element, Limits limits)
    : ExternType(ExternKind::Table), element(element), limits(limits) {
  // Always set max.
  if (!limits.has_max) {
    this->limits.max = std::numeric_limits<u32>::max();
  }
}

//// MemoryType ////
// static
inline bool MemoryType::classof(const ExternType* type) {
  return type->kind == skind;
}

inline MemoryType::MemoryType(Limits limits, uint32_t page_size)
    : ExternType(ExternKind::Memory), limits(limits), page_size(page_size) {
  // Always set max.
  if (!limits.has_max) {
    this->limits.max = WABT_BYTES_TO_MIN_PAGES(
        (limits.is_64 ? UINT64_MAX : UINT32_MAX), page_size);
  }
}

//// GlobalType ////
// static
inline bool GlobalType::classof(const ExternType* type) {
  return type->kind == skind;
}

inline GlobalType::GlobalType(ValueType type, Mutability mut)
    : ExternType(ExternKind::Global), type(type), mut(mut) {}

//// TagType ////
// static
inline bool TagType::classof(const ExternType* type) {
  return type->kind == skind;
}

inline TagType::TagType(TagAttr attr, const ValueTypes& signature)
    : ExternType(ExternKind::Tag), attr(attr), signature(signature) {}

//// ImportType ////
inline ImportType::ImportType(std::string module,
                              std::string name,
                              std::unique_ptr<ExternType> type)
    : module(module), name(name), type(std::move(type)) {}

inline ImportType::ImportType(const ImportType& other)
    : module(other.module), name(other.name), type(other.type->Clone()) {}

inline ImportType& ImportType::operator=(const ImportType& other) {
  if (this != &other) {
    module = other.module;
    name = other.name;
    type = other.type->Clone();
  }
  return *this;
}

//// ExportType ////
inline ExportType::ExportType(std::string name,
                              std::unique_ptr<ExternType> type)
    : name(name), type(std::move(type)) {}

inline ExportType::ExportType(const ExportType& other)
    : name(other.name), type(other.type->Clone()) {}

inline ExportType& ExportType::operator=(const ExportType& other) {
  if (this != &other) {
    name = other.name;
    type = other.type->Clone();
  }
  return *this;
}

//// Frame ////
inline Frame::Frame(Ref func,
                    u32 values,
                    u32 exceptions,
                    u32 offset,
                    Instance* inst,
                    Module* mod)
    : func(func),
      values(values),
      exceptions(exceptions),
      offset(offset),
      inst(inst),
      mod(mod) {}

//// FreeList ////
template <>
inline bool FreeList<Ref>::IsUsed(Index index) const {
  return (list_[index].index & refFreeBit) == 0;
}

template <>
inline FreeList<Ref>::~FreeList() {}

template <>
template <typename... Args>
auto FreeList<Ref>::New(Args&&... args) -> Index {
  if (free_head_ == 0) {
    list_.push_back(Ref(std::forward<Args>(args)...));
    return list_.size() - 1;
  }

  Index index = free_head_ - 1;

  assert(!IsUsed(index));
  assert(free_items_ > 0);

  free_head_ = list_[index].index & (refFreeBit - 1);
  list_[index] = Ref(std::forward<Args>(args)...);
  free_items_--;
  return index;
}

template <>
inline void FreeList<Ref>::Delete(Index index) {
  assert(IsUsed(index));

  list_[index].index = free_head_ | refFreeBit;
  free_head_ = index + 1;
  free_items_++;
}

template <typename T>
bool FreeList<T>::IsUsed(Index index) const {
  return (reinterpret_cast<uintptr_t>(list_[index]) & ptrFreeBit) == 0;
}

template <typename T>
FreeList<T>::~FreeList() {
  for (auto object : list_) {
    if ((reinterpret_cast<uintptr_t>(object) & ptrFreeBit) == 0) {
      delete object;
    }
  }
}

template <typename T>
template <typename... Args>
auto FreeList<T>::New(Args&&... args) -> Index {
  if (free_head_ == 0) {
    list_.push_back(T(std::forward<Args>(args)...));
    return list_.size() - 1;
  }

  Index index = free_head_ - 1;

  assert(!IsUsed(index));
  assert(free_items_ > 0);

  free_head_ = reinterpret_cast<uintptr_t>(list_[index]) >> ptrFreeShift;
  list_[index] = T(std::forward<Args>(args)...);
  free_items_--;
  return index;
}

template <typename T>
void FreeList<T>::Delete(Index index) {
  assert(IsUsed(index));

  delete list_[index];
  list_[index] = reinterpret_cast<T>((free_head_ << ptrFreeShift) | ptrFreeBit);
  free_head_ = index + 1;
  free_items_++;
}

template <typename T>
const T& FreeList<T>::Get(Index index) const {
  assert(IsUsed(index));
  return list_[index];
}

template <typename T>
T& FreeList<T>::Get(Index index) {
  assert(IsUsed(index));
  return list_[index];
}

template <typename T>
auto FreeList<T>::size() const -> Index {
  return list_.size();
}

template <typename T>
auto FreeList<T>::count() const -> Index {
  return list_.size() - free_items_;
}

//// RefPtr ////
template <typename T>
RefPtr<T>::RefPtr() : obj_(nullptr), store_(nullptr), root_index_(0) {}

template <typename T>
RefPtr<T>::RefPtr(Store& store, Ref ref) {
#ifndef NDEBUG
  if (!store.Is<T>(ref)) {
    ObjectKind ref_kind;
    if (ref == Ref::Null) {
      ref_kind = ObjectKind::Null;
    } else {
      ref_kind = store.objects_.Get(ref.index)->kind();
    }
    fprintf(stderr, "Invalid conversion from Ref (%s) to RefPtr<%s>!\n",
            GetName(ref_kind), T::GetTypeName());
    abort();
  }
#endif
  root_index_ = store.NewRoot(ref);
  obj_ = static_cast<T*>(store.objects_.Get(ref.index));
  store_ = &store;
}

template <typename T>
RefPtr<T>::RefPtr(const RefPtr& other)
    : obj_(other.obj_), store_(other.store_) {
  root_index_ = store_ ? store_->CopyRoot(other.root_index_) : 0;
}

template <typename T>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr& other) {
  obj_ = other.obj_;
  store_ = other.store_;
  root_index_ = store_ ? store_->CopyRoot(other.root_index_) : 0;
  return *this;
}

template <typename T>
RefPtr<T>::RefPtr(RefPtr&& other)
    : obj_(other.obj_), store_(other.store_), root_index_(other.root_index_) {
  other.obj_ = nullptr;
  other.store_ = nullptr;
  other.root_index_ = 0;
}

template <typename T>
RefPtr<T>& RefPtr<T>::operator=(RefPtr&& other) {
  obj_ = other.obj_;
  store_ = other.store_;
  root_index_ = other.root_index_;
  other.obj_ = nullptr;
  other.store_ = nullptr;
  other.root_index_ = 0;
  return *this;
}

template <typename T>
RefPtr<T>::~RefPtr() {
  reset();
}

template <typename T>
template <typename U>
RefPtr<T>::RefPtr(const RefPtr<U>& other)
    : obj_(other.obj_), store_(other.store_) {
  root_index_ = store_ ? store_->CopyRoot(other.root_index_) : 0;
}

template <typename T>
template <typename U>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr<U>& other) {
  obj_ = other.obj_;
  store_ = other.store_;
  root_index_ = store_ ? store_->CopyRoot(other.root_index_) : 0;
  return *this;
}

template <typename T>
template <typename U>
RefPtr<T>::RefPtr(RefPtr&& other)
    : obj_(other.obj_), store_(other.store_), root_index_(other.root_index_) {
  other.obj_ = nullptr;
  other.store_ = nullptr;
  other.root_index_ = 0;
}

template <typename T>
template <typename U>
RefPtr<T>& RefPtr<T>::operator=(RefPtr&& other) {
  obj_ = other.obj_;
  store_ = other.store_;
  root_index_ = other.root_index_;
  other.obj_ = nullptr;
  other.store_ = nullptr;
  other.root_index_ = 0;
  return *this;
}

template <typename T>
template <typename U>
RefPtr<U> RefPtr<T>::As() {
  static_assert(std::is_base_of<T, U>::value, "T must be base class of U");
  assert(store_->Is<U>(obj_->self()));
  RefPtr<U> result;
  result.obj_ = static_cast<U*>(obj_);
  result.store_ = store_;
  result.root_index_ = store_->CopyRoot(root_index_);
  return result;
}

template <typename T>
bool RefPtr<T>::empty() const {
  return obj_ == nullptr;
}

template <typename T>
void RefPtr<T>::reset() {
  if (obj_) {
    store_->DeleteRoot(root_index_);
    obj_ = nullptr;
    root_index_ = 0;
    store_ = nullptr;
  }
}

template <typename T>
T* RefPtr<T>::get() const {
  return obj_;
}

template <typename T>
T* RefPtr<T>::operator->() const {
  return obj_;
}

template <typename T>
T& RefPtr<T>::operator*() const {
  return *obj_;
}

template <typename T>
RefPtr<T>::operator bool() const {
  return obj_ != nullptr;
}

template <typename T>
Ref RefPtr<T>::ref() const {
  return store_ ? store_->roots_.Get(root_index_) : Ref::Null;
}

template <typename T>
Store* RefPtr<T>::store() const {
  return store_;
}

template <typename U, typename V>
bool operator==(const RefPtr<U>& lhs, const RefPtr<V>& rhs) {
  return lhs.obj_->self() == rhs.obj_->self();
}

template <typename U, typename V>
bool operator!=(const RefPtr<U>& lhs, const RefPtr<V>& rhs) {
  return lhs.obj_->self() != rhs.obj_->self();
}

//// ValueType ////
inline bool IsReference(ValueType type) { return type.IsRef(); }
template <> inline bool HasType<s32>(ValueType type) { return type == ValueType::I32; }
template <> inline bool HasType<u32>(ValueType type) { return type == ValueType::I32; }
template <> inline bool HasType<s64>(ValueType type) { return type == ValueType::I64; }
template <> inline bool HasType<u64>(ValueType type) { return type == ValueType::I64; }
template <> inline bool HasType<f32>(ValueType type) { return type == ValueType::F32; }
template <> inline bool HasType<f64>(ValueType type) { return type == ValueType::F64; }
template <> inline bool HasType<Ref>(ValueType type) { return IsReference(type); }

template <typename T>
void RequireType(ValueType type) {
  assert(HasType<T>(type));
}

inline bool TypesMatch(ValueType expected, ValueType actual) {
  // Currently there is no subtyping, so expected and actual must match
  // exactly. In the future this may be expanded.
  return expected == actual;
}

//// Value ////
inline Value WABT_VECTORCALL Value::Make(s32 val) { Value res; res.i32_ = val; res.SetType(ValueType::I32); return res; }
inline Value WABT_VECTORCALL Value::Make(u32 val) { Value res; res.i32_ = val; res.SetType(ValueType::I32); return res; }
inline Value WABT_VECTORCALL Value::Make(s64 val) { Value res; res.i64_ = val; res.SetType(ValueType::I64); return res; }
inline Value WABT_VECTORCALL Value::Make(u64 val) { Value res; res.i64_ = val; res.SetType(ValueType::I64); return res; }
inline Value WABT_VECTORCALL Value::Make(f32 val) { Value res; res.f32_ = val; res.SetType(ValueType::F32); return res; }
inline Value WABT_VECTORCALL Value::Make(f64 val) { Value res; res.f64_ = val; res.SetType(ValueType::F64); return res; }
inline Value WABT_VECTORCALL Value::Make(v128 val) { Value res; res.v128_ = val; res.SetType(ValueType::V128); return res; }
inline Value WABT_VECTORCALL Value::Make(Ref val) { Value res; res.ref_ = val; res.SetType(ValueType::ExternRef); return res; }
template <typename T, u8 L>
Value WABT_VECTORCALL Value::Make(Simd<T, L> val) {
  Value res;
  res.v128_ = Bitcast<v128>(val);
  res.SetType(ValueType::V128);
  return res;
}

template <> inline s8 WABT_VECTORCALL Value::Get<s8>() const { CheckType(ValueType::I32); return i32_; }
template <> inline u8 WABT_VECTORCALL Value::Get<u8>() const { CheckType(ValueType::I32); return i32_; }
template <> inline s16 WABT_VECTORCALL Value::Get<s16>() const { CheckType(ValueType::I32); return i32_; }
template <> inline u16 WABT_VECTORCALL Value::Get<u16>() const { CheckType(ValueType::I32); return i32_; }
template <> inline s32 WABT_VECTORCALL Value::Get<s32>() const { CheckType(ValueType::I32); return i32_; }
template <> inline u32 WABT_VECTORCALL Value::Get<u32>() const { CheckType(ValueType::I32); return i32_; }
template <> inline s64 WABT_VECTORCALL Value::Get<s64>() const { CheckType(ValueType::I64); return i64_; }
template <> inline u64 WABT_VECTORCALL Value::Get<u64>() const { CheckType(ValueType::I64); return i64_; }
template <> inline f32 WABT_VECTORCALL Value::Get<f32>() const { CheckType(ValueType::F32); return f32_; }
template <> inline f64 WABT_VECTORCALL Value::Get<f64>() const { CheckType(ValueType::F64); return f64_; }
template <> inline v128 WABT_VECTORCALL Value::Get<v128>() const { CheckType(ValueType::V128); return v128_; }
template <> inline Ref WABT_VECTORCALL Value::Get<Ref>() const { CheckType(ValueType::ExternRef); return ref_; }

template <> inline s8x16 WABT_VECTORCALL Value::Get<s8x16>() const { CheckType(ValueType::V128); return Bitcast<s8x16>(v128_); }
template <> inline u8x16 WABT_VECTORCALL Value::Get<u8x16>() const { CheckType(ValueType::V128); return Bitcast<u8x16>(v128_); }
template <> inline s16x8 WABT_VECTORCALL Value::Get<s16x8>() const { CheckType(ValueType::V128); return Bitcast<s16x8>(v128_); }
template <> inline u16x8 WABT_VECTORCALL Value::Get<u16x8>() const { CheckType(ValueType::V128); return Bitcast<u16x8>(v128_); }
template <> inline s32x4 WABT_VECTORCALL Value::Get<s32x4>() const { CheckType(ValueType::V128); return Bitcast<s32x4>(v128_); }
template <> inline u32x4 WABT_VECTORCALL Value::Get<u32x4>() const { CheckType(ValueType::V128); return Bitcast<u32x4>(v128_); }
template <> inline s64x2 WABT_VECTORCALL Value::Get<s64x2>() const { CheckType(ValueType::V128); return Bitcast<s64x2>(v128_); }
template <> inline u64x2 WABT_VECTORCALL Value::Get<u64x2>() const { CheckType(ValueType::V128); return Bitcast<u64x2>(v128_); }
template <> inline f32x4 WABT_VECTORCALL Value::Get<f32x4>() const { CheckType(ValueType::V128); return Bitcast<f32x4>(v128_); }
template <> inline f64x2 WABT_VECTORCALL Value::Get<f64x2>() const { CheckType(ValueType::V128); return Bitcast<f64x2>(v128_); }

template <> inline void WABT_VECTORCALL Value::Set<s32>(s32 val) { i32_ = val; SetType(ValueType::I32); }
template <> inline void WABT_VECTORCALL Value::Set<u32>(u32 val) { i32_ = val; SetType(ValueType::I32); }
template <> inline void WABT_VECTORCALL Value::Set<s64>(s64 val) { i64_ = val; SetType(ValueType::I64); }
template <> inline void WABT_VECTORCALL Value::Set<u64>(u64 val) { i64_ = val; SetType(ValueType::I64); }
template <> inline void WABT_VECTORCALL Value::Set<f32>(f32 val) { f32_ = val; SetType(ValueType::F32); }
template <> inline void WABT_VECTORCALL Value::Set<f64>(f64 val) { f64_ = val; SetType(ValueType::F64); }
template <> inline void WABT_VECTORCALL Value::Set<v128>(v128 val) { v128_ = val; SetType(ValueType::V128); }
template <> inline void WABT_VECTORCALL Value::Set<Ref>(Ref val) { ref_ = val; SetType(ValueType::ExternRef); }

//// Store ////
inline bool Store::IsValid(Ref ref) const {
  return objects_.IsUsed(ref.index) && objects_.Get(ref.index);
}

template <typename T>
bool Store::Is(Ref ref) const {
  return objects_.IsUsed(ref.index) && isa<T>(objects_.Get(ref.index));
}

template <typename T>
Result Store::Get(Ref ref, RefPtr<T>* out) {
  if (Is<T>(ref)) {
    *out = RefPtr<T>(*this, ref);
    return Result::Ok;
  }
  return Result::Error;
}

template <typename T>
RefPtr<T> Store::UnsafeGet(Ref ref) {
  return RefPtr<T>(*this, ref);
}

template <typename T, typename... Args>
RefPtr<T> Store::Alloc(Args&&... args) {
  Ref ref{objects_.New(new T(std::forward<Args>(args)...))};
  RefPtr<T> ptr{*this, ref};
  ptr->self_ = ref;
  return ptr;
}

inline Store::ObjectList::Index Store::object_count() const {
  return objects_.count();
}

inline const Features& Store::features() const {
  return features_;
}

inline std::set<Thread*>& Store::threads() {
  return threads_;
}

//// Object ////
// static
inline bool Object::classof(const Object* obj) {
  return true;
}

inline Object::Object(ObjectKind kind) : kind_(kind) {}

inline ObjectKind Object::kind() const {
  return kind_;
}

inline Ref Object::self() const {
  return self_;
}

inline void* Object::host_info() const {
  return host_info_;
}

inline void Object::set_host_info(void* host_info) {
  host_info_ = host_info;
}

inline Finalizer Object::get_finalizer() const {
  return finalizer_;
}

inline void Object::set_finalizer(Finalizer finalizer) {
  finalizer_ = finalizer;
}

//// Foreign ////
// static
inline bool Foreign::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Foreign::Ptr Foreign::New(Store& store, void* ptr) {
  return store.Alloc<Foreign>(store, ptr);
}

inline void* Foreign::ptr() {
  return ptr_;
}

//// Trap ////
// static
inline bool Trap::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Trap::Ptr Trap::New(Store& store,
                           const std::string& msg,
                           const std::vector<Frame>& trace) {
  return store.Alloc<Trap>(store, msg, trace);
}

inline std::string Trap::message() const {
  return message_;
}

//// Exception ////
// static
inline bool Exception::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Exception::Ptr Exception::New(Store& store, Ref tag, Values& args) {
  return store.Alloc<Exception>(store, tag, args);
}

inline Ref Exception::tag() const {
  return tag_;
}

inline Values& Exception::args() {
  return args_;
}

//// Extern ////
// static
inline bool Extern::classof(const Object* obj) {
  switch (obj->kind()) {
    case ObjectKind::DefinedFunc:
    case ObjectKind::HostFunc:
    case ObjectKind::Table:
    case ObjectKind::Memory:
    case ObjectKind::Global:
    case ObjectKind::Tag:
      return true;
    default:
      return false;
  }
}

inline Extern::Extern(ObjectKind kind) : Object(kind) {}

//// Func ////
// static
inline bool Func::classof(const Object* obj) {
  switch (obj->kind()) {
    case ObjectKind::DefinedFunc:
    case ObjectKind::HostFunc:
      return true;
    default:
      return false;
  }
}

inline const ExternType& Func::extern_type() {
  return type_;
}

inline const FuncType& Func::type() const {
  return type_;
}

//// DefinedFunc ////
// static
inline bool DefinedFunc::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline DefinedFunc::Ptr DefinedFunc::New(Store& store,
                                         Ref instance,
                                         FuncDesc desc) {
  return store.Alloc<DefinedFunc>(store, instance, desc);
}

inline Ref DefinedFunc::instance() const {
  return instance_;
}

inline const FuncDesc& DefinedFunc::desc() const {
  return desc_;
}

//// HostFunc ////
// static
inline bool HostFunc::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline HostFunc::Ptr HostFunc::New(Store& store, FuncType type, Callback cb) {
  return store.Alloc<HostFunc>(store, type, cb);
}

//// Table ////
// static
inline bool Table::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Table::Ptr Table::New(Store& store, TableType type) {
  return store.Alloc<Table>(store, type);
}

inline const ExternType& Table::extern_type() {
  return type_;
}

inline const TableType& Table::type() const {
  return type_;
}

inline const RefVec& Table::elements() const {
  return elements_;
}

inline u32 Table::size() const {
  return static_cast<u32>(elements_.size());
}

//// Memory ////
// static
inline bool Memory::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Memory::Ptr Memory::New(interp::Store& store, MemoryType type) {
  return store.Alloc<Memory>(store, type);
}

inline bool Memory::IsValidAccess(u64 offset, u64 addend, u64 size) const {
  // FIXME: make this faster.
  return offset <= data_.size() && addend <= data_.size() &&
         size <= data_.size() && offset + addend + size <= data_.size();
}

inline bool Memory::IsValidAtomicAccess(u64 offset,
                                        u64 addend,
                                        u64 size) const {
  return IsValidAccess(offset, addend, size) &&
         ((offset + addend) & (size - 1)) == 0;
}

template <typename T>
Result Memory::Load(u64 offset, u64 addend, T* out) const {
  if (!IsValidAccess(offset, addend, sizeof(T))) {
    return Result::Error;
  }
  MemcpyEndianAware(out, data_.data(), sizeof(T), data_.size(), 0,
                    offset + addend, sizeof(T));
  return Result::Ok;
}

template <typename T>
T WABT_VECTORCALL Memory::UnsafeLoad(u64 offset, u64 addend) const {
  assert(IsValidAccess(offset, addend, sizeof(T)));
  T val;
  MemcpyEndianAware(&val, data_.data(), sizeof(T), data_.size(), 0,
                    offset + addend, sizeof(T));
  return val;
}

template <typename T>
Result WABT_VECTORCALL Memory::Store(u64 offset, u64 addend, T val) {
  if (!IsValidAccess(offset, addend, sizeof(T))) {
    return Result::Error;
  }
  MemcpyEndianAware(data_.data(), &val, data_.size(), sizeof(T),
                    offset + addend, 0, sizeof(T));
  return Result::Ok;
}

template <typename T>
Result Memory::AtomicLoad(u64 offset, u64 addend, T* out) const {
  if (!IsValidAtomicAccess(offset, addend, sizeof(T))) {
    return Result::Error;
  }
  MemcpyEndianAware(out, data_.data(), sizeof(T), data_.size(), 0,
                    offset + addend, sizeof(T));
  return Result::Ok;
}

template <typename T>
Result Memory::AtomicStore(u64 offset, u64 addend, T val) {
  if (!IsValidAtomicAccess(offset, addend, sizeof(T))) {
    return Result::Error;
  }
  MemcpyEndianAware(data_.data(), &val, data_.size(), sizeof(T),
                    offset + addend, 0, sizeof(T));
  return Result::Ok;
}

template <typename T, typename F>
Result Memory::AtomicRmw(u64 offset, u64 addend, T rhs, F&& func, T* out) {
  T lhs;
  CHECK_RESULT(AtomicLoad(offset, addend, &lhs));
  CHECK_RESULT(AtomicStore(offset, addend, func(lhs, rhs)));
  *out = lhs;
  return Result::Ok;
}

template <typename T>
Result Memory::AtomicRmwCmpxchg(u64 offset,
                                u64 addend,
                                T expect,
                                T replace,
                                T* out) {
  T read;
  CHECK_RESULT(AtomicLoad(offset, addend, &read));
  if (read == expect) {
    CHECK_RESULT(AtomicStore(offset, addend, replace));
  }
  *out = read;
  return Result::Ok;
}

inline u8* Memory::UnsafeData() {
  return data_.data();
}

inline u64 Memory::ByteSize() const {
  return data_.size();
}

inline u64 Memory::PageSize() const {
  return pages_;
}

inline const ExternType& Memory::extern_type() {
  return type_;
}

inline const MemoryType& Memory::type() const {
  return type_;
}

//// Global ////
// static
inline bool Global::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Global::Ptr Global::New(Store& store, GlobalType type, Value value) {
  return store.Alloc<Global>(store, type, value);
}

inline Value Global::Get() const {
  return value_;
}

template <typename T>
Result Global::Get(T* out) const {
  if (HasType<T>(type_.type)) {
    *out = value_.Get<T>();
    return Result::Ok;
  }
  return Result::Error;
}

template <typename T>
T WABT_VECTORCALL Global::UnsafeGet() const {
  RequireType<T>(type_.type);
  return value_.Get<T>();
}

template <typename T>
Result WABT_VECTORCALL Global::Set(T val) {
  if (type_.mut == Mutability::Var && HasType<T>(type_.type)) {
    value_.Set(val);
    return Result::Ok;
  }
  return Result::Error;
}

inline const ExternType& Global::extern_type() {
  return type_;
}

inline const GlobalType& Global::type() const {
  return type_;
}

//// Tag ////
// static
inline bool Tag::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Tag::Ptr Tag::New(Store& store, TagType type) {
  return store.Alloc<Tag>(store, type);
}

inline const ExternType& Tag::extern_type() {
  return type_;
}

inline const TagType& Tag::type() const {
  return type_;
}

//// ElemSegment ////
inline void ElemSegment::Drop() {
  elements_.clear();
}

inline const ElemDesc& ElemSegment::desc() const {
  return *desc_;
}

inline const RefVec& ElemSegment::elements() const {
  return elements_;
}

inline u32 ElemSegment::size() const {
  return elements_.size();
}

//// DataSegment ////
inline void DataSegment::Drop() {
  size_ = 0;
}

inline const DataDesc& DataSegment::desc() const {
  return *desc_;
}

inline u64 DataSegment::size() const {
  return size_;
}

//// Module ////
// static
inline bool Module::classof(const Object* obj) {
  return obj->kind() == skind;
}

// static
inline Module::Ptr Module::New(Store& store, ModuleDesc desc) {
  return store.Alloc<Module>(store, std::move(desc));
}

inline const ModuleDesc& Module::desc() const {
  return desc_;
}

inline const std::vector<ImportType>& Module::import_types() const {
  return import_types_;
}

inline const std::vector<ExportType>& Module::export_types() const {
  return export_types_;
}

//// Instance ////
// static
inline bool Instance::classof(const Object* obj) {
  return obj->kind() == skind;
}

inline Ref Instance::module() const {
  return module_;
}

inline const RefVec& Instance::imports() const {
  return imports_;
}

inline const RefVec& Instance::funcs() const {
  return funcs_;
}

inline const RefVec& Instance::tables() const {
  return tables_;
}

inline const RefVec& Instance::memories() const {
  return memories_;
}

inline const RefVec& Instance::globals() const {
  return globals_;
}

inline const RefVec& Instance::tags() const {
  return tags_;
}

inline const RefVec& Instance::exports() const {
  return exports_;
}

inline const std::vector<ElemSegment>& Instance::elems() const {
  return elems_;
}

inline std::vector<ElemSegment>& Instance::elems() {
  return elems_;
}

inline const std::vector<DataSegment>& Instance::datas() const {
  return datas_;
}

inline std::vector<DataSegment>& Instance::datas() {
  return datas_;
}

//// Thread ////
inline Store& Thread::store() {
  return store_;
}

}  // namespace interp
}  // namespace wabt
