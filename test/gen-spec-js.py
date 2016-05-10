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
import cStringIO
import json
import os
import sys

from utils import Error

JS_HEADER = """\
var passed = 0;
var failed = 0;
var quiet = false;

"""

MODULE_RUN = """\
testModule%(module_index)s();
"""

MODULE_RUN_FOOTER = """\
end();

"""

MODULE_TEST_HEADER = """\
function testModule%(module_index)s() {
"""

MODULE_DATA_HEADER = """\
  var module = createModule([
"""

MODULE_DATA_FOOTER = """\
  ]);

"""

INVOKE_COMMAND = """\
  invoke(module, '%(name)s');
"""

ASSERT_RETURN_COMMAND = """\
  assertReturn(module, '%(name)s', '%(file)s', %(line)s);
"""

ASSERT_TRAP_COMMAND = """\
  assertTrap(module, '%(name)s', '%(file)s', %(line)s);
"""

COMMANDS = {
    'invoke': INVOKE_COMMAND,
    'assert_return': ASSERT_RETURN_COMMAND,
    'assert_return_nan': ASSERT_RETURN_COMMAND,
    'assert_trap': ASSERT_TRAP_COMMAND
}

MODULE_TEST_FOOTER = """\
}

"""

JS_FOOTER = """\
function createModule(data) {
  var u8a = new Uint8Array(data);
  var ffi = {spectest: {print: print}};
  return Wasm.instantiateModule(u8a, ffi);
}

function assertReturn(module, name, file, line) {
  try {
    var result = module.exports[name]();
  } catch(e) {
    print(file + ":" + line + ": " + name + " unexpectedly threw: " + e);
  }

  if (result == 1) {
    passed++;
  } else {
    print(file + ":" + line + ": " + name + " failed.");
    failed++;
  }
}

function assertTrap(module, name, file, line) {
  var threw = false;
  try {
    module.exports[name]();
  } catch (e) {
    threw = true;
  }

  if (threw) {
    passed++;
  } else {
    print(file + ":" + line + ": " + name + " failed, didn't throw");
    failed++;
  }
}

function invoke(module, name) {
  try {
    var invokeResult = module.exports[name]();
  } catch(e) {
    print(file + ":" + line + ": " + name + " unexpectedly threw: " + e);
  }

  if (!quiet)
    print(name + " = " + invokeResult);
}

function end() {
  if ((failed > 0) || !quiet)
    print(passed + "/" + (passed + failed) + " tests passed.");
}
"""

def ProcessJsonFile(json_file_path):
  json_file_dir = os.path.dirname(json_file_path)
  with open(json_file_path) as json_file:
    json_data = json.load(json_file)

  output = cStringIO.StringIO()
  output.write(JS_HEADER)
  for module_index in range(len(json_data['modules'])):
    output.write(MODULE_RUN % {'module_index': module_index})
  output.write(MODULE_RUN_FOOTER)

  for module_index, module in enumerate(json_data['modules']):
    module_filepath = os.path.join(json_file_dir, module['filename'])
    with open(module_filepath, 'rb') as wasm_file:
      wasm_data = wasm_file.read()

    output.write(MODULE_TEST_HEADER % {'module_index': module_index})
    output.write(MODULE_DATA_HEADER)
    WriteModuleBytes(output, wasm_data)
    output.write(MODULE_DATA_FOOTER)

    for command_index, command in enumerate(module['commands']):
      output.write(COMMANDS[command['type']] % command)
    output.write(MODULE_TEST_FOOTER)
  output.write(JS_FOOTER)

  return output.getvalue()


def WriteModuleBytes(output, module_data):
  BYTES_PER_LINE = 16
  offset = 0
  while True:
    line_data = module_data[offset:offset+BYTES_PER_LINE]
    if not line_data:
      break
    output.write('    ')
    output.write(', '.join('%3d' % ord(byte) for byte in line_data))
    output.write(',\n')
    offset += BYTES_PER_LINE


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-o', '--output', metavar='PATH', help='output file.')
  parser.add_argument('file', help='spec json file.')
  options = parser.parse_args(args)

  output_data = ProcessJsonFile(options.file)
  if options.output:
    output_file = open(options.output, 'w')
  else:
    output_file = sys.stdout

  try:
    output_file.write(output_data)
  finally:
    output_file.close()

  return 0

if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
