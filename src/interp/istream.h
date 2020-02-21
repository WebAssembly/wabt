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

#ifndef WABT_INTERP_ISTREAM_H_
#define WABT_INTERP_ISTREAM_H_

#include <cstdint>
#include <vector>
#include <string>

#include "src/common.h"
#include "src/opcode.h"
#include "src/stream.h"

namespace wabt {
namespace interp {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using Buffer = std::vector<u8>;

using ValueType = wabt::Type;

// Group instructions based on their immediates their operands. This way we can
// simplify instruction decoding, disassembling, and tracing. There is an
// example of an instruction that uses this encoding on the right.
enum class InstrKind {
  Imm_0_Op_0,             // Nop
  Imm_0_Op_1,             // i32.eqz
  Imm_0_Op_2,             // i32.add
  Imm_0_Op_3,             // select
  Imm_Jump_Op_0,          // br
  Imm_Jump_Op_1,          // br_if
  Imm_Index_Op_0,         // global.get
  Imm_Index_Op_1,         // global.set
  Imm_Index_Op_2,         // table.set
  Imm_Index_Op_3,         // memory.fill
  Imm_Index_Op_N,         // call
  Imm_Index_Index_Op_3,   // memory.init
  Imm_Index_Index_Op_N,   // call_indirect
  Imm_Index_Offset_Op_1,  // i32.load
  Imm_Index_Offset_Op_2,  // i32.store
  Imm_Index_Offset_Op_3,  // i32.atomic.rmw.cmpxchg
  Imm_I32_Op_0,           // i32.const
  Imm_I64_Op_0,           // i64.const
  Imm_F32_Op_0,           // f32.const
  Imm_F64_Op_0,           // f64.const
  Imm_I32_I32_Op_0,       // drop_keep
  Imm_I8_Op_1,            // i32x4.extract_lane
  Imm_I8_Op_2,            // i32x4.replace_lane
  Imm_V128_Op_0,          // v128.const
  Imm_V128_Op_2,          // v8x16.shuffle
};

struct Instr {
  Opcode op;
  InstrKind kind;
  union {
    u8 imm_u8;
    u32 imm_u32;
    f32 imm_f32;
    u64 imm_u64;
    f64 imm_f64;
    v128 imm_v128;
    struct { u32 fst, snd; } imm_u32x2;
  };
};

class Istream {
 public:
  using SerializedOpcode = u32;  // TODO: change to u16
  using Offset = u32;
  static const Offset kInvalidOffset = ~0;
  // Each br_table entry is made up of two instructions:
  //
  //   interp_drop_keep $drop $keep
  //   br $label
  //
  // Each opcode is a SerializedOpcode, and each immediate is a u32.
  static const Offset kBrTableEntrySize =
      sizeof(SerializedOpcode) * 2 + 3 * sizeof(u32);

  // Emit API.
  void Emit(u32);
  void Emit(Opcode::Enum);
  void Emit(Opcode::Enum, u8);
  void Emit(Opcode::Enum, u32);
  void Emit(Opcode::Enum, u64);
  void Emit(Opcode::Enum, v128);
  void Emit(Opcode::Enum, u32, u32);
  void EmitDropKeep(u32 drop, u32 keep);

  Offset EmitFixupU32();
  void ResolveFixupU32(Offset);

  Offset end() const;

  // Read API.
  Instr Read(Offset*) const;

  // Disassemble/Trace API.
  // TODO separate out disassembly/tracing?
  struct TraceSource {
    virtual ~TraceSource() {}
    // Whatever content should go before the instruction on each line, e.g. the
    // call stack size, value stack size, and istream offset.
    virtual std::string Header(Offset) = 0;
    virtual std::string Pick(Index, Instr) = 0;
  };

  struct DisassemblySource : TraceSource {
    std::string Header(Offset) override;
    std::string Pick(Index, Instr) override;
  };

  void Disassemble(Stream*) const;
  Offset Disassemble(Stream*, Offset) const;
  void Disassemble(Stream*, Offset from, Offset to) const;

  Offset Trace(Stream*, Offset, TraceSource*) const;

private:
  template <typename T>
  void WABT_VECTORCALL EmitAt(Offset, T val);
  template <typename T>
  void WABT_VECTORCALL EmitInternal(T val);

  template <typename T>
  T WABT_VECTORCALL ReadAt(Offset*) const;

  Buffer data_;
};

}  // namespace interp
}  // namespace wabt

#endif  // WABT_INTERP_ISTREAM_H_
