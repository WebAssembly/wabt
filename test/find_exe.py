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
    'wast2wasm', 'wasm2wast', 'wasm-objdump', 'wasm-interp', 'wasm-opcodecnt',
    'wast-desugar', 'wasm-link'
]


def GetDefaultPath():
  return os.path.join(REPO_ROOT_DIR, 'out')


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


def GetWast2WasmExecutable(override=None):
  return FindExecutable('wast2wasm', override)


def GetWasm2WastExecutable(override=None):
  return FindExecutable('wasm2wast', override)


def GetWasmdumpExecutable(override=None):
  return FindExecutable('wasm-objdump', override)


def GetWasmlinkExecutable(override=None):
  return FindExecutable('wasm-link', override)


def GetWasmInterpExecutable(override=None):
  return FindExecutable('wasm-interp', override)


def GetWasmOpcodeCntExecutable(override=None):
  return FindExecutable('wasm-opcodecnt', override)


def GetWastDesugarExecutable(override=None):
  return FindExecutable('wast-desugar', override)
