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

#include <stdio.h>

#include <vector>
#include <thread>

#include "gtest/gtest.h"

#include "literal.h"

#if HAVE_SYSCONF
#include <unistd.h>
#endif

#define FOREACH_UINT32_MULTIPLIER 1

#define FOREACH_UINT32(bits) \
  uint32_t last_bits = 0;    \
  uint32_t bits = shard;     \
  int last_top_byte = -1;    \
  for (; bits >= last_bits;  \
       last_bits = bits, bits += num_threads_ * FOREACH_UINT32_MULTIPLIER)

#define LOG_COMPLETION(bits)                                                 \
  if (shard == 0) {                                                          \
    int top_byte = bits >> 24;                                               \
    if (top_byte != last_top_byte) {                                         \
      printf("value: 0x%08x (%d%%)\r", bits,                                 \
             static_cast<int>(static_cast<float>(bits) * 100 / UINT32_MAX)); \
      fflush(stdout);                                                        \
      last_top_byte = top_byte;                                              \
    }                                                                        \
  }

#define LOG_DONE()     \
  if (shard == 0) {    \
    printf("done.\n"); \
    fflush(stdout);    \
  }

template <typename T, typename F>
T bit_cast(F value) {
  T result;
  memcpy(&result, &value, sizeof(result));
  return result;
}

static bool is_infinity_or_nan(uint32_t float_bits) {
  return ((float_bits >> 23) & 0xff) == 0xff;
}

static bool is_infinity_or_nan(uint64_t double_bits) {
  return ((double_bits >> 52) & 0x7ff) == 0x7ff;
}

class ThreadedTest : public ::testing::Test {
 protected:
  static const int kDefaultNumThreads = 2;

  virtual void SetUp() {
#if HAVE_SYSCONF
    num_threads_ = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads_ == -1)
      num_threads_ = kDefaultNumThreads;
#else
    num_threads_ = kDefaultNumThreads;
#endif
  }

  virtual void RunShard(int shard) = 0;

  void RunThreads() {
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads_; ++i) {
      threads.emplace_back(&ThreadedTest::RunShard, this, i);
    }

    for (std::thread& thread : threads) {
      thread.join();
    }
  }

  int num_threads_;
};

/* floats */
class AllFloatsParseTest : public ThreadedTest {
 protected:
  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(bits) {
      LOG_COMPLETION(bits);
      if (is_infinity_or_nan(bits))
        continue;

      float value = bit_cast<float>(bits);
      int len = snprintf(buffer, sizeof(buffer), "%a", value);

      uint32_t me;
      wabt_parse_float(WABT_LITERAL_TYPE_HEXFLOAT, buffer, buffer + len, &me);
      ASSERT_EQ(me, bits);
    }
    LOG_DONE();
  }
};

TEST_F(AllFloatsParseTest, Run) {
  RunThreads();
}

class AllFloatsWriteTest : public ThreadedTest {
 protected:
  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(bits) {
      LOG_COMPLETION(bits);
      if (is_infinity_or_nan(bits))
        continue;

      wabt_write_float_hex(buffer, sizeof(buffer), bits);

      char* endptr;
      float them_float = strtof(buffer, &endptr);
      uint32_t them_bits = bit_cast<uint32_t>(them_float);
      ASSERT_EQ(bits, them_bits);
    }
    LOG_DONE();
  }
};

TEST_F(AllFloatsWriteTest, Run) {
  RunThreads();
}

class AllFloatsRoundtripTest : public ThreadedTest {
 protected:
  static WabtLiteralType ClassifyFloat(uint32_t float_bits) {
    if (is_infinity_or_nan(float_bits)) {
      if (float_bits & 0x7fffff) {
        return WABT_LITERAL_TYPE_NAN;
      } else {
        return WABT_LITERAL_TYPE_INFINITY;
      }
    } else {
      return WABT_LITERAL_TYPE_HEXFLOAT;
    }
  }

  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(bits) {
      LOG_COMPLETION(bits);
      wabt_write_float_hex(buffer, sizeof(buffer), bits);
      int len = strlen(buffer);

      uint32_t new_bits;
      wabt_parse_float(ClassifyFloat(bits), buffer, buffer + len, &new_bits);
      ASSERT_EQ(new_bits, bits);
    }
    LOG_DONE();
  }
};

TEST_F(AllFloatsRoundtripTest, Run) {
  RunThreads();
}

/* doubles */
class ManyDoublesParseTest : public ThreadedTest {
 protected:
  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(halfbits) {
      LOG_COMPLETION(halfbits);
      uint64_t bits = (static_cast<uint64_t>(halfbits) << 32) | halfbits;
      if (is_infinity_or_nan(bits))
        continue;

      double value = bit_cast<double>(bits);
      int len = snprintf(buffer, sizeof(buffer), "%a", value);

      uint64_t me;
      wabt_parse_double(WABT_LITERAL_TYPE_HEXFLOAT, buffer, buffer + len, &me);
      ASSERT_EQ(me, bits);
    }
    LOG_DONE();
  }
};

TEST_F(ManyDoublesParseTest, Run) {
  RunThreads();
}

class ManyDoublesWriteTest : public ThreadedTest {
 protected:
  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(halfbits) {
      LOG_COMPLETION(halfbits);
      uint64_t bits = (static_cast<uint64_t>(halfbits) << 32) | halfbits;
      if (is_infinity_or_nan(bits))
        continue;

      wabt_write_double_hex(buffer, sizeof(buffer), bits);

      char* endptr;
      double them_double = strtod(buffer, &endptr);
      uint64_t them_bits = bit_cast<uint64_t>(them_double);
      ASSERT_EQ(bits, them_bits);
    }
    LOG_DONE();
  }
};

TEST_F(ManyDoublesWriteTest, Run) {
  RunThreads();
}

class ManyDoublesRoundtripTest : public ThreadedTest {
 protected:
  static WabtLiteralType ClassifyDouble(uint64_t double_bits) {
    if (is_infinity_or_nan(double_bits)) {
      if (double_bits & 0xfffffffffffffULL) {
        return WABT_LITERAL_TYPE_NAN;
      } else {
        return WABT_LITERAL_TYPE_INFINITY;
      }
    } else {
      return WABT_LITERAL_TYPE_HEXFLOAT;
    }
  }

  virtual void RunShard(int shard) {
    char buffer[100];
    FOREACH_UINT32(halfbits) {
      LOG_COMPLETION(halfbits);
      uint64_t bits = (static_cast<uint64_t>(halfbits) << 32) | halfbits;
      wabt_write_double_hex(buffer, sizeof(buffer), bits);
      int len = strlen(buffer);

      uint64_t new_bits;
      wabt_parse_double(ClassifyDouble(bits), buffer, buffer + len, &new_bits);
      ASSERT_EQ(new_bits, bits);
    }
    LOG_DONE();
  }
};

TEST_F(ManyDoublesRoundtripTest, Run) {
  RunThreads();
}
