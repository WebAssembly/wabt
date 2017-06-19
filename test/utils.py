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

from __future__ import print_function
import contextlib
import os
import json
import shutil
import signal
import subprocess
import sys
import tempfile

# Get signal names from numbers in Python
# http://stackoverflow.com/a/2549950
SIGNAMES = dict((k, v) for v, k in reversed(sorted(signal.__dict__.items()))
                if v.startswith('SIG') and not v.startswith('SIG_'))


class Error(Exception):
  pass


class Executable(object):

  def __init__(self, exe, *before_args, **kwargs):
    self.exe = exe
    self.before_args = list(before_args)
    self.after_args = []
    self.basename = kwargs.get('basename',
                               os.path.basename(exe)).replace('.exe', '')
    self.error_cmdline = kwargs.get('error_cmdline', True)
    self.clean_stdout = kwargs.get('clean_stdout')
    self.clean_stderr = kwargs.get('clean_stderr')
    self.verbose = False

  def _RunWithArgsInternal(self, *args, **kwargs):
    cmd = [self.exe] + self.before_args + list(args) + self.after_args
    cmd_str = ' '.join(cmd)
    if self.verbose:
      print(cmd_str)

    if self.error_cmdline:
      err_cmd_str = cmd_str.replace('.exe', '')
    else:
      err_cmd_str = self.basename

    stdout = ''
    stderr = ''
    error = None
    try:
      process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, **kwargs)
      stdout, stderr = process.communicate()
      stdout = stdout.decode('utf-8', 'ignore')
      stderr = stderr.decode('utf-8', 'ignore')
      if self.clean_stdout:
        stdout = self.clean_stdout(stdout)
      if self.clean_stderr:
        stderr = self.clean_stderr(stderr)
      if process.returncode < 0:
        # Terminated by signal
        signame = SIGNAMES.get(-process.returncode, '<unknown>')
        error = Error('Signal raised running "%s": %s\n%s' % (err_cmd_str,
                                                              signame, stderr))
      elif process.returncode > 0:
        error = Error('Error running "%s":\n%s' % (err_cmd_str, stderr))
    except OSError as e:
      error = Error('Error running "%s": %s' % (err_cmd_str, str(e)))
    return stdout, stderr, error

  def RunWithArgsForStdout(self, *args, **kwargs):
    stdout, stderr, error = self._RunWithArgsInternal(*args, **kwargs)
    if error:
      raise error
    return stdout

  def RunWithArgs(self, *args, **kwargs):
    stdout, stderr, error = self._RunWithArgsInternal(*args, **kwargs)
    sys.stdout.write(stdout)
    if error:
      raise error

  def AppendArg(self, arg):
    self.after_args.append(arg)

  def AppendOptionalArgs(self, option_dict):
    for option, value in option_dict.items():
      if value:
        if value is True:
          self.AppendArg(option)
        else:
          self.AppendArg('%s=%s' % (option, value))


@contextlib.contextmanager
def TempDirectory(out_dir, prefix=None):
  if out_dir:
    out_dir_is_temp = False
    if not os.path.exists(out_dir):
      os.makedirs(out_dir)
  else:
    out_dir = tempfile.mkdtemp(prefix=prefix)
    out_dir_is_temp = True

  try:
    yield out_dir
  finally:
    if out_dir_is_temp:
      shutil.rmtree(out_dir)


def ChangeExt(path, new_ext):
  return os.path.splitext(path)[0] + new_ext


def ChangeDir(path, new_dir):
  return os.path.join(new_dir, os.path.basename(path))


def Hexdump(data):
  if type(data) is str:
    data = bytearray(data, 'ascii')

  DUMP_OCTETS_PER_LINE = 16
  DUMP_OCTETS_PER_GROUP = 2

  p = 0
  end = len(data)
  lines = []
  while p < end:
    line_start = p
    line_end = p + DUMP_OCTETS_PER_LINE
    line = '%07x: ' % p
    while p < line_end:
      for i in xrange(DUMP_OCTETS_PER_GROUP):
        if p < end:
          line += '%02x' % data[p]
        else:
          line += '  '
        p += 1
      line += ' '
    line += ' '
    p = line_start
    for i in xrange(DUMP_OCTETS_PER_LINE):
      if p >= end:
        break
      x = data[p]
      if x >= 32 and x < 0x7f:
        line += '%c' % x
      else:
        line += '.'
      p += 1
    line += '\n'
    lines.append(line)

  return lines


def GetModuleFilenamesFromSpecJSON(json_filename):
  with open(json_filename) as json_file:
    json_data = json.load(json_file)
  return [m['filename'] for m in json_data['commands'] if 'filename' in m]
