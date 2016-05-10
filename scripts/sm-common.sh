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
SM_DIR="${ROOT_DIR}/third_party/gecko-dev"
SM_OUT_DIR="${SM_DIR}/js/src/build_OPT.OBJ/js/src"
SM_SHA=$(cat "${ROOT_DIR}/third_party/gecko-dev.sha1")
# TODO(binji): other architectures
SM_HOST_ARCH=x86_64
SM_TARGET_ARCH=x86_64
SM_BASE_BUCKET_HTTPS_URL=https://storage.googleapis.com/webassembly/spidermonkey
SM_BASE_BUCKET_GS_URL=gs://webassembly/spidermonkey
SM_BUCKET_PATH=${SM_SHA}/${OS}/${SM_HOST_ARCH}-${SM_TARGET_ARCH}
SM_BUCKET_GS_URL=${SM_BASE_BUCKET_GS_URL}/${SM_BUCKET_PATH}
SM_BUCKET_HTTPS_URL=${SM_BASE_BUCKET_HTTPS_URL}/${SM_BUCKET_PATH}
SM_FILES="js"
