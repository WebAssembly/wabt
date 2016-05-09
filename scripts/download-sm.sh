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
OUT_DIR="${ROOT_DIR}/out"

source "${SCRIPT_DIR}/sm-common.sh"

Download() {
  local URL=$1
  local FILENAME=$2
  echo "Downloading ${URL}..."
  CURL_ARGS="--fail --location"
  if [ -t 1 ]; then
    # Add --progress-bar but only if stdout is a TTY device.
    CURL_ARGS+=" --progress-bar"
  else
    # otherwise suppress all status output, since curl always
    # assumes a TTY and writes \r and \b characters.
    CURL_ARGS+=" --silent"
  fi
  if which curl > /dev/null ; then
    curl ${CURL_ARGS} -o "${FILENAME}" "${URL}"
  else
    echo "error: could not find 'curl' in your PATH"
    exit 1
  fi
}

mkdir -p "${OUT_DIR}"

for file in ${SM_FILES}; do
  Download ${SM_BUCKET_HTTPS_URL}/${file} "${OUT_DIR}/${file}"
done

# Make js executable
chmod ug+x "${OUT_DIR}/js"
