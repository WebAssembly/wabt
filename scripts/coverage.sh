#!/bin/bash
#
# Copyright 2017 WebAssembly Community Group participants
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

COMPILER=${1:-gcc}
BUILD_TYPE=${2:-Debug}

SCRIPT_DIR="$(cd "$(dirname "$0")"; pwd -P)"
ROOT_DIR=${SCRIPT_DIR}/..
BIN_DIR=${ROOT_DIR}/out/${COMPILER}/${BUILD_TYPE}/cov
LCOVRC_FILE=${ROOT_DIR}/lcovrc
COV_FILE=${BIN_DIR}/lcov.info
COV_HTML_DIR=${ROOT_DIR}/out/lcov

log_and_run() {
  echo $*
  (exec $*)
}

log_and_run lcov --zerocounters -d ${BIN_DIR} --rc lcov_branch_coverage=1
log_and_run test/run-tests.py --bindir ${BIN_DIR}
log_and_run lcov -c -d ${BIN_DIR} -o ${COV_FILE} --rc lcov_branch_coverage=1
log_and_run genhtml --branch-coverage -o ${COV_HTML_DIR} ${COV_FILE}
