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
import re
import shutil
import subprocess
import sys
import tempfile

import findtests

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
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
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
    # TODO(binji): add flag to wasm-wast for output
    with open(out_filename, 'wb') as out_file:
      out_file.write(stdout)
  except OSError as e:
    raise Error('Error running "%s": %s' % (cmd_str, str(e)))


def Hexdump(data):
  DUMP_OCTETS_PER_LINE = 16
  DUMP_OCTETS_PER_GROUP = 2

  p = 0
  end = len(data)
  while p < end:
    line_start = p
    line_end = p +  DUMP_OCTETS_PER_LINE
    line = '%07x: ' % p
    while p < line_end:
      for i in xrange(DUMP_OCTETS_PER_GROUP):
        if p < end:
          line += '%02x' % ord(data[p])
        else:
          line += '  '
        p += 1
      line += ' '
    line += ' '
    p = line_start
    for i in xrange(DUMP_OCTETS_PER_LINE):
      if p >= end:
        break
      x = data[p]
      o = ord(x)
      if o >= 32 and o < 0x7f:
        line += '%c' % x
      else:
        line += '.'
      p += 1
    line += '\n'
    yield line


OK = 0
ERROR = 1
SKIPPED = 2


def FilesAreEqual(filename1, filename2, verbose=False):
  try:
    with open(filename1, 'rb') as file1:
      data1 = file1.read()

    with open(filename2, 'rb') as file2:
      data2 = file2.read()
  except OSError as e:
    return (ERROR, str(e))

  if data1 != data2:
    msg = 'files differ'
    if verbose:
      hexdump1 = list(Hexdump(data1))
      hexdump2 = list(Hexdump(data2))
      diff_lines = []
      for line in (difflib.unified_diff(
          hexdump1, hexdump2, fromfile=filename1, tofile=filename2)):
        diff_lines.append(line)
      msg += ''.join(diff_lines)
    msg += '\n'
    return (ERROR, msg)
  return (OK, '')


def Run(out_dir, filename, verbose):
  basename = os.path.basename(filename)
  basename_noext = os.path.splitext(basename)[0]
  wasm1_file = os.path.join(out_dir, basename_noext + '-1.wasm')
  wast2_file = os.path.join(out_dir, basename_noext + '-2.wast')
  wasm3_file = os.path.join(out_dir, basename_noext + '-3.wasm')
  try:
    RunSexprWasm('-o', wasm1_file, filename)
  except Error as e:
    # if the file doesn't parse properly, just skip it (it may be a "bad-*"
    # test)
    return (SKIPPED, None)
  try:
    RunWasmWast(wast2_file, wasm1_file)
    RunSexprWasm('-o', wasm3_file, wast2_file)
  except Error as e:
    return (ERROR, str(e))
  return FilesAreEqual(wasm1_file, wasm3_file, verbose)


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('-o', '--out-dir', help='output directory for files.')
  parser.add_argument('--test-all', help='run on all test files.',
                      action='store_true')
  parser.add_argument('file', nargs='?', help='test file.')
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
    if options.test_all:
      message= {OK: 'OK', ERROR: 'ERROR', SKIPPED: 'SKIPPED'}
      test_names = findtests.FindTestFiles(SCRIPT_DIR, '.txt', '.*')
      counts = [0, 0, 0]

      cwd = os.getcwd()
      for test_name in test_names:
        if test_name.startswith('spec'):
          continue

        filename = os.path.relpath(os.path.join(SCRIPT_DIR, test_name), cwd)
        result, msg = Run(out_dir, filename, verbose=True)
        counts[result] += 1
        if options.verbose:
          sys.stderr.write('%s: %s\n' % (filename, message[result]))
        if result == ERROR:
          sys.stderr.write('%s:\n%s\n' % (filename, msg))

      # summary
      sys.stderr.write('OK: %d ERROR: %d SKIPPED: %d\n' % tuple(counts))
      return counts[ERROR] == 0
    else:
      if not options.file:
        parser.error('expected file or --test-all')
      result, msg = Run(out_dir, options.file, options.verbose)
      if result == ERROR:
        sys.stderr.write(msg)
      return result
  finally:
    if out_dir_is_temp:
      shutil.rmtree(out_dir)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
