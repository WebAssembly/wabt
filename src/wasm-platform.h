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

#ifndef WASM_PLATFORM_H_
#define WASM_PLATFORM_H_

// Linux Specifics
#if defined __GNUC__
#define WARN_UNUSED __attribute__ ((warn_unused_result))

// Windows Specifics
#elif defined _MSC_VER
#define WARN_UNUSED _Check_return_

// unistd.h replacement for Windows
#define YY_NO_UNISTD_H
#include <stdlib.h>
#define isatty _isatty
#define fileno _fileno
#define ssize_t int

#else
#define WARN_UNUSED
#endif

#endif
