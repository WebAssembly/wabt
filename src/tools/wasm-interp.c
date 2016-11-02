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

#include "wasm-allocator.h"
#include "wasm-binary-reader.h"
#include "wasm-binary-reader-interpreter.h"
#include "wasm-interpreter.h"
#include "wasm-option-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-stream.h"

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

static void print_typed_value(WasmInterpreterTypedValue* tv) {
  switch (tv->type) {
    case WASM_TYPE_I32:
      printf("i32:%u", tv->value.i32);
      break;

    case WASM_TYPE_I64:
      printf("i64:%" PRIu64, tv->value.i64);
      break;

    case WASM_TYPE_F32: {
      float value;
      memcpy(&value, &tv->value.f32_bits, sizeof(float));
      printf("f32:%g", value);
      break;
    }

    case WASM_TYPE_F64: {
      double value;
      memcpy(&value, &tv->value.f64_bits, sizeof(double));
      printf("f64:%g", value);
      break;
    }

    default:
      assert(0);
      break;
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
  if (iresult != WASM_INTERPRETER_RETURNED) {
    if (s_trace)
      printf("!!! trapped: %s\n", s_trap_strings[iresult]);
    return iresult;
  }
  /* use OK instead of RETURNED for consistency */
  return WASM_INTERPRETER_OK;
}

static WasmInterpreterResult run_function(WasmInterpreterThread* thread,
                                          uint32_t func_index) {
  assert(func_index < thread->env->funcs.size);
  WasmInterpreterFunc* func = &thread->env->funcs.data[func_index];
  if (func->is_host)
    return wasm_call_host(thread, func);
  else
    return run_defined_function(thread, func->defined.offset);
}

static WasmResult run_start_function(WasmInterpreterThread* thread) {
  WasmResult result = WASM_OK;
  if (thread->module->defined.start_func_index != WASM_INVALID_INDEX) {
    if (s_trace)
      printf(">>> running start function:\n");
    WasmInterpreterResult iresult =
        run_function(thread, thread->module->defined.start_func_index);
    if (iresult != WASM_INTERPRETER_OK) {
      /* trap */
      fprintf(stderr, "error: %s\n", s_trap_strings[iresult]);
      result = WASM_ERROR;
    }
  }
  return result;
}

static WasmInterpreterFuncSignature* get_export_signature(
    WasmInterpreterEnvironment* env,
    WasmInterpreterExport* export) {
  assert(export->kind == WASM_EXTERNAL_KIND_FUNC);
  uint32_t func_index = export->index;
  uint32_t sig_index = env->funcs.data[func_index].sig_index;
  assert(sig_index < env->sigs.size);
  return &env->sigs.data[sig_index];
}

static WasmInterpreterResult run_export(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    WasmInterpreterExport* export,
    WasmInterpreterTypedValueVector* out_results) {
  assert(export->kind == WASM_EXTERNAL_KIND_FUNC);
  WasmInterpreterFuncSignature* sig = get_export_signature(thread->env, export);

  /* push all 0 values as arguments */
  assert(sig->param_types.size < thread->value_stack.size);
  size_t num_args = sig->param_types.size;
  thread->value_stack_top = &thread->value_stack.data[num_args];
  memset(thread->value_stack.data, 0, num_args * sizeof(WasmInterpreterValue));

  WasmInterpreterResult result = run_function(thread, export->index);

  if (result == WASM_INTERPRETER_OK) {
    size_t expected_results = sig->result_types.size;
    size_t value_stack_depth =
        thread->value_stack_top - thread->value_stack.data;
    WASM_USE(value_stack_depth);
    assert(expected_results == value_stack_depth);

    if (out_results) {
      wasm_resize_interpreter_typed_value_vector(allocator, out_results,
                                                 expected_results);
      size_t i;
      for (i = 0; i < expected_results; ++i) {
        WasmInterpreterTypedValue actual_result;
        actual_result.type = sig->result_types.data[i];
        actual_result.value = thread->value_stack.data[i];

        if (out_results) {
          /* copy as many results as the caller wants */
          out_results->data[i] = actual_result;
        }
      }
    }
  }

  thread->value_stack_top = thread->value_stack.data;
  thread->call_stack_top = thread->call_stack.data;

  return result;
}

static WasmInterpreterResult run_export_wrapper(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    WasmInterpreterExport* export,
    WasmInterpreterTypedValueVector* out_results,
    RunVerbosity verbose) {
  if (s_trace) {
    printf(">>> running export \"" PRIstringslice "\":\n",
           WASM_PRINTF_STRING_SLICE_ARG(export->name));
  }

  WasmInterpreterResult result =
      run_export(allocator, thread, export, out_results);

  if (verbose) {
    WasmInterpreterFuncSignature* sig =
        get_export_signature(thread->env, export);
    printf(PRIstringslice "(", WASM_PRINTF_STRING_SLICE_ARG(export->name));
    size_t i;
    for (i = 0; i < sig->param_types.size; ++i) {
      printf("0");
      if (i != sig->param_types.size - 1)
        printf(", ");
    }
    printf(") => ");

    if (result == WASM_INTERPRETER_OK) {
      assert(out_results);
      for (i = 0; i < out_results->size; ++i) {
        print_typed_value(&out_results->data[i]);
        if (i != out_results->size - 1)
          printf(", ");
      }
      printf("\n");
    } else {
      /* trap */
      printf("error: %s\n", s_trap_strings[result]);
    }
  }
  return result;
}

static WasmResult run_export_by_name(
    WasmAllocator* allocator,
    WasmInterpreterThread* thread,
    WasmStringSlice* name,
    WasmInterpreterResult* out_iresult,
    WasmInterpreterTypedValueVector* out_results,
    RunVerbosity verbose) {
  WasmInterpreterExport* export =
      wasm_get_interpreter_export_by_name(thread->module, name);
  if (!export)
    return WASM_ERROR;

  *out_iresult =
      run_export_wrapper(allocator, thread, export, out_results, verbose);
  return WASM_OK;
}

static void run_all_exports(WasmAllocator* allocator,
                            WasmInterpreterModule* module,
                            WasmInterpreterThread* thread,
                            RunVerbosity verbose) {
  WasmInterpreterTypedValueVector results;
  WASM_ZERO_MEMORY(results);
  uint32_t i;
  for (i = 0; i < module->exports.size; ++i) {
    WasmInterpreterExport* export = &module->exports.data[i];
    run_export_wrapper(allocator, thread, export, &results, verbose);
  }
  wasm_destroy_interpreter_typed_value_vector(allocator, &results);
}

static WasmResult read_module(WasmAllocator* allocator,
                              const char* module_filename,
                              WasmInterpreterEnvironment* env,
                              WasmInterpreterModule** out_module,
                              WasmInterpreterThread* out_thread) {
  WasmResult result;
  void* data;
  size_t size;
  WASM_ZERO_MEMORY(*out_thread);
  result = wasm_read_file(allocator, module_filename, &data, &size);
  if (WASM_SUCCEEDED(result)) {
    WasmAllocator* memory_allocator = &g_wasm_libc_allocator;
    result = wasm_read_binary_interpreter(allocator, memory_allocator, env,
                                          data, size, &s_read_binary_options,
                                          &s_error_handler, out_module);

    if (WASM_SUCCEEDED(result)) {
      if (s_verbose)
        wasm_disassemble_module(env, s_stdout_stream, *out_module);

      result = wasm_init_interpreter_thread(allocator, env, *out_module,
                                            out_thread, &s_thread_options);
    }
    wasm_free(allocator, data);
  }
  return result;
}

static void print_typed_values(WasmInterpreterTypedValue* values,
                               size_t num_values) {
  uint32_t i;
  for (i = 0; i < num_values; ++i) {
    print_typed_value(&values[i]);
    if (i != num_values - 1)
      printf(", ");
  }
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

  printf("called host " PRIstringslice "." PRIstringslice "(",
         WASM_PRINTF_STRING_SLICE_ARG(func->host.module_name),
         WASM_PRINTF_STRING_SLICE_ARG(func->host.field_name));
  print_typed_values(args, num_args);
  printf(") => (");
  print_typed_values(out_results, num_results);
  printf(")\n");
  return WASM_OK;
}

static WasmResult spectest_import_func(WasmInterpreterImport* import,
                                       WasmInterpreterFunc* func,
                                       WasmInterpreterFuncSignature* sig,
                                       void* user_data) {
  func->host.callback = default_host_callback;
  return WASM_OK;
}

static WasmResult spectest_import_table(WasmInterpreterImport* import,
                                        WasmInterpreterTable* table,
                                        void* user_data) {
  return WASM_ERROR;
}

static WasmResult spectest_import_memory(WasmInterpreterImport* import,
                                         WasmInterpreterMemory* memory,
                                         void* user_data) {
  return WASM_ERROR;
}

static WasmResult spectest_import_global(WasmInterpreterImport* import,
                                         WasmInterpreterGlobal* global,
                                         void* user_data) {
  return WASM_ERROR;
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
  result = read_module(allocator, module_filename, &env, &module, &thread);
  if (WASM_SUCCEEDED(result)) {
    result = run_start_function(&thread);
    if (s_run_all_exports)
      run_all_exports(allocator, module, &thread, RUN_VERBOSE);
  }
  wasm_destroy_interpreter_thread(allocator, &thread);
  wasm_destroy_interpreter_environment(allocator, &env);
  return result;
}

static WasmResult read_and_run_spec_json(WasmAllocator* allocator,
                                         const char* spec_json_filename) {
  WasmResult result = WASM_OK;
  WasmInterpreterEnvironment env;
  WasmInterpreterModule* module = NULL;
  WasmInterpreterThread thread;
  WasmStringSlice command_file;
  WasmStringSlice command_name;
  WasmInterpreterTypedValueVector result_values;
  uint32_t command_line_no = 0;
  WasmBool has_thread = WASM_FALSE;
  uint32_t passed = 0;
  uint32_t failed = 0;

  WASM_ZERO_MEMORY(thread);
  WASM_ZERO_MEMORY(command_file);
  WASM_ZERO_MEMORY(command_name);
  WASM_ZERO_MEMORY(result_values);

  init_environment(allocator, &env);

  void* data;
  size_t size;
  result = wasm_read_file(allocator, spec_json_filename, &data, &size);
  if (WASM_FAILED(result))
    return WASM_ERROR;

  /* an extremely simple JSON parser that only knows how to parse the expected
   * format from wast2wasm */
  enum {
    INITIAL,
    TOP_OBJECT,
    MODULES_ARRAY,
    END_MODULES_ARRAY,
    MODULE_OBJECT,
    END_MODULE_OBJECT,
    MODULE_FILENAME,
    COMMANDS_ARRAY,
    END_COMMANDS_ARRAY,
    COMMAND_OBJECT,
    END_COMMAND_OBJECT,
    COMMAND_TYPE,
    COMMAND_NAME,
    COMMAND_FILE,
    COMMAND_LINE,
    DONE,
  } state = INITIAL;

  enum {
    NONE,
    ACTION,
    ASSERT_RETURN,
    ASSERT_RETURN_NAN,
    ASSERT_TRAP,
  } command_type = NONE;

#define SKIP_WS()                                                            \
  while ((p < end) && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) \
  p++

#define MATCHES(c) ((p < end) && (*p == c) && (p++))

#define EXPECT(c)  \
  if (!MATCHES(c)) \
    goto fail;

#define STRLEN(s) (sizeof(s) - 1)

#define MATCHES_STR(s) \
  ((p + STRLEN(s) < end) && (strncmp(p, s, STRLEN(s)) == 0) && (p += STRLEN(s)))

#define EXPECT_STR(s)  \
  if (!MATCHES_STR(s)) \
    goto fail;

#define READ_UNTIL(c)        \
  while (p < end && *p != c) \
    p++;                     \
  if (p == end)              \
  goto fail

#define READ_STRING(slice)        \
  EXPECT('"');                    \
  slice.start = p;                \
  READ_UNTIL('"');                \
  slice.length = p - slice.start; \
  EXPECT('"')

#define MAYBE_CONTINUE(name) \
  SKIP_WS();                 \
  if (MATCHES(','))          \
    state = name;            \
  else                       \
  state = END_##name

  const char* start = data;
  const char* p = start;
  const char* end = p + size;
  while (p < end && state != DONE) {
    SKIP_WS();

    switch (state) {
      case INITIAL:
        EXPECT('{');
        state = TOP_OBJECT;
        break;

      case TOP_OBJECT:
        EXPECT_STR("\"modules\"");
        SKIP_WS();
        EXPECT(':');
        SKIP_WS();
        EXPECT('[');
        state = MODULES_ARRAY;
        break;

      case MODULES_ARRAY:
        if (MATCHES(']')) {
          /* should only match with an empty module list */
          SKIP_WS();
          EXPECT('}');
          state = DONE;
        } else {
          EXPECT('{');
          state = MODULE_OBJECT;
        }
        break;

      case END_MODULES_ARRAY:
        EXPECT(']');
        SKIP_WS();
        EXPECT('}');
        state = DONE;
        break;

      case MODULE_OBJECT:
        if (MATCHES_STR("\"filename\"")) {
          SKIP_WS();
          EXPECT(':');
          state = MODULE_FILENAME;
        } else {
          EXPECT_STR("\"commands\"");
          SKIP_WS();
          EXPECT(':');
          SKIP_WS();
          EXPECT('[');
          state = COMMANDS_ARRAY;
        }
        break;

      case END_MODULE_OBJECT:
        EXPECT('}');
        MAYBE_CONTINUE(MODULES_ARRAY);
        assert(has_thread);
        wasm_destroy_interpreter_thread(allocator, &thread);
        has_thread = WASM_FALSE;
        break;

      case MODULE_FILENAME: {
        WasmStringSlice module_filename;
        READ_STRING(module_filename);
        WasmStringSlice dirname = get_dirname(spec_json_filename);
        size_t path_len = dirname.length + 1 + module_filename.length + 1;
        char* path = alloca(path_len);
        if (dirname.length == 0) {
          wasm_snprintf(path, path_len, PRIstringslice,
                        WASM_PRINTF_STRING_SLICE_ARG(module_filename));
        } else {
          wasm_snprintf(path, path_len, PRIstringslice "/" PRIstringslice,
                        WASM_PRINTF_STRING_SLICE_ARG(dirname),
                        WASM_PRINTF_STRING_SLICE_ARG(module_filename));
        }

        result = read_module(allocator, path, &env, &module, &thread);
        if (WASM_FAILED(result))
          goto fail;

        has_thread = WASM_TRUE;

        result = run_start_function(&thread);
        if (WASM_FAILED(result))
          goto fail;

        MAYBE_CONTINUE(MODULE_OBJECT);
        break;
      }

      case COMMANDS_ARRAY:
        if (MATCHES(']')) {
          /* should only match with an empty command array */
          state = END_MODULE_OBJECT;
        } else {
          EXPECT('{');
          state = COMMAND_OBJECT;
        }
        break;

      case END_COMMANDS_ARRAY:
        EXPECT(']');
        MAYBE_CONTINUE(MODULE_OBJECT);
        break;

      case COMMAND_OBJECT:
        if (MATCHES_STR("\"type\"")) {
          state = COMMAND_TYPE;
        } else if (MATCHES_STR("\"name\"")) {
          state = COMMAND_NAME;
        } else if (MATCHES_STR("\"file\"")) {
          state = COMMAND_FILE;
        } else {
          EXPECT_STR("\"line\"");
          state = COMMAND_LINE;
        }
        SKIP_WS();
        EXPECT(':');
        break;

#define FAILED(msg)                                                          \
  fprintf(stderr, "*** " PRIstringslice ":%d: " PRIstringslice " " msg "\n", \
          WASM_PRINTF_STRING_SLICE_ARG(command_file), command_line_no,       \
          WASM_PRINTF_STRING_SLICE_ARG(command_name));

      case END_COMMAND_OBJECT: {
        WasmInterpreterResult iresult;
        EXPECT('}');
        RunVerbosity verbose = command_type == ACTION ? RUN_VERBOSE : RUN_QUIET;
        result = run_export_by_name(allocator, &thread, &command_name, &iresult,
                                    &result_values, verbose);
        if (WASM_FAILED(result)) {
          FAILED("unknown export");
          failed++;
          goto fail;
        }

        switch (command_type) {
          case ACTION:
            if (iresult != WASM_INTERPRETER_OK) {
              FAILED("trapped");
              failed++;
            }
            break;

          case ASSERT_RETURN:
          case ASSERT_RETURN_NAN:
            if (iresult != WASM_INTERPRETER_OK) {
              FAILED("trapped");
              failed++;
            } else if (result_values.size != 1) {
              FAILED("result arity mismatch");
              failed++;
            } else if (result_values.data[0].type != WASM_TYPE_I32) {
              FAILED("type mismatch");
              failed++;
            } else if (result_values.data[0].value.i32 != 1) {
              FAILED("didn't return 1");
              failed++;
            } else {
              passed++;
            }
            break;

          case ASSERT_TRAP:
            if (iresult == WASM_INTERPRETER_OK) {
              FAILED("didn't trap");
              failed++;
            } else {
              passed++;
            }
            break;

          default:
            assert(0);
            goto fail;
        }
        MAYBE_CONTINUE(COMMANDS_ARRAY);
        break;
      }

      case COMMAND_TYPE: {
        if (MATCHES_STR("\"action\"")) {
          command_type = ACTION;
        } else if (MATCHES_STR("\"assert_return\"")) {
          command_type = ASSERT_RETURN;
        } else if (MATCHES_STR("\"assert_return_nan\"")) {
          command_type = ASSERT_RETURN_NAN;
        } else {
          EXPECT_STR("\"assert_trap\"");
          command_type = ASSERT_TRAP;
        }
        MAYBE_CONTINUE(COMMAND_OBJECT);
        break;
      }

      case COMMAND_NAME: {
        READ_STRING(command_name);
        MAYBE_CONTINUE(COMMAND_OBJECT);
        break;
      }

      case COMMAND_FILE: {
        READ_STRING(command_file);
        MAYBE_CONTINUE(COMMAND_OBJECT);
        break;
      }

      case COMMAND_LINE: {
        command_line_no = 0;
        while (p < end && *p >= '0' && *p <= '9') {
          uint32_t new_line_no = command_line_no * 10 + (*p - '0');
          if (new_line_no < command_line_no)
            goto fail;
          command_line_no = new_line_no;
          p++;
        }
        MAYBE_CONTINUE(COMMAND_OBJECT);
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  uint32_t total = passed + failed;
  printf("%d/%d tests passed.\n", passed, total);
  result = passed == total ? WASM_OK : WASM_ERROR;

  goto done;

fail:
  fprintf(stderr, "error parsing spec json file\n");
  fprintf(stderr, "got this far: %" PRIzd ":> %.*s...\n", p - start, 20, p);
  result = WASM_ERROR;

done:
  if (has_thread)
    wasm_destroy_interpreter_thread(allocator, &thread);
  wasm_destroy_interpreter_environment(allocator, &env);
  wasm_destroy_interpreter_typed_value_vector(allocator, &result_values);
  wasm_free(allocator, data);
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
