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

#include "binary-writer-spec.h"

#include <assert.h>
#include <inttypes.h>

#include "ast.h"
#include "binary.h"
#include "binary-writer.h"
#include "config.h"
#include "stream.h"
#include "writer.h"

struct Context {
  WabtMemoryWriter json_writer;
  WabtStream json_stream;
  WabtStringSlice source_filename;
  WabtStringSlice module_filename_noext;
  bool write_modules; /* Whether to write the modules files. */
  const WabtWriteBinarySpecOptions* spec_options;
  WabtResult result;
  size_t num_modules;
};

static void convert_backslash_to_slash(char* s, size_t length) {
  size_t i = 0;
  for (; i < length; ++i)
    if (s[i] == '\\')
      s[i] = '/';
}

static WabtStringSlice strip_extension(const char* s) {
  /* strip .json or .wasm, but leave other extensions, e.g.:
   *
   * s = "foo", => "foo"
   * s = "foo.json" => "foo"
   * s = "foo.wasm" => "foo"
   * s = "foo.bar" => "foo.bar"
   */
  if (!s) {
    WabtStringSlice result;
    result.start = nullptr;
    result.length = 0;
    return result;
  }

  size_t slen = strlen(s);
  const char* ext_start = strrchr(s, '.');
  if (!ext_start)
    ext_start = s + slen;

  WabtStringSlice result;
  result.start = s;

  if (strcmp(ext_start, ".json") == 0 || strcmp(ext_start, ".wasm") == 0) {
    result.length = ext_start - s;
  } else {
    result.length = slen;
  }
  return result;
}

static WabtStringSlice get_basename(const char* s) {
  /* strip everything up to and including the last slash, e.g.:
   *
   * s = "/foo/bar/baz", => "baz"
   * s = "/usr/local/include/stdio.h", => "stdio.h"
   * s = "foo.bar", => "foo.bar"
   */
  size_t slen = strlen(s);
  const char* start = s;
  const char* last_slash = strrchr(s, '/');
  if (last_slash)
    start = last_slash + 1;

  WabtStringSlice result;
  result.start = start;
  result.length = s + slen - start;
  return result;
}

static char* get_module_filename(Context* ctx) {
  size_t buflen = ctx->module_filename_noext.length + 20;
  char* str = static_cast<char*>(wabt_alloc(buflen));
  size_t length =
      wabt_snprintf(str, buflen, PRIstringslice ".%" PRIzd ".wasm",
                    WABT_PRINTF_STRING_SLICE_ARG(ctx->module_filename_noext),
                    ctx->num_modules);
  convert_backslash_to_slash(str, length);
  return str;
}

static void write_string(Context* ctx, const char* s) {
  wabt_writef(&ctx->json_stream, "\"%s\"", s);
}

static void write_key(Context* ctx, const char* key) {
  wabt_writef(&ctx->json_stream, "\"%s\": ", key);
}

static void write_separator(Context* ctx) {
  wabt_writef(&ctx->json_stream, ", ");
}

static void write_escaped_string_slice(Context* ctx, WabtStringSlice ss) {
  size_t i;
  wabt_write_char(&ctx->json_stream, '"');
  for (i = 0; i < ss.length; ++i) {
    uint8_t c = ss.start[i];
    if (c < 0x20 || c == '\\' || c == '"') {
      wabt_writef(&ctx->json_stream, "\\u%04x", c);
    } else {
      wabt_write_char(&ctx->json_stream, c);
    }
  }
  wabt_write_char(&ctx->json_stream, '"');
}

static void write_command_type(Context* ctx, const WabtCommand* command) {
  static const char* s_command_names[] =
      {
          "module",
          "action",
          "register",
          "assert_malformed",
          "assert_invalid",
          nullptr, /* ASSERT_INVALID_NON_BINARY, this command will never be
                      written */
          "assert_unlinkable",
          "assert_uninstantiable",
          "assert_return",
          "assert_return_nan",
          "assert_trap",
          "assert_exhaustion",
      };
  WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(s_command_names) ==
                     WABT_NUM_COMMAND_TYPES);

  write_key(ctx, "type");
  assert(s_command_names[command->type]);
  write_string(ctx, s_command_names[command->type]);
}

static void write_location(Context* ctx, const WabtLocation* loc) {
  write_key(ctx, "line");
  wabt_writef(&ctx->json_stream, "%d", loc->line);
}

static void write_var(Context* ctx, const WabtVar* var) {
  if (var->type == WABT_VAR_TYPE_INDEX)
    wabt_writef(&ctx->json_stream, "\"%" PRIu64 "\"", var->index);
  else
    write_escaped_string_slice(ctx, var->name);
}

static void write_type_object(Context* ctx, WabtType type) {
  wabt_writef(&ctx->json_stream, "{");
  write_key(ctx, "type");
  write_string(ctx, wabt_get_type_name(type));
  wabt_writef(&ctx->json_stream, "}");
}

static void write_const(Context* ctx, const WabtConst* const_) {
  wabt_writef(&ctx->json_stream, "{");
  write_key(ctx, "type");

  /* Always write the values as strings, even though they may be representable
   * as JSON numbers. This way the formatting is consistent. */
  switch (const_->type) {
    case WABT_TYPE_I32:
      write_string(ctx, "i32");
      write_separator(ctx);
      write_key(ctx, "value");
      wabt_writef(&ctx->json_stream, "\"%u\"", const_->u32);
      break;

    case WABT_TYPE_I64:
      write_string(ctx, "i64");
      write_separator(ctx);
      write_key(ctx, "value");
      wabt_writef(&ctx->json_stream, "\"%" PRIu64 "\"", const_->u64);
      break;

    case WABT_TYPE_F32: {
      /* TODO(binji): write as hex float */
      write_string(ctx, "f32");
      write_separator(ctx);
      write_key(ctx, "value");
      wabt_writef(&ctx->json_stream, "\"%u\"", const_->f32_bits);
      break;
    }

    case WABT_TYPE_F64: {
      /* TODO(binji): write as hex float */
      write_string(ctx, "f64");
      write_separator(ctx);
      write_key(ctx, "value");
      wabt_writef(&ctx->json_stream, "\"%" PRIu64 "\"", const_->f64_bits);
      break;
    }

    default:
      assert(0);
  }

  wabt_writef(&ctx->json_stream, "}");
}

static void write_const_vector(Context* ctx, const WabtConstVector* consts) {
  wabt_writef(&ctx->json_stream, "[");
  size_t i;
  for (i = 0; i < consts->size; ++i) {
    const WabtConst* const_ = &consts->data[i];
    write_const(ctx, const_);
    if (i != consts->size - 1)
      write_separator(ctx);
  }
  wabt_writef(&ctx->json_stream, "]");
}

static void write_action(Context* ctx, const WabtAction* action) {
  write_key(ctx, "action");
  wabt_writef(&ctx->json_stream, "{");
  write_key(ctx, "type");
  if (action->type == WABT_ACTION_TYPE_INVOKE) {
    write_string(ctx, "invoke");
  } else {
    assert(action->type == WABT_ACTION_TYPE_GET);
    write_string(ctx, "get");
  }
  write_separator(ctx);
  if (action->module_var.type != WABT_VAR_TYPE_INDEX) {
    write_key(ctx, "module");
    write_var(ctx, &action->module_var);
    write_separator(ctx);
  }
  if (action->type == WABT_ACTION_TYPE_INVOKE) {
    write_key(ctx, "field");
    write_escaped_string_slice(ctx, action->invoke.name);
    write_separator(ctx);
    write_key(ctx, "args");
    write_const_vector(ctx, &action->invoke.args);
  } else {
    write_key(ctx, "field");
    write_escaped_string_slice(ctx, action->get.name);
  }
  wabt_writef(&ctx->json_stream, "}");
}

static void write_action_result_type(Context* ctx,
                                     WabtScript* script,
                                     const WabtAction* action) {
  const WabtModule* module =
      wabt_get_module_by_var(script, &action->module_var);
  const WabtExport* export_;
  wabt_writef(&ctx->json_stream, "[");
  switch (action->type) {
    case WABT_ACTION_TYPE_INVOKE: {
      export_ = wabt_get_export_by_name(module, &action->invoke.name);
      assert(export_->kind == WABT_EXTERNAL_KIND_FUNC);
      WabtFunc* func = wabt_get_func_by_var(module, &export_->var);
      size_t num_results = wabt_get_num_results(func);
      size_t i;
      for (i = 0; i < num_results; ++i)
        write_type_object(ctx, wabt_get_result_type(func, i));
      break;
    }

    case WABT_ACTION_TYPE_GET: {
      export_ = wabt_get_export_by_name(module, &action->get.name);
      assert(export_->kind == WABT_EXTERNAL_KIND_GLOBAL);
      WabtGlobal* global = wabt_get_global_by_var(module, &export_->var);
      write_type_object(ctx, global->type);
      break;
    }
  }
  wabt_writef(&ctx->json_stream, "]");
}

static void write_module(Context* ctx,
                         char* filename,
                         const WabtModule* module) {
  WabtMemoryWriter writer;
  WabtResult result = wabt_init_mem_writer(&writer);
  if (WABT_SUCCEEDED(result)) {
    WabtWriteBinaryOptions options = ctx->spec_options->write_binary_options;
    result = wabt_write_binary_module(&writer.base, module, &options);
    if (WABT_SUCCEEDED(result) && ctx->write_modules)
      result = wabt_write_output_buffer_to_file(&writer.buf, filename);
    wabt_close_mem_writer(&writer);
  }

  ctx->result = result;
}

static void write_raw_module(Context* ctx,
                             char* filename,
                             const WabtRawModule* raw_module) {
  if (raw_module->type == WABT_RAW_MODULE_TYPE_TEXT) {
    write_module(ctx, filename, raw_module->text);
  } else if (ctx->write_modules) {
    WabtFileStream stream;
    WabtResult result = wabt_init_file_writer(&stream.writer, filename);
    if (WABT_SUCCEEDED(result)) {
      wabt_init_stream(&stream.base, &stream.writer.base, nullptr);
      wabt_write_data(&stream.base, raw_module->binary.data,
                      raw_module->binary.size, "");
      wabt_close_file_writer(&stream.writer);
    }
    ctx->result = result;
  }
}

static void write_invalid_module(Context* ctx,
                                 const WabtRawModule* module,
                                 WabtStringSlice text) {
  char* filename = get_module_filename(ctx);
  write_location(ctx, wabt_get_raw_module_location(module));
  write_separator(ctx);
  write_key(ctx, "filename");
  write_escaped_string_slice(ctx, get_basename(filename));
  write_separator(ctx);
  write_key(ctx, "text");
  write_escaped_string_slice(ctx, text);
  write_raw_module(ctx, filename, module);
  wabt_free(filename);
}

static void write_commands(Context* ctx, WabtScript* script) {
  wabt_writef(&ctx->json_stream, "{\"source_filename\": ");
  write_escaped_string_slice(ctx, ctx->source_filename);
  wabt_writef(&ctx->json_stream, ",\n \"commands\": [\n");
  size_t i;
  int last_module_index = -1;
  for (i = 0; i < script->commands.size; ++i) {
    WabtCommand* command = &script->commands.data[i];

    if (command->type == WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY)
      continue;

    if (i != 0)
      write_separator(ctx);
    wabt_writef(&ctx->json_stream, "\n");

    wabt_writef(&ctx->json_stream, "  {");
    write_command_type(ctx, command);
    write_separator(ctx);

    switch (command->type) {
      case WABT_COMMAND_TYPE_MODULE: {
        WabtModule* module = &command->module;
        char* filename = get_module_filename(ctx);
        write_location(ctx, &module->loc);
        write_separator(ctx);
        if (module->name.start) {
          write_key(ctx, "name");
          write_escaped_string_slice(ctx, module->name);
          write_separator(ctx);
        }
        write_key(ctx, "filename");
        write_escaped_string_slice(ctx, get_basename(filename));
        write_module(ctx, filename, module);
        wabt_free(filename);
        ctx->num_modules++;
        last_module_index = static_cast<int>(i);
        break;
      }

      case WABT_COMMAND_TYPE_ACTION:
        write_location(ctx, &command->action.loc);
        write_separator(ctx);
        write_action(ctx, &command->action);
        break;

      case WABT_COMMAND_TYPE_REGISTER:
        write_location(ctx, &command->register_.var.loc);
        write_separator(ctx);
        if (command->register_.var.type == WABT_VAR_TYPE_NAME) {
          write_key(ctx, "name");
          write_var(ctx, &command->register_.var);
          write_separator(ctx);
        } else {
          /* If we're not registering by name, then we should only be
           * registering the last module. */
          WABT_USE(last_module_index);
          assert(command->register_.var.index == last_module_index);
        }
        write_key(ctx, "as");
        write_escaped_string_slice(ctx, command->register_.module_name);
        break;

      case WABT_COMMAND_TYPE_ASSERT_MALFORMED:
        write_invalid_module(ctx, &command->assert_malformed.module,
                             command->assert_malformed.text);
        ctx->num_modules++;
        break;

      case WABT_COMMAND_TYPE_ASSERT_INVALID:
        write_invalid_module(ctx, &command->assert_invalid.module,
                             command->assert_invalid.text);
        ctx->num_modules++;
        break;

      case WABT_COMMAND_TYPE_ASSERT_UNLINKABLE:
        write_invalid_module(ctx, &command->assert_unlinkable.module,
                             command->assert_unlinkable.text);
        ctx->num_modules++;
        break;

      case WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
        write_invalid_module(ctx, &command->assert_uninstantiable.module,
                             command->assert_uninstantiable.text);
        ctx->num_modules++;
        break;

      case WABT_COMMAND_TYPE_ASSERT_RETURN:
        write_location(ctx, &command->assert_return.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_return.action);
        write_separator(ctx);
        write_key(ctx, "expected");
        write_const_vector(ctx, &command->assert_return.expected);
        break;

      case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN:
        write_location(ctx, &command->assert_return_nan.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_return_nan.action);
        write_separator(ctx);
        write_key(ctx, "expected");
        write_action_result_type(ctx, script,
                                 &command->assert_return_nan.action);
        break;

      case WABT_COMMAND_TYPE_ASSERT_TRAP:
        write_location(ctx, &command->assert_trap.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_trap.action);
        write_separator(ctx);
        write_key(ctx, "text");
        write_escaped_string_slice(ctx, command->assert_trap.text);
        break;

      case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
        write_location(ctx, &command->assert_trap.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_trap.action);
        break;

      case WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY:
      case WABT_NUM_COMMAND_TYPES:
        assert(0);
        break;
    }

    wabt_writef(&ctx->json_stream, "}");
  }
  wabt_writef(&ctx->json_stream, "]}\n");
}

WabtResult wabt_write_binary_spec_script(
    WabtScript* script,
    const char* source_filename,
    const WabtWriteBinarySpecOptions* spec_options) {
  assert(source_filename);
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.spec_options = spec_options;
  ctx.result = WABT_OK;
  ctx.source_filename.start = source_filename;
  ctx.source_filename.length = strlen(source_filename);
  ctx.module_filename_noext = strip_extension(
      ctx.spec_options->json_filename ? ctx.spec_options->json_filename
                                      : source_filename);
  ctx.write_modules = !!ctx.spec_options->json_filename;

  WabtResult result = wabt_init_mem_writer(&ctx.json_writer);
  if (WABT_SUCCEEDED(result)) {
    wabt_init_stream(&ctx.json_stream, &ctx.json_writer.base, nullptr);
    write_commands(&ctx, script);
    if (ctx.spec_options->json_filename) {
      wabt_write_output_buffer_to_file(&ctx.json_writer.buf,
                                       ctx.spec_options->json_filename);
    }
    wabt_close_mem_writer(&ctx.json_writer);
  }

  return ctx.result;
}
