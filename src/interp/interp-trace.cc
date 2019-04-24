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

#include "src/interp/interp.h"

#include <cinttypes>

#include "src/interp/interp-internal.h"

namespace wabt {
namespace interp {

void Thread::Trace(Stream* stream) {
  const uint8_t* istream = GetIstream();
  const uint8_t* pc = &istream[pc_];

  stream->Writef("#%u. %4" PRIzd ": V:%-3u| ", call_stack_top_, pc - istream,
                 value_stack_top_);

  Opcode opcode = ReadOpcode(&pc);
  assert(!opcode.IsInvalid());
  switch (opcode) {
    case Opcode::Select:
      // TODO(binji): We don't know the type here so we can't display the value
      // to the user. This used to display the full 64-bit value, but that
      // will potentially display garbage if the value is 32-bit.
      stream->Writef("%s %u, %%[-2], %%[-1]\n", opcode.GetName(), Pick(3).i32);
      break;

    case Opcode::Br:
      stream->Writef("%s @%u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::BrIf:
      stream->Writef("%s @%u, %u\n", opcode.GetName(), ReadU32At(pc),
                     Top().i32);
      break;

    case Opcode::BrTable: {
      const Index num_targets = ReadU32At(pc);
      const IstreamOffset table_offset = ReadU32At(pc + 4);
      const uint32_t key = Top().i32;
      stream->Writef("%s %u, $#%" PRIindex ", table:$%u\n", opcode.GetName(),
                     key, num_targets, table_offset);
      break;
    }

    case Opcode::Nop:
    case Opcode::Return:
    case Opcode::Unreachable:
    case Opcode::Drop:
      stream->Writef("%s\n", opcode.GetName());
      break;

    case Opcode::MemorySize: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex "\n", opcode.GetName(), memory_index);
      break;
    }

    case Opcode::I32Const:
      stream->Writef("%s %u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::I64Const:
      stream->Writef("%s %" PRIu64 "\n", opcode.GetName(), ReadU64At(pc));
      break;

    case Opcode::F32Const:
      stream->Writef("%s %g\n", opcode.GetName(),
                     Bitcast<float>(ReadU32At(pc)));
      break;

    case Opcode::F64Const:
      stream->Writef("%s %g\n", opcode.GetName(),
                     Bitcast<double>(ReadU64At(pc)));
      break;

    case Opcode::LocalGet:
    case Opcode::GlobalGet:
      stream->Writef("%s $%u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::LocalSet:
    case Opcode::GlobalSet:
    case Opcode::LocalTee:
      stream->Writef("%s $%u, %u\n", opcode.GetName(), ReadU32At(pc),
                     Top().i32);
      break;

    case Opcode::Call:
    case Opcode::ReturnCall:
      stream->Writef("%s @%u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::CallIndirect:
    case Opcode::ReturnCallIndirect:
      stream->Writef("%s $%u, %u\n", opcode.GetName(), ReadU32At(pc),
                     Top().i32);
      break;

    case Opcode::InterpCallHost:
      stream->Writef("%s $%u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::I32AtomicLoad8U:
    case Opcode::I32AtomicLoad16U:
    case Opcode::I32AtomicLoad:
    case Opcode::I64AtomicLoad8U:
    case Opcode::I64AtomicLoad16U:
    case Opcode::I64AtomicLoad32U:
    case Opcode::I64AtomicLoad:
    case Opcode::I32Load8S:
    case Opcode::I32Load8U:
    case Opcode::I32Load16S:
    case Opcode::I32Load16U:
    case Opcode::I64Load8S:
    case Opcode::I64Load8U:
    case Opcode::I64Load16S:
    case Opcode::I64Load16U:
    case Opcode::I64Load32S:
    case Opcode::I64Load32U:
    case Opcode::I32Load:
    case Opcode::I64Load:
    case Opcode::F32Load:
    case Opcode::F64Load:
    case Opcode::V128Load: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u\n", opcode.GetName(),
                     memory_index, Top().i32, ReadU32At(pc));
      break;
    }

    case Opcode::AtomicNotify:
    case Opcode::I32AtomicStore:
    case Opcode::I32AtomicStore8:
    case Opcode::I32AtomicStore16:
    case Opcode::I32AtomicRmw8AddU:
    case Opcode::I32AtomicRmw16AddU:
    case Opcode::I32AtomicRmwAdd:
    case Opcode::I32AtomicRmw8SubU:
    case Opcode::I32AtomicRmw16SubU:
    case Opcode::I32AtomicRmwSub:
    case Opcode::I32AtomicRmw8AndU:
    case Opcode::I32AtomicRmw16AndU:
    case Opcode::I32AtomicRmwAnd:
    case Opcode::I32AtomicRmw8OrU:
    case Opcode::I32AtomicRmw16OrU:
    case Opcode::I32AtomicRmwOr:
    case Opcode::I32AtomicRmw8XorU:
    case Opcode::I32AtomicRmw16XorU:
    case Opcode::I32AtomicRmwXor:
    case Opcode::I32AtomicRmw8XchgU:
    case Opcode::I32AtomicRmw16XchgU:
    case Opcode::I32AtomicRmwXchg:
    case Opcode::I32Store8:
    case Opcode::I32Store16:
    case Opcode::I32Store: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %u\n", opcode.GetName(),
                     memory_index, Pick(2).i32, ReadU32At(pc), Pick(1).i32);
      break;
    }

    case Opcode::I32AtomicRmwCmpxchg:
    case Opcode::I32AtomicRmw8CmpxchgU:
    case Opcode::I32AtomicRmw16CmpxchgU: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %u, %u\n", opcode.GetName(),
                     memory_index, Pick(3).i32, ReadU32At(pc), Pick(2).i32,
                     Pick(1).i32);
      break;
    }

    case Opcode::I64AtomicStore8:
    case Opcode::I64AtomicStore16:
    case Opcode::I64AtomicStore32:
    case Opcode::I64AtomicStore:
    case Opcode::I64AtomicRmw8AddU:
    case Opcode::I64AtomicRmw16AddU:
    case Opcode::I64AtomicRmw32AddU:
    case Opcode::I64AtomicRmwAdd:
    case Opcode::I64AtomicRmw8SubU:
    case Opcode::I64AtomicRmw16SubU:
    case Opcode::I64AtomicRmw32SubU:
    case Opcode::I64AtomicRmwSub:
    case Opcode::I64AtomicRmw8AndU:
    case Opcode::I64AtomicRmw16AndU:
    case Opcode::I64AtomicRmw32AndU:
    case Opcode::I64AtomicRmwAnd:
    case Opcode::I64AtomicRmw8OrU:
    case Opcode::I64AtomicRmw16OrU:
    case Opcode::I64AtomicRmw32OrU:
    case Opcode::I64AtomicRmwOr:
    case Opcode::I64AtomicRmw8XorU:
    case Opcode::I64AtomicRmw16XorU:
    case Opcode::I64AtomicRmw32XorU:
    case Opcode::I64AtomicRmwXor:
    case Opcode::I64AtomicRmw8XchgU:
    case Opcode::I64AtomicRmw16XchgU:
    case Opcode::I64AtomicRmw32XchgU:
    case Opcode::I64AtomicRmwXchg:
    case Opcode::I64Store8:
    case Opcode::I64Store16:
    case Opcode::I64Store32:
    case Opcode::I64Store: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %" PRIu64 "\n",
                     opcode.GetName(), memory_index, Pick(2).i32, ReadU32At(pc),
                     Pick(1).i64);
      break;
    }

    case Opcode::I32AtomicWait: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %u, %" PRIu64 "\n",
                     opcode.GetName(), memory_index, Pick(3).i32, ReadU32At(pc),
                     Pick(2).i32, Pick(1).i64);
      break;
    }

    case Opcode::I64AtomicWait:
    case Opcode::I64AtomicRmwCmpxchg:
    case Opcode::I64AtomicRmw8CmpxchgU:
    case Opcode::I64AtomicRmw16CmpxchgU:
    case Opcode::I64AtomicRmw32CmpxchgU: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %" PRIu64 ", %" PRIu64 "\n",
                     opcode.GetName(), memory_index, Pick(3).i32, ReadU32At(pc),
                     Pick(2).i64, Pick(1).i64);
      break;
    }

    case Opcode::F32Store: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %g\n", opcode.GetName(),
                     memory_index, Pick(2).i32, ReadU32At(pc),
                     Bitcast<float>(Pick(1).f32_bits));
      break;
    }

    case Opcode::F64Store: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, %g\n", opcode.GetName(),
                     memory_index, Pick(2).i32, ReadU32At(pc),
                     Bitcast<double>(Pick(1).f64_bits));
      break;
    }

    case Opcode::V128Store: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u+$%u, $0x%08x 0x%08x 0x%08x 0x%08x\n",
                     opcode.GetName(), memory_index, Pick(2).i32, ReadU32At(pc),
                     Pick(1).v128_bits.v[0], Pick(1).v128_bits.v[1],
                     Pick(1).v128_bits.v[2], Pick(1).v128_bits.v[3]);
      break;
    }

    case Opcode::MemoryGrow: {
      const Index memory_index = ReadU32(&pc);
      stream->Writef("%s $%" PRIindex ":%u\n", opcode.GetName(), memory_index,
                     Top().i32);
      break;
    }

    case Opcode::I32Add:
    case Opcode::I32Sub:
    case Opcode::I32Mul:
    case Opcode::I32DivS:
    case Opcode::I32DivU:
    case Opcode::I32RemS:
    case Opcode::I32RemU:
    case Opcode::I32And:
    case Opcode::I32Or:
    case Opcode::I32Xor:
    case Opcode::I32Shl:
    case Opcode::I32ShrU:
    case Opcode::I32ShrS:
    case Opcode::I32Eq:
    case Opcode::I32Ne:
    case Opcode::I32LtS:
    case Opcode::I32LeS:
    case Opcode::I32LtU:
    case Opcode::I32LeU:
    case Opcode::I32GtS:
    case Opcode::I32GeS:
    case Opcode::I32GtU:
    case Opcode::I32GeU:
    case Opcode::I32Rotr:
    case Opcode::I32Rotl:
      stream->Writef("%s %u, %u\n", opcode.GetName(), Pick(2).i32, Pick(1).i32);
      break;

    case Opcode::I32Clz:
    case Opcode::I32Ctz:
    case Opcode::I32Popcnt:
    case Opcode::I32Eqz:
    case Opcode::I32Extend16S:
    case Opcode::I32Extend8S:
    case Opcode::I8X16Splat:
    case Opcode::I16X8Splat:
    case Opcode::I32X4Splat:
      stream->Writef("%s %u\n", opcode.GetName(), Top().i32);
      break;

    case Opcode::I64Add:
    case Opcode::I64Sub:
    case Opcode::I64Mul:
    case Opcode::I64DivS:
    case Opcode::I64DivU:
    case Opcode::I64RemS:
    case Opcode::I64RemU:
    case Opcode::I64And:
    case Opcode::I64Or:
    case Opcode::I64Xor:
    case Opcode::I64Shl:
    case Opcode::I64ShrU:
    case Opcode::I64ShrS:
    case Opcode::I64Eq:
    case Opcode::I64Ne:
    case Opcode::I64LtS:
    case Opcode::I64LeS:
    case Opcode::I64LtU:
    case Opcode::I64LeU:
    case Opcode::I64GtS:
    case Opcode::I64GeS:
    case Opcode::I64GtU:
    case Opcode::I64GeU:
    case Opcode::I64Rotr:
    case Opcode::I64Rotl:
      stream->Writef("%s %" PRIu64 ", %" PRIu64 "\n", opcode.GetName(),
                     Pick(2).i64, Pick(1).i64);
      break;

    case Opcode::I64Clz:
    case Opcode::I64Ctz:
    case Opcode::I64Popcnt:
    case Opcode::I64Eqz:
    case Opcode::I64Extend16S:
    case Opcode::I64Extend32S:
    case Opcode::I64Extend8S:
    case Opcode::I64X2Splat:
      stream->Writef("%s %" PRIu64 "\n", opcode.GetName(), Top().i64);
      break;

    case Opcode::F32Add:
    case Opcode::F32Sub:
    case Opcode::F32Mul:
    case Opcode::F32Div:
    case Opcode::F32Min:
    case Opcode::F32Max:
    case Opcode::F32Copysign:
    case Opcode::F32Eq:
    case Opcode::F32Ne:
    case Opcode::F32Lt:
    case Opcode::F32Le:
    case Opcode::F32Gt:
    case Opcode::F32Ge:
      stream->Writef("%s %g, %g\n", opcode.GetName(),
                     Bitcast<float>(Pick(2).i32), Bitcast<float>(Pick(1).i32));
      break;

    case Opcode::F32Abs:
    case Opcode::F32Neg:
    case Opcode::F32Ceil:
    case Opcode::F32Floor:
    case Opcode::F32Trunc:
    case Opcode::F32Nearest:
    case Opcode::F32Sqrt:
    case Opcode::F32X4Splat:
      stream->Writef("%s %g\n", opcode.GetName(), Bitcast<float>(Top().i32));
      break;

    case Opcode::F64Add:
    case Opcode::F64Sub:
    case Opcode::F64Mul:
    case Opcode::F64Div:
    case Opcode::F64Min:
    case Opcode::F64Max:
    case Opcode::F64Copysign:
    case Opcode::F64Eq:
    case Opcode::F64Ne:
    case Opcode::F64Lt:
    case Opcode::F64Le:
    case Opcode::F64Gt:
    case Opcode::F64Ge:
      stream->Writef("%s %g, %g\n", opcode.GetName(),
                     Bitcast<double>(Pick(2).i64),
                     Bitcast<double>(Pick(1).i64));
      break;

    case Opcode::F64Abs:
    case Opcode::F64Neg:
    case Opcode::F64Ceil:
    case Opcode::F64Floor:
    case Opcode::F64Trunc:
    case Opcode::F64Nearest:
    case Opcode::F64Sqrt:
    case Opcode::F64X2Splat:
      stream->Writef("%s %g\n", opcode.GetName(), Bitcast<double>(Top().i64));
      break;

    case Opcode::I32TruncF32S:
    case Opcode::I32TruncF32U:
    case Opcode::I64TruncF32S:
    case Opcode::I64TruncF32U:
    case Opcode::F64PromoteF32:
    case Opcode::I32ReinterpretF32:
    case Opcode::I32TruncSatF32S:
    case Opcode::I32TruncSatF32U:
    case Opcode::I64TruncSatF32S:
    case Opcode::I64TruncSatF32U:
      stream->Writef("%s %g\n", opcode.GetName(), Bitcast<float>(Top().i32));
      break;

    case Opcode::I32TruncF64S:
    case Opcode::I32TruncF64U:
    case Opcode::I64TruncF64S:
    case Opcode::I64TruncF64U:
    case Opcode::F32DemoteF64:
    case Opcode::I64ReinterpretF64:
    case Opcode::I32TruncSatF64S:
    case Opcode::I32TruncSatF64U:
    case Opcode::I64TruncSatF64S:
    case Opcode::I64TruncSatF64U:
      stream->Writef("%s %g\n", opcode.GetName(), Bitcast<double>(Top().i64));
      break;

    case Opcode::I32WrapI64:
    case Opcode::F32ConvertI64S:
    case Opcode::F32ConvertI64U:
    case Opcode::F64ConvertI64S:
    case Opcode::F64ConvertI64U:
    case Opcode::F64ReinterpretI64:
      stream->Writef("%s %" PRIu64 "\n", opcode.GetName(), Top().i64);
      break;

    case Opcode::I64ExtendI32S:
    case Opcode::I64ExtendI32U:
    case Opcode::F32ConvertI32S:
    case Opcode::F32ConvertI32U:
    case Opcode::F32ReinterpretI32:
    case Opcode::F64ConvertI32S:
    case Opcode::F64ConvertI32U:
      stream->Writef("%s %u\n", opcode.GetName(), Top().i32);
      break;

    case Opcode::InterpAlloca:
      stream->Writef("%s $%u\n", opcode.GetName(), ReadU32At(pc));
      break;

    case Opcode::InterpBrUnless:
      stream->Writef("%s @%u, %u\n", opcode.GetName(), ReadU32At(pc),
                     Top().i32);
      break;

    case Opcode::InterpDropKeep:
      stream->Writef("%s $%u $%u\n", opcode.GetName(), ReadU32At(pc),
                     ReadU32At(pc + 4));
      break;

    case Opcode::V128Const: {
      stream->Writef("%s i32x4 0x%08x 0x%08x 0x%08x 0x%08x\n", opcode.GetName(),
                     ReadU32At(pc), ReadU32At(pc + 4), ReadU32At(pc + 8),
                     ReadU32At(pc + 12));
      break;
    }

    case Opcode::I8X16Neg:
    case Opcode::I16X8Neg:
    case Opcode::I32X4Neg:
    case Opcode::I64X2Neg:
    case Opcode::V128Not:
    case Opcode::I8X16AnyTrue:
    case Opcode::I16X8AnyTrue:
    case Opcode::I32X4AnyTrue:
    case Opcode::I64X2AnyTrue:
    case Opcode::I8X16AllTrue:
    case Opcode::I16X8AllTrue:
    case Opcode::I32X4AllTrue:
    case Opcode::I64X2AllTrue:
    case Opcode::F32X4Neg:
    case Opcode::F64X2Neg:
    case Opcode::F32X4Abs:
    case Opcode::F64X2Abs:
    case Opcode::F32X4Sqrt:
    case Opcode::F64X2Sqrt:
    case Opcode::F32X4ConvertI32X4S:
    case Opcode::F32X4ConvertI32X4U:
    case Opcode::F64X2ConvertI64X2S:
    case Opcode::F64X2ConvertI64X2U:
    case Opcode::I32X4TruncSatF32X4S:
    case Opcode::I32X4TruncSatF32X4U:
    case Opcode::I64X2TruncSatF64X2S:
    case Opcode::I64X2TruncSatF64X2U: {
      stream->Writef("%s $0x%08x 0x%08x 0x%08x 0x%08x\n", opcode.GetName(),
                     Top().v128_bits.v[0], Top().v128_bits.v[1],
                     Top().v128_bits.v[2], Top().v128_bits.v[3]);
      break;
    }

    case Opcode::V128BitSelect:
      stream->Writef(
          "%s $0x%08x %08x %08x %08x $0x%08x %08x %08x %08x $0x%08x %08x %08x "
          "%08x\n",
          opcode.GetName(), Pick(3).v128_bits.v[0], Pick(3).v128_bits.v[1],
          Pick(3).v128_bits.v[2], Pick(3).v128_bits.v[3],
          Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
          Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3],
          Pick(1).v128_bits.v[0], Pick(1).v128_bits.v[1],
          Pick(1).v128_bits.v[2], Pick(1).v128_bits.v[3]);
      break;

    case Opcode::I8X16ExtractLaneS:
    case Opcode::I8X16ExtractLaneU:
    case Opcode::I16X8ExtractLaneS:
    case Opcode::I16X8ExtractLaneU:
    case Opcode::I32X4ExtractLane:
    case Opcode::I64X2ExtractLane:
    case Opcode::F32X4ExtractLane:
    case Opcode::F64X2ExtractLane: {
      stream->Writef("%s : LaneIdx %d From $0x%08x 0x%08x 0x%08x 0x%08x\n",
                     opcode.GetName(), ReadU8At(pc), Top().v128_bits.v[0],
                     Top().v128_bits.v[1], Top().v128_bits.v[2],
                     Top().v128_bits.v[3]);
      break;
    }

    case Opcode::I8X16ReplaceLane:
    case Opcode::I16X8ReplaceLane:
    case Opcode::I32X4ReplaceLane: {
      stream->Writef(
          "%s : Set %u to LaneIdx %d In $0x%08x 0x%08x 0x%08x 0x%08x\n",
          opcode.GetName(), Pick(1).i32, ReadU8At(pc), Pick(2).v128_bits.v[0],
          Pick(2).v128_bits.v[1], Pick(2).v128_bits.v[2],
          Pick(2).v128_bits.v[3]);
      break;
    }
    case Opcode::I64X2ReplaceLane: {
      stream->Writef("%s : Set %" PRIu64
                     " to LaneIdx %d In $0x%08x 0x%08x 0x%08x 0x%08x\n",
                     opcode.GetName(), Pick(1).i64, ReadU8At(pc),
                     Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
                     Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3]);
      break;
    }
    case Opcode::F32X4ReplaceLane: {
      stream->Writef(
          "%s : Set %g to LaneIdx %d In $0x%08x 0x%08x 0x%08x 0x%08x\n",
          opcode.GetName(), Bitcast<float>(Pick(1).f32_bits), ReadU8At(pc),
          Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
          Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3]);

      break;
    }
    case Opcode::F64X2ReplaceLane: {
      stream->Writef(
          "%s : Set %g to LaneIdx %d In $0x%08x 0x%08x 0x%08x 0x%08x\n",
          opcode.GetName(), Bitcast<double>(Pick(1).f64_bits), ReadU8At(pc),
          Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
          Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3]);
      break;
    }

    case Opcode::V8X16Shuffle:
      stream->Writef(
          "%s $0x%08x %08x %08x %08x $0x%08x %08x %08x %08x : with lane imm: "
          "$0x%08x %08x %08x %08x\n",
          opcode.GetName(), Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
          Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3],
          Pick(1).v128_bits.v[0], Pick(1).v128_bits.v[1],
          Pick(1).v128_bits.v[2], Pick(1).v128_bits.v[3], ReadU32At(pc),
          ReadU32At(pc + 4), ReadU32At(pc + 8), ReadU32At(pc + 12));
      break;

    case Opcode::I8X16Add:
    case Opcode::I16X8Add:
    case Opcode::I32X4Add:
    case Opcode::I64X2Add:
    case Opcode::I8X16Sub:
    case Opcode::I16X8Sub:
    case Opcode::I32X4Sub:
    case Opcode::I64X2Sub:
    case Opcode::I8X16Mul:
    case Opcode::I16X8Mul:
    case Opcode::I32X4Mul:
    case Opcode::I8X16AddSaturateS:
    case Opcode::I8X16AddSaturateU:
    case Opcode::I16X8AddSaturateS:
    case Opcode::I16X8AddSaturateU:
    case Opcode::I8X16SubSaturateS:
    case Opcode::I8X16SubSaturateU:
    case Opcode::I16X8SubSaturateS:
    case Opcode::I16X8SubSaturateU:
    case Opcode::V128And:
    case Opcode::V128Or:
    case Opcode::V128Xor:
    case Opcode::I8X16Eq:
    case Opcode::I16X8Eq:
    case Opcode::I32X4Eq:
    case Opcode::F32X4Eq:
    case Opcode::F64X2Eq:
    case Opcode::I8X16Ne:
    case Opcode::I16X8Ne:
    case Opcode::I32X4Ne:
    case Opcode::F32X4Ne:
    case Opcode::F64X2Ne:
    case Opcode::I8X16LtS:
    case Opcode::I8X16LtU:
    case Opcode::I16X8LtS:
    case Opcode::I16X8LtU:
    case Opcode::I32X4LtS:
    case Opcode::I32X4LtU:
    case Opcode::F32X4Lt:
    case Opcode::F64X2Lt:
    case Opcode::I8X16LeS:
    case Opcode::I8X16LeU:
    case Opcode::I16X8LeS:
    case Opcode::I16X8LeU:
    case Opcode::I32X4LeS:
    case Opcode::I32X4LeU:
    case Opcode::F32X4Le:
    case Opcode::F64X2Le:
    case Opcode::I8X16GtS:
    case Opcode::I8X16GtU:
    case Opcode::I16X8GtS:
    case Opcode::I16X8GtU:
    case Opcode::I32X4GtS:
    case Opcode::I32X4GtU:
    case Opcode::F32X4Gt:
    case Opcode::F64X2Gt:
    case Opcode::I8X16GeS:
    case Opcode::I8X16GeU:
    case Opcode::I16X8GeS:
    case Opcode::I16X8GeU:
    case Opcode::I32X4GeS:
    case Opcode::I32X4GeU:
    case Opcode::F32X4Ge:
    case Opcode::F64X2Ge:
    case Opcode::F32X4Min:
    case Opcode::F64X2Min:
    case Opcode::F32X4Max:
    case Opcode::F64X2Max:
    case Opcode::F32X4Add:
    case Opcode::F64X2Add:
    case Opcode::F32X4Sub:
    case Opcode::F64X2Sub:
    case Opcode::F32X4Div:
    case Opcode::F64X2Div:
    case Opcode::F32X4Mul:
    case Opcode::F64X2Mul: {
      stream->Writef("%s $0x%08x %08x %08x %08x  $0x%08x %08x %08x %08x\n",
                     opcode.GetName(), Pick(2).v128_bits.v[0],
                     Pick(2).v128_bits.v[1], Pick(2).v128_bits.v[2],
                     Pick(2).v128_bits.v[3], Pick(1).v128_bits.v[0],
                     Pick(1).v128_bits.v[1], Pick(1).v128_bits.v[2],
                     Pick(1).v128_bits.v[3]);
      break;
    }

    case Opcode::I8X16Shl:
    case Opcode::I16X8Shl:
    case Opcode::I32X4Shl:
    case Opcode::I64X2Shl:
    case Opcode::I8X16ShrS:
    case Opcode::I8X16ShrU:
    case Opcode::I16X8ShrS:
    case Opcode::I16X8ShrU:
    case Opcode::I32X4ShrS:
    case Opcode::I32X4ShrU:
    case Opcode::I64X2ShrS:
    case Opcode::I64X2ShrU: {
      stream->Writef("%s $0x%08x %08x %08x %08x  $0x%08x\n", opcode.GetName(),
                     Pick(2).v128_bits.v[0], Pick(2).v128_bits.v[1],
                     Pick(2).v128_bits.v[2], Pick(2).v128_bits.v[3],
                     Pick(1).i32);
      break;
    }

    case Opcode::TableGet:
    case Opcode::TableSet:
    case Opcode::TableGrow:
    case Opcode::TableSize:
    case Opcode::RefNull:
    case Opcode::RefIsNull:
    case Opcode::RefFunc:
      WABT_UNREACHABLE;
      break;

    case Opcode::MemoryInit:
      WABT_UNREACHABLE;
      break;

    case Opcode::DataDrop:
      WABT_UNREACHABLE;
      break;

    case Opcode::MemoryCopy:
      WABT_UNREACHABLE;
      break;

    case Opcode::MemoryFill:
      WABT_UNREACHABLE;
      break;

    case Opcode::TableInit:
      WABT_UNREACHABLE;
      break;

    case Opcode::ElemDrop:
      WABT_UNREACHABLE;
      break;

    case Opcode::TableCopy:
      WABT_UNREACHABLE;
      break;

    // The following opcodes are either never generated or should never be
    // executed.
    case Opcode::Block:
    case Opcode::BrOnExn:
    case Opcode::Catch:
    case Opcode::Else:
    case Opcode::End:
    case Opcode::If:
    case Opcode::InterpData:
    case Opcode::Invalid:
    case Opcode::Loop:
    case Opcode::Rethrow:
    case Opcode::Throw:
    case Opcode::Try:
      WABT_UNREACHABLE;
      break;
  }
}

}  // namespace interp
}  // namespace wabt
