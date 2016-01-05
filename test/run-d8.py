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
import signal
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEFAULT_EXE = os.path.join(REPO_ROOT_DIR, 'out', 'sexpr-wasm')
BUILT_D8 = os.path.join(REPO_ROOT_DIR, 'third_party', 'v8', 'v8', 'out',
                        'Release', 'd8')
DOWNLOAD_D8 = os.path.join(REPO_ROOT_DIR, 'out', 'd8')
EXPOSE_WASM = '--expose-wasm'
WASM_JS = os.path.join(SCRIPT_DIR, 'wasm.js')
SPEC_JS = os.path.join(SCRIPT_DIR, 'spec.js')

# Get signal names from numbers in Python
# http://stackoverflow.com/a/2549950
SIGNAMES = dict((k, v) for v, k in reversed(sorted(signal.__dict__.items()))
                       if v.startswith('SIG') and not v.startswith('SIG_'))

class Error(Exception):
  pass


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


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-e', '--executable', metavar='EXE',
                      help='override sexpr-wasm executable.')
  parser.add_argument('--d8-executable', metavar='EXE',
                      help='override d8 executable.')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('--spec', help='run spec tests.', action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  if options.executable:
    if not os.path.exists(options.executable):
      parser.error('executable %s does not exist' % options.executable)
    exe = os.path.abspath(options.executable)
  else:
    exe = DEFAULT_EXE

  d8 = options.d8_executable
  if not d8 or not os.path.exists(d8):
    d8 = BUILT_D8
    if not os.path.exists(d8):
      d8 = DOWNLOAD_D8
      if not os.path.exists(d8):
        raise Error('d8 executable does not exist.\n'
                    'Run scripts/build-d8.sh to build it.\n'
                    'path: %s\npath: %s\n' % (BUILT_D8, DOWNLOAD_D8))

  generated = None
  try:
    generated = tempfile.NamedTemporaryFile(prefix='sexpr-wasm-')
    # First compile the file
    cmd = [exe, options.file, '-o', generated.name]
    if options.verbose:
      cmd.append('-v')
    if options.spec:
      cmd.extend(['--spec', '--spec-verbose'])
    try:
      process = subprocess.Popen(cmd, stderr=subprocess.PIPE)
      _, stderr = process.communicate()
      if process.returncode != 0:
        raise Error(stderr)
    except OSError as e:
      raise Error(str(e))

    # Now run the generated file
    if options.spec:
      # The generated file is JavaScript, so run it directly.
      cmd = [d8, EXPOSE_WASM, SPEC_JS, generated.name]
    else:
      cmd = [d8, EXPOSE_WASM, WASM_JS, '--', generated.name]
    try:
      process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
      stdout, stderr = process.communicate()
      sys.stdout.write(CleanD8Stdout(stdout))
      msg = CleanD8Stderr(stderr)
      if process.returncode < 0:
        # Terminated by signal
        signame = SIGNAMES.get(-process.returncode, '<unknown>')
        if msg:
          msg += '\n\n'
        raise Error(msg + 'd8 raised signal ' + signame)
      elif process.returncode > 0:
        raise Error(msg)
    except OSError as e:
      raise Error(str(e))

  finally:
    if generated:
      generated.close()

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
