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

#include "color.h"

#include "common.h"

#if _WIN32
#include <io.h>
#include <windows.h>
#elif HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace wabt {

Color::Color(FILE* file, bool enabled) : file_(file), enabled_(enabled) {
  if (!enabled) {
    goto nocolor;
  }

#if _WIN32

  {
    HANDLE handle;
    if (file == stdout) {
      handle = GetStdHandle(STD_OUTPUT_HANDLE);
    } else if (file == stderr) {
      handle = GetStdHandle(STD_ERROR_HANDLE);
    } else {
      goto nocolor;
    }
    if (!SetConsoleMode(handle, ENABLE_VIRTUAL_TERMINAL_PROCESSING) ||
        !_isatty(_fileno(file))) {
      goto nocolor;
    }
  }

#elif HAVE_UNISTD_H

  if (!isatty(fileno(file))) {
    goto nocolor;
  }

#else

  goto nocolor;

#endif

  return;

nocolor:
  enabled_ = false;
}

void Color::WriteCode(const char* code) const {
  if (enabled_) {
    fputs(code, file_);
  }
}

}  // namespace wabt
