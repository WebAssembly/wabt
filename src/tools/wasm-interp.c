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

#include "allocator.h"
#include "binary-reader.h"
#include "binary-reader-interpreter.h"
#include "interpreter.h"
#include "literal.h"
#include "option-parser.h"
#include "stack-allocator.h"
#include "stream.h"

#define INSTRUCTION_QUANTUM 1000
#define PROGRAM_NAME "wasm-interp"

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static WasmBool s_verbose;
static const char* s_infile;
static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;
static WasmInterpreterThreadOptions s_thread_options =
    WASM_INTERPRETER_THREAD_OPTIONS_DEFAULT;
static WasmBool s_trace;
static WasmBool s_spec;
static WasmBool s_run_all_exports;
static WasmBool s_use_libc_allocator;
static WasmStream* s_stdout_stream;

static WasmBinaryErrorHandler s_error_handler =
    WASM_BINARY_ERROR_HANDLER_DEFAULT;

static WasmFileWriter s_log_stream_writer;
static WasmStream s_log_stream;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

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
  FLAG_USE_LIBC_ALLOCATOR,
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

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_VALUE_STACK_SIZE, 'V', "value-stack-size", "SIZE", YEP,
     "size in elements of the value stack"},
    {FLAG_CALL_STACK_SIZE, 'C', "call-stack-size", "SIZE", YEP,
     "size in frames of the call stack"},
    {FLAG_TRACE, 't', "trace", NULL, NOPE, "trace execution"},
    {FLAG_SPEC, 0, "spec", NULL, NOPE,
     "run spec tests (input file should be .json)"},
    {FLAG_RUN_ALL_EXPORTS, 0, "run-all-exports", NULL, NOPE,
     "run all the exported functions, in order. useful for testing"},
    {FLAG_USE_LIBC_ALLOCATOR, 0, "use-libc-allocator", NULL, NOPE,
     "use malloc, free, etc. instead of stack allocator"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      wasm_init_file_writer_existing(&s_log_stream_writer, stdout);
      wasm_init_stream(&s_log_stream, &s_log_stream_writer.base, NULL);
      s_read_binary_options.log_stream = &s_log_stream;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
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
      s_trace = WASM_TRUE;
      break;

    case FLAG_SPEC:
      s_spec = WASM_TRUE;
      break;

    case FLAG_RUN_ALL_EXPORTS:
      s_run_all_exports = WASM_TRUE;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = WASM_TRUE;
      break;
  }
}

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  s_infile = argument;
}

static void on_option_error(struct WasmOptionParser* parser,
                            const char* message) {
  WASM_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WasmOptionParser parser;
  WASM_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (s_spec && s_run_all_exports)
    WASM_FATAL("--spec and --run-all-exports are incompatible.\n");

  if (!s_infile) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No filename given.\n");
  }
}

static WasmStringSlice get_dirname(const char* s) {
  /* strip everything after and including the last slash, e.g.:
   *
   * s = "foo/bar/baz", => "foo/bar"
   * s = "/usr/local/include/stdio.h", => "/usr/local/include"
   * s = "foo.bar", => ""
   */
  const char* last_slash = strrchr(s, '/');
  if (last_slash == NULL)
    last_slash = s;

  WasmStringSlice result;
  result.start = s;
  result.length = last_slash - s;
  return result;
}

/* Not sure, but 100 chars is probably safe */
#define MAX_TYPED_VALUE_CHARS 100

static void sprint_typed_value(char* buffer,
                               size_t size,
                               const WasmInterpreterTypedValue* tv) {
  switch (tv->type) {
    case WASM_TYPE_I32:
      wasm_snprintf(buffer, size, "i32:%u", tv->value.i32);
      break;

    case WASM_TYPE_I64:
      wasm_snprintf(buffer, size, "i64:%" PRIu64, tv->value.i64);
      break;

    case WASM_TYPE_F32: {
      float value;
      memcpy(&value, &tv->value.f32_bits, sizeof(float));
      wasm_snprintf(buffer, size, "f32:%g", value);
      break;
    }

    case WASM_TYPE_F64: {
      double value;
      memcpy(&value, &tv->value.f64_bits, sizeof(double));
      wasm_snprintf(buffer, size, "f64:%g", value);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void print_typed_value(const WasmInterpreterTypedValue* tv) {
  char buffer[MAX_TYPED_VALUE_CHARS];
  sprint_typed_value(buffer, sizeof(buffer), tv);
  printf("%s", buffer);
}

static void print_typed_values(const WasmInterpreterTypedValue* values,
                               size_t num_values) {
  size_t i;
  for (i = 0; i < num_values; ++i) {
    print_typed_value(&values[i]);
    if (i != num_values - 1)
      printf(", ");
  }
}

static void print_typed_value_vector(
    const WasmInterpreterTypedValueVector* values) {
  print_typed_values(&values->data[0], values->size);
}

static void print_interpreter_result(const char* desc,
                                     WasmInterpreterResult iresult) {
  printf("%s: %s\n", desc, s_trap_strings[iresult]);
}

static void print_call(WasmStringSlice module_name,
                       WasmStringSlice func_name,
                       const WasmInterpreterTypedValueVector* args,
                       const WasmInterpreterTypedValueVector* results,
                       WasmInterpreterResult iresult) {
  if (module_name.length)
    printf(PRIstringslice ".", WASM_PRINTF_STRING_SLICE_ARG(module_name));
  printf(PRIstringslice "(", WASM_PRINTF_STRING_SLICE_ARG(func_name));
  print_typed_value_vector(args);
  printf(") =>");
  if (iresult == WASM_INTERPRETER_OK) {
    if (results->size > 0) {
      printf(" ");
      print_typed_value_vector(results);
    }
    printf("\n");
  } else {
    print_interpreter_result(" error", iresult);
  }
}

static WasmInterpreterResult run_defined_function(WasmInterpreterThread* thread,
                                                  uint32_t offset) {
  thread->pc = offset;
  WasmInterpreterResult iresult = WASM_INTERPRETER_OK;
  uint32_t quantum = s_trace ? 1 : INSTRUCTION_QUANTUM;
  uint32_t* call_stack_return_top = thread->call_stack_top;
  while (iresult == WASM_INTERPRETER_OK) {
    if (s_trace)
      wasm_trace_pc(thread, s_stdout_stream);
    iresult = wasm_run_interpreter(thread, quantum, call_stack_return_top);
  }
  if (iresult != WASM_INTERPRETER_RETURNED)
    return iresult;
  /* use OK instead of RETURNED for consistency */
  return WASM_INTERPRETER_OK;
}

static WasmInterpreterResult push_args(
    WasmInterpreterThread* thread,
    const WasmInterpreterFuncSignature* sig,
    const WasmInterpreterTypedValueVector* args) {
  if (sig->param_types.size != args->size)
    return WASM_INTERPRETER_ARGUMENT_TYPE_MISMATCH;

  size_t i;
  for (i = 0; i < sig->param_types.size; ++i) {
    if (sig->param_types.data[i] != args->data[i].type)
      return WASM_INTERPRETER_ARGUMENT_TYPE_MISMATCH;

    WasmInterpreterResult iresult =
        wasm_push_thread_value(thread, args->data[i].value);
    if (iresult != WASM_INTERPRETER_OK) {
      thread->value_stack_top = thread->value_stack.data;
      return iresult;
    }
  }
  return WASM_INTERPRETER_OK;
}

static void copy_results(WasmAllocator* allocator,
                         WasmInterpreterThread* thread,
                         const WasmInterpreterFuncSignature* sig,
                         WasmInterpreterTypedValueVector* out_results) {
  size_t expected_results = sig->result_types.size;
  size_t value_stack_depth = thread->value_stack_top - thread->value_stack.data;
  WASM_USE(value_stack_depth);
  assert(expected_results == value_stack_depth);

  /* Don't clear out the vector, in case it is being reused. Just reset the
   * size to zero. */
  out_results->size = 0;
  wasm_resize_interpreter_typed_value_vector(allocator, out_results,
                                             expected_results);
  size_t i;
  for (i = 0; i < expected_results; ++i) {
    out_results->data[i].type = sig->result_types.data[i];
    out_results->data[i].value = thread->value_stack.data[i];
  }
}

static WasmInterpreterResult run_function(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    uint32_t func_index,
    const WasmInterpreterTypedValueVector* args,
    WasmInterpreterTypedValueVector* out_results) {
  assert(func_index < thread->env->funcs.size);
  WasmInterpreterFunc* func = &thread->env->funcs.data[func_index];
  uint32_t sig_index = func->sig_index;
  assert(sig_index < thread->env->sigs.size);
  WasmInterpreterFuncSignature* sig = &thread->env->sigs.data[sig_index];

  WasmInterpreterResult iresult = push_args(thread, sig, args);
  if (iresult == WASM_INTERPRETER_OK) {
    iresult = func->is_host
                  ? wasm_call_host(thread, func)
                  : run_defined_function(thread, func->defined.offset);
    if (iresult == WASM_INTERPRETER_OK)
      copy_results(allocator, thread, sig, out_results);
  }

  /* Always reset the value and call stacks */
  thread->value_stack_top = thread->value_stack.data;
  thread->call_stack_top = thread->call_stack.data;
  return iresult;
}

static WasmInterpreterResult run_start_function(WasmAllocator* allocator,
                                                WasmInterpreterThread* thread,
                                                WasmInterpreterModule* module) {
  if (module->defined.start_func_index == WASM_INVALID_INDEX)
    return WASM_INTERPRETER_OK;

  if (s_trace)
    printf(">>> running start function:\n");
  WasmInterpreterTypedValueVector args;
  WasmInterpreterTypedValueVector results;
  WASM_ZERO_MEMORY(args);
  WASM_ZERO_MEMORY(results);

  WasmInterpreterResult iresult = run_function(
      allocator, thread, module->defined.start_func_index, &args, &results);
  assert(results.size == 0);
  return iresult;
}

static WasmInterpreterResult run_export(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    const WasmInterpreterExport* export,
    const WasmInterpreterTypedValueVector* args,
    WasmInterpreterTypedValueVector* out_results) {
  if (s_trace) {
    printf(">>> running export \"" PRIstringslice "\":\n",
           WASM_PRINTF_STRING_SLICE_ARG(export->name));
  }

  assert(export->kind == WASM_EXTERNAL_KIND_FUNC);
  return run_function(allocator, thread, export->index, args, out_results);
}

static WasmInterpreterResult run_export_by_name(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    WasmInterpreterModule* module,
    const WasmStringSlice* name,
    const WasmInterpreterTypedValueVector* args,
    WasmInterpreterTypedValueVector* out_results,
    RunVerbosity verbose) {
  WasmInterpreterExport* export =
      wasm_get_interpreter_export_by_name(module, name);
  if (!export)
    return WASM_INTERPRETER_UNKNOWN_EXPORT;
  if (export->kind != WASM_EXTERNAL_KIND_FUNC)
    return WASM_INTERPRETER_EXPORT_KIND_MISMATCH;
  return run_export(allocator, thread, export, args, out_results);
}

static WasmInterpreterResult get_global_export_by_name(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    WasmInterpreterModule* module,
    const WasmStringSlice* name,
    WasmInterpreterTypedValueVector* out_results) {
  WasmInterpreterExport* export =
      wasm_get_interpreter_export_by_name(module, name);
  if (!export)
    return WASM_INTERPRETER_UNKNOWN_EXPORT;
  if (export->kind != WASM_EXTERNAL_KIND_GLOBAL)
    return WASM_INTERPRETER_EXPORT_KIND_MISMATCH;

  assert(export->index < thread->env->globals.size);
  WasmInterpreterGlobal* global = &thread->env->globals.data[export->index];

  /* Don't clear out the vector, in case it is being reused. Just reset the
   * size to zero. */
  out_results->size = 0;
  wasm_append_interpreter_typed_value_value(allocator, out_results,
                                            &global->typed_value);
  return WASM_INTERPRETER_OK;
}

static void run_all_exports(WasmAllocator* allocator,
                            WasmInterpreterModule* module,
                            WasmInterpreterThread* thread,
                            RunVerbosity verbose) {
  WasmInterpreterTypedValueVector args;
  WasmInterpreterTypedValueVector results;
  WASM_ZERO_MEMORY(args);
  WASM_ZERO_MEMORY(results);
  uint32_t i;
  for (i = 0; i < module->exports.size; ++i) {
    WasmInterpreterExport* export = &module->exports.data[i];
    WasmInterpreterResult iresult =
        run_export(allocator, thread, export, &args, &results);
    if (verbose) {
      print_call(wasm_empty_string_slice(), export->name, &args, &results,
                 iresult);
    }
  }
  wasm_destroy_interpreter_typed_value_vector(allocator, &args);
  wasm_destroy_interpreter_typed_value_vector(allocator, &results);
}

static WasmResult read_module(WasmAllocator* allocator,
                              const char* module_filename,
                              WasmInterpreterEnvironment* env,
                              WasmBinaryErrorHandler* error_handler,
                              WasmInterpreterModule** out_module) {
  WasmResult result;
  void* data;
  size_t size;

  *out_module = NULL;

  result = wasm_read_file(allocator, module_filename, &data, &size);
  if (WASM_SUCCEEDED(result)) {
    WasmAllocator* memory_allocator = &g_wasm_libc_allocator;
    result = wasm_read_binary_interpreter(allocator, memory_allocator, env,
                                          data, size, &s_read_binary_options,
                                          error_handler, out_module);

    if (WASM_SUCCEEDED(result)) {
      if (s_verbose)
        wasm_disassemble_module(env, s_stdout_stream, *out_module);
    }
    wasm_free(allocator, data);
  }
  return result;
}

static WasmResult default_host_callback(const WasmInterpreterFunc* func,
                                        const WasmInterpreterFuncSignature* sig,
                                        uint32_t num_args,
                                        WasmInterpreterTypedValue* args,
                                        uint32_t num_results,
                                        WasmInterpreterTypedValue* out_results,
                                        void* user_data) {
  memset(out_results, 0, sizeof(WasmInterpreterTypedValue) * num_results);
  uint32_t i;
  for (i = 0; i < num_results; ++i)
    out_results[i].type = sig->result_types.data[i];

  WasmInterpreterTypedValueVector vec_args;
  vec_args.size = num_args;
  vec_args.data = args;

  WasmInterpreterTypedValueVector vec_results;
  vec_results.size = num_results;
  vec_results.data = out_results;

  printf("called host ");
  print_call(func->host.module_name, func->host.field_name, &vec_args,
             &vec_results, WASM_INTERPRETER_OK);
  return WASM_OK;
}

#define PRIimport "\"" PRIstringslice "." PRIstringslice "\""
#define PRINTF_IMPORT_ARG(x)                    \
  WASM_PRINTF_STRING_SLICE_ARG((x).module_name) \
  , WASM_PRINTF_STRING_SLICE_ARG((x).field_name)

static void WASM_PRINTF_FORMAT(2, 3)
    print_error(WasmPrintErrorCallback callback, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  callback.print_error(buffer, callback.user_data);
}

static WasmResult spectest_import_func(WasmInterpreterImport* import,
                                       WasmInterpreterFunc* func,
                                       WasmInterpreterFuncSignature* sig,
                                       WasmPrintErrorCallback callback,
                                       void* user_data) {
  if (wasm_string_slice_eq_cstr(&import->field_name, "print")) {
    func->host.callback = default_host_callback;
    return WASM_OK;
  } else {
    print_error(callback, "unknown host function import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WASM_ERROR;
  }
}

static WasmResult spectest_import_table(WasmInterpreterImport* import,
                                        WasmInterpreterTable* table,
                                        WasmPrintErrorCallback callback,
                                        void* user_data) {
  if (wasm_string_slice_eq_cstr(&import->field_name, "table")) {
    table->limits.has_max = WASM_TRUE;
    table->limits.initial = 10;
    table->limits.max = 20;
    return WASM_OK;
  } else {
    print_error(callback, "unknown host table import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WASM_ERROR;
  }
}

static WasmResult spectest_import_memory(WasmInterpreterImport* import,
                                         WasmInterpreterMemory* memory,
                                         WasmPrintErrorCallback callback,
                                         void* user_data) {
  if (wasm_string_slice_eq_cstr(&import->field_name, "memory")) {
    memory->page_limits.has_max = WASM_TRUE;
    memory->page_limits.initial = 1;
    memory->page_limits.max = 2;
    memory->byte_size = memory->page_limits.initial * WASM_MAX_PAGES;
    memory->data = wasm_alloc_zero(memory->allocator, memory->byte_size,
                                   WASM_DEFAULT_ALIGN);
    return WASM_OK;
  } else {
    print_error(callback, "unknown host memory import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WASM_ERROR;
  }
}

static WasmResult spectest_import_global(WasmInterpreterImport* import,
                                         WasmInterpreterGlobal* global,
                                         WasmPrintErrorCallback callback,
                                         void* user_data) {
  if (wasm_string_slice_eq_cstr(&import->field_name, "global")) {
    switch (global->typed_value.type) {
      case WASM_TYPE_I32:
        global->typed_value.value.i32 = 666;
        break;

      case WASM_TYPE_F32: {
        float value = 666.6f;
        memcpy(&global->typed_value.value.f32_bits, &value, sizeof(value));
        break;
      }

      case WASM_TYPE_I64:
        global->typed_value.value.i64 = 666;
        break;

      case WASM_TYPE_F64: {
        double value = 666.6;
        memcpy(&global->typed_value.value.f64_bits, &value, sizeof(value));
        break;
      }

      default:
        print_error(callback, "bad type for host global import " PRIimport,
                    PRINTF_IMPORT_ARG(*import));
        return WASM_ERROR;
    }

    return WASM_OK;
  } else {
    print_error(callback, "unknown host global import " PRIimport,
                PRINTF_IMPORT_ARG(*import));
    return WASM_ERROR;
  }
}

static void init_environment(WasmAllocator* allocator,
                             WasmInterpreterEnvironment* env) {
  wasm_init_interpreter_environment(allocator, env);
  WasmInterpreterModule* host_module = wasm_append_host_module(
      allocator, env, wasm_string_slice_from_cstr("spectest"));
  host_module->host.import_delegate.import_func = spectest_import_func;
  host_module->host.import_delegate.import_table = spectest_import_table;
  host_module->host.import_delegate.import_memory = spectest_import_memory;
  host_module->host.import_delegate.import_global = spectest_import_global;
}

static WasmResult read_and_run_module(WasmAllocator* allocator,
                                      const char* module_filename) {
  WasmResult result;
  WasmInterpreterEnvironment env;
  WasmInterpreterModule* module = NULL;
  WasmInterpreterThread thread;

  init_environment(allocator, &env);
  wasm_init_interpreter_thread(allocator, &env, &thread, &s_thread_options);
  result =
      read_module(allocator, module_filename, &env, &s_error_handler, &module);
  if (WASM_SUCCEEDED(result)) {
    WasmInterpreterResult iresult =
        run_start_function(allocator, &thread, module);
    if (iresult == WASM_INTERPRETER_OK) {
      if (s_run_all_exports)
        run_all_exports(allocator, module, &thread, RUN_VERBOSE);
    } else {
      print_interpreter_result("error running start function", iresult);
    }
  }
  wasm_destroy_interpreter_thread(allocator, &thread);
  wasm_destroy_interpreter_environment(allocator, &env);
  return result;
}

WASM_DEFINE_VECTOR(interpreter_thread, WasmInterpreterThread);

/* An extremely simple JSON parser that only knows how to parse the expected
 * format from wast2wasm. */
typedef struct Context {
  WasmAllocator* allocator;
  WasmInterpreterEnvironment env;
  WasmInterpreterThread thread;
  WasmInterpreterModule* last_module;

  /* Parsing info */
  char* json_data;
  size_t json_data_size;
  WasmStringSlice source_filename;
  size_t json_offset;
  WasmLocation loc;
  WasmLocation prev_loc;
  WasmBool has_prev_loc;
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
  WasmStringSlice module_name;
  WasmStringSlice field_name;
  WasmInterpreterTypedValueVector args;
} Action;

#define CHECK_RESULT(x)  \
  do {                   \
    if (WASM_FAILED(x))  \
      return WASM_ERROR; \
  } while (0)

#define EXPECT(x) CHECK_RESULT(expect(ctx, x))
#define EXPECT_KEY(x) CHECK_RESULT(expect_key(ctx, x))
#define PARSE_KEY_STRING_VALUE(key, value) \
  CHECK_RESULT(parse_key_string_value(ctx, key, value))

static void WASM_PRINTF_FORMAT(2, 3)
    print_parse_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  fprintf(stderr, "%s:%d:%d: %s\n", ctx->loc.filename, ctx->loc.line,
          ctx->loc.first_column, buffer);
}

static void WASM_PRINTF_FORMAT(2, 3)
    print_command_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  printf(PRIstringslice ":%u: %s\n",
         WASM_PRINTF_STRING_SLICE_ARG(ctx->source_filename),
         ctx->command_line_number, buffer);
}

static void putback_char(Context* ctx) {
  assert(ctx->has_prev_loc);
  ctx->json_offset--;
  ctx->loc = ctx->prev_loc;
  ctx->has_prev_loc = WASM_FALSE;
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
  ctx->has_prev_loc = WASM_TRUE;
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

static WasmBool match(Context* ctx, const char* s) {
  skip_whitespace(ctx);
  WasmLocation start_loc = ctx->loc;
  size_t start_offset = ctx->json_offset;
  while (*s && *s == read_char(ctx))
    s++;

  if (*s == 0) {
    return WASM_TRUE;
  } else {
    ctx->json_offset = start_offset;
    ctx->loc = start_loc;
    return WASM_FALSE;
  }
}

static WasmResult expect(Context* ctx, const char* s) {
  if (match(ctx, s)) {
    return WASM_OK;
  } else {
    print_parse_error(ctx, "expected %s", s);
    return WASM_ERROR;
  }
}

static WasmResult expect_key(Context* ctx, const char* key) {
  size_t keylen = strlen(key);
  size_t quoted_len = keylen + 2 + 1;
  char* quoted = alloca(quoted_len);
  wasm_snprintf(quoted, quoted_len, "\"%s\"", key);
  EXPECT(quoted);
  EXPECT(":");
  return WASM_OK;
}

static WasmResult parse_uint32(Context* ctx, uint32_t* out_int) {
  uint32_t result = 0;
  skip_whitespace(ctx);
  while (1) {
    int c = read_char(ctx);
    if (c >= '0' && c <= '9') {
      uint32_t last_result = result;
      result = result * 10 + (uint32_t)(c - '0');
      if (result < last_result) {
        print_parse_error(ctx, "uint32 overflow");
        return WASM_ERROR;
      }
    } else {
      putback_char(ctx);
      break;
    }
  }
  *out_int = result;
  return WASM_OK;
}

static WasmResult parse_string(Context* ctx, WasmStringSlice* out_string) {
  WASM_ZERO_MEMORY(*out_string);

  skip_whitespace(ctx);
  if (read_char(ctx) != '"') {
    print_parse_error(ctx, "expected string");
    return WASM_ERROR;
  }
  /* Modify json_data in-place so we can use the WasmStringSlice directly
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
        return WASM_ERROR;
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
          return WASM_ERROR;
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
  return WASM_OK;
}

static WasmResult parse_key_string_value(Context* ctx,
                                         const char* key,
                                         WasmStringSlice* out_string) {
  WASM_ZERO_MEMORY(*out_string);
  EXPECT_KEY(key);
  return parse_string(ctx, out_string);
}

static WasmResult parse_opt_name_string_value(Context* ctx,
                                              WasmStringSlice* out_string) {
  WASM_ZERO_MEMORY(*out_string);
  if (match(ctx, "\"name\"")) {
    EXPECT(":");
    CHECK_RESULT(parse_string(ctx, out_string));
    EXPECT(",");
  }
  return WASM_OK;
}

static WasmResult parse_line(Context* ctx) {
  EXPECT_KEY("line");
  CHECK_RESULT(parse_uint32(ctx, &ctx->command_line_number));
  return WASM_OK;
}

static WasmResult parse_type_object(Context* ctx, WasmType* out_type) {
  WasmStringSlice type_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT("}");

  if (wasm_string_slice_eq_cstr(&type_str, "i32")) {
    *out_type = WASM_TYPE_I32;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "f32")) {
    *out_type = WASM_TYPE_F32;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "i64")) {
    *out_type = WASM_TYPE_I64;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "f64")) {
    *out_type = WASM_TYPE_F64;
    return WASM_OK;
  } else {
    print_parse_error(ctx, "unknown type: \"" PRIstringslice "\"",
                      WASM_PRINTF_STRING_SLICE_ARG(type_str));
    return WASM_ERROR;
  }
}

static WasmResult parse_type_vector(Context* ctx, WasmTypeVector* out_types) {
  WASM_ZERO_MEMORY(*out_types);
  EXPECT("[");
  WasmBool first = WASM_TRUE;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    WasmType type;
    CHECK_RESULT(parse_type_object(ctx, &type));
    first = WASM_FALSE;
    wasm_append_type_value(ctx->allocator, out_types, &type);
  }
  return WASM_OK;
}

static WasmResult parse_const(Context* ctx,
                              WasmInterpreterTypedValue* out_value) {
  WasmStringSlice type_str;
  WasmStringSlice value_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT(",");
  PARSE_KEY_STRING_VALUE("value", &value_str);
  EXPECT("}");

  const char* value_start = value_str.start;
  const char* value_end = value_str.start + value_str.length;

  if (wasm_string_slice_eq_cstr(&type_str, "i32")) {
    uint32_t value;
    CHECK_RESULT(wasm_parse_int32(value_start, value_end, &value,
                                  WASM_PARSE_UNSIGNED_ONLY));
    out_value->type = WASM_TYPE_I32;
    out_value->value.i32 = value;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "f32")) {
    uint32_t value_bits;
    CHECK_RESULT(wasm_parse_int32(value_start, value_end, &value_bits,
                                  WASM_PARSE_UNSIGNED_ONLY));
    out_value->type = WASM_TYPE_F32;
    out_value->value.f32_bits = value_bits;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "i64")) {
    uint64_t value;
    CHECK_RESULT(wasm_parse_int64(value_start, value_end, &value,
                                  WASM_PARSE_UNSIGNED_ONLY));
    out_value->type = WASM_TYPE_I64;
    out_value->value.i64 = value;
    return WASM_OK;
  } else if (wasm_string_slice_eq_cstr(&type_str, "f64")) {
    uint64_t value_bits;
    CHECK_RESULT(wasm_parse_int64(value_start, value_end, &value_bits,
                                  WASM_PARSE_UNSIGNED_ONLY));
    out_value->type = WASM_TYPE_F64;
    out_value->value.f64_bits = value_bits;
    return WASM_OK;
  } else {
    print_parse_error(ctx, "unknown type: \"" PRIstringslice "\"",
                      WASM_PRINTF_STRING_SLICE_ARG(type_str));
    return WASM_ERROR;
  }
}

static WasmResult parse_const_vector(
    Context* ctx,
    WasmInterpreterTypedValueVector* out_values) {
  WASM_ZERO_MEMORY(*out_values);
  EXPECT("[");
  WasmBool first = WASM_TRUE;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    WasmInterpreterTypedValue value;
    CHECK_RESULT(parse_const(ctx, &value));
    wasm_append_interpreter_typed_value_value(ctx->allocator, out_values,
                                              &value);
    first = WASM_FALSE;
  }
  return WASM_OK;
}

static WasmResult parse_action(Context* ctx, Action* out_action) {
  WASM_ZERO_MEMORY(*out_action);
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
  return WASM_OK;
}

static char* create_module_path(Context* ctx, WasmStringSlice filename) {
  const char* spec_json_filename = ctx->loc.filename;
  WasmStringSlice dirname = get_dirname(spec_json_filename);
  size_t path_len = dirname.length + 1 + filename.length + 1;
  char* path = wasm_alloc(ctx->allocator, path_len, 1);

  if (dirname.length == 0) {
    wasm_snprintf(path, path_len, PRIstringslice,
                  WASM_PRINTF_STRING_SLICE_ARG(filename));
  } else {
    wasm_snprintf(path, path_len, PRIstringslice "/" PRIstringslice,
                  WASM_PRINTF_STRING_SLICE_ARG(dirname),
                  WASM_PRINTF_STRING_SLICE_ARG(filename));
  }

  return path;
}

static WasmResult on_module_command(Context* ctx,
                                    WasmStringSlice filename,
                                    WasmStringSlice name) {
  char* path = create_module_path(ctx, filename);
  WasmInterpreterEnvironmentMark mark =
      wasm_mark_interpreter_environment(&ctx->env);
  WasmResult result = read_module(ctx->allocator, path, &ctx->env,
                                  &s_error_handler, &ctx->last_module);

  if (WASM_FAILED(result)) {
    wasm_reset_interpreter_environment_to_mark(ctx->allocator, &ctx->env, mark);
    print_command_error(ctx, "error reading module: \"%s\"", path);
    wasm_free(ctx->allocator, path);
    return WASM_ERROR;
  }

  wasm_free(ctx->allocator, path);

  WasmInterpreterResult iresult =
      run_start_function(ctx->allocator, &ctx->thread, ctx->last_module);
  if (iresult != WASM_INTERPRETER_OK) {
    wasm_reset_interpreter_environment_to_mark(ctx->allocator, &ctx->env, mark);
    print_interpreter_result("error running start function", iresult);
    return WASM_ERROR;
  }

  if (!wasm_string_slice_is_empty(&name)) {
    ctx->last_module->name = wasm_dup_string_slice(ctx->allocator, name);

    /* The binding also needs its own copy of the name. */
    WasmStringSlice binding_name = wasm_dup_string_slice(ctx->allocator, name);
    WasmBinding* binding = wasm_insert_binding(
        ctx->allocator, &ctx->env.module_bindings, &binding_name);
    binding->index = ctx->env.modules.size - 1;
  }
  return WASM_OK;
}

static WasmResult run_action(Context* ctx,
                             Action* action,
                             WasmInterpreterResult* out_iresult,
                             WasmInterpreterTypedValueVector* out_results,
                             RunVerbosity verbose) {
  WASM_ZERO_MEMORY(*out_results);

  int module_index;
  if (!wasm_string_slice_is_empty(&action->module_name)) {
    module_index = wasm_find_binding_index_by_name(&ctx->env.module_bindings,
                                                   &action->module_name);
  } else {
    module_index = (int)ctx->env.modules.size - 1;
  }

  assert(module_index < (int)ctx->env.modules.size);
  WasmInterpreterModule* module = &ctx->env.modules.data[module_index];

  switch (action->type) {
    case ACTION_TYPE_INVOKE:
      *out_iresult = run_export_by_name(ctx->allocator, &ctx->thread, module,
                                        &action->field_name, &action->args,
                                        out_results, verbose);
      if (verbose) {
        print_call(wasm_empty_string_slice(), action->field_name, &action->args,
                   out_results, *out_iresult);
      }
      return WASM_OK;

    case ACTION_TYPE_GET: {
      *out_iresult =
          get_global_export_by_name(ctx->allocator, &ctx->thread, module,
                                    &action->field_name, out_results);
      return WASM_OK;
    }

    default:
      print_command_error(ctx, "invalid action type %d", action->type);
      return WASM_ERROR;
  }
}

static WasmResult on_action_command(Context* ctx, Action* action) {
  WasmInterpreterTypedValueVector results;
  WasmInterpreterResult iresult;

  ctx->total++;
  WasmResult result = run_action(ctx, action, &iresult, &results, RUN_VERBOSE);
  if (WASM_SUCCEEDED(result)) {
    if (iresult == WASM_INTERPRETER_OK) {
      ctx->passed++;
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WASM_ERROR;
    }
  }

  wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &results);
  return result;
}

static WasmBinaryErrorHandler* new_custom_error_handler(Context* ctx,
                                                        const char* desc) {
  size_t header_size = ctx->source_filename.length + strlen(desc) + 100;
  char* header = wasm_alloc(ctx->allocator, header_size, 1);
  wasm_snprintf(header, header_size, PRIstringslice ":%d: %s passed",
                WASM_PRINTF_STRING_SLICE_ARG(ctx->source_filename),
                ctx->command_line_number, desc);

  WasmDefaultErrorHandlerInfo* info = wasm_alloc_zero(
      ctx->allocator, sizeof(WasmDefaultErrorHandlerInfo), WASM_DEFAULT_ALIGN);
  info->header = header;
  info->out_file = stdout;
  info->print_header = WASM_PRINT_ERROR_HEADER_ONCE;

  WasmBinaryErrorHandler* error_handler = wasm_alloc_zero(
      ctx->allocator, sizeof(WasmBinaryErrorHandler), WASM_DEFAULT_ALIGN);
  error_handler->on_error = wasm_default_binary_error_callback;
  error_handler->user_data = info;
  return error_handler;
}

static void destroy_custom_error_handler(
    WasmAllocator* allocator,
    WasmBinaryErrorHandler* error_handler) {
  WasmDefaultErrorHandlerInfo* info = error_handler->user_data;
  wasm_free(allocator, (void*)info->header);
  wasm_free(allocator, info);
  wasm_free(allocator, error_handler);
}

static WasmResult on_assert_malformed_command(Context* ctx,
                                              WasmStringSlice filename,
                                              WasmStringSlice text) {
  WasmBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_malformed");
  WasmInterpreterEnvironment env;
  WASM_ZERO_MEMORY(env);
  init_environment(ctx->allocator, &env);

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WasmInterpreterModule* module;
  WasmResult result =
      read_module(ctx->allocator, path, &env, error_handler, &module);
  if (WASM_FAILED(result)) {
    ctx->passed++;
    result = WASM_OK;
  } else {
    print_command_error(ctx, "expected module to be malformed: \"%s\"", path);
    result = WASM_ERROR;
  }

  wasm_free(ctx->allocator, path);
  wasm_destroy_interpreter_environment(ctx->allocator, &env);
  destroy_custom_error_handler(ctx->allocator, error_handler);
  return result;
}

static WasmResult on_register_command(Context* ctx,
                                      WasmStringSlice name,
                                      WasmStringSlice as) {
  int module_index;
  if (!wasm_string_slice_is_empty(&name)) {
    /* The module names can be different than their registered names. We don't
     * keep a hash for the module names (just the registered names), so we'll
     * just iterate over all the modules to find it. */
    size_t i;
    module_index = -1;
    for (i = 0; i < ctx->env.modules.size; ++i) {
      const WasmStringSlice* module_name = &ctx->env.modules.data[i].name;
      if (!wasm_string_slice_is_empty(module_name) &&
          wasm_string_slices_are_equal(&name, module_name)) {
        module_index = (int)i;
        break;
      }
    }
  } else {
    module_index = (int)ctx->env.modules.size - 1;
  }

  if (module_index < 0 || module_index >= (int)ctx->env.modules.size) {
    print_command_error(ctx, "unknown module in register");
    return WASM_ERROR;
  }

  WasmStringSlice dup_as = wasm_dup_string_slice(ctx->allocator, as);
  WasmBinding* binding = wasm_insert_binding(
      ctx->allocator, &ctx->env.registered_module_bindings, &dup_as);
  binding->index = module_index;
  return WASM_OK;
}

static WasmResult on_assert_unlinkable_command(Context* ctx,
                                               WasmStringSlice filename,
                                               WasmStringSlice text) {
  WasmBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_unlinkable");

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WasmInterpreterModule* module;
  WasmInterpreterEnvironmentMark mark =
      wasm_mark_interpreter_environment(&ctx->env);
  WasmResult result =
      read_module(ctx->allocator, path, &ctx->env, error_handler, &module);
  wasm_reset_interpreter_environment_to_mark(ctx->allocator, &ctx->env, mark);

  if (WASM_FAILED(result)) {
    ctx->passed++;
    result = WASM_OK;
  } else {
    print_command_error(ctx, "expected module to be unlinkable: \"%s\"", path);
    result = WASM_ERROR;
  }

  wasm_free(ctx->allocator, path);
  destroy_custom_error_handler(ctx->allocator, error_handler);
  return result;
}

static WasmResult on_assert_invalid_command(Context* ctx,
                                            WasmStringSlice filename,
                                            WasmStringSlice text) {
  /* TODO: need to correctly support invalid asserts in interpreter */
  return WASM_OK;
#if 0
  WasmBinaryErrorHandler* error_handler =
      new_custom_error_handler(ctx, "assert_invalid");

  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WasmInterpreterModule* module;
  WasmInterpreterEnvironmentMark mark =
      wasm_mark_interpreter_environment(&ctx->env);
  WasmResult result =
      read_module(ctx->allocator, path, &ctx->env, error_handler, &module);
  wasm_reset_interpreter_environment_to_mark(ctx->allocator, &ctx->env, mark);

  if (WASM_FAILED(result)) {
    ctx->passed++;
    result = WASM_OK;
  } else {
    print_command_error(ctx, "expected module to be invalid: \"%s\"", path);
    result = WASM_ERROR;
  }

  wasm_free(ctx->allocator, path);
  destroy_custom_error_handler(ctx->allocator, error_handler);
  return result;
#endif
}

static WasmResult on_assert_uninstantiable_command(Context* ctx,
                                                   WasmStringSlice filename,
                                                   WasmStringSlice text) {
  ctx->total++;
  char* path = create_module_path(ctx, filename);
  WasmInterpreterModule* module;
  WasmInterpreterEnvironmentMark mark =
      wasm_mark_interpreter_environment(&ctx->env);
  WasmResult result =
      read_module(ctx->allocator, path, &ctx->env, &s_error_handler, &module);

  if (WASM_SUCCEEDED(result)) {
    WasmInterpreterResult iresult =
        run_start_function(ctx->allocator, &ctx->thread, module);
    if (iresult == WASM_INTERPRETER_OK) {
      print_command_error(ctx, "expected error running start function: \"%s\"",
                          path);
      result = WASM_ERROR;
    } else {
      ctx->passed++;
      result = WASM_OK;
    }
  } else {
    print_command_error(ctx, "error reading module: \"%s\"", path);
    result = WASM_ERROR;
  }

  wasm_reset_interpreter_environment_to_mark(ctx->allocator, &ctx->env, mark);
  wasm_free(ctx->allocator, path);
  return result;
}

static WasmBool typed_values_are_equal(const WasmInterpreterTypedValue* tv1,
                                       const WasmInterpreterTypedValue* tv2) {
  if (tv1->type != tv2->type)
    return WASM_FALSE;

  switch (tv1->type) {
    case WASM_TYPE_I32: return tv1->value.i32 == tv2->value.i32;
    case WASM_TYPE_F32: return tv1->value.f32_bits == tv2->value.f32_bits;
    case WASM_TYPE_I64: return tv1->value.i64 == tv2->value.i64;
    case WASM_TYPE_F64: return tv1->value.f64_bits == tv2->value.f64_bits;
    default: assert(0); return WASM_FALSE;
  }
}

static WasmResult on_assert_return_command(
    Context* ctx,
    Action* action,
    WasmInterpreterTypedValueVector* expected) {
  WasmInterpreterTypedValueVector results;
  WasmInterpreterResult iresult;

  ctx->total++;
  WasmResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);

  if (WASM_SUCCEEDED(result)) {
    if (iresult == WASM_INTERPRETER_OK) {
      if (results.size == expected->size) {
        size_t i;
        for (i = 0; i < results.size; ++i) {
          const WasmInterpreterTypedValue* expected_tv = &expected->data[i];
          const WasmInterpreterTypedValue* actual_tv = &results.data[i];
          if (!typed_values_are_equal(expected_tv, actual_tv)) {
            char expected_str[MAX_TYPED_VALUE_CHARS];
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(expected_str, sizeof(expected_str), expected_tv);
            sprint_typed_value(actual_str, sizeof(actual_str), actual_tv);
            print_command_error(ctx, "mismatch in result %" PRIzd
                                     " of assert_return: expected %s, got %s",
                                i, expected_str, actual_str);
            result = WASM_ERROR;
          }
        }
      } else {
        print_command_error(
            ctx, "result length mismatch in assert_return: expected %" PRIzd
                 ", got %" PRIzd,
            expected->size, results.size);
        result = WASM_ERROR;
      }
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WASM_ERROR;
    }
  }

  if (WASM_SUCCEEDED(result))
    ctx->passed++;

  wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &results);
  return result;
}

static WasmResult on_assert_return_nan_command(Context* ctx, Action* action) {
  WasmInterpreterTypedValueVector results;
  WasmInterpreterResult iresult;

  ctx->total++;
  WasmResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
  if (WASM_SUCCEEDED(result)) {
    if (iresult == WASM_INTERPRETER_OK) {
      if (results.size != 1) {
        print_command_error(ctx, "expected one result, got %" PRIzd,
                            results.size);
        result = WASM_ERROR;
      }

      const WasmInterpreterTypedValue* actual = &results.data[0];
      switch (actual->type) {
        case WASM_TYPE_F32:
          if (!is_nan_f32(actual->value.f32_bits)) {
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(actual_str, sizeof(actual_str), actual);
            print_command_error(ctx, "expected result to be nan, got %s",
                                actual_str);
            result = WASM_ERROR;
          }
          break;

        case WASM_TYPE_F64:
          if (!is_nan_f64(actual->value.f64_bits)) {
            char actual_str[MAX_TYPED_VALUE_CHARS];
            sprint_typed_value(actual_str, sizeof(actual_str), actual);
            print_command_error(ctx, "expected result to be nan, got %s",
                                actual_str);
            result = WASM_ERROR;
          }
          break;

        default:
          print_command_error(ctx,
                              "expected result type to be f32 or f64, got %s",
                              wasm_get_type_name(actual->type));
          result = WASM_ERROR;
          break;
      }
    } else {
      print_command_error(ctx, "unexpected trap: %s", s_trap_strings[iresult]);
      result = WASM_ERROR;
    }
  }

  if (WASM_SUCCEEDED(result))
    ctx->passed++;

  wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &results);
  return WASM_OK;
}

static WasmResult on_assert_trap_command(Context* ctx,
                                         Action* action,
                                         WasmStringSlice text) {
  WasmInterpreterTypedValueVector results;
  WasmInterpreterResult iresult;

  ctx->total++;
  WasmResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
  if (WASM_SUCCEEDED(result)) {
    if (iresult != WASM_INTERPRETER_OK) {
      ctx->passed++;
    } else {
      print_command_error(ctx, "expected trap: \"" PRIstringslice "\"",
                          WASM_PRINTF_STRING_SLICE_ARG(text));
      result = WASM_ERROR;
    }
  }

  wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &results);
  return result;
}

static WasmResult on_assert_exhaustion_command(Context* ctx,
                                         Action* action) {
    WasmInterpreterTypedValueVector results;
    WasmInterpreterResult iresult;

    ctx->total++;
    WasmResult result = run_action(ctx, action, &iresult, &results, RUN_QUIET);
    if (WASM_SUCCEEDED(result)) {
        if (iresult == WASM_INTERPRETER_TRAP_CALL_STACK_EXHAUSTED) {
            ctx->passed++;
        } else {
            print_command_error(ctx, "expected call stack exhaustion");
            result = WASM_ERROR;
        }
    }

    wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &results);
    return result;
}

static void destroy_action(WasmAllocator* allocator, Action* action) {
  wasm_destroy_interpreter_typed_value_vector(allocator, &action->args);
}

static WasmResult parse_command(Context* ctx) {
  EXPECT("{");
  EXPECT_KEY("type");
  if (match(ctx, "\"module\"")) {
    WasmStringSlice name;
    WasmStringSlice filename;
    WASM_ZERO_MEMORY(name);
    WASM_ZERO_MEMORY(filename);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_opt_name_string_value(ctx, &name));
    PARSE_KEY_STRING_VALUE("filename", &filename);
    on_module_command(ctx, filename, name);
  } else if (match(ctx, "\"action\"")) {
    Action action;
    WASM_ZERO_MEMORY(action);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    on_action_command(ctx, &action);
    destroy_action(ctx->allocator, &action);
  } else if (match(ctx, "\"register\"")) {
    WasmStringSlice as;
    WasmStringSlice name;
    WASM_ZERO_MEMORY(as);
    WASM_ZERO_MEMORY(name);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_opt_name_string_value(ctx, &name));
    PARSE_KEY_STRING_VALUE("as", &as);
    on_register_command(ctx, name, as);
  } else if (match(ctx, "\"assert_malformed\"")) {
    WasmStringSlice filename;
    WasmStringSlice text;
    WASM_ZERO_MEMORY(filename);
    WASM_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_malformed_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_invalid\"")) {
    WasmStringSlice filename;
    WasmStringSlice text;
    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_invalid_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_unlinkable\"")) {
    WasmStringSlice filename;
    WasmStringSlice text;
    WASM_ZERO_MEMORY(filename);
    WASM_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_unlinkable_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_uninstantiable\"")) {
    WasmStringSlice filename;
    WasmStringSlice text;
    WASM_ZERO_MEMORY(filename);
    WASM_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("filename", &filename);
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_uninstantiable_command(ctx, filename, text);
  } else if (match(ctx, "\"assert_return\"")) {
    Action action;
    WasmInterpreterTypedValueVector expected;
    WASM_ZERO_MEMORY(action);
    WASM_ZERO_MEMORY(expected);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    EXPECT_KEY("expected");
    CHECK_RESULT(parse_const_vector(ctx, &expected));
    on_assert_return_command(ctx, &action, &expected);
    wasm_destroy_interpreter_typed_value_vector(ctx->allocator, &expected);
    destroy_action(ctx->allocator, &action);
  } else if (match(ctx, "\"assert_return_nan\"")) {
    Action action;
    WasmTypeVector expected;
    WASM_ZERO_MEMORY(action);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    /* Not needed for wasm-interp, but useful for other parsers. */
    EXPECT_KEY("expected");
    CHECK_RESULT(parse_type_vector(ctx, &expected));
    on_assert_return_nan_command(ctx, &action);
    wasm_destroy_type_vector(ctx->allocator, &expected);
    destroy_action(ctx->allocator, &action);
  } else if (match(ctx, "\"assert_trap\"")) {
    Action action;
    WasmStringSlice text;
    WASM_ZERO_MEMORY(action);
    WASM_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &text);
    on_assert_trap_command(ctx, &action, text);
    destroy_action(ctx->allocator, &action);
  } else if (match(ctx, "\"assert_exhaustion\"")) {
    Action action;
    WasmStringSlice text;
    WASM_ZERO_MEMORY(action);
    WASM_ZERO_MEMORY(text);

    EXPECT(",");
    CHECK_RESULT(parse_line(ctx));
    EXPECT(",");
    CHECK_RESULT(parse_action(ctx, &action));
    on_assert_exhaustion_command(ctx, &action);
    destroy_action(ctx->allocator, &action);
  } else {
    print_command_error(ctx, "unknown command type");
    return WASM_ERROR;
  }
  EXPECT("}");
  return WASM_OK;
}

static WasmResult parse_commands(Context* ctx) {
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("source_filename", &ctx->source_filename);
  EXPECT(",");
  EXPECT_KEY("commands");
  EXPECT("[");
  WasmBool first = WASM_TRUE;
  while (!match(ctx, "]")) {
    if (!first)
      EXPECT(",");
    CHECK_RESULT(parse_command(ctx));
    first = WASM_FALSE;
  }
  EXPECT("}");
  return WASM_OK;
}

static void destroy_context(Context* ctx) {
  wasm_destroy_interpreter_thread(ctx->allocator, &ctx->thread);
  wasm_destroy_interpreter_environment(ctx->allocator, &ctx->env);
  wasm_free(ctx->allocator, ctx->json_data);
}

static WasmResult read_and_run_spec_json(WasmAllocator* allocator,
                                         const char* spec_json_filename) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.loc.filename = spec_json_filename;
  ctx.loc.line = 1;
  ctx.loc.first_column = 1;
  init_environment(allocator, &ctx.env);
  wasm_init_interpreter_thread(allocator, &ctx.env, &ctx.thread,
                               &s_thread_options);

  void* data;
  size_t size;
  WasmResult result =
      wasm_read_file(allocator, spec_json_filename, &data, &size);
  if (WASM_FAILED(result))
    return WASM_ERROR;

  ctx.json_data = data;
  ctx.json_data_size = size;

  result = parse_commands(&ctx);
  printf("%d/%d tests passed.\n", ctx.passed, ctx.total);
  destroy_context(&ctx);
  return result;
}

int main(int argc, char** argv) {
  WasmStackAllocator stack_allocator;
  WasmAllocator* allocator;

  wasm_init_stdio();
  parse_options(argc, argv);

  s_stdout_stream = wasm_init_stdout_stream();

  if (s_use_libc_allocator) {
    allocator = &g_wasm_libc_allocator;
  } else {
    wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
    allocator = &stack_allocator.allocator;
  }
  WasmResult result;
  if (s_spec) {
    result = read_and_run_spec_json(allocator, s_infile);
  } else {
    result = read_and_run_module(allocator, s_infile);
  }

  wasm_print_allocator_stats(allocator);
  wasm_destroy_allocator(allocator);
  return result;
}
