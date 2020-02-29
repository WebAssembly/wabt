/*
 * Copyright 2020 WebAssembly Community Group participants
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

#include "gtest/gtest.h"

#include "src/common.h"
#include "src/type.h"

using namespace wabt;

TEST(TypeTest, RefT) {
  // Skip some indexes to speed up the tests.
  for (Index i = 0; i < 8388608; i += 16) {
    Type type = Type::MakeRefT(i);

    EXPECT_EQ(Type::RefT, type);  // Implicit cast to Type::Enum.
    EXPECT_TRUE(type.IsRef());
    EXPECT_FALSE(type.IsNullableRef());
    EXPECT_FALSE(type.IsIndex());
    EXPECT_TRUE(type.IsRefT());
    EXPECT_EQ(i, type.GetRefTIndex());
    EXPECT_STREQ("ref $T", type.GetName());

    auto typevec = type.GetInlineVector();
    EXPECT_EQ(1u, typevec.size());
    EXPECT_EQ(typevec[0], type);
  }
}
