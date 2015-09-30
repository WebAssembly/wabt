#!/usr/bin/env python

import argparse
import os
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEFAULT_EXE = os.path.join(REPO_ROOT_DIR, 'out', 'sexpr-wasm')
BUILT_D8 = os.path.join(REPO_ROOT_DIR, 'third_party', 'v8-native-prototype',
                        'v8', 'v8', 'out', 'Release', 'd8')
DOWNLOAD_D8 = os.path.join(REPO_ROOT_DIR, 'out', 'd8')
WASM_JS = os.path.join(SCRIPT_DIR, 'wasm.js')
SPEC_JS = os.path.join(SCRIPT_DIR, 'spec.js')


class Error(Exception):
  pass


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
      cmd.append('--multi-module')
    try:
      subprocess.check_call(cmd)
    except OSError as e:
      raise Error(str(e))

    # Now run the generated file
    if options.spec:
      # The generated file is JavaScript, so run it directly.
      cmd = [d8, SPEC_JS, generated.name]
    else:
      cmd = [d8, WASM_JS, '--', generated.name]
    subprocess.check_call(cmd)
  finally:
    if generated:
      generated.close()

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except (Error, subprocess.CalledProcessError) as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
