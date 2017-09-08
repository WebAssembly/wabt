/*
 * Copyright 2017 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var examples = [
  {
    name: 'simple',
    contents: new Uint8Array([
      0,   97,  115, 109, 1,   0,   0, 0,  1,  7, 1,  96,  2,   127, 127,
      1,   127, 3,   2,   1,   0,   7, 10, 1,  6, 97, 100, 100, 84,  119,
      111, 0,   0,   10,  9,   1,   7, 0,  32, 0, 32, 1,   106, 11,  0,
      25,  4,   110, 97,  109, 101, 1, 9,  1,  0, 6,  97,  100, 100, 84,
      119, 111, 2,   7,   1,   0,   2, 0,  0,  1, 0
    ]),
  },

  {
    name: 'factorial',
    contents: new Uint8Array([
      0,  97, 115, 109, 1,   0,   0,  0,  1,   6,  1,  96,  1,  126, 1,   126,
      3,  2,  1,   0,   7,   7,   1,  3,  102, 97, 99, 0,   0,  10,  25,  1,
      23, 0,  32,  0,   66,  1,   83, 4,  126, 66, 1,  5,   32, 0,   32,  0,
      66, 1,  125, 16,  0,   126, 11, 11, 0,   20, 4,  110, 97, 109, 101, 1,
      6,  1,  0,   3,   102, 97,  99, 2,  5,   1,  0,  1,   0,  0
    ]),
  },

  {
    name: 'stuff',
    contents: new Uint8Array([
      0,   97,  115, 109, 1,   0,   0,   0,   1,   13,  3,  96,  1,   125,
      0,   96,  1,   127, 1,   127, 96,  0,   0,   2,   11, 1,   3,   102,
      111, 111, 3,   98,  97,  114, 0,   0,   3,   3,   2,  2,   0,   4,
      5,   1,   112, 1,   0,   1,   5,   4,   1,   1,   1,  1,   7,   9,
      1,   5,   102, 117, 110, 99,  49,  0,   1,   8,   1,  1,   10,  10,
      2,   2,   0,   11,  5,   0,   65,  42,  26,  11,  11, 8,   1,   0,
      65,  0,   11,  2,   104, 105, 0,   44,  4,   110, 97, 109, 101, 1,
      24,  3,   0,   7,   105, 109, 112, 111, 114, 116, 48, 1,   5,   102,
      117, 110, 99,  48,  2,   5,   102, 117, 110, 99,  49, 2,   11,  3,
      0,   1,   0,   0,   1,   0,   2,   1,   0,   0
    ]),
  },
];
