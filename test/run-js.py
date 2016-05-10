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
WASM_JS = os.path.join(SCRIPT_DIR, 'wasm.js')
SPEC_JS = os.path.join(SCRIPT_DIR, 'spec.js')

def CleanD8Stdout(stdout):
  def FixLine(line):
    idx = line.find('WasmModule::Instantiate()')
    if idx != -1:
      return line[idx:]
    return line
  lines = [FixLine(line) for line in stdout.splitlines(1)]
  return ''.join(lines)


def CleanD8Stderr(stderr):
  idx = stderr.find('==== C stack trace')
  if idx != -1:
    stderr = stderr[:idx]
  return stderr.strip()


def GetExeBasename(exe):
  return os.path.basename(exe)


def GetJSExecutable(options):
  exe = find_exe.GetJSExecutable(options.js_executable)
  if GetExeBasename(exe) == 'd8':
    return utils.Executable(exe,
                            clean_stdout=CleanD8Stdout,
                            clean_stderr=CleanD8Stderr,
                            error_cmdline=options.error_cmdline)
  else:
    return utils.Executable(exe, error_cmdline=options.error_cmdline)


def RunJS(js, js_file, out_file):
  if GetExeBasename(js.exe) == 'd8':
    js.RunWithArgs('--expose-wasm', js_file, '--', out_file)
  else:
    js.RunWithArgs(js_file, out_file)


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--out-dir', metavar='PATH',
                      help='output directory for files.')
  parser.add_argument('-e', '--executable', metavar='PATH',
                      help='override sexpr-wasm executable.')
  parser.add_argument('--js-executable', metavar='PATH',
                      help='override js executable.')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('--no-error-cmdline',
                      help='don\'t display the subprocess\'s commandline when' +
                          ' an error occurs', dest='error_cmdline',
                      action='store_false')
  parser.add_argument('--use-libc-allocator', action='store_true')
  parser.add_argument('--spec', help='run spec tests.', action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  sexpr_wasm = utils.Executable(
      find_exe.GetSexprWasmExecutable(options.executable),
      error_cmdline=options.error_cmdline)
  sexpr_wasm.AppendOptionalArgs({
    '-v': options.verbose,
    '--spec': options.spec,
    '--use-libc-allocator': options.use_libc_allocator
  })

  js = GetJSExecutable(options)
  with utils.TempDirectory(options.out_dir, 'run-js-') as out_dir:
    new_ext = '.json' if options.spec else '.wasm'
    out_file = utils.ChangeDir(utils.ChangeExt(options.file, new_ext), out_dir)
    sexpr_wasm.RunWithArgs(options.file, '-o', out_file)

    js_file = SPEC_JS if options.spec else WASM_JS
    RunJS(js, js_file, out_file)

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
