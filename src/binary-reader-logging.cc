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

#include "binary-reader-logging.h"

#include <inttypes.h>

#include "stream.h"

namespace wabt {

#define INDENT_SIZE 2

#define LOGF_NOINDENT(...) stream->Writef(__VA_ARGS__)

#define LOGF(...)               \
  do {                          \
    WriteIndent();              \
    LOGF_NOINDENT(__VA_ARGS__); \
  } while (0)

namespace {

void sprint_limits(char* dst, size_t size, const Limits* limits) {
  int result;
  if (limits->has_max) {
    result = wabt_snprintf(dst, size, "initial: %" PRIu64 ", max: %" PRIu64,
                           limits->initial, limits->max);
  } else {
    result = wabt_snprintf(dst, size, "initial: %" PRIu64, limits->initial);
  }
  WABT_USE(result);
  assert(static_cast<size_t>(result) < size);
}

}  // namespace

BinaryReaderLogging::BinaryReaderLogging(Stream* stream, BinaryReader* forward)
    : stream(stream), reader(forward), indent(0) {}

void BinaryReaderLogging::Indent() {
  indent += INDENT_SIZE;
}

void BinaryReaderLogging::Dedent() {
  indent -= INDENT_SIZE;
  assert(indent >= 0);
}

void BinaryReaderLogging::WriteIndent() {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t i = indent;
  while (i > s_indent_len) {
    stream->WriteData(s_indent, s_indent_len);
    i -= s_indent_len;
  }
  if (i > 0) {
    stream->WriteData(s_indent, indent);
  }
}

void BinaryReaderLogging::LogTypes(uint32_t type_count, Type* types) {
  LOGF_NOINDENT("[");
  for (uint32_t i = 0; i < type_count; ++i) {
    LOGF_NOINDENT("%s", get_type_name(types[i]));
    if (i != type_count - 1)
      LOGF_NOINDENT(", ");
  }
  LOGF_NOINDENT("]");
}

bool BinaryReaderLogging::OnError(const char* message) {
  return reader->OnError(message);
}

void BinaryReaderLogging::OnSetState(const State* s) {
  BinaryReader::OnSetState(s);
  reader->OnSetState(s);
}

Result BinaryReaderLogging::BeginModule(uint32_t version) {
  LOGF("BeginModule(version: %u)\n", version);
  Indent();
  return reader->BeginModule(version);
}

Result BinaryReaderLogging::BeginSection(BinarySection section_type,
                                         uint32_t size) {
  return reader->BeginSection(section_type, size);
}

Result BinaryReaderLogging::BeginCustomSection(uint32_t size,
                                               StringSlice section_name) {
  LOGF("BeginCustomSection('" PRIstringslice "', size: %d)\n",
       WABT_PRINTF_STRING_SLICE_ARG(section_name), size);
  Indent();
  return reader->BeginCustomSection(size, section_name);
}

Result BinaryReaderLogging::OnType(uint32_t index,
                                   uint32_t param_count,
                                   Type* param_types,
                                   uint32_t result_count,
                                   Type* result_types) {
  LOGF("OnType(index: %u, params: ", index);
  LogTypes(param_count, param_types);
  LOGF_NOINDENT(", results: ");
  LogTypes(result_count, result_types);
  LOGF_NOINDENT(")\n");
  return reader->OnType(index, param_count, param_types, result_count,
                        result_types);
}

Result BinaryReaderLogging::OnImport(uint32_t index,
                                     StringSlice module_name,
                                     StringSlice field_name) {
  LOGF("OnImport(index: %u, module: \"" PRIstringslice
       "\", field: \"" PRIstringslice "\")\n",
       index, WABT_PRINTF_STRING_SLICE_ARG(module_name),
       WABT_PRINTF_STRING_SLICE_ARG(field_name));
  return reader->OnImport(index, module_name, field_name);
}

Result BinaryReaderLogging::OnImportFunc(uint32_t import_index,
                                         StringSlice module_name,
                                         StringSlice field_name,
                                         uint32_t func_index,
                                         uint32_t sig_index) {
  LOGF("OnImportFunc(import_index: %u, func_index: %u, sig_index: %u)\n",
       import_index, func_index, sig_index);
  return reader->OnImportFunc(import_index, module_name, field_name, func_index,
                              sig_index);
}

Result BinaryReaderLogging::OnImportTable(uint32_t import_index,
                                          StringSlice module_name,
                                          StringSlice field_name,
                                          uint32_t table_index,
                                          Type elem_type,
                                          const Limits* elem_limits) {
  char buf[100];
  sprint_limits(buf, sizeof(buf), elem_limits);
  LOGF("OnImportTable(import_index: %u, table_index: %u, elem_type: %s, %s)\n",
       import_index, table_index, get_type_name(elem_type), buf);
  return reader->OnImportTable(import_index, module_name, field_name,
                               table_index, elem_type, elem_limits);
}

Result BinaryReaderLogging::OnImportMemory(uint32_t import_index,
                                           StringSlice module_name,
                                           StringSlice field_name,
                                           uint32_t memory_index,
                                           const Limits* page_limits) {
  char buf[100];
  sprint_limits(buf, sizeof(buf), page_limits);
  LOGF("OnImportMemory(import_index: %u, memory_index: %u, %s)\n", import_index,
       memory_index, buf);
  return reader->OnImportMemory(import_index, module_name, field_name,
                                memory_index, page_limits);
}

Result BinaryReaderLogging::OnImportGlobal(uint32_t import_index,
                                           StringSlice module_name,
                                           StringSlice field_name,
                                           uint32_t global_index,
                                           Type type,
                                           bool mutable_) {
  LOGF(
      "OnImportGlobal(import_index: %u, global_index: %u, type: %s, mutable: "
      "%s)\n",
      import_index, global_index, get_type_name(type),
      mutable_ ? "true" : "false");
  return reader->OnImportGlobal(import_index, module_name, field_name,
                                global_index, type, mutable_);
}

Result BinaryReaderLogging::OnTable(uint32_t index,
                                    Type elem_type,
                                    const Limits* elem_limits) {
  char buf[100];
  sprint_limits(buf, sizeof(buf), elem_limits);
  LOGF("OnTable(index: %u, elem_type: %s, %s)\n", index,
       get_type_name(elem_type), buf);
  return reader->OnTable(index, elem_type, elem_limits);
}

Result BinaryReaderLogging::OnMemory(uint32_t index,
                                     const Limits* page_limits) {
  char buf[100];
  sprint_limits(buf, sizeof(buf), page_limits);
  LOGF("OnMemory(index: %u, %s)\n", index, buf);
  return reader->OnMemory(index, page_limits);
}

Result BinaryReaderLogging::BeginGlobal(uint32_t index,
                                        Type type,
                                        bool mutable_) {
  LOGF("BeginGlobal(index: %u, type: %s, mutable: %s)\n", index,
       get_type_name(type), mutable_ ? "true" : "false");
  return reader->BeginGlobal(index, type, mutable_);
}

Result BinaryReaderLogging::OnExport(uint32_t index,
                                     ExternalKind kind,
                                     uint32_t item_index,
                                     StringSlice name) {
  LOGF("OnExport(index: %u, kind: %s, item_index: %u, name: \"" PRIstringslice
       "\")\n",
       index, get_kind_name(kind), item_index,
       WABT_PRINTF_STRING_SLICE_ARG(name));
  return reader->OnExport(index, kind, item_index, name);
}

Result BinaryReaderLogging::OnLocalDecl(uint32_t decl_index,
                                        uint32_t count,
                                        Type type) {
  LOGF("OnLocalDecl(index: %u, count: %u, type: %s)\n", decl_index, count,
       get_type_name(type));
  return reader->OnLocalDecl(decl_index, count, type);
}

Result BinaryReaderLogging::OnBlockExpr(uint32_t num_types, Type* sig_types) {
  LOGF("OnBlockExpr(sig: ");
  LogTypes(num_types, sig_types);
  LOGF_NOINDENT(")\n");
  return reader->OnBlockExpr(num_types, sig_types);
}

Result BinaryReaderLogging::OnBrExpr(uint32_t depth) {
  LOGF("OnBrExpr(depth: %u)\n", depth);
  return reader->OnBrExpr(depth);
}

Result BinaryReaderLogging::OnBrIfExpr(uint32_t depth) {
  LOGF("OnBrIfExpr(depth: %u)\n", depth);
  return reader->OnBrIfExpr(depth);
}

Result BinaryReaderLogging::OnBrTableExpr(uint32_t num_targets,
                                          uint32_t* target_depths,
                                          uint32_t default_target_depth) {
  LOGF("OnBrTableExpr(num_targets: %u, depths: [", num_targets);
  for (uint32_t i = 0; i < num_targets; ++i) {
    LOGF_NOINDENT("%u", target_depths[i]);
    if (i != num_targets - 1)
      LOGF_NOINDENT(", ");
  }
  LOGF_NOINDENT("], default: %u)\n", default_target_depth);
  return reader->OnBrTableExpr(num_targets, target_depths,
                               default_target_depth);
}

Result BinaryReaderLogging::OnF32ConstExpr(uint32_t value_bits) {
  float value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("OnF32ConstExpr(%g (0x04%x))\n", value, value_bits);
  return reader->OnF32ConstExpr(value_bits);
}

Result BinaryReaderLogging::OnF64ConstExpr(uint64_t value_bits) {
  double value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("OnF64ConstExpr(%g (0x08%" PRIx64 "))\n", value, value_bits);
  return reader->OnF64ConstExpr(value_bits);
}

Result BinaryReaderLogging::OnI32ConstExpr(uint32_t value) {
  LOGF("OnI32ConstExpr(%u (0x%x))\n", value, value);
  return reader->OnI32ConstExpr(value);
}

Result BinaryReaderLogging::OnI64ConstExpr(uint64_t value) {
  LOGF("OnI64ConstExpr(%" PRIu64 " (0x%" PRIx64 "))\n", value, value);
  return reader->OnI64ConstExpr(value);
}

Result BinaryReaderLogging::OnIfExpr(uint32_t num_types, Type* sig_types) {
  LOGF("OnIfExpr(sig: ");
  LogTypes(num_types, sig_types);
  LOGF_NOINDENT(")\n");
  return reader->OnIfExpr(num_types, sig_types);
}

Result BinaryReaderLogging::OnLoadExpr(Opcode opcode,
                                       uint32_t alignment_log2,
                                       uint32_t offset) {
  LOGF("OnLoadExpr(opcode: \"%s\" (%u), align log2: %u, offset: %u)\n",
       get_opcode_name(opcode), static_cast<unsigned>(opcode), alignment_log2,
       offset);
  return reader->OnLoadExpr(opcode, alignment_log2, offset);
}

Result BinaryReaderLogging::OnLoopExpr(uint32_t num_types, Type* sig_types) {
  LOGF("OnLoopExpr(sig: ");
  LogTypes(num_types, sig_types);
  LOGF_NOINDENT(")\n");
  return reader->OnLoopExpr(num_types, sig_types);
}

Result BinaryReaderLogging::OnStoreExpr(Opcode opcode,
                                        uint32_t alignment_log2,
                                        uint32_t offset) {
  LOGF("OnStoreExpr(opcode: \"%s\" (%u), align log2: %u, offset: %u)\n",
       get_opcode_name(opcode), static_cast<unsigned>(opcode), alignment_log2,
       offset);
  return reader->OnStoreExpr(opcode, alignment_log2, offset);
}

Result BinaryReaderLogging::OnDataSegmentData(uint32_t index,
                                              const void* data,
                                              uint32_t size) {
  LOGF("OnDataSegmentData(index:%u, size:%u)\n", index, size);
  return reader->OnDataSegmentData(index, data, size);
}

Result BinaryReaderLogging::OnFunctionNameSubsection(uint32_t index,
                                                     uint32_t name_type,
                                                     uint32_t subsection_size) {
  LOGF("OnFunctionNameSubsection(index:%u, nametype:%u, size:%u)\n", index,
       name_type, subsection_size);
  return reader->OnFunctionNameSubsection(index, name_type, subsection_size);
}

Result BinaryReaderLogging::OnFunctionName(uint32_t index, StringSlice name) {
  LOGF("OnFunctionName(index: %u, name: \"" PRIstringslice "\")\n", index,
       WABT_PRINTF_STRING_SLICE_ARG(name));
  return reader->OnFunctionName(index, name);
}

Result BinaryReaderLogging::OnLocalNameSubsection(uint32_t index,
                                                  uint32_t name_type,
                                                  uint32_t subsection_size) {
  LOGF("OnLocalNameSubsection(index:%u, nametype:%u, size:%u)\n", index,
       name_type, subsection_size);
  return reader->OnLocalNameSubsection(index, name_type, subsection_size);
}

Result BinaryReaderLogging::OnLocalName(uint32_t func_index,
                                        uint32_t local_index,
                                        StringSlice name) {
  LOGF("OnLocalName(func_index: %u, local_index: %u, name: \"" PRIstringslice
       "\")\n",
       func_index, local_index, WABT_PRINTF_STRING_SLICE_ARG(name));
  return reader->OnLocalName(func_index, local_index, name);
}

Result BinaryReaderLogging::OnInitExprF32ConstExpr(uint32_t index,
                                                   uint32_t value_bits) {
  float value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("OnInitExprF32ConstExpr(index: %u, value: %g (0x04%x))\n", index, value,
       value_bits);
  return reader->OnInitExprF32ConstExpr(index, value_bits);
}

Result BinaryReaderLogging::OnInitExprF64ConstExpr(uint32_t index,
                                                   uint64_t value_bits) {
  double value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("OnInitExprF64ConstExpr(index: %u value: %g (0x08%" PRIx64 "))\n", index,
       value, value_bits);
  return reader->OnInitExprF64ConstExpr(index, value_bits);
}

Result BinaryReaderLogging::OnInitExprI32ConstExpr(uint32_t index,
                                                   uint32_t value) {
  LOGF("OnInitExprI32ConstExpr(index: %u, value: %u)\n", index, value);
  return reader->OnInitExprI32ConstExpr(index, value);
}

Result BinaryReaderLogging::OnInitExprI64ConstExpr(uint32_t index,
                                                   uint64_t value) {
  LOGF("OnInitExprI64ConstExpr(index: %u, value: %" PRIu64 ")\n", index, value);
  return reader->OnInitExprI64ConstExpr(index, value);
}

Result BinaryReaderLogging::OnRelocCount(uint32_t count,
                                         BinarySection section_code,
                                         StringSlice section_name) {
  LOGF("OnRelocCount(count: %d, section: %s, section_name: " PRIstringslice
       ")\n",
       count, get_section_name(section_code),
       WABT_PRINTF_STRING_SLICE_ARG(section_name));
  return reader->OnRelocCount(count, section_code, section_name);
}

Result BinaryReaderLogging::OnReloc(RelocType type,
                                    uint32_t offset,
                                    uint32_t index,
                                    int32_t addend) {
  LOGF("OnReloc(type: %s, offset: %u, index: %u, addend: %d)\n",
       get_reloc_type_name(type), offset, index, addend);
  return reader->OnReloc(type, offset, index, addend);
}

#define DEFINE_BEGIN(name)                          \
  Result BinaryReaderLogging::name(uint32_t size) { \
    LOGF(#name "(%u)\n", size);                     \
    Indent();                                       \
    return reader->name(size);                      \
  }

#define DEFINE_END(name)               \
  Result BinaryReaderLogging::name() { \
    Dedent();                          \
    LOGF(#name "\n");                  \
    return reader->name();             \
  }

#define DEFINE_UINT32(name)                          \
  Result BinaryReaderLogging::name(uint32_t value) { \
    LOGF(#name "(%u)\n", value);                     \
    return reader->name(value);                      \
  }

#define DEFINE_UINT32_DESC(name, desc)               \
  Result BinaryReaderLogging::name(uint32_t value) { \
    LOGF(#name "(" desc ": %u)\n", value);           \
    return reader->name(value);                      \
  }

#define DEFINE_UINT32_UINT32(name, desc0, desc1)                       \
  Result BinaryReaderLogging::name(uint32_t value0, uint32_t value1) { \
    LOGF(#name "(" desc0 ": %u, " desc1 ": %u)\n", value0, value1);    \
    return reader->name(value0, value1);                               \
  }

#define DEFINE_OPCODE(name)                                \
  Result BinaryReaderLogging::name(Opcode opcode) {        \
    LOGF(#name "(\"%s\" (%u))\n", get_opcode_name(opcode), \
         static_cast<unsigned>(opcode));                   \
    return reader->name(opcode);                           \
  }

#define DEFINE0(name)                  \
  Result BinaryReaderLogging::name() { \
    LOGF(#name "\n");                  \
    return reader->name();             \
  }

DEFINE_END(EndModule)

DEFINE_END(EndCustomSection)

DEFINE_BEGIN(BeginTypeSection)
DEFINE_UINT32(OnTypeCount)
DEFINE_END(EndTypeSection)

DEFINE_BEGIN(BeginImportSection)
DEFINE_UINT32(OnImportCount)
DEFINE_END(EndImportSection)

DEFINE_BEGIN(BeginFunctionSection)
DEFINE_UINT32(OnFunctionCount)
DEFINE_UINT32_UINT32(OnFunction, "index", "sig_index")
DEFINE_END(EndFunctionSection)

DEFINE_BEGIN(BeginTableSection)
DEFINE_UINT32(OnTableCount)
DEFINE_END(EndTableSection)

DEFINE_BEGIN(BeginMemorySection)
DEFINE_UINT32(OnMemoryCount)
DEFINE_END(EndMemorySection)

DEFINE_BEGIN(BeginGlobalSection)
DEFINE_UINT32(OnGlobalCount)
DEFINE_UINT32(BeginGlobalInitExpr)
DEFINE_UINT32(EndGlobalInitExpr)
DEFINE_UINT32(EndGlobal)
DEFINE_END(EndGlobalSection)

DEFINE_BEGIN(BeginExportSection)
DEFINE_UINT32(OnExportCount)
DEFINE_END(EndExportSection)

DEFINE_BEGIN(BeginStartSection)
DEFINE_UINT32(OnStartFunction)
DEFINE_END(EndStartSection)

DEFINE_BEGIN(BeginCodeSection)
DEFINE_UINT32(OnFunctionBodyCount)
DEFINE_UINT32(BeginFunctionBody)
DEFINE_UINT32(EndFunctionBody)
DEFINE_UINT32(OnLocalDeclCount)
DEFINE_OPCODE(OnBinaryExpr)
DEFINE_UINT32_DESC(OnCallExpr, "func_index")
DEFINE_UINT32_DESC(OnCallIndirectExpr, "sig_index")
DEFINE_OPCODE(OnCompareExpr)
DEFINE_OPCODE(OnConvertExpr)
DEFINE0(OnCurrentMemoryExpr)
DEFINE0(OnDropExpr)
DEFINE0(OnElseExpr)
DEFINE0(OnEndExpr)
DEFINE_UINT32_DESC(OnGetGlobalExpr, "index")
DEFINE_UINT32_DESC(OnGetLocalExpr, "index")
DEFINE0(OnGrowMemoryExpr)
DEFINE0(OnNopExpr)
DEFINE0(OnReturnExpr)
DEFINE0(OnSelectExpr)
DEFINE_UINT32_DESC(OnSetGlobalExpr, "index")
DEFINE_UINT32_DESC(OnSetLocalExpr, "index")
DEFINE_UINT32_DESC(OnTeeLocalExpr, "index")
DEFINE0(OnUnreachableExpr)
DEFINE_OPCODE(OnUnaryExpr)
DEFINE_END(EndCodeSection)

DEFINE_BEGIN(BeginElemSection)
DEFINE_UINT32(OnElemSegmentCount)
DEFINE_UINT32_UINT32(BeginElemSegment, "index", "table_index")
DEFINE_UINT32(BeginElemSegmentInitExpr)
DEFINE_UINT32(EndElemSegmentInitExpr)
DEFINE_UINT32_UINT32(OnElemSegmentFunctionIndexCount, "index", "count")
DEFINE_UINT32_UINT32(OnElemSegmentFunctionIndex, "index", "func_index")
DEFINE_UINT32(EndElemSegment)
DEFINE_END(EndElemSection)

DEFINE_BEGIN(BeginDataSection)
DEFINE_UINT32(OnDataSegmentCount)
DEFINE_UINT32_UINT32(BeginDataSegment, "index", "memory_index")
DEFINE_UINT32(BeginDataSegmentInitExpr)
DEFINE_UINT32(EndDataSegmentInitExpr)
DEFINE_UINT32(EndDataSegment)
DEFINE_END(EndDataSection)

DEFINE_BEGIN(BeginNamesSection)
DEFINE_UINT32(OnFunctionNamesCount)
DEFINE_UINT32(OnLocalNameFunctionCount)
DEFINE_UINT32_UINT32(OnLocalNameLocalCount, "index", "count")
DEFINE_END(EndNamesSection)

DEFINE_BEGIN(BeginRelocSection)
DEFINE_END(EndRelocSection)
DEFINE_UINT32_UINT32(OnInitExprGetGlobalExpr, "index", "global_index")

// We don't need to log these (the individual opcodes are logged instead), but
// we still need to forward the calls.
Result BinaryReaderLogging::OnOpcode(Opcode opcode) {
  return reader->OnOpcode(opcode);
}

Result BinaryReaderLogging::OnOpcodeBare() {
  return reader->OnOpcodeBare();
}

Result BinaryReaderLogging::OnOpcodeUint32(uint32_t value) {
  return reader->OnOpcodeUint32(value);
}

Result BinaryReaderLogging::OnOpcodeUint32Uint32(uint32_t value,
                                                 uint32_t value2) {
  return reader->OnOpcodeUint32Uint32(value, value2);
}

Result BinaryReaderLogging::OnOpcodeUint64(uint64_t value) {
  return reader->OnOpcodeUint64(value);
}

Result BinaryReaderLogging::OnOpcodeF32(uint32_t value) {
  return reader->OnOpcodeF32(value);
}

Result BinaryReaderLogging::OnOpcodeF64(uint64_t value) {
  return reader->OnOpcodeF64(value);
}

Result BinaryReaderLogging::OnOpcodeBlockSig(uint32_t num_types,
                                             Type* sig_types) {
  return reader->OnOpcodeBlockSig(num_types, sig_types);
}

Result BinaryReaderLogging::OnEndFunc() {
  return reader->OnEndFunc();
}

}  // namespace wabt
