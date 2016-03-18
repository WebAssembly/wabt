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

#include "wasm-ast-writer.h"

#include <stdarg.h>
#include <stdio.h>

#include "wasm-ast.h"
#include "wasm-writer.h"

typedef struct WasmWriteContext {
  WasmWriter* writer;
  size_t offset;
  WasmResult result;
} WasmWriteContext;

static void out_data_at(WasmWriteContext* ctx,
                        size_t offset,
                        const void* src,
                        size_t size,
                        const char* desc) {
  if (ctx->result != WASM_OK)
    return;
  if (ctx->writer->write_data) {
    ctx->result =
        ctx->writer->write_data(offset, src, size, ctx->writer->user_data);
  }
}

static void out_printf(WasmWriteContext* ctx, const char* format, ...) {
  va_list args;
  va_list args_copy;
  va_start(args, format);
  va_copy(args_copy, args);

  char buffer[128];
  int len = wasm_vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  if (len >= sizeof(buffer)) {
    char* buffer2 = alloca(len + 1);
    len = wasm_vsnprintf(buffer2, len + 1, format, args_copy);
    va_end(args_copy);
  }

  /* - 1 to remove the trailing \0 that was added by wasm_vsnprintf */
  out_data_at(ctx, ctx->offset, buffer, len - 1, "");
  ctx->offset += len - 1;
}

static void out_newline(WasmWriteContext* ctx) {
}

static void out_indent(WasmWriteContext* ctx) {
}

static void out_dedent(WasmWriteContext* ctx) {
}

static void out_begin(WasmWriteContext* ctx, const char* name) {
  out_printf(ctx, "(%s", name);
  out_indent(ctx);
  out_newline(ctx);
}

static void out_end(WasmWriteContext* ctx) {
  out_dedent(ctx);
  out_printf(ctx, ")");
}

static void write_func(WasmWriteContext* ctx, WasmFunc* func) {
}

static void write_import(WasmWriteContext* ctx, WasmImport* import) {
}

static void write_export(WasmWriteContext* ctx, WasmExport* export) {
}

static void write_export_memory(WasmWriteContext* ctx,
                                WasmExportMemory* export_memory) {}

static void write_table(WasmWriteContext* ctx, WasmVarVector* table) {}

static void write_segment(WasmWriteContext* ctx, WasmSegment* segment) {
  out_begin(ctx, "segment");
  out_printf(ctx, "%u", segment->addr);
  /* TODO(binji): optimize */
  uint8_t* data = segment->data;
  size_t i;
  for (i = 0; i < segment->size; ++i) {
    uint8_t b = data[i];
    if (b >= 32 && b < 127)
      out_printf(ctx, "%c", b);
    else
      out_printf(ctx, "\\%02x", b);
  }
  out_end(ctx);
}

static void write_memory(WasmWriteContext* ctx, WasmMemory* memory) {
  out_begin(ctx, "memory");
  out_printf(ctx, "%u %u", memory->initial_pages, memory->max_pages);
  int i;
  for (i = 0; i < memory->segments.size; ++i) {
    WasmSegment* segment = &memory->segments.data[i];
    write_segment(ctx, segment);
  }
  out_end(ctx);
}

static void write_func_type(WasmWriteContext* ctx, WasmFuncType* func_type) {}

static void write_start_function(WasmWriteContext* ctx, WasmVar* start) {}

static void write_module(WasmWriteContext* ctx, WasmModule* module) {
  out_begin(ctx, "module");
  WasmModuleField* field;
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        write_func(ctx, &field->func);
        break;
      case WASM_MODULE_FIELD_TYPE_IMPORT:
        write_import(ctx, &field->import);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT:
        write_export(ctx, &field->export_);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
        write_export_memory(ctx, &field->export_memory);
        break;
      case WASM_MODULE_FIELD_TYPE_TABLE:
        write_table(ctx, &field->table);
        break;
      case WASM_MODULE_FIELD_TYPE_MEMORY:
        write_memory(ctx, &field->memory);
        break;
      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        write_func_type(ctx, &field->func_type);
        break;
      case WASM_MODULE_FIELD_TYPE_START:
        write_start_function(ctx, &field->start);
        break;
    }
  }
  out_end(ctx);
}

WasmResult wasm_write_ast(struct WasmWriter* writer,
                          struct WasmModule* module) {
  WasmWriteContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.result = WASM_OK;
  write_module(&ctx, module);
  return ctx.result;
}
