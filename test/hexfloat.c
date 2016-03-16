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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm-config.h"
#include "wasm-parse-literal.h"

#define DEFAULT_NUM_CPUS 4
#define OK 0
#define ERROR 1

static void print_float_bits(const char* desc, uint32_t bits) {
  printf("%s: %x = sign:%x exp:%d sig:%x\n", desc, bits, (bits >> 31),
         (int)((bits >> 23) & 0xff) - 127, (bits & 0x7fffff));
}

static void print_double_bits(const char* desc, uint64_t bits) {
  printf("%s: %" PRIx64 " = sign:%d exp:%d sig:%" PRIx64 "\n", desc, bits,
         (int)(bits >> 63), (int)((bits >> 52) & 0x7ff) - 1023,
         (bits & 0xfffffffffffff));
}

int test_all_floats(int shard, int num_shards) {
  uint32_t last_value;
  uint32_t value = shard;
  uint32_t last_exp = 0;
  do {
    uint32_t exp = (value >> 23) & 0xff;
    /* skip infinity and nan */
    if (exp != 0xff) {
      float f;
      memcpy(&f, &value, sizeof(f));

      if (exp != last_exp) {
        printf("shard %d: value: %g (%a)\n", shard, f, f);
        last_exp = exp;
      }

      char buffer[100];
      snprintf(buffer, 100, "%a", f);
      size_t len = strlen(buffer);

      uint32_t me;
      wasm_parse_float(WASM_LITERAL_TYPE_HEXFLOAT, buffer, buffer + len, &me);

      char* endptr;
      float them_float = strtof(buffer, &endptr);
      uint32_t them;
      memcpy(&them, &them_float, sizeof(them));

      if (me != them) {
        printf("shard %d: value: %g (%a)\n", shard, f, f);
        print_float_bits("me", me);
        print_float_bits("them", them);
        assert(0);
        return 1;
      }
    }

    last_value = value;
    value += num_shards;
  } while (last_value < value);
  return 0;
}

int test_many_doubles(int shard, int num_shards) {
  uint32_t last_v;
  uint32_t v = shard;
  uint32_t last_exp = 0;
  do {
    uint64_t value = ((uint64_t)v << 32) | v;
    uint64_t exp = (value >> 52) & 0x7ff;
    /* skip infinity and nan */
    if (exp != 0x7ff) {
      double d;
      memcpy(&d, &value, sizeof(d));

      if (exp != last_exp) {
        printf("shard %d: value: %g (%a)\n", shard, d, d);
        last_exp = exp;
      }

      char buffer[100];
      snprintf(buffer, 100, "%a", d);
      size_t len = strlen(buffer);

      uint64_t me;
      wasm_parse_double(WASM_LITERAL_TYPE_HEXFLOAT, buffer, buffer + len, &me);

      char* endptr;
      double them_double = strtod(buffer, &endptr);
      uint64_t them;
      memcpy(&them, &them_double, sizeof(them));

      if (me != them) {
        printf("shard %d: value: %g (%a)\n", shard, d, d);
        print_double_bits("me", me);
        print_double_bits("them", them);
        assert(0);
      }
    }

    last_v = v;
    v += num_shards;
  } while (last_v < v);
  return 0;
}

typedef struct ThreadInfo {
  pthread_t thread;
  int shard;
  int num_shards;
  int result;
} ThreadInfo;

void* thread_start(void* param) {
  ThreadInfo* info = param;
  if (!test_all_floats(info->shard, info->num_shards))
    return NULL;

  if (!test_many_doubles(info->shard, info->num_shards))
    return NULL;

  info->result = OK;
  return NULL;
}

int main() {
  int num_cpus;
#if HAS_SYSCONF
  num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  if (num_cpus == -1)
    num_cpus = DEFAULT_NUM_CPUS;
#else
  num_cpus = DEFAULT_NUM_CPUS;
#endif

  ThreadInfo* threads = calloc(num_cpus, sizeof(ThreadInfo));
  int i;
  for (i = 0; i < num_cpus; ++i) {
    threads[i].shard = i;
    threads[i].num_shards = num_cpus;
    threads[i].result = ERROR;
    pthread_create(&threads[i].thread, NULL, thread_start, &threads[i]);
  }

  int result = OK;
  for (i = 0; i < num_cpus; ++i) {
    pthread_join(threads[i].thread, NULL);
    if (threads[i].result != OK)
      result = ERROR;
  }

  return result;
}
