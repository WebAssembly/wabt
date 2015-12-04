#include "wasm-vector.h"

#include <memory.h>

#define INITIAL_VECTOR_CAPACITY 8

static WasmResult ensure_capacity(void** data,
                                  size_t* capacity,
                                  size_t desired_size,
                                  size_t elt_byte_size) {
  if (desired_size > *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : INITIAL_VECTOR_CAPACITY;
    while (new_capacity < desired_size)
      new_capacity *= 2;
    size_t new_byte_size = new_capacity * elt_byte_size;
    *data = realloc(*data, new_byte_size);
    if (*data == NULL)
      return WASM_ERROR;
    *capacity = new_capacity;
  }
  return WASM_OK;
}

void* append_element(void** data,
                     size_t* size,
                     size_t* capacity,
                     size_t elt_byte_size) {
  if (ensure_capacity(data, capacity, *size + 1, elt_byte_size) == WASM_ERROR)
    return NULL;
  return *data + (*size)++ * elt_byte_size;
}

WasmResult extend_elements(void** dst,
                           size_t* dst_size,
                           size_t* dst_capacity,
                           const void** src,
                           size_t src_size,
                           size_t elt_byte_size) {
  WasmResult result =
      ensure_capacity(dst, dst_capacity, *dst_size + src_size, elt_byte_size);
  if (result != WASM_OK)
    return result;
  memcpy(*dst + (*dst_size * elt_byte_size), *src, src_size * elt_byte_size);
  *dst_size += src_size;
  return result;
}
