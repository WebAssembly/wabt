#!/usr/bin/env python
# Test runner: table allocation OOM must fail closed (abort), not produce
# a table with data==NULL && size!=0.
#
# Mechanism: compile a module declaring a huge table with wasm2c, build a
# tiny host that instantiates it inside a fork()ed child limited by
# RLIMIT_AS, and expect the child to die with SIGABRT.
#
# Windows is skipped (fork/setrlimit are POSIX). CI has this on Linux/macOS.
import os
import subprocess
import sys
import tempfile

import find_exe
from utils import Error

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO = find_exe.REPO_ROOT_DIR
LIMIT_AS = 512 * 1024 * 1024          # 512 MB address space for the child
WAT = r'''
(module
  (table $t 2000000000 4000000000 funcref)
  (func $f (export "run") (result i32) (i32.const 0))
)
'''
MAIN_C = r'''
#include <stdio.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bigtable.h"
int main(void) {
  pid_t pid = fork();
  if (pid == 0) {
    struct rlimit rl = { %d, %d };
    if (setrlimit(RLIMIT_AS, &rl) != 0) _exit(120);
    w2c_bigtable inst;
    wasm_rt_init();
    wasm2c_bigtable_instantiate(&inst);  /* patched: abort() -> SIGABRT */
    wasm2c_bigtable_free(&inst);
    _exit(0);                            /* unpatched: survives OOM */
  }
  int status = 0;
  waitpid(pid, &status, 0);
  if (WIFSIGNALED(status) && WTERMSIG(status) == 6) return 0;
  if (WIFEXITED(status)) {
    fprintf(stderr, "child exited rc=%%d (expected SIGABRT)\\n",
            WEXITSTATUS(status));
    return 1;
  }
  fprintf(stderr, "child killed by signal %%d (expected 6)\\n",
          WTERMSIG(status));
  return 1;
}
''' % (LIMIT_AS, LIMIT_AS)


def main(args):
    if sys.platform == 'win32':
        return 3  # SKIPPED
    with tempfile.TemporaryDirectory() as d:
        wat = os.path.join(d, 'bigtable.wat')
        with open(wat, 'w') as f:
            f.write(WAT)
        wasm = os.path.join(d, 'bigtable.wasm')
        subprocess.run(
            [find_exe.GetWat2WasmExecutable(), wat, '-o', wasm],
            check=True)
        c = os.path.join(d, 'bigtable.c')
        subprocess.run(
            [find_exe.GetWasm2CExecutable(), wasm, '-n', 'bigtable',
             '-o', c],
            check=True)
        main_c = os.path.join(d, 'main.c')
        with open(main_c, 'w') as f:
            f.write(MAIN_C)
        exe = os.path.join(d, 'oom_test')
        inc = os.path.join(REPO, 'wasm2c')
        cmd = ['cc', '-O0', '-I', inc, '-I', d, '-o', exe, main_c, c,
               os.path.join(inc, 'wasm-rt-impl.c'),
               os.path.join(inc, 'wasm-rt-exceptions-impl.c'),
               os.path.join(inc, 'wasm-rt-mem-impl.c'), '-lpthread', '-lm']
        subprocess.run(cmd, check=True)
        r = subprocess.run([exe], capture_output=True, text=True)
        if r.returncode != 0:
            sys.stderr.write(r.stderr)
            raise Error('table OOM was not fail-closed (expected SIGABRT)')
    print('1/1 tests passed.')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except Error as e:
        sys.stderr.write(str(e) + '\n')
        sys.exit(1)
