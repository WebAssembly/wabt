/*
 * Copyright 2018 WebAssembly Community Group participants
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

#include "wasm-rt-impl.h"
#include "wasm-rt-os.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#define PAGE_SIZE 65536

typedef struct FuncType {
  wasm_rt_type_t* params;
  wasm_rt_type_t* results;
  uint32_t param_count;
  uint32_t result_count;
} FuncType;

uint32_t wasm_rt_call_stack_depth;
uint32_t g_saved_call_stack_depth;

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER
bool g_signal_handler_installed = false;
#endif

jmp_buf g_jmp_buf;
FuncType* g_func_types;
uint32_t g_func_type_count;

void wasm_rt_trap(wasm_rt_trap_t code) {
  assert(code != WASM_RT_TRAP_NONE);
  wasm_rt_call_stack_depth = g_saved_call_stack_depth;
  WASM_RT_LONGJMP(g_jmp_buf, code);
}

static bool func_types_are_equal(FuncType* a, FuncType* b) {
  if (a->param_count != b->param_count || a->result_count != b->result_count)
    return 0;
  uint32_t i;
  for (i = 0; i < a->param_count; ++i)
    if (a->params[i] != b->params[i])
      return 0;
  for (i = 0; i < a->result_count; ++i)
    if (a->results[i] != b->results[i])
      return 0;
  return 1;
}

uint32_t wasm_rt_register_func_type(uint32_t param_count,
                                    uint32_t result_count,
                                    ...) {
  FuncType func_type;
  func_type.param_count = param_count;
  func_type.params = malloc(param_count * sizeof(wasm_rt_type_t));
  func_type.result_count = result_count;
  func_type.results = malloc(result_count * sizeof(wasm_rt_type_t));

  va_list args;
  va_start(args, result_count);

  uint32_t i;
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

  uint32_t idx = g_func_type_count++;
  g_func_types = realloc(g_func_types, g_func_type_count * sizeof(FuncType));
  g_func_types[idx] = func_type;
  return idx + 1;
}

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
static void signal_handler(int sig, siginfo_t* si, void* unused) {
  wasm_rt_trap(WASM_RT_TRAP_OOB);
}
#endif

// Heap aligned to 4GB
#define WASM_HEAP_ALIGNMENT 0x100000000ull

bool wasm_rt_allocate_memory(wasm_rt_memory_t* memory,
                             uint32_t initial_pages,
                             uint32_t max_pages) {
  uint32_t byte_length = initial_pages * PAGE_SIZE;
#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
  if (!g_signal_handler_installed) {
    g_signal_handler_installed = true;
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
  }

  /* Reserve 8GiB. */
  void* addr =
      os_mmap_aligned(NULL, 0x200000000ul, MMAP_PROT_NONE, MMAP_MAP_NONE,
                      WASM_HEAP_ALIGNMENT, /*alignment_offset=*/0);

  if (addr == (void*)-1) {
    os_print_last_error("os_mmap failed.");
    abort();
  }
  int ret = os_mmap_commit(addr, byte_length, MMAP_PROT_READ | MMAP_PROT_WRITE);
  if (ret != 0) {
    return false;
  }
  memory->data = addr;
#else
  memory->data = calloc(byte_length, 1);
#endif
  memory->size = byte_length;
  memory->pages = initial_pages;
  memory->max_pages = max_pages;
  return true;
}

uint32_t wasm_rt_grow_memory(wasm_rt_memory_t* memory, uint32_t delta) {
  uint32_t old_pages = memory->pages;
  uint32_t new_pages = memory->pages + delta;
  if (new_pages == 0) {
    return 0;
  }
  if (new_pages < old_pages || new_pages > memory->max_pages) {
    return (uint32_t)-1;
  }
  uint32_t old_size = old_pages * PAGE_SIZE;
  uint32_t new_size = new_pages * PAGE_SIZE;
  uint32_t delta_size = delta * PAGE_SIZE;
#if WASM_RT_MEMCHECK_SIGNAL_HANDLER_POSIX
  uint8_t* new_data = memory->data;
  int ret = os_mmap_commit(new_data + old_size, delta_size,
                           MMAP_PROT_READ | MMAP_PROT_WRITE);
  if (ret != 0) {
    return (uint32_t)-1;
  }
#else
  uint8_t* new_data = realloc(memory->data, new_size);
  if (new_data == NULL) {
    return (uint32_t)-1;
  }
#if !WABT_BIG_ENDIAN
  memset(new_data + old_size, 0, delta_size);
#endif
#endif
#if WABT_BIG_ENDIAN
  memmove(new_data + new_size - old_size, new_data, old_size);
  memset(new_data, 0, delta_size);
#endif
  memory->pages = new_pages;
  memory->size = new_size;
  memory->data = new_data;
  return old_pages;
}

#ifdef _WIN32
static float quiet_nanf(float x) {
  uint32_t tmp;
  memcpy(&tmp, &x, 4);
  tmp |= 0x7fc00000lu;
  memcpy(&x, &tmp, 4);
  return x;
}

static double quiet_nan(double x) {
  uint64_t tmp;
  memcpy(&tmp, &x, 8);
  tmp |= 0x7ff8000000000000llu;
  memcpy(&x, &tmp, 8);
  return x;
}
#endif

double wasm_rt_trunc(double x) {
#ifdef _WIN32
  if (isnan(x))
    return quiet_nan(x);
#endif
  return trunc(x);
}

float wasm_rt_truncf(float x) {
#ifdef _WIN32
  if (isnan(x))
    return quiet_nanf(x);
#endif
  return truncf(x);
}

float wasm_rt_nearbyintf(float x) {
#ifdef _WIN32
  if (isnan(x))
    return quiet_nanf(x);
#endif
  return nearbyintf(x);
}

double wasm_rt_nearbyint(double x) {
#ifdef _WIN32
  if (isnan(x))
    return quiet_nan(x);
#endif
  return nearbyint(x);
}

float wasm_rt_fabsf(float x) {
#ifdef _WIN32
  if (isnan(x)) {
    uint32_t tmp;
    memcpy(&tmp, &x, 4);
    if (tmp & (1 << 31)) {
      tmp = tmp & ~(1 << 31);
      memcpy(&x, &tmp, 4);
    }
    return x;
  }
#endif
  return fabsf(x);
}

double wasm_rt_fabs(double x) {
#ifdef _WIN32
  if (isnan(x)) {
    uint64_t tmp;
    memcpy(&tmp, &x, 8);
    if (tmp & (1ll << 63)) {
      tmp = tmp & ~(1ll << 63);
      memcpy(&x, &tmp, 8);
    }
    return x;
  }
#endif
  return fabs(x);
}

void wasm_rt_allocate_table(wasm_rt_table_t* table,
                            uint32_t elements,
                            uint32_t max_elements) {
  table->size = elements;
  table->max_size = max_elements;
  table->data = calloc(table->size, sizeof(wasm_rt_elem_t));
}

const char* wasm_rt_strerror(wasm_rt_trap_t trap) {
  switch (trap) {
    case WASM_RT_TRAP_NONE:
      return "No error";
    case WASM_RT_TRAP_OOB:
      return "Out-of-bounds access in linear memory";
    case WASM_RT_TRAP_INT_OVERFLOW:
      return "Integer overflow on divide or truncation";
    case WASM_RT_TRAP_DIV_BY_ZERO:
      return "Integer divide by zero";
    case WASM_RT_TRAP_INVALID_CONVERSION:
      return "Conversion from NaN to integer";
    case WASM_RT_TRAP_UNREACHABLE:
      return "Unreachable instruction executed";
    case WASM_RT_TRAP_CALL_INDIRECT:
      return "Invalid call_indirect";
    case WASM_RT_TRAP_EXHAUSTION:
      return "Call stack exhausted";
  }
  return "invalid trap code";
}
