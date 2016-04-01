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
  (cd ${ROOT_DIR} && log_and_run test/run-tests.py ${RUN_TEST_ARGS} $* --timeout=10)
}

set_run_test_args() {
  local COMPILER=$1
  local BUILD_TYPE=$2
  local CONFIG=${3:-}
  SEXPR_WASM=out/${COMPILER}/${BUILD_TYPE}/${CONFIG}/sexpr-wasm
  WASM_WAST=out/${COMPILER}/${BUILD_TYPE}/${CONFIG}/wasm-wast
  RUN_TEST_ARGS="--sexpr-wasm ${SEXPR_WASM} --wasm-wast ${WASM_WAST}"
}

if [ ${CC} = "gcc" ]; then
  set_run_test_args gcc Debug no-flex-bison
  run_tests
fi

for COMPILER in ${COMPILERS}; do
  for BUILD_TYPE in ${BUILD_TYPES_UPPER}; do
    set_run_test_args ${COMPILER} ${BUILD_TYPE}
    if [ -e ${SEXPR_WASM} ] && [ -e ${WASM_WAST} ]; then
      run_tests
      run_tests -a=--use-libc-allocator
    else
      echo "${SEXPR_WASM} or ${WASM_WAST} doesn't exist; skipping."
    fi

    if [ ${COMPILER} = "clang" ]; then
      for SANITIZER in ${SANITIZERS}; do
        set_run_test_args ${COMPILER} ${BUILD_TYPE} ${SANITIZER}
        if [ -e ${SEXPR_WASM} ] && [ -e ${WASM_WAST} ]; then
          run_tests
          run_tests -a=--use-libc-allocator
        else
          echo "${SEXPR_WASM} or ${WASM_WAST} doesn't exist; skipping."
        fi
      done
    fi
  done
done
