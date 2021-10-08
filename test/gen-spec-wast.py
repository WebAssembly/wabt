#!/usr/bin/env python3
#
# Copyright 2020 WebAssembly Community Group participants
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
import json
import io
import os
import struct
import sys

import find_exe
from utils import ChangeDir, Error, Executable
import utils

F32_INF = 0x7f800000
F32_NEG_INF = 0xff800000
F32_NEG_ZERO = 0x80000000
F32_SIGN_BIT = F32_NEG_ZERO
F32_SIG_MASK = 0x7fffff
F32_QUIET_NAN_TAG = 0x400000
F64_INF = 0x7ff0000000000000
F64_NEG_INF = 0xfff0000000000000
F64_NEG_ZERO = 0x8000000000000000
F64_SIGN_BIT = F64_NEG_ZERO
F64_SIG_MASK = 0xfffffffffffff
F64_QUIET_NAN_TAG = 0x8000000000000


def EscapeWasmString(s):
    result = ''
    for c in s:
        if c == 34:
            result += '\\"'
        elif c == 92:
            result += '\\\\'
        elif 32 <= c < 127:
            result += chr(c)
        else:
            result += '\\%02x' % c
    return result


def IsNaNF32(f32_bits):
    return (F32_INF < f32_bits < F32_NEG_ZERO) or (f32_bits > F32_NEG_INF)


def ReinterpretF32(f32_bits):
    return struct.unpack('<f', struct.pack('<I', f32_bits))[0]


def NaNF32ToString(f32_bits):
    result = '-' if f32_bits & F32_SIGN_BIT else ''
    result += 'nan'
    sig = f32_bits & F32_SIG_MASK
    if sig != F32_QUIET_NAN_TAG:
        result += ':0x%x' % sig
    return result


def F32ToWasm(f32_bits):
    if IsNaNF32(f32_bits):
        return 'f32.const %s' % NaNF32ToString(f32_bits)
    elif f32_bits == F32_INF:
        return 'f32.const infinity'
    elif f32_bits == F32_NEG_INF:
        return 'f32.const -infinity'
    else:
        return 'f32.const %s' % float.hex(ReinterpretF32(f32_bits))


def IsNaNF64(f64_bits):
    return (F64_INF < f64_bits < F64_NEG_ZERO) or (f64_bits > F64_NEG_INF)


def ReinterpretF64(f64_bits):
    return struct.unpack('<d', struct.pack('<Q', f64_bits))[0]


def NaNF64ToString(f64_bits):
    result = '-' if f64_bits & F64_SIGN_BIT else ''
    result += 'nan'
    sig = f64_bits & F64_SIG_MASK
    if sig != F64_QUIET_NAN_TAG:
        result += ':0x%x' % sig
    return result


def F64ToWasm(f64_bits):
    if IsNaNF64(f64_bits):
        return 'f64.const %s' % NaNF64ToString(f64_bits)
    elif f64_bits == F64_INF:
        return 'f64.const infinity'
    elif f64_bits == F64_NEG_INF:
        return 'f64.const -infinity'
    else:
        return 'f64.const %s' % float.hex(ReinterpretF64(f64_bits))


class WastWriter(object):

    def __init__(self, base_dir, spec_json, out_file):
        self.base_dir = base_dir
        self.source_filename = os.path.basename(spec_json['source_filename'])
        self.commands = spec_json['commands']
        self.out_file = out_file
        self.module_idx = 0

    def Write(self):
        for command in self.commands:
            self._WriteCommand(command)

    def _WriteFileAndLine(self, command):
        self.out_file.write(';; %s:%d\n' % (self.source_filename, command['line']))

    def _WriteCommand(self, command):
        command_funcs = {
            'module': self._WriteModuleCommand,
            'action': self._WriteActionCommand,
            'register': self._WriteRegisterCommand,
            'assert_malformed': self._WriteAssertModuleCommand,
            'assert_invalid': self._WriteAssertModuleCommand,
            'assert_unlinkable': self._WriteAssertModuleCommand,
            'assert_uninstantiable': self._WriteAssertModuleCommand,
            'assert_return': self._WriteAssertReturnCommand,
            'assert_trap': self._WriteAssertActionCommand,
            'assert_exhaustion': self._WriteAssertActionCommand,
        }

        func = command_funcs.get(command['type'])
        if func is None:
            raise Error('Unexpected type: %s' % command['type'])
        self._WriteFileAndLine(command)
        func(command)
        self.out_file.write('\n')

    def _WriteModuleCommand(self, command):
        self.out_file.write('(module %s binary \"%s\")\n' % (
            command.get('name', ''), self._Binary(command['filename'])))

    def _WriteActionCommand(self, command):
        self.out_file.write('(action %s)\n' % self._Action(command['action']))

    def _WriteRegisterCommand(self, command):
        self.out_file.write('(register "%s" %s)\n' % (
            command['as'], command.get('name', '')))

    def _WriteAssertModuleCommand(self, command):
        if command['module_type'] == 'binary':
            self.out_file.write('(%s (module binary "%s") "%s")\n' % (
                command['type'], self._Binary(command['filename']), command['text']))
        elif command['module_type'] == 'text':
            self.out_file.write('(%s (module quote "%s") "%s")\n' % (
                command['type'], self._Text(command['filename']), command['text']))
        else:
            raise Error('Unknown module type: %s' % command['module_type'])

    def _WriteAssertReturnCommand(self, command):
        expected = command['expected']
        self.out_file.write('(assert_return %s%s)\n' % (
            self._Action(command['action']), self._ConstantList(expected)))

    def _WriteAssertActionCommand(self, command):
        self.out_file.write('(%s %s)\n' % (
            command['type'], self._Action(command['action'])))

    def _Binary(self, filename):
        with open(os.path.join(self.base_dir, filename), 'rb') as wasm_file:
            return ''.join('\\%02x' % c for c in wasm_file.read())

    def _Text(self, filename):
        with open(os.path.join(self.base_dir, filename), 'rb') as wasm_file:
            return '%s' % EscapeWasmString(wasm_file.read())

    def _Constant(self, const):
        type_ = const['type']
        value = const['value']
        if type_ in ('f32', 'f64') and value in ('nan:canonical', 'nan:arithmetic'):
            return value
        if type_ == 'i32':
            return 'i32.const %s' % value
        elif type_ == 'i64':
            return 'i64.const %s' % value
        elif type_ == 'f32':
            return F32ToWasm(int(value))
        elif type_ == 'f64':
            return F64ToWasm(int(value))
        elif type_ == 'externref':
            return 'ref.extern %s' % value
        elif type_ == 'funcref':
            return 'ref.func %s' % value
        else:
            raise Error('Unknown type: %s' % type_)

    def _ConstantList(self, consts):
        if consts:
            return ' ' + (' '.join('(%s)' % self._Constant(const) for const in consts))
        return ''

    def _Action(self, action):
        type_ = action['type']
        module = action.get('module', '')
        field = action['field']
        if type_ == 'invoke':
            args = self._ConstantList(action.get('args', []))
            return '(invoke %s "%s"%s)' % (module, field, args)
        elif type_ == 'get':
            return '(get %s "%s")' % (module, field)
        else:
            raise Error('Unexpected action type: %s' % type_)


def main(args):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-o', '--output', metavar='PATH', help='output file.')
    parser.add_argument('file', help='spec json file.')
    parser.add_argument('--bindir', metavar='PATH',
                        default=find_exe.GetDefaultPath(),
                        help='directory to search for all executables.')
    parser.add_argument('--temp-dir', metavar='PATH',
                        help='set the directory that temporary wasm/wat'
                        ' files are written.')
    parser.add_argument('--no-error-cmdline',
                        help='don\'t display the subprocess\'s commandline when'
                        ' an error occurs', dest='error_cmdline',
                        action='store_false')
    parser.add_argument('-p', '--print-cmd',
                        help='print the commands that are run.',
                        action='store_true')
    parser.add_argument('--enable-exceptions', action='store_true')
    parser.add_argument('--enable-saturating-float-to-int', action='store_true')
    parser.add_argument('--enable-threads', action='store_true')
    parser.add_argument('--enable-sign-extension', action='store_true')
    parser.add_argument('--enable-multi-value', action='store_true')
    parser.add_argument('--enable-tail-call', action='store_true')
    parser.add_argument('--enable-reference-types', action='store_true')
    parser.add_argument('--enable-memory64', action='store_true')
    options = parser.parse_args(args)

    wast2json = Executable(
        find_exe.GetWast2JsonExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    wast2json.verbose = options.print_cmd
    wast2json.AppendOptionalArgs({
        '--enable-exceptions': options.enable_exceptions,
        '--enable-multi-value': options.enable_multi_value,
        '--enable-saturating-float-to-int':
            options.enable_saturating_float_to_int,
        '--enable-sign-extension': options.enable_sign_extension,
        '--enable-threads': options.enable_threads,
        '--enable-tail-call': options.enable_tail_call,
        '--enable-reference-types': options.enable_reference_types,
        '--enable-memory64': options.enable_memory64,
    })

    json_filename = options.file

    with utils.TempDirectory(options.temp_dir, 'gen-spec-wast-') as temp_dir:
        file_base, file_ext = os.path.splitext(json_filename)
        if file_ext == '.wast':
            wast_filename = options.file
            json_filename = ChangeDir(file_base + '.json', temp_dir)
            wast2json.RunWithArgs(wast_filename, '-o', json_filename)

        with open(json_filename) as json_file:
            json_dir = os.path.dirname(json_filename)
            spec_json = json.load(json_file)

        output = io.StringIO()
        WastWriter(json_dir, spec_json, output).Write()

    if options.output:
        out_file = open(options.output, 'w')
    else:
        out_file = sys.stdout

    try:
        out_file.write(output.getvalue())
    finally:
        out_file.close()

    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except Error as e:
        sys.stderr.write(str(e) + '\n')
        sys.exit(1)
