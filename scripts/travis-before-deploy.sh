#!/bin/bash
#
# Copyright 2018 WebAssembly Community Group participants
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
source "${SCRIPT_DIR}/travis-common.sh"

if [[ -n ${WABT_DEPLOY:-} ]]; then
  # Rebuild the WABT_DEPLOY target so it copies the results into bin/
  make ${WABT_DEPLOY}

  PKGNAME="wabt-${TRAVIS_TAG}-${TRAVIS_OS_NAME}"
  mv bin wabt-${TRAVIS_TAG}
  tar -czf ${PKGNAME}.tar.gz wabt-${TRAVIS_TAG}
  sha256sum ${PKGNAME}.tar.gz > ${PKGNAME}.tar.gz.sha256
fi
