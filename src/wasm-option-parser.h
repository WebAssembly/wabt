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

#ifndef WASM_OPTION_PARSER_H_
#define WASM_OPTION_PARSER_H_

#include "wasm-common.h"

typedef enum WasmHasArgument {
  WASM_OPTION_NO_ARGUMENT,
  WASM_OPTION_HAS_ARGUMENT,
} WasmHasArgument;

struct WasmOption;
struct WasmOptionParser;
typedef void (*WasmOptionCallback)(struct WasmOptionParser*,
                                   struct WasmOption*,
                                   const char* argument);
typedef void (*WasmArgumentCallback)(struct WasmOptionParser*,
                                     const char* argument);
typedef void (*WasmOptionErrorCallback)(struct WasmOptionParser*,
                                        const char* message);

typedef struct WasmOption {
  int id;
  char short_name;
  const char* long_name;
  const char* metavar;
  WasmHasArgument has_argument;
  const char* help;
} WasmOption;

typedef struct WasmOptionParser {
  const char* description;
  WasmOption* options;
  int num_options;
  WasmOptionCallback on_option;
  WasmArgumentCallback on_argument;
  WasmOptionErrorCallback on_error;
  void* user_data;

  /* cached after call to wasm_parse_options */
  char* argv0;
} WasmOptionParser;

WASM_EXTERN_C_BEGIN
void wasm_parse_options(WasmOptionParser* parser,
                        int argc,
                        char** argv);
void wasm_print_help(WasmOptionParser* parser, const char* program_name);
WASM_EXTERN_C_END

#endif /* WASM_OPTION_PARSER_H_ */
