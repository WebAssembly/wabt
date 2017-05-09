/*
 * Copyright 2017 WebAssembly Community Group participants
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
#include "source-maps.h"

TEST(source_maps, constructor) { SourceMapGenerator("file", "source-root"); }

TEST(source_maps, sources) {
  SourceMapGenerator smg("source.out", "source-root");
  smg.AddMapping({1, 1}, {2, 3}, "asdf1");
  smg.AddMapping({1, 1}, {2, 2}, "");
  smg.AddMapping({1, 1}, {2, 3}, "asdf2");
  smg.DumpMappings();
  const auto map = smg.GetMap();
  EXPECT_EQ("source.out", map.file);
  EXPECT_EQ("source-root", map.source_root);
  ASSERT_EQ(2UL, map.sources.size());
  EXPECT_EQ("asdf1", map.sources[0]);
  EXPECT_EQ("asdf2", map.sources[1]);
}
