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

#define INSTRUCTION_QUANTUM 1000

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static int s_verbose;
static const char* s_infile;
static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;
static WasmInterpreterThreadOptions s_thread_options =
    WASM_INTERPRETER_THREAD_OPTIONS_DEFAULT;
static int s_trace;
static int s_spec;
static int s_run_all_exports;
static int s_use_libc_allocator;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

typedef enum WasmRunVerbosity {
  RUN_QUIET = 0,
  RUN_VERBOSE = 1,
} WasmRunVerbosity;

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
      break;

    case FLAG_HELP:
      wasm_print_help(parser);
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
      s_trace = 1;
      break;

    case FLAG_SPEC:
      s_spec = 1;
      break;

    case FLAG_RUN_ALL_EXPORTS:
      s_run_all_exports = 1;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = 1;
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
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (s_spec && s_run_all_exports)
    WASM_FATAL("--spec and --run-all-exports are incompatible.\n");

  if (!s_infile) {
    wasm_print_help(&parser);
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

static void read_file(const char* filename,
                      const void** out_data,
                      size_t* out_size) {
  FILE* infile = fopen(filename, "rb");
  if (!infile)
    WASM_FATAL("unable to read %s\n", filename);

  if (fseek(infile, 0, SEEK_END) < 0)
    WASM_FATAL("fseek to end failed.\n");

  long size = ftell(infile);
  if (size < 0)
    WASM_FATAL("ftell failed.\n");

  if (fseek(infile, 0, SEEK_SET) < 0)
    WASM_FATAL("fseek to beginning failed.\n");

  void* data = malloc(size);
  if (fread(data, size, 1, infile) != 1)
    WASM_FATAL("fread failed.\n");

  *out_data = data;
  *out_size = size;
  fclose(infile);
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

static WasmInterpreterTypedValue default_import_callback(
    WasmInterpreterModule* module,
    WasmInterpreterImport* import,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    void* user_data) {
  printf("called import %.*s.%.*s(", (int)import->module_name.length,
         import->module_name.start, (int)import->func_name.length,
         import->func_name.start);
  uint32_t i;
  for (i = 0; i < num_args; ++i) {
    print_typed_value(&args[i]);
    if (i != num_args - 1)
      printf(", ");
  }

  assert(import->sig_index < module->sigs.size);
  WasmInterpreterFuncSignature* sig = &module->sigs.data[import->sig_index];

  WasmInterpreterTypedValue result;
  WASM_ZERO_MEMORY(result);
  result.type = sig->result_type;

  if (sig->result_type != WASM_TYPE_VOID) {
    printf(") => ");
    print_typed_value(&result);
    printf("\n");
  } else {
    printf(")\n");
  }
  return result;
}

static void set_all_import_callbacks_to_default(WasmInterpreterModule* module) {
  uint32_t i;
  for (i = 0; i < module->imports.size; ++i) {
    WasmInterpreterImport* import = &module->imports.data[i];
    import->callback = default_import_callback;
    import->user_data = NULL;
  }
}

static WasmInterpreterResult run_function(WasmInterpreterModule* module,
                                          WasmInterpreterThread* thread,
                                          uint32_t offset) {
  thread->pc = offset;
  WasmInterpreterResult iresult = WASM_INTERPRETER_OK;
  uint32_t quantum = s_trace ? 1 : INSTRUCTION_QUANTUM;
  while (iresult == WASM_INTERPRETER_OK) {
    if (s_trace)
      wasm_trace_pc(module, thread);
    iresult = wasm_run_interpreter(module, thread, quantum);
  }
  if (s_trace && iresult != WASM_INTERPRETER_RETURNED)
    printf("!!! trapped: %s\n", s_trap_strings[iresult]);
  return iresult;
}

static WasmResult run_start_function(WasmInterpreterModule* module,
                                     WasmInterpreterThread* thread) {
  WasmResult result = WASM_OK;
  if (module->start_func_offset != WASM_INVALID_OFFSET) {
    if (s_trace)
      printf(">>> running start function:\n");
    WasmInterpreterResult iresult =
        run_function(module, thread, module->start_func_offset);
    if (iresult != WASM_INTERPRETER_RETURNED) {
      /* trap */
      fprintf(stderr, "error: %s\n", s_trap_strings[iresult]);
      result = WASM_ERROR;
    }
  }
  return result;
}

static WasmInterpreterResult run_export(
    WasmInterpreterModule* module,
    WasmInterpreterThread* thread,
    WasmInterpreterExport* export,
    WasmInterpreterTypedValue* out_return_value,
    WasmRunVerbosity verbose) {
  /* pass all zeroes to the function */
  assert(export->sig_index < module->sigs.size);
  WasmInterpreterFuncSignature* sig = &module->sigs.data[export->sig_index];
  assert(sig->param_types.size < thread->value_stack.size);
  uint32_t num_params = sig->param_types.size;
  thread->value_stack_top = num_params;
  memset(thread->value_stack.data, 0,
         num_params * sizeof(WasmInterpreterValue));

  if (s_trace)
    printf(">>> running export \"%.*s\":\n", (int)export->name.length,
           export->name.start);
  WasmInterpreterResult result =
      run_function(module, thread, export->func_offset);

  if (verbose) {
    printf("%.*s(", (int)export->name.length, export->name.start);
    uint32_t i;
    for (i = 0; i < num_params; ++i) {
      printf("0");
      if (i != num_params - 1)
        printf(", ");
    }
    if (sig->result_type != WASM_TYPE_VOID)
      printf(") => ");
    else
      printf(") ");
  }

  if (result == WASM_INTERPRETER_RETURNED) {
    out_return_value->type = sig->result_type;

    if (sig->result_type != WASM_TYPE_VOID) {
      assert(thread->value_stack_top == 1);
      out_return_value->value = thread->value_stack.data[0];

      if (verbose)
        print_typed_value(out_return_value);
    } else {
      assert(thread->value_stack_top == 0);
    }

    if (verbose)
      printf("\n");
  } else {
    /* trap */
    if (verbose)
      printf("error: %s\n", s_trap_strings[result]);
  }
  thread->value_stack_top = 0;
  thread->call_stack_top = 0;
  return result;
}

static WasmResult run_export_by_name(
    WasmInterpreterModule* module,
    WasmInterpreterThread* thread,
    WasmStringSlice* name,
    WasmInterpreterResult* out_iresult,
    WasmInterpreterTypedValue* out_return_value,
    WasmRunVerbosity verbose) {
  uint32_t i;
  for (i = 0; i < module->exports.size; ++i) {
    WasmInterpreterExport* export = &module->exports.data[i];
    if (wasm_string_slices_are_equal(name, &export->name)) {
      *out_iresult =
          run_export(module, thread, export, out_return_value, verbose);
      return WASM_OK;
    }
  }
  return WASM_ERROR;
}

static void run_all_exports(WasmInterpreterModule* module,
                            WasmInterpreterThread* thread,
                            WasmRunVerbosity verbose) {
  uint32_t i;
  for (i = 0; i < module->exports.size; ++i) {
    WasmInterpreterExport* export = &module->exports.data[i];
    WasmInterpreterTypedValue return_value;
    run_export(module, thread, export, &return_value, verbose);
  }
}

static WasmResult read_module(WasmAllocator* allocator,
                              const char* module_filename,
                              WasmInterpreterModule* out_module,
                              WasmInterpreterThread* out_thread) {
  const void* data;
  size_t size;
  read_file(module_filename, &data, &size);

  WasmAllocator* memory_allocator = &g_wasm_libc_allocator;
  WASM_ZERO_MEMORY(*out_module);
  WasmResult result =
      wasm_read_binary_interpreter(allocator, memory_allocator, data, size,
                                   &s_read_binary_options, out_module);

  if (result == WASM_OK) {
    set_all_import_callbacks_to_default(out_module);
    WASM_ZERO_MEMORY(*out_thread);
    result = wasm_init_interpreter_thread(allocator, out_module, out_thread,
                                          &s_thread_options);
  }
  free((void*)data);
  return result;
}

static void destroy_module_and_thread(WasmAllocator* allocator,
                                      WasmInterpreterModule* module,
                                      WasmInterpreterThread* thread) {
  wasm_destroy_interpreter_thread(allocator, thread);
  wasm_destroy_interpreter_module(allocator, module);
}

static WasmResult read_and_run_module(WasmAllocator* allocator,
                                      const char* module_filename) {
  WasmResult result;
  WasmInterpreterModule module;
  WasmInterpreterThread thread;
  result = read_module(allocator, module_filename, &module, &thread);
  if (result == WASM_OK)
    result = run_start_function(&module, &thread);

  if (result == WASM_OK && s_run_all_exports)
    run_all_exports(&module, &thread, RUN_VERBOSE);
  destroy_module_and_thread(allocator, &module, &thread);
  return result;
}

static WasmResult read_and_run_spec_json(WasmAllocator* allocator,
                                         const char* spec_json_filename) {
  WasmResult result = WASM_OK;
  WasmInterpreterModule module;
  WasmInterpreterThread thread;
  WasmStringSlice command_file;
  WasmStringSlice command_name;
  uint32_t command_line_no;
  int has_module = 0;
  uint32_t passed = 0;
  uint32_t failed = 0;

  const void* data;
  size_t size;
  read_file(spec_json_filename, &data, &size);

  /* an extremely simple JSON parser that only knows how to parse the expected
   * format from sexpr-wasm */
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
    INVOKE,
    ASSERT_RETURN,
    ASSERT_RETURN_NAN,
    ASSERT_TRAP,
  } command_type = NONE;

#define SKIP_WS()                                                        \
  while (p < end && *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') \
  p++

#define MATCHES(c) \
  ((p < end) && (*p == c) && (p++))

#define EXPECT(c) \
  if (!MATCHES(c))    \
    goto fail;    \

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
        EXPECT('{');
        state = MODULE_OBJECT;
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
        destroy_module_and_thread(allocator, &module, &thread);
        has_module = 0;
        break;

      case MODULE_FILENAME: {
        WasmStringSlice module_filename;
        READ_STRING(module_filename);
        WasmStringSlice dirname = get_dirname(spec_json_filename);
        size_t path_len = dirname.length + 1 + module_filename.length + 1;
        char* path = alloca(path_len);
        if (dirname.length == 0) {
          wasm_snprintf(path, path_len, "%.*s", (int)module_filename.length,
                        module_filename.start);
        } else {
          wasm_snprintf(path, path_len, "%.*s/%.*s", (int)dirname.length,
                        dirname.start, (int)module_filename.length,
                        module_filename.start);
        }

        result = read_module(allocator, path, &module, &thread);
        if (result != WASM_OK)
          goto fail;

        has_module = 1;

        result = run_start_function(&module, &thread);
        if (result != WASM_OK)
          goto fail;

        MAYBE_CONTINUE(MODULE_OBJECT);
        break;
      }

      case COMMANDS_ARRAY:
        EXPECT('{');
        state = COMMAND_OBJECT;
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

      case END_COMMAND_OBJECT: {
        WasmInterpreterResult iresult;
        WasmInterpreterTypedValue return_value;
        EXPECT('}');
        result = run_export_by_name(&module, &thread, &command_name, &iresult,
                                    &return_value, RUN_QUIET);
        if (result != WASM_OK)
          goto fail;

        switch (command_type) {
          case INVOKE:
            if (iresult == WASM_INTERPRETER_RETURNED)
              passed++;
            else
              failed++;
            break;

          case ASSERT_RETURN:
          case ASSERT_RETURN_NAN:
            if (iresult == WASM_INTERPRETER_RETURNED &&
                return_value.type == WASM_TYPE_I32 &&
                return_value.value.i32 == 1) {
              passed++;
            } else {
              failed++;
            }
            break;

          case ASSERT_TRAP:
            if (iresult == WASM_INTERPRETER_RETURNED)
              failed++;
            else
              passed++;
            break;

          default:
            assert(0);
            break;
        }
        MAYBE_CONTINUE(COMMANDS_ARRAY);
        break;
      }

      case COMMAND_TYPE: {
        if (MATCHES_STR("\"invoke\"")) {
          command_type = INVOKE;
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

  printf("%d/%d tests passed.\n", passed, passed + failed);

  goto done;

fail:
  fprintf(stderr, "error parsing spec json file\n");
  fprintf(stderr, "got this far: %" PRIzd ":> %.*s...\n", p - start, 20, p);

done:
  if (has_module)
    destroy_module_and_thread(allocator, &module, &thread);
  free((void*)data);
  return result;
}

int main(int argc, char** argv) {
  WasmStackAllocator stack_allocator;
  WasmAllocator* allocator;

  parse_options(argc, argv);

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

  if (!s_use_libc_allocator)
    wasm_destroy_stack_allocator(&stack_allocator);
  return result;
}
