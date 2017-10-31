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
import shutil
import subprocess
import sys
import tempfile

import find_exe
import utils
from utils import Error

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
  parser.add_argument('-p', '--print-cmd', action='store_true',
                      help='print the commands that are run.')
  parser.add_argument('--no-debug-names', action='store_true')
  parser.add_argument('--objdump', action='store_true',
                      help="objdump the resulting binary")
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  gen_wasm = utils.Executable(sys.executable, GEN_WASM_PY,
                              error_cmdline=options.error_cmdline,
                              basename=os.path.basename(GEN_WASM_PY))

  objdump = utils.Executable(
      find_exe.GetWasmdumpExecutable(options.bindir),
      error_cmdline=options.error_cmdline)

  wasm2wat = utils.Executable(
      find_exe.GetWasm2WatExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasm2wat.AppendOptionalArgs({
      '--no-debug-names': options.no_debug_names,
      '--verbose': options.verbose,
  })

  wasmvalidate = utils.Executable(
      find_exe.GetWasmValidateExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasmvalidate.AppendOptionalArgs({
      '--no-debug-names': options.no_debug_names,
  })

  gen_wasm.verbose = options.print_cmd
  wasm2wat.verbose = options.print_cmd
  wasmvalidate.verbose = options.print_cmd
  objdump.verbose = options.print_cmd

  with utils.TempDirectory(options.out_dir, 'run-gen-wasm-') as out_dir:
    out_file = utils.ChangeDir(utils.ChangeExt(options.file, '.wasm'), out_dir)
    gen_wasm.RunWithArgs(options.file, '-o', out_file)
    if options.objdump:
      objdump.RunWithArgs(out_file, '-x')
    else:
      # Test running wasm-validate on all files. wasm2wat should produce the
      # same errors, so let's make sure that's true.
      validate_ok = False
      wasm2wat_ok = False

      try:
        try:
          wasmvalidate.RunWithArgs(out_file)
          validate_ok = True
        except Error as e:
          sys.stderr.write(str(e) + '\n')

        try:
          wasm2wat.RunWithArgs(out_file)
          wasm2wat_ok = True
        except Error as e:
          raise
      finally:
        if validate_ok != wasm2wat_ok:
          sys.stderr.write('wasm-validate and wasm2wat have different results!')


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
