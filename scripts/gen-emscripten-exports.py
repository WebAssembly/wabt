#!/usr/bin/env python3
#
# Copyright 2024 WebAssembly Community Group participants
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

import argparse
import os
import re
import sys

def parse_features(feature_def_path):
    features = []
    # Match WABT_FEATURE(name, flag, default, help)
    pattern = re.compile(r'WABT_FEATURE\(\s*([a-zA-Z0-9_]+)\s*,\s*"[^"]+"\s*,\s*(true|false)\s*,')
    with open(feature_def_path, 'r') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                features.append((match.group(1), match.group(2)))
    return features

def generate_post_js(post_js_in_path, post_js_out_path, features):
    with open(post_js_in_path, 'r') as f:
        content = f.read()
    
    # Generate the JS dictionary entries
    js_features = []
    for name, default in features:
        js_features.append(f"  '{name}': {default},")
    js_features_str = '\n'.join(js_features)
    
    content = content.replace('@FEATURES@', js_features_str)
    
    with open(post_js_out_path, 'w') as f:
        f.write(content)

def generate_exports(exports_in_path, exports_out_path, features):
    with open(exports_in_path, 'r') as f:
        content = f.read()
        
    exports = []
    for name, default in features:
        exports.append(f"_wabt_{name}_enabled")
        exports.append(f"_wabt_set_{name}_enabled")
    
    # Sort for deterministic output
    exports.sort()
    exports_str = '\n'.join(exports)
    
    # If the file already has a trailing newline, we want to just insert our block
    if content.endswith('\n'):
        content = content[:-1]
        content = content.replace('@FEATURES@', exports_str)
        content += '\n'
    else:
        content = content.replace('@FEATURES@', exports_str)
    
    # Now sort everything alphabetically (some non-feature exports were already there)
    all_exports = [e for e in content.split('\n') if e.strip()]
    all_exports.sort()
    
    with open(exports_out_path, 'w') as f:
        f.write('\n'.join(all_exports) + '\n')

def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--feature-def', required=True, help='Path to include/wabt/feature.def')
    parser.add_argument('--postjs-in', required=True, help='Path to src/wabt.post.js.in')
    parser.add_argument('--postjs-out', required=True, help='Output path for wabt.post.js')
    parser.add_argument('--exports-in', required=True, help='Path to src/emscripten-exports.txt.in')
    parser.add_argument('--exports-out', required=True, help='Output path for emscripten-exports.txt')
    options = parser.parse_args(args)

    features = parse_features(options.feature_def)
    if not features:
        print("Error: No features found in", options.feature_def)
        return 1
        
    generate_post_js(options.postjs_in, options.postjs_out, features)
    generate_exports(options.exports_in, options.exports_out, features)
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
