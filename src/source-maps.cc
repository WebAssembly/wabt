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
#include "source-maps.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#define INDEX_NONE static_cast<size_t>(-1)

static int32_t cmpLocation(const SourceMapGenerator::SourceLocation& a,
                           const SourceMapGenerator::SourceLocation& b) {
  int32_t cmp = a.line - b.line;
  return cmp ? cmp : a.col - b.col;
}

bool SourceMapGenerator::SourceMapping::operator<(
    const SourceMapGenerator::SourceMapping& rhs) const {
  return cmpLocation(generated, rhs.generated) ||
                 cmpLocation(original, rhs.original) || source_idx != INDEX_NONE
             ? (rhs.source_idx != INDEX_NONE ? source_idx < rhs.source_idx
                                             : false)
             : rhs.source_idx == INDEX_NONE;
}

void SourceMapGenerator::AddMapping(SourceLocation original,
                                    SourceLocation generated,
                                    std::string source) {
  map_prepared = false;  // New mapping invalidates compressed map.
  size_t source_idx = INDEX_NONE;
  if (!source.empty()) {
    auto s = sources_map.find(source);
    if (s == sources_map.end()) {
      size_t source_idx = map.sources.size();
      map.sources.push_back(source);
      bool inserted;
      std::tie(std::ignore, inserted) =
          sources_map.insert({source, source_idx});
      assert(inserted);
    } else {
      source_idx = s->second;
    }
  }
  mappings.push_back({original, generated, source_idx});
}

void SourceMapGenerator::CompressMappings() {
  // Sort mappings
  std::sort(mappings.begin(), mappings.end());

  map_prepared = true;
}

std::string SourceMapGenerator::SerializeMappings() {
  std::vector<std::string> mapping_results;
  mapping_results.reserve(mappings.size());
  CompressMappings();
  return "";
}

void SourceMapGenerator::DumpRawMappings() {
  CompressMappings();  // Just to sort them
  std::cout << "Map: " << map.file << " " << map.source_root << "\n"
            << "Sources: ";
  for (size_t i = 0; i < map.sources.size(); ++i) {
    std::cout << i << ":" << map.sources[i] << " ";
  }
  std::cout << "\n";
  for (const auto& m : mappings) {
    std::cout << "Mapping " << m.original.line << ":" << m.original.col
              << " -> " << m.generated.line << ":" << m.generated.col << ":"
              << m.source_idx << "\n";
  }
}
