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

OS=$(uname -s)
SCRIPT_DIR="$(cd "$(dirname "$0")"; pwd -P)"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"
V8_DIR="${ROOT_DIR}/third_party/v8/v8"
V8_OUT_DIR="${V8_DIR}/out/Release"
V8_SHA=$(cd "${V8_DIR}" && git rev-parse HEAD)
# TODO(binji): other architectures
V8_HOST_ARCH=x86_64
V8_TARGET_ARCH=x86_64
V8_BASE_BUCKET_HTTPS_URL=https://storage.googleapis.com/webassembly/v8
V8_BASE_BUCKET_GS_URL=gs://webassembly/v8
V8_BUCKET_PATH=${V8_SHA}/${OS}/${V8_HOST_ARCH}-${V8_TARGET_ARCH}
V8_BUCKET_GS_URL=${V8_BASE_BUCKET_GS_URL}/${V8_BUCKET_PATH}
V8_BUCKET_HTTPS_URL=${V8_BASE_BUCKET_HTTPS_URL}/${V8_BUCKET_PATH}
D8_FILES="d8 natives_blob.bin snapshot_blob.bin"
