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
V8_DIR="${ROOT_DIR}/third_party/v8/v8"
V8_OUT_DIR="${V8_DIR}/out/Release"

V8_SHA=$(cd ${V8_DIR} && git rev-parse HEAD)
BUCKET_URL=gs://webassembly/v8-native-prototype/${V8_SHA}/
FILES="d8 natives_blob.bin snapshot_blob.bin"

for file in ${FILES}; do
  gsutil cp -a public-read ${V8_OUT_DIR}/${file} ${BUCKET_URL}
done
