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

# snapshot of older version of proposals where WABT doesn't support current version
OLD_PROPOSALS_DIR = os.path.join(REPO_ROOT_DIR, 'test', 'old-spec', 'proposals')

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

            # Check for extra flags based on filename
            test_name = os.path.basename(test_filename)
            if flags:
                f.write(';;; ARGS*: %s\n' % flags)

            if 'relaxed' in test_name:
                f.write(';;; ARGS*: --enable-relaxed-simd\n')
            if 'memory64' in test_name or 'memory_max' in test_name:
                f.write(';;; ARGS*: --enable-memory64\n')
            if 'call_ref' in test_name or 'ref_' in test_name or 'br_on_' in test_name or 'return_call_ref' in test_name or 'struct' in test_name or 'array' in test_name or 'i31' in test_name or 'unreached-' in test_name:
                f.write(';;; ARGS*: --enable-function-references --enable-gc\n')
            if 'tail' in test_name or 'return_call' in test_name:
                 f.write(';;; ARGS*: --enable-tail-call\n')
            if 'relaxed-simd' in test_name:
                f.write(';;; ARGS*: --enable-relaxed-simd\n')
            if 'extended-const' in test_name:
                 f.write(';;; ARGS*: --enable-extended-const\n') 
            if 'throw' in test_name or 'try' in test_name or 'catch' in test_name or 'tag' in test_name or 'exception' in test_name:
                 f.write(';;; ARGS*: --enable-exceptions\n')
            if 'multi' in test_name and 'memory' in test_name:
                 f.write(';;; ARGS*: --enable-multi-memory\n')


def ProcessProposalDir(name, flags=None, old=False):
    proposals_dir = OLD_PROPOSALS_DIR if old else PROPOSALS_DIR
    ProcessDir(os.path.join(SPEC_TEST_DIR, name),
               os.path.join(proposals_dir, name),
               'run-interp-spec',
               flags)
    ProcessDir(os.path.join(WASM2C_SPEC_TEST_DIR, name),
               os.path.join(proposals_dir, name),
               'run-spec-wasm2c',
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
        'custom-page-sizes': '--enable-custom-page-sizes',
    }

    # Proposals that were recently merged or moved might be missing.
    # We only assert for proposals that we expect to be in the directory.
    
    unimplemented = set([
        'threads',
        'wide-arithmetic',
        'custom-descriptors',
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
