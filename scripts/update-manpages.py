#!/usr/bin/env python3
#
# Copyright 2026 WebAssembly Community Group participants
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

"""Runs help2man.lua for each binary to update the man pages.
"""

import os
import re
import subprocess
import sys


def main():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    wabt_dir = os.path.dirname(script_dir)
    bin_dir = os.path.join(wabt_dir, 'bin')
    man_dir = os.path.join(wabt_dir, 'man')
    help2man_lua = os.path.join(script_dir, 'help2man.lua')

    # Read tools from help2man.lua
    with open(help2man_lua, 'r') as f:
        content = f.read()

    # Find the tools list
    match = re.search(r'local tools = \{(.*?)\}', content, re.DOTALL)
    if not match:
        print("Error: Could not find tools list in help2man.lua", file=sys.stderr)
        return 1

    tools_str = match.group(1)
    # Extract all quoted strings
    tools = re.findall(r'"([^"]+)"', tools_str)
    print(f"Found {len(tools)} tools: {', '.join(tools)}")
    assert tools

    for tool in tools:
        binary_path = os.path.join(bin_dir, tool)
        man_path = os.path.join(man_dir, f"{tool}.1")

        print(f"Generating man page for {tool}...")

        # Run binary --help
        help_proc = subprocess.Popen([binary_path, '--help'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Run lua help2man.lua
        stdout = subprocess.check_output(['lua', help2man_lua], stdin=help_proc.stdout)
        help_proc.stdout.close()  # Allow help_proc to receive a SIGPIPE if lua_proc exits

        with open(man_path, 'wb') as f:
            f.write(stdout)

    # Run generate-html-docs.sh if it exists
    gen_html_sh = os.path.join(script_dir, 'generate-html-docs.sh')
    print("Running generate-html-docs.sh...")
    subprocess.check_call([gen_html_sh], cwd=wabt_dir)
    return 0


if __name__ == '__main__':
    sys.exit(main())
