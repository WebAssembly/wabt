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

#ifndef WABT_BINARY_READER_H_
#define WABT_BINARY_READER_H_

#include <stddef.h>
#include <stdint.h>

#include "binary.h"
#include "common.h"

struct WabtAllocator;

#define WABT_READ_BINARY_OPTIONS_DEFAULT \
  { NULL, WABT_FALSE }

typedef struct WabtReadBinaryOptions {
  struct WabtStream* log_stream;
  WabtBool read_debug_names;
} WabtReadBinaryOptions;

typedef struct WabtBinaryReaderContext {
  const uint8_t* data;
  size_t size;
  size_t offset;
  void* user_data;
} WabtBinaryReaderContext;

typedef struct WabtBinaryReader {
  void* user_data;

  void (*on_error)(WabtBinaryReaderContext* ctx, const char* message);

  /* module */
  WabtResult (*begin_module)(uint32_t version, void* user_data);
  WabtResult (*end_module)(void* user_data);

  WabtResult (*begin_section)(WabtBinaryReaderContext* ctx,
                              WabtBinarySection section_type,
                              uint32_t size);

  /* custom section */
  WabtResult (*begin_custom_section)(WabtBinaryReaderContext* ctx,
                                     uint32_t size,
                                     WabtStringSlice section_name);
  WabtResult (*end_custom_section)(WabtBinaryReaderContext* ctx);

  /* signatures section */
  /* TODO(binji): rename to "type" section */
  WabtResult (*begin_signature_section)(WabtBinaryReaderContext* ctx,
                                        uint32_t size);
  WabtResult (*on_signature_count)(uint32_t count, void* user_data);
  WabtResult (*on_signature)(uint32_t index,
                             uint32_t param_count,
                             WabtType* param_types,
                             uint32_t result_count,
                             WabtType* result_types,
                             void* user_data);
  WabtResult (*end_signature_section)(WabtBinaryReaderContext* ctx);

  /* import section */
  WabtResult (*begin_import_section)(WabtBinaryReaderContext* ctx,
                                     uint32_t size);
  WabtResult (*on_import_count)(uint32_t count, void* user_data);
  WabtResult (*on_import)(uint32_t index,
                          WabtStringSlice module_name,
                          WabtStringSlice field_name,
                          void* user_data);
  WabtResult (*on_import_func)(uint32_t import_index,
                               uint32_t func_index,
                               uint32_t sig_index,
                               void* user_data);
  WabtResult (*on_import_table)(uint32_t import_index,
                                uint32_t table_index,
                                WabtType elem_type,
                                const WabtLimits* elem_limits,
                                void* user_data);
  WabtResult (*on_import_memory)(uint32_t import_index,
                                 uint32_t memory_index,
                                 const WabtLimits* page_limits,
                                 void* user_data);
  WabtResult (*on_import_global)(uint32_t import_index,
                                 uint32_t global_index,
                                 WabtType type,
                                 WabtBool mutable_,
                                 void* user_data);
  WabtResult (*end_import_section)(WabtBinaryReaderContext* ctx);

  /* function signatures section */
  /* TODO(binji): rename to "function" section */
  WabtResult (*begin_function_signatures_section)(WabtBinaryReaderContext* ctx,
                                                  uint32_t size);
  WabtResult (*on_function_signatures_count)(uint32_t count, void* user_data);
  WabtResult (*on_function_signature)(uint32_t index,
                                      uint32_t sig_index,
                                      void* user_data);
  WabtResult (*end_function_signatures_section)(WabtBinaryReaderContext* ctx);

  /* table section */
  WabtResult (*begin_table_section)(WabtBinaryReaderContext* ctx,
                                    uint32_t size);
  WabtResult (*on_table_count)(uint32_t count, void* user_data);
  WabtResult (*on_table)(uint32_t index,
                         WabtType elem_type,
                         const WabtLimits* elem_limits,
                         void* user_data);
  WabtResult (*end_table_section)(WabtBinaryReaderContext* ctx);

  /* memory section */
  WabtResult (*begin_memory_section)(WabtBinaryReaderContext* ctx,
                                     uint32_t size);
  WabtResult (*on_memory_count)(uint32_t count, void* user_data);
  WabtResult (*on_memory)(uint32_t index,
                          const WabtLimits* limits,
                          void* user_data);
  WabtResult (*end_memory_section)(WabtBinaryReaderContext* ctx);

  /* global section */
  WabtResult (*begin_global_section)(WabtBinaryReaderContext* ctx,
                                     uint32_t size);
  WabtResult (*on_global_count)(uint32_t count, void* user_data);
  WabtResult (*begin_global)(uint32_t index,
                             WabtType type,
                             WabtBool mutable_,
                             void* user_data);
  WabtResult (*begin_global_init_expr)(uint32_t index, void* user_data);
  WabtResult (*end_global_init_expr)(uint32_t index, void* user_data);
  WabtResult (*end_global)(uint32_t index, void* user_data);
  WabtResult (*end_global_section)(WabtBinaryReaderContext* ctx);

  /* exports section */
  WabtResult (*begin_export_section)(WabtBinaryReaderContext* ctx,
                                     uint32_t size);
  WabtResult (*on_export_count)(uint32_t count, void* user_data);
  WabtResult (*on_export)(uint32_t index,
                          WabtExternalKind kind,
                          uint32_t item_index,
                          WabtStringSlice name,
                          void* user_data);
  WabtResult (*end_export_section)(WabtBinaryReaderContext* ctx);

  /* start section */
  WabtResult (*begin_start_section)(WabtBinaryReaderContext* ctx,
                                    uint32_t size);
  WabtResult (*on_start_function)(uint32_t func_index, void* user_data);
  WabtResult (*end_start_section)(WabtBinaryReaderContext* ctx);

  /* function bodies section */
  /* TODO(binji): rename to code section */
  WabtResult (*begin_function_bodies_section)(WabtBinaryReaderContext* ctx,
                                              uint32_t size);
  WabtResult (*on_function_bodies_count)(uint32_t count, void* user_data);
  WabtResult (*begin_function_body_pass)(uint32_t index,
                                         uint32_t pass,
                                         void* user_data);
  WabtResult (*begin_function_body)(WabtBinaryReaderContext* ctx,
                                    uint32_t index);
  WabtResult (*on_local_decl_count)(uint32_t count, void* user_data);
  WabtResult (*on_local_decl)(uint32_t decl_index,
                              uint32_t count,
                              WabtType type,
                              void* user_data);

  /* function expressions; called between begin_function_body and
   end_function_body */
  WabtResult (*on_opcode)(WabtBinaryReaderContext* ctx, WabtOpcode Opcode);
  WabtResult (*on_opcode_bare)(WabtBinaryReaderContext* ctx);
  WabtResult (*on_opcode_uint32)(WabtBinaryReaderContext* ctx, uint32_t value);
  WabtResult (*on_opcode_uint32_uint32)(WabtBinaryReaderContext* ctx,
                                        uint32_t value,
                                        uint32_t value2);
  WabtResult (*on_opcode_uint64)(WabtBinaryReaderContext* ctx, uint64_t value);
  WabtResult (*on_opcode_f32)(WabtBinaryReaderContext* ctx, uint32_t value);
  WabtResult (*on_opcode_f64)(WabtBinaryReaderContext* ctx, uint64_t value);
  WabtResult (*on_opcode_block_sig)(WabtBinaryReaderContext* ctx,
                                    uint32_t num_types,
                                    WabtType* sig_types);
  WabtResult (*on_binary_expr)(WabtOpcode opcode, void* user_data);
  WabtResult (*on_block_expr)(uint32_t num_types,
                              WabtType* sig_types,
                              void* user_data);
  WabtResult (*on_br_expr)(uint32_t depth, void* user_data);
  WabtResult (*on_br_if_expr)(uint32_t depth, void* user_data);
  WabtResult (*on_br_table_expr)(WabtBinaryReaderContext* ctx,
                                 uint32_t num_targets,
                                 uint32_t* target_depths,
                                 uint32_t default_target_depth);
  WabtResult (*on_call_expr)(uint32_t func_index, void* user_data);
  WabtResult (*on_call_import_expr)(uint32_t import_index, void* user_data);
  WabtResult (*on_call_indirect_expr)(uint32_t sig_index, void* user_data);
  WabtResult (*on_compare_expr)(WabtOpcode opcode, void* user_data);
  WabtResult (*on_convert_expr)(WabtOpcode opcode, void* user_data);
  WabtResult (*on_drop_expr)(void* user_data);
  WabtResult (*on_else_expr)(void* user_data);
  WabtResult (*on_end_expr)(void* user_data);
  WabtResult (*on_f32_const_expr)(uint32_t value_bits, void* user_data);
  WabtResult (*on_f64_const_expr)(uint64_t value_bits, void* user_data);
  WabtResult (*on_get_global_expr)(uint32_t global_index, void* user_data);
  WabtResult (*on_get_local_expr)(uint32_t local_index, void* user_data);
  WabtResult (*on_grow_memory_expr)(void* user_data);
  WabtResult (*on_i32_const_expr)(uint32_t value, void* user_data);
  WabtResult (*on_i64_const_expr)(uint64_t value, void* user_data);
  WabtResult (*on_if_expr)(uint32_t num_types,
                           WabtType* sig_types,
                           void* user_data);
  WabtResult (*on_load_expr)(WabtOpcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset,
                             void* user_data);
  WabtResult (*on_loop_expr)(uint32_t num_types,
                             WabtType* sig_types,
                             void* user_data);
  WabtResult (*on_current_memory_expr)(void* user_data);
  WabtResult (*on_nop_expr)(void* user_data);
  WabtResult (*on_return_expr)(void* user_data);
  WabtResult (*on_select_expr)(void* user_data);
  WabtResult (*on_set_global_expr)(uint32_t global_index, void* user_data);
  WabtResult (*on_set_local_expr)(uint32_t local_index, void* user_data);
  WabtResult (*on_store_expr)(WabtOpcode opcode,
                              uint32_t alignment_log2,
                              uint32_t offset,
                              void* user_data);
  WabtResult (*on_tee_local_expr)(uint32_t local_index, void* user_data);
  WabtResult (*on_unary_expr)(WabtOpcode opcode, void* user_data);
  WabtResult (*on_unreachable_expr)(void* user_data);
  WabtResult (*end_function_body)(uint32_t index, void* user_data);
  WabtResult (*end_function_body_pass)(uint32_t index,
                                       uint32_t pass,
                                       void* user_data);
  WabtResult (*end_function_bodies_section)(WabtBinaryReaderContext* ctx);

  /* elem section */
  WabtResult (*begin_elem_section)(WabtBinaryReaderContext* ctx, uint32_t size);
  WabtResult (*on_elem_segment_count)(uint32_t count, void* user_data);
  WabtResult (*begin_elem_segment)(uint32_t index,
                                   uint32_t table_index,
                                   void* user_data);
  WabtResult (*begin_elem_segment_init_expr)(uint32_t index, void* user_data);
  WabtResult (*end_elem_segment_init_expr)(uint32_t index, void* user_data);
  WabtResult (*on_elem_segment_function_index_count)(
      WabtBinaryReaderContext* ctx,
      uint32_t index,
      uint32_t count);
  WabtResult (*on_elem_segment_function_index)(uint32_t index,
                                               uint32_t func_index,
                                               void* user_data);
  WabtResult (*end_elem_segment)(uint32_t index, void* user_data);
  WabtResult (*end_elem_section)(WabtBinaryReaderContext* ctx);

  /* data section */
  WabtResult (*begin_data_section)(WabtBinaryReaderContext* ctx, uint32_t size);
  WabtResult (*on_data_segment_count)(uint32_t count, void* user_data);
  WabtResult (*begin_data_segment)(uint32_t index,
                                   uint32_t memory_index,
                                   void* user_data);
  WabtResult (*begin_data_segment_init_expr)(uint32_t index, void* user_data);
  WabtResult (*end_data_segment_init_expr)(uint32_t index, void* user_data);
  WabtResult (*on_data_segment_data)(uint32_t index,
                                     const void* data,
                                     uint32_t size,
                                     void* user_data);
  WabtResult (*end_data_segment)(uint32_t index, void* user_data);
  WabtResult (*end_data_section)(WabtBinaryReaderContext* ctx);

  /* names section */
  WabtResult (*begin_names_section)(WabtBinaryReaderContext* ctx,
                                    uint32_t size);
  WabtResult (*on_function_names_count)(uint32_t count, void* user_data);
  WabtResult (*on_function_name)(uint32_t index,
                                 WabtStringSlice name,
                                 void* user_data);
  WabtResult (*on_local_names_count)(uint32_t index,
                                     uint32_t count,
                                     void* user_data);
  WabtResult (*on_local_name)(uint32_t func_index,
                              uint32_t local_index,
                              WabtStringSlice name,
                              void* user_data);
  WabtResult (*end_names_section)(WabtBinaryReaderContext* ctx);

  /* names section */
  WabtResult (*begin_reloc_section)(WabtBinaryReaderContext* ctx,
                                    uint32_t size);
  WabtResult (*on_reloc_count)(uint32_t count,
                               WabtBinarySection section_code,
                               WabtStringSlice section_name,
                               void* user_data);
  WabtResult (*on_reloc)(WabtRelocType type,
                         uint32_t offset,
                         void* user_data);
  WabtResult (*end_reloc_section)(WabtBinaryReaderContext* ctx);

  /* init_expr - used by elem, data and global sections; these functions are
   * only called between calls to begin_*_init_expr and end_*_init_expr */
  WabtResult (*on_init_expr_f32_const_expr)(uint32_t index,
                                            uint32_t value,
                                            void* user_data);
  WabtResult (*on_init_expr_f64_const_expr)(uint32_t index,
                                            uint64_t value,
                                            void* user_data);
  WabtResult (*on_init_expr_get_global_expr)(uint32_t index,
                                             uint32_t global_index,
                                             void* user_data);
  WabtResult (*on_init_expr_i32_const_expr)(uint32_t index,
                                            uint32_t value,
                                            void* user_data);
  WabtResult (*on_init_expr_i64_const_expr)(uint32_t index,
                                            uint64_t value,
                                            void* user_data);
} WabtBinaryReader;

WABT_EXTERN_C_BEGIN
WabtResult wabt_read_binary(struct WabtAllocator* allocator,
                            const void* data,
                            size_t size,
                            WabtBinaryReader* reader,
                            uint32_t num_function_passes,
                            const WabtReadBinaryOptions* options);

size_t wabt_read_u32_leb128(const uint8_t* ptr,
                            const uint8_t* end,
                            uint32_t* out_value);

size_t wabt_read_i32_leb128(const uint8_t* ptr,
                            const uint8_t* end,
                            uint32_t* out_value);
WABT_EXTERN_C_END

#endif /* WABT_BINARY_READER_H_ */
