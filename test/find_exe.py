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

import os
import sys

from utils import Error

IS_WINDOWS = sys.platform == 'win32'
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(SCRIPT_DIR)
EXECUTABLES = [
    'wat2wasm', 'wast2json', 'wasm2wat', 'wasm-objdump', 'wasm-interp',
    'wasm-opcodecnt', 'wat-desugar', 'wasm-link', 'spectest-interp',
    'wasm-validate', 'wasm2c',
]

GEN_WASM_PY = os.path.join(SCRIPT_DIR, 'gen-wasm.py')
GEN_SPEC_JS_PY = os.path.join(SCRIPT_DIR, 'gen-spec-js.py')


def GetDefaultPath():
  return os.path.join(REPO_ROOT_DIR, 'bin')


def GetDefaultExe(basename):
  result = os.path.join(GetDefaultPath(), basename)
  if IS_WINDOWS:
    result += '.exe'
  return result


def FindExeWithFallback(name, default_exe_list, override_exe=None):
  result = override_exe
  if result is not None:
    if os.path.isdir(result):
      result = os.path.join(result, name)
    if IS_WINDOWS and os.path.splitext(result)[1] != '.exe':
      result += '.exe'
    if os.path.exists(result):
      return os.path.abspath(result)
    raise Error('%s executable not found.\nsearch path: %s\n' % (name, result))

  for result in default_exe_list:
    if os.path.exists(result):
      return os.path.abspath(result)

  raise Error('%s executable not found.\n%s\n' %
              (name, '\n'.join('search path: %s' % path
                               for path in default_exe_list)))


def FindExecutable(basename, override=None):
  return FindExeWithFallback(basename, [GetDefaultExe(basename)], override)


def GetWat2WasmExecutable(override=None):
  return FindExecutable('wat2wasm', override)


def GetWast2JsonExecutable(override=None):
  return FindExecutable('wast2json', override)


def GetWasm2WatExecutable(override=None):
  return FindExecutable('wasm2wat', override)


def GetWasmdumpExecutable(override=None):
  return FindExecutable('wasm-objdump', override)


def GetWasmlinkExecutable(override=None):
  return FindExecutable('wasm-link', override)


def GetWasmInterpExecutable(override=None):
  return FindExecutable('wasm-interp', override)


def GetSpectestInterpExecutable(override=None):
  return FindExecutable('spectest-interp', override)


def GetWasmOpcodeCntExecutable(override=None):
  return FindExecutable('wasm-opcodecnt', override)


def GetWatDesugarExecutable(override=None):
  return FindExecutable('wat-desugar', override)


def GetWasmValidateExecutable(override=None):
  return FindExecutable('wasm-validate', override)


def GetWasm2CExecutable(override=None):
  return FindExecutable('wasm2c', override)
