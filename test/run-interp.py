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
import subprocess
import sys

import find_exe
import utils
from utils import Error

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--out-dir', metavar='PATH',
                      help='output directory for files.')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
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
  parser.add_argument('--run-all-exports', action='store_true')
  parser.add_argument('--host-print', action='store_true')
  parser.add_argument('--spec', action='store_true')
  parser.add_argument('-t', '--trace', action='store_true')
  parser.add_argument('file', help='test file.')
  parser.add_argument('--enable-saturating-float-to-int', action='store_true')
  parser.add_argument('--enable-threads', action='store_true')
  options = parser.parse_args(args)

  wast_tool = None
  interp_tool = None
  if options.spec:
    wast_tool = utils.Executable(
        find_exe.GetWast2JsonExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    interp_tool = utils.Executable(
        find_exe.GetSpectestInterpExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
  else:
    wast_tool = utils.Executable(
        find_exe.GetWat2WasmExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    interp_tool = utils.Executable(
        find_exe.GetWasmInterpExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    interp_tool.AppendOptionalArgs({
        '--host-print': options.host_print,
        '--run-all-exports': options.run_all_exports,
    })

  wast_tool.AppendOptionalArgs({
      '-v': options.verbose,
      '--enable-saturating-float-to-int':
          options.enable_saturating_float_to_int,
      '--enable-threads': options.enable_threads,
  })

  interp_tool.AppendOptionalArgs({
      '-v': options.verbose,
      '--run-all-exports': options.run_all_exports,
      '--trace': options.trace,
      '--enable-saturating-float-to-int':
          options.enable_saturating_float_to_int,
      '--enable-threads': options.enable_threads,
  })

  wast_tool.verbose = options.print_cmd
  interp_tool.verbose = options.print_cmd

  with utils.TempDirectory(options.out_dir, 'run-interp-') as out_dir:
    new_ext = '.json' if options.spec else '.wasm'
    out_file = utils.ChangeDir(utils.ChangeExt(options.file, new_ext), out_dir)
    wast_tool.RunWithArgs(options.file, '-o', out_file)
    interp_tool.RunWithArgs(out_file)

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
