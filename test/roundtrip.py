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
import difflib
import os
import shutil
import subprocess
import string
import struct
import sys
import tempfile

IS_WINDOWS = sys.platform == 'win32'
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
SEXPR_WASM = os.path.join(REPO_ROOT_DIR, 'out', 'sexpr-wasm')
WASM_WAST = os.path.join(REPO_ROOT_DIR, 'out', 'wasm-wast')

if IS_WINDOWS:
  SEXPR_WASM += '.exe'
  WASM_WAST += '.exe'

class Error(Exception):
  pass


def RunSexprWasm(*args):
  cmd = [SEXPR_WASM] + list(args)
  cmd_str = ' '.join(cmd)
  try:
    process = subprocess.Popen(cmd, stderr=subprocess.PIPE)
    _, stderr = process.communicate()
    if process.returncode != 0:
      raise Error('Error running "%s":\n%s' % (cmd_str, stderr))
  except OSError as e:
    raise Error('Error running "%s": %s' % (cmd_str, str(e)))


def RunWasmWast(out_filename, *args):
  cmd = [WASM_WAST] + list(args)
  cmd_str = ' '.join(cmd)
  try:
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    if process.returncode != 0:
      raise Error('Error running "%s":\n%s' % (cmd_str, stderr))
    with open(out_filename, 'wb') as out_file:
      out_file.write(stdout)
  except OSError as e:
    raise Error('Error running "%s": %s' % (cmd_str, str(e)))


def Hexdump(data):
  def Line(length):
    return '0x%08x: ' + '%02x%02x ' * (length / 2) + '%c' * length + '\n'

  line_len = 16
  line = Line(line_len)
  unpack_fmt = 'B' * line_len
  isprint = set(string.printable)
  offset = 0
  while offset < len(data):
    row_str = data[offset:offset+line_len]
    if len(row_str) < line_len:
      line_len = len(row_str)
      line = Line(line_len)
      unpack_fmt = 'B' * line_len
    row = struct.unpack(unpack_fmt, row_str)
    args = [offset]
    args.extend(row)
    args.extend(chr(c) if chr(c) in isprint else '.' for c in row)
    yield line % tuple(args)
    offset += line_len


def FilesAreEqual(filename1, filename2, verbose=False):
  with open(filename1, 'rb') as file1:
    data1 = file1.read()

  with open(filename2, 'rb') as file2:
    data2 = file2.read()

  if data1 != data2:
    sys.stderr.write('files differ\n')
    if verbose:
      hexdump1 = list(Hexdump(data1))
      hexdump2 = list(Hexdump(data2))
      for line in (difflib.unified_diff(hexdump1, hexdump2,
                                        fromfile=filename1, tofile=filename2)):
        sys.stderr.write(line)
    return False
  return True


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('-o', '--out-dir', help='output directory for files.')
  parser.add_argument('file', help='test file.')
  options = parser.parse_args(args)

  if options.out_dir:
    out_dir = options.out_dir
    out_dir_is_temp = False
    if not os.path.exists(out_dir):
      os.makedirs(out_dir)
  else:
    out_dir = tempfile.mkdtemp(prefix='roundtrip-')
    out_dir_is_temp = True

  try:
    basename = os.path.basename(options.file)
    basename_noext = os.path.splitext(basename)[0]
    wasm1_file = os.path.join(out_dir, basename_noext + '-1.wasm')
    wast2_file = os.path.join(out_dir, basename_noext + '-2.wast')
    wasm3_file = os.path.join(out_dir, basename_noext + '-3.wasm')
    RunSexprWasm('-o', wasm1_file, options.file)
    RunWasmWast(wast2_file, wasm1_file)
    RunSexprWasm('-o', wasm3_file, wast2_file)
    if not FilesAreEqual(wasm1_file, wasm3_file, options.verbose):
      return 1
    return 0
  finally:
    if out_dir_is_temp:
      shutil.rmtree(out_dir)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
