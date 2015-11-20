#include "wasm-vector.h"

#include <memory.h>

#define INITIAL_VECTOR_CAPACITY 8

static int ensure_capacity(void** data, size_t* size, size_t* capacity,
                           size_t desired_size, size_t elt_byte_size) {
  if (desired_size > *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : INITIAL_VECTOR_CAPACITY;
    size_t new_byte_size = new_capacity * elt_byte_size;
    *data = realloc(*data, new_byte_size);
    if (*data == NULL)
      return 0;
    *capacity = new_capacity;
  }
  return 1;
}

void* append_element(void** data,
                     size_t* size,
                     size_t* capacity,
                     size_t elt_byte_size) {
  if (!ensure_capacity(data, size, capacity, *size + 1, elt_byte_size))
    return NULL;
  return *data + (*size)++ * elt_byte_size;
}

int extend_elements(void** dst,
                    size_t* dst_size,
                    size_t* dst_capacity,
                    const void** src,
                    size_t src_size,
                    size_t elt_byte_size) {
  if (!ensure_capacity(dst, dst_size, dst_capacity, *dst_size + src_size,
                       elt_byte_size))
    return 0;
  memcpy(*dst + (*dst_size * elt_byte_size), *src, src_size * elt_byte_size);
  *dst_size += src_size;
  return 1;
}
