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

SCRIPT_DIR="$(cd "$(dirname "$0")"; pwd -P)"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"

# See https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Build_Documentation
cd ${ROOT_DIR}/third_party
if [[ ! -d gecko-dev ]]; then
  echo "Cloning gecko-dev --depth 1"
  git clone --depth 1 https://github.com/mozilla/gecko-dev.git
fi
cd gecko-dev/js/src
autoconf2.13
mkdir -p build_OPT.OBJ
cd build_OPT.OBJ
../configure
time make -j8
