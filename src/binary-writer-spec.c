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

typedef struct Context {
  WasmAllocator* allocator;
  WasmMemoryWriter json_writer;
  WasmStream json_stream;
  const char* source_filename;
  WasmStringSlice json_filename_noext;
  const WasmWriteBinarySpecOptions* spec_options;
  WasmResult result;
  size_t num_modules;
} Context;

static void convert_backslash_to_slash(char* s, size_t length) {
  size_t i = 0;
  for (; i < length; ++i)
    if (s[i] == '\\')
      s[i] = '/';
}

static WasmStringSlice strip_extension(const char* s) {
  /* strip .json or .wasm, but leave other extensions, e.g.:
   *
   * s = "foo", => "foo"
   * s = "foo.json" => "foo"
   * s = "foo.wasm" => "foo"
   * s = "foo.bar" => "foo.bar"
   */
  if (s == NULL) {
    WasmStringSlice result;
    result.start = NULL;
    result.length = 0;
    return result;
  }

  size_t slen = strlen(s);
  const char* ext_start = strrchr(s, '.');
  if (ext_start == NULL)
    ext_start = s + slen;

  WasmStringSlice result;
  result.start = s;

  if (strcmp(ext_start, ".json") == 0 || strcmp(ext_start, ".wasm") == 0) {
    result.length = ext_start - s;
  } else {
    result.length = slen;
  }
  return result;
}

static WasmStringSlice get_basename(const char* s) {
  /* strip everything up to and including the last slash, e.g.:
   *
   * s = "/foo/bar/baz", => "baz"
   * s = "/usr/local/include/stdio.h", => "stdio.h"
   * s = "foo.bar", => "foo.bar"
   */
  size_t slen = strlen(s);
  const char* start = s;
  const char* last_slash = strrchr(s, '/');
  if (last_slash != NULL)
    start = last_slash + 1;

  WasmStringSlice result;
  result.start = start;
  result.length = s + slen - start;
  return result;
}

static char* get_module_filename(Context* ctx) {
  size_t buflen = ctx->json_filename_noext.length + 20;
  char* str = wasm_alloc(ctx->allocator, buflen, WASM_DEFAULT_ALIGN);
  size_t length = wasm_snprintf(
      str, buflen, PRIstringslice ".%" PRIzd ".wasm",
      WASM_PRINTF_STRING_SLICE_ARG(ctx->json_filename_noext), ctx->num_modules);
  convert_backslash_to_slash(str, length);
  return str;
}

static void write_string(Context* ctx, const char* s) {
  wasm_writef(&ctx->json_stream, "\"%s\"", s);
}

static void write_key(Context* ctx, const char* key) {
  wasm_writef(&ctx->json_stream, "\"%s\": ", key);
}

static void write_separator(Context* ctx) {
  wasm_writef(&ctx->json_stream, ", ");
}

static void write_escaped_string_slice(Context* ctx, WasmStringSlice ss) {
  size_t i;
  wasm_write_char(&ctx->json_stream, '"');
  for (i = 0; i < ss.length; ++i) {
    uint8_t c = ss.start[i];
    if (c < 0x20 || c == '\\' || c == '"') {
      wasm_writef(&ctx->json_stream, "\\u%04x", c);
    } else {
      wasm_write_char(&ctx->json_stream, c);
    }
  }
  wasm_write_char(&ctx->json_stream, '"');
}

static void write_command_type(Context* ctx, const WasmCommand* command) {
  static const char* s_command_names[] = {
      "module",
      "action",
      "register",
      "assert_malformed",
      "assert_invalid",
      "assert_unlinkable",
      "assert_uninstantiable",
      "assert_return",
      "assert_return_nan",
      "assert_trap",
  };
  WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_command_names) ==
                     WASM_NUM_COMMAND_TYPES);

  write_key(ctx, "type");
  write_string(ctx, s_command_names[command->type]);
}

static void write_location(Context* ctx, const WasmLocation* loc) {
  write_key(ctx, "line");
  wasm_writef(&ctx->json_stream, "%d", loc->line);
}

static void write_var(Context* ctx, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_INDEX)
    wasm_writef(&ctx->json_stream, "\"%" PRIu64 "\"", var->index);
  else
    write_escaped_string_slice(ctx, var->name);
}

static void write_const(Context* ctx, const WasmConst* const_) {
  wasm_writef(&ctx->json_stream, "{");
  write_key(ctx, "type");

  /* Always write the values as strings, even though they may be representable
   * as JSON numbers. This way the formatting is consistent. */
  switch (const_->type) {
    case WASM_TYPE_I32:
      write_string(ctx, "i32");
      write_separator(ctx);
      write_key(ctx, "value");
      wasm_writef(&ctx->json_stream, "\"%u\"", const_->u32);
      break;

    case WASM_TYPE_I64:
      write_string(ctx, "i64");
      write_separator(ctx);
      write_key(ctx, "value");
      wasm_writef(&ctx->json_stream, "\"%" PRIu64 "\"", const_->u64);
      break;

    case WASM_TYPE_F32: {
      /* TODO(binji): write as hex float */
      write_string(ctx, "f32");
      write_separator(ctx);
      write_key(ctx, "value");
      wasm_writef(&ctx->json_stream, "\"%u\"", const_->f32_bits);
      break;
    }

    case WASM_TYPE_F64: {
      /* TODO(binji): write as hex float */
      write_string(ctx, "f64");
      write_separator(ctx);
      write_key(ctx, "value");
      wasm_writef(&ctx->json_stream, "\"%" PRIu64 "\"", const_->f64_bits);
      break;
    }

    default:
      assert(0);
  }

  wasm_writef(&ctx->json_stream, "}");
}

static void write_const_vector(Context* ctx, const WasmConstVector* consts) {
  wasm_writef(&ctx->json_stream, "[");
  size_t i;
  for (i = 0; i < consts->size; ++i) {
    const WasmConst* const_ = &consts->data[i];
    write_const(ctx, const_);
    if (i != consts->size - 1)
      write_separator(ctx);
  }
  wasm_writef(&ctx->json_stream, "]");
}

static void write_action(Context* ctx, const WasmAction* action) {
  write_key(ctx, "action");
  wasm_writef(&ctx->json_stream, "{");
  write_key(ctx, "type");
  if (action->type == WASM_ACTION_TYPE_INVOKE) {
    write_string(ctx, "invoke");
  } else {
    assert(action->type == WASM_ACTION_TYPE_GET);
    write_string(ctx, "get");
  }
  write_separator(ctx);
  if (action->module_var.type != WASM_VAR_TYPE_INDEX) {
    write_key(ctx, "module");
    write_var(ctx, &action->module_var);
    write_separator(ctx);
  }
  if (action->type == WASM_ACTION_TYPE_INVOKE) {
    write_key(ctx, "field");
    write_escaped_string_slice(ctx, action->invoke.name);
    write_separator(ctx);
    write_key(ctx, "args");
    write_const_vector(ctx, &action->invoke.args);
  } else {
    write_key(ctx, "field");
    write_escaped_string_slice(ctx, action->get.name);
  }
  wasm_writef(&ctx->json_stream, "}");
}

static void write_module(Context* ctx,
                         char* filename,
                         const WasmModule* module) {
  WasmMemoryWriter writer;
  WasmResult result = wasm_init_mem_writer(ctx->allocator, &writer);
  if (WASM_SUCCEEDED(result)) {
    result = wasm_write_binary_module(ctx->allocator, &writer.base, module,
                                      &ctx->spec_options->write_binary_options);
    if (WASM_SUCCEEDED(result))
      result = wasm_write_output_buffer_to_file(&writer.buf, filename);
    wasm_close_mem_writer(&writer);
  }

  ctx->result = result;
}

static void write_raw_module(Context* ctx,
                             char* filename,
                             const WasmRawModule* raw_module) {
  if (raw_module->type == WASM_RAW_MODULE_TYPE_TEXT) {
    write_module(ctx, filename, raw_module->text);
  } else {
    WasmFileStream stream;
    WasmResult result = wasm_init_file_writer(&stream.writer, filename);
    if (WASM_SUCCEEDED(result)) {
      wasm_init_stream(&stream.base, &stream.writer.base, NULL);
      wasm_write_data(&stream.base, raw_module->binary.data,
                      raw_module->binary.size, "");
      wasm_close_file_writer(&stream.writer);
    }
    ctx->result = result;
  }
}

static void write_invalid_module(Context* ctx,
                                 const WasmRawModule* module,
                                 WasmStringSlice text) {
  char* filename = get_module_filename(ctx);
  write_location(ctx, wasm_get_raw_module_location(module));
  write_separator(ctx);
  write_key(ctx, "filename");
  write_escaped_string_slice(ctx, get_basename(filename));
  write_separator(ctx);
  write_key(ctx, "text");
  write_escaped_string_slice(ctx, text);
  write_raw_module(ctx, filename, module);
  wasm_free(ctx->allocator, filename);
}

static void write_commands(Context* ctx, WasmScript* script) {
  wasm_writef(&ctx->json_stream, "{\"source_filename\": \"%s\",\n",
              ctx->source_filename);
  wasm_writef(&ctx->json_stream, " \"commands\": [\n");
  size_t i;
  int last_module_index = -1;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];

    wasm_writef(&ctx->json_stream, "  {");
    write_command_type(ctx, command);
    write_separator(ctx);

    switch (command->type) {
      case WASM_COMMAND_TYPE_MODULE: {
        WasmModule* module = &command->module;
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
        wasm_free(ctx->allocator, filename);
        ctx->num_modules++;
        last_module_index = (int)i;
        break;
      }

      case WASM_COMMAND_TYPE_ACTION:
        write_location(ctx, &command->action.loc);
        write_separator(ctx);
        write_action(ctx, &command->action);
        break;

      case WASM_COMMAND_TYPE_REGISTER:
        write_location(ctx, &command->register_.var.loc);
        write_separator(ctx);
        if (command->register_.var.type == WASM_VAR_TYPE_NAME) {
          write_key(ctx, "name");
          write_var(ctx, &command->register_.var);
          write_separator(ctx);
        } else {
          /* If we're not registering by name, then we should only be
           * registering the last module. */
          WASM_USE(last_module_index);
          assert(command->register_.var.index == last_module_index);
        }
        write_key(ctx, "as");
        write_escaped_string_slice(ctx, command->register_.module_name);
        break;

      case WASM_COMMAND_TYPE_ASSERT_MALFORMED:
        write_invalid_module(ctx, &command->assert_malformed.module,
                             command->assert_malformed.text);
        ctx->num_modules++;
        break;

      case WASM_COMMAND_TYPE_ASSERT_INVALID:
        /* TODO(binji): this doesn't currently work because of various
         * assertions in wasm-binary-writer.c */
#if 0
        write_invalid_module(ctx, &command->assert_invalid.module,
                             command->assert_invalid.text);
        ctx->num_modules++;
#else
        write_location(
            ctx, wasm_get_raw_module_location(&command->assert_invalid.module));
#endif
        break;

      case WASM_COMMAND_TYPE_ASSERT_UNLINKABLE:
        write_invalid_module(ctx, &command->assert_unlinkable.module,
                             command->assert_unlinkable.text);
        ctx->num_modules++;
        break;

      case WASM_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
        write_invalid_module(ctx, &command->assert_uninstantiable.module,
                             command->assert_uninstantiable.text);
        ctx->num_modules++;
        break;

      case WASM_COMMAND_TYPE_ASSERT_RETURN:
        write_location(ctx, &command->assert_return.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_return.action);
        write_separator(ctx);
        write_key(ctx, "expected");
        write_const_vector(ctx, &command->assert_return.expected);
        break;

      case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
        write_location(ctx, &command->assert_return_nan.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_return_nan.action);
        break;

      case WASM_COMMAND_TYPE_ASSERT_TRAP:
        write_location(ctx, &command->assert_trap.action.loc);
        write_separator(ctx);
        write_action(ctx, &command->assert_trap.action);
        write_separator(ctx);
        write_key(ctx, "text");
        write_escaped_string_slice(ctx, command->assert_trap.text);
        break;

      case WASM_NUM_COMMAND_TYPES:
        assert(0);
        break;
    }

    wasm_writef(&ctx->json_stream, "}");
    if (i != script->commands.size - 1)
      write_separator(ctx);
    wasm_writef(&ctx->json_stream, "\n");
  }
  wasm_writef(&ctx->json_stream, "]}\n");
}

WasmResult wasm_write_binary_spec_script(
    WasmAllocator* allocator,
    WasmScript* script,
    const char* source_filename,
    const WasmWriteBinarySpecOptions* spec_options) {
  assert(source_filename);
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.spec_options = spec_options;
  ctx.result = WASM_OK;
  ctx.source_filename = source_filename;
  ctx.json_filename_noext = strip_extension(ctx.spec_options->json_filename);

  WasmResult result = wasm_init_mem_writer(ctx.allocator, &ctx.json_writer);
  if (WASM_SUCCEEDED(result)) {
    wasm_init_stream(&ctx.json_stream, &ctx.json_writer.base, NULL);
    write_commands(&ctx, script);
    if (ctx.spec_options->json_filename) {
      wasm_write_output_buffer_to_file(&ctx.json_writer.buf,
                                       ctx.spec_options->json_filename);
    }
    wasm_close_mem_writer(&ctx.json_writer);
  }

  return ctx.result;
}
