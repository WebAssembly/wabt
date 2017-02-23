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

#ifndef WABT_TYPE_CHECKER_H_
#define WABT_TYPE_CHECKER_H_

#include "common.h"
#include "type-vector.h"
#include "vector.h"

struct WabtAllocator;

typedef void (*WabtTypeCheckerErrorCallback)(const char* msg, void* user_data);

typedef struct WabtTypeCheckerErrorHandler {
  WabtTypeCheckerErrorCallback on_error;
  void* user_data;
} WabtTypeCheckerErrorHandler;

typedef struct WabtTypeCheckerLabel {
  WabtLabelType label_type;
  WabtTypeVector sig;
  size_t type_stack_limit;
  WabtBool unreachable;
} WabtTypeCheckerLabel;
WABT_DEFINE_VECTOR(type_checker_label, WabtTypeCheckerLabel);

typedef struct WabtTypeChecker {
  WabtTypeCheckerErrorHandler* error_handler;
  WabtTypeVector type_stack;
  WabtTypeCheckerLabelVector label_stack;
/* TODO(binji): will need to be complete signature when signatures with
 * multiple types are allowed. */
  WabtType br_table_sig;
} WabtTypeChecker;

WABT_EXTERN_C_BEGIN

void wabt_destroy_typechecker(WabtTypeChecker*);

WabtBool wabt_typechecker_is_unreachable(WabtTypeChecker* tc);
WabtResult wabt_typechecker_get_label(WabtTypeChecker* tc,
                                      size_t depth,
                                      WabtTypeCheckerLabel** out_label);

WabtResult wabt_typechecker_begin_function(WabtTypeChecker*,
                                           const WabtTypeVector* sig);
WabtResult wabt_typechecker_on_binary(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_block(WabtTypeChecker*,
                                     const WabtTypeVector* sig);
WabtResult wabt_typechecker_on_br(WabtTypeChecker*, size_t depth);
WabtResult wabt_typechecker_on_br_if(WabtTypeChecker*, size_t depth);
WabtResult wabt_typechecker_begin_br_table(WabtTypeChecker*);
WabtResult wabt_typechecker_on_br_table_target(WabtTypeChecker*, size_t depth);
WabtResult wabt_typechecker_end_br_table(WabtTypeChecker*);
WabtResult wabt_typechecker_on_call(WabtTypeChecker*,
                                    const WabtTypeVector* param_types,
                                    const WabtTypeVector* result_types);
WabtResult wabt_typechecker_on_call_indirect(
    WabtTypeChecker*,
    const WabtTypeVector* param_types,
    const WabtTypeVector* result_types);
WabtResult wabt_typechecker_on_compare(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_const(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_convert(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_current_memory(WabtTypeChecker*);
WabtResult wabt_typechecker_on_drop(WabtTypeChecker*);
WabtResult wabt_typechecker_on_else(WabtTypeChecker*);
WabtResult wabt_typechecker_on_end(WabtTypeChecker*);
WabtResult wabt_typechecker_on_get_global(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_get_local(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_grow_memory(WabtTypeChecker*);
WabtResult wabt_typechecker_on_if(WabtTypeChecker*, const WabtTypeVector* sig);
WabtResult wabt_typechecker_on_load(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_loop(WabtTypeChecker*,
                                    const WabtTypeVector* sig);
WabtResult wabt_typechecker_on_return(WabtTypeChecker*);
WabtResult wabt_typechecker_on_select(WabtTypeChecker*);
WabtResult wabt_typechecker_on_set_global(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_set_local(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_store(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_tee_local(WabtTypeChecker*, WabtType);
WabtResult wabt_typechecker_on_unary(WabtTypeChecker*, WabtOpcode);
WabtResult wabt_typechecker_on_unreachable(WabtTypeChecker*);
WabtResult wabt_typechecker_end_function(WabtTypeChecker*);

WABT_EXTERN_C_END

#endif /* WABT_TYPE_CHECKER_H_ */
