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
import os
import sys
import tempfile

import find_exe
import utils

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
GEN_WASM_PY = os.path.join(SCRIPT_DIR, 'gen-wasm.py')

def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('-o', '--out-dir', metavar='PATH',
                      help='output directory for files.')
  parser.add_argument('--bindir', metavar='PATH',
                      default=find_exe.GetDefaultPath(),
                      help='directory to search for all executables.')
  parser.add_argument('--no-error-cmdline',
                      help='don\'t display the subprocess\'s commandline when'
                      + ' an error occurs', dest='error_cmdline',
                      action='store_false')
  parser.add_argument('-p', '--print-cmd',
                      help='print the commands that are run.',
                      action='store_true')
  parser.add_argument('--headers', action='store_true')
  parser.add_argument('--no-check', action='store_true')
  parser.add_argument('-c', '--compile-only', action='store_true')
  parser.add_argument('--details', action='store_true',
                      help='Show section details')
  parser.add_argument('--dump-verbose', action='store_true')
  parser.add_argument('--dump-debug', action='store_true')
  parser.add_argument('--future-exceptions', action='store_true')
  parser.add_argument('--gen-wasm', action='store_true',
                      help='parse with gen-wasm')
  parser.add_argument('--spec', action='store_true')
  parser.add_argument('-r', '--relocatable', action='store_true')
  parser.add_argument('--no-canonicalize-leb128s', action='store_true')
  parser.add_argument('--debug-names', action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  if options.gen_wasm and options.spec:
    parser.error('Can\'t use both --gen-wasm and --spec')

  gen_wasm = utils.Executable(sys.executable, GEN_WASM_PY,
                              error_cmdline=options.error_cmdline)

  wast2wasm = utils.Executable(
      find_exe.GetWast2WasmExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wast2wasm.AppendOptionalArgs({
      '--debug-names': options.debug_names,
      '--future-exceptions': options.future_exceptions,
      '--no-check': options.no_check,
      '--no-canonicalize-leb128s': options.no_canonicalize_leb128s,
      '--spec': options.spec,
      '-v': options.verbose,
      '-r': options.relocatable,
      '-c': options.compile_only,
  })

  wasm_objdump = utils.Executable(
      find_exe.GetWasmdumpExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasm_objdump.AppendOptionalArgs({
      '--details': options.details,
      '--future-exceptions': options.future_exceptions,
      '-h': options.headers,
      '-x': options.dump_verbose,
      '--debug': options.dump_debug,
  })

  gen_wasm.verbose = options.print_cmd
  wast2wasm.verbose = options.print_cmd
  wasm_objdump.verbose = options.print_cmd

  filename = options.file

  with utils.TempDirectory(options.out_dir, 'objdump-') as out_dir:
    basename = os.path.basename(filename)
    basename_noext = os.path.splitext(basename)[0]
    if options.gen_wasm:
      out_file = os.path.join(out_dir, basename_noext + '.wasm')
      gen_wasm.RunWithArgs('-o', out_file, filename)
    else:
      if options.spec:
        out_file = os.path.join(out_dir, basename_noext + '.json')
      else:
        out_file = os.path.join(out_dir, basename_noext + '.wasm')
      wast2wasm.RunWithArgs('-o', out_file, filename)

    if options.spec:
      wasm_files = utils.GetModuleFilenamesFromSpecJSON(out_file)
      wasm_files = [utils.ChangeDir(f, out_dir) for f in wasm_files]
    else:
      wasm_files = [out_file]

    wasm_objdump.RunWithArgs('-r', '-d', *wasm_files)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except utils.Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
