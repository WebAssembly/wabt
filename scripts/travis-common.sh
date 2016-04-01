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

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"
BUILD_TYPES="debug release"
BUILD_TYPES_UPPER="Debug Release"
SANITIZERS="asan lsan msan"

if [ ${CC} = "gcc" ]; then
  COMPILERS="gcc gcc-i686"
elif [ ${CC} = "clang" ]; then
  COMPILERS="clang"
else
  echo "unknown compiler: ${CC}"
  exit 1
fi
