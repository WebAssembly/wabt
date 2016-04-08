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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-array.h"
#include "wasm-binary-reader.h"
#include "wasm-binary-reader-interpreter.h"
#include "wasm-interpreter.h"
#include "wasm-option-parser.h"
#include "wasm-stack-allocator.h"

#include "squirrel.h"
#include "sqstdblob.h"
#include "sqstdio.h"

#define INSTRUCTION_QUANTUM 1000
#define INITIAL_SQ_STACK_SIZE 1024

#define WASM_ALLOCATOR_NAME "wasm_allocator"

static int s_verbose;
static const char* s_infile;
static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;
static WasmInterpreterThreadOptions s_thread_options =
    WASM_INTERPRETER_THREAD_OPTIONS_DEFAULT;
static int s_trace;
static int s_use_libc_allocator;

#define V(name, str) str,
static const char* s_trap_strings[] = {FOREACH_INTERPRETER_RESULT(V)};
#undef V

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES);

typedef struct SquirrelModuleContext {
  WasmAllocator* allocator;
  HSQUIRRELVM squirrel_vm;
  HSQOBJECT import_array;
  WasmInterpreterModule module;
  WasmInterpreterThread thread;
} SquirrelModuleContext;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_VALUE_STACK_SIZE,
  FLAG_CALL_STACK_SIZE,
  FLAG_TRACE,
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

static void read_file(const char* filename, void** out_data, size_t* out_size) {
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

static WasmInterpreterResult run_function(WasmInterpreterModule* module,
                                          WasmInterpreterThread* thread,
                                          uint32_t offset) {
  thread->pc = offset;
  WasmInterpreterResult iresult = WASM_INTERPRETER_OK;
  uint32_t quantum = s_trace ? 1 : INSTRUCTION_QUANTUM;
  uint32_t call_stack_return_top = thread->call_stack_top;
  while (iresult == WASM_INTERPRETER_OK) {
    if (s_trace)
      wasm_trace_pc(module, thread);
    iresult =
        wasm_run_interpreter(module, thread, quantum, call_stack_return_top);
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

static WasmResult read_module(WasmAllocator* allocator,
                              const void* data,
                              size_t size,
                              WasmInterpreterModule* out_module,
                              WasmInterpreterThread* out_thread) {
  WasmAllocator* memory_allocator = &g_wasm_libc_allocator;
  WASM_ZERO_MEMORY(*out_module);
  return wasm_read_binary_interpreter(allocator, memory_allocator, data, size,
                                      &s_read_binary_options, out_module);
}

static WasmResult init_thread(WasmAllocator* allocator,
                              WasmInterpreterModule* module,
                              WasmInterpreterThread* thread) {
  WASM_ZERO_MEMORY(*thread);
  return wasm_init_interpreter_thread(allocator, module, thread,
                                      &s_thread_options);
}

static void squirrel_print(HSQUIRRELVM v, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
}

static void squirrel_error(HSQUIRRELVM v, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

void squirrel_compiler_error(HSQUIRRELVM v,
                             const char* desc,
                             const char* source,
                             SQInteger line,
                             SQInteger column) {
  fprintf(stderr, "%s:" _PRINT_INT_FMT
                  ":"_PRINT_INT_FMT
                  ": %s.\n",
          source, line, column, desc);
}

static SQInteger squirrel_errorhandler(HSQUIRRELVM v) {
  const char* error_msg = "unknown";
  if (sq_gettop(v) >= 1)
    sq_getstring(v, 2, &error_msg);
  squirrel_error(v, "error: %s\n", error_msg);
  squirrel_error(v, "callstack:\n");
  SQStackInfos stack_info;
  SQInteger depth;
  for (depth = 1; SQ_SUCCEEDED(sq_stackinfos(v, depth, &stack_info)); depth++) {
    const char* source = stack_info.source ? stack_info.source : "unknown";
    const char* funcname =
        stack_info.funcname ? stack_info.funcname : "unknown";
    SQInteger line = stack_info.line;
    squirrel_error(v, "  #%d: %s:%d: %s()\n", depth, source, line, funcname);
  }
  return 0;
}

static WasmResult run_squirrel_file(HSQUIRRELVM v, const char* filename) {
  void* data;
  size_t size;
  read_file(filename, &data, &size);
  WasmResult result = WASM_ERROR;
  const SQBool raise_error = SQTrue;
  if (SQ_SUCCEEDED(sq_compilebuffer(v, data, size, filename, raise_error))) {
    sq_pushroottable(v);
    const SQBool with_retval = SQFalse;
    if (SQ_SUCCEEDED(sq_call(v, 1, with_retval, raise_error))) {
      /* do some other stuff... */
      result = WASM_OK;
    }
  }
  free(data);
  return result;
}

static SQInteger squirrel_throwerrorf(HSQUIRRELVM v, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  return sq_throwerror(v, buffer);
}

#define CHECK_SQ_RESULT(cond) \
  do {                        \
    if (SQ_FAILED(cond))      \
      return -1;              \
  } while (0)

static SQRESULT squirrel_get_allocator(HSQUIRRELVM v,
                                       WasmAllocator** out_allocator) {
  SQUserPointer allocator_up;
  sq_pushregistrytable(v);
  sq_pushstring(v, WASM_ALLOCATOR_NAME, -1);
  CHECK_SQ_RESULT(sq_get(v, -2));
  CHECK_SQ_RESULT(sq_getuserpointer(v, -1, &allocator_up));
  *out_allocator = allocator_up;
  sq_pop(v, 2);
  return SQ_OK;
}

static SQInteger squirrel_wasm_release_module(SQUserPointer data,
                                              SQInteger size) {
  SquirrelModuleContext* smc = data;
  wasm_destroy_interpreter_module(smc->allocator, &smc->module);
  wasm_destroy_interpreter_thread(smc->allocator, &smc->thread);
  return SQ_OK;
}

static uint32_t bitcast_f32_to_u32(float x) {
  uint32_t result;
  memcpy(&result, &x, sizeof(result));
  return result;
}

static float bitcast_u32_to_f32(uint32_t x) {
  float result;
  memcpy(&result, &x, sizeof(result));
  return result;
}

static uint64_t bitcast_f64_to_u64(double x) {
  uint64_t result;
  memcpy(&result, &x, sizeof(result));
  return result;
}

static double bitcast_u64_to_f64(uint64_t x) {
  float result;
  memcpy(&result, &x, sizeof(result));
  return result;
}

WASM_STATIC_ASSERT(sizeof(SQInteger) == sizeof(uint32_t) ||
                   sizeof(SQInteger) == sizeof(uint64_t));
WASM_STATIC_ASSERT(sizeof(SQFloat) == sizeof(float) ||
                   sizeof(SQFloat) == sizeof(double));

static void squirrel_push_typedvalue(HSQUIRRELVM v,
                                     WasmInterpreterTypedValue* tv) {
  switch (tv->type) {
    case WASM_TYPE_I32: {
      if (sizeof(SQInteger) == sizeof(uint32_t)) {
        sq_pushinteger(v, tv->value.i32);
      } else {
        /* sign extend */
        sq_pushinteger(v, (int64_t)(int32_t)tv->value.i32);
      }
      break;
    }

    case WASM_TYPE_I64:
      sq_pushinteger(v, tv->value.i64);
      break;

    case WASM_TYPE_F32:
      sq_pushfloat(v, bitcast_u32_to_f32(tv->value.f32_bits));
      break;

    case WASM_TYPE_F64:
      sq_pushfloat(v, bitcast_u64_to_f64(tv->value.f64_bits));
      break;
  }
}

static SQRESULT squirrel_get_typedvalue(HSQUIRRELVM v,
                                        SQInteger idx,
                                        WasmInterpreterTypedValue* out_tv) {
  SQObjectType type = sq_gettype(v, idx);
  switch (type) {
    case OT_INTEGER: {
      SQInteger value;
      sq_getinteger(v, idx, &value);

      if (sizeof(SQInteger) == sizeof(uint32_t)) {
        out_tv->type = WASM_TYPE_I32;
        out_tv->value.i32 = value;
      } else if (sizeof(SQInteger) == sizeof(uint64_t)) {
        out_tv->type = WASM_TYPE_I64;
        out_tv->value.i64 = value;
      }
      return SQ_OK;
    }

    case OT_FLOAT: {
      SQFloat value;
      sq_getfloat(v, idx, &value);

      if (sizeof(SQFloat) == sizeof(float)) {
        out_tv->type = WASM_TYPE_F32;
        out_tv->value.f32_bits = bitcast_f32_to_u32(value);
      } else if (sizeof(SQFloat) == sizeof(double)) {
        out_tv->type = WASM_TYPE_F64;
        out_tv->value.f64_bits = bitcast_f64_to_u64(value);
      }
      return SQ_OK;
    }

    case OT_NULL: {
      out_tv->type = WASM_TYPE_VOID;
      return SQ_OK;
    }

    default:
      return sq_throwerror(v, "unexpected type.");
  }
}

static WasmResult convert_typed_value(WasmInterpreterTypedValue* from_tv,
                                      WasmType to_type,
                                      WasmInterpreterTypedValue* to_tv) {
  if (from_tv->type == to_type) {
    *to_tv = *from_tv;
    return WASM_OK;
  }

  to_tv->type = to_type;

  /* TODO(binji): other safe conversions? */
  switch (to_type) {
    case WASM_TYPE_VOID:
      return WASM_ERROR;

    case WASM_TYPE_I32:
      if (from_tv->type == WASM_TYPE_I64) {
        /* squirrel's integers are always signed, so assume that it is safe to
         * to truncate if the high 32-bits are zero- or sign-extended. */
        uint32_t high_32bits = from_tv->value.i64 >> 32;
        if (high_32bits == 0 || high_32bits == UINT32_MAX) {
          to_tv->value.i32 = (uint32_t)from_tv->value.i64;
          return WASM_OK;
        }
      }
      break;

    case WASM_TYPE_I64:
      if (from_tv->type == WASM_TYPE_I32) {
        /* sign-extend to 64-bits. */
        to_tv->value.i64 = (uint64_t)(int64_t)(int32_t)from_tv->value.i32;
        return WASM_OK;
      }
      break;

    case WASM_TYPE_F32:
      if (from_tv->type == WASM_TYPE_F64) {
        to_tv->value.f32_bits = bitcast_f32_to_u32(
            (float)bitcast_u64_to_f64(from_tv->value.f64_bits));
        return WASM_OK;
      }
      break;

    case WASM_TYPE_F64:
      if (from_tv->type == WASM_TYPE_F32) {
        to_tv->value.f64_bits = bitcast_f64_to_u64(
            (double)bitcast_u32_to_f32(from_tv->value.f32_bits));
        return WASM_OK;
      }
      break;
  }

  return WASM_ERROR;
}

static WasmInterpreterTypedValue squirrel_wasm_import_callback(
    WasmInterpreterModule* module,
    WasmInterpreterImport* import,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    void* user_data) {
  SquirrelModuleContext* smc = user_data;
  HSQUIRRELVM v = smc->squirrel_vm;

  /* TODO(binji): kinda nasty, should pass the import index instead */
  int import_index = import - module->imports.data;
  sq_pushobject(v, smc->import_array);
  sq_pushinteger(v, import_index);
  sq_get(v, -2);
  sq_remove(v, -2);

  sq_getclosureroot(v, -1);

  uint32_t i;
  for (i = 0; i < num_args; ++i) {
    WasmInterpreterTypedValue* arg = &args[i];
    squirrel_push_typedvalue(v, arg);
  }

  /* TODO(binji): need way to trap from import callback */
  WasmInterpreterTypedValue top_tv;
  top_tv.type = WASM_TYPE_VOID;

  const SQBool retval = SQTrue;
  const SQBool raise_error = SQTrue;
  if (SQ_SUCCEEDED(sq_call(v, num_args + 1, retval, raise_error))) {
    squirrel_get_typedvalue(v, -1, &top_tv);
    sq_pop(v, 1); /* pop the return value */
  }
  sq_pop(v, 1); /* pop the closure */

  assert(import->sig_index < module->sigs.size);
  WasmInterpreterFuncSignature* sig = &module->sigs.data[import->sig_index];
  WasmInterpreterTypedValue result;
  if (convert_typed_value(&top_tv, sig->result_type, &result) != WASM_OK) {
    fprintf(stderr,
            "error: cannot convert return value of type %s to type %s.\n",
            s_type_names[top_tv.type], s_type_names[sig->result_type]);
  }
  return result;
}

static SQRESULT squirrel_wasm_export_callback(HSQUIRRELVM v) {
  const SQUnsignedInteger num_free_vars = 2;
  SQUnsignedInteger num_params = sq_gettop(v) - num_free_vars;

  SQUserPointer smc_up;
  CHECK_SQ_RESULT(sq_getuserdata(v, num_params + 1, &smc_up, NULL));
  SquirrelModuleContext* smc = smc_up;

  SQInteger export_index;
  CHECK_SQ_RESULT(sq_getinteger(v, num_params + 2, &export_index));
  assert((size_t)export_index < smc->module.exports.size);
  WasmInterpreterExport* export = &smc->module.exports.data[export_index];

  WasmInterpreterFuncSignature* sig = &smc->module.sigs.data[export->sig_index];
  /* don't count the implicit environment param */
  assert(sig->param_types.size == num_params - 1);

  WasmInterpreterResult result;
  size_t i;
  for (i = 0; i < sig->param_types.size; ++i) {
    WasmType param_type = sig->param_types.data[i];
    WasmInterpreterTypedValue arg;
    /* + 1 to skip first arg (environment), and + 1 because stack is 1-based */
    CHECK_SQ_RESULT(squirrel_get_typedvalue(v, i + 1 + 1, &arg));
    WasmInterpreterTypedValue converted_arg;
    if (convert_typed_value(&arg, param_type, &converted_arg) != WASM_OK)
      return squirrel_throwerrorf(v, "unable to convert arg %d.", i);
    result = wasm_push_thread_value(&smc->thread, converted_arg.value);
    if (result != WASM_INTERPRETER_OK)
      return sq_throwerror(v, s_trap_strings[result]);
  }

  result = run_function(&smc->module, &smc->thread, export->func_offset);
  if (result != WASM_INTERPRETER_RETURNED) {
    return squirrel_throwerrorf(v, PRIstringslice " trapped.",
                                WASM_PRINTF_STRING_SLICE_ARG(export->name));
  }

  if (sig->result_type != WASM_TYPE_VOID) {
    WasmInterpreterTypedValue return_tv;
    return_tv.type = sig->result_type;
    return_tv.value =
        smc->thread.value_stack.data[smc->thread.value_stack_top - 1];
    squirrel_push_typedvalue(v, &return_tv);
    return 1;
  } else {
    return 0;
  }
}

static SQInteger squirrel_wasm_instantiate_module(HSQUIRRELVM v) {
  const SQInteger blob_index = 2;
  const SQInteger ffi_index = 3;

  WasmAllocator* allocator;
  CHECK_SQ_RESULT(squirrel_get_allocator(v, &allocator));

  SQUserPointer blob_data;
  CHECK_SQ_RESULT(sqstd_getblob(v, blob_index, &blob_data));
  SQInteger blob_size = sqstd_getblobsize(v, blob_index);

  SQInteger mtp_size = sizeof(SquirrelModuleContext);
  SquirrelModuleContext* smc =
      (SquirrelModuleContext*)sq_newuserdata(v, mtp_size);
  smc->allocator = allocator;
  smc->squirrel_vm = v;

  WasmResult result =
      read_module(allocator, blob_data, blob_size, &smc->module, &smc->thread);
  if (result == WASM_OK) {
    /* STACK: ... module */
    sq_setreleasehook(v, -1, squirrel_wasm_release_module);
    sq_newtable(v); /* the delegate table */

    /* search through the imports trying to match them to the FFI table */
    if (smc->module.imports.size > 0) {
      sq_newarray(v, 0); /* import array */

      size_t i;
      for (i = 0; i < smc->module.imports.size; ++i) {
        /* STACK: ... module delegate import_array */
        WasmInterpreterImport* import = &smc->module.imports.data[i];
        WasmStringSlice* module_name = &import->module_name;
        sq_pushstring(v, module_name->start, module_name->length);
        if (SQ_FAILED(sq_get(v, ffi_index))) {
          return squirrel_throwerrorf(
              v, "FFI module named \"" PRIstringslice "\" not found.",
              WASM_PRINTF_STRING_SLICE_ARG(*module_name));
        }

        /* STACK: ... module delegate import_array module_ffi */
        WasmStringSlice* func_name = &import->func_name;
        sq_pushstring(v, func_name->start, func_name->length);
        if (SQ_FAILED(sq_get(v, -2))) {
          return squirrel_throwerrorf(
              v, "FFI func named \"" PRIstringslice
                 "\" not found in module \"" PRIstringslice "\".",
              WASM_PRINTF_STRING_SLICE_ARG(*func_name),
              WASM_PRINTF_STRING_SLICE_ARG(*module_name));
        }

        /* STACK: ... module delegate import_array module_ffi import_closure */
        sq_remove(v, -2);

        /* STACK: ... module delegate import_array import_closure */
        SQObjectType import_type = sq_gettype(v, -1);
        if (import_type != OT_CLOSURE) {
          return squirrel_throwerrorf(
              v, "FFI entry \"" PRIstringslice "." PRIstringslice
                 "\" is not a closure.",
              WASM_PRINTF_STRING_SLICE_ARG(*module_name),
              WASM_PRINTF_STRING_SLICE_ARG(*func_name));
        }

        SQUnsignedInteger num_closure_params;
        SQUnsignedInteger num_closure_free_vars;
        CHECK_SQ_RESULT(sq_getclosureinfo(v, -1, &num_closure_params,
                                          &num_closure_free_vars));

        /* don't count the implicit environment param */
        num_closure_params--;

        WasmInterpreterFuncSignature* sig =
            &smc->module.sigs.data[import->sig_index];
        if (sig->param_types.size != num_closure_params) {
          return squirrel_throwerrorf(
              v, "FFI entry \"" PRIstringslice "." PRIstringslice
                 "\" expects parameter count of %d, got %d.",
              WASM_PRINTF_STRING_SLICE_ARG(*module_name),
              WASM_PRINTF_STRING_SLICE_ARG(*func_name), sig->param_types.size,
              num_closure_params);
        }

        import->callback = squirrel_wasm_import_callback;
        import->user_data = smc;

        /* STACK: ... module delegate import_array bound_closure */
        sq_arrayappend(v, -2);
      }

      /* STACK: ... module delegate import_array */
      sq_pushstring(v, "imports", -1);
      sq_push(v, -2); /* the import array */
      CHECK_SQ_RESULT(sq_newslot(v, -4, SQFalse));
      /* store the module import array for use in the import callback. We don't
       * need to addref because the module owns it. */
      CHECK_SQ_RESULT(sq_getstackobj(v, -1, &smc->import_array));
      sq_pop(v, 1);

      /* STACK: ... module delegate */
    }

    /* add exports */
    if (smc->module.exports.size > 0) {
      sq_newtable(v);  /* the exports table */

      size_t i;
      for (i = 0; i < smc->module.exports.size; ++i) {
        WasmInterpreterExport* export = &smc->module.exports.data[i];
        WasmInterpreterFuncSignature* sig =
            &smc->module.sigs.data[export->sig_index];

        WasmStringSlice* name = &export->name;
        sq_pushstring(v, name->start, name->length);

        /* STACK: ... module delegate export_table name */
        sq_pushinteger(v, i);
        sq_push(v, -5);

        /* STACK: ... module delegate export_table name i module */
        sq_newclosure(v, squirrel_wasm_export_callback, 2);
        /* TODO(binji): add typemask */
        sq_setparamscheck(v, sig->param_types.size + 1, NULL);

        /* STACK: ... module delegate export_table name export_closure */
        CHECK_SQ_RESULT(sq_newslot(v, -3, SQFalse));
      }

      /* STACK: ... module delegate export_table */
      sq_pushstring(v, "exports", -1);
      sq_push(v, -2);
      CHECK_SQ_RESULT(sq_newslot(v, -4, SQFalse));
      sq_pop(v, 1);
    }

    /* STACK: ... module delegate */
    CHECK_SQ_RESULT(sq_setdelegate(v, -2));

    result = init_thread(allocator, &smc->module, &smc->thread);
    if (result != WASM_OK)
      return sq_throwerror(v, "unable to initialize thread.");

    result = run_start_function(&smc->module, &smc->thread);
    if (result != WASM_OK)
      return sq_throwerror(v, "start function trapped.");

    return 1;
  } else {
    return sq_throwerror(v, "error reading module.");
  }
}

#undef CHECK_SQ_RESULT

#define CHECK_SQ_RESULT(cond)                                            \
  do {                                                                   \
    if (SQ_FAILED(cond)) {                                               \
      fprintf(stderr, "%s:%d: %s failed.\n", __FILE__, __LINE__, #cond); \
      return WASM_ERROR;                                                 \
    }                                                                    \
  } while (0)

static WasmResult init_squirrel(WasmAllocator* allocator, HSQUIRRELVM* out_sq) {
  HSQUIRRELVM v = sq_open(INITIAL_SQ_STACK_SIZE);
  sq_setprintfunc(v, squirrel_print, squirrel_error);
  sq_setcompilererrorhandler(v, squirrel_compiler_error);
  sq_newclosure(v, squirrel_errorhandler, 0);
  sq_seterrorhandler(v);

  sq_pushroottable(v);
  CHECK_SQ_RESULT(sqstd_register_bloblib(v));
  CHECK_SQ_RESULT(sqstd_register_iolib(v));
  sq_pop(v, 1);

  /* put allocator into the registry */
  sq_pushregistrytable(v);
  sq_pushstring(v, WASM_ALLOCATOR_NAME, -1);
  sq_pushuserpointer(v, (SQUserPointer)allocator);
  CHECK_SQ_RESULT(sq_newslot(v, -3, SQFalse));
  sq_pop(v, 1);

  /* initialize Wasm API */
  sq_newtable(v);
  sq_pushstring(v, "instantiateModule", -1);
  sq_newclosure(v, squirrel_wasm_instantiate_module, 0);
  CHECK_SQ_RESULT(sq_setparamscheck(v, 3, "t x t"));
  CHECK_SQ_RESULT(sq_newslot(v, -3, SQFalse));

  sq_pushroottable(v);
  sq_pushstring(v, "Wasm", -1);
  sq_push(v, -3);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v, 2);

  *out_sq = v;
  return WASM_OK;
}

#undef CHECK_SQ_RESULT

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

  HSQUIRRELVM v;
  result = init_squirrel(allocator, &v);
  if (result == WASM_OK) {
    result = run_squirrel_file(v, s_infile);
    sq_close(v);
  }

  wasm_print_allocator_stats(allocator);
  wasm_destroy_allocator(allocator);
  return result;
}
