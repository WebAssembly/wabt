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

from __future__ import print_function
import argparse
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DIR = SCRIPT_DIR
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
TESTSUITE_DIR = os.path.join(REPO_ROOT_DIR, 'third_party', 'testsuite')
SPEC_TEST_DIR = os.path.join(TEST_DIR, 'spec')


def GetFilesWithExtension(src_dir, want_ext):
  result = []
  for filename in os.listdir(src_dir):
    name, ext = os.path.splitext(filename)
    if ext == want_ext:
      result.append(name)
  return result


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  options = parser.parse_args(args)

  testsuite_tests = set(GetFilesWithExtension(TESTSUITE_DIR, '.wast'))
  spec_tests = set(GetFilesWithExtension(SPEC_TEST_DIR, '.txt'))

  for removed_test_name in spec_tests - testsuite_tests:
    test_filename = os.path.join(SPEC_TEST_DIR, removed_test_name + '.txt')
    if options.verbose:
      print('Removing %s' % test_filename)
    os.remove(test_filename)

  for added_test_name in testsuite_tests - spec_tests:
    wast_filename = os.path.join(
        os.path.relpath(TESTSUITE_DIR, REPO_ROOT_DIR),
        added_test_name + '.wast')
    test_filename = os.path.join(SPEC_TEST_DIR, added_test_name + '.txt')
    if options.verbose:
      print('Adding %s' % test_filename)
    with open(test_filename, 'w') as f:
      if added_test_name.endswith('.fail'):
        f.write(';;; ERROR: 1\n')
      f.write(';;; TOOL: run-interp-spec\n')
      f.write(';;; STDIN_FILE: %s\n' % wast_filename)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
