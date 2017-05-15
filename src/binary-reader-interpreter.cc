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

#include "binary-reader-interpreter.h"

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <vector>

#include "binary-error-handler.h"
#include "binary-reader-nop.h"
#include "interpreter.h"
#include "type-checker.h"
#include "writer.h"

#define CHECK_RESULT(expr)  \
  do {                      \
    if (WABT_FAILED(expr))  \
      return Result::Error; \
  } while (0)

namespace wabt {

namespace {

typedef std::vector<Index> IndexVector;
typedef std::vector<IstreamOffset> IstreamOffsetVector;
typedef std::vector<IstreamOffsetVector> IstreamOffsetVectorVector;

struct Label {
  Label(IstreamOffset offset, IstreamOffset fixup_offset);

  IstreamOffset offset;
  IstreamOffset fixup_offset;
};

Label::Label(IstreamOffset offset, IstreamOffset fixup_offset)
    : offset(offset), fixup_offset(fixup_offset) {}

struct ElemSegmentInfo {
  ElemSegmentInfo(Index* dst, Index func_index)
      : dst(dst), func_index(func_index) {}

  Index* dst;
  Index func_index;
};

struct DataSegmentInfo {
  DataSegmentInfo(void* dst_data, const void* src_data, IstreamOffset size)
      : dst_data(dst_data), src_data(src_data), size(size) {}

  void* dst_data;        // Not owned.
  const void* src_data;  // Not owned.
  IstreamOffset size;
};

class BinaryReaderInterpreter : public BinaryReaderNop {
 public:
  BinaryReaderInterpreter(InterpreterEnvironment* env,
                          DefinedInterpreterModule* module,
                          IstreamOffset istream_offset,
                          BinaryErrorHandler* error_handler);

  std::unique_ptr<OutputBuffer> ReleaseOutputBuffer();
  IstreamOffset get_istream_offset() { return istream_offset; }

  // Implement BinaryReader.
  bool OnError(const char* message) override;

  Result EndModule() override;

  Result OnTypeCount(Index count) override;
  Result OnType(Index index,
                Index param_count,
                Type* param_types,
                Index result_count,
                Type* result_types) override;

  Result OnImportCount(Index count) override;
  Result OnImport(Index index,
                  StringSlice module_name,
                  StringSlice field_name) override;
  Result OnImportFunc(Index import_index,
                      StringSlice module_name,
                      StringSlice field_name,
                      Index func_index,
                      Index sig_index) override;
  Result OnImportTable(Index import_index,
                       StringSlice module_name,
                       StringSlice field_name,
                       Index table_index,
                       Type elem_type,
                       const Limits* elem_limits) override;
  Result OnImportMemory(Index import_index,
                        StringSlice module_name,
                        StringSlice field_name,
                        Index memory_index,
                        const Limits* page_limits) override;
  Result OnImportGlobal(Index import_index,
                        StringSlice module_name,
                        StringSlice field_name,
                        Index global_index,
                        Type type,
                        bool mutable_) override;

  Result OnFunctionCount(Index count) override;
  Result OnFunction(Index index, Index sig_index) override;

  Result OnTable(Index index,
                 Type elem_type,
                 const Limits* elem_limits) override;

  Result OnMemory(Index index, const Limits* limits) override;

  Result OnGlobalCount(Index count) override;
  Result BeginGlobal(Index index, Type type, bool mutable_) override;
  Result EndGlobalInitExpr(Index index) override;

  Result OnExport(Index index,
                  ExternalKind kind,
                  Index item_index,
                  StringSlice name) override;

  Result OnStartFunction(Index func_index) override;

  Result BeginFunctionBody(Index index) override;
  Result OnLocalDeclCount(Index count) override;
  Result OnLocalDecl(Index decl_index, Index count, Type type) override;

  Result OnBinaryExpr(Opcode opcode) override;
  Result OnBlockExpr(Index num_types, Type* sig_types) override;
  Result OnBrExpr(Index depth) override;
  Result OnBrIfExpr(Index depth) override;
  Result OnBrTableExpr(Index num_targets,
                       Index* target_depths,
                       Index default_target_depth) override;
  Result OnCallExpr(Index func_index) override;
  Result OnCallIndirectExpr(Index sig_index) override;
  Result OnCompareExpr(Opcode opcode) override;
  Result OnConvertExpr(Opcode opcode) override;
  Result OnCurrentMemoryExpr() override;
  Result OnDropExpr() override;
  Result OnElseExpr() override;
  Result OnEndExpr() override;
  Result OnF32ConstExpr(uint32_t value_bits) override;
  Result OnF64ConstExpr(uint64_t value_bits) override;
  Result OnGetGlobalExpr(Index global_index) override;
  Result OnGetLocalExpr(Index local_index) override;
  Result OnGrowMemoryExpr() override;
  Result OnI32ConstExpr(uint32_t value) override;
  Result OnI64ConstExpr(uint64_t value) override;
  Result OnIfExpr(Index num_types, Type* sig_types) override;
  Result OnLoadExpr(Opcode opcode,
                    uint32_t alignment_log2,
                    Address offset) override;
  Result OnLoopExpr(Index num_types, Type* sig_types) override;
  Result OnNopExpr() override;
  Result OnReturnExpr() override;
  Result OnSelectExpr() override;
  Result OnSetGlobalExpr(Index global_index) override;
  Result OnSetLocalExpr(Index local_index) override;
  Result OnStoreExpr(Opcode opcode,
                     uint32_t alignment_log2,
                     Address offset) override;
  Result OnTeeLocalExpr(Index local_index) override;
  Result OnUnaryExpr(Opcode opcode) override;
  Result OnUnreachableExpr() override;
  Result EndFunctionBody(Index index) override;

  Result EndElemSegmentInitExpr(Index index) override;
  Result OnElemSegmentFunctionIndex(Index index,
                                    Index func_index) override;

  Result OnDataSegmentData(Index index,
                           const void* data,
                           Address size) override;

  Result OnInitExprF32ConstExpr(Index index, uint32_t value) override;
  Result OnInitExprF64ConstExpr(Index index, uint64_t value) override;
  Result OnInitExprGetGlobalExpr(Index index,
                                 Index global_index) override;
  Result OnInitExprI32ConstExpr(Index index, uint32_t value) override;
  Result OnInitExprI64ConstExpr(Index index, uint64_t value) override;

 private:
  Label* GetLabel(Index depth);
  Label* TopLabel();
  void PushLabel(IstreamOffset offset, IstreamOffset fixup_offset);
  void PopLabel();

  bool HandleError(Offset offset, const char* message);
  void PrintError(const char* format, ...);
  static void OnTypecheckerError(const char* msg, void* user_data);

  Index TranslateSigIndexToEnv(Index sig_index);
  InterpreterFuncSignature* GetSignatureByEnvIndex(Index sig_index);
  InterpreterFuncSignature* GetSignatureByModuleIndex(Index sig_index);
  Index TranslateFuncIndexToEnv(Index func_index);
  Index TranslateModuleFuncIndexToDefined(Index func_index);
  InterpreterFunc* GetFuncByEnvIndex(Index func_index);
  InterpreterFunc* GetFuncByModuleIndex(Index func_index);
  Index TranslateGlobalIndexToEnv(Index global_index);
  InterpreterGlobal* GetGlobalByEnvIndex(Index global_index);
  InterpreterGlobal* GetGlobalByModuleIndex(Index global_index);
  Type GetGlobalTypeByModuleIndex(Index global_index);
  Index TranslateLocalIndex(Index local_index);
  Type GetLocalTypeByIndex(InterpreterFunc* func, Index local_index);

  IstreamOffset GetIstreamOffset();

  Result EmitDataAt(IstreamOffset offset,
                    const void* data,
                    IstreamOffset size);
  Result EmitData(const void* data, IstreamOffset size);
  Result EmitOpcode(Opcode opcode);
  Result EmitOpcode(InterpreterOpcode opcode);
  Result EmitI8(uint8_t value);
  Result EmitI32(uint32_t value);
  Result EmitI64(uint64_t value);
  Result EmitI32At(IstreamOffset offset, uint32_t value);
  Result EmitDropKeep(uint32_t drop, uint8_t keep);
  Result AppendFixup(IstreamOffsetVectorVector* fixups_vector, Index index);
  Result EmitBrOffset(Index depth, IstreamOffset offset);
  Result GetBrDropKeepCount(Index depth,
                            Index* out_drop_count,
                            Index* out_keep_count);
  Result GetReturnDropKeepCount(Index* out_drop_count,
                                Index* out_keep_count);
  Result EmitBr(Index depth, Index drop_count, Index keep_count);
  Result EmitBrTableOffset(Index depth);
  Result FixupTopLabel();
  Result EmitFuncOffset(DefinedInterpreterFunc* func, Index func_index);

  Result CheckLocal(Index local_index);
  Result CheckGlobal(Index global_index);
  Result CheckImportKind(InterpreterImport* import, ExternalKind expected_kind);
  Result CheckImportLimits(const Limits* declared_limits,
                           const Limits* actual_limits);
  Result CheckHasMemory(Opcode opcode);
  Result CheckAlign(uint32_t alignment_log2, Address natural_alignment);

  Result AppendExport(InterpreterModule* module,
                      ExternalKind kind,
                      Index item_index,
                      StringSlice name);

  PrintErrorCallback MakePrintErrorCallback();
  static void OnHostImportPrintError(const char* msg, void* user_data);

  BinaryErrorHandler* error_handler = nullptr;
  InterpreterEnvironment* env = nullptr;
  DefinedInterpreterModule* module = nullptr;
  DefinedInterpreterFunc* current_func = nullptr;
  TypeCheckerErrorHandler tc_error_handler;
  TypeChecker typechecker;
  std::vector<Label> label_stack;
  IstreamOffsetVectorVector func_fixups;
  IstreamOffsetVectorVector depth_fixups;
  MemoryWriter istream_writer;
  IstreamOffset istream_offset = 0;
  /* mappings from module index space to env index space; this won't just be a
   * translation, because imported values will be resolved as well */
  IndexVector sig_index_mapping;
  IndexVector func_index_mapping;
  IndexVector global_index_mapping;

  Index num_func_imports = 0;
  Index num_global_imports = 0;

  // Changes to linear memory and tables should not apply if a validation error
  // occurs; these vectors cache the changes that must be applied after we know
  // that there are no validation errors.
  std::vector<ElemSegmentInfo> elem_segment_infos;
  std::vector<DataSegmentInfo> data_segment_infos;

  /* values cached so they can be shared between callbacks */
  InterpreterTypedValue init_expr_value;
  IstreamOffset table_offset = 0;
  bool is_host_import = false;
  HostInterpreterModule* host_import_module = nullptr;
  Index import_env_index = 0;
};

BinaryReaderInterpreter::BinaryReaderInterpreter(
    InterpreterEnvironment* env,
    DefinedInterpreterModule* module,
    IstreamOffset istream_offset,
    BinaryErrorHandler* error_handler)
    : error_handler(error_handler),
      env(env),
      module(module),
      istream_writer(std::move(env->istream)),
      istream_offset(istream_offset) {
  tc_error_handler.on_error = OnTypecheckerError;
  tc_error_handler.user_data = this;
  typechecker.error_handler = &tc_error_handler;
}

std::unique_ptr<OutputBuffer> BinaryReaderInterpreter::ReleaseOutputBuffer() {
  return istream_writer.ReleaseOutputBuffer();
}

Label* BinaryReaderInterpreter::GetLabel(Index depth) {
  assert(depth < label_stack.size());
  return &label_stack[label_stack.size() - depth - 1];
}

Label* BinaryReaderInterpreter::TopLabel() {
  return GetLabel(0);
}

bool BinaryReaderInterpreter::HandleError(Offset offset, const char* message) {
  return error_handler->OnError(offset, message);
}

void WABT_PRINTF_FORMAT(2, 3)
    BinaryReaderInterpreter::PrintError(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  HandleError(kInvalidOffset, buffer);
}

// static
void BinaryReaderInterpreter::OnTypecheckerError(const char* msg,
                                                 void* user_data) {
  static_cast<BinaryReaderInterpreter*>(user_data)->PrintError("%s", msg);
}

Index BinaryReaderInterpreter::TranslateSigIndexToEnv(Index sig_index) {
  assert(sig_index < sig_index_mapping.size());
  return sig_index_mapping[sig_index];
}

InterpreterFuncSignature* BinaryReaderInterpreter::GetSignatureByEnvIndex(
    Index sig_index) {
  return &env->sigs[sig_index];
}

InterpreterFuncSignature* BinaryReaderInterpreter::GetSignatureByModuleIndex(
    Index sig_index) {
  return GetSignatureByEnvIndex(TranslateSigIndexToEnv(sig_index));
}

Index BinaryReaderInterpreter::TranslateFuncIndexToEnv(Index func_index) {
  assert(func_index < func_index_mapping.size());
  return func_index_mapping[func_index];
}

Index BinaryReaderInterpreter::TranslateModuleFuncIndexToDefined(
    Index func_index) {
  assert(func_index >= num_func_imports);
  return func_index - num_func_imports;
}

InterpreterFunc* BinaryReaderInterpreter::GetFuncByEnvIndex(
    Index func_index) {
  return env->funcs[func_index].get();
}

InterpreterFunc* BinaryReaderInterpreter::GetFuncByModuleIndex(
    Index func_index) {
  return GetFuncByEnvIndex(TranslateFuncIndexToEnv(func_index));
}

Index BinaryReaderInterpreter::TranslateGlobalIndexToEnv(Index global_index) {
  return global_index_mapping[global_index];
}

InterpreterGlobal* BinaryReaderInterpreter::GetGlobalByEnvIndex(
    Index global_index) {
  return &env->globals[global_index];
}

InterpreterGlobal* BinaryReaderInterpreter::GetGlobalByModuleIndex(
    Index global_index) {
  return GetGlobalByEnvIndex(TranslateGlobalIndexToEnv(global_index));
}

Type BinaryReaderInterpreter::GetGlobalTypeByModuleIndex(Index global_index) {
  return GetGlobalByModuleIndex(global_index)->typed_value.type;
}

Type BinaryReaderInterpreter::GetLocalTypeByIndex(InterpreterFunc* func,
                                                  Index local_index) {
  assert(!func->is_host);
  return func->as_defined()->param_and_local_types[local_index];
}

IstreamOffset BinaryReaderInterpreter::GetIstreamOffset() {
  return istream_offset;
}

Result BinaryReaderInterpreter::EmitDataAt(IstreamOffset offset,
                                           const void* data,
                                           IstreamOffset size) {
  return istream_writer.WriteData(offset, data, size);
}

Result BinaryReaderInterpreter::EmitData(const void* data, IstreamOffset size) {
  CHECK_RESULT(EmitDataAt(istream_offset, data, size));
  istream_offset += size;
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitOpcode(Opcode opcode) {
  return EmitData(&opcode, sizeof(uint8_t));
}

Result BinaryReaderInterpreter::EmitOpcode(InterpreterOpcode opcode) {
  return EmitData(&opcode, sizeof(uint8_t));
}

Result BinaryReaderInterpreter::EmitI8(uint8_t value) {
  return EmitData(&value, sizeof(value));
}

Result BinaryReaderInterpreter::EmitI32(uint32_t value) {
  return EmitData(&value, sizeof(value));
}

Result BinaryReaderInterpreter::EmitI64(uint64_t value) {
  return EmitData(&value, sizeof(value));
}

Result BinaryReaderInterpreter::EmitI32At(IstreamOffset offset,
                                          uint32_t value) {
  return EmitDataAt(offset, &value, sizeof(value));
}

Result BinaryReaderInterpreter::EmitDropKeep(uint32_t drop, uint8_t keep) {
  assert(drop != UINT32_MAX);
  assert(keep <= 1);
  if (drop > 0) {
    if (drop == 1 && keep == 0) {
      CHECK_RESULT(EmitOpcode(InterpreterOpcode::Drop));
    } else {
      CHECK_RESULT(EmitOpcode(InterpreterOpcode::DropKeep));
      CHECK_RESULT(EmitI32(drop));
      CHECK_RESULT(EmitI8(keep));
    }
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::AppendFixup(
    IstreamOffsetVectorVector* fixups_vector,
    Index index) {
  if (index >= fixups_vector->size())
    fixups_vector->resize(index + 1);
  (*fixups_vector)[index].push_back(GetIstreamOffset());
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBrOffset(Index depth,
                                             IstreamOffset offset) {
  if (offset == kInvalidIstreamOffset) {
    /* depth_fixups stores the depth counting up from zero, where zero is the
     * top-level function scope. */
    depth = label_stack.size() - 1 - depth;
    CHECK_RESULT(AppendFixup(&depth_fixups, depth));
  }
  CHECK_RESULT(EmitI32(offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::GetBrDropKeepCount(Index depth,
                                                   Index* out_drop_count,
                                                   Index* out_keep_count) {
  TypeCheckerLabel* label;
  CHECK_RESULT(typechecker_get_label(&typechecker, depth, &label));
  *out_keep_count =
      label->label_type != LabelType::Loop ? label->sig.size() : 0;
  if (typechecker_is_unreachable(&typechecker)) {
    *out_drop_count = 0;
  } else {
    *out_drop_count =
        (typechecker.type_stack.size() - label->type_stack_limit) -
        *out_keep_count;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::GetReturnDropKeepCount(Index* out_drop_count,
                                                       Index* out_keep_count) {
  if (WABT_FAILED(GetBrDropKeepCount(label_stack.size() - 1, out_drop_count,
                                     out_keep_count))) {
    return Result::Error;
  }

  *out_drop_count += current_func->param_and_local_types.size();
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBr(Index depth,
                                       Index drop_count,
                                       Index keep_count) {
  CHECK_RESULT(EmitDropKeep(drop_count, keep_count));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Br));
  CHECK_RESULT(EmitBrOffset(depth, GetLabel(depth)->offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBrTableOffset(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  CHECK_RESULT(EmitBrOffset(depth, GetLabel(depth)->offset));
  CHECK_RESULT(EmitI32(drop_count));
  CHECK_RESULT(EmitI8(keep_count));
  return Result::Ok;
}

Result BinaryReaderInterpreter::FixupTopLabel() {
  IstreamOffset offset = GetIstreamOffset();
  Index top = label_stack.size() - 1;
  if (top >= depth_fixups.size()) {
    /* nothing to fixup */
    return Result::Ok;
  }

  IstreamOffsetVector& fixups = depth_fixups[top];
  for (IstreamOffset fixup : fixups)
    CHECK_RESULT(EmitI32At(fixup, offset));
  fixups.clear();
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitFuncOffset(DefinedInterpreterFunc* func,
                                               Index func_index) {
  if (func->offset == kInvalidIstreamOffset) {
    Index defined_index = TranslateModuleFuncIndexToDefined(func_index);
    CHECK_RESULT(AppendFixup(&func_fixups, defined_index));
  }
  CHECK_RESULT(EmitI32(func->offset));
  return Result::Ok;
}

bool BinaryReaderInterpreter::OnError(const char* message) {
  return HandleError(state->offset, message);
}

Result BinaryReaderInterpreter::OnTypeCount(Index count) {
  sig_index_mapping.resize(count);
  for (Index i = 0; i < count; ++i)
    sig_index_mapping[i] = env->sigs.size() + i;
  env->sigs.resize(env->sigs.size() + count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnType(Index index,
                                       Index param_count,
                                       Type* param_types,
                                       Index result_count,
                                       Type* result_types) {
  InterpreterFuncSignature* sig = GetSignatureByModuleIndex(index);
  sig->param_types.insert(sig->param_types.end(), param_types,
                          param_types + param_count);
  sig->result_types.insert(sig->result_types.end(), result_types,
                           result_types + result_count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImportCount(Index count) {
  module->imports.resize(count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImport(Index index,
                                         StringSlice module_name,
                                         StringSlice field_name) {
  InterpreterImport* import = &module->imports[index];
  import->module_name = dup_string_slice(module_name);
  import->field_name = dup_string_slice(field_name);
  int module_index =
      env->registered_module_bindings.find_index(import->module_name);
  if (module_index < 0) {
    PrintError("unknown import module \"" PRIstringslice "\"",
               WABT_PRINTF_STRING_SLICE_ARG(import->module_name));
    return Result::Error;
  }

  InterpreterModule* module = env->modules[module_index].get();
  if (module->is_host) {
    /* We don't yet know the kind of a host import module, so just assume it
     * exists for now. We'll fail later (in on_import_* below) if it doesn't
     * exist). */
    is_host_import = true;
    host_import_module = module->as_host();
  } else {
    InterpreterExport* export_ =
        get_interpreter_export_by_name(module, &import->field_name);
    if (!export_) {
      PrintError("unknown module field \"" PRIstringslice "\"",
                 WABT_PRINTF_STRING_SLICE_ARG(import->field_name));
      return Result::Error;
    }

    import->kind = export_->kind;
    is_host_import = false;
    import_env_index = export_->index;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckLocal(Index local_index) {
  Index max_local_index = current_func->param_and_local_types.size();
  if (local_index >= max_local_index) {
    PrintError("invalid local_index: %" PRIindex " (max %" PRIindex ")",
               local_index, max_local_index);
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckGlobal(Index global_index) {
  Index max_global_index = global_index_mapping.size();
  if (global_index >= max_global_index) {
    PrintError("invalid global_index: %" PRIindex " (max %" PRIindex ")",
               global_index, max_global_index);
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckImportKind(InterpreterImport* import,
                                                ExternalKind expected_kind) {
  if (import->kind != expected_kind) {
    PrintError("expected import \"" PRIstringslice "." PRIstringslice
               "\" to have kind %s, not %s",
               WABT_PRINTF_STRING_SLICE_ARG(import->module_name),
               WABT_PRINTF_STRING_SLICE_ARG(import->field_name),
               get_kind_name(expected_kind), get_kind_name(import->kind));
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckImportLimits(const Limits* declared_limits,
                                                  const Limits* actual_limits) {
  if (actual_limits->initial < declared_limits->initial) {
    PrintError("actual size (%" PRIu64 ") smaller than declared (%" PRIu64 ")",
               actual_limits->initial, declared_limits->initial);
    return Result::Error;
  }

  if (declared_limits->has_max) {
    if (!actual_limits->has_max) {
      PrintError("max size (unspecified) larger than declared (%" PRIu64 ")",
                 declared_limits->max);
      return Result::Error;
    } else if (actual_limits->max > declared_limits->max) {
      PrintError("max size (%" PRIu64 ") larger than declared (%" PRIu64 ")",
                 actual_limits->max, declared_limits->max);
      return Result::Error;
    }
  }

  return Result::Ok;
}

Result BinaryReaderInterpreter::AppendExport(InterpreterModule* module,
                                             ExternalKind kind,
                                             Index item_index,
                                             StringSlice name) {
  if (module->export_bindings.find_index(name) != -1) {
    PrintError("duplicate export \"" PRIstringslice "\"",
               WABT_PRINTF_STRING_SLICE_ARG(name));
    return Result::Error;
  }

  module->exports.emplace_back(dup_string_slice(name), kind, item_index);
  InterpreterExport* export_ = &module->exports.back();

  module->export_bindings.emplace(string_slice_to_string(export_->name),
                                  Binding(module->exports.size() - 1));
  return Result::Ok;
}

// static
void BinaryReaderInterpreter::OnHostImportPrintError(const char* msg,
                                                     void* user_data) {
  static_cast<BinaryReaderInterpreter*>(user_data)->PrintError("%s", msg);
}

PrintErrorCallback BinaryReaderInterpreter::MakePrintErrorCallback() {
  PrintErrorCallback result;
  result.print_error = OnHostImportPrintError;
  result.user_data = this;
  return result;
}

Result BinaryReaderInterpreter::OnImportFunc(Index import_index,
                                             StringSlice module_name,
                                             StringSlice field_name,
                                             Index func_index,
                                             Index sig_index) {
  InterpreterImport* import = &module->imports[import_index];
  import->func.sig_index = TranslateSigIndexToEnv(sig_index);

  Index func_env_index;
  if (is_host_import) {
    HostInterpreterFunc* func = new HostInterpreterFunc(
        import->module_name, import->field_name, import->func.sig_index);

    env->funcs.emplace_back(func);

    InterpreterHostImportDelegate* host_delegate =
        &host_import_module->import_delegate;
    InterpreterFuncSignature* sig = &env->sigs[func->sig_index];
    CHECK_RESULT(host_delegate->import_func(
        import, func, sig, MakePrintErrorCallback(), host_delegate->user_data));
    assert(func->callback);

    func_env_index = env->funcs.size() - 1;
    AppendExport(host_import_module, ExternalKind::Func, func_env_index,
                 import->field_name);
  } else {
    CHECK_RESULT(CheckImportKind(import, ExternalKind::Func));
    InterpreterFunc* func = env->funcs[import_env_index].get();
    if (!func_signatures_are_equal(env, import->func.sig_index,
                                   func->sig_index)) {
      PrintError("import signature mismatch");
      return Result::Error;
    }

    func_env_index = import_env_index;
  }
  func_index_mapping.push_back(func_env_index);
  num_func_imports++;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImportTable(Index import_index,
                                              StringSlice module_name,
                                              StringSlice field_name,
                                              Index table_index,
                                              Type elem_type,
                                              const Limits* elem_limits) {
  if (module->table_index != kInvalidIndex) {
    PrintError("only one table allowed");
    return Result::Error;
  }

  InterpreterImport* import = &module->imports[import_index];

  if (is_host_import) {
    env->tables.emplace_back(*elem_limits);
    InterpreterTable* table = &env->tables.back();

    InterpreterHostImportDelegate* host_delegate =
        &host_import_module->import_delegate;
    CHECK_RESULT(host_delegate->import_table(
        import, table, MakePrintErrorCallback(), host_delegate->user_data));

    CHECK_RESULT(CheckImportLimits(elem_limits, &table->limits));

    module->table_index = env->tables.size() - 1;
    AppendExport(host_import_module, ExternalKind::Table, module->table_index,
                 import->field_name);
  } else {
    CHECK_RESULT(CheckImportKind(import, ExternalKind::Table));
    InterpreterTable* table = &env->tables[import_env_index];
    CHECK_RESULT(CheckImportLimits(elem_limits, &table->limits));

    import->table.limits = *elem_limits;
    module->table_index = import_env_index;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImportMemory(Index import_index,
                                               StringSlice module_name,
                                               StringSlice field_name,
                                               Index memory_index,
                                               const Limits* page_limits) {
  if (module->memory_index != kInvalidIndex) {
    PrintError("only one memory allowed");
    return Result::Error;
  }

  InterpreterImport* import = &module->imports[import_index];

  if (is_host_import) {
    env->memories.emplace_back();
    InterpreterMemory* memory = &env->memories.back();

    InterpreterHostImportDelegate* host_delegate =
        &host_import_module->import_delegate;
    CHECK_RESULT(host_delegate->import_memory(
        import, memory, MakePrintErrorCallback(), host_delegate->user_data));

    CHECK_RESULT(CheckImportLimits(page_limits, &memory->page_limits));

    module->memory_index = env->memories.size() - 1;
    AppendExport(host_import_module, ExternalKind::Memory, module->memory_index,
                 import->field_name);
  } else {
    CHECK_RESULT(CheckImportKind(import, ExternalKind::Memory));
    InterpreterMemory* memory = &env->memories[import_env_index];
    CHECK_RESULT(CheckImportLimits(page_limits, &memory->page_limits));

    import->memory.limits = *page_limits;
    module->memory_index = import_env_index;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImportGlobal(Index import_index,
                                               StringSlice module_name,
                                               StringSlice field_name,
                                               Index global_index,
                                               Type type,
                                               bool mutable_) {
  InterpreterImport* import = &module->imports[import_index];

  Index global_env_index = env->globals.size() - 1;
  if (is_host_import) {
    env->globals.emplace_back(InterpreterTypedValue(type), mutable_);
    InterpreterGlobal* global = &env->globals.back();

    InterpreterHostImportDelegate* host_delegate =
        &host_import_module->import_delegate;
    CHECK_RESULT(host_delegate->import_global(
        import, global, MakePrintErrorCallback(), host_delegate->user_data));

    global_env_index = env->globals.size() - 1;
    AppendExport(host_import_module, ExternalKind::Global, global_env_index,
                 import->field_name);
  } else {
    CHECK_RESULT(CheckImportKind(import, ExternalKind::Global));
    // TODO: check type and mutability
    import->global.type = type;
    import->global.mutable_ = mutable_;
    global_env_index = import_env_index;
  }
  global_index_mapping.push_back(global_env_index);
  num_global_imports++;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnFunctionCount(Index count) {
  for (Index i = 0; i < count; ++i)
    func_index_mapping.push_back(env->funcs.size() + i);
  env->funcs.reserve(env->funcs.size() + count);
  func_fixups.resize(count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnFunction(Index index, Index sig_index) {
  DefinedInterpreterFunc* func =
      new DefinedInterpreterFunc(TranslateSigIndexToEnv(sig_index));
  env->funcs.emplace_back(func);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnTable(Index index,
                                        Type elem_type,
                                        const Limits* elem_limits) {
  if (module->table_index != kInvalidIndex) {
    PrintError("only one table allowed");
    return Result::Error;
  }
  env->tables.emplace_back(*elem_limits);
  module->table_index = env->tables.size() - 1;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnMemory(Index index,
                                         const Limits* page_limits) {
  if (module->memory_index != kInvalidIndex) {
    PrintError("only one memory allowed");
    return Result::Error;
  }
  env->memories.emplace_back(*page_limits);
  module->memory_index = env->memories.size() - 1;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnGlobalCount(Index count) {
  for (Index i = 0; i < count; ++i)
    global_index_mapping.push_back(env->globals.size() + i);
  env->globals.resize(env->globals.size() + count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::BeginGlobal(Index index,
                                            Type type,
                                            bool mutable_) {
  InterpreterGlobal* global = GetGlobalByModuleIndex(index);
  global->typed_value.type = type;
  global->mutable_ = mutable_;
  init_expr_value.type = Type::Void;
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndGlobalInitExpr(Index index) {
  InterpreterGlobal* global = GetGlobalByModuleIndex(index);
  if (init_expr_value.type != global->typed_value.type) {
    PrintError("type mismatch in global, expected %s but got %s.",
               get_type_name(global->typed_value.type),
               get_type_name(init_expr_value.type));
    return Result::Error;
  }
  global->typed_value = init_expr_value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprF32ConstExpr(Index index,
                                                       uint32_t value_bits) {
  init_expr_value.type = Type::F32;
  init_expr_value.value.f32_bits = value_bits;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprF64ConstExpr(Index index,
                                                       uint64_t value_bits) {
  init_expr_value.type = Type::F64;
  init_expr_value.value.f64_bits = value_bits;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprGetGlobalExpr(Index index,
                                                        Index global_index) {
  if (global_index >= num_global_imports) {
    PrintError("initializer expression can only reference an imported global");
    return Result::Error;
  }
  InterpreterGlobal* ref_global = GetGlobalByModuleIndex(global_index);
  if (ref_global->mutable_) {
    PrintError("initializer expression cannot reference a mutable global");
    return Result::Error;
  }
  init_expr_value = ref_global->typed_value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprI32ConstExpr(Index index,
                                                       uint32_t value) {
  init_expr_value.type = Type::I32;
  init_expr_value.value.i32 = value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprI64ConstExpr(Index index,
                                                       uint64_t value) {
  init_expr_value.type = Type::I64;
  init_expr_value.value.i64 = value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnExport(Index index,
                                         ExternalKind kind,
                                         Index item_index,
                                         StringSlice name) {
  switch (kind) {
    case ExternalKind::Func:
      item_index = TranslateFuncIndexToEnv(item_index);
      break;

    case ExternalKind::Table:
      item_index = module->table_index;
      break;

    case ExternalKind::Memory:
      item_index = module->memory_index;
      break;

    case ExternalKind::Global: {
      item_index = TranslateGlobalIndexToEnv(item_index);
      InterpreterGlobal* global = &env->globals[item_index];
      if (global->mutable_) {
        PrintError("mutable globals cannot be exported");
        return Result::Error;
      }
      break;
    }
  }
  return AppendExport(module, kind, item_index, name);
}

Result BinaryReaderInterpreter::OnStartFunction(Index func_index) {
  Index start_func_index = TranslateFuncIndexToEnv(func_index);
  InterpreterFunc* start_func = GetFuncByEnvIndex(start_func_index);
  InterpreterFuncSignature* sig = GetSignatureByEnvIndex(start_func->sig_index);
  if (sig->param_types.size() != 0) {
    PrintError("start function must be nullary");
    return Result::Error;
  }
  if (sig->result_types.size() != 0) {
    PrintError("start function must not return anything");
    return Result::Error;
  }
  module->start_func_index = start_func_index;
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndElemSegmentInitExpr(Index index) {
  if (init_expr_value.type != Type::I32) {
    PrintError("type mismatch in elem segment, expected i32 but got %s",
               get_type_name(init_expr_value.type));
    return Result::Error;
  }
  table_offset = init_expr_value.value.i32;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnElemSegmentFunctionIndex(Index index,
                                                           Index func_index) {
  assert(module->table_index != kInvalidIndex);
  InterpreterTable* table = &env->tables[module->table_index];
  if (table_offset >= table->func_indexes.size()) {
    PrintError("elem segment offset is out of bounds: %u >= max value %" PRIzd,
               table_offset, table->func_indexes.size());
    return Result::Error;
  }

  Index max_func_index = func_index_mapping.size();
  if (func_index >= max_func_index) {
    PrintError("invalid func_index: %" PRIindex " (max %" PRIindex ")",
               func_index, max_func_index);
    return Result::Error;
  }

  elem_segment_infos.emplace_back(&table->func_indexes[table_offset++],
                                  TranslateFuncIndexToEnv(func_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnDataSegmentData(Index index,
                                                  const void* src_data,
                                                  Address size) {
  assert(module->memory_index != kInvalidIndex);
  InterpreterMemory* memory = &env->memories[module->memory_index];
  if (init_expr_value.type != Type::I32) {
    PrintError("type mismatch in data segment, expected i32 but got %s",
               get_type_name(init_expr_value.type));
    return Result::Error;
  }
  Address address = init_expr_value.value.i32;
  uint64_t end_address =
      static_cast<uint64_t>(address) + static_cast<uint64_t>(size);
  if (end_address > memory->data.size()) {
    PrintError("data segment is out of bounds: [%" PRIaddress ", %" PRIu64
               ") >= max value %" PRIzd,
               address, end_address, memory->data.size());
    return Result::Error;
  }

  if (size > 0)
    data_segment_infos.emplace_back(&memory->data[address], src_data, size);

  return Result::Ok;
}

void BinaryReaderInterpreter::PushLabel(IstreamOffset offset,
                                        IstreamOffset fixup_offset) {
  label_stack.emplace_back(offset, fixup_offset);
}

void BinaryReaderInterpreter::PopLabel() {
  label_stack.pop_back();
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * label_stack so only do it conditionally. */
  if (depth_fixups.size() > label_stack.size()) {
    depth_fixups.erase(depth_fixups.begin() + label_stack.size(),
                       depth_fixups.end());
  }
}

Result BinaryReaderInterpreter::BeginFunctionBody(Index index) {
  DefinedInterpreterFunc* func = GetFuncByModuleIndex(index)->as_defined();
  InterpreterFuncSignature* sig = GetSignatureByEnvIndex(func->sig_index);

  func->offset = GetIstreamOffset();
  func->local_decl_count = 0;
  func->local_count = 0;

  current_func = func;
  depth_fixups.clear();
  label_stack.clear();

  /* fixup function references */
  Index defined_index = TranslateModuleFuncIndexToDefined(index);
  IstreamOffsetVector& fixups = func_fixups[defined_index];
  for (IstreamOffset fixup : fixups)
    CHECK_RESULT(EmitI32At(fixup, func->offset));

  /* append param types */
  for (Type param_type : sig->param_types)
    func->param_and_local_types.push_back(param_type);

  CHECK_RESULT(typechecker_begin_function(&typechecker, &sig->result_types));

  /* push implicit func label (equivalent to return) */
  PushLabel(kInvalidIstreamOffset, kInvalidIstreamOffset);
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndFunctionBody(Index index) {
  FixupTopLabel();
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(typechecker_end_function(&typechecker));
  CHECK_RESULT(EmitDropKeep(drop_count, keep_count));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Return));
  PopLabel();
  current_func = nullptr;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLocalDeclCount(Index count) {
  current_func->local_decl_count = count;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLocalDecl(Index decl_index,
                                            Index count,
                                            Type type) {
  current_func->local_count += count;

  for (Index i = 0; i < count; ++i)
    current_func->param_and_local_types.push_back(type);

  if (decl_index == current_func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(EmitOpcode(InterpreterOpcode::Alloca));
    CHECK_RESULT(EmitI32(current_func->local_count));
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckHasMemory(Opcode opcode) {
  if (module->memory_index == kInvalidIndex) {
    PrintError("%s requires an imported or defined memory.",
               get_opcode_name(opcode));
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckAlign(uint32_t alignment_log2,
                                           Address natural_alignment) {
  if (alignment_log2 >= 32 || (1U << alignment_log2) > natural_alignment) {
    PrintError("alignment must not be larger than natural alignment (%u)",
               natural_alignment);
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnUnaryExpr(Opcode opcode) {
  CHECK_RESULT(typechecker_on_unary(&typechecker, opcode));
  CHECK_RESULT(EmitOpcode(opcode));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBinaryExpr(Opcode opcode) {
  CHECK_RESULT(typechecker_on_binary(&typechecker, opcode));
  CHECK_RESULT(EmitOpcode(opcode));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBlockExpr(Index num_types, Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_block(&typechecker, &sig));
  PushLabel(kInvalidIstreamOffset, kInvalidIstreamOffset);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLoopExpr(Index num_types, Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_loop(&typechecker, &sig));
  PushLabel(GetIstreamOffset(), kInvalidIstreamOffset);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnIfExpr(Index num_types, Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_if(&typechecker, &sig));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrUnless));
  IstreamOffset fixup_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(kInvalidIstreamOffset));
  PushLabel(kInvalidIstreamOffset, fixup_offset);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnElseExpr() {
  CHECK_RESULT(typechecker_on_else(&typechecker));
  Label* label = TopLabel();
  IstreamOffset fixup_cond_offset = label->fixup_offset;
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Br));
  label->fixup_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(kInvalidIstreamOffset));
  CHECK_RESULT(EmitI32At(fixup_cond_offset, GetIstreamOffset()));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnEndExpr() {
  TypeCheckerLabel* label;
  CHECK_RESULT(typechecker_get_label(&typechecker, 0, &label));
  LabelType label_type = label->label_type;
  CHECK_RESULT(typechecker_on_end(&typechecker));
  if (label_type == LabelType::If || label_type == LabelType::Else) {
    CHECK_RESULT(EmitI32At(TopLabel()->fixup_offset, GetIstreamOffset()));
  }
  FixupTopLabel();
  PopLabel();
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBrExpr(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  CHECK_RESULT(typechecker_on_br(&typechecker, depth));
  CHECK_RESULT(EmitBr(depth, drop_count, keep_count));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBrIfExpr(Index depth) {
  Index drop_count, keep_count;
  CHECK_RESULT(typechecker_on_br_if(&typechecker, depth));
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  /* flip the br_if so if <cond> is true it can drop values from the stack */
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrUnless));
  IstreamOffset fixup_br_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(kInvalidIstreamOffset));
  CHECK_RESULT(EmitBr(depth, drop_count, keep_count));
  CHECK_RESULT(EmitI32At(fixup_br_offset, GetIstreamOffset()));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBrTableExpr(Index num_targets,
                                              Index* target_depths,
                                              Index default_target_depth) {
  CHECK_RESULT(typechecker_begin_br_table(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrTable));
  CHECK_RESULT(EmitI32(num_targets));
  IstreamOffset fixup_table_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(kInvalidIstreamOffset));
  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Data));
  CHECK_RESULT(EmitI32((num_targets + 1) * WABT_TABLE_ENTRY_SIZE));
  CHECK_RESULT(EmitI32At(fixup_table_offset, GetIstreamOffset()));

  for (Index i = 0; i <= num_targets; ++i) {
    Index depth = i != num_targets ? target_depths[i] : default_target_depth;
    CHECK_RESULT(typechecker_on_br_table_target(&typechecker, depth));
    CHECK_RESULT(EmitBrTableOffset(depth));
  }

  CHECK_RESULT(typechecker_end_br_table(&typechecker));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnCallExpr(Index func_index) {
  InterpreterFunc* func = GetFuncByModuleIndex(func_index);
  InterpreterFuncSignature* sig = GetSignatureByEnvIndex(func->sig_index);
  CHECK_RESULT(
      typechecker_on_call(&typechecker, &sig->param_types, &sig->result_types));

  if (func->is_host) {
    CHECK_RESULT(EmitOpcode(InterpreterOpcode::CallHost));
    CHECK_RESULT(EmitI32(TranslateFuncIndexToEnv(func_index)));
  } else {
    CHECK_RESULT(EmitOpcode(InterpreterOpcode::Call));
    CHECK_RESULT(EmitFuncOffset(func->as_defined(), func_index));
  }

  return Result::Ok;
}

Result BinaryReaderInterpreter::OnCallIndirectExpr(Index sig_index) {
  if (module->table_index == kInvalidIndex) {
    PrintError("found call_indirect operator, but no table");
    return Result::Error;
  }
  InterpreterFuncSignature* sig = GetSignatureByModuleIndex(sig_index);
  CHECK_RESULT(typechecker_on_call_indirect(&typechecker, &sig->param_types,
                                            &sig->result_types));

  CHECK_RESULT(EmitOpcode(InterpreterOpcode::CallIndirect));
  CHECK_RESULT(EmitI32(module->table_index));
  CHECK_RESULT(EmitI32(TranslateSigIndexToEnv(sig_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnCompareExpr(Opcode opcode) {
  return OnBinaryExpr(opcode);
}

Result BinaryReaderInterpreter::OnConvertExpr(Opcode opcode) {
  return OnUnaryExpr(opcode);
}

Result BinaryReaderInterpreter::OnDropExpr() {
  CHECK_RESULT(typechecker_on_drop(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Drop));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnI32ConstExpr(uint32_t value) {
  CHECK_RESULT(typechecker_on_const(&typechecker, Type::I32));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::I32Const));
  CHECK_RESULT(EmitI32(value));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnI64ConstExpr(uint64_t value) {
  CHECK_RESULT(typechecker_on_const(&typechecker, Type::I64));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::I64Const));
  CHECK_RESULT(EmitI64(value));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnF32ConstExpr(uint32_t value_bits) {
  CHECK_RESULT(typechecker_on_const(&typechecker, Type::F32));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::F32Const));
  CHECK_RESULT(EmitI32(value_bits));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnF64ConstExpr(uint64_t value_bits) {
  CHECK_RESULT(typechecker_on_const(&typechecker, Type::F64));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::F64Const));
  CHECK_RESULT(EmitI64(value_bits));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnGetGlobalExpr(Index global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  Type type = GetGlobalTypeByModuleIndex(global_index);
  CHECK_RESULT(typechecker_on_get_global(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::GetGlobal));
  CHECK_RESULT(EmitI32(TranslateGlobalIndexToEnv(global_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnSetGlobalExpr(Index global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  InterpreterGlobal* global = GetGlobalByModuleIndex(global_index);
  if (!global->mutable_) {
    PrintError("can't set_global on immutable global at index %" PRIindex ".",
               global_index);
    return Result::Error;
  }
  CHECK_RESULT(
      typechecker_on_set_global(&typechecker, global->typed_value.type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::SetGlobal));
  CHECK_RESULT(EmitI32(TranslateGlobalIndexToEnv(global_index)));
  return Result::Ok;
}

Index BinaryReaderInterpreter::TranslateLocalIndex(Index local_index) {
  return typechecker.type_stack.size() +
         current_func->param_and_local_types.size() - local_index;
}

Result BinaryReaderInterpreter::OnGetLocalExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  Type type = GetLocalTypeByIndex(current_func, local_index);
  /* Get the translated index before calling typechecker_on_get_local
   * because it will update the type stack size. We need the index to be
   * relative to the old stack size. */
  Index translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(typechecker_on_get_local(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::GetLocal));
  CHECK_RESULT(EmitI32(translated_local_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnSetLocalExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  Type type = GetLocalTypeByIndex(current_func, local_index);
  CHECK_RESULT(typechecker_on_set_local(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::SetLocal));
  CHECK_RESULT(EmitI32(TranslateLocalIndex(local_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnTeeLocalExpr(Index local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  Type type = GetLocalTypeByIndex(current_func, local_index);
  CHECK_RESULT(typechecker_on_tee_local(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::TeeLocal));
  CHECK_RESULT(EmitI32(TranslateLocalIndex(local_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnGrowMemoryExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::GrowMemory));
  CHECK_RESULT(typechecker_on_grow_memory(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::GrowMemory));
  CHECK_RESULT(EmitI32(module->memory_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLoadExpr(Opcode opcode,
                                           uint32_t alignment_log2,
                                           Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAlign(alignment_log2, get_opcode_memory_size(opcode)));
  CHECK_RESULT(typechecker_on_load(&typechecker, opcode));
  CHECK_RESULT(EmitOpcode(opcode));
  CHECK_RESULT(EmitI32(module->memory_index));
  CHECK_RESULT(EmitI32(offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnStoreExpr(Opcode opcode,
                                            uint32_t alignment_log2,
                                            Address offset) {
  CHECK_RESULT(CheckHasMemory(opcode));
  CHECK_RESULT(CheckAlign(alignment_log2, get_opcode_memory_size(opcode)));
  CHECK_RESULT(typechecker_on_store(&typechecker, opcode));
  CHECK_RESULT(EmitOpcode(opcode));
  CHECK_RESULT(EmitI32(module->memory_index));
  CHECK_RESULT(EmitI32(offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnCurrentMemoryExpr() {
  CHECK_RESULT(CheckHasMemory(Opcode::CurrentMemory));
  CHECK_RESULT(typechecker_on_current_memory(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::CurrentMemory));
  CHECK_RESULT(EmitI32(module->memory_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnNopExpr() {
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnReturnExpr() {
  Index drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(typechecker_on_return(&typechecker));
  CHECK_RESULT(EmitDropKeep(drop_count, keep_count));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Return));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnSelectExpr() {
  CHECK_RESULT(typechecker_on_select(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Select));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnUnreachableExpr() {
  CHECK_RESULT(typechecker_on_unreachable(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Unreachable));
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndModule() {
  for (ElemSegmentInfo& info : elem_segment_infos) {
    *info.dst = info.func_index;
  }
  for (DataSegmentInfo& info : data_segment_infos) {
    memcpy(info.dst_data, info.src_data, info.size);
  }
  return Result::Ok;
}

}  // namespace

Result read_binary_interpreter(InterpreterEnvironment* env,
                               const void* data,
                               size_t size,
                               const ReadBinaryOptions* options,
                               BinaryErrorHandler* error_handler,
                               DefinedInterpreterModule** out_module) {
  IstreamOffset istream_offset = env->istream->data.size();
  DefinedInterpreterModule* module =
      new DefinedInterpreterModule(istream_offset);

  // Need to mark before constructing the reader since it takes ownership of
  // env->istream, which makes env->istream == nullptr.
  InterpreterEnvironmentMark mark = mark_interpreter_environment(env);
  BinaryReaderInterpreter reader(env, module, istream_offset, error_handler);
  env->modules.emplace_back(module);

  Result result = read_binary(data, size, &reader, options);
  env->istream = reader.ReleaseOutputBuffer();
  if (WABT_SUCCEEDED(result)) {
    env->istream->data.resize(reader.get_istream_offset());
    module->istream_end = env->istream->data.size();
    *out_module = module;
  } else {
    reset_interpreter_environment_to_mark(env, mark);
    *out_module = nullptr;
  }
  return result;
}

}  // namespace wabt
