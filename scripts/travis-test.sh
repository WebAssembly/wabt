#!/bin/bash
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

set -o nounset
set -o errexit

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
source ${SCRIPT_DIR}/travis-common.sh

log_and_run() {
  echo $*
  exec $*
}

run_tests() {
  (cd ${ROOT_DIR} && log_and_run test/run-tests.py $* --timeout=10)
}

if [ ${CC} = "gcc" ]; then
  run_tests out/gcc/Debug-no-flex-bison/sexpr-wasm
fi

for COMPILER in ${COMPILERS}; do
  for BUILD_TYPE in ${BUILD_TYPES_UPPER}; do
    SEXPR_WASM=out/${COMPILER}/${BUILD_TYPE}/sexpr-wasm
    WASM_WAST=out/${COMPILER}/${BUILD_TYPE}/wasm-wast
        RUN_TEST_ARGS="--sexpr-wasm ${SEXPR_WASM} --wasm-wast ${WASM_WAST}"
    if [ -e ${SEXPR_WASM} ] && [ -e ${WASM_WAST} ]; then
      run_tests ${RUN_TEST_ARGS}
      run_tests ${RUN_TEST_ARGS} -a=--use-libc-allocator
    else
      echo "${EXE} doesn't exist; skipping."
    fi

    if [ ${COMPILER} = "clang" ]; then
      for SANITIZER in ${SANITIZERS}; do
        SEXPR_WASM=out/${COMPILER}/${BUILD_TYPE}/sexpr-wasm${SANITIZER}
        WASM_WAST=out/${COMPILER}/${BUILD_TYPE}/wasm-wast${SANITIZER}
        RUN_TEST_ARGS="--sexpr-wasm ${SEXPR_WASM} --wasm-wast ${WASM_WAST}"
        if [ -e ${SEXPR_WASM} ] && [ -e ${WASM_WAST} ]; then
          run_tests ${RUN_TEST_ARGS}
          run_tests ${RUN_TEST_ARGS} -a=--use-libc-allocator
        else
          echo "${EXE} doesn't exist; skipping."
        fi
      done
    fi
  done
done
