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
OLD_SPEC_DIR = os.path.join(REPO_ROOT_DIR, 'test', 'old-spec')
OLD_PROPOSALS_DIR = os.path.join(OLD_SPEC_DIR, 'proposals')

# Core testsuite files patched for spec changes not yet in the pinned testsuite.
CORE_OLD_SPEC_OVERRIDES = {
    'global': os.path.join(OLD_SPEC_DIR, 'global.wast'),
    'elem': os.path.join(OLD_SPEC_DIR, 'elem.wast'),
    'data': os.path.join(OLD_SPEC_DIR, 'data.wast'),
}

# Proposal directories fully replaced by patched snapshots under test/old-spec/.
OLD_PROPOSAL_DIRS = {
    'extended-const': os.path.join(OLD_PROPOSALS_DIR, 'extended-const'),
}

# Individual proposal tests replaced by patched snapshots.
OLD_PROPOSAL_WAST_OVERRIDES = {
    'multi-memory/data': os.path.join(OLD_PROPOSALS_DIR, 'multi-memory', 'data.wast'),
}

options = None


def GetFilesWithExtension(src_dir, want_ext):
    result = set()
    if os.path.exists(src_dir):
        for filename in os.listdir(src_dir):
            name, ext = os.path.splitext(filename)
            if ext == want_ext:
                result.add(name)
    return result


def GetWastPath(testsuite_dir, test_name, overrides=None):
    if overrides and test_name in overrides:
        override_path = overrides[test_name]
        assert os.path.exists(override_path), override_path
        return os.path.relpath(override_path, REPO_ROOT_DIR).replace(os.sep, '/')
    return os.path.join(
        os.path.relpath(testsuite_dir, REPO_ROOT_DIR),
        test_name + '.wast').replace(os.sep, '/')


def ProcessDir(wabt_test_dir, testsuite_dir, tool, flags=None, overrides=None):
    testsuite_tests = GetFilesWithExtension(testsuite_dir, '.wast')
    wabt_tests = GetFilesWithExtension(wabt_test_dir, '.txt')

    for removed_test_name in wabt_tests - testsuite_tests:
        test_filename = os.path.join(wabt_test_dir, removed_test_name + '.txt')
        if options.verbose:
            print('Removing %s' % test_filename)
        os.remove(test_filename)

    for added_test_name in testsuite_tests - wabt_tests:
        wast_filename = GetWastPath(testsuite_dir, added_test_name, overrides)
        test_filename = os.path.join(wabt_test_dir, added_test_name + '.txt')
        if options.verbose:
            print('Adding %s' % test_filename)

        test_dirname = os.path.dirname(test_filename)
        if not os.path.exists(test_dirname):
            os.makedirs(test_dirname)

        with open(test_filename, 'w') as f:
            f.write(';;; TOOL: %s\n' % tool)
            f.write(';;; STDIN_FILE: %s\n' % wast_filename)
            if flags:
                f.write(';;; ARGS*: %s\n' % flags)


def ProcessProposalDir(name, flags=None, old=False):
    if old:
        proposals_dir = os.path.join(OLD_PROPOSALS_DIR, name)
    elif name in OLD_PROPOSAL_DIRS:
        proposals_dir = OLD_PROPOSAL_DIRS[name]
    else:
        proposals_dir = os.path.join(PROPOSALS_DIR, name)

    per_proposal_overrides = {}
    for test_name in GetFilesWithExtension(proposals_dir, '.wast'):
        override_key = '%s/%s' % (name, test_name)
        if override_key in OLD_PROPOSAL_WAST_OVERRIDES:
            per_proposal_overrides[test_name] = OLD_PROPOSAL_WAST_OVERRIDES[override_key]

    ProcessDir(os.path.join(SPEC_TEST_DIR, name),
               proposals_dir,
               'run-interp-spec',
               flags,
               per_proposal_overrides)
    ProcessDir(os.path.join(WASM2C_SPEC_TEST_DIR, name),
               proposals_dir,
               'run-spec-wasm2c',
               flags,
               per_proposal_overrides)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                        action='store_true')
    global options
    options = parser.parse_args(args)

    ProcessDir(SPEC_TEST_DIR, TESTSUITE_DIR, 'run-interp-spec',
               overrides=CORE_OLD_SPEC_OVERRIDES)
    ProcessDir(WASM2C_SPEC_TEST_DIR, TESTSUITE_DIR, 'run-spec-wasm2c',
               overrides=CORE_OLD_SPEC_OVERRIDES)

    all_proposals = [e.name for e in os.scandir(PROPOSALS_DIR) if e.is_dir()]

    flags = {
        'multi-memory': '--enable-multi-memory',
        'exception-handling': '--enable-exceptions',
        'extended-const': '--enable-extended-const',
        'tail-call': '--enable-tail-call',
        'relaxed-simd': '--enable-relaxed-simd',
        'custom-page-sizes': '--enable-custom-page-sizes',
        'function-references': '--enable-function-references',
    }

    old_proposal_flags = {
        'memory64': '--enable-memory64',
    }

    unimplemented = set([
        'gc',
        'threads',
        'annotations',
        'wide-arithmetic',
        'wasm-3.0',
    ])

    # sanity check to verify that all flags are valid
    for proposal in flags:
        assert proposal in all_proposals, proposal
    # sanity check to verify that all unimplemented are valid
    for proposal in unimplemented:
        assert proposal in all_proposals, proposal

    for name, path in CORE_OLD_SPEC_OVERRIDES.items():
        assert os.path.exists(path), path
    for name, path in OLD_PROPOSAL_WAST_OVERRIDES.items():
        assert os.path.exists(path), path
    for name, path in OLD_PROPOSAL_DIRS.items():
        assert os.path.isdir(path), path

    proposals = [p for p in all_proposals if p not in unimplemented]
    for proposal in proposals:
        ProcessProposalDir(proposal, flags.get(proposal))

    for proposal in old_proposal_flags:
        ProcessProposalDir(proposal, old_proposal_flags.get(proposal), True)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
