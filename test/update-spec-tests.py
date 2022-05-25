#!/usr/bin/env python3
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

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DIR = SCRIPT_DIR
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
TESTSUITE_DIR = os.path.join(REPO_ROOT_DIR, 'third_party', 'testsuite')
PROPOSALS_DIR = os.path.join(TESTSUITE_DIR, 'proposals')
SPEC_TEST_DIR = os.path.join(TEST_DIR, 'spec')
WASM2C_SPEC_TEST_DIR = os.path.join(TEST_DIR, 'wasm2c', 'spec')

options = None


def GetFilesWithExtension(src_dir, want_ext):
    result = set()
    if os.path.exists(src_dir):
        for filename in os.listdir(src_dir):
            name, ext = os.path.splitext(filename)
            if ext == want_ext:
                result.add(name)
    return result


def ProcessDir(wabt_test_dir, testsuite_dir, tool, flags=None):
    testsuite_tests = GetFilesWithExtension(testsuite_dir, '.wast')
    wabt_tests = GetFilesWithExtension(wabt_test_dir, '.txt')

    for removed_test_name in wabt_tests - testsuite_tests:
        test_filename = os.path.join(wabt_test_dir, removed_test_name + '.txt')
        if options.verbose:
            print('Removing %s' % test_filename)
        os.remove(test_filename)

    for added_test_name in testsuite_tests - wabt_tests:
        wast_filename = os.path.join(
            os.path.relpath(testsuite_dir, REPO_ROOT_DIR),
            added_test_name + '.wast')
        test_filename = os.path.join(wabt_test_dir, added_test_name + '.txt')
        if options.verbose:
            print('Adding %s' % test_filename)

        test_dirname = os.path.dirname(test_filename)
        if not os.path.exists(test_dirname):
            os.makedirs(test_dirname)

        with open(test_filename, 'w') as f:
            f.write(';;; TOOL: %s\n' % tool)
            f.write(';;; STDIN_FILE: %s\n' % wast_filename.replace(os.sep, '/'))
            if flags:
                f.write(';;; ARGS*: %s\n' % flags)


def ProcessProposalDir(name, flags=None):
    ProcessDir(os.path.join(SPEC_TEST_DIR, name),
               os.path.join(PROPOSALS_DIR, name),
               'run-interp-spec',
               flags)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                        action='store_true')
    global options
    options = parser.parse_args(args)

    ProcessDir(SPEC_TEST_DIR, TESTSUITE_DIR, 'run-interp-spec')
    ProcessDir(WASM2C_SPEC_TEST_DIR, TESTSUITE_DIR, 'run-spec-wasm2c')

    all_proposals = [e.name for e in os.scandir(PROPOSALS_DIR) if e.is_dir()]

    flags = {
        'memory64': '--enable-memory64',
        'multi-memory': '--enable-multi-memory',
        'exception-handling': '--enable-exceptions',
        'extended-const': '--enable-extended-const',
    }

    unimplemented = set([
        'gc',
        'tail-call',
        'function-references',
        'threads',
        'annotations',
        'exception-handling',
    ])

    # sanity check to verify that all flags are valid
    for proposal in flags:
        assert proposal in all_proposals, proposal
    # sanity check to verify that all unimplemented are valid
    for proposal in unimplemented:
        assert proposal in all_proposals, proposal

    proposals = [p for p in all_proposals if p not in unimplemented]
    for proposal in proposals:
        ProcessProposalDir(proposal, flags.get(proposal))

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
