#!/bin/bash
#
# Copyright 2020 WebAssembly Community Group participants
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
MAN_DIR="$ROOT_DIR/man"
DOC_DIR="$ROOT_DIR/docs/doc"

if ! which mandoc > /dev/null; then
  echo 'mandoc not found'
  exit
fi

set -x
for file in "$MAN_DIR"/*.1; do
  mandoc -T html -O man='%N.%S.html' "$file" > "$DOC_DIR/$(basename "$file").html"
done
set +x

