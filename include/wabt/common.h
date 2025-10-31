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

#ifndef WABT_COMMON_H_
#define WABT_COMMON_H_

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "wabt/config.h"

#include "wabt/base-types.h"
#include "wabt/result.h"
#include "wabt/string-format.h"
#include "wabt/type.h"

#define WABT_FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define WABT_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define WABT_USE(x) static_cast<void>(x)

// 64k
#define WABT_DEFAULT_PAGE_SIZE 0x10000

inline uint64_t WABT_BYTES_TO_MIN_PAGES(uint64_t num_bytes,
                                        uint32_t page_size) {
  if ((page_size == 0) ||
      (page_size & (page_size - 1))) {  // malformed page sizes
    WABT_UNREACHABLE;
    return 0;
  }
  uint64_t num_pages = num_bytes / page_size;
  return (page_size * num_pages == num_bytes) ? num_pages : num_pages + 1;
}

#define WABT_ENUM_COUNT(name) \
  (static_cast<int>(name::Last) - static_cast<int>(name::First) + 1)

#define WABT_DISALLOW_COPY_AND_ASSIGN(type) \
  type(const type&) = delete;               \
  type& operator=(const type&) = delete;

#if WITH_EXCEPTIONS
#define WABT_TRY try {
#define WABT_CATCH_BAD_ALLOC \
  }                          \
  catch (std::bad_alloc&) {  \
  }
#define WABT_CATCH_BAD_ALLOC_AND_EXIT           \
  }                                             \
  catch (std::bad_alloc&) {                     \
    WABT_FATAL("Memory allocation failure.\n"); \
  }
#else
#define WABT_TRY
#define WABT_CATCH_BAD_ALLOC
#define WABT_CATCH_BAD_ALLOC_AND_EXIT
#endif

#define PRIindex "u"
#define PRIaddress PRIu64
#define PRIoffset PRIzx

namespace wabt {
inline void MemcpyEndianAware(void* dst,
                              const void* src,
                              size_t dsize,
                              size_t ssize,
                              size_t doff,
                              size_t soff,
                              size_t len) {
#if WABT_BIG_ENDIAN
  memcpy(static_cast<char*>(dst) + (dsize) - (len) - (doff),
         static_cast<const char*>(src) + (ssize) - (len) - (soff), (len));
#else
  memcpy(static_cast<char*>(dst) + (doff),
         static_cast<const char*>(src) + (soff), (len));
#endif
}
}

struct v128 {
  v128() = default;
  v128(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3) {
    set_u32(0, x0);
    set_u32(1, x1);
    set_u32(2, x2);
    set_u32(3, x3);
  }

  bool operator==(const v128& other) const {
    return std::equal(std::begin(v), std::end(v), std::begin(other.v));
  }
  bool operator!=(const v128& other) const { return !(*this == other); }

  uint8_t u8(int lane) const { return To<uint8_t>(lane); }
  uint16_t u16(int lane) const { return To<uint16_t>(lane); }
  uint32_t u32(int lane) const { return To<uint32_t>(lane); }
  uint64_t u64(int lane) const { return To<uint64_t>(lane); }
  uint32_t f32_bits(int lane) const { return To<uint32_t>(lane); }
  uint64_t f64_bits(int lane) const { return To<uint64_t>(lane); }

  void set_u8(int lane, uint8_t x) { return From<uint8_t>(lane, x); }
  void set_u16(int lane, uint16_t x) { return From<uint16_t>(lane, x); }
  void set_u32(int lane, uint32_t x) { return From<uint32_t>(lane, x); }
  void set_u64(int lane, uint64_t x) { return From<uint64_t>(lane, x); }
  void set_f32_bits(int lane, uint32_t x) { return From<uint32_t>(lane, x); }
  void set_f64_bits(int lane, uint64_t x) { return From<uint64_t>(lane, x); }

  bool is_zero() const {
    return std::all_of(std::begin(v), std::end(v),
                       [](uint8_t x) { return x == 0; });
  }
  void set_zero() { std::fill(std::begin(v), std::end(v), 0); }

  template <typename T>
  T To(int lane) const {
    static_assert(sizeof(T) <= sizeof(v), "Invalid cast!");
    assert((lane + 1) * sizeof(T) <= sizeof(v));
    T result;
    wabt::MemcpyEndianAware(&result, v, sizeof(result), sizeof(v), 0,
                            lane * sizeof(T), sizeof(result));
    return result;
  }

  template <typename T>
  void From(int lane, T data) {
    static_assert(sizeof(T) <= sizeof(v), "Invalid cast!");
    assert((lane + 1) * sizeof(T) <= sizeof(v));
    wabt::MemcpyEndianAware(v, &data, sizeof(v), sizeof(data), lane * sizeof(T),
                            0, sizeof(data));
  }

  uint8_t v[16];
};

namespace wabt {

template <typename Dst, typename Src>
Dst WABT_VECTORCALL Bitcast(Src&& value) {
  static_assert(sizeof(Src) == sizeof(Dst), "Bitcast sizes must match.");
  Dst result;
  memcpy(&result, &value, sizeof(result));
  return result;
}

template <typename T>
void ZeroMemory(T& v) {
  WABT_STATIC_ASSERT(std::is_trivial<T>::value);
  memset(&v, 0, sizeof(v));
}

// Placement construct
template <typename T, typename... Args>
void Construct(T& placement, Args&&... args) {
  new (&placement) T(std::forward<Args>(args)...);
}

// Placement destruct
template <typename T>
void Destruct(T& placement) {
  placement.~T();
}

enum class LabelType {
  Func,
  InitExpr,
  Block,
  Loop,
  If,
  Else,
  Try,
  TryTable,
  Catch,
  ExtractFunc,
  First = Func,
  Last = Catch,
};
constexpr int kLabelTypeCount = WABT_ENUM_COUNT(LabelType);

struct Location {
  enum class Type {
    Text,
    Binary,
  };

  Location() : line(0), first_column(0), last_column(0) {}
  Location(std::string_view filename,
           int line,
           int first_column,
           int last_column)
      : filename(filename),
        line(line),
        first_column(first_column),
        last_column(last_column) {}
  explicit Location(size_t offset) : offset(offset) {}

  std::string_view filename;
  union {
    // For text files.
    struct {
      int line;
      int first_column;
      int last_column;
    };
    // For binary files.
    struct {
      size_t offset;
    };
  };
};

enum class SegmentKind {
  Active,
  Passive,
  Declared,
};

enum class CatchKind {
  Catch,
  CatchRef,
  CatchAll,
  CatchAllRef,
};

// Used in test asserts for special expected values "nan:canonical" and
// "nan:arithmetic"
enum class ExpectedNan {
  None,
  Canonical,
  Arithmetic,
};

// Matches binary format, do not change.
enum SegmentFlags : uint8_t {
  SegFlagsNone = 0,
  SegPassive = 1,        // bit 0: Is passive
  SegExplicitIndex = 2,  // bit 1: Has explict index (Implies table 0 if absent)
  SegDeclared = 3,       // Only used for declared segments
  SegUseElemExprs = 4,   // bit 2: Is elemexpr (Or else index sequence)

  SegFlagMax = (SegUseElemExprs << 1) - 1,  // All bits set.
};

enum class RelocType {
  FuncIndexLEB = 0,       // e.g. Immediate of call instruction
  TableIndexSLEB = 1,     // e.g. Loading address of function
  TableIndexI32 = 2,      // e.g. Function address in DATA
  MemoryAddressLEB = 3,   // e.g. Memory address in load/store offset immediate
  MemoryAddressSLEB = 4,  // e.g. Memory address in i32.const
  MemoryAddressI32 = 5,   // e.g. Memory address in DATA
  TypeIndexLEB = 6,       // e.g. Immediate type in call_indirect
  GlobalIndexLEB = 7,     // e.g. Immediate of global.get inst
  FunctionOffsetI32 = 8,  // e.g. Code offset in DWARF metadata
  SectionOffsetI32 = 9,   // e.g. Section offset in DWARF metadata
  TagIndexLEB = 10,       // Used in throw instructions
  MemoryAddressRelSLEB = 11,  // In PIC code, addr relative to __memory_base
  TableIndexRelSLEB = 12,   // In PIC code, table index relative to __table_base
  GlobalIndexI32 = 13,      // e.g. Global index in data (e.g. DWARF)
  MemoryAddressLEB64 = 14,  // Memory64: Like MemoryAddressLEB
  MemoryAddressSLEB64 = 15,     // Memory64: Like MemoryAddressSLEB
  MemoryAddressI64 = 16,        // Memory64: Like MemoryAddressI32
  MemoryAddressRelSLEB64 = 17,  // Memory64: Like MemoryAddressRelSLEB
  TableIndexSLEB64 = 18,        // Memory64: Like TableIndexSLEB
  TableIndexI64 = 19,           // Memory64: Like TableIndexI32
  TableNumberLEB = 20,          // e.g. Immediate of table.get
  MemoryAddressTLSSLEB = 21,    // Address relative to __tls_base
  FunctionOffsetI64 = 22,       // Memory64: Like FunctionOffsetI32
  MemoryAddressLocRelI32 = 23,  // Address relative to the relocation's location
  TableIndexRelSLEB64 = 24,     // Memory64: TableIndexRelSLEB
  MemoryAddressTLSSLEB64 = 25,  // Memory64: MemoryAddressTLSSLEB
  FuncIndexI32 = 26,            // Function index as an I32

  First = FuncIndexLEB,
  Last = FuncIndexI32,
};
constexpr int kRelocTypeCount = WABT_ENUM_COUNT(RelocType);

struct Reloc {
  Reloc(RelocType, size_t offset, Index index, int32_t addend = 0);

  RelocType type;
  size_t offset;
  Index index;
  int32_t addend;
};

enum class LinkingEntryType {
  SegmentInfo = 5,
  InitFunctions = 6,
  ComdatInfo = 7,
  SymbolTable = 8,
};

enum class DylinkEntryType {
  MemInfo = 1,
  Needed = 2,
  ExportInfo = 3,
  ImportInfo = 4,
};

enum class SymbolType {
  Function = 0,
  Data = 1,
  Global = 2,
  Section = 3,
  Tag = 4,
  Table = 5,
};

enum class ComdatType {
  Data = 0x0,
  Function = 0x1,
};

#define WABT_SYMBOL_MASK_VISIBILITY 0x4
#define WABT_SYMBOL_MASK_BINDING 0x3
#define WABT_SYMBOL_FLAG_UNDEFINED 0x10
#define WABT_SYMBOL_FLAG_EXPORTED 0x20
#define WABT_SYMBOL_FLAG_EXPLICIT_NAME 0x40
#define WABT_SYMBOL_FLAG_NO_STRIP 0x80
#define WABT_SYMBOL_FLAG_TLS 0x100
#define WABT_SYMBOL_FLAG_ABS 0x200
#define WABT_SYMBOL_FLAG_MAX 0x3ff

#define WABT_SEGMENT_FLAG_STRINGS 0x1
#define WABT_SEGMENT_FLAG_TLS 0x2
#define WASM_SEGMENT_FLAG_RETAIN 0x4
#define WABT_SEGMENT_FLAG_MAX 0xff

enum class SymbolVisibility {
  Default = 0,
  Hidden = 4,
};

enum class SymbolBinding {
  Global = 0,
  Weak = 1,
  Local = 2,
};

/* matches binary format, do not change */
enum class ExternalKind {
  Func = 0,
  Table = 1,
  Memory = 2,
  Global = 3,
  Tag = 4,

  First = Func,
  Last = Tag,
};
constexpr int kExternalKindCount = WABT_ENUM_COUNT(ExternalKind);

struct Limits {
  Limits() = default;
  explicit Limits(uint64_t initial) : initial(initial) {}
  Limits(uint64_t initial, uint64_t max)
      : initial(initial), max(max), has_max(true) {}
  Limits(uint64_t initial, uint64_t max, bool is_shared)
      : initial(initial), max(max), has_max(true), is_shared(is_shared) {}
  Limits(uint64_t initial, uint64_t max, bool is_shared, bool is_64)
      : initial(initial),
        max(max),
        has_max(true),
        is_shared(is_shared),
        is_64(is_64) {}
  Type IndexType() const { return is_64 ? Type::I64 : Type::I32; }

  uint64_t initial = 0;
  uint64_t max = 0;
  bool has_max = false;
  bool is_shared = false;
  bool is_64 = false;
};

enum { WABT_USE_NATURAL_ALIGNMENT = 0xFFFFFFFFFFFFFFFF };

Result ReadFile(std::string_view filename, std::vector<uint8_t>* out_data);

void InitStdio();

/* external kind */

extern const char* g_kind_name[];

static inline const char* GetKindName(ExternalKind kind) {
  return static_cast<size_t>(kind) < kExternalKindCount
             ? g_kind_name[static_cast<size_t>(kind)]
             : "<error_kind>";
}

/* reloc */

extern const char* g_reloc_type_name[];

static inline const char* GetRelocTypeName(RelocType reloc) {
  return static_cast<size_t>(reloc) < kRelocTypeCount
             ? g_reloc_type_name[static_cast<size_t>(reloc)]
             : "<error_reloc_type>";
}

/* symbol */

static inline const char* GetSymbolTypeName(SymbolType type) {
  switch (type) {
    case SymbolType::Function:
      return "func";
    case SymbolType::Global:
      return "global";
    case SymbolType::Data:
      return "data";
    case SymbolType::Section:
      return "section";
    case SymbolType::Tag:
      return "tag";
    case SymbolType::Table:
      return "table";
    default:
      return "<error_symbol_type>";
  }
}

template <typename T>
void ConvertBackslashToSlash(T begin, T end) {
  std::replace(begin, end, '\\', '/');
}

inline void ConvertBackslashToSlash(char* s, size_t length) {
  ConvertBackslashToSlash(s, s + length);
}

inline void ConvertBackslashToSlash(char* s) {
  ConvertBackslashToSlash(s, strlen(s));
}

inline void ConvertBackslashToSlash(std::string* s) {
  ConvertBackslashToSlash(s->begin(), s->end());
}

inline void SwapBytesSized(void* addr, size_t size) {
  auto bytes = static_cast<uint8_t*>(addr);
  for (size_t i = 0; i < size / 2; i++) {
    std::swap(bytes[i], bytes[size - 1 - i]);
  }
}

}  // namespace wabt

#endif  // WABT_COMMON_H_
