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
import sys
import tempfile
from collections import OrderedDict

import find_exe
import utils


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('-r', '--relocatable', action='store_true',
                      help='final output is relocatable')
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
  parser.add_argument('--dump-verbose', action='store_true')
  parser.add_argument('--spec', action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  wast2wasm = utils.Executable(
      find_exe.GetWast2WasmExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wast2wasm.AppendOptionalArgs({
      '--debug-names': options.debug_names,
      '-v': options.dump_verbose,
  })

  wasm_link = utils.Executable(
      find_exe.GetWasmlinkExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasm_link.AppendOptionalArgs({
      '-v': options.verbose,
      '-r': options.relocatable,
  })

  wasmdump = utils.Executable(
      find_exe.GetWasmdumpExecutable(options.bindir),
      error_cmdline=options.error_cmdline)

  wasm_interp = utils.Executable(find_exe.GetWasmInterpExecutable(
      options.bindir), error_cmdline=options.error_cmdline)

  wast2wasm.verbose = options.print_cmd
  wasm_link.verbose = options.print_cmd
  wasmdump.verbose = options.print_cmd
  wasm_interp.verbose = options.print_cmd

  filename = options.file

  with utils.TempDirectory(options.out_dir, 'wasm-link-') as out_dir:
    basename = os.path.basename(filename)
    basename_noext = os.path.splitext(basename)[0]
    out_file = os.path.join(out_dir, basename_noext + '.json')
    wast2wasm.RunWithArgs('--spec', '--debug-names', '--no-check', '-r', '-o',
                          out_file, filename)

    wasm_files = utils.GetModuleFilenamesFromSpecJSON(out_file)
    wasm_files = [utils.ChangeDir(f, out_dir) for f in wasm_files]

    output = os.path.join(out_dir, 'linked.wasm')
    if options.incremental:
      partially_linked = output + '.partial'
      for i, f in enumerate(wasm_files):
        if i == 0:
          wasm_link.RunWithArgs('-o', output, f)
        else:
          if os.path.exists(partially_linked):
            os.remove(partially_linked)
          os.rename(output, partially_linked)
          wasm_link.RunWithArgs('-r', '-o', output, partially_linked, f)
        #wasmdump.RunWithArgs('-d', '-h', output)
      wasmdump.RunWithArgs('-d', '-x', '-r', '-h', output)
    else:
      wasm_link.RunWithArgs('-o', output, *wasm_files)
      wasmdump.RunWithArgs('-d', '-x', '-r', '-h', output)

    if options.spec:
      with open(out_file) as json_file:
        spec = json.load(json_file, object_pairs_hook=OrderedDict)
        spec['commands'] = [c for c in spec['commands']
                            if c['type'] != 'module']
        module = OrderedDict([('type', 'module'),
                              ('line', 0),
                              ('filename', os.path.basename(output)),])
        spec['commands'].insert(0, module)

      with open(out_file, 'wb') as json_file:
        json.dump(spec, json_file, indent=4)

      wasm_interp.RunWithArgs('--spec', out_file)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except utils.Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
