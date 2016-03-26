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

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;
static int s_use_libc_allocator;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_OUTPUT,
  FLAG_USE_LIBC_ALLOCATOR,
  NUM_FLAGS
};

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the generated wast file"},
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

    case FLAG_OUTPUT:
      s_outfile = argument;
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

  if (!s_infile) {
    wasm_print_help(&parser);
    WASM_FATAL("No filename given.\n");
  }
}

static void read_file(const char* filename,
                      const void** out_data,
                      size_t* out_size) {
  FILE* infile = fopen(s_infile, "rb");
  if (!infile)
    WASM_FATAL("unable to read %s\n", s_infile);

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
      printf("f32:%f", value);
      break;
    }

    case WASM_TYPE_F64: {
      double value;
      memcpy(&value, &tv->value.f64_bits, sizeof(double));
      printf("f64:%f", value);
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

int main(int argc, char** argv) {
  WasmStackAllocator stack_allocator;
  WasmAllocator* allocator;
  WasmAllocator* memory_allocator;

  parse_options(argc, argv);

  if (s_use_libc_allocator) {
    allocator = &g_wasm_libc_allocator;
  } else {
    wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
    allocator = &stack_allocator.allocator;
  }
  memory_allocator = &g_wasm_libc_allocator;

  const void* data;
  size_t size;
  read_file(s_infile, &data, &size);

  WasmInterpreterModule module;
  WasmResult result = wasm_read_binary_interpreter(
      allocator, memory_allocator, data, size, &s_read_binary_options, &module);

  if (result == WASM_OK) {
    if (module.start_func_offset != WASM_INVALID_OFFSET) {
      set_all_import_callbacks_to_default(&module);

      WasmInterpreterThreadOptions thread_options;
      WASM_ZERO_MEMORY(thread_options);
      thread_options.value_stack_size = 1 * 1024 * 1024;
      thread_options.call_stack_size = 64 * 1024;
      thread_options.pc = module.start_func_offset;

      WasmInterpreterThread thread;
      result = wasm_init_interpreter_thread(allocator, &module, &thread,
                                            &thread_options);
      if (result == WASM_OK) {
        int instruction_quantum = 1000;

        WasmInterpreterResult iresult = WASM_INTERPRETER_OK;
        while (iresult == WASM_INTERPRETER_OK) {
          iresult = wasm_run_interpreter(&module, &thread, instruction_quantum);
        }

        if (iresult != WASM_INTERPRETER_RETURNED) {
          /* trap */
          fprintf(stderr, "error: %s\n", s_trap_strings[iresult]);
          result = WASM_ERROR;
        }
        wasm_destroy_interpreter_thread(allocator, &thread);
      }
    } else {
      fprintf(stderr, "no start function defined.\n");
      result = WASM_ERROR;
    }
  }

  if (!s_use_libc_allocator)
    wasm_destroy_stack_allocator(&stack_allocator);
  free((void*)data);
  return result;
}
