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

#include "type-checker.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WABT_ERROR;   \
  } while (0)

#define COMBINE_RESULT(result_var, result) \
  do {                                     \
    (result_var) |= (result);              \
  } while (0)

static void WABT_PRINTF_FORMAT(2, 3)
    print_error(WabtTypeChecker* tc, const char* fmt, ...) {
  if (tc->error_handler->on_error) {
    WABT_SNPRINTF_ALLOCA(buffer, length, fmt);
    tc->error_handler->on_error(buffer, tc->error_handler->user_data);
  }
}

WabtResult wabt_typechecker_get_label(WabtTypeChecker* tc,
                                      size_t depth,
                                      WabtTypeCheckerLabel** out_label) {
  if (depth >= tc->label_stack.size) {
    assert(tc->label_stack.size > 0);
    print_error(tc, "invalid depth: %" PRIzd " (max %" PRIzd ")",
                depth, tc->label_stack.size - 1);
    *out_label = NULL;
    return WABT_ERROR;
  }
  *out_label = &tc->label_stack.data[tc->label_stack.size - depth - 1];
  return WABT_OK;
}

static WabtResult top_label(WabtTypeChecker* tc,
                            WabtTypeCheckerLabel** out_label) {
  return wabt_typechecker_get_label(tc, 0, out_label);
}

WabtBool wabt_typechecker_is_unreachable(WabtTypeChecker* tc) {
  WabtTypeCheckerLabel* label;
  if (WABT_FAILED(top_label(tc, &label)))
    return WABT_TRUE;
  return label->unreachable;
}

static void reset_type_stack_to_label(WabtTypeChecker* tc,
                                      WabtTypeCheckerLabel* label) {
  tc->type_stack.size = label->type_stack_limit;
}

static WabtResult set_unreachable(WabtTypeChecker* tc) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  label->unreachable = WABT_TRUE;
  reset_type_stack_to_label(tc, label);
  return WABT_OK;
}

static void push_label(WabtTypeChecker* tc,
                       WabtLabelType label_type,
                       const WabtTypeVector* sig) {
  WabtTypeCheckerLabel* label =
      wabt_append_type_checker_label(&tc->label_stack);
  label->label_type = label_type;
  wabt_extend_types(&label->sig, sig);
  label->type_stack_limit = tc->type_stack.size;
  label->unreachable = WABT_FALSE;
}

static void wabt_destroy_type_checker_label(WabtTypeCheckerLabel* label) {
  wabt_destroy_type_vector(&label->sig);
}

static WabtResult pop_label(WabtTypeChecker* tc) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  wabt_destroy_type_checker_label(label);
  tc->label_stack.size--;
  return WABT_OK;
}

static WabtResult check_label_type(WabtTypeCheckerLabel* label,
                                   WabtLabelType label_type) {
  return label->label_type == label_type ? WABT_OK : WABT_ERROR;
}

static WabtResult peek_type(WabtTypeChecker* tc,
                            uint32_t depth,
                            WabtType* out_type) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));

  if (label->type_stack_limit + depth >= tc->type_stack.size) {
    *out_type = WABT_TYPE_ANY;
    return label->unreachable ? WABT_OK : WABT_ERROR;
  }
  *out_type = tc->type_stack.data[tc->type_stack.size - depth - 1];
  return WABT_OK;
}

static WabtResult top_type(WabtTypeChecker* tc, WabtType* out_type) {
  return peek_type(tc, 0, out_type);
}

static WabtResult pop_type(WabtTypeChecker* tc, WabtType* out_type) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  WabtResult result = top_type(tc, out_type);
  if (tc->type_stack.size > label->type_stack_limit)
    tc->type_stack.size--;
  return result;
}

static WabtResult drop_types(WabtTypeChecker* tc, size_t drop_count) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  if (label->type_stack_limit + drop_count > tc->type_stack.size) {
    if (label->unreachable) {
      reset_type_stack_to_label(tc, label);
      return WABT_OK;
    }
    return WABT_ERROR;
  }
  tc->type_stack.size -= drop_count;
  return WABT_OK;
}

static void push_type(WabtTypeChecker* tc, WabtType type) {
  if (type != WABT_TYPE_VOID)
    wabt_append_type_value(&tc->type_stack, &type);
}

static void push_types(WabtTypeChecker* tc, const WabtTypeVector* types) {
  size_t i;
  for (i = 0; i < types->size; ++i)
    push_type(tc, types->data[i]);
}

static WabtResult check_type_stack_limit(WabtTypeChecker* tc,
                                         size_t expected,
                                         const char* desc) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  size_t avail = tc->type_stack.size - label->type_stack_limit;
  if (!label->unreachable && expected > avail) {
    print_error(tc, "type stack size too small at %s. got %" PRIzd
                    ", expected at least %" PRIzd,
                desc, avail, expected);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult check_type_stack_end(WabtTypeChecker* tc, const char* desc) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  if (tc->type_stack.size != label->type_stack_limit) {
    print_error(tc, "type stack at end of %s is %" PRIzd ", expected %" PRIzd,
                desc, tc->type_stack.size, label->type_stack_limit);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult check_type(WabtTypeChecker* tc,
                             WabtType actual,
                             WabtType expected,
                             const char* desc) {
  if (expected != actual && expected != WABT_TYPE_ANY &&
      actual != WABT_TYPE_ANY) {
    print_error(tc, "type mismatch in %s, expected %s but got %s.", desc,
                wabt_get_type_name(expected), wabt_get_type_name(actual));
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult check_signature(WabtTypeChecker* tc,
                                  const WabtTypeVector* sig,
                                  const char* desc) {
  WabtResult result = WABT_OK;
  size_t i;
  COMBINE_RESULT(result, check_type_stack_limit(tc, sig->size, desc));
  for (i = 0; i < sig->size; ++i) {
    WabtType actual = WABT_TYPE_ANY;
    COMBINE_RESULT(result, peek_type(tc, sig->size - i -  1, &actual));
    COMBINE_RESULT(result, check_type(tc, actual, sig->data[i], desc));
  }
  return result;
}

static WabtResult pop_and_check_signature(WabtTypeChecker* tc,
                                          const WabtTypeVector* sig,
                                          const char* desc) {
  WabtResult result = WABT_OK;
  COMBINE_RESULT(result, check_signature(tc, sig, desc));
  COMBINE_RESULT(result, drop_types(tc, sig->size));
  return result;
}

static WabtResult pop_and_check_call(WabtTypeChecker* tc,
                                     const WabtTypeVector* param_types,
                                     const WabtTypeVector* result_types,
                                     const char* desc) {
  WabtResult result = WABT_OK;
  size_t i;
  COMBINE_RESULT(result, check_type_stack_limit(tc, param_types->size, desc));
  for (i = 0; i < param_types->size; ++i) {
    WabtType actual = WABT_TYPE_ANY;
    COMBINE_RESULT(result, peek_type(tc, param_types->size - i - 1, &actual));
    COMBINE_RESULT(result, check_type(tc, actual, param_types->data[i], desc));
  }
  COMBINE_RESULT(result, drop_types(tc, param_types->size));
  push_types(tc, result_types);
  return result;
}

static WabtResult pop_and_check_1_type(WabtTypeChecker* tc,
                                       WabtType expected,
                                       const char* desc) {
  WabtResult result = WABT_OK;
  WabtType actual = WABT_TYPE_ANY;
  COMBINE_RESULT(result, check_type_stack_limit(tc, 1, desc));
  COMBINE_RESULT(result, pop_type(tc, &actual));
  COMBINE_RESULT(result, check_type(tc, actual, expected, desc));
  return result;
}

static WabtResult pop_and_check_2_types(WabtTypeChecker* tc,
                                        WabtType expected1,
                                        WabtType expected2,
                                        const char* desc) {
  WabtResult result = WABT_OK;
  WabtType actual1 = WABT_TYPE_ANY;
  WabtType actual2 = WABT_TYPE_ANY;
  COMBINE_RESULT(result, check_type_stack_limit(tc, 2, desc));
  COMBINE_RESULT(result, pop_type(tc, &actual2));
  COMBINE_RESULT(result, pop_type(tc, &actual1));
  COMBINE_RESULT(result, check_type(tc, actual1, expected1, desc));
  COMBINE_RESULT(result, check_type(tc, actual2, expected2, desc));
  return result;
}

static WabtResult pop_and_check_2_types_are_equal(WabtTypeChecker* tc,
                                                  WabtType* out_type,
                                                  const char* desc) {
  WabtResult result = WABT_OK;
  WabtType right = WABT_TYPE_ANY;
  WabtType left = WABT_TYPE_ANY;
  COMBINE_RESULT(result, check_type_stack_limit(tc, 2, desc));
  COMBINE_RESULT(result, pop_type(tc, &right));
  COMBINE_RESULT(result, pop_type(tc, &left));
  COMBINE_RESULT(result, check_type(tc, left, right, desc));
  *out_type = right;
  return result;
}

static WabtResult check_opcode1(WabtTypeChecker* tc, WabtOpcode opcode) {
  WabtResult result = pop_and_check_1_type(
      tc, wabt_get_opcode_param_type_1(opcode), wabt_get_opcode_name(opcode));
  push_type(tc, wabt_get_opcode_result_type(opcode));
  return result;
}

static WabtResult check_opcode2(WabtTypeChecker* tc, WabtOpcode opcode) {
  WabtResult result = pop_and_check_2_types(
      tc, wabt_get_opcode_param_type_1(opcode),
      wabt_get_opcode_param_type_2(opcode), wabt_get_opcode_name(opcode));
  push_type(tc, wabt_get_opcode_result_type(opcode));
  return result;
}

void wabt_destroy_typechecker(WabtTypeChecker* tc) {
  wabt_destroy_type_vector(&tc->type_stack);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(tc->label_stack, type_checker_label);
}

WabtResult wabt_typechecker_begin_function(WabtTypeChecker* tc,
                                           const WabtTypeVector* sig) {
  tc->type_stack.size = 0;
  tc->label_stack.size = 0;
  push_label(tc, WABT_LABEL_TYPE_FUNC, sig);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_binary(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode2(tc, opcode);
}

WabtResult wabt_typechecker_on_block(WabtTypeChecker* tc,
                                     const WabtTypeVector* sig) {
  push_label(tc, WABT_LABEL_TYPE_BLOCK, sig);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_br(WabtTypeChecker* tc, size_t depth) {
  WabtResult result = WABT_OK;
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(wabt_typechecker_get_label(tc, depth, &label));
  if (label->label_type != WABT_LABEL_TYPE_LOOP)
    COMBINE_RESULT(result, check_signature(tc, &label->sig, "br"));
  CHECK_RESULT(set_unreachable(tc));
  return result;
}

WabtResult wabt_typechecker_on_br_if(WabtTypeChecker* tc, size_t depth) {
  WabtResult result = WABT_OK;
  COMBINE_RESULT(result, pop_and_check_1_type(tc, WABT_TYPE_I32, "br_if"));
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(wabt_typechecker_get_label(tc, depth, &label));
  if (label->label_type != WABT_LABEL_TYPE_LOOP)
    COMBINE_RESULT(result, check_signature(tc, &label->sig, "br_if"));
  return result;
}

WabtResult wabt_typechecker_begin_br_table(WabtTypeChecker* tc) {
  tc->br_table_sig = WABT_TYPE_ANY;
  return pop_and_check_1_type(tc, WABT_TYPE_I32, "br_table");
}

WabtResult wabt_typechecker_on_br_table_target(WabtTypeChecker* tc,
                                               size_t depth) {
  WabtResult result = WABT_OK;
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(wabt_typechecker_get_label(tc, depth, &label));
  assert(label->sig.size <= 1);
  WabtType label_sig =
      label->sig.size == 0 ? WABT_TYPE_VOID : label->sig.data[0];
  COMBINE_RESULT(result,
                 check_type(tc, tc->br_table_sig, label_sig, "br_table"));
  tc->br_table_sig = label_sig;

  if (label->label_type != WABT_LABEL_TYPE_LOOP)
    COMBINE_RESULT(result, check_signature(tc, &label->sig, "br_table"));
  return result;
}

WabtResult wabt_typechecker_end_br_table(WabtTypeChecker* tc) {
  return set_unreachable(tc);
}

WabtResult wabt_typechecker_on_call(WabtTypeChecker* tc,
                                    const WabtTypeVector* param_types,
                                    const WabtTypeVector* result_types) {
  return pop_and_check_call(tc, param_types, result_types, "call");
}

WabtResult wabt_typechecker_on_call_indirect(
    WabtTypeChecker* tc,
    const WabtTypeVector* param_types,
    const WabtTypeVector* result_types) {
  WabtResult result = WABT_OK;
  COMBINE_RESULT(result,
                 pop_and_check_1_type(tc, WABT_TYPE_I32, "call_indirect"));
  COMBINE_RESULT(result, pop_and_check_call(tc, param_types, result_types,
                                            "call_indirect"));
  return result;
}

WabtResult wabt_typechecker_on_compare(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode2(tc, opcode);
}

WabtResult wabt_typechecker_on_const(WabtTypeChecker* tc, WabtType type) {
  push_type(tc, type);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_convert(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode1(tc, opcode);
}

WabtResult wabt_typechecker_on_current_memory(WabtTypeChecker* tc) {
  push_type(tc, WABT_TYPE_I32);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_drop(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  WabtType type = WABT_TYPE_ANY;
  COMBINE_RESULT(result, check_type_stack_limit(tc, 1, "drop"));
  COMBINE_RESULT(result, pop_type(tc, &type));
  return result;
}

WabtResult wabt_typechecker_on_else(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  COMBINE_RESULT(result, check_label_type(label, WABT_LABEL_TYPE_IF));
  COMBINE_RESULT(result,
                 pop_and_check_signature(tc, &label->sig, "if true branch"));
  COMBINE_RESULT(result, check_type_stack_end(tc, "if true branch"));
  reset_type_stack_to_label(tc, label);
  label->label_type = WABT_LABEL_TYPE_ELSE;
  label->unreachable = WABT_FALSE;
  return result;
}

static WabtResult on_end(WabtTypeChecker* tc,
                         WabtTypeCheckerLabel* label,
                         const char* sig_desc,
                         const char* end_desc) {
  WabtResult result = WABT_OK;
  COMBINE_RESULT(result, pop_and_check_signature(tc, &label->sig, sig_desc));
  COMBINE_RESULT(result, check_type_stack_end(tc, end_desc));
  reset_type_stack_to_label(tc, label);
  push_types(tc, &label->sig);
  pop_label(tc);
  return result;
}

WabtResult wabt_typechecker_on_end(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  static const char* s_label_type_name[] = {"function", "block", "loop", "if",
                                            "if false branch"};
  WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(s_label_type_name) ==
                     WABT_NUM_LABEL_TYPES);
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  assert(label->label_type < WABT_NUM_LABEL_TYPES);
  if (label->label_type == WABT_LABEL_TYPE_IF) {
    if (label->sig.size != 0) {
      print_error(tc, "if without else cannot have type signature.");
      result = WABT_ERROR;
    }
  }
  const char* desc = s_label_type_name[label->label_type];
  COMBINE_RESULT(result, on_end(tc, label, desc, desc));
  return result;
}

WabtResult wabt_typechecker_on_grow_memory(WabtTypeChecker* tc) {
  return check_opcode1(tc, WABT_OPCODE_GROW_MEMORY);
}

WabtResult wabt_typechecker_on_if(WabtTypeChecker* tc,
                                  const WabtTypeVector* sig) {
  WabtResult result = pop_and_check_1_type(tc, WABT_TYPE_I32, "if");
  push_label(tc, WABT_LABEL_TYPE_IF, sig);
  return result;
}

WabtResult wabt_typechecker_on_get_global(WabtTypeChecker* tc, WabtType type) {
  push_type(tc, type);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_get_local(WabtTypeChecker* tc, WabtType type) {
  push_type(tc, type);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_load(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode1(tc, opcode);
}

WabtResult wabt_typechecker_on_loop(WabtTypeChecker* tc,
                                    const WabtTypeVector* sig) {
  push_label(tc, WABT_LABEL_TYPE_LOOP, sig);
  return WABT_OK;
}

WabtResult wabt_typechecker_on_return(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  WabtTypeCheckerLabel* func_label;
  CHECK_RESULT(
      wabt_typechecker_get_label(tc, tc->label_stack.size - 1, &func_label));
  COMBINE_RESULT(result,
                 pop_and_check_signature(tc, &func_label->sig, "return"));
  CHECK_RESULT(set_unreachable(tc));
  return result;
}

WabtResult wabt_typechecker_on_select(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  COMBINE_RESULT(result, pop_and_check_1_type(tc, WABT_TYPE_I32, "select"));
  WabtType type = WABT_TYPE_ANY;
  COMBINE_RESULT(result, pop_and_check_2_types_are_equal(tc, &type, "select"));
  push_type(tc, type);
  return result;
}

WabtResult wabt_typechecker_on_set_global(WabtTypeChecker* tc, WabtType type) {
  return pop_and_check_1_type(tc, type, "set_global");
}

WabtResult wabt_typechecker_on_set_local(WabtTypeChecker* tc, WabtType type) {
  return pop_and_check_1_type(tc, type, "set_local");
}

WabtResult wabt_typechecker_on_store(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode2(tc, opcode);
}

WabtResult wabt_typechecker_on_tee_local(WabtTypeChecker* tc, WabtType type) {
  WabtResult result = WABT_OK;
  WabtType value = WABT_TYPE_ANY;
  COMBINE_RESULT(result, check_type_stack_limit(tc, 1, "tee_local"));
  COMBINE_RESULT(result, top_type(tc, &value));
  COMBINE_RESULT(result, check_type(tc, value, type, "tee_local"));
  return result;
}

WabtResult wabt_typechecker_on_unary(WabtTypeChecker* tc, WabtOpcode opcode) {
  return check_opcode1(tc, opcode);
}

WabtResult wabt_typechecker_on_unreachable(WabtTypeChecker* tc) {
  return set_unreachable(tc);
}

WabtResult wabt_typechecker_end_function(WabtTypeChecker* tc) {
  WabtResult result = WABT_OK;
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(top_label(tc, &label));
  COMBINE_RESULT(result, check_label_type(label, WABT_LABEL_TYPE_FUNC));
  COMBINE_RESULT(result, on_end(tc, label, "implicit return", "function"));
  return result;
}
