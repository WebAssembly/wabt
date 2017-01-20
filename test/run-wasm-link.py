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
  parser.add_argument('--incremental', help='incremenatly link one object at' +
                      ' a time to produce the final linked binary.',
                      action='store_true')
  parser.add_argument('--debug-names', action='store_true')
  parser.add_argument('--use-libc-allocator', action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  wast2wasm = utils.Executable(
      find_exe.GetWast2WasmExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wast2wasm.AppendOptionalArgs({
      '--debug-names': options.debug_names,
      '--use-libc-allocator': options.use_libc_allocator,
      '-v': options.verbose,
  })

  wasm_link = utils.Executable(
      find_exe.GetWasmlinkExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasm_link.AppendOptionalArgs({'-v': options.verbose,})

  wasmdump = utils.Executable(
      find_exe.GetWasmdumpExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasmdump.AppendOptionalArgs({})

  wast2wasm.verbose = options.print_cmd
  wasm_link.verbose = options.print_cmd
  wasmdump.verbose = options.print_cmd

  filename = options.file

  with utils.TempDirectory(options.out_dir, 'wasm-link-') as out_dir:
    basename = os.path.basename(filename)
    basename_noext = os.path.splitext(basename)[0]
    out_file = os.path.join(out_dir, basename_noext + '.json')
    wast2wasm.RunWithArgs('--spec', '-r', '-o', out_file, filename)

    wasm_files = utils.GetModuleFilenamesFromSpecJSON(out_file)
    wasm_files = [utils.ChangeDir(f, out_dir) for f in wasm_files]

    output = os.path.join(out_dir, 'linked.wasm')
    if options.incremental:
      partialy_linked = output + '.partial'
      for i, f in enumerate(wasm_files):
        if i == 0:
          wasm_link.RunWithArgs('-o', output, f)
        else:
          os.rename(output, partialy_linked)
          wasm_link.RunWithArgs('-o', output, partialy_linked, f)
        #wasmdump.RunWithArgs('-d', '-h', output)
      wasmdump.RunWithArgs('-d', '-x', '-h', output)
    else:
      wasm_link.RunWithArgs('-o', output, *wasm_files)
      wasmdump.RunWithArgs('-d', '-x', '-h', output)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except utils.Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
