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

#ifndef WABT_OPTION_PARSER_H_
#define WABT_OPTION_PARSER_H_

#include "common.h"

typedef enum WabtHasArgument {
  WABT_OPTION_NO_ARGUMENT,
  WABT_OPTION_HAS_ARGUMENT,
} WabtHasArgument;

struct WabtOption;
struct WabtOptionParser;
typedef void (*WabtOptionCallback)(struct WabtOptionParser*,
                                   struct WabtOption*,
                                   const char* argument);
typedef void (*WabtArgumentCallback)(struct WabtOptionParser*,
                                     const char* argument);
typedef void (*WabtOptionErrorCallback)(struct WabtOptionParser*,
                                        const char* message);

typedef struct WabtOption {
  int id;
  char short_name;
  const char* long_name;
  const char* metavar;
  WabtHasArgument has_argument;
  const char* help;
} WabtOption;

typedef struct WabtOptionParser {
  const char* description;
  WabtOption* options;
  int num_options;
  WabtOptionCallback on_option;
  WabtArgumentCallback on_argument;
  WabtOptionErrorCallback on_error;
  void* user_data;

  /* cached after call to wabt_parse_options */
  char* argv0;
} WabtOptionParser;

WABT_EXTERN_C_BEGIN
void wabt_parse_options(WabtOptionParser* parser,
                        int argc,
                        char** argv);
void wabt_print_help(WabtOptionParser* parser, const char* program_name);
WABT_EXTERN_C_END

#endif /* WABT_OPTION_PARSER_H_ */
