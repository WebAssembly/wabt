#include "src/embedding/bindings.h"
#include <stdlib.h>

__attribute__((weak, export_name("canonical_abi_realloc"))) void*
canonical_abi_realloc(void* ptr,
                      size_t orig_size,
                      size_t org_align,
                      size_t new_size) {
  void* ret = realloc(ptr, new_size);
  if (!ret)
    abort();
  return ret;
}

__attribute__((weak, export_name("canonical_abi_free"))) void
canonical_abi_free(void* ptr, size_t size, size_t align) {
  free(ptr);
}
#include <string.h>

void input_string_set(input_string_t* ret, const char* s) {
  ret->ptr = (char*)s;
  ret->len = strlen(s);
}

void input_string_dup(input_string_t* ret, const char* s) {
  // ret->len = strlen(s);
  // ret->ptr = canonical_abi_realloc(NULL, 0, 1, ret->len);
  // memcpy(ret->ptr, s, ret->len);
}

void input_string_free(input_string_t* ret) {
  canonical_abi_free(ret->ptr, ret->len, 1);
  ret->ptr = NULL;
  ret->len = 0;
}
typedef struct {
  // 0 if `val` is `ok`, 1 otherwise
  uint8_t tag;
  union {
    input_list_u8_t ok;
    input_errno_t err;
  } val;
} input_expected_list_u8_errno_t;
static int64_t RET_AREA[3];
__attribute__((export_name("wat2wasm"))) int32_t
__wasm_export_input_wat2wasm(int32_t arg, int32_t arg0, int32_t arg1) {
  input_string_t arg2 = (input_string_t){(char*)(arg), (size_t)(arg0)};
  input_list_u8_t ok;
  input_errno_t ret = input_wat2wasm(&arg2, arg1, &ok);

  input_expected_list_u8_errno_t ret3;
  if (ret <= 2) {
    ret3.tag = 1;
    ret3.val.err = ret;
  } else {
    ret3.tag = 0;
    ret3.val.ok = ok;
  }
  int32_t variant6;
  int32_t variant7;
  int32_t variant8;
  switch ((int32_t)(ret3).tag) {
    case 0: {
      const input_list_u8_t* payload = &(ret3).val.ok;
      variant6 = 0;
      variant7 = (int32_t)(*payload).ptr;
      variant8 = (int32_t)(*payload).len;
      break;
    }
    case 1: {
      const input_errno_t* payload4 = &(ret3).val.err;
      int32_t variant;
      switch ((int32_t)*payload4) {
        case 0: {
          variant = 0;
          break;
        }
      }
      variant6 = 1;
      variant7 = variant;
      variant8 = 0;
      break;
    }
  }
  int32_t ptr = (int32_t)&RET_AREA;
  *((int32_t*)(ptr + 16)) = variant8;
  *((int32_t*)(ptr + 8)) = variant7;
  *((int32_t*)(ptr + 0)) = variant6;
  return ptr;
}

#include <cstddef>

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "src/apply-names.h"
#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/binary-writer-spec.h"
#include "src/binary-writer.h"
#include "src/common.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/filenames.h"
#include "src/generate-names.h"
#include "src/ir.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-lexer.h"
#include "src/wast-parser.h"
#include "src/wat-writer.h"

input_errno_t input_wat2wasm(input_string_t* wat,
                             input_wasm_feature_t features,
                             input_list_u8_t* ret0) {
  std::unique_ptr<wabt::WastLexer> lexer =
      wabt::WastLexer::CreateBufferLexer("", wat->ptr, wat->len);
  wabt::Features* wabtfeatures = new wabt::Features();

  wabt::WastParseOptions options(*wabtfeatures);
  std::unique_ptr<wabt::Module> module;
  wabt::Errors* errors = new wabt::Errors();
  wabt::ParseWatModule(lexer.release(), &module, errors, &options);
  wabt::Module* moduleFinal = module.get();

  bool log = true;
  wabt::MemoryStream log_stream;
  wabt::WriteBinaryOptions writeoptions;
  writeoptions.canonicalize_lebs = 1;
  writeoptions.relocatable = 1;
  writeoptions.write_debug_names = 1;

  wabt::MemoryStream stream(log ? &log_stream : nullptr);
  wabt::Result result =
      WriteBinaryModule(&stream, moduleFinal, writeoptions);
  // if (result == wabt::Result::Ok) {
  //   std::unique_ptr<wabt::OutputBuffer> output_buffer;
  //   output_buffer = stream.ReleaseOutputBuffer();
  //   ret0->ptr = output_buffer->data.data();
  //   ret0->len = output_buffer->data.size();
  //   // result->log_buffer = log ? log_stream.ReleaseOutputBuffer() : nullptr;
  //   return 20;
  // }
  // else {
  //   return INPUT_ERRNO_BASEX;
  // }
  return 20;

  //   return lexer.release();
  // }

  // WabtParseWatResult* wabt_parse_wat(wabt::WastLexer* lexer,
  //                                    wabt::Features* features,
  //                                    wabt::Errors* errors) {
  //   wabt::WastParseOptions options(*features);
  //   WabtParseWatResult* result = new WabtParseWatResult();
  //   std::unique_ptr<wabt::Module> module;
  //   result->result = wabt::ParseWatModule(lexer, &module, errors, &options);
  //   result->module = std::move(module);
  //   return result;
}
