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

#ifndef WABT_OPCODE_H_
#define WABT_OPCODE_H_

#include "common.h"

namespace wabt {

/*
 *   tr: result type
 *   t1: type of the 1st parameter
 *   t2: type of the 2nd parameter
 *    m: memory size of the operation, if any
 * code: opcode
 * NAME: used to generate the opcode enum
 * text: a string of the opcode name in the AST format
 *
 *  tr  t1    t2   m  code  NAME text
 *  ============================ */
#define WABT_FOREACH_OPCODE(V)                                        \
  V(___, ___, ___, 0, 0x00, Unreachable, "unreachable")               \
  V(___, ___, ___, 0, 0x01, Nop, "nop")                               \
  V(___, ___, ___, 0, 0x02, Block, "block")                           \
  V(___, ___, ___, 0, 0x03, Loop, "loop")                             \
  V(___, ___, ___, 0, 0x04, If, "if")                                 \
  V(___, ___, ___, 0, 0x05, Else, "else")                             \
  V(___, ___, ___, 0, 0x06, Invalid_0x06, "<invalid:0x06>")           \
  V(___, ___, ___, 0, 0x07, Invalid_0x07, "<invalid:0x07>")           \
  V(___, ___, ___, 0, 0x08, Invalid_0x08, "<invalid:0x08>")           \
  V(___, ___, ___, 0, 0x09, Invalid_0x09, "<invalid:0x09>")           \
  V(___, ___, ___, 0, 0x0a, Invalid_0x0A, "<invalid:0x0a>")           \
  V(___, ___, ___, 0, 0x0b, End, "end")                               \
  V(___, ___, ___, 0, 0x0c, Br, "br")                                 \
  V(___, ___, ___, 0, 0x0d, BrIf, "br_if")                            \
  V(___, ___, ___, 0, 0x0e, BrTable, "br_table")                      \
  V(___, ___, ___, 0, 0x0f, Return, "return")                         \
  V(___, ___, ___, 0, 0x10, Call, "call")                             \
  V(___, ___, ___, 0, 0x11, CallIndirect, "call_indirect")            \
  V(___, ___, ___, 0, 0x12, Invalid_0x12, "<invalid:0x12>")           \
  V(___, ___, ___, 0, 0x13, Invalid_0x13, "<invalid:0x13>")           \
  V(___, ___, ___, 0, 0x14, Invalid_0x14, "<invalid:0x14>")           \
  V(___, ___, ___, 0, 0x15, Invalid_0x15, "<invalid:0x15>")           \
  V(___, ___, ___, 0, 0x16, Invalid_0x16, "<invalid:0x16>")           \
  V(___, ___, ___, 0, 0x17, Invalid_0x17, "<invalid:0x17>")           \
  V(___, ___, ___, 0, 0x18, Invalid_0x18, "<invalid:0x18>")           \
  V(___, ___, ___, 0, 0x19, Invalid_0x19, "<invalid:0x19>")           \
  V(___, ___, ___, 0, 0x1a, Drop, "drop")                             \
  V(___, ___, ___, 0, 0x1b, Select, "select")                         \
  V(___, ___, ___, 0, 0x1c, Invalid_0x1c, "<invalid:0x1c>")           \
  V(___, ___, ___, 0, 0x1d, Invalid_0x1d, "<invalid:0x1d>")           \
  V(___, ___, ___, 0, 0x1e, Invalid_0x1e, "<invalid:0x1e>")           \
  V(___, ___, ___, 0, 0x1f, Invalid_0x1f, "<invalid:0x1f>")           \
  V(___, ___, ___, 0, 0x20, GetLocal, "get_local")                    \
  V(___, ___, ___, 0, 0x21, SetLocal, "set_local")                    \
  V(___, ___, ___, 0, 0x22, TeeLocal, "tee_local")                    \
  V(___, ___, ___, 0, 0x23, GetGlobal, "get_global")                  \
  V(___, ___, ___, 0, 0x24, SetGlobal, "set_global")                  \
  V(___, ___, ___, 0, 0x25, Invalid_0x25, "<invalid:0x25>")           \
  V(___, ___, ___, 0, 0x26, Invalid_0x26, "<invalid:0x26>")           \
  V(___, ___, ___, 0, 0x27, Invalid_0x27, "<invalid:0x27>")           \
  V(I32, I32, ___, 4, 0x28, I32Load, "i32.load")                      \
  V(I64, I32, ___, 8, 0x29, I64Load, "i64.load")                      \
  V(F32, I32, ___, 4, 0x2a, F32Load, "f32.load")                      \
  V(F64, I32, ___, 8, 0x2b, F64Load, "f64.load")                      \
  V(I32, I32, ___, 1, 0x2c, I32Load8S, "i32.load8_s")                 \
  V(I32, I32, ___, 1, 0x2d, I32Load8U, "i32.load8_u")                 \
  V(I32, I32, ___, 2, 0x2e, I32Load16S, "i32.load16_s")               \
  V(I32, I32, ___, 2, 0x2f, I32Load16U, "i32.load16_u")               \
  V(I64, I32, ___, 1, 0x30, I64Load8S, "i64.load8_s")                 \
  V(I64, I32, ___, 1, 0x31, I64Load8U, "i64.load8_u")                 \
  V(I64, I32, ___, 2, 0x32, I64Load16S, "i64.load16_s")               \
  V(I64, I32, ___, 2, 0x33, I64Load16U, "i64.load16_u")               \
  V(I64, I32, ___, 4, 0x34, I64Load32S, "i64.load32_s")               \
  V(I64, I32, ___, 4, 0x35, I64Load32U, "i64.load32_u")               \
  V(___, I32, I32, 4, 0x36, I32Store, "i32.store")                    \
  V(___, I32, I64, 8, 0x37, I64Store, "i64.store")                    \
  V(___, I32, F32, 4, 0x38, F32Store, "f32.store")                    \
  V(___, I32, F64, 8, 0x39, F64Store, "f64.store")                    \
  V(___, I32, I32, 1, 0x3a, I32Store8, "i32.store8")                  \
  V(___, I32, I32, 2, 0x3b, I32Store16, "i32.store16")                \
  V(___, I32, I64, 1, 0x3c, I64Store8, "i64.store8")                  \
  V(___, I32, I64, 2, 0x3d, I64Store16, "i64.store16")                \
  V(___, I32, I64, 4, 0x3e, I64Store32, "i64.store32")                \
  V(I32, ___, ___, 0, 0x3f, CurrentMemory, "current_memory")          \
  V(I32, I32, ___, 0, 0x40, GrowMemory, "grow_memory")                \
  V(I32, ___, ___, 0, 0x41, I32Const, "i32.const")                    \
  V(I64, ___, ___, 0, 0x42, I64Const, "i64.const")                    \
  V(F32, ___, ___, 0, 0x43, F32Const, "f32.const")                    \
  V(F64, ___, ___, 0, 0x44, F64Const, "f64.const")                    \
  V(I32, I32, ___, 0, 0x45, I32Eqz, "i32.eqz")                        \
  V(I32, I32, I32, 0, 0x46, I32Eq, "i32.eq")                          \
  V(I32, I32, I32, 0, 0x47, I32Ne, "i32.ne")                          \
  V(I32, I32, I32, 0, 0x48, I32LtS, "i32.lt_s")                       \
  V(I32, I32, I32, 0, 0x49, I32LtU, "i32.lt_u")                       \
  V(I32, I32, I32, 0, 0x4a, I32GtS, "i32.gt_s")                       \
  V(I32, I32, I32, 0, 0x4b, I32GtU, "i32.gt_u")                       \
  V(I32, I32, I32, 0, 0x4c, I32LeS, "i32.le_s")                       \
  V(I32, I32, I32, 0, 0x4d, I32LeU, "i32.le_u")                       \
  V(I32, I32, I32, 0, 0x4e, I32GeS, "i32.ge_s")                       \
  V(I32, I32, I32, 0, 0x4f, I32GeU, "i32.ge_u")                       \
  V(I32, I64, ___, 0, 0x50, I64Eqz, "i64.eqz")                        \
  V(I32, I64, I64, 0, 0x51, I64Eq, "i64.eq")                          \
  V(I32, I64, I64, 0, 0x52, I64Ne, "i64.ne")                          \
  V(I32, I64, I64, 0, 0x53, I64LtS, "i64.lt_s")                       \
  V(I32, I64, I64, 0, 0x54, I64LtU, "i64.lt_u")                       \
  V(I32, I64, I64, 0, 0x55, I64GtS, "i64.gt_s")                       \
  V(I32, I64, I64, 0, 0x56, I64GtU, "i64.gt_u")                       \
  V(I32, I64, I64, 0, 0x57, I64LeS, "i64.le_s")                       \
  V(I32, I64, I64, 0, 0x58, I64LeU, "i64.le_u")                       \
  V(I32, I64, I64, 0, 0x59, I64GeS, "i64.ge_s")                       \
  V(I32, I64, I64, 0, 0x5a, I64GeU, "i64.ge_u")                       \
  V(I32, F32, F32, 0, 0x5b, F32Eq, "f32.eq")                          \
  V(I32, F32, F32, 0, 0x5c, F32Ne, "f32.ne")                          \
  V(I32, F32, F32, 0, 0x5d, F32Lt, "f32.lt")                          \
  V(I32, F32, F32, 0, 0x5e, F32Gt, "f32.gt")                          \
  V(I32, F32, F32, 0, 0x5f, F32Le, "f32.le")                          \
  V(I32, F32, F32, 0, 0x60, F32Ge, "f32.ge")                          \
  V(I32, F64, F64, 0, 0x61, F64Eq, "f64.eq")                          \
  V(I32, F64, F64, 0, 0x62, F64Ne, "f64.ne")                          \
  V(I32, F64, F64, 0, 0x63, F64Lt, "f64.lt")                          \
  V(I32, F64, F64, 0, 0x64, F64Gt, "f64.gt")                          \
  V(I32, F64, F64, 0, 0x65, F64Le, "f64.le")                          \
  V(I32, F64, F64, 0, 0x66, F64Ge, "f64.ge")                          \
  V(I32, I32, ___, 0, 0x67, I32Clz, "i32.clz")                        \
  V(I32, I32, ___, 0, 0x68, I32Ctz, "i32.ctz")                        \
  V(I32, I32, ___, 0, 0x69, I32Popcnt, "i32.popcnt")                  \
  V(I32, I32, I32, 0, 0x6a, I32Add, "i32.add")                        \
  V(I32, I32, I32, 0, 0x6b, I32Sub, "i32.sub")                        \
  V(I32, I32, I32, 0, 0x6c, I32Mul, "i32.mul")                        \
  V(I32, I32, I32, 0, 0x6d, I32DivS, "i32.div_s")                     \
  V(I32, I32, I32, 0, 0x6e, I32DivU, "i32.div_u")                     \
  V(I32, I32, I32, 0, 0x6f, I32RemS, "i32.rem_s")                     \
  V(I32, I32, I32, 0, 0x70, I32RemU, "i32.rem_u")                     \
  V(I32, I32, I32, 0, 0x71, I32And, "i32.and")                        \
  V(I32, I32, I32, 0, 0x72, I32Or, "i32.or")                          \
  V(I32, I32, I32, 0, 0x73, I32Xor, "i32.xor")                        \
  V(I32, I32, I32, 0, 0x74, I32Shl, "i32.shl")                        \
  V(I32, I32, I32, 0, 0x75, I32ShrS, "i32.shr_s")                     \
  V(I32, I32, I32, 0, 0x76, I32ShrU, "i32.shr_u")                     \
  V(I32, I32, I32, 0, 0x77, I32Rotl, "i32.rotl")                      \
  V(I32, I32, I32, 0, 0x78, I32Rotr, "i32.rotr")                      \
  V(I64, I64, ___, 0, 0x79, I64Clz, "i64.clz")                        \
  V(I64, I64, ___, 0, 0x7a, I64Ctz, "i64.ctz")                        \
  V(I64, I64, ___, 0, 0x7b, I64Popcnt, "i64.popcnt")                  \
  V(I64, I64, I64, 0, 0x7c, I64Add, "i64.add")                        \
  V(I64, I64, I64, 0, 0x7d, I64Sub, "i64.sub")                        \
  V(I64, I64, I64, 0, 0x7e, I64Mul, "i64.mul")                        \
  V(I64, I64, I64, 0, 0x7f, I64DivS, "i64.div_s")                     \
  V(I64, I64, I64, 0, 0x80, I64DivU, "i64.div_u")                     \
  V(I64, I64, I64, 0, 0x81, I64RemS, "i64.rem_s")                     \
  V(I64, I64, I64, 0, 0x82, I64RemU, "i64.rem_u")                     \
  V(I64, I64, I64, 0, 0x83, I64And, "i64.and")                        \
  V(I64, I64, I64, 0, 0x84, I64Or, "i64.or")                          \
  V(I64, I64, I64, 0, 0x85, I64Xor, "i64.xor")                        \
  V(I64, I64, I64, 0, 0x86, I64Shl, "i64.shl")                        \
  V(I64, I64, I64, 0, 0x87, I64ShrS, "i64.shr_s")                     \
  V(I64, I64, I64, 0, 0x88, I64ShrU, "i64.shr_u")                     \
  V(I64, I64, I64, 0, 0x89, I64Rotl, "i64.rotl")                      \
  V(I64, I64, I64, 0, 0x8a, I64Rotr, "i64.rotr")                      \
  V(F32, F32, F32, 0, 0x8b, F32Abs, "f32.abs")                        \
  V(F32, F32, F32, 0, 0x8c, F32Neg, "f32.neg")                        \
  V(F32, F32, F32, 0, 0x8d, F32Ceil, "f32.ceil")                      \
  V(F32, F32, F32, 0, 0x8e, F32Floor, "f32.floor")                    \
  V(F32, F32, F32, 0, 0x8f, F32Trunc, "f32.trunc")                    \
  V(F32, F32, F32, 0, 0x90, F32Nearest, "f32.nearest")                \
  V(F32, F32, F32, 0, 0x91, F32Sqrt, "f32.sqrt")                      \
  V(F32, F32, F32, 0, 0x92, F32Add, "f32.add")                        \
  V(F32, F32, F32, 0, 0x93, F32Sub, "f32.sub")                        \
  V(F32, F32, F32, 0, 0x94, F32Mul, "f32.mul")                        \
  V(F32, F32, F32, 0, 0x95, F32Div, "f32.div")                        \
  V(F32, F32, F32, 0, 0x96, F32Min, "f32.min")                        \
  V(F32, F32, F32, 0, 0x97, F32Max, "f32.max")                        \
  V(F32, F32, F32, 0, 0x98, F32Copysign, "f32.copysign")              \
  V(F64, F64, F64, 0, 0x99, F64Abs, "f64.abs")                        \
  V(F64, F64, F64, 0, 0x9a, F64Neg, "f64.neg")                        \
  V(F64, F64, F64, 0, 0x9b, F64Ceil, "f64.ceil")                      \
  V(F64, F64, F64, 0, 0x9c, F64Floor, "f64.floor")                    \
  V(F64, F64, F64, 0, 0x9d, F64Trunc, "f64.trunc")                    \
  V(F64, F64, F64, 0, 0x9e, F64Nearest, "f64.nearest")                \
  V(F64, F64, F64, 0, 0x9f, F64Sqrt, "f64.sqrt")                      \
  V(F64, F64, F64, 0, 0xa0, F64Add, "f64.add")                        \
  V(F64, F64, F64, 0, 0xa1, F64Sub, "f64.sub")                        \
  V(F64, F64, F64, 0, 0xa2, F64Mul, "f64.mul")                        \
  V(F64, F64, F64, 0, 0xa3, F64Div, "f64.div")                        \
  V(F64, F64, F64, 0, 0xa4, F64Min, "f64.min")                        \
  V(F64, F64, F64, 0, 0xa5, F64Max, "f64.max")                        \
  V(F64, F64, F64, 0, 0xa6, F64Copysign, "f64.copysign")              \
  V(I32, I64, ___, 0, 0xa7, I32WrapI64, "i32.wrap/i64")               \
  V(I32, F32, ___, 0, 0xa8, I32TruncSF32, "i32.trunc_s/f32")          \
  V(I32, F32, ___, 0, 0xa9, I32TruncUF32, "i32.trunc_u/f32")          \
  V(I32, F64, ___, 0, 0xaa, I32TruncSF64, "i32.trunc_s/f64")          \
  V(I32, F64, ___, 0, 0xab, I32TruncUF64, "i32.trunc_u/f64")          \
  V(I64, I32, ___, 0, 0xac, I64ExtendSI32, "i64.extend_s/i32")        \
  V(I64, I32, ___, 0, 0xad, I64ExtendUI32, "i64.extend_u/i32")        \
  V(I64, F32, ___, 0, 0xae, I64TruncSF32, "i64.trunc_s/f32")          \
  V(I64, F32, ___, 0, 0xaf, I64TruncUF32, "i64.trunc_u/f32")          \
  V(I64, F64, ___, 0, 0xb0, I64TruncSF64, "i64.trunc_s/f64")          \
  V(I64, F64, ___, 0, 0xb1, I64TruncUF64, "i64.trunc_u/f64")          \
  V(F32, I32, ___, 0, 0xb2, F32ConvertSI32, "f32.convert_s/i32")      \
  V(F32, I32, ___, 0, 0xb3, F32ConvertUI32, "f32.convert_u/i32")      \
  V(F32, I64, ___, 0, 0xb4, F32ConvertSI64, "f32.convert_s/i64")      \
  V(F32, I64, ___, 0, 0xb5, F32ConvertUI64, "f32.convert_u/i64")      \
  V(F32, F64, ___, 0, 0xb6, F32DemoteF64, "f32.demote/f64")           \
  V(F64, I32, ___, 0, 0xb7, F64ConvertSI32, "f64.convert_s/i32")      \
  V(F64, I32, ___, 0, 0xb8, F64ConvertUI32, "f64.convert_u/i32")      \
  V(F64, I64, ___, 0, 0xb9, F64ConvertSI64, "f64.convert_s/i64")      \
  V(F64, I64, ___, 0, 0xba, F64ConvertUI64, "f64.convert_u/i64")      \
  V(F64, F32, ___, 0, 0xbb, F64PromoteF32, "f64.promote/f32")         \
  V(I32, F32, ___, 0, 0xbc, I32ReinterpretF32, "i32.reinterpret/f32") \
  V(I64, F64, ___, 0, 0xbd, I64ReinterpretF64, "i64.reinterpret/f64") \
  V(F32, I32, ___, 0, 0xbe, F32ReinterpretI32, "f32.reinterpret/i32") \
  V(F64, I64, ___, 0, 0xbf, F64ReinterpretI64, "f64.reinterpret/i64")

enum class Opcode {
#define V(rtype, type1, type2, mem_size, code, Name, text) Name = code,
  WABT_FOREACH_OPCODE(V)
#undef V

  First = Unreachable,
  Last = F64ReinterpretI64,
};
static const int kOpcodeCount = WABT_ENUM_COUNT(Opcode);

struct OpcodeInfo {
  const char* name;
  Type result_type;
  Type param1_type;
  Type param2_type;
  int memory_size;
};

// Return 1 if |alignment| matches the alignment of |opcode|, or if |alignment|
// is WABT_USE_NATURAL_ALIGNMENT.
bool is_naturally_aligned(Opcode opcode, uint32_t alignment);

// If |alignment| is WABT_USE_NATURAL_ALIGNMENT, return the alignment of
// |opcode|, else return |alignment|.
uint32_t get_opcode_alignment(Opcode opcode, uint32_t alignment);

extern OpcodeInfo g_opcode_info[];

inline const char* get_opcode_name(Opcode opcode) {
  assert(static_cast<int>(opcode) < kOpcodeCount);
  return g_opcode_info[static_cast<size_t>(opcode)].name;
}

inline Type get_opcode_result_type(Opcode opcode) {
  assert(static_cast<int>(opcode) < kOpcodeCount);
  return g_opcode_info[static_cast<size_t>(opcode)].result_type;
}

inline Type get_opcode_param_type_1(Opcode opcode) {
  assert(static_cast<int>(opcode) < kOpcodeCount);
  return g_opcode_info[static_cast<size_t>(opcode)].param1_type;
}

inline Type get_opcode_param_type_2(Opcode opcode) {
  assert(static_cast<int>(opcode) < kOpcodeCount);
  return g_opcode_info[static_cast<size_t>(opcode)].param2_type;
}

inline int get_opcode_memory_size(Opcode opcode) {
  assert(static_cast<int>(opcode) < kOpcodeCount);
  return g_opcode_info[static_cast<size_t>(opcode)].memory_size;
}

}  // namespace

#endif  // WABT_OPCODE_H_
