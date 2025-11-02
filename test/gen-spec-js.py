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

"""Convert a JSON descrption of a spec test into a JavaScript."""

import argparse
import io
import json
import os
import re
import struct
import sys

import find_exe
from utils import ChangeDir, ChangeExt, Error, Executable
import utils

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

F32_INF = 0x7f800000
F32_NEG_INF = 0xff800000
F32_NEG_ZERO = 0x80000000
F32_SIGN_BIT = F32_NEG_ZERO
F32_SIG_MASK = 0x7fffff
F32_QUIET_NAN = 0x7fc00000
F32_QUIET_NAN_TAG = 0x400000
F64_INF = 0x7ff0000000000000
F64_NEG_INF = 0xfff0000000000000
F64_NEG_ZERO = 0x8000000000000000
F64_SIGN_BIT = F64_NEG_ZERO
F64_SIG_MASK = 0xfffffffffffff
F64_QUIET_NAN = 0x7ff8000000000000
F64_QUIET_NAN_TAG = 0x8000000000000


def I32ToJS(value):
    # JavaScript will return all i32 values as signed.
    if value >= 2**31:
        value -= 2**32
    return str(value)


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


def F32ToJS(f32_bits):
    assert not IsNaNF32(f32_bits)
    if f32_bits == F32_INF:
        return 'Infinity'
    elif f32_bits == F32_NEG_INF:
        return '-Infinity'
    else:
        return 'f32(%s)' % ReinterpretF32(f32_bits)


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


def F64ToJS(f64_bits):
    assert not IsNaNF64(f64_bits)
    if f64_bits == F64_INF:
        return 'Infinity'
    elif f64_bits == F64_NEG_INF:
        return '-Infinity'
    else:
        # Use repr to get full precision
        return repr(ReinterpretF64(f64_bits))


def UnescapeWasmString(s):
    # Wat allows for more escape characters than this, but we assume that
    # wasm2wat will only use the \xx escapes.
    result = ''
    i = 0
    while i < len(s):
        c = s[i]
        if c == '\\':
            x = s[i + 1:i + 3]
            if len(x) != 2:
                raise Error('String with invalid escape: \"%s\"' % s)
            result += chr(int(x, 16))
            i += 3
        else:
            result += c
            i += 1
    return result


def EscapeJSString(s):
    result = ''
    for c in s:
        if 32 <= ord(c) < 127 and c not in '"\\':
            result += c
        else:
            result += '\\x%02x' % ord(c)
    return result


def IsValidJSConstant(const):
    type_ = const['type']
    value = const['value']
    if type_ in ('f32', 'f64') and value in ('nan:canonical', 'nan:arithmetic'):
        return True
    if type_ == 'i32':
        return True
    elif type_ == 'i64':
        return False
    elif type_ == 'f32':
        return not IsNaNF32(int(value))
    elif type_ == 'f64':
        return not IsNaNF64(int(value))


def IsValidJSAction(action):
    return all(IsValidJSConstant(x) for x in action.get('args', []))


def IsValidJSCommand(command):
    type_ = command['type']
    action = command['action']
    if type_ == 'assert_return':
        expected = command['expected']
        return (IsValidJSAction(action) and
                all(IsValidJSConstant(x) for x in expected))
    elif type_ in ('assert_trap', 'assert_exhaustion'):
        return IsValidJSAction(action)


def CollectInvalidModuleCommands(commands):
    modules = []
    module_map = {}
    for command in commands:
        if command['type'] == 'module':
            pair = (command, [])
            modules.append(pair)
            module_name = command.get('name')
            if module_name:
                module_map[module_name] = pair
        elif command['type'] in ('assert_return', 'assert_trap',
                                 'assert_exhaustion'):
            if IsValidJSCommand(command):
                continue

            action = command['action']
            module_name = action.get('module')
            if module_name:
                pair = module_map[module_name]
            else:
                pair = modules[-1]
            pair[1].append(command)
    return modules


class ModuleExtender(object):

    def __init__(self, wat2wasm, wasm2wat, temp_dir):
        self.wat2wasm = wat2wasm
        self.wasm2wat = wasm2wat
        self.temp_dir = temp_dir
        self.lines = []
        self.exports = {}

    def Extend(self, wasm_path, commands):
        wat_path = self._RunWasm2Wat(wasm_path)
        with open(wat_path) as wat_file:
            wat = wat_file.read()

        self.lines = []
        self.exports = self._GetExports(wat)
        for i, command in enumerate(commands):
            self._Command(i, command)

        wat = wat[:wat.rindex(')')] + '\n\n'
        wat += '\n'.join(self.lines) + ')'
        # print wat
        with open(wat_path, 'w') as wat_file:
            wat_file.write(wat)
        return self._RunWat2Wasm(wat_path)

    def _Command(self, index, command):
        command_type = command['type']
        new_field = 'assert_%d' % index
        if command_type == 'assert_return':
            self.lines.append('(func (export "%s")' % new_field)
            self.lines.append('block')
            self._Action(command['action'])
            for expected in command['expected']:
                self._Reinterpret(expected['type'])
                if expected['value'] in ('nan:canonical', 'nan:arithmetic'):
                    self._NanBitmask(expected['value'] == 'nan:canonical', expected['type'])
                    self._And(expected['type'])
                    self._QuietNan(expected['type'])
                else:
                    self._Constant(expected)
                    self._Reinterpret(expected['type'])
                self._Eq(expected['type'])
                self.lines.extend(['i32.eqz', 'br_if 0'])
            self.lines.extend(['return', 'end', 'unreachable', ')'])
        elif command_type in ('assert_trap', 'assert_exhaustion'):
            self.lines.append('(func (export "%s")' % new_field)
            self._Action(command['action'])
            self.lines.extend(['br 0', ')'])
        else:
            raise Error('Unexpected command: %s' % command_type)

        # Update command to point to the new exported function.
        command['action']['field'] = new_field
        command['action']['args'] = []
        command['expected'] = []

    def _GetExports(self, wat):
        result = {}
        pattern = r'^\s*\(export \"(.*?)\"\s*\((\w+) (\d+)'
        for name, type_, index in re.findall(pattern, wat, re.MULTILINE):
            result[UnescapeWasmString(name)] = (type_, index)
        return result

    def _Action(self, action):
        export = self.exports[action['field']]
        if action['type'] == 'invoke':
            for arg in action['args']:
                self._Constant(arg)
            self.lines.append('call %s' % export[1])
        elif action['type'] == 'get':
            self.lines.append('global.get %s' % export[1])
        else:
            raise Error('Unexpected action: %s' % action['type'])

    def _Reinterpret(self, type_):
        self.lines.extend({
            'i32': [],
            'i64': [],
            'f32': ['i32.reinterpret_f32'],
            'f64': ['i64.reinterpret_f64']
        }[type_])

    def _Eq(self, type_):
        self.lines.append({
            'i32': 'i32.eq',
            'i64': 'i64.eq',
            'f32': 'i32.eq',
            'f64': 'i64.eq'
        }[type_])

    def _And(self, type_):
        self.lines.append({
            'i32': 'i32.and',
            'i64': 'i64.and',
            'f32': 'i32.and',
            'f64': 'i64.and'
        }[type_])

    def _NanBitmask(self, canonical, type_):
        # When checking for canonical NaNs, the value can differ only in the sign
        # bit from +nan. For arithmetic NaNs, the sign bit and the rest of the tag
        # can differ as well.
        assert type_ in ('f32', 'f64')
        if not canonical:
            return self._QuietNan(type_)

        if type_ == 'f32':
            line = 'i32.const 0x7fffffff'
        else:
            line = 'i64.const 0x7fffffffffffffff'
        self.lines.append(line)

    def _QuietNan(self, type_):
        assert type_ in ('f32', 'f64')
        if type_ == 'f32':
            line = 'i32.const 0x%x' % F32_QUIET_NAN
        else:
            line = 'i64.const 0x%x' % F64_QUIET_NAN
        self.lines.append(line)

    def _Constant(self, const):
        inst = None
        type_ = const['type']
        value = const['value']
        assert value not in ('nan:canonical', 'nan:arithmetic')
        if type_ == 'i32':
            inst = 'i32.const %s' % value
        elif type_ == 'i64':
            inst = 'i64.const %s' % value
        elif type_ == 'f32':
            inst = F32ToWasm(int(value))
        elif type_ == 'f64':
            inst = F64ToWasm(int(value))
        self.lines.append(inst)

    def _RunWasm2Wat(self, wasm_path):
        wat_path = ChangeDir(ChangeExt(wasm_path, '.wat'), self.temp_dir)
        self.wasm2wat.RunWithArgs(wasm_path, '-o', wat_path)
        return wat_path

    def _RunWat2Wasm(self, wat_path):
        wasm_path = ChangeDir(ChangeExt(wat_path, '.wasm'), self.temp_dir)
        self.wat2wasm.RunWithArgs(wat_path, '-o', wasm_path)
        return wasm_path


class JSWriter(object):

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
        self.out_file.write('// %s:%d\n' % (self.source_filename, command['line']))

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

    def _ModuleIdxName(self):
        return '$%d' % self.module_idx

    def _WriteModuleCommand(self, command):
        self.module_idx += 1
        idx_name = self._ModuleIdxName()

        self.out_file.write('let %s = instance("%s");\n' %
                            (idx_name, self._Module(command['filename'])))
        if 'name' in command:
            self.out_file.write('let %s = %s;\n' % (command['name'], idx_name))

    def _WriteActionCommand(self, command):
        self.out_file.write('%s;\n' % self._Action(command['action']))

    def _WriteRegisterCommand(self, command):
        self.out_file.write('register("%s", %s)\n' % (
            command['as'], command.get('name', self._ModuleIdxName())))

    def _WriteAssertModuleCommand(self, command):
        # Don't bother writing out text modules; they can't be parsed by JS.
        if command['module_type'] == 'binary':
            self.out_file.write('%s("%s");\n' % (command['type'],
                                self._Module(command['filename'])))

    def _WriteAssertReturnCommand(self, command):
        expected = command['expected']
        if len(expected) == 1:
            self.out_file.write('assert_return(() => %s, %s);\n' %
                                (self._Action(command['action']),
                                 self._ConstantList(expected)))
        elif len(expected) == 0:
            self._WriteAssertActionCommand(command)
        else:
            raise Error('Unexpected result with multiple values: %s' % expected)

    def _WriteAssertActionCommand(self, command):
        self.out_file.write('%s(() => %s);\n' % (command['type'],
                            self._Action(command['action'])))

    def _Module(self, filename):
        with open(os.path.join(self.base_dir, filename), 'rb') as wasm_file:
            return ''.join('\\x%02x' % c for c in bytearray(wasm_file.read()))

    def _Constant(self, const):
        assert IsValidJSConstant(const), 'Invalid JS const: %s' % const
        type_ = const['type']
        value = const['value']
        if type_ in ('f32', 'f64') and value in ('nan:canonical', 'nan:arithmetic'):
            return value
        if type_ == 'i32':
            return I32ToJS(int(value))
        elif type_ == 'f32':
            return F32ToJS(int(value))
        elif type_ == 'f64':
            return F64ToJS(int(value))
        else:
            assert False

    def _ConstantList(self, consts):
        return ', '.join(self._Constant(const) for const in consts)

    def _Action(self, action):
        type_ = action['type']
        module = action.get('module', self._ModuleIdxName())
        field = EscapeJSString(action['field'])
        if type_ == 'invoke':
            args = '[%s]' % self._ConstantList(action.get('args', []))
            return 'call(%s, "%s", %s)' % (module, field, args)
        elif type_ == 'get':
            return 'get(%s, "%s")' % (module, field)
        else:
            raise Error('Unexpected action type: %s' % type_)


def main(args):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-o', '--output', metavar='PATH', help='output file.')
    parser.add_argument('-P', '--prefix', metavar='PATH', help='prefix file.',
                        default=os.path.join(SCRIPT_DIR, 'gen-spec-prefix.js'))
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
    parser.add_argument('file', help='spec json file.')
    options = parser.parse_args(args)

    wat2wasm = Executable(
        find_exe.GetWat2WasmExecutable(options.bindir),
        error_cmdline=options.error_cmdline)
    wasm2wat = Executable(
        find_exe.GetWasm2WatExecutable(options.bindir),
        error_cmdline=options.error_cmdline)

    wat2wasm.verbose = options.print_cmd
    wasm2wat.verbose = options.print_cmd

    with open(options.file) as json_file:
        json_dir = os.path.dirname(options.file)
        spec_json = json.load(json_file)
        all_commands = spec_json['commands']

    # modules is a list of pairs: [(module_command, [assert_command, ...]), ...]
    modules = CollectInvalidModuleCommands(all_commands)

    with utils.TempDirectory(options.temp_dir, 'gen-spec-js-') as temp_dir:
        extender = ModuleExtender(wat2wasm, wasm2wat, temp_dir)
        for module_command, assert_commands in modules:
            if assert_commands:
                wasm_path = os.path.join(json_dir, module_command['filename'])
                new_module_filename = extender.Extend(wasm_path, assert_commands)
                module_command['filename'] = new_module_filename

        output = io.StringIO()
        if options.prefix:
            with open(options.prefix) as prefix_file:
                output.write(prefix_file.read())
                output.write('\n')

        JSWriter(json_dir, spec_json, output).Write()

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
