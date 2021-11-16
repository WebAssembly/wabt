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

#include "src/interp/interp-util.h"

#include <cinttypes>

#include "src/stream.h"

namespace wabt {
namespace interp {

std::string TypedValueToString(const TypedValue& tv) {
  switch (tv.type) {
    case Type::I32:
      return StringPrintf("i32:%u", tv.value.Get<s32>());

    case Type::I64:
      return StringPrintf("i64:%" PRIu64, tv.value.Get<s64>());

    case Type::F32:
      return StringPrintf("f32:%f", tv.value.Get<f32>());

    case Type::F64:
      return StringPrintf("f64:%f", tv.value.Get<f64>());

    case Type::V128: {
      v128 simd = tv.value.Get<v128>();
      return StringPrintf("v128 i32x4:0x%08x 0x%08x 0x%08x 0x%08x", simd.u32(0),
                          simd.u32(1), simd.u32(2), simd.u32(3));
    }

    case Type::I8:  // For SIMD lane.
      return StringPrintf("i8:%u", tv.value.Get<u32>() & 0xff);

    case Type::I16:  // For SIMD lane.
      return StringPrintf("i16:%u", tv.value.Get<u32>() & 0xffff);

    case Type::FuncRef:
      return StringPrintf("funcref:%" PRIzd, tv.value.Get<Ref>().index);

    case Type::ExternRef:
      return StringPrintf("externref:%" PRIzd, tv.value.Get<Ref>().index);

    case Type::Reference:
    case Type::Func:
    case Type::Struct:
    case Type::Array:
    case Type::Void:
    case Type::Any:
    case Type::I8U:
    case Type::I16U:
    case Type::I32U:
      // These types are not concrete types and should never exist as a value
      WABT_UNREACHABLE;
  }
  WABT_UNREACHABLE;
}

void WriteValue(Stream* stream, const TypedValue& tv) {
  std::string s = TypedValueToString(tv);
  stream->WriteData(s.data(), s.size());
}

void WriteValues(Stream* stream,
                 const ValueTypes& types,
                 const Values& values) {
  assert(types.size() == values.size());
  for (size_t i = 0; i < values.size(); ++i) {
    WriteValue(stream, TypedValue{types[i], values[i]});
    if (i != values.size() - 1) {
      stream->Writef(", ");
    }
  }
}

void WriteTrap(Stream* stream, const char* desc, const Trap::Ptr& trap) {
  stream->Writef("%s: %s\n", desc, trap->message().c_str());
}

void WriteCall(Stream* stream,
               string_view name,
               const FuncType& func_type,
               const Values& params,
               const Values& results,
               const Trap::Ptr& trap) {
  stream->Writef(PRIstringview "(", WABT_PRINTF_STRING_VIEW_ARG(name));
  WriteValues(stream, func_type.params, params);
  stream->Writef(") =>");
  if (!trap) {
    if (!results.empty()) {
      stream->Writef(" ");
      WriteValues(stream, func_type.results, results);
    }
    stream->Writef("\n");
  } else {
    WriteTrap(stream, " error", trap);
  }
}

}  // namespace interp
}  // namespace wabt
