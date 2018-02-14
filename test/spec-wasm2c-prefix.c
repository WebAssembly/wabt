#include <assert.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 65536

typedef struct FuncType {
  wasm_rt_type_t* params;
  wasm_rt_type_t* results;
  u32 param_count;
  u32 result_count;
} FuncType;

int g_tests_run;
int g_tests_passed;
jmp_buf g_jmp_buf;
FuncType* g_func_types;
u32 g_func_type_count;

static void run_spec_tests(void);

static void error(const char* file, int line, const char* format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "%s:%d: assertion failed: ", file, line);
  vfprintf(stderr, format, args);
  va_end(args);
}

#define ASSERT_TRAP(f)                                         \
  do {                                                         \
    g_tests_run++;                                             \
    if (setjmp(g_jmp_buf) != 0) {                              \
      g_tests_passed++;                                        \
    } else {                                                   \
      (void)(f);                                               \
      error(__FILE__, __LINE__, "expected " #f " to trap.\n"); \
    }                                                          \
  } while (0)

#define ASSERT_EXHAUSTION(f)                                     \
  do {                                                           \
    g_tests_run++;                                               \
    wasm_rt_trap_t code = setjmp(g_jmp_buf);                     \
    switch (code) {                                              \
      case WASM_RT_TRAP_NONE:                                    \
        (void)(f);                                               \
        error(__FILE__, __LINE__, "expected " #f " to trap.\n"); \
        break;                                                   \
      case WASM_RT_TRAP_EXHAUSTION:                              \
        g_tests_passed++;                                        \
        break;                                                   \
      default:                                                   \
        error(__FILE__, __LINE__,                                \
              "expected " #f                                     \
              " to trap due to exhaustion, got trap code %d.\n", \
              code);                                             \
        break;                                                   \
    }                                                            \
  } while (0)

#define ASSERT_RETURN(f)                           \
  do {                                             \
    g_tests_run++;                                 \
    if (setjmp(g_jmp_buf) != 0) {                  \
      error(__FILE__, __LINE__, #f " trapped.\n"); \
    } else {                                       \
      f;                                           \
      g_tests_passed++;                            \
    }                                              \
  } while (0)

#define ASSERT_RETURN_T(type, fmt, f, expected)                          \
  do {                                                                   \
    g_tests_run++;                                                       \
    if (setjmp(g_jmp_buf) != 0) {                                        \
      error(__FILE__, __LINE__, #f " trapped.\n");                       \
    } else {                                                             \
      type actual = f;                                                   \
      if (is_equal_##type(actual, expected)) {                           \
        g_tests_passed++;                                                \
      } else {                                                           \
        error(__FILE__, __LINE__,                                        \
              "in " #f ": expected %" fmt ", got %" fmt ".\n", expected, \
              actual);                                                   \
      }                                                                  \
    }                                                                    \
  } while (0)

#define ASSERT_RETURN_NAN_T(type, itype, fmt, f, kind)                        \
  do {                                                                        \
    g_tests_run++;                                                            \
    if (setjmp(g_jmp_buf) != 0) {                                             \
      error(__FILE__, __LINE__, #f " trapped.\n");                            \
    } else {                                                                  \
      type actual = f;                                                        \
      itype iactual;                                                          \
      memcpy(&iactual, &actual, sizeof(iactual));                             \
      if (is_##kind##_nan_##type(iactual)) {                                  \
        g_tests_passed++;                                                     \
      } else {                                                                \
        error(__FILE__, __LINE__,                                             \
              "in " #f ": expected result to be a " #kind " nan, got 0x%" fmt \
              ".\n",                                                          \
              iactual);                                                       \
      }                                                                       \
    }                                                                         \
  } while (0)

#define ASSERT_RETURN_I32(f, expected) ASSERT_RETURN_T(u32, "u", f, expected)
#define ASSERT_RETURN_I64(f, expected) ASSERT_RETURN_T(u64, PRIu64, f, expected)
#define ASSERT_RETURN_F32(f, expected) ASSERT_RETURN_T(f32, ".9g", f, expected)
#define ASSERT_RETURN_F64(f, expected) ASSERT_RETURN_T(f64, ".17g", f, expected)

#define ASSERT_RETURN_CANONICAL_NAN_F32(f) \
  ASSERT_RETURN_NAN_T(f32, u32, "08x", f, canonical)
#define ASSERT_RETURN_CANONICAL_NAN_F64(f) \
  ASSERT_RETURN_NAN_T(f64, u64, "016x", f, canonical)
#define ASSERT_RETURN_ARITHMETIC_NAN_F32(f) \
  ASSERT_RETURN_NAN_T(f32, u32, "08x", f, arithmetic)
#define ASSERT_RETURN_ARITHMETIC_NAN_F64(f) \
  ASSERT_RETURN_NAN_T(f64, u64, "016x", f, arithmetic)

static bool is_equal_u32(u32 x, u32 y) {
  return x == y;
}

static bool is_equal_u64(u64 x, u64 y) {
  return x == y;
}

static bool is_equal_f32(f32 x, f32 y) {
  u32 ux, uy;
  memcpy(&ux, &x, sizeof(ux));
  memcpy(&uy, &y, sizeof(uy));
  return ux == uy;
}

static bool is_equal_f64(f64 x, f64 y) {
  u64 ux, uy;
  memcpy(&ux, &x, sizeof(ux));
  memcpy(&uy, &y, sizeof(uy));
  return ux == uy;
}

static f32 make_nan_f32(u32 x) {
  x |= 0x7f800000;
  f32 res;
  memcpy(&res, &x, sizeof(res));
  return res;
}

static f64 make_nan_f64(u64 x) {
  x |= 0x7ff0000000000000;
  f64 res;
  memcpy(&res, &x, sizeof(res));
  return res;
}

static bool is_canonical_nan_f32(u32 x) {
  return (x & 0x7fffffff) == 0x7fc00000;
}

static bool is_canonical_nan_f64(u64 x) {
  return (x & 0x7fffffffffffffff) == 0x7ff8000000000000;
}

static bool is_arithmetic_nan_f32(u32 x) {
  return (x & 0x7fc00000) == 0x7fc00000;
}

static bool is_arithmetic_nan_f64(u64 x) {
  return (x & 0x7ff8000000000000) == 0x7ff8000000000000;
}

static bool func_types_are_equal(FuncType* a, FuncType* b) {
  if (a->param_count != b->param_count || a->result_count != b->result_count)
    return 0;
  int i;
  for (i = 0; i < a->param_count; ++i)
    if (a->params[i] != b->params[i])
      return 0;
  for (i = 0; i < a->result_count; ++i)
    if (a->results[i] != b->results[i])
      return 0;
  return 1;
}


/*
 * wasm_rt_* implementations
 */

uint32_t wasm_rt_call_stack_depth;

void wasm_rt_trap(wasm_rt_trap_t code) {
  assert(code != WASM_RT_TRAP_NONE);
  longjmp(g_jmp_buf, code);
}

u32 wasm_rt_register_func_type(u32 param_count, u32 result_count, ...) {
  FuncType func_type;
  func_type.param_count = param_count;
  func_type.params = malloc(param_count * sizeof(wasm_rt_type_t));
  func_type.result_count = result_count;
  func_type.results = malloc(result_count * sizeof(wasm_rt_type_t));

  va_list args;
  va_start(args, result_count);

  u32 i;
  for (i = 0; i < param_count; ++i)
    func_type.params[i] = va_arg(args, wasm_rt_type_t);
  for (i = 0; i < result_count; ++i)
    func_type.results[i] = va_arg(args, wasm_rt_type_t);
  va_end(args);

  for (i = 0; i < g_func_type_count; ++i) {
    if (func_types_are_equal(&g_func_types[i], &func_type)) {
      free(func_type.params);
      free(func_type.results);
      return i + 1;
    }
  }

  u32 idx = g_func_type_count++;
  g_func_types = realloc(g_func_types, g_func_type_count * sizeof(FuncType));
  g_func_types[idx] = func_type;
  return idx + 1;
}

void wasm_rt_allocate_memory(wasm_rt_memory_t* memory,
                             u32 initial_pages,
                             u32 max_pages) {
  memory->pages = initial_pages;
  memory->max_pages = max_pages;
  memory->size = initial_pages * PAGE_SIZE;
  memory->data = calloc(memory->size, 1);
}

u32 wasm_rt_grow_memory(wasm_rt_memory_t* memory, u32 delta) {
  u32 old_pages = memory->pages;
  u32 new_pages = memory->pages + delta;
  if (new_pages < old_pages || new_pages > memory->max_pages) {
    return (u32)-1;
  }
  memory->data = realloc(memory->data, new_pages);
  memory->pages = new_pages;
  memory->size = new_pages * PAGE_SIZE;
  return old_pages;
}

void wasm_rt_allocate_table(wasm_rt_table_t* table,
                            u32 elements,
                            u32 max_elements) {
  table->size = elements;
  table->max_size = max_elements;
  table->data = calloc(table->size, sizeof(wasm_rt_elem_t));
}


/*
 * spectest implementations
 */
static void spectest_print(void) {
  printf("spectest.print()\n");
}

static void spectest_print_i32(uint32_t i) {
  printf("spectest.print_i32(%d)\n", i);
}

static void spectest_print_f32(float f) {
  printf("spectest.print_f32(%g)\n", f);
}

static void spectest_print_i32_f32(uint32_t i, float f) {
  printf("spectest.print_i32_f32(%d %g)\n", i, f);
}

static void spectest_print_f64(double d) {
  printf("spectest.print_f64(%g)\n", d);
}

static void spectest_print_f64_f64(double d1, double d2) {
  printf("spectest.print_f64_f64(%g %g)\n", d1, d2);
}

static wasm_rt_table_t spectest_table;
static wasm_rt_memory_t spectest_memory;
static uint32_t spectest_global_i32 = 666;

void (*Z_spectestZ_printZ_vv)(void) = &spectest_print;
void (*Z_spectestZ_print_i32Z_vi)(uint32_t) = &spectest_print_i32;
void (*Z_spectestZ_print_f32Z_vf)(float) = &spectest_print_f32;
void (*Z_spectestZ_print_i32_f32Z_vif)(uint32_t,
                                       float) = &spectest_print_i32_f32;
void (*Z_spectestZ_print_f64Z_vd)(double) = &spectest_print_f64;
void (*Z_spectestZ_print_f64_f64Z_vdd)(double,
                                       double) = &spectest_print_f64_f64;
wasm_rt_table_t* Z_spectestZ_table = &spectest_table;
wasm_rt_memory_t* Z_spectestZ_memory = &spectest_memory;
uint32_t* Z_spectestZ_global_i32Z_i = &spectest_global_i32;

static void init_spectest_module(void) {
  wasm_rt_allocate_memory(&spectest_memory, 1, 2);
  wasm_rt_allocate_table(&spectest_table, 10, 20);
}


int main(int argc, char** argv) {
  init_spectest_module();
  run_spec_tests();
  printf("%u/%u tests passed.\n", g_tests_passed, g_tests_run);
  return g_tests_passed != g_tests_run;
}

