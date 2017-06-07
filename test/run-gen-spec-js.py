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
GEN_SPEC_JS_PY = os.path.join(SCRIPT_DIR, 'gen-spec-js.py')


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--out-dir', metavar='PATH',
                      help='output directory for files.')
  parser.add_argument('--bindir', metavar='PATH',
                      default=find_exe.GetDefaultPath(),
                      help='directory to search for all executables.')
  parser.add_argument(
      '--js-engine', metavar='PATH',
      help='the path to the JavaScript engine with which to run'
      ' the generated JavaScript. If not specified, JavaScript'
      ' output will be written to stdout.')
  parser.add_argument('--js-engine-flags', metavar='FLAGS',
                      help='additional flags for JavaScript engine.',
                      action='append', default=[])
  parser.add_argument('--prefix-js',
                      help='Prefix JavaScript file to pass to gen-spec-js')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('--no-error-cmdline',
                      help='don\'t display the subprocess\'s commandline when'
                      + ' an error occurs', dest='error_cmdline',
                      action='store_false')
  parser.add_argument('-p', '--print-cmd',
                      help='print the commands that are run.',
                      action='store_true')
  parser.add_argument('file', help='wast file.')
  options = parser.parse_args(args)

  with utils.TempDirectory(options.out_dir, 'run-gen-spec-js-') as out_dir:
    wast2wasm = utils.Executable(
        find_exe.GetWast2WasmExecutable(options.bindir), '--spec',
        error_cmdline=options.error_cmdline)
    wast2wasm.AppendOptionalArgs({'-v': options.verbose})

    gen_spec_js = utils.Executable(sys.executable, GEN_SPEC_JS_PY,
                                   '--temp-dir', out_dir,
                                   error_cmdline=options.error_cmdline)
    gen_spec_js.AppendOptionalArgs({
        '--bindir': options.bindir,
        '--prefix': options.prefix_js,
    })
    gen_spec_js.verbose = options.print_cmd

    json_file = utils.ChangeDir(
        utils.ChangeExt(options.file, '.json'), out_dir)
    js_file = utils.ChangeExt(json_file, '.js')
    wast2wasm.RunWithArgs(options.file, '-o', json_file)

    if options.js_engine:
      gen_spec_js.RunWithArgs(json_file, '-o', js_file)
      js = utils.Executable(options.js_engine, *options.js_engine_flags)
      js.RunWithArgs(js_file)
    else:
      # Write JavaScript output to stdout
      gen_spec_js.RunWithArgs(json_file)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
