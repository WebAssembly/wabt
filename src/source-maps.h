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
#ifndef WABT_SOURCE_MAPS_H
#define WABT_SOURCE_MAPS_H
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct SourceMap {
  static constexpr const int32_t kSourceMapVersion = 3;
  // Representation of mappings
  struct Segment {
    // Field 1
    uint32_t generated_col = 0;        // Start column in generated code. Remove?
    uint32_t generated_col_delta = 0;  // Delta from previous generated col
    bool has_source = false;               // If true, fields 2-4 will be valid.
    // Field 2
    size_t source = 0;  // Index into sources list
    // Field 3
    uint32_t source_line = 0;        // Start line in source. Remove?
    int32_t source_line_delta = 0;     // Delta from previous source line
    // Field 4
    uint32_t source_col = 0;        // Start column in source. Remove?
    int32_t source_col_delta = 0;     // Delta from previous source column
    bool has_name = false; // If true, field 5 will be valid.
    // Field 5
    size_t name = 0;  // Index into names list
    Segment() = default;
    // Explicit constructor, mostly used for testing.
    Segment(std::pair<uint32_t, int32_t> generated_col_p,
            std::pair<bool, size_t> source_p,
            std::pair<uint32_t, int32_t> source_line_p,
            std::pair<uint32_t, int32_t> source_col_p,
            std::pair<bool, size_t> name_p) {
      std::tie(generated_col, generated_col_delta) = generated_col_p;
      std::tie(has_source, source) = source_p;
      std::tie(source_line, source_line_delta) = source_line_p;
      std::tie(source_col, source_col_delta) = source_col_p;
      std::tie(has_name, name) = name_p;
    }
  };
  struct SegmentGroup {
    uint32_t generated_line;  // Line in the generated file for all segments
    std::vector<Segment> segments;
  };

  // Top level fields
  std::string file;         // Generated code filename; optional
  std::string source_root;  // Prepended to entries in sources list; optional
  std::vector<std::string> sources;          // List of sources use by mappings
  std::vector<std::string> sources_content;  // Not supported yet.
  std::vector<std::string> names;            // Not supported yet.
  std::vector<SegmentGroup> segment_groups;

  SourceMap(std::string file_, std::string source_root_)
      : file(file_), source_root(source_root_) {}

  void Dump();
  bool Validate(bool fatal=false) const;
};

class SourceMapGenerator {
 public:
  struct SourceLocation {
    uint32_t line;
    uint32_t col;
  };

  SourceMapGenerator(std::string file_, std::string source_root_)
      : map(file_, source_root_) {}

  void AddMapping(SourceLocation generated, SourceLocation original,
                  std::string source);
  void DumpMappings() { DumpRawMappings(); }
  SourceMap& GetMap() {
    CompressMappings();
    return map;
  };
  // AddMapping(SourceLocation original, token, str source)
 public:
  struct SourceMapping {
    SourceLocation original;
    SourceLocation generated;  // Use binary location?
    size_t source_idx;         // pointer to src?
    // We don't use the 'name' field currently.
    bool operator<(const SourceMapping& other) const;
    bool operator==(const SourceMapping& other) const;
    void Dump() const;
  };
  friend class SourceMappingTest;
  void CompressMappings();

  std::string SerializeMappings();
  bool map_prepared = false;  // Is the map compressed and ready for export?
  SourceMap map;
  std::map<std::string, size_t> sources_map;
  std::vector<SourceMapping> mappings;
  // Parse from file
  // Incrementally add full mappings
  // Dump to file
  // Lookup mapping bidirectionally (future?)
  // Future? Support modifiable mappings (e.g. a way to pin source location to
  // IR) Add source location without generated mapping (with e.g. an opaque
  // handle/token) Apply generated-location to each handle (
  void DumpRawMappings();
};

#endif  // WABT_SOURCE_MAPS_H
