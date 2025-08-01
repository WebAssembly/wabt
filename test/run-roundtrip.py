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
import difflib
import os
import sys

import find_exe
import utils
from utils import Error

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
            hexdump1 = utils.Hexdump(data1)
            hexdump2 = utils.Hexdump(data2)
            diff_lines = []
            for line in difflib.unified_diff(hexdump1, hexdump2,
                                             fromfile=filename1,
                                             tofile=filename2):
                diff_lines.append(line)
            msg += ''.join(diff_lines)
        msg += '\n'
        return (ERROR, msg)
    return (OK, '')


def DoRoundtrip(wat2wasm, wasm2wat, out_dir, filename, verbose, stdout, skip_roundtrip_check):
    basename = os.path.basename(filename)
    basename_noext = os.path.splitext(basename)[0]
    wasm1_file = os.path.join(out_dir, basename_noext + '-1.wasm')
    wat2_file = os.path.join(out_dir, basename_noext + '-2.wat')
    wasm3_file = os.path.join(out_dir, basename_noext + '-3.wasm')
    try:
        wat2wasm.RunWithArgs('-o', wasm1_file, filename)
    except Error:
        # if the file doesn't parse properly, just skip it (it may be a "bad-*"
        # test)
        return (SKIPPED, None)
    try:
        wasm2wat.RunWithArgs('-o', wat2_file, wasm1_file)
        wat2wasm.RunWithArgs('-o', wasm3_file, wat2_file)
    except Error as e:
        return (ERROR, str(e))
    if stdout:
        with open(wat2_file) as f:
            sys.stdout.write(f.read())
    if skip_roundtrip_check:
        return (OK, '')
    return FilesAreEqual(wasm1_file, wasm3_file, verbose)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                        action='store_true')
    parser.add_argument('-o', '--out-dir', metavar='PATH',
                        help='output directory for files.')
    parser.add_argument('--bindir', metavar='PATH',
                        default=find_exe.GetDefaultPath(),
                        help='directory to search for all executables.')
    parser.add_argument('--stdout', action='store_true',
                        help='write wat output to stdout')
    parser.add_argument('--no-error-cmdline',
                        help='don\'t display the subprocess\'s commandline when '
                        'an error occurs', dest='error_cmdline',
                        action='store_false')
    parser.add_argument('-p', '--print-cmd',
                        help='print the commands that are run.',
                        action='store_true')
    parser.add_argument('--no-check', action='store_true')
    parser.add_argument('--debug-names', action='store_true')
    # --generate-names modifies name section, so skip roundtrip check
    parser.add_argument('--generate-names', action='store_true',
                        help="write debug names and skip end-to-end roundtrip check")
    parser.add_argument('--fold-exprs', action='store_true')
    parser.add_argument('--enable-exceptions', action='store_true')
    parser.add_argument('--enable-saturating-float-to-int', action='store_true')
    parser.add_argument('--enable-function-references', action='store_true')
    parser.add_argument('--enable-gc', action='store_true')
    parser.add_argument('--enable-threads', action='store_true')
    parser.add_argument('--enable-sign-extension', action='store_true')
    parser.add_argument('--enable-multi-value', action='store_true')
    parser.add_argument('--enable-tail-call', action='store_true')
    parser.add_argument('--disable-reference-types', action='store_true')
    parser.add_argument('--enable-memory64', action='store_true')
    parser.add_argument('--enable-multi-memory', action='store_true')
    parser.add_argument('--enable-annotations', action='store_true')
    parser.add_argument('--enable-code-metadata', action='store_true')
    parser.add_argument('--enable-custom-page-sizes', action='store_true')
    # --inline-exports can reorder exports, so skip roundtrip check
    parser.add_argument('--inline-exports', action='store_true',
                        help="write exports inline and skip end-to-end roundtrip check")
    parser.add_argument('--inline-imports', action='store_true')
    parser.add_argument('--reloc', action='store_true')
    parser.add_argument('file', help='test file.')
    options = parser.parse_args(args)

    wat2wasm = utils.Executable(
        find_exe.GetWat2WasmExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    wat2wasm.AppendOptionalArgs({
        '--debug-names': options.debug_names,
        '--enable-exceptions': options.enable_exceptions,
        '--enable-multi-value': options.enable_multi_value,
        '--enable-saturating-float-to-int':
            options.enable_saturating_float_to_int,
        '--enable-sign-extension': options.enable_sign_extension,
        '--enable-function-references': options.enable_function_references,
        '--enable-gc': options.enable_gc,
        '--enable-threads': options.enable_threads,
        '--enable-tail-call': options.enable_tail_call,
        '--disable-reference-types': options.disable_reference_types,
        '--enable-memory64': options.enable_memory64,
        '--enable-multi-memory': options.enable_multi_memory,
        '--enable-annotations': options.enable_annotations,
        '--enable-code-metadata': options.enable_code_metadata,
        '--enable-custom-page-sizes': options.enable_custom_page_sizes,
        '--reloc': options.reloc,
        '--no-check': options.no_check,
    })

    wasm2wat = utils.Executable(
        find_exe.GetWasm2WatExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    wasm2wat.AppendOptionalArgs({
        '--fold-exprs': options.fold_exprs,
        '--enable-exceptions': options.enable_exceptions,
        '--enable-multi-value': options.enable_multi_value,
        '--enable-saturating-float-to-int':
            options.enable_saturating_float_to_int,
        '--enable-sign-extension': options.enable_sign_extension,
        '--enable-tail-call': options.enable_tail_call,
        '--enable-function-references': options.enable_function_references,
        '--enable-gc': options.enable_gc,
        '--disable-reference-types': options.disable_reference_types,
        '--enable-threads': options.enable_threads,
        '--enable-memory64': options.enable_memory64,
        '--enable-multi-memory': options.enable_multi_memory,
        '--enable-annotations': options.enable_annotations,
        '--enable-code-metadata': options.enable_code_metadata,
        '--enable-custom-page-sizes': options.enable_custom_page_sizes,
        '--inline-exports': options.inline_exports,
        '--inline-imports': options.inline_imports,
        '--no-debug-names': not options.debug_names,
        '--generate-names': options.generate_names,
        '--no-check': options.no_check,
    })

    wat2wasm.verbose = options.print_cmd
    wasm2wat.verbose = options.print_cmd

    filename = options.file
    if not os.path.exists(filename):
        sys.stderr.write('File not found: %s\n' % filename)
        return ERROR

    skip_roundtrip_check = options.generate_names or options.inline_exports
    with utils.TempDirectory(options.out_dir, 'roundtrip-') as out_dir:
        result, msg = DoRoundtrip(wat2wasm, wasm2wat, out_dir, filename,
                                  options.verbose, options.stdout, skip_roundtrip_check)
        if result == ERROR:
            sys.stderr.write(msg)
        return result


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except Error as e:
        sys.stderr.write(str(e) + '\n')
        sys.exit(1)
