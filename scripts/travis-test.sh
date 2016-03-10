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
  local EXE=$1
  if [ "${2+defined}" = "defined" ]; then
    local ARG_FLAG="-a=$2"
  fi
  (cd ${ROOT_DIR} && log_and_run test/run-tests.py -e ${EXE} ${ARG_FLAG-} --timeout=10)
}

if [ ${CC} = "gcc" ]; then
  run_tests out/gcc/Debug-no-flex-bison/sexpr-wasm
fi

for COMPILER in ${COMPILERS}; do
  for BUILD_TYPE in ${BUILD_TYPES_UPPER}; do
    EXE=out/${COMPILER}/${BUILD_TYPE}/sexpr-wasm
    if [ -e ${EXE} ]; then
      run_tests ${EXE}
      run_tests ${EXE} --use-libc-allocator
    else
      echo "${EXE} doesn't exist; skipping."
    fi

    if [ ${COMPILER} = "clang" ]; then
      for SANITIZER in ${SANITIZERS}; do
        EXE=out/${COMPILER}/${BUILD_TYPE}/sexpr-wasm${SANITIZER}
        if [ -e ${EXE} ]; then
          run_tests ${EXE}
          run_tests ${EXE} --use-libc-allocator
        else
          echo "${EXE} doesn't exist; skipping."
        fi
      done
    fi
  done
done
