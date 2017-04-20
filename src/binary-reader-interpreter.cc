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

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

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

typedef std::vector<uint32_t> Uint32Vector;
typedef std::vector<Uint32Vector> Uint32VectorVector;

struct Label {
  Label(uint32_t offset, uint32_t fixup_offset);

  uint32_t offset; /* branch location in the istream */
  uint32_t fixup_offset;
};

Label::Label(uint32_t offset, uint32_t fixup_offset)
    : offset(offset), fixup_offset(fixup_offset) {}

struct ElemSegmentInfo {
  ElemSegmentInfo(uint32_t* dst, uint32_t func_index)
      : dst(dst), func_index(func_index) {}

  uint32_t* dst;
  uint32_t func_index;
};

struct DataSegmentInfo {
  DataSegmentInfo(void* dst_data, const void* src_data, uint32_t size)
      : dst_data(dst_data), src_data(src_data), size(size) {}

  void* dst_data;        // Not owned.
  const void* src_data;  // Not owned.
  uint32_t size;
};

class BinaryReaderInterpreter : public BinaryReaderNop {
 public:
  BinaryReaderInterpreter(InterpreterEnvironment* env,
                          DefinedInterpreterModule* module,
                          size_t istream_offset,
                          BinaryErrorHandler* error_handler);

  std::unique_ptr<OutputBuffer> ReleaseOutputBuffer();
  size_t get_istream_offset() { return istream_offset; }

  // Implement BinaryReader.
  virtual bool OnError(const char* message);

  virtual Result EndModule();

  virtual Result OnTypeCount(uint32_t count);
  virtual Result OnType(uint32_t index,
                        uint32_t param_count,
                        Type* param_types,
                        uint32_t result_count,
                        Type* result_types);

  virtual Result OnImportCount(uint32_t count);
  virtual Result OnImport(uint32_t index,
                          StringSlice module_name,
                          StringSlice field_name);
  virtual Result OnImportFunc(uint32_t import_index,
                              StringSlice module_name,
                              StringSlice field_name,
                              uint32_t func_index,
                              uint32_t sig_index);
  virtual Result OnImportTable(uint32_t import_index,
                               StringSlice module_name,
                               StringSlice field_name,
                               uint32_t table_index,
                               Type elem_type,
                               const Limits* elem_limits);
  virtual Result OnImportMemory(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t memory_index,
                                const Limits* page_limits);
  virtual Result OnImportGlobal(uint32_t import_index,
                                StringSlice module_name,
                                StringSlice field_name,
                                uint32_t global_index,
                                Type type,
                                bool mutable_);

  virtual Result OnFunctionCount(uint32_t count);
  virtual Result OnFunction(uint32_t index, uint32_t sig_index);

  virtual Result OnTable(uint32_t index,
                         Type elem_type,
                         const Limits* elem_limits);

  virtual Result OnMemory(uint32_t index, const Limits* limits);

  virtual Result OnGlobalCount(uint32_t count);
  virtual Result BeginGlobal(uint32_t index, Type type, bool mutable_);
  virtual Result EndGlobalInitExpr(uint32_t index);

  virtual Result OnExport(uint32_t index,
                          ExternalKind kind,
                          uint32_t item_index,
                          StringSlice name);

  virtual Result OnStartFunction(uint32_t func_index);

  virtual Result BeginFunctionBody(uint32_t index);
  virtual Result OnLocalDeclCount(uint32_t count);
  virtual Result OnLocalDecl(uint32_t decl_index, uint32_t count, Type type);

  virtual Result OnBinaryExpr(Opcode opcode);
  virtual Result OnBlockExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnBrExpr(uint32_t depth);
  virtual Result OnBrIfExpr(uint32_t depth);
  virtual Result OnBrTableExpr(uint32_t num_targets,
                               uint32_t* target_depths,
                               uint32_t default_target_depth);
  virtual Result OnCallExpr(uint32_t func_index);
  virtual Result OnCallIndirectExpr(uint32_t sig_index);
  virtual Result OnCompareExpr(Opcode opcode);
  virtual Result OnConvertExpr(Opcode opcode);
  virtual Result OnCurrentMemoryExpr();
  virtual Result OnDropExpr();
  virtual Result OnElseExpr();
  virtual Result OnEndExpr();
  virtual Result OnF32ConstExpr(uint32_t value_bits);
  virtual Result OnF64ConstExpr(uint64_t value_bits);
  virtual Result OnGetGlobalExpr(uint32_t global_index);
  virtual Result OnGetLocalExpr(uint32_t local_index);
  virtual Result OnGrowMemoryExpr();
  virtual Result OnI32ConstExpr(uint32_t value);
  virtual Result OnI64ConstExpr(uint64_t value);
  virtual Result OnIfExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnLoadExpr(Opcode opcode,
                            uint32_t alignment_log2,
                            uint32_t offset);
  virtual Result OnLoopExpr(uint32_t num_types, Type* sig_types);
  virtual Result OnNopExpr();
  virtual Result OnReturnExpr();
  virtual Result OnSelectExpr();
  virtual Result OnSetGlobalExpr(uint32_t global_index);
  virtual Result OnSetLocalExpr(uint32_t local_index);
  virtual Result OnStoreExpr(Opcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset);
  virtual Result OnTeeLocalExpr(uint32_t local_index);
  virtual Result OnUnaryExpr(Opcode opcode);
  virtual Result OnUnreachableExpr();
  virtual Result EndFunctionBody(uint32_t index);

  virtual Result EndElemSegmentInitExpr(uint32_t index);
  virtual Result OnElemSegmentFunctionIndex(uint32_t index,
                                            uint32_t func_index);

  virtual Result OnDataSegmentData(uint32_t index,
                                   const void* data,
                                   uint32_t size);

  virtual Result OnInitExprF32ConstExpr(uint32_t index, uint32_t value);
  virtual Result OnInitExprF64ConstExpr(uint32_t index, uint64_t value);
  virtual Result OnInitExprGetGlobalExpr(uint32_t index, uint32_t global_index);
  virtual Result OnInitExprI32ConstExpr(uint32_t index, uint32_t value);
  virtual Result OnInitExprI64ConstExpr(uint32_t index, uint64_t value);

 private:
  Label* GetLabel(uint32_t depth);
  Label* TopLabel();
  void PushLabel(uint32_t offset, uint32_t fixup_offset);
  void PopLabel();

  bool HandleError(uint32_t offset, const char* message);
  void PrintError(const char* format, ...);
  static void OnTypecheckerError(const char* msg, void* user_data);

  uint32_t TranslateSigIndexToEnv(uint32_t sig_index);
  InterpreterFuncSignature* GetSignatureByEnvIndex(uint32_t sig_index);
  InterpreterFuncSignature* GetSignatureByModuleIndex(uint32_t sig_index);
  uint32_t TranslateFuncIndexToEnv(uint32_t func_index);
  uint32_t TranslateModuleFuncIndexToDefined(uint32_t func_index);
  InterpreterFunc* GetFuncByEnvIndex(uint32_t func_index);
  InterpreterFunc* GetFuncByModuleIndex(uint32_t func_index);
  uint32_t TranslateGlobalIndexToEnv(uint32_t global_index);
  InterpreterGlobal* GetGlobalByEnvIndex(uint32_t global_index);
  InterpreterGlobal* GetGlobalByModuleIndex(uint32_t global_index);
  Type GetGlobalTypeByModuleIndex(uint32_t global_index);
  uint32_t TranslateLocalIndex(uint32_t local_index);
  Type GetLocalTypeByIndex(InterpreterFunc* func, uint32_t local_index);

  uint32_t GetIstreamOffset();

  Result EmitDataAt(size_t offset, const void* data, size_t size);
  Result EmitData(const void* data, size_t size);
  Result EmitOpcode(Opcode opcode);
  Result EmitOpcode(InterpreterOpcode opcode);
  Result EmitI8(uint8_t value);
  Result EmitI32(uint32_t value);
  Result EmitI64(uint64_t value);
  Result EmitI32At(uint32_t offset, uint32_t value);
  Result EmitDropKeep(uint32_t drop, uint8_t keep);
  Result AppendFixup(Uint32VectorVector* fixups_vector, uint32_t index);
  Result EmitBrOffset(uint32_t depth, uint32_t offset);
  Result GetBrDropKeepCount(uint32_t depth,
                            uint32_t* out_drop_count,
                            uint32_t* out_keep_count);
  Result GetReturnDropKeepCount(uint32_t* out_drop_count,
                                uint32_t* out_keep_count);
  Result EmitBr(uint32_t depth, uint32_t drop_count, uint32_t keep_count);
  Result EmitBrTableOffset(uint32_t depth);
  Result FixupTopLabel();
  Result EmitFuncOffset(DefinedInterpreterFunc* func, uint32_t func_index);

  Result CheckLocal(uint32_t local_index);
  Result CheckGlobal(uint32_t global_index);
  Result CheckImportKind(InterpreterImport* import, ExternalKind expected_kind);
  Result CheckImportLimits(const Limits* declared_limits,
                           const Limits* actual_limits);
  Result CheckHasMemory(Opcode opcode);
  Result CheckAlign(uint32_t alignment_log2, uint32_t natural_alignment);

  Result AppendExport(InterpreterModule* module,
                      ExternalKind kind,
                      uint32_t item_index,
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
  Uint32VectorVector func_fixups;
  Uint32VectorVector depth_fixups;
  MemoryWriter istream_writer;
  uint32_t istream_offset = 0;
  /* mappings from module index space to env index space; this won't just be a
   * translation, because imported values will be resolved as well */
  Uint32Vector sig_index_mapping;
  Uint32Vector func_index_mapping;
  Uint32Vector global_index_mapping;

  uint32_t num_func_imports = 0;
  uint32_t num_global_imports = 0;

  // Changes to linear memory and tables should not apply if a validation error
  // occurs; these vectors cache the changes that must be applied after we know
  // that there are no validation errors.
  std::vector<ElemSegmentInfo> elem_segment_infos;
  std::vector<DataSegmentInfo> data_segment_infos;

  /* values cached so they can be shared between callbacks */
  InterpreterTypedValue init_expr_value;
  uint32_t table_offset = 0;
  bool is_host_import = false;
  HostInterpreterModule* host_import_module = nullptr;
  uint32_t import_env_index = 0;
};

BinaryReaderInterpreter::BinaryReaderInterpreter(
    InterpreterEnvironment* env,
    DefinedInterpreterModule* module,
    size_t istream_offset,
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

Label* BinaryReaderInterpreter::GetLabel(uint32_t depth) {
  assert(depth < label_stack.size());
  return &label_stack[label_stack.size() - depth - 1];
}

Label* BinaryReaderInterpreter::TopLabel() {
  return GetLabel(0);
}

bool BinaryReaderInterpreter::HandleError(uint32_t offset,
                                          const char* message) {
  return error_handler->OnError(offset, message);
}

void WABT_PRINTF_FORMAT(2, 3)
    BinaryReaderInterpreter::PrintError(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  HandleError(WABT_INVALID_OFFSET, buffer);
}

// static
void BinaryReaderInterpreter::OnTypecheckerError(const char* msg,
                                                 void* user_data) {
  static_cast<BinaryReaderInterpreter*>(user_data)->PrintError("%s", msg);
}

uint32_t BinaryReaderInterpreter::TranslateSigIndexToEnv(uint32_t sig_index) {
  assert(sig_index < sig_index_mapping.size());
  return sig_index_mapping[sig_index];
}

InterpreterFuncSignature* BinaryReaderInterpreter::GetSignatureByEnvIndex(
    uint32_t sig_index) {
  return &env->sigs[sig_index];
}

InterpreterFuncSignature* BinaryReaderInterpreter::GetSignatureByModuleIndex(
    uint32_t sig_index) {
  return GetSignatureByEnvIndex(TranslateSigIndexToEnv(sig_index));
}

uint32_t BinaryReaderInterpreter::TranslateFuncIndexToEnv(uint32_t func_index) {
  assert(func_index < func_index_mapping.size());
  return func_index_mapping[func_index];
}

uint32_t BinaryReaderInterpreter::TranslateModuleFuncIndexToDefined(
    uint32_t func_index) {
  assert(func_index >= num_func_imports);
  return func_index - num_func_imports;
}

InterpreterFunc* BinaryReaderInterpreter::GetFuncByEnvIndex(
    uint32_t func_index) {
  return env->funcs[func_index].get();
}

InterpreterFunc* BinaryReaderInterpreter::GetFuncByModuleIndex(
    uint32_t func_index) {
  return GetFuncByEnvIndex(TranslateFuncIndexToEnv(func_index));
}

uint32_t BinaryReaderInterpreter::TranslateGlobalIndexToEnv(
    uint32_t global_index) {
  return global_index_mapping[global_index];
}

InterpreterGlobal* BinaryReaderInterpreter::GetGlobalByEnvIndex(
    uint32_t global_index) {
  return &env->globals[global_index];
}

InterpreterGlobal* BinaryReaderInterpreter::GetGlobalByModuleIndex(
    uint32_t global_index) {
  return GetGlobalByEnvIndex(TranslateGlobalIndexToEnv(global_index));
}

Type BinaryReaderInterpreter::GetGlobalTypeByModuleIndex(
    uint32_t global_index) {
  return GetGlobalByModuleIndex(global_index)->typed_value.type;
}

Type BinaryReaderInterpreter::GetLocalTypeByIndex(InterpreterFunc* func,
                                                  uint32_t local_index) {
  assert(!func->is_host);
  return func->as_defined()->param_and_local_types[local_index];
}

uint32_t BinaryReaderInterpreter::GetIstreamOffset() {
  return istream_offset;
}

Result BinaryReaderInterpreter::EmitDataAt(size_t offset,
                                           const void* data,
                                           size_t size) {
  return istream_writer.WriteData(offset, data, size);
}

Result BinaryReaderInterpreter::EmitData(const void* data, size_t size) {
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

Result BinaryReaderInterpreter::EmitI32At(uint32_t offset, uint32_t value) {
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

Result BinaryReaderInterpreter::AppendFixup(Uint32VectorVector* fixups_vector,
                                            uint32_t index) {
  if (index >= fixups_vector->size())
    fixups_vector->resize(index + 1);
  (*fixups_vector)[index].push_back(GetIstreamOffset());
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBrOffset(uint32_t depth, uint32_t offset) {
  if (offset == WABT_INVALID_OFFSET) {
    /* depth_fixups stores the depth counting up from zero, where zero is the
     * top-level function scope. */
    depth = label_stack.size() - 1 - depth;
    CHECK_RESULT(AppendFixup(&depth_fixups, depth));
  }
  CHECK_RESULT(EmitI32(offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::GetBrDropKeepCount(uint32_t depth,
                                                   uint32_t* out_drop_count,
                                                   uint32_t* out_keep_count) {
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

Result BinaryReaderInterpreter::GetReturnDropKeepCount(
    uint32_t* out_drop_count,
    uint32_t* out_keep_count) {
  if (WABT_FAILED(GetBrDropKeepCount(label_stack.size() - 1, out_drop_count,
                                     out_keep_count))) {
    return Result::Error;
  }

  *out_drop_count += current_func->param_and_local_types.size();
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBr(uint32_t depth,
                                       uint32_t drop_count,
                                       uint32_t keep_count) {
  CHECK_RESULT(EmitDropKeep(drop_count, keep_count));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Br));
  CHECK_RESULT(EmitBrOffset(depth, GetLabel(depth)->offset));
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitBrTableOffset(uint32_t depth) {
  uint32_t drop_count, keep_count;
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  CHECK_RESULT(EmitBrOffset(depth, GetLabel(depth)->offset));
  CHECK_RESULT(EmitI32(drop_count));
  CHECK_RESULT(EmitI8(keep_count));
  return Result::Ok;
}

Result BinaryReaderInterpreter::FixupTopLabel() {
  uint32_t offset = GetIstreamOffset();
  uint32_t top = label_stack.size() - 1;
  if (top >= depth_fixups.size()) {
    /* nothing to fixup */
    return Result::Ok;
  }

  Uint32Vector& fixups = depth_fixups[top];
  for (uint32_t fixup : fixups)
    CHECK_RESULT(EmitI32At(fixup, offset));
  fixups.clear();
  return Result::Ok;
}

Result BinaryReaderInterpreter::EmitFuncOffset(DefinedInterpreterFunc* func,
                                               uint32_t func_index) {
  if (func->offset == WABT_INVALID_OFFSET) {
    uint32_t defined_index = TranslateModuleFuncIndexToDefined(func_index);
    CHECK_RESULT(AppendFixup(&func_fixups, defined_index));
  }
  CHECK_RESULT(EmitI32(func->offset));
  return Result::Ok;
}

bool BinaryReaderInterpreter::OnError(const char* message) {
  return HandleError(state->offset, message);
}

Result BinaryReaderInterpreter::OnTypeCount(uint32_t count) {
  sig_index_mapping.resize(count);
  for (uint32_t i = 0; i < count; ++i)
    sig_index_mapping[i] = env->sigs.size() + i;
  env->sigs.resize(env->sigs.size() + count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnType(uint32_t index,
                                       uint32_t param_count,
                                       Type* param_types,
                                       uint32_t result_count,
                                       Type* result_types) {
  InterpreterFuncSignature* sig = GetSignatureByModuleIndex(index);
  sig->param_types.insert(sig->param_types.end(), param_types,
                          param_types + param_count);
  sig->result_types.insert(sig->result_types.end(), result_types,
                           result_types + result_count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImportCount(uint32_t count) {
  module->imports.resize(count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnImport(uint32_t index,
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

Result BinaryReaderInterpreter::CheckLocal(uint32_t local_index) {
  uint32_t max_local_index = current_func->param_and_local_types.size();
  if (local_index >= max_local_index) {
    PrintError("invalid local_index: %d (max %d)", local_index,
               max_local_index);
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckGlobal(uint32_t global_index) {
  uint32_t max_global_index = global_index_mapping.size();
  if (global_index >= max_global_index) {
    PrintError("invalid global_index: %d (max %d)", global_index,
               max_global_index);
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
                                             uint32_t item_index,
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

Result BinaryReaderInterpreter::OnImportFunc(uint32_t import_index,
                                             StringSlice module_name,
                                             StringSlice field_name,
                                             uint32_t func_index,
                                             uint32_t sig_index) {
  InterpreterImport* import = &module->imports[import_index];
  import->func.sig_index = TranslateSigIndexToEnv(sig_index);

  uint32_t func_env_index;
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

Result BinaryReaderInterpreter::OnImportTable(uint32_t import_index,
                                              StringSlice module_name,
                                              StringSlice field_name,
                                              uint32_t table_index,
                                              Type elem_type,
                                              const Limits* elem_limits) {
  if (module->table_index != WABT_INVALID_INDEX) {
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

Result BinaryReaderInterpreter::OnImportMemory(uint32_t import_index,
                                               StringSlice module_name,
                                               StringSlice field_name,
                                               uint32_t memory_index,
                                               const Limits* page_limits) {
  if (module->memory_index != WABT_INVALID_INDEX) {
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

Result BinaryReaderInterpreter::OnImportGlobal(uint32_t import_index,
                                               StringSlice module_name,
                                               StringSlice field_name,
                                               uint32_t global_index,
                                               Type type,
                                               bool mutable_) {
  InterpreterImport* import = &module->imports[import_index];

  uint32_t global_env_index = env->globals.size() - 1;
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

Result BinaryReaderInterpreter::OnFunctionCount(uint32_t count) {
  for (uint32_t i = 0; i < count; ++i)
    func_index_mapping.push_back(env->funcs.size() + i);
  env->funcs.reserve(env->funcs.size() + count);
  func_fixups.resize(count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnFunction(uint32_t index, uint32_t sig_index) {
  DefinedInterpreterFunc* func =
      new DefinedInterpreterFunc(TranslateSigIndexToEnv(sig_index));
  env->funcs.emplace_back(func);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnTable(uint32_t index,
                                        Type elem_type,
                                        const Limits* elem_limits) {
  if (module->table_index != WABT_INVALID_INDEX) {
    PrintError("only one table allowed");
    return Result::Error;
  }
  env->tables.emplace_back(*elem_limits);
  module->table_index = env->tables.size() - 1;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnMemory(uint32_t index,
                                         const Limits* page_limits) {
  if (module->memory_index != WABT_INVALID_INDEX) {
    PrintError("only one memory allowed");
    return Result::Error;
  }
  env->memories.emplace_back(*page_limits);
  module->memory_index = env->memories.size() - 1;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnGlobalCount(uint32_t count) {
  for (uint32_t i = 0; i < count; ++i)
    global_index_mapping.push_back(env->globals.size() + i);
  env->globals.resize(env->globals.size() + count);
  return Result::Ok;
}

Result BinaryReaderInterpreter::BeginGlobal(uint32_t index,
                                            Type type,
                                            bool mutable_) {
  InterpreterGlobal* global = GetGlobalByModuleIndex(index);
  global->typed_value.type = type;
  global->mutable_ = mutable_;
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndGlobalInitExpr(uint32_t index) {
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

Result BinaryReaderInterpreter::OnInitExprF32ConstExpr(uint32_t index,
                                                       uint32_t value_bits) {
  init_expr_value.type = Type::F32;
  init_expr_value.value.f32_bits = value_bits;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprF64ConstExpr(uint32_t index,
                                                       uint64_t value_bits) {
  init_expr_value.type = Type::F64;
  init_expr_value.value.f64_bits = value_bits;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprGetGlobalExpr(uint32_t index,
                                                        uint32_t global_index) {
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

Result BinaryReaderInterpreter::OnInitExprI32ConstExpr(uint32_t index,
                                                       uint32_t value) {
  init_expr_value.type = Type::I32;
  init_expr_value.value.i32 = value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnInitExprI64ConstExpr(uint32_t index,
                                                       uint64_t value) {
  init_expr_value.type = Type::I64;
  init_expr_value.value.i64 = value;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnExport(uint32_t index,
                                         ExternalKind kind,
                                         uint32_t item_index,
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

Result BinaryReaderInterpreter::OnStartFunction(uint32_t func_index) {
  uint32_t start_func_index = TranslateFuncIndexToEnv(func_index);
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

Result BinaryReaderInterpreter::EndElemSegmentInitExpr(uint32_t index) {
  if (init_expr_value.type != Type::I32) {
    PrintError("type mismatch in elem segment, expected i32 but got %s",
               get_type_name(init_expr_value.type));
    return Result::Error;
  }
  table_offset = init_expr_value.value.i32;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnElemSegmentFunctionIndex(

    uint32_t index,
    uint32_t func_index) {
  assert(module->table_index != WABT_INVALID_INDEX);
  InterpreterTable* table = &env->tables[module->table_index];
  if (table_offset >= table->func_indexes.size()) {
    PrintError("elem segment offset is out of bounds: %u >= max value %" PRIzd,
               table_offset, table->func_indexes.size());
    return Result::Error;
  }

  uint32_t max_func_index = func_index_mapping.size();
  if (func_index >= max_func_index) {
    PrintError("invalid func_index: %d (max %d)", func_index, max_func_index);
    return Result::Error;
  }

  elem_segment_infos.emplace_back(&table->func_indexes[table_offset++],
                                  TranslateFuncIndexToEnv(func_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnDataSegmentData(uint32_t index,
                                                  const void* src_data,
                                                  uint32_t size) {
  assert(module->memory_index != WABT_INVALID_INDEX);
  InterpreterMemory* memory = &env->memories[module->memory_index];
  if (init_expr_value.type != Type::I32) {
    PrintError("type mismatch in data segment, expected i32 but got %s",
               get_type_name(init_expr_value.type));
    return Result::Error;
  }
  uint32_t address = init_expr_value.value.i32;
  uint64_t end_address =
      static_cast<uint64_t>(address) + static_cast<uint64_t>(size);
  if (end_address > memory->data.size()) {
    PrintError("data segment is out of bounds: [%u, %" PRIu64
               ") >= max value %" PRIzd,
               address, end_address, memory->data.size());
    return Result::Error;
  }

  if (size > 0)
    data_segment_infos.emplace_back(&memory->data[address], src_data, size);

  return Result::Ok;
}

void BinaryReaderInterpreter::PushLabel(uint32_t offset,
                                        uint32_t fixup_offset) {
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

Result BinaryReaderInterpreter::BeginFunctionBody(uint32_t index) {
  DefinedInterpreterFunc* func = GetFuncByModuleIndex(index)->as_defined();
  InterpreterFuncSignature* sig = GetSignatureByEnvIndex(func->sig_index);

  func->offset = GetIstreamOffset();
  func->local_decl_count = 0;
  func->local_count = 0;

  current_func = func;
  depth_fixups.clear();
  label_stack.clear();

  /* fixup function references */
  uint32_t defined_index = TranslateModuleFuncIndexToDefined(index);
  Uint32Vector& fixups = func_fixups[defined_index];
  for (uint32_t fixup : fixups)
    CHECK_RESULT(EmitI32At(fixup, func->offset));

  /* append param types */
  for (Type param_type : sig->param_types)
    func->param_and_local_types.push_back(param_type);

  CHECK_RESULT(typechecker_begin_function(&typechecker, &sig->result_types));

  /* push implicit func label (equivalent to return) */
  PushLabel(WABT_INVALID_OFFSET, WABT_INVALID_OFFSET);
  return Result::Ok;
}

Result BinaryReaderInterpreter::EndFunctionBody(uint32_t index) {
  FixupTopLabel();
  uint32_t drop_count, keep_count;
  CHECK_RESULT(GetReturnDropKeepCount(&drop_count, &keep_count));
  CHECK_RESULT(typechecker_end_function(&typechecker));
  CHECK_RESULT(EmitDropKeep(drop_count, keep_count));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Return));
  PopLabel();
  current_func = nullptr;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLocalDeclCount(uint32_t count) {
  current_func->local_decl_count = count;
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLocalDecl(uint32_t decl_index,
                                            uint32_t count,
                                            Type type) {
  current_func->local_count += count;

  for (uint32_t i = 0; i < count; ++i)
    current_func->param_and_local_types.push_back(type);

  if (decl_index == current_func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(EmitOpcode(InterpreterOpcode::Alloca));
    CHECK_RESULT(EmitI32(current_func->local_count));
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckHasMemory(Opcode opcode) {
  if (module->memory_index == WABT_INVALID_INDEX) {
    PrintError("%s requires an imported or defined memory.",
               get_opcode_name(opcode));
    return Result::Error;
  }
  return Result::Ok;
}

Result BinaryReaderInterpreter::CheckAlign(uint32_t alignment_log2,
                                           uint32_t natural_alignment) {
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

Result BinaryReaderInterpreter::OnBlockExpr(uint32_t num_types,
                                            Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_block(&typechecker, &sig));
  PushLabel(WABT_INVALID_OFFSET, WABT_INVALID_OFFSET);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnLoopExpr(uint32_t num_types,
                                           Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_loop(&typechecker, &sig));
  PushLabel(GetIstreamOffset(), WABT_INVALID_OFFSET);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnIfExpr(uint32_t num_types, Type* sig_types) {
  TypeVector sig(sig_types, sig_types + num_types);
  CHECK_RESULT(typechecker_on_if(&typechecker, &sig));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrUnless));
  uint32_t fixup_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(WABT_INVALID_OFFSET));
  PushLabel(WABT_INVALID_OFFSET, fixup_offset);
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnElseExpr() {
  CHECK_RESULT(typechecker_on_else(&typechecker));
  Label* label = TopLabel();
  uint32_t fixup_cond_offset = label->fixup_offset;
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Br));
  label->fixup_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(WABT_INVALID_OFFSET));
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

Result BinaryReaderInterpreter::OnBrExpr(uint32_t depth) {
  uint32_t drop_count, keep_count;
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  CHECK_RESULT(typechecker_on_br(&typechecker, depth));
  CHECK_RESULT(EmitBr(depth, drop_count, keep_count));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBrIfExpr(uint32_t depth) {
  uint32_t drop_count, keep_count;
  CHECK_RESULT(typechecker_on_br_if(&typechecker, depth));
  CHECK_RESULT(GetBrDropKeepCount(depth, &drop_count, &keep_count));
  /* flip the br_if so if <cond> is true it can drop values from the stack */
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrUnless));
  uint32_t fixup_br_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(WABT_INVALID_OFFSET));
  CHECK_RESULT(EmitBr(depth, drop_count, keep_count));
  CHECK_RESULT(EmitI32At(fixup_br_offset, GetIstreamOffset()));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnBrTableExpr(uint32_t num_targets,
                                              uint32_t* target_depths,
                                              uint32_t default_target_depth) {
  CHECK_RESULT(typechecker_begin_br_table(&typechecker));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::BrTable));
  CHECK_RESULT(EmitI32(num_targets));
  uint32_t fixup_table_offset = GetIstreamOffset();
  CHECK_RESULT(EmitI32(WABT_INVALID_OFFSET));
  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::Data));
  CHECK_RESULT(EmitI32((num_targets + 1) * WABT_TABLE_ENTRY_SIZE));
  CHECK_RESULT(EmitI32At(fixup_table_offset, GetIstreamOffset()));

  for (uint32_t i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    CHECK_RESULT(typechecker_on_br_table_target(&typechecker, depth));
    CHECK_RESULT(EmitBrTableOffset(depth));
  }

  CHECK_RESULT(typechecker_end_br_table(&typechecker));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnCallExpr(uint32_t func_index) {
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

Result BinaryReaderInterpreter::OnCallIndirectExpr(uint32_t sig_index) {
  if (module->table_index == WABT_INVALID_INDEX) {
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

Result BinaryReaderInterpreter::OnGetGlobalExpr(uint32_t global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  Type type = GetGlobalTypeByModuleIndex(global_index);
  CHECK_RESULT(typechecker_on_get_global(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::GetGlobal));
  CHECK_RESULT(EmitI32(TranslateGlobalIndexToEnv(global_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnSetGlobalExpr(uint32_t global_index) {
  CHECK_RESULT(CheckGlobal(global_index));
  InterpreterGlobal* global = GetGlobalByModuleIndex(global_index);
  if (!global->mutable_) {
    PrintError("can't set_global on immutable global at index %u.",
               global_index);
    return Result::Error;
  }
  CHECK_RESULT(
      typechecker_on_set_global(&typechecker, global->typed_value.type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::SetGlobal));
  CHECK_RESULT(EmitI32(TranslateGlobalIndexToEnv(global_index)));
  return Result::Ok;
}

uint32_t BinaryReaderInterpreter::TranslateLocalIndex(uint32_t local_index) {
  return typechecker.type_stack.size() +
         current_func->param_and_local_types.size() - local_index;
}

Result BinaryReaderInterpreter::OnGetLocalExpr(uint32_t local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  Type type = GetLocalTypeByIndex(current_func, local_index);
  /* Get the translated index before calling typechecker_on_get_local
   * because it will update the type stack size. We need the index to be
   * relative to the old stack size. */
  uint32_t translated_local_index = TranslateLocalIndex(local_index);
  CHECK_RESULT(typechecker_on_get_local(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::GetLocal));
  CHECK_RESULT(EmitI32(translated_local_index));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnSetLocalExpr(uint32_t local_index) {
  CHECK_RESULT(CheckLocal(local_index));
  Type type = GetLocalTypeByIndex(current_func, local_index);
  CHECK_RESULT(typechecker_on_set_local(&typechecker, type));
  CHECK_RESULT(EmitOpcode(InterpreterOpcode::SetLocal));
  CHECK_RESULT(EmitI32(TranslateLocalIndex(local_index)));
  return Result::Ok;
}

Result BinaryReaderInterpreter::OnTeeLocalExpr(uint32_t local_index) {
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
                                           uint32_t offset) {
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
                                            uint32_t offset) {
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
  uint32_t drop_count, keep_count;
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
  size_t istream_offset = env->istream->data.size();
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
