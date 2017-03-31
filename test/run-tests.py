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
import argparse
import difflib
import fnmatch
import multiprocessing
import os
import re
import shlex
import shutil
import subprocess
import sys
import threading
import time

print(sys.path)

import find_exe
from utils import Error

IS_WINDOWS = sys.platform == 'win32'
TEST_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT_DIR = os.path.dirname(TEST_DIR)
OUT_DIR = os.path.join(REPO_ROOT_DIR, 'out')
ROUNDTRIP_PY = os.path.join(TEST_DIR, 'run-roundtrip.py')
DEFAULT_TIMEOUT = 10  # seconds
SLOW_TIMEOUT_MULTIPLIER = 2

# default configurations for tests
TOOLS = {
    'wast2wasm': {
        'EXE': '%(wast2wasm)s',
        'VERBOSE-FLAGS': ['-v']
    },
    'wast-desugar': {
        'EXE': '%(wast-desugar)s'
    },
    'run-wasmdump': {
        'EXE': 'test/run-wasmdump.py',
        'FLAGS': ['--bindir=%(bindir)s', '--no-error-cmdline'],
        'VERBOSE-FLAGS': ['-v']
    },
    'run-wasm-link': {
        'EXE': 'test/run-wasm-link.py',
        'FLAGS': ['--bindir=%(bindir)s'],
        'VERBOSE-FLAGS': ['-v']
    },
    'run-roundtrip': {
        'EXE': 'test/run-roundtrip.py',
        'FLAGS': [
                '-v',
                '--bindir=%(bindir)s',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
        ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-interp': {
        'EXE': 'test/run-interp.py',
        'FLAGS': [
                '--bindir=%(bindir)s',
                '--run-all-exports',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
        ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-interp-spec': {
        'EXE': 'test/run-interp.py',
        'FLAGS': [
                '--bindir=%(bindir)s',
                '--spec',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
         ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-gen-wasm': {
        'EXE': 'test/run-gen-wasm.py',
        'FLAGS': [
                '--bindir=%(bindir)s',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
         ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-gen-wasm-interp': {
        'EXE': 'test/run-gen-wasm-interp.py',
        'FLAGS': [
                '--bindir=%(bindir)s',
                '--run-all-exports',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
        ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-opcodecnt': {
        'EXE': 'test/run-opcodecnt.py',
        'FLAGS': ['--bindir=%(bindir)s', '--no-error-cmdline'],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
    'run-gen-spec-js': {
        'EXE': 'test/run-gen-spec-js.py',
        'FLAGS': [
                '--bindir=%(bindir)s',
                '--no-error-cmdline',
                '-o',
                '%(out_dir)s',
        ],
        'VERBOSE-FLAGS': ['--print-cmd', '-v']
    },
}

ROUNDTRIP_TOOLS = ('wast2wasm',)


def Indent(s, spaces):
  return ''.join(' ' * spaces + l for l in s.splitlines(1))


def DiffLines(expected, actual):
  expected_lines = [line for line in expected.splitlines() if line]
  actual_lines = [line for line in actual.splitlines() if line]
  return list(
      difflib.unified_diff(expected_lines, actual_lines, fromfile='expected',
                           tofile='actual', lineterm=''))


def AppendBeforeExt(file_path, suffix):
  file_path_noext, ext = os.path.splitext(file_path)
  return file_path_noext + suffix + ext


class Cell(object):

  def __init__(self, value):
    self.value = [value]

  def Set(self, value):
    self.value[0] = value

  def Get(self):
    return self.value[0]


def RunCommandWithTimeout(command, cwd, timeout, console_out=False):
  process = None
  is_timeout = Cell(False)

  def KillProcess(timeout=True):
    if process:
      try:
        if IS_WINDOWS:
          # http://stackoverflow.com/a/10830753: deleting child processes in
          # Windows
          subprocess.call(['taskkill', '/F', '/T', '/PID', str(process.pid)])
        else:
          os.killpg(os.getpgid(process.pid), 15)
      except OSError:
        pass
    is_timeout.Set(timeout)

  try:
    start_time = time.time()
    kwargs = {}
    if not IS_WINDOWS:
      kwargs['preexec_fn'] = os.setsid

    # http://stackoverflow.com/a/10012262: subprocess with a timeout
    # http://stackoverflow.com/a/22582602: kill subprocess and children
    process = subprocess.Popen(command, cwd=cwd, stdout=None if console_out
                               else subprocess.PIPE, stderr=None if console_out
                               else subprocess.PIPE, universal_newlines=True,
                               **kwargs)
    timer = threading.Timer(timeout, KillProcess)
    try:
      timer.start()
      stdout, stderr = process.communicate()
    finally:
      returncode = process.returncode
      process = None
      timer.cancel()
    if is_timeout.Get():
      raise Error('TIMEOUT\nSTDOUT:\n%s\nSTDERR:\n%s\n' % (stdout, stderr))
    duration = time.time() - start_time
  except OSError as e:
    raise Error(str(e))
  finally:
    KillProcess(False)

  return stdout, stderr, returncode, duration


class TestInfo(object):

  def __init__(self):
    self.filename = ''
    self.header = []
    self.input_filename = ''
    self.input_ = ''
    self.expected_stdout = ''
    self.expected_stderr = ''
    self.tool = 'wast2wasm'
    self.exe = '%(wast2wasm)s'
    self.flags = []
    self.last_cmd = ''
    self.expected_error = 0
    self.slow = False
    self.skip = False
    self.is_roundtrip = False

  def CreateRoundtripInfo(self):
    result = TestInfo()
    result.filename = self.filename
    result.header = self.header
    result.input_filename = self.input_filename
    result.input_ = self.input_
    result.expected_stdout = ''
    result.expected_stderr = ''
    result.tool = 'run-roundtrip'
    result.exe = ROUNDTRIP_PY
    result.flags = ['--bindir', '%(bindir)s', '-v']
    result.expected_error = 0
    result.slow = self.slow
    result.skip = self.skip
    result.is_roundtrip = True
    return result

  def GetName(self):
    name = self.filename
    if self.is_roundtrip:
      name += ' (roundtrip)'
    return name

  def GetGeneratedInputFilename(self):
    path = OUT_DIR
    if self.input_filename:
      path = os.path.join(path, self.input_filename)
    else:
      path = os.path.join(path, self.filename)

    if self.is_roundtrip:
      dirname = os.path.dirname(path)
      basename = os.path.basename(path)
      path = os.path.join(dirname, 'roundtrip', basename)
    return path

  def ShouldCreateRoundtrip(self):
    return self.tool in ROUNDTRIP_TOOLS

  def ParseDirective(self, key, value):
    if key == 'EXE':
      self.exe = value
    elif key == 'STDIN_FILE':
      self.input_filename = value
    elif key == 'FLAGS':
      if not isinstance(value, list):
        value = shlex.split(value)
      self.flags += value
    elif key == 'ERROR':
      self.expected_error = int(value)
    elif key == 'SLOW':
      self.slow = True
    elif key == 'SKIP':
      self.skip = True
    elif key == 'VERBOSE-FLAGS':
      self.verbose_flags = [shlex.split(level) for level in value]
    elif key in ['TODO', 'NOTE']:
      pass
    elif key == 'TOOL':
      if not value in TOOLS:
        raise Error('Unknown tool: %s' % value)
      self.tool = value
      for tool_key, tool_value in TOOLS[value].items():
        self.ParseDirective(tool_key, tool_value)
    else:
      raise Error('Unknown directive: %s' % key)

  def Parse(self, filename):
    self.filename = filename

    test_path = os.path.join(REPO_ROOT_DIR, filename)
    with open(test_path) as f:
      seen_keys = set()
      state = 'header'
      empty = True
      header_lines = []
      input_lines = []
      stdout_lines = []
      stderr_lines = []
      for line in f.readlines():
        empty = False
        m = re.match(r'\s*\(;; (STDOUT|STDERR) ;;;$', line)
        if m:
          directive = m.group(1)
          if directive == 'STDERR':
            state = 'stderr'
            continue
          elif directive == 'STDOUT':
            state = 'stdout'
            continue
        else:
          m = re.match(r'\s*;;;(.*)$', line)
          if m:
            directive = m.group(1).strip()
            if state == 'header':
              key, value = directive.split(':', 1)
              key = key.strip()
              value = value.strip()
              if key in seen_keys:
                raise Error('%s already set' % key)
              seen_keys.add(key)
              self.ParseDirective(key, value)
            elif state in ('stdout', 'stderr'):
              if not re.match(r'%s ;;\)$' % state.upper(), directive):
                raise Error('Bad directive in %s block: %s' % (state,
                                                               directive))
              state = 'none'
            else:
              raise Error('Unexpected directive: %s' % directive)
          elif state == 'header':
            state = 'input'

        if state == 'header':
          header_lines.append(line)
        if state == 'input':
          if self.input_filename:
            raise Error('Can\'t have STDIN_FILE and input')
          input_lines.append(line)
        elif state == 'stderr':
          stderr_lines.append(line)
        elif state == 'stdout':
          stdout_lines.append(line)
    if empty:
      raise Error('empty test file')

    self.header = ''.join(header_lines)
    self.input_ = ''.join(input_lines)
    self.expected_stdout = ''.join(stdout_lines)
    self.expected_stderr = ''.join(stderr_lines)

  def GetExecutable(self):
    if os.path.splitext(self.exe)[1] == '.py':
      return [sys.executable, os.path.join(REPO_ROOT_DIR, self.exe)]
    else:
      return [self.exe]

  def FormatCommand(self, cmd, variables):
    return [arg % variables for arg in cmd]

  def GetCommand(self, filename, variables, extra_args=None, verbose_level=0):
    cmd = self.GetExecutable()
    vl = 0
    while vl < verbose_level and vl < len(self.verbose_flags):
      cmd += self.verbose_flags[vl]
      vl += 1
    if extra_args:
      cmd += extra_args
    cmd += self.flags
    cmd += [filename.replace(os.path.sep, '/')]
    cmd = self.FormatCommand(cmd, variables)
    self.last_cmd = cmd
    return cmd

  def CreateInputFile(self):
    gen_input_path = self.GetGeneratedInputFilename()
    gen_input_dir = os.path.dirname(gen_input_path)
    try:
      os.makedirs(gen_input_dir)
    except OSError as e:
      if not os.path.isdir(gen_input_dir):
        raise

    # Read/write as binary because spec tests may have invalid UTF-8 codes.
    with open(gen_input_path, 'wb') as gen_input_file:
      if self.input_filename:
        input_path = os.path.join(REPO_ROOT_DIR, self.input_filename)
        with open(input_path, 'rb') as input_file:
          gen_input_file.write(input_file.read())
      else:
        # add an empty line for each header line so the line numbers match
        gen_input_file.write(('\n' * self.header.count('\n')).encode('ascii'))
        gen_input_file.write(self.input_.encode('ascii'))
      gen_input_file.flush()
      return gen_input_file.name

  def Rebase(self, stdout, stderr):
    test_path = os.path.join(REPO_ROOT_DIR, self.filename)
    with open(test_path, 'wb') as f:
      f.write(self.header)
      f.write(self.input_)
      if stderr:
        f.write('(;; STDERR ;;;\n')
        f.write(stderr)
        f.write(';;; STDERR ;;)\n')
      if stdout:
        f.write('(;; STDOUT ;;;\n')
        f.write(stdout)
        f.write(';;; STDOUT ;;)\n')

  def Diff(self, stdout, stderr):
    msg = ''
    if self.expected_stderr != stderr:
      diff_lines = DiffLines(self.expected_stderr, stderr)
      if len(diff_lines) > 0:
        msg += 'STDERR MISMATCH:\n' + '\n'.join(diff_lines) + '\n'

    if self.expected_stdout != stdout:
      diff_lines = DiffLines(self.expected_stdout, stdout)
      if len(diff_lines) > 0:
        msg += 'STDOUT MISMATCH:\n' + '\n'.join(diff_lines) + '\n'

    if msg:
      raise Error(msg)


class Status(object):

  def __init__(self, verbose):
    self.verbose = verbose
    self.start_time = None
    self.last_length = 0
    self.last_finished = None
    self.skipped = 0
    self.passed = 0
    self.failed = 0
    self.total = 0
    self.failed_tests = []

  def Start(self, total):
    self.total = total
    self.start_time = time.time()

  def Passed(self, info, duration):
    self.passed += 1
    if self.verbose:
      sys.stderr.write('+ %s (%.3fs)\n' % (info.GetName(), duration))
    else:
      self.Clear()
      self._PrintShortStatus(info)
      sys.stderr.flush()

  def Failed(self, info, error_msg):
    self.failed += 1
    self.failed_tests.append(info)
    if not self.verbose:
      self.Clear()
    sys.stderr.write('- %s\n%s\n' % (info.GetName(), Indent(error_msg, 2)))

  def Skipped(self, info):
    self.skipped += 1
    if self.verbose:
      sys.stderr.write('. %s (skipped)\n' % info.GetName())

  def UpdateTimer(self):
    self._PrintShortStatus(self.last_finished)

  def Print(self):
    self._PrintShortStatus(None)
    sys.stderr.write('\n')

  def _PrintShortStatus(self, info):
    total_duration = time.time() - self.start_time
    name = info.GetName() if info else ''
    if (self.total - self.skipped):
      percent = 100 * (self.passed + self.failed) / (self.total - self.skipped)
    else:
      percent = 100
    status = '[+%d|-%d|%%%d] (%.2fs) %s\r' % (self.passed, self.failed,
                                              percent, total_duration, name)
    self.last_length = len(status)
    self.last_finished = info
    sys.stderr.write(status)

  def Clear(self):
    if not self.verbose:
      sys.stderr.write('%s\r' % (' ' * self.last_length))


def FindTestFiles(ext, filter_pattern_re):
  tests = []
  for root, dirs, files in os.walk(TEST_DIR):
    for f in files:
      path = os.path.join(root, f)
      if os.path.splitext(f)[1] == ext:
        tests.append(os.path.relpath(path, REPO_ROOT_DIR))
  tests.sort()
  return [test for test in tests if re.match(filter_pattern_re, test)]


def GetAllTestInfo(test_names, status):
  infos = []
  for test_name in test_names:
    info = TestInfo()
    try:
      info.Parse(test_name)
      infos.append(info)
    except Error as e:
      status.Failed(info, str(e))
  return infos


def RunTest(info, options, variables, verbose_level=0):
  timeout = options.timeout
  if info.slow:
    timeout *= SLOW_TIMEOUT_MULTIPLIER

  # Clone variables dict so it can be safely modified.
  variables = dict(variables)

  cwd = REPO_ROOT_DIR
  gen_input_path = info.CreateInputFile()
  rel_gen_input_path = os.path.relpath(gen_input_path, cwd)
  variables['out_dir'] = os.path.dirname(rel_gen_input_path)
  cmd = info.GetCommand(rel_gen_input_path, variables, options.arg,
                        verbose_level)
  if options.print_cmd:
    print(' '.join(cmd))

  try:
    return RunCommandWithTimeout(cmd, cwd, timeout, verbose_level > 0)
  except (Error, KeyboardInterrupt) as e:
    return e


def HandleTestResult(status, info, result, rebase=False):
  try:
    if isinstance(result, (Error, KeyboardInterrupt)):
      raise result

    stdout, stderr, returncode, duration = result
    if info.is_roundtrip:
      if returncode == 0:
        status.Passed(info, duration)
      elif returncode == 2:
        # run-roundtrip.py returns 2 if the file couldn't be parsed.
        # it's likely a "bad-*" file.
        status.Skipped(info)
      else:
        raise Error(stderr)
    else:
      if returncode != info.expected_error:
        # This test has already failed, but diff it anyway.
        msg = 'expected error code %d, got %d.' % (info.expected_error,
                                                   returncode)
        try:
          info.Diff(stdout, stderr)
        except Error as e:
          msg += '\n' + str(e)
        raise Error(msg)
      else:
        if rebase:
          info.Rebase(stdout, stderr)
        else:
          info.Diff(stdout, stderr)
        status.Passed(info, duration)
  except Error as e:
    status.Failed(info, str(e))


#Source : http://stackoverflow.com/questions/3041986/python-command-line-yes-no-input
def YesNoPrompt(question, default='yes'):
  """Ask a yes/no question via raw_input() and return their answer.

  "question" is a string that is presented to the user.
  "default" is the presumed answer if the user just hits <Enter>.
    It must be "yes" (the default), "no" or None (meaning
    an answer is required of the user).

  The "answer" return value is True for "yes" or False for "no".
  """
  valid = {'yes': True, 'y': True, 'ye': True, 'no': False, 'n': False}
  if default is None:
    prompt = ' [y/n] '
  elif default == 'yes':
    prompt = ' [Y/n] '
  elif default == 'no':
    prompt = ' [y/N] '
  else:
    raise ValueError('invalid default answer: \'%s\'' % default)

  while True:
    sys.stdout.write(question + prompt)
    choice = raw_input().lower()
    if default is not None and choice == '':
      return valid[default]
    elif choice in valid:
      return valid[choice]
    else:
      sys.stdout.write('Please respond with \'yes\' or \'no\' '
                       '(or \'y\' or \'n\').\n')


def RunMultiThreaded(infos_to_run, status, options, variables):
  test_count = len(infos_to_run)
  pool = multiprocessing.Pool(options.jobs)
  try:
    results = [(info, pool.apply_async(RunTest, (info, options, variables)))
               for info in infos_to_run]
    while results:
      new_results = []
      for info, result in results:
        if result.ready():
          HandleTestResult(status, info, result.get(0), options.rebase)
        else:
          new_results.append((info, result))
      time.sleep(0.01)
      results = new_results
    pool.close()
  finally:
    pool.terminate()
    pool.join()


def RunSingleThreaded(infos_to_run, status, options, variables):
  continued_errors = 0

  for info in infos_to_run:
    result = RunTest(info, options, variables)
    HandleTestResult(status, info, result, options.rebase)
    if status.failed > continued_errors:
      if options.fail_fast:
        break
      elif options.stop_interactive:
        rerun_verbose = YesNoPrompt(question='Rerun with verbose option?',
                                    default='no')
        if rerun_verbose:
          RunTest(info, options, variables, verbose_level=2)
        should_continue = YesNoPrompt(question='Continue testing?',
                                      default='yes')
        if not should_continue:
          break
      elif options.verbose:
        RunTest(info, options, variables, verbose_level=1)
      continued_errors += 1


def GetDefaultJobCount():
  cpu_count = multiprocessing.cpu_count()
  if cpu_count <= 1:
    return 1
  else:
    return cpu_count // 2


def main(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('-a', '--arg',
                      help='additional args to pass to executable',
                      action='append')
  parser.add_argument('--bindir', metavar='PATH',
                      default=find_exe.GetDefaultPath(),
                      help='directory to search for all executables.')
  parser.add_argument('-v', '--verbose', help='print more diagnotic messages.',
                      action='store_true')
  parser.add_argument('-f', '--fail-fast', help='Exit on first failure. '
                      'Extra options with \'--jobs 1\'', action='store_true')
  parser.add_argument('--stop-interactive',
                      help='Enter interactive mode on errors. '
                      'Extra options with \'--jobs 1\'', action='store_true')
  parser.add_argument('-l', '--list', help='list all tests.',
                      action='store_true')
  parser.add_argument('-r', '--rebase',
                      help='rebase a test to its current output.',
                      action='store_true')
  parser.add_argument('-j', '--jobs',
                      help='number of jobs to use to run tests', type=int,
                      default=GetDefaultJobCount())
  parser.add_argument('-t', '--timeout', type=float, default=DEFAULT_TIMEOUT,
                      help='per test timeout in seconds')
  parser.add_argument('--no-roundtrip',
                      help='don\'t run roundtrip.py on all tests',
                      action='store_false', default=True, dest='roundtrip')
  parser.add_argument('-p', '--print-cmd',
                      help='print the commands that are run.',
                      action='store_true')
  parser.add_argument('patterns', metavar='pattern', nargs='*',
                      help='test patterns.')
  options = parser.parse_args(args)

  if options.jobs != 1:
    if options.fail_fast:
      parser.error('--fail-fast only works with -j1')
    if options.stop_interactive:
      parser.error('--stop-interactive only works with -j1')

  if options.patterns:
    pattern_re = '|'.join(
        fnmatch.translate('*%s*' % p) for p in options.patterns)
  else:
    pattern_re = '.*'

  test_names = FindTestFiles('.txt', pattern_re)

  if options.list:
    for test_name in test_names:
      print(test_name)
    return 0
  if not test_names:
    print('no tests match that filter')
    return 1

  variables = {}
  variables['test_dir'] = os.path.abspath(TEST_DIR)
  variables['bindir'] = options.bindir
  for exe_basename in find_exe.EXECUTABLES:
    exe_override = os.path.join(options.bindir, exe_basename)
    variables[exe_basename] = find_exe.FindExecutable(exe_basename,
                                                      exe_override)

  status = Status(options.verbose)
  infos = GetAllTestInfo(test_names, status)
  infos_to_run = []
  for info in infos:
    if info.skip:
      status.Skipped(info)
      continue
    infos_to_run.append(info)

    if options.roundtrip and info.ShouldCreateRoundtrip():
      infos_to_run.append(info.CreateRoundtripInfo())

  if not os.path.exists(OUT_DIR):
    os.makedirs(OUT_DIR)

  status.Start(len(infos_to_run))

  try:
    if options.jobs > 1:
      RunMultiThreaded(infos_to_run, status, options, variables)
    else:
      RunSingleThreaded(infos_to_run, status, options, variables)
  except KeyboardInterrupt:
    print('\nInterrupted testing\n')

  status.Clear()

  ret = 0

  if status.failed:
    sys.stderr.write('**** FAILED %s\n' % ('*' * (80 - 14)))
    for info in status.failed_tests:
      sys.stderr.write('- %s\n    %s\n' % (info.GetName(),
                                           ' '.join(info.last_cmd)))
    ret = 1

  status.Print()
  return ret


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
