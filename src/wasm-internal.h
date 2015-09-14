#ifndef WASM_INTERNAL_H
#define WASM_INTERNAL_H

#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

#endif /* WASM_INTERNAL_H */
