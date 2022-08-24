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

void bindings_wat2wasm(bindings_string_t* wat,
                       bindings_wasm_feature_t features,
                       bindings_expected_list_u8_string_t* ret0) {
  std::unique_ptr<wabt::WastLexer> lexer =
      wabt::WastLexer::CreateBufferLexer("", wat->ptr, wat->len);
  wabt::Features* wabtfeatures = new wabt::Features();

  wabt::WastLexer* finalLexer = lexer.release();
  wabt::WastParseOptions options(*wabtfeatures);
  std::unique_ptr<wabt::Module> module;
  wabt::Errors* errors = new wabt::Errors();
  wabt::Result parseResult =
      wabt::ParseWatModule(finalLexer, &module, errors, &options);

  if (parseResult != wabt::Result::Ok) {
    // Line finder triggers bus error in Node
    // auto line_finder = lexer->MakeLineFinder();
    std::string string_result =
        FormatErrorsToString(*errors, wabt::Location::Type::Binary);
    // std::string string_result = FormatErrorsToString(
    //     *errors, wabt::Location::Type::Text, line_finder.get());
    ret0->tag = 1;
    bindings_string_set(&ret0->val.err, string_result.c_str());
    return;
  }
  wabt::Module* moduleFinal = module.get();

  bool log = true;
  wabt::MemoryStream log_stream;
  wabt::WriteBinaryOptions writeoptions;
  writeoptions.canonicalize_lebs = 1;
  writeoptions.relocatable = 1;
  writeoptions.write_debug_names = 1;

  wabt::MemoryStream stream(log ? &log_stream : nullptr);
  wabt::Result result = WriteBinaryModule(&stream, moduleFinal, writeoptions);
  if (result == wabt::Result::Ok) {
    std::unique_ptr<wabt::OutputBuffer> output_buffer;
    output_buffer = stream.ReleaseOutputBuffer();
    ret0->tag = 0;
    bindings_list_u8_t okval;
    wabt::OutputBuffer* realOut = output_buffer.release();
    okval.ptr = realOut->data.data();
    okval.len = realOut->data.size();
    ret0->val.ok = okval;
    return;
  } else {
    ret0->tag = 1;
    return;
  }
}

void bindings_wasm2wat(bindings_list_u8_t* wasm,
                       bindings_wasm_feature_t features,
                       bindings_expected_string_string_t* ret0) {
  wabt::ReadBinaryOptions options;
  wabt::Features* wabtfeatures;
  wabtfeatures = new wabt::Features();
  options.features = *wabtfeatures;
  // options.read_debug_names = read_debug_names;

  wabt::Module* module = new wabt::Module();
  // TODO(binji): Pass through from wabt_read_binary parameter.
  const char* filename = "<binary>";
  wabt::Result parseResult;
  wabt::Errors* errors = new wabt::Errors();
  parseResult = wabt::ReadBinaryIr(filename, wasm->ptr, wasm->len, options,
                                   errors, module);

  if (parseResult != wabt::Result::Ok) {
    std::string string_result =
        FormatErrorsToString(*errors, wabt::Location::Type::Binary);
    ret0->tag = 1;
    bindings_string_set(&ret0->val.err, string_result.c_str());
    return;
  }

  wabt::WriteWatOptions watoptions;
  watoptions.fold_exprs = false;
  watoptions.inline_export = false;

  wabt::MemoryStream stream;
  wabt::Result watResult;
  watResult = WriteWat(&stream, module, watoptions);
  if (watResult == wabt::Result::Ok) {
    bindings_string_t okval;
    std::unique_ptr<wabt::OutputBuffer> output_buffer;
    output_buffer = stream.ReleaseOutputBuffer();
    wabt::OutputBuffer* realOut = output_buffer.release();
    std::string string_result(realOut->data.begin(), realOut->data.end());
    bindings_string_set(&okval, string_result.c_str());
    ret0->tag = 0;
    ret0->val.ok = okval;
    return;
  } else {
    ret0->tag = 1;
    return;
  }
}
