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

if [[ ${COMPILER} = "gcc" ]]; then
  # Build without re2c/bison to test prebuilt C sources
  make gcc-debug-no-re2c-bison
elif [[ ${COMPILER} = "clang" && -z ${SANITIZER:-} ]]; then
  # Test building without GTest submodule
  make clang-debug-no-tests
fi

for BUILD_TYPE in ${BUILD_TYPES}; do
  if [[ -n ${SANITIZER:-} ]]; then
    make ${COMPILER}-${BUILD_TYPE}-${SANITIZER}
  else
    make ${COMPILER}-${BUILD_TYPE}
  fi
done
