#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm.h"
#include "wasm-internal.h"
#include "wasm-parse.h"

#define TABS_TO_SPACES 8
#define INITIAL_VECTOR_CAPACITY 8

#define FATAL_AT(parser, loc, ...) fatal_at(parser, loc, __VA_ARGS__)
#define CALLBACK(parser, name, args) \
  (((parser)->callbacks->name) ? (parser)->callbacks->name args : 0)
#define CHECK_ALLOC(parser, p, loc) \
  if (!p)                           \
    fatal_at(parser, loc, "allocation of " #p " failed");

typedef enum WasmMemType {
  WASM_MEM_TYPE_I8,
  WASM_MEM_TYPE_I16,
  WASM_MEM_TYPE_I32,
  WASM_MEM_TYPE_I64,
  WASM_MEM_TYPE_F32,
  WASM_MEM_TYPE_F64,
} WasmMemType;

typedef enum WasmOpType {
  WASM_OP_NONE,
  WASM_OP_ASSERT_INVALID,
  WASM_OP_ASSERT_RETURN,
  WASM_OP_ASSERT_RETURN_NAN,
  WASM_OP_ASSERT_TRAP,
  WASM_OP_BINARY,
  WASM_OP_BLOCK,
  WASM_OP_BREAK,
  WASM_OP_CALL,
  WASM_OP_CALL_IMPORT,
  WASM_OP_CALL_INDIRECT,
  WASM_OP_COMPARE,
  WASM_OP_CONST,
  WASM_OP_CONVERT,
  WASM_OP_DESTRUCT,
  WASM_OP_EXPORT,
  WASM_OP_FUNC,
  WASM_OP_GET_LOCAL,
  WASM_OP_GLOBAL,
  WASM_OP_IF,
  WASM_OP_IMPORT,
  WASM_OP_INVOKE,
  WASM_OP_LABEL,
  WASM_OP_LOAD,
  WASM_OP_LOAD_GLOBAL,
  WASM_OP_LOCAL,
  WASM_OP_LOOP,
  WASM_OP_MEMORY,
  WASM_OP_MEMORY_SIZE,
  WASM_OP_MODULE,
  WASM_OP_NOP,
  WASM_OP_PAGE_SIZE,
  WASM_OP_PARAM,
  WASM_OP_RESIZE_MEMORY,
  WASM_OP_RESULT,
  WASM_OP_RETURN,
  WASM_OP_SET_LOCAL,
  WASM_OP_STORE,
  WASM_OP_STORE_GLOBAL,
  WASM_OP_SWITCH,
  WASM_OP_TABLE,
  WASM_OP_TYPE,
  WASM_OP_UNARY,
} WasmOpType;

enum {
  WASM_CONTINUATION_NORMAL = 1,
  WASM_CONTINUATION_RETURN = 2,
  WASM_CONTINUATION_BREAK_0 = 4,
};
typedef uint32_t WasmContinuation;
#define CONTINUATION_BREAK_MASK (~(WasmContinuation)3)
#define MAX_BREAK_DEPTH 30

#include "wasm-keywords.h"

typedef struct OpInfo OpInfo;

typedef enum WasmTokenType {
  WASM_TOKEN_TYPE_EOF,
  WASM_TOKEN_TYPE_OPEN_PAREN,
  WASM_TOKEN_TYPE_CLOSE_PAREN,
  WASM_TOKEN_TYPE_ATOM,
  WASM_TOKEN_TYPE_STRING,
} WasmTokenType;

typedef struct WasmSourceRange {
  WasmSourceLocation start;
  WasmSourceLocation end;
} WasmSourceRange;

typedef struct WasmToken {
  WasmTokenType type;
  WasmSourceRange range;
} WasmToken;

typedef struct WasmTokenizer {
  WasmSource source;
  WasmSourceLocation loc;
} WasmTokenizer;

typedef struct WasmParser {
  WasmParserCallbacks* callbacks;
  WasmTokenizer tokenizer;
  WasmParserTypeCheck type_check;
  jmp_buf jump_buf;
  void* user_data; /* copied from callbacks->user_data, for convenience */
} WasmParser;

static void* append_element(void** data,
                            size_t* size,
                            size_t* capacity,
                            size_t elt_size);

#define DEFINE_VECTOR(name, type)                                           \
  void wasm_destroy_##name##_vector(type##Vector* vec) { free(vec->data); } \
  type* wasm_append_##name(type##Vector* vec) {                             \
    return append_element((void**)&vec->data, &vec->size, &vec->capacity,   \
                          sizeof(type));                                    \
  }

DEFINE_VECTOR(binding, WasmBinding)
DEFINE_VECTOR(variable, WasmVariable)
DEFINE_VECTOR(signature, WasmSignature)
DEFINE_VECTOR(function, WasmFunction)
DEFINE_VECTOR(import, WasmImport)
DEFINE_VECTOR(segment, WasmSegment)
DEFINE_VECTOR(label, WasmLabel)

typedef struct NameTypePair {
  const char* name;
  WasmType type;
} NameTypePair;

static NameTypePair s_types[] = {
    {"i32", WASM_TYPE_I32},
    {"i64", WASM_TYPE_I64},
    {"f32", WASM_TYPE_F32},
    {"f64", WASM_TYPE_F64},
};

static const char* s_type_names[] = {
    "void",
    "i32",
    "i64",
    "i32 or i64",
    "f32",
    "i32 or f32",
    "i64 or f32",
    "i32, i64 or f32",
    "f64",
    "i32 or f64",
    "i64 or f64",
    "i32, i64 or f64",
    "f32 or f64",
    "i32, f32 or f64",
    "i64, f32 or f64",
    "i32, i64, f32 or f64",
};
STATIC_ASSERT(ARRAY_SIZE(s_type_names) == WASM_TYPE_ALL + 1);

static int is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static int hexdigit(char c, uint32_t* out) {
  if ((unsigned int)(c - '0') <= 9) {
    *out = c - '0';
    return 1;
  } else if ((unsigned int)(c - 'a') <= 6) {
    *out = 10 + (c - 'a');
    return 1;
  } else if ((unsigned int)(c - 'A') <= 6) {
    *out = 10 + (c - 'A');
    return 1;
  }
  return 0;
}

/* return 1 if the non-NULL-terminated string starting with |start| and ending
 with |end| is equal to the NULL-terminated string |other|. */
static int string_eq(const char* start, const char* end, const char* other) {
  while (start < end && *other) {
    if (*start != *other)
      return 0;
    start++;
    other++;
  }
  return start == end && *other == 0;
}

/* return 1 if the non-NULL-terminated string starting with |start| and ending
 with |end| starts with the NULL-terminated string |prefix|. */
static int string_starts_with(const char* start,
                              const char* end,
                              const char* prefix) {
  while (start < end && *prefix) {
    if (*start != *prefix)
      return 0;
    start++;
    prefix++;
  }
  return *prefix == 0;
}

static void fatal_at(WasmParser* parser,
                     WasmSourceLocation loc,
                     const char* format,
                     ...) __attribute__((noreturn));
static void fatal_at(WasmParser* parser,
                     WasmSourceLocation loc,
                     const char* format,
                     ...) {
  va_list args;
  va_list args_copy;

  va_start(args, format);
  va_copy(args_copy, args);
  int len = vsnprintf(NULL, 0, format, args) + 1; /* +1 for \0 */
  va_end(args);

  char* buffer = alloca(len);
  vsnprintf(buffer, len, format, args_copy);
  va_end(args_copy);

  CALLBACK(parser, error, (loc, buffer, parser->user_data));

  longjmp(parser->jump_buf, 1);
}

static void* append_element(void** data,
                            size_t* size,
                            size_t* capacity,
                            size_t elt_size) {
  if (*size + 1 > *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : INITIAL_VECTOR_CAPACITY;
    size_t new_size = new_capacity * elt_size;
    *data = realloc(*data, new_size);
    if (*data == NULL)
      return NULL;
    *capacity = new_capacity;
  }
  return *data + (*size)++ * elt_size;
}

static int get_binding_by_name(WasmBindingVector* bindings, const char* name) {
  int i;
  for (i = 0; i < bindings->size; ++i) {
    WasmBinding* binding = &bindings->data[i];
    if (binding->name && strcmp(name, binding->name) == 0)
      return i;
  }
  return -1;
}

static int get_function_by_export_name(WasmFunctionVector* functions,
                                       const char* name) {
  int i;
  for (i = 0; i < functions->size; ++i) {
    WasmFunction* function = &functions->data[i];
    if (function->exported) {
      WasmExportedNameList* exported_name = &function->exported_name;
      while (exported_name) {
        if (strcmp(name, exported_name->name) == 0)
          return i;
        exported_name = exported_name->next;
      }
    }
  }
  return -1;
}

static const OpInfo* get_op_info(WasmToken t) {
  return in_word_set(t.range.start.pos,
                     (int)(t.range.end.pos - t.range.start.pos));
}

static int read_uint64(const char* s, const char* end, uint64_t* out) {
  if (s == end)
    return 0;
  uint64_t value = 0;
  if (*s == '0' && s + 1 < end && s[1] == 'x') {
    s += 2;
    if (s == end)
      return 0;
    for (; s < end; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      uint64_t old_value = value;
      value = value * 16 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  } else {
    for (; s < end; ++s) {
      uint32_t digit = (*s - '0');
      if (digit > 9)
        return 0;
      uint64_t old_value = value;
      value = value * 10 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  }
  if (s != end)
    return 0;
  *out = value;
  return 1;
}

static int read_int64(const char* s, const char* end, uint64_t* out) {
  int has_sign = 0;
  if (*s == '-') {
    has_sign = 1;
    s++;
  }
  uint64_t value;
  int result = read_uint64(s, end, &value);
  if (has_sign) {
    if (value > (uint64_t)INT64_MAX + 1) /* abs(INT64_MIN) == INT64_MAX + 1 */
      return 0;
    value = UINT64_MAX - value + 1;
  }
  *out = value;
  return result;
}

static int read_int32(const char* s,
                      const char* end,
                      uint32_t* out,
                      int allow_signed) {
  uint64_t value;
  int has_sign = 0;
  if (*s == '-') {
    if (!allow_signed)
      return 0;
    has_sign = 1;
    s++;
  }
  if (!read_uint64(s, end, &value))
    return 0;

  if (has_sign) {
    if (value > (uint64_t)INT32_MAX + 1) /* abs(INT32_MIN) == INT32_MAX + 1 */
      return 0;
    value = UINT32_MAX - value + 1;
  } else {
    if (value > (uint64_t)UINT32_MAX)
      return 0;
  }
  *out = (uint32_t)value;
  return 1;
}

static int read_float_nan(const char* s, const char* end, float* out) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  if (!string_starts_with(s, end, "nan"))
    return 0;
  s += 3;

  uint32_t tag;
  if (s != end) {
    tag = 0;
    if (!string_starts_with(s, end, "(0x"))
      return 0;
    s += 3;

    for (; s < end && *s != ')'; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      const uint32_t max_tag = 0x7fffff;
      if (tag > max_tag)
        return 0;
    }

    if (s + 1 != end || *s != ')')
      return 0;

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x400000;
  }

  uint32_t bits = 0x7f800000 | tag;
  if (is_neg)
    bits |= 0x80000000U;
  memcpy(out, &bits, sizeof(*out));
  return 1;
}

static int read_float(const char* s, const char* end, float* out) {
  if (read_float_nan(s, end, out))
    return 1;

  errno = 0;
  char* endptr;
  float value;
  value = strtof(s, &endptr);
  if (endptr != end ||
      ((value == 0 || value == HUGE_VALF || value == -HUGE_VALF) && errno != 0))
    return 0;

  *out = value;
  return 1;
}

static int read_double_nan(const char* s, const char* end, double* out) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  if (!string_starts_with(s, end, "nan"))
    return 0;
  s += 3;

  uint64_t tag;
  if (s != end) {
    tag = 0;
    if (!string_starts_with(s, end, "(0x"))
      return 0;
    s += 3;

    for (; s < end && *s != ')'; ++s) {
      uint32_t digit;
      if (!hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      const uint64_t max_tag = 0xfffffffffffffULL;
      if (tag > max_tag)
        return 0;
    }

    if (s + 1 != end || *s != ')')
      return 0;

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x8000000000000ULL;
  }

  uint64_t bits = 0x7ff0000000000000ULL | tag;
  if (is_neg)
    bits |= 0x8000000000000000ULL;
  memcpy(out, &bits, sizeof(*out));
  return 1;
}

static int read_double(const char* s, const char* end, double* out) {
  if (read_double_nan(s, end, out))
    return 1;

  errno = 0;
  char* endptr;
  double value;
  value = strtod(s, &endptr);
  if (endptr != end ||
      ((value == 0 || value == HUGE_VAL || value == -HUGE_VAL) && errno != 0))
    return 0;

  *out = value;
  return 1;
}

static int read_uint32_token(WasmToken t, uint32_t* out) {
  const char* p = t.range.start.pos;
  return read_int32(p, t.range.end.pos, out, 0);
}

static WasmToken read_token(WasmParser* parser) {
  WasmToken result;
  WasmTokenizer* t = &parser->tokenizer;

  while (t->loc.pos < t->source.end) {
    switch (*t->loc.pos) {
      case ' ':
        t->loc.col++;
        t->loc.pos++;
        break;

      case '\t':
        t->loc.col += TABS_TO_SPACES;
        t->loc.pos++;
        break;

      case '\n':
        t->loc.line++;
        t->loc.col = 1;
        t->loc.pos++;
        break;

      case '"':
        result.type = WASM_TOKEN_TYPE_STRING;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;

        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case '\\':
              if (t->loc.pos + 1 < t->source.end) {
                t->loc.col++;
                t->loc.pos++;
                if (*t->loc.pos == 'n' || *t->loc.pos == 't' ||
                    *t->loc.pos == '\\' || *t->loc.pos == '\'' ||
                    *t->loc.pos == '"') {
                  /* newline or tab */
                } else if (isxdigit(*t->loc.pos)) {
                  /* \xx for arbitrary value */
                  if (t->loc.pos + 1 < t->source.end) {
                    t->loc.col++;
                    t->loc.pos++;
                    if (!isxdigit(*t->loc.pos))
                      FATAL_AT(parser, t->loc, "bad \\xx escape sequence\n");
                  } else {
                    FATAL_AT(parser, t->loc, "eof in \\xx escape sequence\n");
                  }
                } else {
                  FATAL_AT(parser, t->loc, "bad escape sequence\n");
                }
              } else {
                FATAL_AT(parser, t->loc, "eof in escape sequence\n");
              }
              break;

            case '\n':
              FATAL_AT(parser, t->loc, "newline in string\n");
              break;

            case '"':
              t->loc.col++;
              t->loc.pos++;
              goto done_string;
          }

          t->loc.col++;
          t->loc.pos++;
        }
      done_string:
        result.range.end = t->loc;
        return result;

      case ';':
        if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
          /* line comment */
          while (t->loc.pos < t->source.end) {
            if (*t->loc.pos == '\n')
              break;

            t->loc.col++;
            t->loc.pos++;
          }
        } else {
          FATAL_AT(parser, t->loc, "unexpected semicolon\n");
        }
        break;

      case '(':
        if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
          int nesting = 1;
          t->loc.pos += 2;
          t->loc.col += 2;
          while (nesting > 0 && t->loc.pos < t->source.end) {
            switch (*t->loc.pos) {
              case ';':
                if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ')') {
                  nesting--;
                  t->loc.col++;
                  t->loc.pos++;
                }
                t->loc.col++;
                t->loc.pos++;
                break;

              case '(':
                if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
                  nesting++;
                  t->loc.col++;
                  t->loc.pos++;
                }
                t->loc.col++;
                t->loc.pos++;
                break;

              case '\t':
                t->loc.col += TABS_TO_SPACES;
                t->loc.pos++;
                break;

              case '\n':
                t->loc.line++;
                t->loc.col = 1;
                t->loc.pos++;
                break;

              default:
                t->loc.pos++;
                t->loc.col++;
                break;
            }
          }
          break;
        } else {
          result.type = WASM_TOKEN_TYPE_OPEN_PAREN;
          result.range.start = t->loc;
          t->loc.col++;
          t->loc.pos++;
          result.range.end = t->loc;
          return result;
        }
        break;

      case ')':
        result.type = WASM_TOKEN_TYPE_CLOSE_PAREN;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        result.range.end = t->loc;
        return result;

      default: {
        const char* start = t->loc.pos;
        int is_nan = 0;
        result.type = WASM_TOKEN_TYPE_ATOM;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case '(': {
              /* special case for a nan value, e.g. -nan(0xaabb) */
              const char* end = t->loc.pos;
              if (string_eq(start, end, "nan") ||
                  string_eq(start, end, "-nan") ||
                  string_eq(start, end, "+nan")) {
                is_nan = 1;
                goto continue_atom;
              }
              /* fallthrough, this is not a nan atom */
            }

            case ' ':
            case '\t':
            case '\n':
              result.range.end = t->loc;
              return result;

            continue_atom:
            default:
              t->loc.col++;
              t->loc.pos++;
              break;

            case ')':
              /* if this is an nan bits atom, keep the closing paren */
              if (is_nan) {
                t->loc.col++;
                t->loc.pos++;
              }
              result.range.end = t->loc;
              return result;
          }
        }
        break;
      }
    }
  }

  result.type = WASM_TOKEN_TYPE_EOF;
  result.range.start = t->loc;
  result.range.end = t->loc;
  return result;
}

static void rewind_token(WasmParser* parser, WasmToken t) {
  parser->tokenizer.loc = t.range.start;
}

static size_t string_contents_length(WasmToken t) {
  assert(t.type == WASM_TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;
  size_t length = 0;
  while (src < end) {
    if (*src == '\\') {
      src++;
      switch (*src) {
        case 'n':
        case 't':
        case '\\':
        case '\'':
        case '\"':
          src++;
          length++;
          break;

        default:
          /* must be a hex sequence */
          src += 2;
          length++;
          break;
      }
    } else {
      src++;
      length++;
    }
  }
  return length;
}

static size_t copy_string_contents(WasmToken t, char* dest, size_t size) {
  assert(t.type == WASM_TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;

  char* dest_start = dest;

  while (src < end) {
    if (*src == '\\') {
      src++;
      switch (*src) {
        case 'n':
          *dest++ = '\n';
          break;
        case 't':
          *dest++ = '\t';
          break;
        case '\\':
          *dest++ = '\\';
          break;
        case '\'':
          *dest++ = '\'';
          break;
        case '\"':
          *dest++ = '\"';
          break;
        default: {
          /* The string should be validated already, so we know this is a hex
           * sequence */
          uint32_t hi;
          uint32_t lo;
          if (!hexdigit(src[0], &hi) || !hexdigit(src[1], &lo))
            assert(0);
          *dest++ = (hi << 4) | lo;
          src++;
          break;
        }
      }
      src++;
    } else {
      *dest++ = *src++;
    }
  }
  /* return the data length */
  return dest - dest_start;
}

void wasm_copy_segment_data(WasmSegmentData data, char* dest, size_t size) {
  copy_string_contents(*(WasmToken*)data, dest, size);
}

static char* dup_string_contents(WasmToken t) {
  assert(t.type == WASM_TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src + 1; /* +1 for \0 */
  char* result = malloc(size);
  size_t actual_size = copy_string_contents(t, result, size);
  result[actual_size] = '\0';
  return result;
}

static void expect(WasmParser* parser,
                   WasmToken t,
                   WasmTokenType token_type,
                   const char* desc) {
  if (t.type == WASM_TOKEN_TYPE_EOF) {
    FATAL_AT(parser, t.range.start, "unexpected EOF\n");
  } else if (t.type != token_type) {
    FATAL_AT(parser, t.range.start, "expected %s, not \"%.*s\"\n", desc,
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  }
}

static void unexpected_token(WasmParser* parser, WasmToken t) {
  if (t.type == WASM_TOKEN_TYPE_EOF) {
    FATAL_AT(parser, t.range.start, " unexpected EOF\n");
  } else {
    FATAL_AT(parser, t.range.start, "unexpected token \"%.*s\"\n",
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  }
}

static void expect_open(WasmParser* parser, WasmToken t) {
  expect(parser, t, WASM_TOKEN_TYPE_OPEN_PAREN, "'('");
}

static void expect_close(WasmParser* parser, WasmToken t) {
  expect(parser, t, WASM_TOKEN_TYPE_CLOSE_PAREN, "')'");
}

static void expect_atom(WasmParser* parser, WasmToken t) {
  expect(parser, t, WASM_TOKEN_TYPE_ATOM, "ATOM");
}

static void expect_string(WasmParser* parser, WasmToken t) {
  expect(parser, t, WASM_TOKEN_TYPE_STRING, "STRING");
}

static void expect_var_name(WasmParser* parser, WasmToken t) {
  expect_atom(parser, t);
  if (t.range.end.pos - t.range.start.pos < 1 || t.range.start.pos[0] != '$')
    FATAL_AT(parser, t.range.start, "expected name to begin with $\n");
}

static void expect_atom_op(WasmParser* parser,
                           WasmToken t,
                           WasmOpType op,
                           const char* desc) {
  expect_atom(parser, t);
  const OpInfo* op_info = get_op_info(t);
  if (!op_info || op_info->op_type != op)
    FATAL_AT(parser, t.range.start, "expected \"%s\"\n", desc);
}

static void check_opcode(WasmParser* parser,
                         WasmSourceLocation loc,
                         WasmOpcode opcode) {
  if (opcode == WASM_OPCODE_INVALID)
    FATAL_AT(parser, loc, "no opcode for instruction\n");
}

static int match_atom(WasmToken t, const char* s) {
  return string_eq(t.range.start.pos, t.range.end.pos, s);
}

static int match_type(WasmToken t, WasmType* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_types); ++i) {
    if (match_atom(t, s_types[i].name)) {
      if (type)
        *type = s_types[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_load_store_aligned(WasmParser* parser,
                                    WasmToken t,
                                    OpInfo* op_info) {
  /* look for a slash near the end of the string */
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos - 1;
  while (end > p) {
    if (*end == '/') {
      const OpInfo* found = in_word_set(p, (int)(end - p));
      if (!(found && (found->op_type == WASM_OP_LOAD ||
                      found->op_type == WASM_OP_STORE)))
        return 0;

      ++end;
      uint32_t alignment;
      if (!read_int32(end, t.range.end.pos, &alignment, 0))
        return 0;

      /* check that alignment is power-of-two */
      if (!is_power_of_two(alignment))
        FATAL_AT(parser, t.range.start, "alignment must be power-of-two\n");

      *op_info = *found;
#if 0
  /* TODO(binji): setting the unaligned memory access bit is currently an error
   in v8-native-prototype. */

      int native_mem_type_alignment[] = {
          1, /* MEM_TYPE_I8 */
          2, /* MEM_TYPE_I16 */
          4, /* MEM_TYPE_I32 */
          8, /* MEM_TYPE_I64 */
          4, /* MEM_TYPE_F32 */
          8, /* MEM_TYPE_F64 */
      };

      if (alignment && alignment < native_mem_type_alignment[op_info->type2])
        op_info->access |= 0x8; /* unaligned access */
#endif
      return 1;
    }
    end--;
  }
  return 0;
}

static void check_type(WasmParser* parser,
                       WasmSourceLocation loc,
                       WasmType actual,
                       WasmType expected,
                       const char* desc) {
  if (actual != expected) {
    FATAL_AT(parser, loc, "type mismatch%s. got %s, expected %s\n", desc,
             s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg(WasmParser* parser,
                           WasmSourceLocation loc,
                           WasmType actual,
                           WasmType expected,
                           const char* desc,
                           int arg_index) {
  if (actual != expected) {
    FATAL_AT(parser, loc,
             "type mismatch for argument %d of %s. got %s, expected %s\n",
             arg_index, desc, s_type_names[actual], s_type_names[expected]);
  }
}

static WasmType check_label_type(WasmParser* parser,
                                 WasmSourceLocation loc,
                                 WasmLabel* label,
                                 WasmType type,
                                 WasmContinuation continuation,
                                 const char* desc) {
  if (parser->type_check == WASM_PARSER_TYPE_CHECK_SPEC) {
    return type & label->type;
  } else {
    assert(parser->type_check == WASM_PARSER_TYPE_CHECK_V8_NATIVE);
    if (label->type != WASM_TYPE_ALL) {
      if (type != WASM_TYPE_VOID || continuation & WASM_CONTINUATION_NORMAL) {
        check_type(parser, loc, type, label->type, desc);
      }
      return label->type;
    } else {
      return WASM_TYPE_VOID;
    }
  }
}

static WasmLabel* push_anonymous_label(WasmParser* parser,
                                       WasmFunction* function,
                                       WasmToken t) {
  WasmBinding* binding = wasm_append_binding(&function->label_bindings);
  CHECK_ALLOC(parser, binding, t.range.start);
  binding->name = NULL;
  binding->index = function->label_bindings.size;

  WasmLabel* label = wasm_append_label(&function->labels);
  CHECK_ALLOC(parser, label, t.range.start);
  label->type = WASM_TYPE_ALL;
  return label;
}

static WasmLabel* push_label(WasmParser* parser,
                             WasmFunction* function,
                             WasmToken t) {
  expect_var_name(parser, t);
  WasmBinding* binding = wasm_append_binding(&function->label_bindings);
  CHECK_ALLOC(parser, binding, t.range.start);
  binding->name =
      strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
  binding->index = function->label_bindings.size;

  WasmLabel* label = wasm_append_label(&function->labels);
  CHECK_ALLOC(parser, label, t.range.start);
  label->type = WASM_TYPE_ALL;
  return label;
}

static void pop_label(WasmFunction* function) {
  assert(function->label_bindings.size > 0);
  assert(function->labels.size > 0);
  free(function->label_bindings.data[function->label_bindings.size - 1].name);
  function->label_bindings.size--;
  function->labels.size--;
}

static WasmContinuation pop_break_continuation(WasmContinuation continuation) {
  if (continuation & WASM_CONTINUATION_BREAK_0) {
    continuation =
        (continuation & ~WASM_CONTINUATION_BREAK_0) | WASM_CONTINUATION_NORMAL;
  }
  /* Shift all of the break depths down by 1, keeping the other
   continuations. Since we cleared WASM_CONTINUATION_BREAK_0 above if it
   was set, we don't have to worry about it shifting into a
   WASM_CONTINUATION_RETURN */
  continuation = (continuation & ~CONTINUATION_BREAK_MASK) |
                 ((continuation & CONTINUATION_BREAK_MASK) >> 1);
  return continuation;
}

static void parse_generic(WasmParser* parser) {
  int nesting = 1;
  while (nesting > 0) {
    WasmToken t = read_token(parser);
    if (t.type == WASM_TOKEN_TYPE_OPEN_PAREN) {
      nesting++;
    } else if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN) {
      nesting--;
    } else if (t.type == WASM_TOKEN_TYPE_EOF) {
      FATAL_AT(parser, t.range.start, "unexpected eof\n");
    }
  }
}

static int parse_var(WasmParser* parser,
                     WasmBindingVector* bindings,
                     int num_vars,
                     const char* desc) {
  WasmToken t = read_token(parser);
  expect_atom(parser, t);

  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  if (end - p >= 1 && p[0] == '$') {
    /* var name */
    int i;
    for (i = 0; i < bindings->size; ++i) {
      WasmBinding* binding = &bindings->data[i];
      if (binding->name && string_eq(p, end, binding->name))
        return binding->index;
    }
    FATAL_AT(parser, t.range.start, "undefined %s variable \"%.*s\"\n", desc,
             (int)(end - p), p);
  } else {
    /* var index */
    uint32_t index;
    if (!read_int32(p, t.range.end.pos, &index, 0)) {
      FATAL_AT(parser, t.range.start, "invalid var index\n");
    }

    if (index >= num_vars) {
      FATAL_AT(parser, t.range.start, "%s variable out of range (max %d)\n",
               desc, num_vars);
    }

    return index;
  }
}

static int parse_function_var(WasmParser* parser, WasmModule* module) {
  return parse_var(parser, &module->function_bindings, module->functions.size,
                   "function");
}

static int parse_import_var(WasmParser* parser, WasmModule* module) {
  return parse_var(parser, &module->import_bindings, module->imports.size,
                   "import");
}

static int parse_global_var(WasmParser* parser, WasmModule* module) {
  return parse_var(parser, &module->global_bindings, module->globals.size,
                   "global");
}

static int parse_local_var(WasmParser* parser, WasmFunction* function) {
  return parse_var(parser, &function->local_bindings, function->locals.size,
                   "local");
}

/* Labels are indexed in reverse order (0 is the most recent, etc.), so handle
 * them separately */
static int parse_label_var(WasmParser* parser, WasmFunction* function) {
  WasmToken t = read_token(parser);
  expect_atom(parser, t);

  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  if (end - p >= 1 && p[0] == '$') {
    /* var name */
    int i;
    for (i = 0; i < function->label_bindings.size; ++i) {
      WasmBinding* binding = &function->label_bindings.data[i];
      if (binding->name && string_eq(p, end, binding->name))
        /* index is actually the current label depth */
        return binding->index;
    }
    FATAL_AT(parser, t.range.start, "undefined label variable \"%.*s\"\n",
             (int)(end - p), p);
  } else {
    /* var index */
    uint32_t index;
    if (!read_int32(p, t.range.end.pos, &index, 0)) {
      FATAL_AT(parser, t.range.start, "invalid var index\n");
    }

    if (index >= function->label_bindings.size) {
      FATAL_AT(parser, t.range.start, "label variable out of range (max %zd)\n",
               function->label_bindings.size);
    }

    /* Index in reverse, so 0 is the most recent label. The stored "index" is
     * actually the label depth, so return that instead */
    index = (function->label_bindings.size - 1) - index;
    return function->label_bindings.data[index].index;
  }
}

static void parse_type(WasmParser* parser, WasmType* type) {
  WasmToken t = read_token(parser);
  if (!match_type(t, type))
    FATAL_AT(parser, t.range.start, "expected type\n");
}

static WasmType parse_expr(WasmParser* parser,
                           WasmModule* module,
                           WasmFunction* function,
                           WasmContinuation* out_continuation);

static WasmType parse_block(WasmParser* parser,
                            WasmModule* module,
                            WasmFunction* function,
                            int* out_num_exprs,
                            WasmContinuation* out_continuation) {
  int num_exprs = 0;
  WasmContinuation continuation = WASM_CONTINUATION_NORMAL;
  WasmType type;
  while (1) {
    WasmContinuation this_continuation;
    type = parse_expr(parser, module, function, &this_continuation);
    if (continuation & WASM_CONTINUATION_NORMAL) {
      /* If the continuation is a power of two, then at this point we've hit a
       return or break that is forced. A previous return/break is still
       possible, but a normal continuation is not. */
      if (this_continuation != WASM_CONTINUATION_NORMAL &&
          is_power_of_two(this_continuation))
        continuation =
            (continuation & ~WASM_CONTINUATION_NORMAL) | this_continuation;
      else
        continuation |= this_continuation;
    } else {
      /* The current continuation does not include normal, so the current
       expression will never be executed. */
      /* TODO(binji): should this raise an error or warning? */
    }
    ++num_exprs;
    WasmToken t = read_token(parser);
    if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(parser, t);
  }
  *out_num_exprs = num_exprs;
  if (out_continuation)
    *out_continuation = continuation;
  return type;
}

static void parse_literal(WasmParser* parser,
                          WasmType type,
                          WasmNumber* number) {
  WasmToken t = read_token(parser);
  expect_atom(parser, t);
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  switch (type) {
    case WASM_TYPE_I32: {
      uint32_t value;
      if (!read_int32(p, end, &value, 1))
        FATAL_AT(parser, t.range.start, "invalid i32 literal \"%.*s\"\n",
                 (int)(end - p), p);
      number->i32 = value;
      break;
    }

    case WASM_TYPE_I64: {
      uint64_t value;
      if (!read_int64(p, end, &value))
        FATAL_AT(parser, t.range.start, "invalid i64 literal \"%.*s\"\n",
                 (int)(end - p), p);
      number->i64 = value;
      break;
    }

    case WASM_TYPE_F32: {
      float value;
      if (!read_float(p, end, &value))
        FATAL_AT(parser, t.range.start, "invalid f32 literal \"%.*s\"\n",
                 (int)(end - p), p);
      number->f32 = value;
      break;
    }

    case WASM_TYPE_F64: {
      double value;
      if (!read_double(p, end, &value))
        FATAL_AT(parser, t.range.start, "invalid f64 literal \"%.*s\"\n",
                 (int)(end - p), p);
      number->f64 = value;
      break;
    }

    default:
      assert(0);
  }
}

static WasmType parse_switch(WasmParser* parser,
                             WasmModule* module,
                             WasmFunction* function,
                             WasmType in_type) {
  WasmParserCookie switch_cookie =
      CALLBACK(parser, before_switch, (parser->user_data));
  WasmType cond_type = parse_expr(parser, module, function, NULL);
  check_type(parser, parser->tokenizer.loc, cond_type, in_type,
             " in switch condition");
  WasmType type = WASM_TYPE_ALL;
  WasmToken t = read_token(parser);
  while (1) {
    expect_open(parser, t);
    WasmToken open = t;
    t = read_token(parser);
    expect_atom(parser, t);
    if (match_atom(t, "case")) {
      WasmNumber number;
      int fallthrough = 0;
      int num_exprs = 0;
      parse_literal(parser, in_type, &number);
      WasmParserCookie cookie =
          CALLBACK(parser, before_switch_case, (number, parser->user_data));
      t = read_token(parser);
      if (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(parser, t);
        WasmType value_type;
        WasmContinuation case_continuation;
        while (1) {
          value_type = parse_expr(parser, module, function, &case_continuation);
          ++num_exprs;
          t = read_token(parser);
          if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN) {
            break;
          } else if (t.type == WASM_TOKEN_TYPE_ATOM &&
                     match_atom(t, "fallthrough")) {
            fallthrough = 1;
            expect_close(parser, read_token(parser));
            break;
          }
          rewind_token(parser, t);
        }

        if (case_continuation & WASM_CONTINUATION_NORMAL && fallthrough == 0)
          type &= value_type;
      } else {
        /* Empty case statement, implicit fallthrough */
        fallthrough = 1;
      }
      CALLBACK(parser, after_switch_case,
               (num_exprs, fallthrough, cookie, parser->user_data));
    } else {
      /* default case */
      WasmParserCookie cookie =
          CALLBACK(parser, before_switch_default, (parser->user_data));
      WasmContinuation case_continuation;
      rewind_token(parser, open);
      WasmType value_type =
          parse_expr(parser, module, function, &case_continuation);
      if (case_continuation & WASM_CONTINUATION_NORMAL)
        type &= value_type;
      expect_close(parser, read_token(parser));
      CALLBACK(parser, after_switch_default, (cookie, parser->user_data));
      break;
    }
    t = read_token(parser);
    if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
      break;
  }
  CALLBACK(parser, after_switch, (switch_cookie, parser->user_data));
  return type;
}

static WasmType parse_call_args(WasmParser* parser,
                                WasmModule* module,
                                WasmFunction* function,
                                WasmFunction* callee,
                                const char* desc) {
  WasmToken t;
  int num_args = 0;
  while (1) {
    t = read_token(parser);
    if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(parser, t);
    if (++num_args > callee->num_args) {
      FATAL_AT(parser, t.range.start,
               "too many arguments to function. got %d, expected %d\n",
               num_args, callee->num_args);
    }
    WasmType arg_type = parse_expr(parser, module, function, NULL);
    WasmType expected = callee->locals.data[num_args - 1].type;
    check_type_arg(parser, t.range.start, arg_type, expected, desc,
                   num_args - 1);
  }

  if (num_args < callee->num_args) {
    FATAL_AT(parser, t.range.start,
             "too few arguments to function. got %d, expected %d\n", num_args,
             callee->num_args);
  }

  return callee->result_type;
}

static WasmType parse_expr(WasmParser* parser,
                           WasmModule* module,
                           WasmFunction* function,
                           WasmContinuation* out_continuation) {
  WasmContinuation continuation = WASM_CONTINUATION_NORMAL;
  WasmType type = WASM_TYPE_VOID;
  expect_open(parser, read_token(parser));
  WasmToken t = read_token(parser);
  expect_atom(parser, t);

  OpInfo op_info_aligned;
  const OpInfo* op_info = get_op_info(t);
  if (!op_info) {
    if (!match_load_store_aligned(parser, t, &op_info_aligned))
      unexpected_token(parser, t);
    op_info = &op_info_aligned;
  }
  switch (op_info->op_type) {
    case WASM_OP_BINARY: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_binary, (op_info->opcode, parser->user_data));
      WasmType value0_type = parse_expr(parser, module, function, NULL);
      check_type_arg(parser, t.range.start, value0_type, op_info->type,
                     "binary op", 0);
      WasmType value1_type = parse_expr(parser, module, function, NULL);
      check_type_arg(parser, t.range.start, value1_type, op_info->type,
                     "binary op", 1);
      assert(value0_type == value1_type);
      type = value0_type;
      expect_close(parser, read_token(parser));
      break;
    }

    case WASM_OP_BLOCK: {
      t = read_token(parser);
      WasmLabel* label = NULL;
      if (t.type == WASM_TOKEN_TYPE_ATOM)
        label = push_label(parser, function, t);
      else
        rewind_token(parser, t);

      int with_label = label != NULL;
      WasmParserCookie cookie =
          CALLBACK(parser, before_block, (with_label, parser->user_data));

      int num_exprs;
      type = parse_block(parser, module, function, &num_exprs, &continuation);
      if (label) {
        type = check_label_type(parser, t.range.start, label, type,
                                continuation, " of block final expression");
        continuation = pop_break_continuation(continuation);
        pop_label(function);
      }
      CALLBACK(parser, after_block,
               (type, num_exprs, cookie, parser->user_data));
      break;
    }

    case WASM_OP_BREAK: {
      t = read_token(parser);
      int depth = 0;
      if (t.type == WASM_TOKEN_TYPE_ATOM) {
        rewind_token(parser, t);
        /* parse_label_var returns the depth counting up (so 0 is the topmost
         block of this function). We want the depth counting down (so 0 is the
         most recent block). */
        depth =
            function->label_bindings.size - parse_label_var(parser, function);
        t = read_token(parser);
      }
      if (depth == 0 && function->label_bindings.size == 0) {
        FATAL_AT(parser, t.range.start,
                 "label variable out of range (max 0)\n");
      } else if (depth > MAX_BREAK_DEPTH) {
        FATAL_AT(parser, t.range.start,
                 "break depth %d not supported (max %d)\n", depth,
                 MAX_BREAK_DEPTH);
      }
      int with_expr = t.type == WASM_TOKEN_TYPE_OPEN_PAREN;
      WasmParserCookie cookie =
          CALLBACK(parser, before_break, (with_expr, depth, parser->user_data));
      if (with_expr) {
        int label_index = (function->labels.size - 1) - depth;
        WasmLabel* break_label = &function->labels.data[label_index];
        rewind_token(parser, t);
        WasmType break_type = parse_expr(parser, module, function, NULL);
        if (parser->type_check == WASM_PARSER_TYPE_CHECK_SPEC) {
          break_label->type &= break_type;
        } else {
          assert(parser->type_check == WASM_PARSER_TYPE_CHECK_V8_NATIVE);
          if (break_label->type == WASM_TYPE_ALL) {
            break_label->type = break_type;
          } else {
            check_type(parser, t.range.start, break_type, break_label->type,
                       " of break expression");
          }
        }
        t = read_token(parser);
      }
      expect_close(parser, t);
      continuation = WASM_CONTINUATION_BREAK_0 << depth;
      CALLBACK(parser, after_break, (cookie, parser->user_data));
      break;
    }

    case WASM_OP_CALL: {
      int index = parse_function_var(parser, module);
      CALLBACK(parser, before_call, (index, parser->user_data));
      WasmFunction* callee = &module->functions.data[index];
      type = parse_call_args(parser, module, function, callee, "call");
      break;
    }

    case WASM_OP_CALL_IMPORT: {
      int index = parse_import_var(parser, module);
      CALLBACK(parser, before_call_import, (index, parser->user_data));
      WasmImport* callee = &module->imports.data[index];

      int num_args = 0;
      while (1) {
        WasmToken t = read_token(parser);
        if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
          break;
        rewind_token(parser, t);
        if (++num_args > callee->signature.args.size) {
          FATAL_AT(parser, t.range.start,
                   "too many arguments to function. got %d, expected %zd\n",
                   num_args, callee->signature.args.size);
        }
        WasmType arg_type = parse_expr(parser, module, function, NULL);
        WasmType expected = callee->signature.args.data[num_args - 1].type;
        check_type_arg(parser, t.range.start, arg_type, expected, "call_import",
                       num_args - 1);
      }

      if (num_args < callee->signature.args.size) {
        FATAL_AT(parser, t.range.start,
                 "too few arguments to function. got %d, expected %zd\n",
                 num_args, callee->signature.args.size);
      }

      type = callee->signature.result_type;
      break;
    }

    case WASM_OP_CALL_INDIRECT:
      /* TODO(binji) */
      break;

    case WASM_OP_COMPARE: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_compare, (op_info->opcode, parser->user_data));
      WasmType value0_type = parse_expr(parser, module, function, NULL);
      check_type_arg(parser, t.range.start, value0_type, op_info->type,
                     "compare op", 0);
      WasmType value1_type = parse_expr(parser, module, function, NULL);
      check_type_arg(parser, t.range.start, value1_type, op_info->type,
                     "compare op", 1);
      type = WASM_TYPE_I32;
      expect_close(parser, read_token(parser));
      break;
    }

    case WASM_OP_CONST: {
      check_opcode(parser, t.range.start, op_info->opcode);
      WasmNumber value;
      parse_literal(parser, op_info->type, &value);
      expect_close(parser, read_token(parser));
      CALLBACK(parser, after_const,
               (op_info->opcode, op_info->type, value, parser->user_data));
      type = op_info->type;
      break;
    }

    case WASM_OP_CONVERT: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_convert, (op_info->opcode, parser->user_data));
      WasmType value_type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, value_type, op_info->type2,
                 " of convert op");
      expect_close(parser, read_token(parser));
      type = op_info->type;
      break;
    }

    case WASM_OP_GET_LOCAL: {
      int index = parse_local_var(parser, function);
      WasmVariable* variable = &function->locals.data[index];
      type = variable->type;
      expect_close(parser, read_token(parser));
      CALLBACK(parser, after_get_local, (index, parser->user_data));
      break;
    }

    case WASM_OP_IF: {
      WasmParserCookie cookie =
          CALLBACK(parser, before_if, (parser->user_data));
      WasmType cond_type = parse_expr(parser, module, function, NULL);
      check_type(parser, parser->tokenizer.loc, cond_type, WASM_TYPE_I32,
                 " of condition");
      WasmContinuation true_continuation;
      WasmType true_type =
          parse_expr(parser, module, function, &true_continuation);
      t = read_token(parser);
      int with_else = 0;
      if (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
        with_else = 1;
        rewind_token(parser, t);
        WasmContinuation false_continuation;
        WasmType false_type =
            parse_expr(parser, module, function, &false_continuation);
        type = true_type & false_type;
        continuation = true_continuation | false_continuation;
        expect_close(parser, read_token(parser));
      } else {
        type = WASM_TYPE_VOID;
        continuation = true_continuation | WASM_CONTINUATION_NORMAL;
      }
      CALLBACK(parser, after_if, (type, with_else, cookie, parser->user_data));
      break;
    }

    case WASM_OP_LABEL: {
      WasmParserCookie cookie =
          CALLBACK(parser, before_label, (parser->user_data));

      t = read_token(parser);
      WasmLabel* label = NULL;
      if (t.type == WASM_TOKEN_TYPE_ATOM) {
        label = push_label(parser, function, t);
      } else if (t.type == WASM_TOKEN_TYPE_OPEN_PAREN) {
        label = push_anonymous_label(parser, function, t);
        rewind_token(parser, t);
      } else {
        unexpected_token(parser, t);
      }

      type = parse_expr(parser, module, function, &continuation);
      type = check_label_type(parser, t.range.start, label, type, continuation,
                              " of final label expression");
      continuation = pop_break_continuation(continuation);
      pop_label(function);
      expect_close(parser, read_token(parser));
      CALLBACK(parser, after_label, (type, cookie, parser->user_data));
      break;
    }

    case WASM_OP_LOAD: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_load,
               (op_info->opcode, op_info->access, parser->user_data));
      WasmType index_type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, index_type, WASM_TYPE_I32,
                 " of load index");
      expect_close(parser, read_token(parser));
      type = op_info->type;
      break;
    }

    case WASM_OP_LOAD_GLOBAL: {
      int index = parse_global_var(parser, module);
      type = module->globals.data[index].type;
      expect_close(parser, read_token(parser));
      CALLBACK(parser, after_load_global, (index, parser->user_data));
      break;
    }

    case WASM_OP_LOOP: {
      t = read_token(parser);
      WasmLabel* outer_label = NULL;
      WasmLabel* inner_label = NULL;
      if (t.type == WASM_TOKEN_TYPE_ATOM) {
        /* outer label (i.e. break continuation) */
        outer_label = push_label(parser, function, t);

        t = read_token(parser);
        if (t.type == WASM_TOKEN_TYPE_ATOM) {
          /* inner label (i.e. continue continuation) */
          inner_label = push_label(parser, function, t);
        } else {
          rewind_token(parser, t);
        }
      } else {
        rewind_token(parser, t);
      }

      int with_inner_label = inner_label != NULL;
      int with_outer_label = outer_label != NULL;
      WasmParserCookie cookie =
          CALLBACK(parser, before_loop,
                   (with_inner_label, with_outer_label, parser->user_data));

      int num_exprs;
      type = parse_block(parser, module, function, &num_exprs, &continuation);
      if (inner_label) {
        type = check_label_type(parser, t.range.start, inner_label, type,
                                continuation, " of inner loop block");
        continuation = pop_break_continuation(continuation);
        pop_label(function);
      }
      /* Loops are infinite, so there is never a "normal" continuation */
      continuation &= ~WASM_CONTINUATION_NORMAL;
      if (outer_label) {
        type = check_label_type(parser, t.range.start, outer_label, type,
                                continuation, " of outer loop block");
        continuation = pop_break_continuation(continuation);
        pop_label(function);
      }
      CALLBACK(parser, after_loop, (num_exprs, cookie, parser->user_data));
      break;
    }

    case WASM_OP_MEMORY_SIZE:
      CALLBACK(parser, after_memory_size, (parser->user_data));
      expect_close(parser, read_token(parser));
      type = WASM_TYPE_I32;
      break;

    case WASM_OP_NOP:
      CALLBACK(parser, after_nop, (parser->user_data));
      expect_close(parser, read_token(parser));
      break;

    case WASM_OP_PAGE_SIZE:
      CALLBACK(parser, after_page_size, (parser->user_data));
      expect_close(parser, read_token(parser));
      type = WASM_TYPE_I32;
      break;

    case WASM_OP_RESIZE_MEMORY: {
      CALLBACK(parser, before_resize_memory, (parser->user_data));
      type = parse_expr(parser, module, function, NULL);
      expect_close(parser, read_token(parser));
      break;
    }

    case WASM_OP_RETURN: {
      CALLBACK(parser, before_return, (parser->user_data));
      WasmType result_type = WASM_TYPE_VOID;
      t = read_token(parser);
      if (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(parser, t);
        result_type = parse_expr(parser, module, function, NULL);
        expect_close(parser, read_token(parser));
      }

      check_type(parser, t.range.start, result_type, function->result_type, "");
      type = WASM_TYPE_VOID;
      continuation = WASM_CONTINUATION_RETURN;
      CALLBACK(parser, after_return, (type, parser->user_data));
      break;
    }

    case WASM_OP_SET_LOCAL: {
      int index = parse_local_var(parser, function);
      WasmVariable* variable = &function->locals.data[index];
      CALLBACK(parser, before_set_local, (index, parser->user_data));
      type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, type, variable->type, "");
      expect_close(parser, read_token(parser));
      break;
    }

    case WASM_OP_STORE: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_store,
               (op_info->opcode, op_info->access, parser->user_data));
      WasmType index_type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, index_type, WASM_TYPE_I32,
                 " of store index");
      WasmType value_type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, value_type, op_info->type,
                 " of store value");
      expect_close(parser, read_token(parser));
      type = op_info->type;
      break;
    }

    case WASM_OP_STORE_GLOBAL: {
      int index = parse_global_var(parser, module);
      CALLBACK(parser, before_store_global, (index, parser->user_data));
      WasmVariable* variable = &module->globals.data[index];
      type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, type, variable->type, "");
      expect_close(parser, read_token(parser));
      break;
    }

    case WASM_OP_SWITCH:
      /* TODO(binji): we won't use the switch opcodes anyway */
#if 0
      check_opcode(parser, t.range.start, op_info->opcode);
#endif
      type = parse_switch(parser, module, function, op_info->type);
      break;

    case WASM_OP_UNARY: {
      check_opcode(parser, t.range.start, op_info->opcode);
      CALLBACK(parser, before_unary, (op_info->opcode, parser->user_data));
      WasmType value_type = parse_expr(parser, module, function, NULL);
      check_type(parser, t.range.start, value_type, op_info->type,
                 " of unary op");
      type = value_type;
      expect_close(parser, read_token(parser));
      break;
    }

    default:
      unexpected_token(parser, t);
      break;
  }
  if (out_continuation)
    *out_continuation = continuation;
  return type;
}

static int parse_func(WasmParser* parser,
                      WasmModule* module,
                      WasmFunction* function) {
  WasmToken t = read_token(parser);
  if (t.type == WASM_TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(parser);
  }

  int num_exprs = 0;
  WasmType type = WASM_TYPE_VOID;
  WasmContinuation continuation;
  while (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(parser, t);
    WasmToken open = t;
    t = read_token(parser);
    expect_atom(parser, t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_PARAM:
      case WASM_OP_RESULT:
      case WASM_OP_LOCAL:
        /* Skip, this has already been pre-parsed */
        parse_generic(parser);
        break;

      default:
        rewind_token(parser, open);
        type = parse_expr(parser, module, function, &continuation);
        ++num_exprs;
        break;
    }
    t = read_token(parser);
  }

  if (function->result_type != WASM_TYPE_VOID &&
      continuation != WASM_CONTINUATION_RETURN)
    check_type(parser, t.range.start, type, function->result_type,
               " in function result");

  return num_exprs;
}

static void parse_type_list(WasmParser* parser,
                            WasmVariableVector* variables,
                            int allow_empty) {
  WasmToken t = read_token(parser);
  WasmType type;
  if (allow_empty && t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
    return;
  if (!match_type(t, &type))
    FATAL_AT(parser, t.range.start, "expected a type, not \"%.*s\"\n",
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  while (1) {
    WasmVariable* variable = wasm_append_variable(variables);
    CHECK_ALLOC(parser, variable, t.range.start);
    variable->type = type;

    t = read_token(parser);
    if (t.type == WASM_TOKEN_TYPE_CLOSE_PAREN)
      break;
    else if (!match_type(t, &type))
      unexpected_token(parser, t);
  }
}

static void preparse_binding_name(WasmParser* parser,
                                  WasmBindingVector* bindings,
                                  size_t index,
                                  const char* desc,
                                  int required) {
  WasmToken t = read_token(parser);
  if (required || t.type == WASM_TOKEN_TYPE_ATOM) {
    expect_var_name(parser, t);
    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_binding_by_name(bindings, name) != -1) {
      free(name);
      FATAL_AT(parser, t.range.start, "redefinition of %s \"%.*s\"\n", desc,
               (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
    }

    WasmBinding* binding = wasm_append_binding(bindings);
    CHECK_ALLOC(parser, binding, t.range.start);
    binding->name = name;
    binding->index = index;
  } else {
    rewind_token(parser, t);
  }
}

static void preparse_binding_list(WasmParser* parser,
                                  WasmVariableVector* variables,
                                  WasmBindingVector* bindings,
                                  const char* desc) {
  WasmToken t = read_token(parser);
  rewind_token(parser, t);
  WasmType type;
  if (match_type(t, &type)) {
    parse_type_list(parser, variables, 0);
  } else {
    WasmVariable* variable = wasm_append_variable(variables);
    CHECK_ALLOC(parser, variable, t.range.start);
    preparse_binding_name(parser, bindings, variables->size - 1, desc, 1);
    parse_type(parser, &variable->type);
    expect_close(parser, read_token(parser));
  }
}

static void preparse_func(WasmParser* parser, WasmModule* module) {
  WasmFunction* function = wasm_append_function(&module->functions);
  CHECK_ALLOC(parser, function, parser->tokenizer.loc);
  memset(function, 0, sizeof(*function));

  preparse_binding_name(parser, &module->function_bindings,
                        module->functions.size - 1, "function", 0);
  WasmToken t = read_token(parser);

  while (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(parser, t);
    t = read_token(parser);
    expect_atom(parser, t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_PARAM:
        /* Don't allow adding params after locals */
        if (function->num_args != function->locals.size)
          FATAL_AT(parser, t.range.start,
                   "parameters must come before locals\n");
        preparse_binding_list(parser, &function->locals,
                              &function->local_bindings, "function argument");
        function->num_args = function->locals.size;
        break;

      case WASM_OP_RESULT: {
        t = read_token(parser);
        WasmType type;
        if (!match_type(t, &type))
          unexpected_token(parser, t);
        function->result_type = type;
        expect_close(parser, read_token(parser));
        break;
      }

      case WASM_OP_LOCAL:
        preparse_binding_list(parser, &function->locals,
                              &function->local_bindings, "local");
        break;

      default:
        rewind_token(parser, t);
        parse_generic(parser);
        break;
    }
    t = read_token(parser);
  }
}

static void preparse_signature(WasmParser* parser, WasmSignature* sig) {
  WasmToken t = read_token(parser);
  if (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(parser, t);
    t = read_token(parser);
    expect_atom(parser, t);
    const OpInfo* op_info = get_op_info(t);
    sig->result_type = WASM_TYPE_VOID;

    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_PARAM:
        parse_type_list(parser, &sig->args, 1);
        t = read_token(parser);
        if (t.type != WASM_TOKEN_TYPE_OPEN_PAREN) {
          expect_close(parser, t);
          break;
        }

        /* fallthrough to parsing result */
        t = read_token(parser);
        expect_atom_op(parser, t, WASM_OP_RESULT, "result");

      case WASM_OP_RESULT:
        parse_type(parser, &sig->result_type);
        expect_close(parser, read_token(parser)); /* close result */
        expect_close(parser, read_token(parser)); /* close sig */
        break;

      default:
        unexpected_token(parser, t);
        break;
    }
  }
}

static void preparse_module(WasmParser* parser, WasmModule* module) {
  int seen_memory = 0;
  WasmToken t = read_token(parser);
  WasmToken first = t;
  while (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(parser, t);
    t = read_token(parser);
    expect_atom(parser, t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_FUNC:
        preparse_func(parser, module);
        break;

      case WASM_OP_GLOBAL:
        preparse_binding_list(parser, &module->globals,
                              &module->global_bindings, "global");
        break;

      case WASM_OP_MEMORY: {
        if (seen_memory)
          FATAL_AT(parser, t.range.start, "only one memory block allowed\n");
        seen_memory = 1;
        t = read_token(parser);
        expect_atom(parser, t);
        if (!read_uint32_token(t, &module->initial_memory_size))
          FATAL_AT(parser, t.range.start, "invalid initial memory size\n");

        t = read_token(parser);
        if (t.type == WASM_TOKEN_TYPE_ATOM) {
          if (!read_uint32_token(t, &module->max_memory_size))
            FATAL_AT(parser, t.range.start, "invalid max memory size\n");

          if (module->max_memory_size < module->initial_memory_size) {
            FATAL_AT(parser, t.range.start,
                     "max size (%u) must be greater than or equal to initial "
                     "size (%u)\n",
                     module->max_memory_size, module->initial_memory_size);
          }

          t = read_token(parser);
        } else {
          module->max_memory_size = module->initial_memory_size;
        }

        uint32_t last_segment_end = 0;
        while (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
          expect_open(parser, t);
          t = read_token(parser);
          if (!match_atom(t, "segment"))
            unexpected_token(parser, t);
          t = read_token(parser);
          expect_atom(parser, t);

          uint32_t address;
          if (!read_uint32_token(t, &address))
            FATAL_AT(parser, t.range.start, "invalid memory segment address\n");

          if (address < last_segment_end) {
            FATAL_AT(parser, t.range.start,
                     "address (%u) less than end of previous segment (%u)\n",
                     address, last_segment_end);
          }

          if (address > module->initial_memory_size) {
            FATAL_AT(parser, t.range.start,
                     "address (%u) greater than initial memory size (%u)\n",
                     address, module->initial_memory_size);
          }

          t = read_token(parser);
          expect_string(parser, t);
          /* Allocate memory for the segment data to hide the underlying token
           object from the caller */
          WasmSegmentData data = malloc(sizeof(WasmToken));
          *(WasmToken*)data = t;

          WasmSegment* segment = wasm_append_segment(&module->segments);
          CHECK_ALLOC(parser, segment, t.range.start);
          segment->address = address;
          segment->data = data;
          segment->size = string_contents_length(t);

          last_segment_end = segment->address + segment->size;

          if (last_segment_end > module->initial_memory_size) {
            FATAL_AT(
                parser, t.range.start,
                "segment ends past the end of the initial memory size (%u)\n",
                module->initial_memory_size);
          }

          expect_close(parser, read_token(parser));
          t = read_token(parser);
        }
        break;
      }

      case WASM_OP_IMPORT: {
        WasmImport* import = wasm_append_import(&module->imports);
        CHECK_ALLOC(parser, import, t.range.start);
        memset(import, 0, sizeof(*import));

        preparse_binding_name(parser, &module->import_bindings,
                              module->imports.size - 1, "import", 0);
        WasmToken t = read_token(parser);

        expect_string(parser, t);
        import->module_name = dup_string_contents(t);
        t = read_token(parser);
        expect_string(parser, t);
        import->func_name = dup_string_contents(t);
        preparse_signature(parser, &import->signature);
        break;
      }

      case WASM_OP_TYPE: {
        WasmSignature* sig = wasm_append_signature(&module->signatures);
        CHECK_ALLOC(parser, sig, t.range.start);
        memset(sig, 0, sizeof(*sig));

        preparse_binding_name(parser, &module->signature_bindings,
                              module->signatures.size - 1, "signature", 0);
        WasmToken t = read_token(parser);
        expect_open(parser, t);
        t = read_token(parser);
        expect_atom_op(parser, t, WASM_OP_FUNC, "func");
        preparse_signature(parser, sig);
        expect_close(parser, read_token(parser));
        break;
      }

      default:
        parse_generic(parser);
        break;
    }
    t = read_token(parser);
  }
  rewind_token(parser, first);
}

static void wasm_destroy_binding_list(WasmBindingVector* bindings) {
  int i;
  for (i = 0; i < bindings->size; ++i)
    free(bindings->data[i].name);
  wasm_destroy_binding_vector(bindings);
}

static void wasm_destroy_signature(WasmSignature* sig) {
  wasm_destroy_variable_vector(&sig->args);
}

static void wasm_destroy_module(WasmModule* module) {
  int i;
  for (i = 0; i < module->functions.size; ++i) {
    WasmFunction* function = &module->functions.data[i];
    wasm_destroy_variable_vector(&function->locals);
    wasm_destroy_binding_list(&function->local_bindings);
    wasm_destroy_label_vector(&function->labels);
    wasm_destroy_binding_list(&function->label_bindings);
    free(function->exported_name.name);
    WasmExportedNameList* node = function->exported_name.next;
    while (node) {
      WasmExportedNameList* next_node = node->next;
      free(node->name);
      free(node);
      node = next_node;
    }
  }
  wasm_destroy_binding_list(&module->function_bindings);
  wasm_destroy_binding_list(&module->global_bindings);
  wasm_destroy_function_vector(&module->functions);
  wasm_destroy_variable_vector(&module->globals);
  wasm_destroy_binding_list(&module->import_bindings);
  for (i = 0; i < module->imports.size; ++i) {
    WasmImport* import = &module->imports.data[i];
    free(import->module_name);
    free(import->func_name);
    wasm_destroy_signature(&import->signature);
  }
  wasm_destroy_import_vector(&module->imports);
  for (i = 0; i < module->segments.size; ++i)
    free(module->segments.data[i].data);
  wasm_destroy_segment_vector(&module->segments);
  for (i = 0; i < module->signatures.size; ++i)
    wasm_destroy_signature(&module->signatures.data[i]);
  wasm_destroy_signature_vector(&module->signatures);
}

static void parse_module(WasmParser* parser, WasmModule* module) {
  WasmToken t = read_token(parser);
  expect_open(parser, t);
  t = read_token(parser);
  expect_atom_op(parser, t, WASM_OP_MODULE, "module");

  preparse_module(parser, module);
  CALLBACK(parser, before_module, (module, parser->user_data));

  int function_index = 0;
  t = read_token(parser);
  while (t.type != WASM_TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(parser, t);
    t = read_token(parser);
    expect_atom(parser, t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_FUNC: {
        WasmFunction* function = &module->functions.data[function_index++];
        CALLBACK(parser, before_function,
                 (module, function, parser->user_data));
        int num_exprs = parse_func(parser, module, function);
        CALLBACK(parser, after_function,
                 (module, function, num_exprs, parser->user_data));
        break;
      }

      case WASM_OP_EXPORT: {
        CALLBACK(parser, before_export, (module, parser->user_data));
        t = read_token(parser);
        expect_string(parser, t);
        int index = parse_function_var(parser, module);
        char* export_name = dup_string_contents(t);
        if (get_function_by_export_name(&module->functions, export_name) !=
            -1) {
          free(export_name);
          FATAL_AT(parser, t.range.start, "duplicate function export %.*s\n",
                   (int)(t.range.end.pos - t.range.start.pos),
                   t.range.start.pos);
        }
        WasmFunction* function = &module->functions.data[index];
        if (function->exported) {
          /* Exporting with another name */
          WasmExportedNameList* new_node = malloc(sizeof(WasmExportedNameList));
          new_node->name = export_name;
          new_node->next = function->exported_name.next;
          function->exported_name.next = new_node;
        } else {
          function->exported = 1;
          function->exported_name.name = export_name;
          function->exported_name.next = NULL;
        }

        expect_close(parser, read_token(parser));
        CALLBACK(parser, after_export,
                 (module, function, export_name, parser->user_data));
        break;
      }

      case WASM_OP_TABLE:
        parse_generic(parser);
        break;

      case WASM_OP_GLOBAL:
      case WASM_OP_MEMORY:
      case WASM_OP_IMPORT:
      case WASM_OP_TYPE:
        /* Skip, this has already been pre-parsed */
        parse_generic(parser);
        break;

      default:
        unexpected_token(parser, t);
        break;
    }
    t = read_token(parser);
  }

  CALLBACK(parser, after_module, (module, parser->user_data));
}

static void init_parser(WasmParser* parser,
                        WasmParserCallbacks* callbacks,
                        WasmSource* source,
                        WasmParserTypeCheck type_check) {
  parser->callbacks = callbacks;
  parser->user_data = callbacks->user_data;
  parser->tokenizer.source = *source;
  parser->tokenizer.loc.source = &parser->tokenizer.source;
  parser->tokenizer.loc.pos = parser->tokenizer.source.start;
  parser->tokenizer.loc.line = 1;
  parser->tokenizer.loc.col = 1;
  parser->type_check = type_check;
}

static void assert_invalid_error(WasmSourceLocation loc,
                                 const char* msg,
                                 void* user_data) {
  WasmParser* parser = user_data;
  CALLBACK(parser, assert_invalid_error, (loc, msg, parser->user_data));
}

int wasm_parse_module(WasmSource* source,
                      WasmParserCallbacks* callbacks,
                      WasmParserTypeCheck type_check) {
  WasmModule module = {};
  WasmParser parser = {};
  init_parser(&parser, callbacks, source, type_check);

  if (setjmp(parser.jump_buf)) {
    wasm_destroy_module(&module);
    /* error */
    return 1;
  }

  parse_module(&parser, &module);
  WasmToken t = read_token(&parser);
  if (t.type != WASM_TOKEN_TYPE_EOF)
    FATAL_AT(&parser, t.range.start, "expected EOF\n");
  wasm_destroy_module(&module);
  return 0;
}

static WasmType parse_invoke(WasmParser* parser, WasmModule* module) {
  WasmToken t = read_token(parser);
  expect_string(parser, t);
  char* name = dup_string_contents(t);
  /* Find the function with this exported name */
  int function_index = get_function_by_export_name(&module->functions, name);
  if (function_index < 0) {
    free(name);
    FATAL_AT(parser, t.range.start, "unknown function export %.*s\n",
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  }

  WasmParserCookie cookie =
      CALLBACK(parser, before_invoke,
               (t.range.start, name, function_index, parser->user_data));
  free(name);

  WasmFunction dummy_function = {};
  WasmFunction* invokee = &module->functions.data[function_index];
  WasmType result_type =
      parse_call_args(parser, module, &dummy_function, invokee, "invoke");
  CALLBACK(parser, after_invoke, (cookie, parser->user_data));
  return result_type;
}

int wasm_parse_file(WasmSource* source,
                    WasmParserCallbacks* callbacks,
                    WasmParserTypeCheck type_check) {
  WasmModule module = {};
  WasmParser parser_object = {};
  WasmParser* parser = &parser_object;
  init_parser(parser, callbacks, source, type_check);

  if (setjmp(parser->jump_buf)) {
    /* error */
    wasm_destroy_module(&module);
    return 1;
  }

  WasmToken t = read_token(parser);
  int seen_module = 0;
  do {
    expect_open(parser, t);
    WasmToken open = t;
    t = read_token(parser);
    expect_atom(parser, t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : WASM_OP_NONE) {
      case WASM_OP_MODULE: {
        seen_module = 1;
        rewind_token(parser, open);
        /* Destroy previous module, if any */
        wasm_destroy_module(&module);
        memset(&module, 0, sizeof(module));
        parse_module(parser, &module);
        break;
      }

      case WASM_OP_ASSERT_RETURN: {
        if (!seen_module)
          FATAL_AT(parser, t.range.start,
                   "assert_return must occur after a module definition\n");

        expect_open(parser, read_token(parser));
        expect_atom_op(parser, read_token(parser), WASM_OP_INVOKE, "invoke");

        WasmParserCookie cookie = CALLBACK(parser, before_assert_return,
                                           (t.range.start, parser->user_data));

        WasmFunction dummy_function = {};
        WasmType left_type = parse_invoke(parser, &module);
        if (left_type != WASM_TYPE_VOID) {
          WasmType right_type =
              parse_expr(parser, &module, &dummy_function, NULL);
          check_type(parser, parser->tokenizer.loc, right_type, left_type, "");
        }

        CALLBACK(parser, after_assert_return,
                 (left_type, cookie, parser->user_data));
        expect_close(parser, read_token(parser));
        break;
      }

      case WASM_OP_ASSERT_RETURN_NAN: {
        if (!seen_module)
          FATAL_AT(parser, t.range.start,
                   "assert_return_nan must occur after a module definition\n");

        expect_open(parser, read_token(parser));
        expect_atom_op(parser, read_token(parser), WASM_OP_INVOKE, "invoke");

        WasmParserCookie cookie = CALLBACK(parser, before_assert_return_nan,
                                           (t.range.start, parser->user_data));

        WasmType type = parse_invoke(parser, &module);
        if (type != WASM_TYPE_F32 && type != WASM_TYPE_F64) {
          FATAL_AT(parser, parser->tokenizer.loc,
                   "type mismatch. got %s, expected f32 or f64\n",
                   s_type_names[type]);
        }

        CALLBACK(parser, after_assert_return_nan,
                 (type, cookie, parser->user_data));
        expect_close(parser, read_token(parser));
        break;
      }

      case WASM_OP_ASSERT_TRAP: {
        if (!seen_module)
          FATAL_AT(parser, t.range.start,
                   "assert_trap must occur after a module definition\n");

        expect_open(parser, read_token(parser));
        expect_atom_op(parser, read_token(parser), WASM_OP_INVOKE, "invoke");

        CALLBACK(parser, before_assert_trap,
                 (t.range.start, parser->user_data));
        parse_invoke(parser, &module);
        CALLBACK(parser, after_assert_trap, (parser->user_data));
        /* ignore string, these are error messages that will only match the
           spec repo */
        expect_string(parser, read_token(parser));
        expect_close(parser, read_token(parser));
        break;
      }

      case WASM_OP_ASSERT_INVALID: {
        t = read_token(parser);
        WasmToken open = t;
        expect_open(parser, t);
        expect_atom_op(parser, read_token(parser), WASM_OP_MODULE, "module");

        {
          WasmParserCallbacks callbacks = {};
          callbacks.error = assert_invalid_error;
          callbacks.user_data = parser;

          WasmParser parser2 = {};
          parser2.callbacks = &callbacks;
          parser2.user_data = callbacks.user_data;
          parser2.tokenizer = parser->tokenizer;

          WasmModule module2 = {};
          int result = 0;
          if (setjmp(parser2.jump_buf)) {
            /* error occurred */
            result = 1;
          } else {
            rewind_token(&parser2, open);
            parse_module(&parser2, &module2);
          }
          wasm_destroy_module(&module2);

          if (result == 0)
            FATAL_AT(parser, parser->tokenizer.loc,
                     "expected module to be invalid\n");
        }

        /* skip over the module, regardless of how much was parsed by parser2 */
        parse_generic(parser);

        /* ignore string, these are error messages that will only match the
           spec repo */
        expect_string(parser, read_token(parser));
        expect_close(parser, read_token(parser));
        break;
      }

      case WASM_OP_INVOKE:
        if (!seen_module)
          FATAL_AT(parser, t.range.start,
                   "invoke must occur after a module definition\n");
        parse_invoke(parser, &module);
        break;

      default:
        unexpected_token(parser, t);
        break;
    }
    t = read_token(parser);
  } while (t.type != WASM_TOKEN_TYPE_EOF);

  wasm_destroy_module(&module);
  return 0;
}
