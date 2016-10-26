#!/usr/bin/env python
#
# Copyright 2016 WebAssembly Community Group participants
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import json
import os
import re
import sys

from utils import Executable, Error

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
TEST_DIR = os.path.join(ROOT_DIR, 'test')
DEFAULT_EMSCRIPTEN_DIR = os.path.join(ROOT_DIR, 'emscripten')

sys.path.append(TEST_DIR)


def FindFiles(cmake_build_dir):
  result = []
  for root, dirs, files in os.walk(cmake_build_dir):
    for file_ in files:
      path = os.path.join(root, file_)
      if file_ == 'libwasm.a' or re.search(r'wasm-emscripten-helpers', file_):
        result.append(path)
  return result


def GetNM(emscripten_dir):
  python = Executable(sys.executable)
  stdout = python.RunWithArgsForStdout(
      '-c', 'import tools.shared; print(tools.shared.LLVM_ROOT)',
      cwd=emscripten_dir)
  return Executable(os.path.join(stdout.strip(), 'llvm-nm'))


def ProcessFile(nm, file_):
  names = []
  for line in nm.RunWithArgsForStdout(file_).splitlines():
    line = line.rstrip()
    if not line.lstrip():
      continue
    if line.endswith(':'):
      # displaying the archive name, e.g. "foo.c.o:"
      continue
    # line looks like:
    #
    # -------- d yycheck
    # -------- t wasm_strndup_
    #          U wasm_parse_int64
    # -------- T wasm_offsetof_allocator_alloc
    #
    # we want to only keep the "T" names; the extern function symbols defined
    # in the object.
    type_ = line[9]
    if type_ != 'T':
      continue
    name = line[11:]
    names.append('_' + name)
  return names


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--output', metavar='PATH', help='output file.')
  parser.add_argument('-v', '--verbose',
                      help='print more diagnostic messages.',
                      action='store_true')
  parser.add_argument('--emscripten-dir', metavar='PATH',
                      help='emscripten directory',
                      default=DEFAULT_EMSCRIPTEN_DIR)
  parser.add_argument('cmake_build_dir', metavar='cmake_build_dir')
  options = parser.parse_args(args)
  nm = GetNM(options.emscripten_dir)

  names = []
  for file_ in FindFiles(options.cmake_build_dir):
    names.extend(ProcessFile(nm, file_))

  out_data = json.dumps(sorted(names), separators=(',\n', ':\n'))
  if options.output:
    with open(options.output, 'w') as out_file:
      out_file.write(out_data)
  else:
    print out_data
  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
