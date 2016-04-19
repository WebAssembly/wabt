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

#ifndef WASM_LEXER_H_
#define WASM_LEXER_H_

#include <stddef.h>
#include <stdio.h>

#include "wasm-common.h"
#include "wasm-vector.h"

struct WasmAllocator;

typedef size_t WasmLineOffset;
WASM_DEFINE_VECTOR(line_offset, WasmLineOffset);

typedef enum WasmLexerSourceType {
  WASM_LEXER_SOURCE_TYPE_FILE,
  WASM_LEXER_SOURCE_TYPE_BUFFER,
} WasmLexerSourceType;

typedef struct WasmLexerSource {
  WasmLexerSourceType type;
  union {
    FILE* file;
    struct {
      const void* data;
      size_t size;
      size_t read_offset;
    } buffer;
  };
} WasmLexerSource;

typedef struct WasmLexer {
  struct WasmAllocator* allocator;
  WasmLexerSource source;
  const char* filename;
  WasmLineOffsetVector line_offsets;
  int line;
  int comment_nesting;
  size_t buffer_file_offset;
  size_t last_line_file_offset;

  /* lexing data needed by re2c */
  WasmBool eof;
  char* buffer;
  size_t buffer_size;
  char* marker;
  char* token;
  char* cursor;
  char* limit;
} WasmLexer;

WASM_EXTERN_C_BEGIN

WasmLexer* wasm_new_file_lexer(struct WasmAllocator*, const char* filename);
WasmLexer* wasm_new_buffer_lexer(struct WasmAllocator*,
                                 const char* filename,
                                 const void* data,
                                 size_t size);
void wasm_destroy_lexer(WasmLexer*);
WASM_EXTERN_C_END

#endif /* WASM_LEXER_H_ */
