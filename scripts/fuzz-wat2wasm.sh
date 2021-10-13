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
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"

ENABLE="--enable-except --enable-sat --enable-sign --enable-thread --enable-multi --enable-tail"

${FUZZ_BIN_DIR}/afl-fuzz -x fuzz-in/wast.dict -i fuzz-in/wast/ -o fuzz-out -- out/gcc-fuzz/Debug/wat2wasm @@ ${ENABLE}
