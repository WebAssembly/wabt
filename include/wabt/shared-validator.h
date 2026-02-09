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

#ifndef WABT_SHARED_VALIDATOR_H_
#define WABT_SHARED_VALIDATOR_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "wabt/common.h"
#include "wabt/error.h"
#include "wabt/feature.h"
#include "wabt/ir.h"
#include "wabt/opcode.h"
#include "wabt/type-checker.h"

namespace wabt {

struct ValidateOptions {
  ValidateOptions() = default;
  ValidateOptions(const Features& features) : features(features) {}

  Features features;
};

enum class TableImportStatus {
  TableIsImported,
  TableIsNotImported,
};

class SharedValidator {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(SharedValidator);
  using TypeEntry = TypeChecker::TypeEntry;
  using FuncType = TypeChecker::FuncType;
  using StructType = TypeChecker::StructType;
  using ArrayType = TypeChecker::ArrayType;
  using RecGroup = TypeChecker::RecGroup;
  SharedValidator(Errors*,
                  std::string_view filename,
                  const ValidateOptions& options);

  // TODO: Move into SharedValidator?
  using Label = TypeChecker::Label;
  size_t type_stack_size() const { return typechecker_.type_stack_size(); }
  Result GetLabel(Index depth, Label** out_label) {
    return typechecker_.GetLabel(depth, out_label);
  }
  Result GetCatchCount(Index depth, Index* out_count) {
    return typechecker_.GetCatchCount(depth, out_count);
  }

  Result WABT_PRINTF_FORMAT(3, 4)
      PrintError(const Location& loc, const char* fmt, ...);

  void OnTypecheckerError(const char* msg);

  Index GetLocalCount() const;

  // The canonical index is the lowest index, which represents
  // the same type as the type_index. The canonical index is
  // always less or equal than type_index.
  Index GetCanonicalTypeIndex(Index type_index);

  Result EndModule();

  Result OnRecursiveGroup(Index first_type_index, Index type_count);
  Result OnFuncType(const Location&,
                    Index param_count,
                    const Type* param_types,
                    Index result_count,
                    const Type* result_types,
                    Index type_index,
                    SupertypesInfo* supertypes);
  Result OnStructType(const Location&,
                      Index field_count,
                      TypeMut* fields,
                      SupertypesInfo* supertypes);
  Result OnArrayType(const Location&,
                     TypeMut field,
                     SupertypesInfo* supertypes);

  Result OnFunction(const Location&, Var sig_var);
  Result OnTable(const Location&, Type elem_type, const Limits&, TableImportStatus import_status, TableInitExprStatus init_provided);
  Result OnMemory(const Location&, const Limits&, uint32_t page_size);
  Result OnGlobalImport(const Location&, Type type, bool mutable_);
  Result BeginGlobal(const Location&, Type type, bool mutable_);
  Result EndGlobal(const Location&);
  Result OnTag(const Location&, Var sig_var);

  Result OnExport(const Location&,
                  ExternalKind,
                  Var item_var,
                  std::string_view name);

  Result OnStart(const Location&, Var func_var);

  Result OnElemSegment(const Location&, Var table_var, SegmentKind);
  Result OnElemSegmentElemType(const Location&, Type elem_type);

  void OnDataCount(Index count);
  Result OnDataSegment(const Location&, Var memory_var, SegmentKind);

  Result BeginInitExpr(const Location&, Type type);
  Result EndInitExpr();

  Result BeginFunctionBody(const Location&, Index func_index);
  Result EndFunctionBody(const Location&);
  Result OnLocalDecl(const Location&, Index count, Type type);

  Result OnArrayCopy(const Location&, Var dst_type, Var src_type);
  Result OnArrayFill(const Location&, Var type);
  Result OnArrayGet(const Location&, Opcode, Var type);
  Result OnArrayInitData(const Location&, Var type, Var segment_var);
  Result OnArrayInitElem(const Location&, Var type, Var segment_var);
  Result OnArrayNew(const Location&, Var type);
  Result OnArrayNewData(const Location&, Var type, Var segment_var);
  Result OnArrayNewDefault(const Location&, Var type);
  Result OnArrayNewElem(const Location&, Var type, Var segment_var);
  Result OnArrayNewFixed(const Location&, Var type, Index count);
  Result OnArraySet(const Location&, Var type);
  Result OnAtomicFence(const Location&, uint32_t consistency_model);
  Result OnAtomicLoad(const Location&,
                      Opcode,
                      Var memidx,
                      Address align,
                      Address offset);
  Result OnAtomicNotify(const Location&,
                        Opcode,
                        Var memidx,
                        Address align,
                        Address offset);
  Result OnAtomicRmwCmpxchg(const Location&,
                            Opcode,
                            Var memidx,
                            Address align,
                            Address offset);
  Result OnAtomicRmw(const Location&,
                     Opcode,
                     Var memidx,
                     Address align,
                     Address offset);
  Result OnAtomicStore(const Location&,
                       Opcode,
                       Var memidx,
                       Address align,
                       Address offset);
  Result OnAtomicWait(const Location&,
                      Opcode,
                      Var memidx,
                      Address align,
                      Address offset);
  Result OnBinary(const Location&, Opcode);
  Result OnTernary(const Location&, Opcode);
  Result OnQuaternary(const Location&, Opcode);
  Result OnBlock(const Location&, Type sig_type);
  Result OnBr(const Location&, Var depth);
  Result OnBrIf(const Location&, Var depth);
  Result OnBrOnCast(const Location&,
                    Opcode, Var depth,
                    Var type1_var,
                    Var type2_var);
  Result OnBrOnNonNull(const Location&, Var depth);
  Result OnBrOnNull(const Location&, Var depth);
  Result BeginBrTable(const Location&);
  Result OnBrTableTarget(const Location&, Var depth);
  Result EndBrTable(const Location&);
  Result OnCall(const Location&, Var func_var);
  Result OnCallIndirect(const Location&, Var sig_var, Var table_var);
  Result OnCallRef(const Location&, Var function_type_var);
  Result OnCatch(const Location&, Var tag_var, bool is_catch_all);
  Result OnCompare(const Location&, Opcode);
  Result OnConst(const Location&, Type);
  Result OnConvert(const Location&, Opcode);
  Result OnDataDrop(const Location&, Var segment_var);
  Result OnDelegate(const Location&, Var depth);
  Result OnDrop(const Location&);
  Result OnElemDrop(const Location&, Var segment_var);
  Result OnElse(const Location&);
  Result OnEnd(const Location&);
  Result OnGCUnary(const Location&, Opcode);
  Result OnGlobalGet(const Location&, Var);
  Result OnGlobalSet(const Location&, Var);
  Result OnIf(const Location&, Type sig_type);
  Result OnLoad(const Location&, Opcode, Var memidx, Address align, Address offset);
  Result OnLoadSplat(const Location&,
                     Opcode,
                     Var memidx,
                     Address align,
                     Address offset);
  Result OnLoadZero(const Location&,
                    Opcode,
                    Var memidx,
                    Address align,
                    Address offset);
  Result OnLocalGet(const Location&, Var);
  Result OnLocalSet(const Location&, Var);
  Result OnLocalTee(const Location&, Var);
  Result OnLoop(const Location&, Type sig_type);
  Result OnMemoryCopy(const Location&, Var destmemidx, Var srcmemidx);
  Result OnMemoryFill(const Location&, Var memidx);
  Result OnMemoryGrow(const Location&, Var memidx);
  Result OnMemoryInit(const Location&, Var segment_var, Var memidx);
  Result OnMemorySize(const Location&, Var memidx);
  Result OnNop(const Location&);
  Result OnRefAsNonNull(const Location&);
  Result OnRefCast(const Location&, Var type_var);
  Result OnRefFunc(const Location&, Var func_var);
  Result OnRefIsNull(const Location&);
  Result OnRefNull(const Location&, Var func_type_var);
  Result OnRefTest(const Location&, Var type_var);
  Result OnRethrow(const Location&, Var depth);
  Result OnReturnCall(const Location&, Var func_var);
  Result OnReturnCallIndirect(const Location&, Var sig_var, Var table_var);
  Result OnReturnCallRef(const Location&, Var function_type_var);
  Result OnReturn(const Location&);
  Result OnSelect(const Location&, Index result_count, Type* result_types);
  Result OnSimdLaneOp(const Location&, Opcode, uint64_t lane_idx);
  Result OnSimdLoadLane(const Location&,
                        Opcode,
                        Var memidx,
                        Address align,
                        Address offset,
                        uint64_t lane_idx);
  Result OnSimdStoreLane(const Location&,
                         Opcode,
                         Var memidx,
                         Address align,
                         Address offset,
                         uint64_t lane_idx);
  Result OnSimdShuffleOp(const Location&, Opcode, v128 lane_idx);
  Result OnStore(const Location&,
                 Opcode,
                 Var memidx,
                 Address align,
                 Address offset);
  Result OnStructGet(const Location&, Opcode, Var type, Var field);
  Result OnStructNew(const Location&, Var type);
  Result OnStructNewDefault(const Location&, Var type);
  Result OnStructSet(const Location&, Var type, Var field);
  Result OnTableCopy(const Location&, Var dst_var, Var src_var);
  Result OnTableFill(const Location&, Var table_var);
  Result OnTableGet(const Location&, Var table_var);
  Result OnTableGrow(const Location&, Var table_var);
  Result OnTableInit(const Location&, Var segment_var, Var table_var);
  Result OnTableSet(const Location&, Var table_var);
  Result OnTableSize(const Location&, Var table_var);
  Result OnThrow(const Location&, Var tag_var);
  Result OnThrowRef(const Location&);
  Result OnTry(const Location&, Type sig_type);
  Result BeginTryTable(const Location&, Type sig_type);
  Result OnTryTableCatch(const Location&, const TableCatch&);
  Result EndTryTable(const Location&, Type sig_type);
  Result OnUnary(const Location&, Opcode);
  Result OnUnreachable(const Location&);

 private:
  struct TableType {
    TableType() = default;
    TableType(Type element, Limits limits) : element(element), limits(limits) {}

    Type element = Type::Any;
    Limits limits;
  };

  struct MemoryType {
    MemoryType() = default;
    MemoryType(Limits limits) : limits(limits) {}

    Limits limits;
  };

  struct GlobalType {
    GlobalType() = default;
    GlobalType(Type type, bool mutable_) : type(type), mutable_(mutable_) {}

    Type type = Type::Any;
    bool mutable_ = true;
  };

  struct TagType {
    TypeVector params;
  };

  struct ElemType {
    ElemType() = default;
    ElemType(Type element, bool is_active, Type table_type)
        : element(element), is_active(is_active), table_type(table_type) {}

    Type element;
    bool is_active;
    Type table_type;
  };

  struct LocalDecl {
    Type type;
    Index end;
  };

  struct LocalReferenceMap {
    Type type;
    // An index for a single bit value, which represents that
    // the corresponding local reference has been set before.
    size_t local_ref_is_set;
  };

  bool ValidInitOpcode(Opcode opcode) const;
  Result CheckInstr(Opcode opcode, const Location& loc);
  Result CheckType(const Location&,
                   Type actual,
                   Type expected,
                   const char* desc);
  Result CheckReferenceType(const Location&,
                            Type type,
                            Index end_index,
                            const char* desc);
  Result CheckSupertypes(const Location&, SupertypesInfo* supertypes);
  Result CheckLimits(const Location&,
                     const Limits&,
                     uint64_t absolute_max,
                     const char* desc);

  Result CheckLocalIndex(Var local_var, Type* out_type);

  Result CheckDeclaredFunc(Var func_var);

  Result CheckIndex(Var var, Index max_index, const char* desc);
  template <typename T>
  Result CheckIndexWithValue(Var var,
                             const std::vector<T>& values,
                             T* out,
                             const char* desc);
  Result CheckFuncTypeIndex(Var sig_var, FuncType* out);
  Result CheckStructTypeIndex(Var type_var, Type* out_ref, StructType* out);
  Result CheckArrayTypeIndex(Var type_var, Type* out_ref, TypeMut* out);
  Result CheckFuncIndex(Var func_var, FuncType* out = nullptr);
  Result CheckTableIndex(Var table_var, TableType* out = nullptr);
  Result CheckMemoryIndex(Var memory_var, MemoryType* out = nullptr);
  Result CheckGlobalIndex(Var global_var, GlobalType* out = nullptr);
  Result CheckTagIndex(Var tag_var, TagType* out = nullptr);
  Result CheckElemSegmentIndex(Var elem_segment_var, ElemType* out = nullptr);
  Result CheckDataSegmentIndex(Var data_segment_var);

  Result CheckAlign(const Location&, Address align, Address natural_align);
  Result CheckOffset(const Location&, Address offset, const Limits& limits);
  Result CheckAtomicAlign(const Location&,
                          Address align,
                          Address natural_align);

  Result CheckBlockSignature(const Location&,
                             Opcode,
                             Type sig_type,
                             TypeVector* out_param_types,
                             TypeVector* out_result_types);

  Index GetFunctionTypeIndex(Index func_index) const;

  TypeVector ToTypeVector(Index count, const Type* types);

  void SaveLocalRefs();
  void RestoreLocalRefs(Result result);
  void IgnoreLocalRefs();

  Index GetRecGroupEnd();

  ValidateOptions options_;
  Errors* errors_;
  std::string_view filename_;
  TypeChecker typechecker_;  // TODO: Move into SharedValidator.
  // Cached for access by OnTypecheckerError.
  Location expr_loc_ = Location(kInvalidOffset);
  bool in_init_expr_ = false;

  TypeChecker::TypeFields type_fields_;

  std::vector<FuncType> funcs_;       // Includes imported and defined.
  std::vector<TableType> tables_;     // Includes imported and defined.
  std::vector<MemoryType> memories_;  // Includes imported and defined.
  std::vector<GlobalType> globals_;   // Includes imported and defined.
  std::vector<TagType> tags_;         // Includes imported and defined.
  std::vector<ElemType> elems_;
  Index starts_ = 0;
  Index last_initialized_global_ = 0;
  Index data_segments_ = 0;
  Index last_rec_type_end_ = 0;
  // Recursive type checks may enter to infinite loop for invalid values.
  Result type_validation_result_ = Result::Ok;

  // Includes parameters, since this is only used for validating
  // local.{get,set,tee} instructions.
  std::vector<LocalDecl> locals_;
  std::map<Index, LocalReferenceMap> local_refs_map_;
  std::vector<bool> local_ref_is_set_;

  std::set<std::string> export_names_;  // Used to check for duplicates.
  std::set<Index> declared_funcs_;      // TODO: optimize?
  std::vector<Var> check_declared_funcs_;
};

class SharedComponentValidator {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(SharedComponentValidator);
  SharedComponentValidator(Errors* errors,
                           std::string_view filename,
                           const ValidateOptions& options);

  Result WABT_PRINTF_FORMAT(3, 4)
      PrintError(const Location& loc, const char* format, ...);

  Result BeginComponent();
  Result EndComponent();

  Result OnCoreInstance(const ComponentIndexLoc& module_index,
                        uint32_t argument_count);
  Result OnCoreInstanceArg(const ComponentStringLoc& name,
                           ComponentSort sort,
                           const ComponentIndexLoc& index);
  Result OnInlineCoreInstance(uint32_t argument_count);
  Result OnInlineCoreInstanceArg(const ComponentStringLoc& name,
                                 ComponentSort sort,
                                 const ComponentIndexLoc& index);

  Result OnInstance(const ComponentIndexLoc& component_index,
                    uint32_t argument_count);
  Result OnInstanceArg(const ComponentStringLoc& name,
                       ComponentSort sort,
                       const ComponentIndexLoc& index);
  Result OnInlineInstance();
  Result OnInlineInstanceArg(const ComponentStringLoc& name,
                             bool has_version_suffix,
                             std::string_view version_suffix,
                             ComponentSort sort,
                             const ComponentIndexLoc& index);

  Result OnAliasExport(ComponentSort sort,
                       const ComponentIndexLoc& instance_index,
                       const ComponentStringLoc& name);
  Result OnAliasCoreExport(ComponentSort sort,
                           const ComponentIndexLoc& core_instance_index,
                           const ComponentStringLoc& name);
  Result OnAliasOuter(const Location& loc,
                      ComponentSort sort,
                      uint32_t counter,
                      uint32_t index);

  Result OnPrimitiveType(const ComponentType& type);
  Result OnRecordType(const Location& loc, uint32_t field_count);
  Result OnRecordField(const ComponentStringLoc& field_name,
                       const ComponentTypeLoc& field_type);
  Result OnVariantType(const Location& loc, uint32_t case_count);
  Result OnVariantCase(const ComponentStringLoc& case_name,
                       const ComponentTypeLoc& case_type);
  Result OnListType(const ComponentTypeLoc& type);
  Result OnListFixedType(const Location& loc,
                         const ComponentTypeLoc& type,
                         uint32_t size);
  Result OnTupleType(const Location& loc, uint32_t type_count);
  Result OnTupleItem(const ComponentTypeLoc& item);
  Result OnFlagsType(const Location& loc, uint32_t flag_count);
  Result OnFlagsLabel(const ComponentStringLoc& label);
  Result OnEnumType(const Location& loc, uint32_t enum_count);
  Result OnEnumLabel(const ComponentStringLoc& label);
  Result OnOptionType(const ComponentTypeLoc& type);
  Result OnResultType(const ComponentTypeLoc& result_type,
                      const ComponentTypeLoc& error_type);
  Result OnOwnType(const ComponentIndexLoc& index);
  Result OnBorrowType(const ComponentIndexLoc& index);
  Result OnStreamType(const ComponentTypeLoc& type);
  Result OnFutureType(const ComponentTypeLoc& type);
  Result OnFuncType(ComponentTypeDef type,
                    uint32_t param_count);
  Result OnFuncParam(ComponentStringLoc name,
                     ComponentTypeLoc type);
  Result OnFuncResult(const ComponentTypeLoc& type);
  Result OnResourceType(const Location& loc,
                        ComponentResourceRep rep,
                        const ComponentIndexLoc& dtor);
  Result OnResourceAsyncType(const Location& loc,
                             ComponentResourceRep rep,
                             const ComponentIndexLoc& dtor,
                             const ComponentIndexLoc& callback);
  Result BeginInstanceType(uint32_t count);
  Result EndInstanceType();
  Result BeginComponentType(uint32_t count);
  Result EndComponentType();

  Result OnCanonLift(const ComponentIndexLoc& core_func_index,
                     uint32_t option_count,
                     const ComponentCanonOption* options,
                     const ComponentIndexLoc& type_index);
  Result OnCanonLower(const ComponentIndexLoc& func_index,
                      uint32_t option_count,
                      const ComponentCanonOption* options);
  Result OnCanonType(ComponentCanon canon,
                     const ComponentIndexLoc& type_index);

  Result OnImport(const ComponentStringLoc& name,
                  std::string_view* version_suffix,
                  const ComponentExternalInfo& external_info);
  Result OnExport(const ComponentStringLoc& name,
                  std::string_view* version_suffix,
                  ComponentExternalInfo* external_info,
                  ComponentExportInfo* export_info);

 private:
  // Checks may ignore the last (partly created) item of a sort.
  enum CheckMode {
    IncludeLast,
    ExcludeLast,
  };

  enum TypeInfoBits : uint8_t {
    // Used by defined value types.
    HasResource = 0x01,
    HasBorrow = 0x02,
    // Used by TypeDefList type.
    IsObject = 0x4,
  };

  struct ValueType;
  struct ValueTypePair;
  struct TypeItems;
  struct TypeTuple;
  struct TypeLabels;
  struct TypeFunc;
  struct TypeExternalList;

  struct TypeBase {
    TypeBase(ComponentTypeDef type_def)
        : type_def(type_def), info_bits(0) {}

    virtual ~TypeBase() {}

    bool IsValueType() const {
      return type_def == ComponentTypeDef::ValueType ||
             type_def == ComponentTypeDef::List ||
             type_def == ComponentTypeDef::Option ||
             type_def == ComponentTypeDef::Own ||
             type_def == ComponentTypeDef::Borrow ||
             type_def == ComponentTypeDef::Stream ||
             type_def == ComponentTypeDef::Future;
    }

    ValueType* AsValueType() {
      assert(IsValueType());
      return reinterpret_cast<ValueType*>(this);
    }

    bool IsValueTypePair() const {
      return type_def == ComponentTypeDef::Result ||
             type_def == ComponentTypeDef::ResourceAsync;
    }

    ValueTypePair* AsValueTypePair() {
      assert(IsValueTypePair());
      return reinterpret_cast<ValueTypePair*>(this);
    }

    bool IsTypeItems() const {
      return type_def == ComponentTypeDef::Record ||
             type_def == ComponentTypeDef::Variant;
    }

    TypeItems* AsTypeItems() {
      assert(IsTypeItems());
      return reinterpret_cast<TypeItems*>(this);
    }

    TypeTuple* AsTypeTuple() {
      assert(type_def == ComponentTypeDef::Tuple);
      return reinterpret_cast<TypeTuple*>(this);
    }

    bool IsTypeLabels() const {
      return type_def == ComponentTypeDef::Flags ||
             type_def == ComponentTypeDef::Enum;
    }

    TypeLabels* AsTypeLabels() {
      assert(IsTypeLabels());
      return reinterpret_cast<TypeLabels*>(this);
    }

    bool IsTypeFunc() const {
      return type_def == ComponentTypeDef::Func ||
             type_def == ComponentTypeDef::AsyncFunc;
    }

    TypeFunc* AsTypeFunc() {
      assert(IsTypeFunc());
      return reinterpret_cast<TypeFunc*>(this);
    }

    TypeExternalList* AsTypeExternalList() {
      assert(type_def == ComponentTypeDef::Instance ||
             type_def == ComponentTypeDef::Component);
      return reinterpret_cast<TypeExternalList*>(this);
    }

    ComponentTypeDef type_def;
    uint8_t info_bits;
  };

  struct TypeRef {
    TypeRef()
        : type(ComponentType::TypeNone), ref(nullptr) {}

    TypeRef(const TypeBase* ref)
        : type(ComponentType::TypeIndex), ref(ref) {}

    TypeRef(ComponentType::Enum type)
        : type(type), ref(nullptr) {}

    ComponentType::Enum type;
    const TypeBase* ref;
  };

  struct ValueType : public TypeBase {
    ValueType(ComponentTypeDef type_def, const TypeRef& type)
        : TypeBase(type_def), type(type) {
      assert(IsValueType());
      if (type.ref != nullptr) {
        info_bits |= type.ref->info_bits;
      }
    }

    ValueType()
        : TypeBase(ComponentTypeDef::ValueType) {}

    TypeRef type;
  };

  struct ValueTypePair : public TypeBase {
    ValueTypePair(ComponentTypeDef type_def,
                  const TypeRef& first,
                  const TypeRef& second)
        : TypeBase(type_def), first(first), second(second) {
      assert(IsValueTypePair());
      if (first.ref != nullptr) {
        info_bits |= first.ref->info_bits;
      }
      if (second.ref != nullptr) {
        info_bits |= second.ref->info_bits;
      }
    }

    TypeRef first;
    TypeRef second;
  };

  struct TypeItems : public TypeBase {
    struct Item {
      const std::string* name;
      TypeRef type;
    };

    TypeItems(ComponentTypeDef type_def)
        : TypeBase(type_def) {
      assert(IsTypeItems());
    }

    std::vector<Item> items;
  };

  struct TypeListFixed : public TypeBase {
    TypeListFixed(const TypeRef& type, uint32_t size)
        : TypeBase(ComponentTypeDef::ListFixed), type(type), size(size) {
      assert(IsValueType());
      if (type.ref != nullptr) {
        info_bits |= type.ref->info_bits;
      }
    }

    TypeRef type;
    uint32_t size;
  };

  struct TypeTuple : public TypeBase {
    TypeTuple()
        : TypeBase(ComponentTypeDef::Tuple) {}

    std::vector<TypeRef> items;
  };

  struct TypeLabels : public TypeBase {
    TypeLabels(ComponentTypeDef type_def)
        : TypeBase(type_def) {
      assert(IsTypeLabels());
    }

    std::vector<const std::string*> items;
  };

  struct TypeFunc : public TypeBase {
    struct Param {
      const std::string* name;
      TypeRef type;
    };

    TypeFunc(ComponentTypeDef type_def)
        : TypeBase(type_def) {
      assert(IsTypeFunc());
    }

    std::vector<Param> params;
    TypeRef result;
  };

  using TypeBaseVector = std::vector<TypeBase*>;

  struct TypeExternalList : public TypeBase {
    struct External {
      const std::string* name;
      ComponentSort sort;
      TypeBase* type_base;
    };

    TypeExternalList(ComponentTypeDef type)
        : TypeBase(type) {
      assert(type == ComponentTypeDef::Instance
             || type == ComponentTypeDef::Component);
    }

    using ExternalVector = std::vector<External>;
    ExternalVector imports;
    ExternalVector exports;
  };

  struct TypeDefList : public TypeExternalList {
    TypeDefList(ComponentTypeDef type, TypeDefList* parent, uint8_t info = 0)
        : TypeExternalList(type), parent(parent) {
      assert(type == ComponentTypeDef::Instance
             || type == ComponentTypeDef::Component);
      info_bits = info;
    }

    TypeDefList* parent;

    // Sorts.
    TypeBaseVector types;
  };

  struct Component : public TypeDefList {
    Component(TypeDefList* parent)
        : TypeDefList(ComponentTypeDef::Component, parent, IsObject) {}

    // Sorts.
    TypeBaseVector instances;
    TypeBaseVector components;
  };

  Component* CurrentAsComponent() {
    assert(current_->type_def == ComponentTypeDef::Component &&
           (current_->info_bits & IsObject) != 0);
    return reinterpret_cast<Component*>(current_);
  }

  void UpdateTypeInfo(const TypeRef& type_ref) {
    if (type_ref.ref != nullptr) {
      current_->types.back()->info_bits |= type_ref.ref->info_bits;
    }
  }

  static TypeBaseVector* GetSort(TypeDefList* def_list, ComponentSort sort);
  Result CheckIndex(const Location& loc,
                    ComponentSort sort,
                    Index index,
                    CheckMode mode,
                    TypeBase** out_type);
  Result CheckIndex(ComponentSort sort,
                    const ComponentIndexLoc& index,
                    TypeBase** out_type);
  Result CheckIndex(const ComponentIndexLoc& index,
                    TypeRef* out_type_ref);
  Result CheckType(const ComponentTypeLoc& type,
                   CheckMode mode,
                   TypeRef* out_type_ref);
  Result CheckDefValType(const Location& loc, const TypeBase* type);
  Result CheckResource(const Location& loc, TypeBase** type);
  Result CheckBorrow(const Location& loc,
                     TypeRef* type_ref,
                     const char* desc);
  Result CheckCanonOptions(uint32_t option_count,
                           const ComponentCanonOption* options);

  Result CheckExternalInfo(const ComponentExternalInfo& external_info,
                           TypeBase** out_type_base);

  std::vector<std::unique_ptr<TypeBase>> objects_;
  TypeDefList* current_;
  Location current_loc_;
  uint32_t argument_count_;
  uint32_t not_found_count_;
  std::map<const std::string*, Index> caseful_names_;
  ValidateOptions options_;
  Errors* errors_;
  std::string_view filename_;
  std::vector<std::unique_ptr<std::string>> string_list_;
  wabt::Component::StringTable string_table_;
};

}  // namespace wabt

#endif  // WABT_SHARED_VALIDATOR_H_
