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

#define WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX 1
#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "wasm-rt.h"

int g_tests_run;
int g_tests_passed;
wasm2c_sandbox_funcs_t g_curr_sbx_details;
wasm2c_sandbox_t* g_curr_sbx = 0;

static void run_spec_tests(void);

static void error(const char* file, int line, const char* format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "%s:%d: assertion failed: ", file, line);
  vfprintf(stderr, format, args);
  va_end(args);
}

jmp_buf g_jmp_buf;

void w2c_trap_handler(uint32_t code, const char* msg) {
  g_curr_sbx_details.reset_wasm2c_stack_depth(g_curr_sbx);
  siglongjmp(g_jmp_buf, code);
}

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
static void signal_handler(int sig, siginfo_t* si, void* unused) {
  wasm_rt_trap(WASM_RT_TRAP_OOB);
}
#endif

#define ASSERT_TRAP(f)                                         \
  do {                                                         \
    g_tests_run++;                                             \
    if (sigsetjmp(g_jmp_buf, 1) != 0) {                             \
      g_tests_passed++;                                        \
    } else {                                                   \
      (void)(f);                                               \
      error(__FILE__, __LINE__, "expected " #f " to trap.\n"); \
    }                                                          \
  } while (0)

#define ASSERT_EXHAUSTION(f)                                     \
  do {                                                           \
    g_tests_run++;                                               \
    wasm_rt_trap_t code = sigsetjmp(g_jmp_buf, 1);                    \
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
    if (sigsetjmp(g_jmp_buf, 1) != 0) {                 \
      error(__FILE__, __LINE__, #f " trapped.\n"); \
    } else {                                       \
      f;                                           \
      g_tests_passed++;                            \
    }                                              \
  } while (0)

#define ASSERT_RETURN_T(type, fmt, f, expected)                          \
  do {                                                                   \
    g_tests_run++;                                                       \
    if (sigsetjmp(g_jmp_buf, 1) != 0) {                                       \
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
    if (sigsetjmp(g_jmp_buf, 1) != 0) {                                            \
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

#define MULTI_T_UNPACK_(...) __VA_ARGS__
#define MULTI_T_UNPACK(arg) MULTI_T_UNPACK_ arg
#define MULTI_i32 "%u"
#define MULTI_i64 "%" PRIu64
#define MULTI_f32 "%.9g"
#define MULTI_f64 "%.17g"
#define ASSERT_RETURN_MULTI_T(type, fmt, f, compare, expected, found)    \
  do {                                                                   \
    g_tests_run++;                                                       \
    if (sigsetjmp(g_jmp_buf, 1) != 0) {                                       \
      error(__FILE__, __LINE__, #f " trapped.\n");                       \
    } else {                                                             \
      type actual = f;                                                   \
      if (compare) {                                                     \
        g_tests_passed++;                                                \
      } else {                                                           \
        error(__FILE__, __LINE__,                                        \
              "in " #f ": expected " fmt ", got " fmt ".\n",             \
              MULTI_T_UNPACK(expected), MULTI_T_UNPACK(found));          \
      }                                                                  \
    }                                                                    \
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

#define is_equal_i32 is_equal_u32
#define is_equal_i64 is_equal_u64

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
static uint64_t spectest_global_i64 = 666l;

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
uint64_t* Z_spectestZ_global_i64Z_j = &spectest_global_i64;

static void init_spectest_module(void) {
  wasm_rt_allocate_memory(&spectest_memory, 1, 2);
  wasm_rt_allocate_table(&spectest_table, 10, 20);
}

static void install_signal_handler() {
  #if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = signal_handler;

  /* Install SIGSEGV and SIGBUS handlers, since macOS seems to use SIGBUS. */
  if (sigaction(SIGSEGV, &sa, NULL) != 0 ||
      sigaction(SIGBUS, &sa, NULL) != 0) {
    perror("sigaction failed");
    abort();
  }
  #endif
}


int main(int argc, char** argv) {
  init_spectest_module();
  install_signal_handler();
  run_spec_tests();
  printf("%u/%u tests passed.\n", g_tests_passed, g_tests_run);
  return g_tests_passed != g_tests_run;
}
