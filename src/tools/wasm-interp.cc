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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "binary-reader.h"
#include "binary-reader-interpreter.h"
#include "interpreter.h"
#include "literal.h"
#include "option-parser.h"
#include "stream.h"

#define INSTRUCTION_QUANTUM 1000
#define PROGRAM_NAME "wasm-interp"

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static int s_verbose;
static const char* s_infile;
static WabtReadBinaryOptions s_read_binary_options =
    WABT_READ_BINARY_OPTIONS_DEFAULT;
static WabtInterpreterThreadOptions s_thread_options =
    WABT_INTERPRETER_THREAD_OPTIONS_DEFAULT;
static bool s_trace;
static bool s_spec;
static bool s_run_all_exports;
static WabtStream* s_stdout_stream;

static WabtBinaryErrorHandler s_error_handler =
    WABT_BINARY_ERROR_HANDLER_DEFAULT;

static WabtFileWriter s_log_stream_writer;
static WabtStream s_log_stream;

#define NOPE WABT_OPTION_NO_ARGUMENT
#define YEP WABT_OPTION_HAS_ARGUMENT

typedef enum RunVerbosity {
  RUN_QUIET = 0,
  RUN_VERBOSE = 1,
} RunVerbosity;

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_VALUE_STACK_SIZE,
  FLAG_CALL_STACK_SIZE,
  FLAG_TRACE,
  FLAG_SPEC,
  FLAG_RUN_ALL_EXPORTS,
  NUM_FLAGS
};

static const char s_description[] =
    "  read a file in the wasm binary format, and run in it a stack-based\n"
    "  interpreter.\n"
    "\n"
    "examples:\n"
    "  # parse binary file test.wasm, and type-check it\n"
    "  $ wasm-interp test.wasm\n"
    "\n"
    "  # parse test.wasm and run all its exported functions\n"
    "  $ wasm-interp test.wasm --run-all-exports\n"
    "\n"
    "  # parse test.wasm, run the exported functions and trace the output\n"
    "  $ wasm-interp test.wasm --run-all-exports --trace\n"
    "\n"
    "  # parse test.json and run the spec tests\n"
    "  $ wasm-interp test.json --spec\n"
    "\n"
    "  # parse test.wasm and run all its exported functions, setting the\n"
    "  # value stack size to 100 elements\n"
    "  $ wasm-interp test.wasm -V 100 --run-all-exports\n";

static WabtOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", nullptr, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", nullptr, NOPE, "print this help message"},
    {FLAG_VALUE_STACK_SIZE, 'V', "value-stack-size", "SIZE", YEP,
     "size in elements of the value stack"},
    {FLAG_CALL_STACK_SIZE, 'C', "call-stack-size", "SIZE", YEP,
     "size in frames of the call stack"},
    {FLAG_TRACE, 't', "trace", nullptr, NOPE, "trace execution"},
    {FLAG_SPEC, 0, "spec", nullptr, NOPE,
     "run spec tests (input file should be .json)"},
    {FLAG_RUN_ALL_EXPORTS, 0, "run-all-exports", nullptr, NOPE,
     "run all the exported functions, in order. useful for testing"},
};
WABT_STATIC_ASSERT(NUM_FLAGS == WABT_ARRAY_SIZE(s_options));

static void on_option(struct WabtOptionParser* parser,
                      struct WabtOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      wabt_init_file_writer_existing(&s_log_stream_writer, stdout);
      wabt_init_stream(&s_log_stream, &s_log_stream_writer.base, nullptr);
      s_read_binary_options.log_stream = &s_log_stream;
      break;

    case FLAG_HELP:
      wabt_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;

    case FLAG_VALUE_STACK_SIZE:
      /* TODO(binji): validate */
      s_thread_options.value_stack_size = atoi(argument);
      break;

    case FLAG_CALL_STACK_SIZE:
      /* TODO(binji): validate */
      s_thread_options.call_stack_size = atoi(argument);
      break;

    case FLAG_TRACE:
      s_trace = true;
      break;

    case FLAG_SPEC:
      s_spec = true;
      break;

    case FLAG_RUN_ALL_EXPORTS:
      s_run_all_exports = true;
      break;
  }
}

static void on_argument(struct WabtOptionParser* parser, const char* argument) {
  s_infile = argument;
}

static void on_option_error(struct WabtOptionParser* parser,
                            const char* message) {
  WABT_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WabtOptionParser parser;
  WABT_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WABT_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wabt_parse_options(&parser, argc, argv);

  if (s_spec && s_run_all_exports)
    WABT_FATAL("--spec and --run-all-exports are incompatible.\n");

  if (!s_infile) {
    wabt_print_help(&parser, PROGRAM_NAME);
    WABT_FATAL("No filename given.\n");
  }
}

static WabtStringSlice get_dirname(const char* s) {
  /* strip everything after and including the last slash, e.g.:
   *
   * s = "foo/bar/baz", => "foo/bar"
   * s = "/usr/local/include/stdio.h", => "/usr/local/include"
   * s = "foo.bar", => ""
   */
  const char* last_slash = strrchr(s, '/');
  if (last_slash == nullptr)
    last_slash = s;

  WabtStringSlice result;
  result.start = s;
  result.length = last_slash - s;
  return result;
}

/* Not sure, but 100 chars is probably safe */
#define MAX_TYPED_VALUE_CHARS 100

static void sprint_typed_value(char* buffer,
                               size_t size,
                               const WabtInterpreterTypedValue* tv) {
  switch (tv->type) {
    case WABT_TYPE_I32:
      wabt_snprintf(buffer, size, "i32:%u", tv->value.i32);
      break;

    case WABT_TYPE_I64:
      wabt_snprintf(buffer, size, "i64:%" PRIu64, tv->value.i64);
      break;

    case WABT_TYPE_F32: {
      float value;
      memcpy(&value, &tv->value.f32_bits, sizeof(float));
      wabt_snprintf(buffer, size, "f32:%g", value);
      break;
    }

    case WABT_TYPE_F64: {
      double value;
      memcpy(&value, &tv->value.f64_bits, sizeof(double));
      wabt_snprintf(buffer, size, "f64:%g", value);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void print_typed_value(const WabtInterpreterTypedValue* tv) {
  char buffer[MAX_TYPED_VALUE_CHARS];
  sprint_typed_value(buffer, sizeof(buffer), tv);
  printf("%s", buffer);
}

static void print_typed_values(const WabtInterpreterTypedValue* values,
                               size_t num_values) {
  size_t i;
  for (i = 0; i < num_values; ++i) {
    print_typed_value(&values[i]);
    if (i != num_values - 1)
      printf(", ");
  }
}

static void print_typed_value_vector(
    const WabtInterpreterTypedValueVector* values) {
  print_typed_values(&values->data[0], values->size);
}

static void print_interpreter_result(const char* desc,
                                     WabtInterpreterResult iresult) {
  printf("%s: %s\n", desc, s_trap_strings[iresult]);
}

static void print_call(WabtStringSlice module_name,
                       WabtStringSlice func_name,
                       const WabtInterpreterTypedValueVector* args,
                       const WabtInterpreterTypedValueVector* results,
                       WabtInterpreterResult iresult) {
  if (module_name.length)
    printf(PRIstringslice ".", WABT_PRINTF_STRING_SLICE_ARG(module_name));
  printf(PRIstringslice "(", WABT_PRINTF_STRING_SLICE_ARG(func_name));
  print_typed_value_vector(args);
  printf(") =>");
  if (iresult == WABT_INTERPRETER_OK) {
    if (results->size > 0) {
      printf(" ");
      print_typed_value_vector(results);
    }
    printf("\n");
  } else {
    print_interpreter_result(" error", iresult);
  }
}

static WabtInterpreterResult run_defined_function(WabtInterpreterThread* thread,
                                                  uint32_t offset) {
  thread->pc = offset;
  WabtInterpreterResult iresult = WABT_INTERPRETER_OK;
  uint32_t quantum = s_trace ? 1 : INSTRUCTION_QUANTUM;
  uint32_t* call_stack_return_top = thread->call_stack_top;
  while (iresult == WABT_INTERPRETER_OK) {
    if (s_trace)
      wabt_trace_pc(thread, s_stdout_stream);
    iresult = wabt_run_interpreter(thread, quantum, call_stack_return_top);
  }
  if (iresult != WABT_INTERPRETER_RETURNED)
    return iresult;
  /* use OK instead of RETURNED for consistency */
  return WABT_INTERPRETER_OK;
}

static WabtInterpreterResult push_args(
    WabtInterpreterThread* thread,
    const WabtInterpreterFuncSignature* sig,
    const WabtInterpreterTypedValueVector* args) {
  if (sig->param_types.size != args->size)
    return WABT_INTERPRETER_ARGUMENT_TYPE_MISMATCH;

  size_t i;
  for (i = 0; i < sig->param_types.size; ++i) {
    if (sig->param_types.data[i] != args->data[i].type)
      return WABT_INTERPRETER_ARGUMENT_TYPE_MISMATCH;

    WabtInterpreterResult iresult =
        wabt_push_thread_value(thread, args->data[i].value);
    if (iresult != WABT_INTERPRETER_OK) {
      thread->value_stack_top = thread->value_stack.data;
      return iresult;
    }
  }
  return WABT_INTERPRETER_OK;
}

static void copy_results(WabtInterpreterThread* thread,
                         const WabtInterpreterFuncSignature* sig,
                         WabtInterpreterTypedValueVector* out_results) {
  size_t expected_results = sig->result_types.size;
  size_t value_stack_depth = thread->value_stack_top - thread->value_stack.data;
  WABT_USE(value_stack_depth);
  assert(expected_results == value_stack_depth);

  /* Don't clear out the vector, in case it is being reused. Just reset the
   * size to zero. */
  out_results->size = 0;
  wabt_resize_interpreter_typed_value_vector(out_results, expected_results);
  size_t i;
  for (i = 0; i < expected_results; ++i) {
    out_results->data[i].type = sig->result_types.data[i];
    out_results->data[i].value = thread->value_stack.data[i];
  }
}

static WabtInterpreterResult run_function(
    WabtInterpreterThread* thread,
    uint32_t func_index,
    const WabtInterpreterTypedValueVector* args,
    WabtInterpreterTypedValueVector* out_results) {
  assert(func_index < thread->env->funcs.size);
  WabtInterpreterFunc* func = &thread->env->funcs.data[func_index];
  uint32_t sig_index = func->sig_index;
  assert(sig_index < thread->env->sigs.size);
  WabtInterpreterFuncSignature* sig = &thread->env->sigs.data[sig_index];

  WabtInterpreterResult iresult = push_args(thread, sig, args);
  if (iresult == WABT_INTERPRETER_OK) {
    iresult = func->is_host
                  ? wabt_call_host(thread, func)
                  : run_defined_function(thread, func->defined.offset);
    if (iresult == WABT_INTERPRETER_OK)
      copy_results(thread, sig, out_results);
  }

  /* Always reset the value and call stacks */
  thread->value_stack_top = thread->value_stack.data;
  thread->call_stack_top = thread->call_stack.data;
  return iresult;
}

static WabtInterpreterResult run_start_function(WabtInterpreterThread* thread,
                                                WabtInterpreterModule* module) {
  if (module->defined.start_func_index == WABT_INVALID_INDEX)
    return WABT_INTERPRETER_OK;

  if (s_trace)
    printf(">>> running start function:\n");
  WabtInterpreterTypedValueVector args;
  WabtInterpreterTypedValueVector results;
  WABT_ZERO_MEMORY(args);
  WABT_ZERO_MEMORY(results);

  WabtInterpreterResult iresult =
      run_function(thread, module->defined.start_func_index, &args, &results);
  assert(results.size == 0);
  return iresult;
}

static WabtInterpreterResult run_export(
    WabtInterpreterThread* thread,
    const WabtInterpreterExport* export_,
    const WabtInterpreterTypedValueVector* args,
    WabtInterpreterTypedValueVector* out_results) {
  if (s_trace) {
    printf(">>> running export \"" PRIstringslice "\":\n",
           WABT_PRINTF_STRING_SLICE_ARG(export_->name));
  }

  assert(export_->kind == WABT_EXTERNAL_KIND_FUNC);
  return run_function(thread, export_->index, args, out_results);
}

static WabtInterpreterResult run_export_by_name(
    WabtInterpreterThread* thread,
    WabtInterpreterModule* module,
    const WabtStringSlice* name,
    const WabtInterpreterTypedValueVector* args,
    WabtInterpreterTypedValueVector* out_results,
    RunVerbosity verbose) {
  WabtInterpreterExport* export_ =
      wabt_get_interpreter_export_by_name(module, name);
  if (!export_)
    return WABT_INTERPRETER_UNKNOWN_EXPORT;
  if (export_->kind != WABT_EXTERNAL_KIND_FUNC)
    return WABT_INTERPRETER_EXPORT_KIND_MISMATCH;
  return run_export(thread, export_, args, out_results);
}

static WabtInterpreterResult get_global_export_by_name(
    WabtInterpreterThread* thread,
    WabtInterpreterModule* module,
    const WabtStringSlice* name,
    WabtInterpreterTypedValueVector* out_results) {
  WabtInterpreterExport* export_ =
      wabt_get_interpreter_export_by_name(module, name);
  if (!export_)
    return WABT_INTERPRETER_UNKNOWN_EXPORT;
  if (export_->kind != WABT_EXTERNAL_KIND_GLOBAL)
    return WABT_INTERPRETER_EXPORT_KIND_MISMATCH;

  assert(export_->index < thread->env->globals.size);
  WabtInterpreterGlobal* global = &thread->env->globals.data[export_->index];

  /* Don't clear out the vector, in case it is being reused. Just reset the
   * size to zero. */
  out_results->size = 0;
  wabt_append_interpreter_typed_value_value(out_results, &global->typed_value);
  return WABT_INTERPRETER_OK;
}

static void run_all_exports(WabtInterpreterModule* module,
                            WabtInterpreterThread* thread,
                            RunVerbosity verbose) {
  WabtInterpreterTypedValueVector args;
  WabtInterpreterTypedValueVector results;
  WABT_ZERO_MEMORY(args);
  WABT_ZERO_MEMORY(results);
  uint32_t i;
  for (i = 0; i < module->exports.size; ++i) {
    WabtInterpreterExport* export_ = &module->exports.data[i];
    WabtInterpreterResult iresult =
        run_export(thread, export_, &args, &results);
    if (verbose) {
      print_call(wabt_empty_string_slice(), export_->name, &args, &results,
                 iresult);
    }
  }
  wabt_destroy_interpreter_typed_value_vector(&args);
  wabt_destroy_interpreter_typed_value_vector(&results);
}

static WabtResult read_module(const char* module_filename,
                              WabtInterpreterEnvironment* env,
                              WabtBinaryErrorHandler* error_handler,
                              WabtInterpreterModule** out_module) {
  WabtResult result;
  void* data;
  size_t size;

  *out_module = nullptr;

  result = wabt_read_file(module_filename, &data, &size);
  if (WABT_SUCCEEDED(result)) {
    result = wabt_read_binary_interpreter(
        env, data, size, &s_read_binary_options, error_handler, out_module);

    if (WABT_SUCCEEDED(result)) {
      if (s_verbose)
        wabt_disassemble_module(env, s_stdout_stream, *out_module);
    }
    wabt_free(data);
  }
  return result;
}

static WabtResult default_host_callback(const WabtInterpreterFunc* func,
                                        const WabtInterpreterFuncSignature* sig,
                                        uint32_t num_args,
                                        WabtInterpreterTypedValue* args,
                                        uint32_t num_results,
                                        WabtInterpreterTypedValue* out_results,
                                        void* user_data) {
  memset(out_results, 0, sizeof(WabtInterpreterTypedValue) * num_results);
  uint32_t i;
  for (i = 0; i < num_results; ++i)
    out_results[i].type = sig->result_types.data[i];

  WabtInterpreterTypedValueVector vec_args;
  vec_args.size = num_args;
  vec_args.data = args;

  WabtInterpreterTypedValueVector vec_results;
  vec_results.size = num_results;
  vec_results.data = out_results;

  printf("called host ");
  print_call(func->host.module_name, func->host.field_name, &vec_args,
             &vec_results, WABT_INTERPRETER_OK);
  return WABT_OK;
}

#define PRIimport "\"" PRIstringslice "." PRIstringslice "\""
#define PRINTF_IMPORT_ARG(x)                    \
  WABT_PRINTF_STRING_SLICE_ARG((x).module_name) \
  , WABT_PRINTF_STRING_SLICE_ARG((x).field_name)

static void WABT_PRINTF_FORMAT(2, 3)
    print_error(WabtPrintErrorCallback callback, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  callback.print_error(buffer, callback.user_data);
}

static WabtResult spectest_import_func(WabtInterpreterImport* import,
                                       WabtInterpreterFunc* func,
                                       WabtInterpreterFuncSignature* sig,
                                       WabtPrintErrorCallback callback,
                                       void* user_data) {
  if (wabt_string_slice_eq_cstr(&import->field_name, "print")) {
    func->host.callback = default_host_callback;
    return WABT_OK;
  } else {
    print_error(callback, "unknown host function import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WABT_ERROR;
  }
}

static WabtResult spectest_import_table(WabtInterpreterImport* import,
                                        WabtInterpreterTable* table,
                                        WabtPrintErrorCallback callback,
                                        void* user_data) {
  if (wabt_string_slice_eq_cstr(&import->field_name, "table")) {
    table->limits.has_max = true;
    table->limits.initial = 10;
    table->limits.max = 20;
    return WABT_OK;
  } else {
    print_error(callback, "unknown host table import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WABT_ERROR;
  }
}

static WabtResult spectest_import_memory(WabtInterpreterImport* import,
                                         WabtInterpreterMemory* memory,
                                         WabtPrintErrorCallback callback,
                                         void* user_data) {
  if (wabt_string_slice_eq_cstr(&import->field_name, "memory")) {
    memory->page_limits.has_max = true;
    memory->page_limits.initial = 1;
    memory->page_limits.max = 2;
    memory->byte_size = memory->page_limits.initial * WABT_MAX_PAGES;
    memory->data = wabt_alloc_zero(memory->byte_size);
    return WABT_OK;
  } else {
    print_error(callback, "unknown host memory import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WABT_ERROR;
  }
}

static WabtResult spectest_import_global(WabtInterpreterImport* import,
                                         WabtInterpreterGlobal* global,
                                         WabtPrintErrorCallback callback,
                                         void* user_data) {
  if (wabt_string_slice_eq_cstr(&import->field_name, "global")) {
    switch (global->typed_value.type) {
      case WABT_TYPE_I32:
        global->typed_value.value.i32 = 666;
        break;

      case WABT_TYPE_F32: {
        float value = 666.6f;
        memcpy(&global->typed_value.value.f32_bits, &value, sizeof(value));
        break;
      }

      case WABT_TYPE_I64:
        global->typed_value.value.i64 = 666;
        break;

      case WABT_TYPE_F64: {
        double value = 666.6;
        memcpy(&global->typed_value.value.f64_bits, &value, sizeof(value));
        break;
      }

      default:
        print_error(callback, "bad type for host global import " PRIimport,
                    PRINTF_IMPORT_ARG(*import));
        return WABT_ERROR;
    }

    return WABT_OK;
  } else {
    print_error(callback, "unknown host global import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WABT_ERROR;
  }
}

static void init_environment(WabtInterpreterEnvironment* env) {
  wabt_init_interpreter_environment(env);
  WabtInterpreterModule* host_module =
      wabt_append_host_module(env, wabt_string_slice_from_cstr("spectest"));
  host_module->host.import_delegate.import_func = spectest_import_func;
  host_module->host.import_delegate.import_table = spectest_import_table;
  host_module->host.import_delegate.import_memory = spectest_import_memory;
  host_module->host.import_delegate.import_global = spectest_import_global;
}

static WabtResult read_and_run_module(const char* module_filename) {
  WabtResult result;
  WabtInterpreterEnvironment env;
  WabtInterpreterModule* module = nullptr;
  WabtInterpreterThread thread;

  init_environment(&env);
  wabt_init_interpreter_thread(&env, &thread, &s_thread_options);
  result = read_module(module_filename, &env, &s_error_handler, &module);
  if (WABT_SUCCEEDED(result)) {
    WabtInterpreterResult iresult = run_start_function(&thread, module);
    if (iresult == WABT_INTERPRETER_OK) {
      if (s_run_all_exports)
        run_all_exports(module, &thread, RUN_VERBOSE);
    } else {
      print_interpreter_result("error running start function", iresult);
    }
  }
  wabt_destroy_interpreter_thread(&thread);
  wabt_destroy_interpreter_environment(&env);
  return result;
}

WABT_DEFINE_VECTOR(interpreter_thread, WabtInterpreterThread);

/* An extremely simple JSON parser that only knows how to parse the expected
 * format from wast2wabt. */
typedef struct Context {
  WabtInterpreterEnvironment env;
  WabtInterpreterThread thread;
  WabtInterpreterModule* last_module;

  /* Parsing info */
  char* json_data;
  size_t json_data_size;
  WabtStringSlice source_filename;
  size_t json_offset;
  WabtLocation loc;
  WabtLocation prev_loc;
  bool has_prev_loc;
  uint32_t command_line_number;

  /* Test info */
  int passed;
  int total;
} Context;

typedef enum ActionType {
  ACTION_TYPE_INVOKE,
  ACTION_TYPE_GET,
} ActionType;

typedef struct Action {
  ActionType type;
  WabtStringSlice module_name;
  WabtStringSlice field_name;
  WabtInterpreterTypedValueVector args;
} Action;

#define CHECK_RESULT(x)  \
  do {                   \
    if (WABT_FAILED(x))  \
      return WABT_ERROR; \
  } while (0)

#define EXPECT(x) CHECK_RESULT(expect(ctx, x))
#define EXPECT_KEY(x) CHECK_RESULT(expect_key(ctx, x))
#define PARSE_KEY_STRING_VALUE(key, value) \
  CHECK_RESULT(parse_key_string_value(ctx, key, value))

static void WABT_PRINTF_FORMAT(2, 3)
    print_parse_error(Context* ctx, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  fprintf(stderr, "%s:%d:%d: %s\n", ctx->loc.filename, ctx->loc.line,
          ctx->loc.first_column, buffer);
}

static void WABT_PRINTF_FORMAT(2, 3)
    print_command_error(Context* ctx, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  printf(PRIstringslice ":%u: %s\n",
         WABT_PRINTF_STRING_SLICE_ARG(ctx->source_filename),
         ctx->command_line_number, buffer);
}

static void putback_char(Context* ctx) {
  assert(ctx->has_prev_loc);
  ctx->json_offset--;
  ctx->loc = ctx->prev_loc;
  ctx->has_prev_loc = false;
}

static int read_char(Context* ctx) {
  if (ctx->json_offset >= ctx->json_data_size)
    return -1;
  ctx->prev_loc = ctx->loc;
  char c = ctx->json_data[ctx->json_offset++];
  if (c == '\n') {
    ctx->loc.line++;
    ctx->loc.first_column = 1;
  } else {
    ctx->loc.first_column++;
  }
  ctx->has_prev_loc = true;
  return c;
}

static void skip_whitespace(Context* ctx) {
  while (1) {
    switch (read_char(ctx)) {
      case -1:
        return;

      case ' ':
      case '\t':
      case '\n':
      case '\r':
        break;

      default:
        putback_char(ctx);
        return;
    }
  }
}

static bool match(Context* ctx, const char* s) {
  skip_whitespace(ctx);
  WabtLocation start_loc = ctx->loc;
  size_t start_offset = ctx->json_offset;
  while (*s && *s == read_char(ctx))
    s++;

  if (*s == 0) {
    return true;
  } else {
    ctx->json_offset = start_offset;
    ctx->loc = start_loc;
    return false;
  }
}

static WabtResult expect(Context* ctx, const char* s) {
  if (match(ctx, s)) {
    return WABT_OK;
  } else {
    print_parse_error(ctx, "expected %s", s);
    return WABT_ERROR;
  }
}

static WabtResult expect_key(Context* ctx, const char* key) {
  size_t keylen = strlen(key);
  size_t quoted_len = keylen + 2 + 1;
  char* quoted = (char*)alloca(quoted_len);
  wabt_snprintf(quoted, quoted_len, "\"%s\"", key);
  EXPECT(quoted);
  EXPECT(":");
  return WABT_OK;
}

static WabtResult parse_uint32(Context* ctx, uint32_t* out_int) {
  uint32_t result = 0;
  skip_whitespace(ctx);
  while (1) {
    int c = read_char(ctx);
    if (c >= '0' && c <= '9') {
      uint32_t last_result = result;
      result = result * 10 + (uint32_t)(c - '0');
      if (result < last_result) {
        print_parse_error(ctx, "uint32 overflow");
        return WABT_ERROR;
      }
    } else {
      putback_char(ctx);
      break;
    }
  }
  *out_int = result;
  return WABT_OK;
}

static WabtResult parse_string(Context* ctx, WabtStringSlice* out_string) {
  WABT_ZERO_MEMORY(*out_string);

  skip_whitespace(ctx);
  if (read_char(ctx) != '"') {
    print_parse_error(ctx, "expected string");
    return WABT_ERROR;
  }
  /* Modify json_data in-place so we can use the WabtStringSlice directly
   * without having to allocate additional memory; this is only necessary when
   * the string contains an escape, but we do it always because the code is
   * simpler. */
  char* start = &ctx->json_data[ctx->json_offset];
  char* p = start;
  out_string->start = start;
  while (1) {
    int c = read_char(ctx);
    if (c == '"') {
      break;
    } else if (c == '\\') {
      /* The only escape supported is \uxxxx. */
      c = read_char(ctx);
      if (c != 'u') {
        print_parse_error(ctx, "expected escape: \\uxxxx");
        return WABT_ERROR;
      }
      int i;
      uint16_t code = 0;
      for (i = 0; i < 4; ++i) {
        c = read_char(ctx);
        int cval;
        if (c >= '0' && c <= '9') {
          cval = c - '0';
        } else if (c >= 'a' && c <= 'f') {
          cval = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
          cval = c - 'A' + 10;
        } else {
          print_parse_error(ctx, "expected hex char");
          return WABT_ERROR;
        }
        code = (code << 4) + cval;
      }

      if (code < 256) {
        *p++ = code;
      } else {
        print_parse_error(ctx, "only escape codes < 256 allowed, got %u\n",
                          code);
      }
    } else {
      *p++ = c;
    }
  }
  out_string->length = p - start;
  return WABT_OK;
}

static WabtResult parse_key_string_value(Context* ctx,
                                         const char* key,
                                         WabtStringSlice* out_string) {
  WABT_ZERO_MEMORY(*out_string);
  EXPECT_KEY(key);
  return parse_string(ctx, out_string);
}

static WabtResult parse_opt_name_string_value(Context* ctx,
                                              WabtStringSlice* out_string) {
  WABT_ZERO_MEMORY(*out_string);
  if (match(ctx, "\"name\"")) {
    EXPECT(":");
    CHECK_RESULT(parse_string(ctx, out_string));
    EXPECT(",");
  }
  return WABT_OK;
}

static WabtResult parse_line(Context* ctx) {
  EXPECT_KEY("line");
  CHECK_RESULT(parse_uint32(ctx, &ctx->command_line_number));
  return WABT_OK;
}

static WabtResult parse_type_object(Context* ctx, WabtType* out_type) {
  WabtStringSlice type_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT("}");

  if (wabt_string_slice_eq_cstr(&type_str, "i32")) {
    *out_type = WABT_TYPE_I32;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "f32")) {
    *out_type = WABT_TYPE_F32;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "i64")) {
    *out_type = WABT_TYPE_I64;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "f64")) {
    *out_type = WABT_TYPE_F64;
    return WABT_OK;
  } else {
    print_parse_error(ctx, "unknown type: \"" PRIstringslice "\"",
                      WABT_PRINTF_STRING_SLICE_ARG(type_str));
    return WABT_ERROR;
  }
}

static WabtResult parse_type_vector(Context* ctx, WabtTypeVector* out_types) {
  WABT_ZERO_MEMORY(*out_types);
  EXPECT("[");
  bool first = true;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    WabtType type;
    CHECK_RESULT(parse_type_object(ctx, &type));
    first = false;
    wabt_append_type_value(out_types, &type);
  }
  return WABT_OK;
}

static WabtResult parse_const(Context* ctx,
                              WabtInterpreterTypedValue* out_value) {
  WabtStringSlice type_str;
  WabtStringSlice value_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT(",");
  PARSE_KEY_STRING_VALUE("value", &value_str);
  EXPECT("}");

  const char* value_start = value_str.start;
  const char* value_end = value_str.start + value_str.length;

  if (wabt_string_slice_eq_cstr(&type_str, "i32")) {
    uint32_t value;
    CHECK_RESULT(wabt_parse_int32(value_start, value_end, &value,
                                  WABT_PARSE_UNSIGNED_ONLY));
    out_value->type = WABT_TYPE_I32;
    out_value->value.i32 = value;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "f32")) {
    uint32_t value_bits;
    CHECK_RESULT(wabt_parse_int32(value_start, value_end, &value_bits,
                                  WABT_PARSE_UNSIGNED_ONLY));
    out_value->type = WABT_TYPE_F32;
    out_value->value.f32_bits = value_bits;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "i64")) {
    uint64_t value;
    CHECK_RESULT(wabt_parse_int64(value_start, value_end, &value,
                                  WABT_PARSE_UNSIGNED_ONLY));
    out_value->type = WABT_TYPE_I64;
    out_value->value.i64 = value;
    return WABT_OK;
  } else if (wabt_string_slice_eq_cstr(&type_str, "f64")) {
    uint64_t value_bits;
    CHECK_RESULT(wabt_parse_int64(value_start, value_end, &value_bits,
                                  WABT_PARSE_UNSIGNED_ONLY));
    out_value->type = WABT_TYPE_F64;
    out_value->value.f64_bits = value_bits;
    return WABT_OK;
  } else {
    print_parse_error(ctx, "unknown type: \"" PRIstringslice "\"",
                      WABT_PRINTF_STRING_SLICE_ARG(type_str));
    return WABT_ERROR;
  }
}

static WabtResult parse_const_vector(
    Context* ctx,
    WabtInterpreterTypedValueVector* out_values) {
  WABT_ZERO_MEMORY(*out_values);
  EXPECT("[");
  bool first = true;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    WabtInterpreterTypedValue value;
    CHECK_RESULT(parse_const(ctx, &value));
    wabt_append_interpreter_typed_value_value(out_values, &value);
    first = false;
  }
  return WABT_OK;
}

static WabtResult parse_action(Context* ctx, Action* out_action) {
  WABT_ZERO_MEMORY(*out_action);
  EXPECT_KEY("action");
  EXPECT("{");
  EXPECT_KEY("type");
  if (match(ctx, "\"invoke\"")) {
    out_action->type = ACTION_TYPE_INVOKE;
  } else {
    EXPECT("\"get\"");
    out_action->type = ACTION_TYPE_GET;
  }
  EXPECT(",");
  if (match(ctx, "\"module\"")) {
    EXPECT(":");
    CHECK_RESULT(parse_string(ctx, &out_action->module_name));
    EXPECT(",");
  }
  PARSE_KEY_STRING_VALUE("field", &out_action->field_name);
  if (out_action->type == ACTION_TYPE_INVOKE) {
    EXPECT(",");
    EXPECT_KEY("args");
    CHECK_RESULT(parse_const_vector(ctx, &out_action->args));
  }
  EXPECT("}");
  return WABT_OK;
}

static char* create_module_path(Context* ctx, WabtStringSlice filename) {
  const char* spec_json_filename = ctx->loc.filename;
  WabtStringSlice dirname = get_dirname(spec_json_filename);
  size_t path_len = dirname.length + 1 + filename.length + 1;
  char* path = (char*)wabt_alloc(path_len);

  if (dirname.length == 0) {
    wabt_snprintf(path, path_len, PRIstringslice,
                  WABT_PRINTF_STRING_SLICE_ARG(filename));
  } else {
    wabt_snprintf(path, path_len, PRIstringslice "/" PRIstringslice,
                  WABT_PRINTF_STRING_SLICE_ARG(dirname),
                  WABT_PRINTF_STRING_SLICE_ARG(filename));
  }

  return path;
}

static WabtResult on_module_command(Context* ctx,
                                    WabtStringSlice filename,
                                    WabtStringSlice name) {
  char* path = create_module_path(ctx, filename);
  WabtInterpreterEnvironmentMark mark =
      wabt_mark_interpreter_environment(&ctx->env);
  WabtResult result =
      read_module(path, &ctx->env, &s_error_handler, &ctx->last_module);

  if (WABT_FAILED(result)) {
    wabt_reset_interpreter_environment_to_mark(&ctx->env, mark);
    print_command_error(ctx, "error reading module: \"%s\"", path);
    wabt_free(path);
    return WABT_ERROR;
  }

  wabt_free(path);

  WabtInterpreterResult iresult =
      run_start_function(&ctx->thread, ctx->last_module);
  if (iresult != WABT_INTERPRETER_OK) {
    wabt_reset_interpreter_environment_to_mark(&ctx->env, mark);
    print_interpreter_result("error running start function", iresult);
    return WABT_ERROR;
  }

  if (!wabt_string_slice_is_empty(&name)) {
    ctx->last_module->name = wabt_dup_string_slice(name);

    /* The binding also needs its own copy of the name. */
    WabtStringSlice binding_name = wabt_dup_string_slice(name);
    WabtBinding* binding =
        wabt_insert_binding(&ctx->env.module_bindings, &binding_name);
    binding->index = ctx->env.modules.size - 1;
  }
  return WABT_OK;
}

static WabtResult run_action(Context* ctx,
                             Action* action,
                             WabtInterpreterResult* out_iresult,
                             WabtInterpreterTypedValueVector* out_results,
                             RunVerbosity verbose) {
  WABT_ZERO_MEMORY(*out_results);

  int module_index;
  if (!wabt_string_slice_is_empty(&action->module_name)) {
    module_index = wabt_find_binding_index_by_name(&ctx->env.module_bindings,
                                                   &action->module_name);
  } else {
    module_index = (int)ctx->env.modules.size - 1;
  }

  assert(module_index < (int)ctx->env.modules.size);
  WabtInterpreterModule* module = &ctx->env.modules.data[module_index];

  switch (action->type) {
    case ACTION_TYPE_INVOKE:
      *out_iresult =
          run_export_by_name(&ctx->thread, module, &action->field_name,
                             &action->args, out_results, verbose);
      if (verbose) {
        print_call(wabt_empty_string_slice(), action->field_name, &action->args,
                   out_results, *out_iresult);
      }
      return WABT_OK;

    case ACTION_TYPE_GET: {
      *out_iresult = get_global_export_by_name(
          &ctx->thread, module, &action->field_name, out_results);
      return WABT_OK;
    }

    default:
      print_command_error(ctx, "invalid action type %d", action->type);
      return WABT_ERROR;
  }
}

static WabtResult on_action_command(Context* ctx, Action* action) {
  WabtInterpreterTypedValueVector results;
  WabtInterpreterResult iresult;

  ctx->total++;
  WabtResult result = run_action(ctx, action, &iresult, &results, RUN_VERBOSE);
  if (WABT_SUCCEEDED(result)) {
    if (iresult == WABT_INTERPRETER_OK) {
      ctx->passed++;
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WABT_ERROR;
    }
  }

  wabt_destroy_interpreter_typed_value_vector(&results);
  return result;
}

static WabtBinaryErrorHandler* new_custom_error_handler(Context* ctx,
                                                        const char* desc) {
  size_t header_size = ctx->source_filename.length + strlen(desc) + 100;
  char* header = (char*)wabt_alloc(header_size);
  wabt_snprintf(header, header_size, PRIstringslice ":%d: %s passed",
                WABT_PRINTF_STRING_SLICE_ARG(ctx->source_filename),
                ctx->command_line_number, desc);

  WabtDefaultErrorHandlerInfo* info =
      (WabtDefaultErrorHandlerInfo*)wabt_alloc_zero(
          sizeof(WabtDefaultErrorHandlerInfo));
  info->header = header;
  info->out_file = stdout;
  info->print_header = WABT_PRINT_ERROR_HEADER_ONCE;

  WabtBinaryErrorHandler* error_handler =
      (WabtBinaryErrorHandler*)wabt_alloc_zero(sizeof(WabtBinaryErrorHandler));
  error_handler->on_error = wabt_default_binary_error_callback;
  error_handler->user_data = info;
  return error_handler;
}

static void destroy_custom_error_handler(
    WabtBinaryErrorHandler* error_handler) {
  WabtDefaultErrorHandlerInfo* info =
      (WabtDefaultErrorHandlerInfo*)error_handler->user_data;
  wabt_free((void*)info->header);
  wabt_free(info);
  wabt_free(error_handler);
}

static WabtResult on_assert_malformed_command(Context* ctx,
                                              WabtStringSlice filename,
                                              WabtStringSlice text) {
  WabtBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_malformed");
  WabtInterpreterEnvironment env;
  WABT_ZERO_MEMORY(env);
  init_environment(&env);

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WabtInterpreterModule* module;
  WabtResult result = read_module(path, &env, error_handler, &module);
  if (WABT_FAILED(result)) {
    ctx->passed++;
    result = WABT_OK;
  } else {
    print_command_error(ctx, "expected module to be malformed: \"%s\"", path);
    result = WABT_ERROR;
  }

  wabt_free(path);
  wabt_destroy_interpreter_environment(&env);
  destroy_custom_error_handler(error_handler);
  return result;
}

static WabtResult on_register_command(Context* ctx,
                                      WabtStringSlice name,
                                      WabtStringSlice as) {
  int module_index;
  if (!wabt_string_slice_is_empty(&name)) {
    /* The module names can be different than their registered names. We don't
     * keep a hash for the module names (just the registered names), so we'll
     * just iterate over all the modules to find it. */
    size_t i;
    module_index = -1;
    for (i = 0; i < ctx->env.modules.size; ++i) {
      const WabtStringSlice* module_name = &ctx->env.modules.data[i].name;
      if (!wabt_string_slice_is_empty(module_name) &&
          wabt_string_slices_are_equal(&name, module_name)) {
        module_index = (int)i;
        break;
      }
    }
  } else {
    module_index = (int)ctx->env.modules.size - 1;
  }

  if (module_index < 0 || module_index >= (int)ctx->env.modules.size) {
    print_command_error(ctx, "unknown module in register");
    return WABT_ERROR;
  }

  WabtStringSlice dup_as = wabt_dup_string_slice(as);
  WabtBinding* binding =
      wabt_insert_binding(&ctx->env.registered_module_bindings, &dup_as);
  binding->index = module_index;
  return WABT_OK;
}

static WabtResult on_assert_unlinkable_command(Context* ctx,
                                               WabtStringSlice filename,
                                               WabtStringSlice text) {
  WabtBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_unlinkable");

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WabtInterpreterModule* module;
  WabtInterpreterEnvironmentMark mark =
      wabt_mark_interpreter_environment(&ctx->env);
  WabtResult result = read_module(path, &ctx->env, error_handler, &module);
  wabt_reset_interpreter_environment_to_mark(&ctx->env, mark);

  if (WABT_FAILED(result)) {
    ctx->passed++;
    result = WABT_OK;
  } else {
    print_command_error(ctx, "expected module to be unlinkable: \"%s\"", path);
    result = WABT_ERROR;
  }

  wabt_free(path);
  destroy_custom_error_handler(error_handler);
  return result;
}

static WabtResult on_assert_invalid_command(Context* ctx,
                                            WabtStringSlice filename,
                                            WabtStringSlice text) {
  WabtBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_invalid");
  WabtInterpreterEnvironment env;
  WABT_ZERO_MEMORY(env);
  init_environment(&env);

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WabtInterpreterModule* module;
  WabtResult result = read_module(path, &env, error_handler, &module);
  if (WABT_FAILED(result)) {
    ctx->passed++;
    result = WABT_OK;
  } else {
    print_command_error(ctx, "expected module to be invalid: \"%s\"", path);
    result = WABT_ERROR;
  }

  wabt_free(path);
  wabt_destroy_interpreter_environment(&env);
  destroy_custom_error_handler(error_handler);
  return result;
}

static WabtResult on_assert_uninstantiable_command(Context* ctx,
                                                   WabtStringSlice filename,
                                                   WabtStringSlice text) {
  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WabtInterpreterModule* module;
  WabtInterpreterEnvironmentMark mark =
      wabt_mark_interpreter_environment(&ctx->env);
  WabtResult result = read_module(path, &ctx->env, &s_error_handler, &module);

  if (WABT_SUCCEEDED(result)) {
    WabtInterpreterResult iresult = run_start_function(&ctx->thread, module);
    if (iresult == WABT_INTERPRETER_OK) {
      print_command_error(ctx, "expected error running start function: \"%s\"",
                          path);
      result = WABT_ERROR;
    } else {
      ctx->passed++;
      result = WABT_OK;
    }
  } else {
    print_command_error(ctx, "error reading module: \"%s\"", path);
    result = WABT_ERROR;
  }

  wabt_reset_interpreter_environment_to_mark(&ctx->env, mark);
  wabt_free(path);
  return result;
}

static bool typed_values_are_equal(const WabtInterpreterTypedValue* tv1,
                                       const WabtInterpreterTypedValue* tv2) {
  if (tv1->type != tv2->type)
    return false;

  switch (tv1->type) {
    case WABT_TYPE_I32: return tv1->value.i32 == tv2->value.i32;
    case WABT_TYPE_F32: return tv1->value.f32_bits == tv2->value.f32_bits;
    case WABT_TYPE_I64: return tv1->value.i64 == tv2->value.i64;
    case WABT_TYPE_F64: return tv1->value.f64_bits == tv2->value.f64_bits;
    default: assert(0); return false;
  }
}

static WabtResult on_assert_return_command(
    Context* ctx,
    Action* action,
    WabtInterpreterTypedValueVector* expected) {
  WabtInterpreterTypedValueVector results;
  WabtInterpreterResult iresult;

  ctx->total++;
  WabtResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);

  if (WABT_SUCCEEDED(result)) {
    if (iresult == WABT_INTERPRETER_OK) {
      if (results.size == expected->size) {
        size_t i;
        for (i = 0; i < results.size; ++i) {
          const WabtInterpreterTypedValue* expected_tv = &expected->data[i];
          const WabtInterpreterTypedValue* actual_tv = &results.data[i];
          if (!typed_values_are_equal(expected_tv, actual_tv)) {
            char expected_str[MAX_TYPED_VALUE_CHARS];
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(expected_str, sizeof(expected_str), expected_tv);
            sprint_typed_value(actual_str, sizeof(actual_str), actual_tv);
            print_command_error(ctx, "mismatch in result %" PRIzd
                                     " of assert_return: expected %s, got %s",
                                i, expected_str, actual_str);
            result = WABT_ERROR;
          }
        }
      } else {
        print_command_error(
            ctx, "result length mismatch in assert_return: expected %" PRIzd
                 ", got %" PRIzd,
            expected->size, results.size);
        result = WABT_ERROR;
      }
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WABT_ERROR;
    }
  }

  if (WABT_SUCCEEDED(result))
    ctx->passed++;

  wabt_destroy_interpreter_typed_value_vector(&results);
  return result;
}

static WabtResult on_assert_return_nan_command(Context* ctx, Action* action) {
  WabtInterpreterTypedValueVector results;
  WabtInterpreterResult iresult;

  ctx->total++;
  WabtResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
  if (WABT_SUCCEEDED(result)) {
    if (iresult == WABT_INTERPRETER_OK) {
      if (results.size != 1) {
        print_command_error(ctx, "expected one result, got %" PRIzd,
                            results.size);
        result = WABT_ERROR;
      }

      const WabtInterpreterTypedValue* actual = &results.data[0];
      switch (actual->type) {
        case WABT_TYPE_F32:
          if (!wabt_is_nan_f32(actual->value.f32_bits)) {
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(actual_str, sizeof(actual_str), actual);
            print_command_error(ctx, "expected result to be nan, got %s",
                                actual_str);
            result = WABT_ERROR;
          }
          break;

        case WABT_TYPE_F64:
          if (!wabt_is_nan_f64(actual->value.f64_bits)) {
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(actual_str, sizeof(actual_str), actual);
            print_command_error(ctx, "expected result to be nan, got %s",
                                actual_str);
            result = WABT_ERROR;
          }
          break;

        default:
          print_command_error(ctx,
                              "expected result type to be f32 or f64, got %s",
                              wabt_get_type_name(actual->type));
          result = WABT_ERROR;
          break;
      }
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WABT_ERROR;
    }
  }

  if (WABT_SUCCEEDED(result))
    ctx->passed++;

  wabt_destroy_interpreter_typed_value_vector(&results);
  return WABT_OK;
}

static WabtResult on_assert_trap_command(Context* ctx,
                                         Action* action,
                                         WabtStringSlice text) {
  WabtInterpreterTypedValueVector results;
  WabtInterpreterResult iresult;

  ctx->total++;
  WabtResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
  if (WABT_SUCCEEDED(result)) {
    if (iresult != WABT_INTERPRETER_OK) {
      ctx->passed++;
    } else {
      print_command_error(ctx, "expected trap: \"" PRIstringslice "\"",
                          WABT_PRINTF_STRING_SLICE_ARG(text));
      result = WABT_ERROR;
    }
  }

  wabt_destroy_interpreter_typed_value_vector(&results);
  return result;
}

static WabtResult on_assert_exhaustion_command(Context* ctx,
                                         Action* action) {
    WabtInterpreterTypedValueVector results;
    WabtInterpreterResult iresult;

    ctx->total++;
    WabtResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
    if (WABT_SUCCEEDED(result)) {
      if (iresult == WABT_INTERPRETER_TRAP_CALL_STACK_EXHAUSTED ||
          iresult == WABT_INTERPRETER_TRAP_VALUE_STACK_EXHAUSTED) {
        ctx->passed++;
      } else {
        print_command_error(ctx, "expected call stack exhaustion");
        result = WABT_ERROR;
      }
    }

    wabt_destroy_interpreter_typed_value_vector(&results);
    return result;
}

static void destroy_action(Action* action) {
  wabt_destroy_interpreter_typed_value_vector(&action->args);
}

static WabtResult parse_command(Context* ctx) {
  EXPECT("{");
  EXPECT_KEY("type");
  if (match(ctx, "\"module\"")) {
    WabtStringSlice name;
    WabtStringSlice filename;
    WABT_ZERO_MEMORY(name);
    WABT_ZERO_MEMORY(filename);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_opt_name_string_value(ctx, &name));
    PARSE_KEY_STRING_VALUE("filename", &filename);
    on_module_command(ctx, filename, name);
  } else if (match(ctx, "\"action\"")) {
    Action action;
    WABT_ZERO_MEMORY(action);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    on_action_command(ctx, &action);
    destroy_action(&action);
  } else if (match(ctx, "\"register\"")) {
    WabtStringSlice as;
    WabtStringSlice name;
    WABT_ZERO_MEMORY(as);
    WABT_ZERO_MEMORY(name);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_opt_name_string_value(ctx, &name));
    PARSE_KEY_STRING_VALUE("as", &as);
    on_register_command(ctx, name, as);
  } else if (match(ctx, "\"assert_malformed\"")) {
    WabtStringSlice filename;
    WabtStringSlice text;
    WABT_ZERO_MEMORY(filename);
    WABT_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_malformed_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_invalid\"")) {
    WabtStringSlice filename;
    WabtStringSlice text;
    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_invalid_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_unlinkable\"")) {
    WabtStringSlice filename;
    WabtStringSlice text;
    WABT_ZERO_MEMORY(filename);
    WABT_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_unlinkable_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_uninstantiable\"")) {
    WabtStringSlice filename;
    WabtStringSlice text;
    WABT_ZERO_MEMORY(filename);
    WABT_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_uninstantiable_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_return\"")) {
    Action action;
    WabtInterpreterTypedValueVector expected;
    WABT_ZERO_MEMORY(action);
    WABT_ZERO_MEMORY(expected);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    EXPECT_KEY("expected");
    CHECK_RESULT(parse_const_vector(ctx, &expected));
    on_assert_return_command(ctx, &action, &expected);
    wabt_destroy_interpreter_typed_value_vector(&expected);
    destroy_action(&action);
  } else if (match(ctx, "\"assert_return_nan\"")) {
    Action action;
    WabtTypeVector expected;
    WABT_ZERO_MEMORY(action);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    /* Not needed for wabt-interp, but useful for other parsers. */
    EXPECT_KEY("expected");
    CHECK_RESULT(parse_type_vector(ctx, &expected));
    on_assert_return_nan_command(ctx, &action);
    wabt_destroy_type_vector(&expected);
    destroy_action(&action);
  } else if (match(ctx, "\"assert_trap\"")) {
    Action action;
    WabtStringSlice text;
    WABT_ZERO_MEMORY(action);
    WABT_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_trap_command(ctx, &action, text);
    destroy_action(&action);
  } else if (match(ctx, "\"assert_exhaustion\"")) {
    Action action;
    WabtStringSlice text;
    WABT_ZERO_MEMORY(action);
    WABT_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    on_assert_exhaustion_command(ctx, &action);
    destroy_action(&action);
  } else {
    print_command_error(ctx, "unknown command type");
    return WABT_ERROR;
  }
  EXPECT("}");
  return WABT_OK;
}

static WabtResult parse_commands(Context* ctx) {
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("source_filename", &ctx->source_filename);
  EXPECT(",");
  EXPECT_KEY("commands");
  EXPECT("[");
  bool first = true;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    CHECK_RESULT(parse_command(ctx));
    first = false;
  }
  EXPECT("}");
  return WABT_OK;
}

static void destroy_context(Context* ctx) {
  wabt_destroy_interpreter_thread(&ctx->thread);
  wabt_destroy_interpreter_environment(&ctx->env);
  wabt_free(ctx->json_data);
}

static WabtResult read_and_run_spec_json(const char* spec_json_filename) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.loc.filename = spec_json_filename;
  ctx.loc.line = 1;
  ctx.loc.first_column = 1;
  init_environment(&ctx.env);
  wabt_init_interpreter_thread(&ctx.env, &ctx.thread, &s_thread_options);

  void* data;
  size_t size;
  WabtResult result = wabt_read_file(spec_json_filename, &data, &size);
  if (WABT_FAILED(result))
    return WABT_ERROR;

  ctx.json_data = (char*)data;
  ctx.json_data_size = size;

  result = parse_commands(&ctx);
  printf("%d/%d tests passed.\n", ctx.passed, ctx.total);
  destroy_context(&ctx);
  return result;
}

int main(int argc, char** argv) {
  wabt_init_stdio();
  parse_options(argc, argv);

  s_stdout_stream = wabt_init_stdout_stream();

  if (s_spec) {
    return read_and_run_spec_json(s_infile);
  } else {
    return read_and_run_module(s_infile);
  }
}
