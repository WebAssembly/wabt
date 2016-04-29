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

config=Release

while [[ $# > 0 ]]; do
  flag="$1"
  case $flag in
    --debug)
      config=Debug
      ;;
    *)
      echo "unknown arg ${flag}"
      ;;
  esac
  shift
done

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"

cd ${ROOT_DIR}/third_party/v8

if [[ ! -d depot_tools ]]; then
  echo "Cloning depot_tools"
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH=$PWD/depot_tools:$PATH
gclient sync

# Don't use the CC from the environment; v8 doesn't seem to build properly with
# it.
unset CC
cd v8
GYP_GENERATORS=ninja gypfiles/gyp_v8
time ninja -C out/${config} d8
