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

#ifndef WABT_EMSCRIPTEN_HELPERS_H_
#define WABT_EMSCRIPTEN_HELPERS_H_

#include <cstddef>

#include <memory>

#include "ast.h"
#include "ast-lexer.h"
#include "ast-parser.h"
#include "binary-writer.h"
#include "common.h"
#include "resolve-names.h"
#include "source-error-handler.h"
#include "stream.h"
#include "validator.h"
#include "writer.h"

struct WabtParseAstResult {
  wabt::Result result;
  std::unique_ptr<wabt::Script> script;
};

struct WabtWriteBinaryModuleResult {
  wabt::Result result;
  std::unique_ptr<wabt::OutputBuffer> binary_buffer;
  std::unique_ptr<wabt::OutputBuffer> log_buffer;
};

extern "C" {

wabt::AstLexer* wabt_new_ast_buffer_lexer(const char* filename,
                                          const void* data,
                                          size_t size) {
  return wabt::new_ast_buffer_lexer(filename, data, size);
}

WabtParseAstResult* wabt_parse_ast(
    wabt::AstLexer* lexer,
    wabt::SourceErrorHandlerBuffer* error_handler) {
  WabtParseAstResult* result = new WabtParseAstResult();
  wabt::Script* script = nullptr;
  result->result = wabt::parse_ast(lexer, &script, error_handler);
  result->script.reset(script);
  return result;

}

wabt::Result wabt_resolve_names_script(
    wabt::AstLexer* lexer,
    wabt::Script* script,
    wabt::SourceErrorHandlerBuffer* error_handler) {
  return resolve_names_script(lexer, script, error_handler);
}

wabt::Result wabt_validate_script(
    wabt::AstLexer* lexer,
    wabt::Script* script,
    wabt::SourceErrorHandlerBuffer* error_handler) {
  return validate_script(lexer, script, error_handler);
}

WabtWriteBinaryModuleResult* wabt_write_binary_module(wabt::Script* script,
                                                      int log,
                                                      int canonicalize_lebs,
                                                      int relocatable,
                                                      int write_debug_names) {
  wabt::MemoryStream stream;
  wabt::WriteBinaryOptions options;
  options.log_stream = log ? &stream : nullptr;
  options.canonicalize_lebs = canonicalize_lebs;
  options.relocatable = relocatable;
  options.write_debug_names = write_debug_names;

  wabt::MemoryWriter writer;
  wabt::Module* module = wabt::get_first_module(script);
  WabtWriteBinaryModuleResult* result = new WabtWriteBinaryModuleResult();
  result->result = write_binary_module(&writer, module, &options);
  if (result->result == wabt::Result::Ok) {
    result->binary_buffer = writer.ReleaseOutputBuffer();
    result->log_buffer = log ? stream.ReleaseOutputBuffer() : nullptr;
  }
  return result;
}

void wabt_destroy_script(wabt::Script* script) {
  delete script;
}

void wabt_destroy_ast_lexer(wabt::AstLexer* lexer) {
  destroy_ast_lexer(lexer);
}

// SourceErrorHandlerBuffer
wabt::SourceErrorHandlerBuffer* wabt_new_source_error_handler_buffer(void) {
  return new wabt::SourceErrorHandlerBuffer();
}

const void* wabt_source_error_handler_buffer_get_data(
    wabt::SourceErrorHandlerBuffer* error_handler) {
  return error_handler->buffer().data();
}

size_t wabt_source_error_handler_buffer_get_size(
    wabt::SourceErrorHandlerBuffer* error_handler) {
  return error_handler->buffer().size();
}

void wabt_destroy_source_error_handler_buffer(
    wabt::SourceErrorHandlerBuffer* error_handler) {
  delete error_handler;
}

// WabtParseAstResult
wabt::Result wabt_parse_ast_result_get_result(WabtParseAstResult* result) {
  return result->result;
}

wabt::Script* wabt_parse_ast_result_release_script(WabtParseAstResult* result) {
  return result->script.release();
}

void wabt_destroy_parse_ast_result(WabtParseAstResult* result) {
  delete result;
}

// WabtWriteBinaryModuleResult
wabt::Result wabt_write_binary_module_result_get_result(
    WabtWriteBinaryModuleResult* result) {
  return result->result;
}

wabt::OutputBuffer*
wabt_write_binary_module_result_release_binary_output_buffer(
    WabtWriteBinaryModuleResult* result) {
  return result->binary_buffer.release();
}

wabt::OutputBuffer* wabt_write_binary_module_result_release_log_output_buffer(
    WabtWriteBinaryModuleResult* result) {
  return result->log_buffer.release();
}

void wabt_destroy_write_binary_module_result(
    WabtWriteBinaryModuleResult* result) {
  delete result;
}

// wabt::OutputBuffer*
const void* wabt_output_buffer_get_data(wabt::OutputBuffer* output_buffer) {
  return output_buffer->data.data();
}

size_t wabt_output_buffer_get_size(wabt::OutputBuffer* output_buffer) {
  return output_buffer->data.size();
}

void wabt_destroy_output_buffer(wabt::OutputBuffer* output_buffer) {
  delete output_buffer;
}

}  // extern "C"

#endif /* WABT_EMSCRIPTEN_HELPERS_H_ */
