/*
 * Copyright 2016 WebAssembly Community Group participants
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

#ifndef WABT_WAT_WRITER_H_
#define WABT_WAT_WRITER_H_

#include "wabt/common.h"
#include "wabt/feature.h"

namespace wabt {

struct Module;
class Stream;

struct WriteWatOptions {
  WriteWatOptions() = default;
  WriteWatOptions(const Features& features) : features(features) {}
  Features features;
  bool fold_exprs = false;  // Write folded expressions.
  bool inline_export = false;
  bool inline_import = false;
  bool relocatable = false;
};

Result WriteWat(Stream*, const Module*, const WriteWatOptions&);

}  // namespace wabt

#endif /* WABT_WAT_WRITER_H_ */
