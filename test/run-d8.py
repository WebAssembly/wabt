#!/usr/bin/env python

import argparse
import os
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEFAULT_EXE = os.path.join(REPO_ROOT_DIR, 'out', 'sexpr-wasm')
D8 = os.path.join(REPO_ROOT_DIR, 'third_party', 'v8-native-prototype', 'v8',
                  'v8', 'out', 'Release', 'd8')
WASM_JS = os.path.join(SCRIPT_DIR, 'wasm.js')


class Error(Exception):
  pass


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-e', '--executable', metavar='EXE',
                      help='override executable.')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  if options.executable:
    if not os.path.exists(options.executable):
      parser.error('executable %s does not exist' % options.executable)
    exe = os.path.abspath(options.executable)
  else:
    exe = DEFAULT_EXE

  generated = tempfile.NamedTemporaryFile(prefix='sexpr-wasm-')
  try:
    # First compile the file
    cmd = [exe, options.file, '-o', generated.name]
    try:
      process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
      stdout, stderr = process.communicate()
    except OSError as e:
      raise Error(str(e))

    if process.returncode != 0:
      raise Error(stderr)

    # Now run the generated file
    try:
      subprocess.check_call([D8, WASM_JS, '--', generated.name])
    except subprocess.CalledProcessError as e:
      raise Error(str(e))
  finally:
    generated.close()

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
