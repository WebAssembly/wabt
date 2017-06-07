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
from collections import namedtuple
try:
  from cStringIO import StringIO
except ImportError:
  from io import StringIO
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
  # Wast allows for more escape characters than this, but we assume that
  # wasm2wast will only use the \xx escapes.
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
  if type_ == 'i32':
    return True
  elif type_ == 'i64':
    return False
  elif type_ == 'f32':
    return not IsNaNF32(int(const['value']))
  elif type_ == 'f64':
    return not IsNaNF64(int(const['value']))


def IsValidJSAction(action):
  return all(IsValidJSConstant(x) for x in action.get('args', []))


def IsValidJSCommand(command):
  type_ = command['type']
  action = command['action']
  if type_ == 'assert_return':
    expected = command['expected']
    return (IsValidJSAction(action) and
            all(IsValidJSConstant(x) for x in expected))
  elif type_ in ('assert_return_canonical_nan', 'assert_return_arithmetic_nan',
                 'assert_trap', 'assert_exhaustion'):
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
    elif command['type'] in (
        'assert_return', 'assert_return_canonical_nan',
        'assert_return_arithmetic_nan', 'assert_trap', 'assert_exhaustion'):
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

  def __init__(self, wast2wasm, wasm2wast, temp_dir):
    self.wast2wasm = wast2wasm
    self.wasm2wast = wasm2wast
    self.temp_dir = temp_dir
    self.lines = []
    self.exports = {}

  def Extend(self, wasm_path, commands):
    wast_path = self._RunWasm2Wast(wasm_path)
    with open(wast_path) as wast_file:
      wast = wast_file.read()

    self.lines = []
    self.exports = self._GetExports(wast)
    for i, command in enumerate(commands):
      self._Command(i, command)

    wast = wast[:wast.rindex(')')] + '\n\n'
    wast += '\n'.join(self.lines) + ')'
    # print wast
    with open(wast_path, 'w') as wast_file:
      wast_file.write(wast)
    return self._RunWast2Wasm(wast_path)

  def _Command(self, index, command):
    command_type = command['type']
    new_field = 'assert_%d' % index
    if command_type == 'assert_return':
      self.lines.append('(func (export "%s")' % new_field)
      self.lines.append('block')
      self._Action(command['action'])
      for expected in command['expected']:
        self._Reinterpret(expected['type'])
        self._Constant(expected)
        self._Reinterpret(expected['type'])
        self._Eq(expected['type'])
        self.lines.extend(['i32.eqz', 'br_if 0'])
      self.lines.extend(['return', 'end', 'unreachable', ')'])
    elif command_type == 'assert_return_canonical_nan':
      self._AssertReturnNan(new_field, command, True)
    elif command_type == 'assert_return_arithmetic_nan':
      self._AssertReturnNan(new_field, command, False)
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

  def _AssertReturnNan(self, new_field, command, canonical):
      type_ = command['expected'][0]['type']
      self.lines.append('(func (export "%s")' % new_field)
      self.lines.append('block')
      self._Action(command['action'])
      self._Reinterpret(type_)
      self._NanBitmask(canonical, type_)
      self._And(type_)
      self._QuietNan(type_)
      self._Eq(type_)
      self.lines.extend(
          ['i32.eqz', 'br_if 0', 'return', 'end', 'unreachable', ')'])

      # Change the command to assert_return, it won't return NaN anymore.
      command['type'] = 'assert_return'

  def _GetExports(self, wast):
    result = {}
    pattern = r'^\s*\(export \"(.*?)\"\s*\((\w+) (\d+)'
    for name, type_, index in re.findall(pattern, wast, re.MULTILINE):
      result[UnescapeWasmString(name)] = (type_, index)
    return result

  def _Action(self, action):
    export = self.exports[action['field']]
    if action['type'] == 'invoke':
      for arg in action['args']:
        self._Constant(arg)
      self.lines.append('call %s' % export[1])
    elif action['type'] == 'get':
      self.lines.append('get_global %s' % export[1])
    else:
      raise Error('Unexpected action: %s' % action['type'])

  def _Reinterpret(self, type_):
    self.lines.extend({
        'i32': [],
        'i64': [],
        'f32': ['i32.reinterpret/f32'],
        'f64': ['i64.reinterpret/f64']
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
    assert(type_ in ('f32', 'f64'))
    if not canonical:
      return self._QuietNan(type_)

    if type_ == 'f32':
      line = 'i32.const 0x7fffffff'
    else:
      line = 'i64.const 0x7fffffffffffffff'
    self.lines.append(line)

  def _QuietNan(self, type_):
    assert(type_ in ('f32', 'f64'))
    if type_ == 'f32':
      line = 'i32.const 0x%x' % F32_QUIET_NAN
    else:
      line = 'i64.const 0x%x' % F64_QUIET_NAN
    self.lines.append(line)

  def _Constant(self, const):
    inst = None
    type_ = const['type']
    if type_ == 'i32':
      inst = 'i32.const %s' % const['value']
    elif type_ == 'i64':
      inst = 'i64.const %s' % const['value']
    elif type_ == 'f32':
      inst = F32ToWasm(int(const['value']))
    elif type_ == 'f64':
      inst = F64ToWasm(int(const['value']))
    self.lines.append(inst)

  def _RunWasm2Wast(self, wasm_path):
    wast_path = ChangeDir(ChangeExt(wasm_path, '.wast'), self.temp_dir)
    self.wasm2wast.RunWithArgs(wasm_path, '-o', wast_path)
    return wast_path

  def _RunWast2Wasm(self, wast_path):
    wasm_path = ChangeDir(ChangeExt(wast_path, '.wasm'), self.temp_dir)
    self.wast2wasm.RunWithArgs(wast_path, '-o', wasm_path)
    return wasm_path


class JSWriter(object):

  def __init__(self, base_dir, commands, out_file):
    self.base_dir = base_dir
    self.commands = commands
    self.out_file = out_file

  def Write(self):
    for command in self.commands:
      self._WriteCommand(command)

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
        'assert_return_canonical_nan': self._WriteAssertActionCommand,
        'assert_return_arithmetic_nan': self._WriteAssertActionCommand,
        'assert_trap': self._WriteAssertActionCommand,
        'assert_exhaustion': self._WriteAssertActionCommand,
    }

    func = command_funcs.get(command['type'])
    if func is None:
      raise Error('Unexpected type: %s' % command['type'])
    func(command)

  def _WriteModuleCommand(self, command):
    if 'name' in command:
      self.out_file.write('let %s = ' % command['name'])
    self.out_file.write('$$ = instance("%s");\n' %
                        self._Module(command['filename']))

  def _WriteActionCommand(self, command):
    self.out_file.write('%s;\n' % self._Action(command['action']))

  def _WriteRegisterCommand(self, command):
    self.out_file.write('register("%s", %s)\n' % (command['as'],
                                                  command.get('name', '$$')))

  def _WriteAssertModuleCommand(self, command):
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
    value = int(const['value'])
    if type_ == 'i32':
      return I32ToJS(value)
    elif type_ == 'f32':
      return F32ToJS(value)
    elif type_ == 'f64':
      return F64ToJS(value)
    else:
      assert False

  def _ConstantList(self, consts):
    return ', '.join(self._Constant(const) for const in consts)

  def _Action(self, action):
    type_ = action['type']
    if type_ not in ('invoke', 'get'):
      raise Error('Unexpected action type: %s' % type_)

    args = ''
    if type_ == 'invoke':
      args = '(%s)' % self._ConstantList(action.get('args', []))

    return '%s.exports["%s"]%s' % (action.get('module', '$$'),
                                   EscapeJSString(action['field']), args)


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--output', metavar='PATH', help='output file.')
  parser.add_argument('-P', '--prefix', metavar='PATH', help='prefix file.',
                      default=os.path.join(SCRIPT_DIR, 'gen-spec-prefix.js'))
  parser.add_argument('--bindir', metavar='PATH',
                      default=find_exe.GetDefaultPath(),
                      help='directory to search for all executables.')
  parser.add_argument('--temp-dir', metavar='PATH',
                      help='set the directory that temporary wasm/wast'
                      ' files are written.')
  parser.add_argument('--no-error-cmdline',
                      help='don\'t display the subprocess\'s commandline when'
                      + ' an error occurs', dest='error_cmdline',
                      action='store_false')
  parser.add_argument('-p', '--print-cmd',
                      help='print the commands that are run.',
                      action='store_true')
  parser.add_argument('file', help='spec json file.')
  options = parser.parse_args(args)

  wast2wasm = Executable(
      find_exe.GetWast2WasmExecutable(options.bindir),
      error_cmdline=options.error_cmdline)
  wasm2wast = Executable(
      find_exe.GetWasm2WastExecutable(options.bindir),
      error_cmdline=options.error_cmdline)

  wast2wasm.verbose = options.print_cmd
  wasm2wast.verbose = options.print_cmd

  with open(options.file) as json_file:
    json_dir = os.path.dirname(options.file)
    spec_json = json.load(json_file)
    all_commands = spec_json['commands']

  # modules is a list of pairs: [(module_command, [assert_command, ...]), ...]
  modules = CollectInvalidModuleCommands(all_commands)

  temp_dir = options.temp_dir
  assert(temp_dir)

  extender = ModuleExtender(wast2wasm, wasm2wast, temp_dir)
  for module_command, assert_commands in modules:
    if assert_commands:
      wasm_path = os.path.join(json_dir, module_command['filename'])
      new_module_filename = extender.Extend(wasm_path, assert_commands)
      module_command['filename'] = os.path.relpath(new_module_filename,
                                                   json_dir)

  output = StringIO()
  if options.prefix:
    with open(options.prefix) as prefix_file:
      output.write(prefix_file.read())
      output.write('\n')

  JSWriter(json_dir, all_commands, output).Write()

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
