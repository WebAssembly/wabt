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

#include "wasm-option-parser.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "wasm-config.h"

#if HAVE_ALLOCA
#include <alloca.h>
#endif

static int option_match(const char* s,
                        const char* full,
                        WasmHasArgument has_argument) {
  int i;
  for (i = 0; ; i++) {
    if (full[i] == '\0') {
      /* perfect match. return +1, so it will be preferred over a longer option
       * with the same prefix */
      if (s[i] == '\0')
        return i + 1;

      /* we want to fail if s is longer than full, e.g. --foobar vs. --foo.
       * However, if s ends with an '=', it's OK. */
      if (!(has_argument && s[i] == '='))
        return -1;
      break;
    }
    if (s[i] != full[i])
      break;
  }
  return i;
}

static void error(WasmOptionParser* parser, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  parser->on_error(parser, buffer);
}

void wasm_parse_options(WasmOptionParser* parser,
                        int argc,
                        char** argv) {
  parser->argv0 = argv[0];

  int i;
  int j;
  int k;
  for (i = 1; i < argc; ++i) {
    char* arg = argv[i];
    if (arg[0] == '-') {
      if (arg[1] == '-') {
        /* long option */
        int best_index = -1;
        int best_length = 0;
        int best_count = 0;
        for (j = 0; j < parser->num_options; ++j) {
          WasmOption* option = &parser->options[j];
          if (option->long_name) {
            int match_length =
                option_match(&arg[2], option->long_name, option->has_argument);
            if (match_length > best_length) {
              best_index = j;
              best_length = match_length;
              best_count = 1;
            } else if (match_length == best_length && best_length > 0) {
              best_count++;
            }
          }
        }

        if (best_count > 1) {
          error(parser, "ambiguous option \"%s\"", arg);
          continue;
        } else if (best_count == 0) {
          error(parser, "unknown option \"%s\"", arg);
          continue;
        }

        WasmOption* best_option = &parser->options[best_index];
        const char* option_argument = NULL;
        if (best_option->has_argument) {
          if (arg[best_length] == '=') {
            option_argument = &arg[best_length + 1];
          } else {
            if (i + 1 == argc || argv[i + 1][0] == '-') {
              error(parser, "option \"--%s\" requires argument",
                    best_option->long_name);
              continue;
            }
            ++i;
            option_argument = argv[i];
          }
        }
        parser->on_option(parser, best_option, option_argument);
      } else {
        /* short option */
        if (arg[1] == '\0') {
          /* just "-" */
          parser->on_argument(parser, arg);
          continue;
        }

        /* allow short names to be combined, e.g. "-d -v" => "-dv" */
        for (k = 1; arg[k]; ++k) {
          WasmBool matched = WASM_FALSE;
          for (j = 0; j < parser->num_options; ++j) {
            WasmOption* option = &parser->options[j];
            if (option->short_name && arg[k] == option->short_name) {
              const char* option_argument = NULL;
              if (option->has_argument) {
                /* a short option with a required argument cannot be followed
                 * by other short options */
                if (arg[k + 1] != '\0') {
                  error(parser, "option \"-%c\" requires argument",
                        option->short_name);
                  break;
                }

                if (i + 1 == argc || argv[i + 1][0] == '-') {
                  error(parser, "option \"-%c\" requires argument",
                        option->short_name);
                  break;
                }
                ++i;
                option_argument = argv[i];
              }
              parser->on_option(parser, option, option_argument);
              matched = WASM_TRUE;
              break;
            }
          }

          if (!matched) {
            error(parser, "unknown option \"-%c\"", arg[k]);
            continue;
          }
        }
      }
    } else {
      /* non-option argument */
      parser->on_argument(parser, arg);
    }
  }
}

static const char* trim_argv0(const char* argv0) {
  char* last_slash = strrchr(argv0, '/');
  if (last_slash)
    return last_slash + 1;
  return argv0;
}

void wasm_print_help(WasmOptionParser* parser) {
  int i;
  /* TODO(binji): do something more generic for filename here */
  printf("usage: %s [options] filename\n\n", trim_argv0(parser->argv0));
  printf("%s\n", parser->description);
  printf("options:\n");

  const int extra_space = 8;
  int longest_name_length = 0;
  for (i = 0; i < parser->num_options; ++i) {
    WasmOption* option = &parser->options[i];
    int length;
    if (option->long_name) {
      if (option->metavar) {
        length =
            wasm_snprintf(NULL, 0, "%s=%s", option->long_name, option->metavar);
      } else {
        length = wasm_snprintf(NULL, 0, "%s", option->long_name);
      }
    } else {
      continue;
    }

    if (length > longest_name_length)
      longest_name_length = length;
  }

  size_t buffer_size = longest_name_length + 1;
  char* buffer = alloca(buffer_size);

  for (i = 0; i < parser->num_options; ++i) {
    WasmOption* option = &parser->options[i];
    if (!option->short_name && !option->long_name)
      continue;

    if (option->short_name)
      printf("  -%c, ", option->short_name);
    else
      printf("      ");

    char format[20];
    if (option->long_name) {
      wasm_snprintf(format, sizeof(format), "--%%-%ds",
                    longest_name_length + extra_space);

      if (option->metavar) {
        wasm_snprintf(buffer, buffer_size, "%s=%s", option->long_name,
                      option->metavar);
        printf(format, buffer);
      } else {
        printf(format, option->long_name);
      }
    } else {
      /* +2 for the extra "--" above */
      wasm_snprintf(format, sizeof(format), "%%-%ds",
                    longest_name_length + extra_space + 2);
      printf(format, "");
    }

    if (option->help)
      printf("%s", option->help);
    printf("\n");
  }
}
