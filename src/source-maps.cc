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

#define INVALID() do { \
    if (fatal) { abort(); } else { return false; }      \
  } while(0);

bool SourceMap::Validate(bool fatal) const {
  for (size_t i = 0; i < segment_groups.size(); ++i) {
    const auto& group = segment_groups[i];
    if (i > 0 &&
        group.generated_line <= segment_groups[i - 1].generated_line) {
      INVALID();
    }
    for (size_t j = 0; j < group.segments.size(); ++j) {
      const auto& seg = group.segments[j];
      const Segment* last_seg = nullptr;
      if (j > 0) last_seg = &group.segments[j - 1];
      if (seg.generated_col_delta == 0) INVALID();
      if (!seg.has_source && seg.has_name) INVALID();
      if (!seg.has_source) return true;
      if (seg.source >= sources.size()) INVALID();
      if (last_seg) {
        if (seg.source_line_delta == 0) INVALID();
        // FIXME: This has a limitation that if this seg has a source, the last one must.
        if (last_seg->source_line + seg.source_line_delta != seg.source_line) {
          INVALID();
        }
        if (seg.source_col_delta == 0) INVALID();
        if (last_seg->source_col + seg.source_col_delta != seg.source_col) {
          INVALID();
        }
      }
      if (!seg.has_name) return true;
      if (seg.name >= names.size()) INVALID();
    }
  }
  return true;
}

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

bool SourceMapGenerator::SourceMapping::operator==(
    const SourceMapGenerator::SourceMapping& rhs) const {
  return !cmpLocation(generated, rhs.generated) &&
      !cmpLocation(original, rhs.original) &&
      source_idx == rhs.source_idx;
}

void SourceMapGenerator::AddMapping(SourceLocation original,
                                    SourceLocation generated,
                                    std::string source) {
  map_prepared = false;  // New mapping invalidates compressed map.
  size_t source_idx = INDEX_NONE;
    auto s = sources_map.find(source);
    if (s == sources_map.end()) {
      source_idx = map.sources.size();
      map.sources.push_back(source);
      bool inserted;
      std::tie(std::ignore, inserted) =
          sources_map.insert({source, source_idx});
      assert(inserted);
    } else {
      source_idx = s->second;
    }
  mappings.push_back({original, generated, source_idx});
}

void SourceMapGenerator::CompressMappings() {
  std::sort(mappings.begin(), mappings.end());
  uint32_t last_gen_line = static_cast<uint32_t>(-1);
  uint32_t last_gen_col = 0;
  uint32_t last_source_line = 0;
  uint32_t last_source_col = 0;
  map.segment_groups.clear();
  SourceMapGenerator::SourceMapping* last_mapping = nullptr;
  for (auto& mapping : mappings) {
    if (mapping.generated.line != last_gen_line) {
      // Output an empty segment group for each line between the previous
      // and current.
      assert(map.segment_groups.empty() ||
             mapping.generated.line > last_gen_line); // Not sorted.
      while (++last_gen_line <= mapping.generated.line) {
        map.segment_groups.push_back(
            {last_gen_line, std::vector<SourceMap::Segment>()});
      }
      last_gen_line = mapping.generated.line;
      last_gen_col = 0;
    }
    if (last_mapping != nullptr && mapping == *last_mapping)
      continue;
    last_mapping = &mapping;

    auto& group = map.segment_groups.back();
    group.segments.emplace_back();
    SourceMap::Segment& seg = group.segments.back();
    memset(&seg, 0x0, sizeof(seg));
    seg.generated_col = mapping.generated.col;
    seg.generated_col_delta = mapping.generated.col - last_gen_col;
    last_gen_col = mapping.generated.col;
    seg.has_source = mapping.source_idx != INDEX_NONE;
    assert(seg.has_source); // TODO(dschuff): support mappings without source
    if (seg.has_source) {
      seg.source_line = mapping.original.line;
      seg.source_line_delta = mapping.original.line - last_source_line;
      last_source_line = mapping.original.line;
      seg.source_col = mapping.original.col;
      seg.source_col_delta = mapping.original.col - last_source_col;
      last_source_col = mapping.original.col;
    }
    seg.has_name = false; // TODO(dschuff): add support
  }
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
            << "Sources [";
  for (size_t i = 0; i < map.sources.size(); ++i) {
    std::cout << i << ":" << map.sources[i] << ", ";
  }
  std::cout << "]\n";
  for (const auto& m : mappings) {
    std::cout << "Mapping " << m.original.line << ":" << m.original.col
              << " -> " << m.generated.line << ":" << m.generated.col << ":"
              << m.source_idx << "\n";
  }
}
