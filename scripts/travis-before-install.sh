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

if [[ ${TRAVIS_OS_NAME} = "linux" ]]; then
  sudo add-apt-repository ppa:jbboehr/build-deps -y # For re2c 0.16.
  sudo apt-get update -qq
  sudo apt-get install -qq re2c bison gcc-4.8-multilib g++-4.8-multilib -y
  sudo pip install flake8
elif [[ ${TRAVIS_OS_NAME} = "osx" ]]; then
  brew update
  brew install re2c bison
else
  echo "unknown TRAVIS_OS_NAME: ${TRAVIS_OS_NAME}"
  exit 1
fi
