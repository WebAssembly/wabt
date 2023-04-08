#!/usr/bin/env python3
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

"""Check for clean checkout. This is run after tests during CI to ensure
the generated source code in src/prebuilt has been updated.
"""

import os
import subprocess
import sys


def main():
    print("Running 'git status --short'")
    print('')

    here = os.path.dirname(__file__)
    root = os.path.dirname(here)
    output = subprocess.check_output(['git', 'status', '--short'], cwd=root)
    output = output.decode('utf-8').strip()
    if not output:
        print('Tree is clean.')
        return 0

    print(output)
    print('\nCheckout is not clean.  See above for list of dirty/untracked files.')
    return 1


if __name__ == '__main__':
    sys.exit(main())
