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

BUILD_TYPES="debug release"
SANITIZERS="-asan -lsan -msan"

for BUILD_TYPE in ${BUILD_TYPES}; do
  make ${CC}-${BUILD_TYPE}-sexpr-wasm
  if [ ${CC} = "clang" ]; then
    for SANITIZER in ${SANITIZERS}; do
      make ${CC}-${BUILD_TYPE}-sexpr-wasm${SANITIZER}
    done
  fi
done
