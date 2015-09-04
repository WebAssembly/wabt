#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABS_TO_SPACES 8
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

typedef enum Type {
  TYPE_VOID,
  TYPE_I32,
  TYPE_I64,
  TYPE_F32,
  TYPE_F64,
  NUM_TYPES,
} Type;

typedef enum MemType {
  MEM_TYPE_I8,
  MEM_TYPE_I16,
  MEM_TYPE_I32,
  MEM_TYPE_I64,
  MEM_TYPE_F32,
  MEM_TYPE_F64,
} MemType;

typedef enum TokenType {
  TOKEN_TYPE_EOF,
  TOKEN_TYPE_OPEN_PAREN,
  TOKEN_TYPE_CLOSE_PAREN,
  TOKEN_TYPE_ATOM,
  TOKEN_TYPE_STRING,
} TokenType;

typedef struct SourceLocation {
  const char* pos;
  int line;
  int col;
} SourceLocation;

typedef struct SourceRange {
  SourceLocation start;
  SourceLocation end;
} SourceRange;

typedef struct Token {
  TokenType type;
  SourceRange range;
} Token;

typedef struct Source {
  const char* start;
  const char* end;
} Source;

typedef struct Tokenizer {
  Source source;
  SourceLocation loc;
} Tokenizer;

typedef struct Binding {
  char* name;
  union {
    Type type;
    struct {
      Type* result_types;
      struct Binding* locals; /* Includes args, they're at the start */
      struct Binding* labels; /* TODO(binji): hack, shouldn't be Binding */
      int num_results;
      int num_args;
      int num_locals; /* Total size of the locals array above, including args */
      int num_labels;
    };
  };
} Binding;

typedef Binding Function;

typedef struct Export {
  char* name;
  int index;
} Export;

typedef struct Module {
  Function* functions;
  Binding* globals;
  Export* exports;
  int num_functions;
  int num_globals;
  int num_exports;
} Module;

typedef struct NameTypePair {
  const char* name;
  Type type;
} NameTypePair;

typedef struct NameType2Pair {
  const char* name;
  Type in_type;
  Type out_type;
} NameType2Pair;

typedef struct NameMemTypePair {
  const char* name;
  MemType type;
} NameMemTypePair;

static NameTypePair s_unary_ops[] = {
    {"neg.i32", TYPE_I32},   {"neg.i64", TYPE_I64},   {"neg.f32", TYPE_F32},
    {"neg.f64", TYPE_F64},   {"abs.i32", TYPE_I32},   {"abs.i64", TYPE_I64},
    {"abs.f32", TYPE_F32},   {"abs.f64", TYPE_F64},   {"not.i32", TYPE_I32},
    {"not.i64", TYPE_I64},   {"not.f32", TYPE_F32},   {"not.f64", TYPE_F64},
    {"clz.i32", TYPE_I32},   {"clz.i64", TYPE_I64},   {"ctz.i32", TYPE_I32},
    {"ctz.i64", TYPE_I64},   {"ceil.f32", TYPE_F32},  {"ceil.f64", TYPE_F64},
    {"floor.f32", TYPE_F32}, {"floor.f64", TYPE_F64}, {"trunc.f32", TYPE_F32},
    {"trunc.f64", TYPE_F64}, {"round.f32", TYPE_F32}, {"round.f64", TYPE_F64},
};

static NameTypePair s_binary_ops[] = {
    {"add.i32", TYPE_I32},      {"add.i64", TYPE_I64},
    {"add.f32", TYPE_F32},      {"add.f64", TYPE_F64},
    {"sub.i32", TYPE_I32},      {"sub.i64", TYPE_I64},
    {"sub.f32", TYPE_F32},      {"sub.f64", TYPE_F64},
    {"mul.i32", TYPE_I32},      {"mul.i64", TYPE_I64},
    {"mul.f32", TYPE_F32},      {"mul.f64", TYPE_F64},
    {"divs.i32", TYPE_I32},     {"divs.i64", TYPE_I64},
    {"divu.i32", TYPE_I32},     {"divu.i64", TYPE_I64},
    {"div.f32", TYPE_F32},      {"div.f64", TYPE_F64},
    {"mods.i32", TYPE_I32},     {"mods.i64", TYPE_I64},
    {"modu.i32", TYPE_I32},     {"modu.i64", TYPE_I64},
    {"and.i32", TYPE_I32},      {"and.i64", TYPE_I64},
    {"or.i32", TYPE_I32},       {"or.i64", TYPE_I64},
    {"xor.i32", TYPE_I32},      {"xor.i64", TYPE_I64},
    {"shl.i32", TYPE_I32},      {"shl.i64", TYPE_I64},
    {"shr.i32", TYPE_I32},      {"shr.i64", TYPE_I64},
    {"sar.i32", TYPE_I32},      {"sar.i64", TYPE_I64},
    {"copysign.f32", TYPE_F32}, {"copysign.f64", TYPE_F64},
};

static NameTypePair s_compare_ops[] = {
    {"eq.i32", TYPE_I32},  {"eq.i64", TYPE_I64},  {"eq.f32", TYPE_F32},
    {"eq.f64", TYPE_F64},  {"neq.i32", TYPE_I32}, {"neq.i64", TYPE_I64},
    {"neq.f32", TYPE_F32}, {"neq.f64", TYPE_F64}, {"lts.i32", TYPE_I32},
    {"lts.i64", TYPE_I64}, {"ltu.i32", TYPE_I32}, {"ltu.i64", TYPE_I64},
    {"lt.f32", TYPE_F32},  {"lt.f64", TYPE_F64},  {"les.i32", TYPE_I32},
    {"les.i64", TYPE_I64}, {"leu.i32", TYPE_I32}, {"leu.i64", TYPE_I64},
    {"le.f32", TYPE_F32},  {"le.f64", TYPE_F64},  {"gts.i32", TYPE_I32},
    {"gts.i64", TYPE_I64}, {"gtu.i32", TYPE_I32}, {"gtu.i64", TYPE_I64},
    {"gt.f32", TYPE_F32},  {"gt.f64", TYPE_F64},  {"ges.i32", TYPE_I32},
    {"ges.i64", TYPE_I64}, {"geu.i32", TYPE_I32}, {"geu.i64", TYPE_I64},
    {"ge.f32", TYPE_F32},  {"ge.f64", TYPE_F64},
};

static NameType2Pair s_convert_ops[] = {
    {"converts.i32.i32", TYPE_I32, TYPE_I32},
    {"convertu.i32.i32", TYPE_I32, TYPE_I32},
    {"converts.i32.i64", TYPE_I32, TYPE_I64},
    {"convertu.i32.i64", TYPE_I32, TYPE_I64},
    {"converts.i64.i32", TYPE_I64, TYPE_I32},
    {"convertu.i64.i32", TYPE_I64, TYPE_I32},
    {"converts.i64.i64", TYPE_I64, TYPE_I64},
    {"convertu.i64.i64", TYPE_I64, TYPE_I64},
    {"converts.i32.f32", TYPE_I32, TYPE_F32},
    {"convertu.i32.f32", TYPE_I32, TYPE_F32},
    {"converts.i32.f64", TYPE_I32, TYPE_F64},
    {"convertu.i32.f64", TYPE_I32, TYPE_F64},
    {"converts.i64.f32", TYPE_I64, TYPE_F32},
    {"convertu.i64.f32", TYPE_I64, TYPE_F32},
    {"converts.i64.f64", TYPE_I64, TYPE_F64},
    {"convertu.i64.f64", TYPE_I64, TYPE_F64},
    {"converts.f32.i32", TYPE_F32, TYPE_I32},
    {"convertu.f32.i32", TYPE_F32, TYPE_I32},
    {"converts.f32.i64", TYPE_F32, TYPE_I64},
    {"convertu.f32.i64", TYPE_F32, TYPE_I64},
    {"converts.f64.i32", TYPE_F64, TYPE_I32},
    {"convertu.f64.i32", TYPE_F64, TYPE_I32},
    {"converts.f64.i64", TYPE_F64, TYPE_I64},
    {"convertu.f64.i64", TYPE_F64, TYPE_I64},
    {"convert.f32.f32", TYPE_F32, TYPE_F32},
    {"convert.f32.f64", TYPE_F32, TYPE_F64},
    {"convert.f64.f32", TYPE_F64, TYPE_F32},
    {"convert.f64.f64", TYPE_F64, TYPE_F64},
};

static NameType2Pair s_cast_ops[] = {
    {"cast.i32.f32", TYPE_I32, TYPE_F32},
    {"cast.f32.i32", TYPE_F32, TYPE_I32},
    {"cast.i64.f64", TYPE_I64, TYPE_F64},
    {"cast.f64.i64", TYPE_F64, TYPE_I64},
};

static NameTypePair s_const_ops[] = {
    {"const.i32", TYPE_I32},
    {"const.i64", TYPE_I64},
    {"const.f32", TYPE_F32},
    {"const.f64", TYPE_F64},
};

static NameTypePair s_switch_ops[] = {
    {"switch.i32", TYPE_I32},
    {"switch.i64", TYPE_I64},
    {"switch.f32", TYPE_F32},
    {"switch.f64", TYPE_F64},
};

static NameTypePair s_types[] = {
    {"i32", TYPE_I32},
    {"i64", TYPE_I64},
    {"f32", TYPE_F32},
    {"f64", TYPE_F64},
};

static NameMemTypePair s_mem_int_types[] = {
    {"i8", MEM_TYPE_I8},
    {"i16", MEM_TYPE_I16},
    {"i32", MEM_TYPE_I32},
    {"i64", MEM_TYPE_I64},
};

static NameMemTypePair s_mem_float_types[] = {
    {"f32", MEM_TYPE_F32},
    {"f64", MEM_TYPE_F64},
};

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64",
};
STATIC_ASSERT(ARRAY_SIZE(s_type_names) == NUM_TYPES);

static Type mem_type_to_type(MemType type) {
  switch (type) {
    case MEM_TYPE_I8:
      return TYPE_I32;

    case MEM_TYPE_I16:
      return TYPE_I32;

    case MEM_TYPE_I32:
      return TYPE_I32;

    case MEM_TYPE_I64:
      return TYPE_I64;

    case MEM_TYPE_F32:
      return TYPE_F32;

    case MEM_TYPE_F64:
      return TYPE_F64;

    default:
      assert(0);
      return TYPE_VOID;
  }
}

static void realloc_list(void** elts, int* num_elts, int elt_size) {
  (*num_elts)++;
  int new_size = *num_elts * elt_size;
  *elts = realloc(*elts, new_size);
  if (*elts == NULL) {
    FATAL("unable to alloc %d bytes", new_size);
  }
}

static int get_binding_by_name(Binding* bindings,
                               int num_bindings,
                               const char* name) {
  int i;
  for (i = 0; i < num_bindings; ++i) {
    Binding* binding = &bindings[i];
    if (binding->name && strcmp(name, binding->name) == 0)
      return i;
  }
  return -1;
}

static int get_function_by_name(Module* module, const char* name) {
  return get_binding_by_name(module->functions, module->num_functions, name);
}

static int read_uint32(const char** s, const char* end, uint32_t* out) {
  errno = 0;
  char* endptr;
  uint64_t value = strtoul(*s, &endptr, 10);
  if (endptr != end || errno != 0 || value >= (uint64_t)UINT32_MAX + 1)
    return 0;
  *out = value;
  *s = endptr;
  return 1;
}

static int read_uint64(const char** s, const char* end, uint64_t* out) {
  errno = 0;
  char* endptr;
  *out = strtoull(*s, &endptr, 10);
  if (endptr != end || errno != 0)
    return 0;
  *s = endptr;
  return 1;
}

static int read_double(const char** s, const char* end, double* out) {
  errno = 0;
  char* endptr;
  *out = strtod(*s, &endptr);
  if (endptr != end || errno != 0)
    return 0;
  *s = endptr;
  return 1;
}

static Token read_token(Tokenizer* t) {
  Token result;

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
        result.type = TOKEN_TYPE_STRING;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;

        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case '\\':
              if (t->loc.pos + 1 < t->source.end) {
                t->loc.col++;
                t->loc.pos++;
              }
              break;

            case '\n':
              FATAL("newline in string\n");
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
          result.type = TOKEN_TYPE_OPEN_PAREN;
          result.range.start = t->loc;
          t->loc.col++;
          t->loc.pos++;
          result.range.end = t->loc;
          return result;
        }
        break;

      case ')':
        result.type = TOKEN_TYPE_CLOSE_PAREN;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        result.range.end = t->loc;
        return result;

      default:
        result.type = TOKEN_TYPE_ATOM;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case ' ':
            case '\t':
            case '\n':
            case '(':
            case ')':
              result.range.end = t->loc;
              return result;

            default:
              t->loc.col++;
              t->loc.pos++;
              break;
          }
        }
        break;
    }
  }

  result.type = TOKEN_TYPE_EOF;
  result.range.start = t->loc;
  result.range.end = t->loc;
  return result;
}

static void rewind_token(Tokenizer* tokenizer, Token t) {
  tokenizer->loc = t.range.start;
}

#if 0
static void print_tokens(Tokenizer* tokenizer) {
  while (1) {
    Token token = read_token(&tokenizer);
    fprintf(stderr, "[%d:%d]:[%d:%d]: ", token.range.start.line,
            token.range.start.col, token.range.end.line, token.range.end.col);
    switch (token.type) {
      case TOKEN_TYPE_EOF:
        fprintf(stderr, "EOF\n");
        return;

      case TOKEN_TYPE_OPEN_PAREN:
        fprintf(stderr, "OPEN_PAREN\n");
        break;

      case TOKEN_TYPE_CLOSE_PAREN:
        fprintf(stderr, "CLOSE_PAREN\n");
        break;

      case TOKEN_TYPE_ATOM:
        fprintf(stderr, "ATOM: %.*s\n",
                (int)(token.range.end.pos - token.range.start.pos),
                token.range.start.pos);
        break;

      case TOKEN_TYPE_STRING:
        fprintf(stderr, "STRING: %.*s\n",
                (int)(token.range.end.pos - token.range.start.pos),
                token.range.start.pos);
        break;
    }
  }
}
#endif

static void expect_open(Token t) {
  if (t.type != TOKEN_TYPE_OPEN_PAREN) {
    FATAL("%d:%d: expected '(', not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_close(Token t) {
  if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    FATAL("%d:%d: expected ')', not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_atom(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    FATAL("%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_string(Token t) {
  if (t.type != TOKEN_TYPE_STRING) {
    FATAL("%d:%d: expected STRING, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_var_name(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    FATAL("%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }

  if (t.range.end.pos - t.range.start.pos < 1 ||
      t.range.start.pos[0] != '$') {
    FATAL("%d:%d: expected name to begin with $\n", t.range.start.line,
          t.range.start.col);
  }
}

static int match_atom(Token t, const char* s) {
  return strncmp(t.range.start.pos, s, t.range.end.pos - t.range.start.pos) ==
         0;
}

static int match_unary(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_unary_ops); ++i) {
    if (match_atom(t, s_unary_ops[i].name)) {
      *type = s_unary_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_binary(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_binary_ops); ++i) {
    if (match_atom(t, s_binary_ops[i].name)) {
      *type = s_binary_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_compare(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_compare_ops); ++i) {
    if (match_atom(t, s_compare_ops[i].name)) {
      *type = s_compare_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_convert(Token t, Type* in_type, Type* out_type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_convert_ops); ++i) {
    if (match_atom(t, s_convert_ops[i].name)) {
      *in_type = s_convert_ops[i].in_type;
      *out_type = s_convert_ops[i].out_type;
      return 1;
    }
  }
  return 0;
}

static int match_cast(Token t, Type* in_type, Type* out_type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_cast_ops); ++i) {
    if (match_atom(t, s_cast_ops[i].name)) {
      *in_type = s_cast_ops[i].in_type;
      *out_type = s_cast_ops[i].out_type;
      return 1;
    }
  }
  return 0;
}

static int match_const(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_const_ops); ++i) {
    if (match_atom(t, s_const_ops[i].name)) {
      *type = s_const_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_switch(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_switch_ops); ++i) {
    if (match_atom(t, s_switch_ops[i].name)) {
      *type = s_switch_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_type(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_types); ++i) {
    if (match_atom(t, s_types[i].name)) {
      if (type) {
        *type = s_types[i].type;
      }
      return 1;
    }
  }
  return 0;
}

static int match_load_store(Token t, const char* prefix, MemType* type) {
  size_t plen = strlen(prefix);
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  size_t len = end - p;
  NameMemTypePair* types = NULL;
  int num_types = 0;

  if (len < plen + 1)
    return 0;
  if (strncmp(p, prefix, plen) != 0)
    return 0;
  p += plen;
  switch (*p) {
    case 's':
    case 'u':
      types = s_mem_int_types;
      num_types = ARRAY_SIZE(s_mem_int_types);
      p++;
      if (p >= end || *p != '.')
        return 0;
      p++;
      break;

    case '.':
      types = s_mem_float_types;
      num_types = ARRAY_SIZE(s_mem_float_types);
      p++;
      break;

    default:
      return 0;
  }

  /* try to read alignment */
  if (p >= end)
    return 0;

  switch (*p) {
    case 'i':
    case 'f':
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      /* find first non-digit */
      const char* align_end = p;
      while (align_end < end && *align_end >= '0' && *align_end <= '9')
        align_end++;

      uint32_t value;
      if (!read_uint32(&p, align_end, &value)) {
        FATAL("%d:%d: invalid alignment\n", t.range.start.line,
              t.range.start.col);
      }
      if ((value & (value - 1)) != 0) {
        FATAL("%d:%d: alignment must be power-of-two\n", t.range.start.line,
              t.range.start.col);
      }

      if (p >= end)
        return 0;
      if (*p != '.')
        return 0;
      p++;
      break;
    }

    default:
      return 0;
  }

  /* read mem type suffix */
  int i;
  for (i = 0; i < num_types; ++i) {
    const char* name = types[i].name;
    int name_len = strlen(name);
    if (p + name_len == end) {
      if (strncmp(p, name, name_len) == 0) {
        *type = types[i].type;
        return 1;
      }
    }
  }

  return 0;
}

static void unexpected_token(Token t) {
  if (t.type == TOKEN_TYPE_EOF) {
    FATAL("%d:%d: unexpected EOF\n", t.range.start.line, t.range.start.col);
  } else {
    FATAL("%d:%d: unexpected token \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void check_type(SourceLocation loc,
                       Type actual,
                       Type expected,
                       const char* desc) {
  if (actual != expected) {
    FATAL("%d:%d: type mismatch%s. got %s, expected %s\n", loc.line, loc.col,
          desc, s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg(SourceLocation loc,
                           Type actual,
                           Type expected,
                           const char* desc,
                           int arg_index) {
  if (actual != expected) {
    FATAL("%d:%d: type mismatch for argument %d of %s. got %s, expected %s\n",
          loc.line, loc.col, arg_index, desc, s_type_names[actual],
          s_type_names[expected]);
  }
}

static Type get_result_type(SourceLocation loc, Function* function) {
  switch (function->num_results) {
    case 0:
      return TYPE_VOID;

    case 1:
      return function->result_types[0];

    default:
      FATAL("%d:%d: multiple return values currently unsupported\n", loc.line,
            loc.col);
      return TYPE_VOID;
  }
}

static void parse_generic(Tokenizer* tokenizer) {
  int nesting = 1;
  while (nesting > 0) {
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      nesting++;
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      nesting--;
    } else if (t.type == TOKEN_TYPE_EOF) {
      FATAL("unexpected eof.\n");
    }
  }
}

static int parse_var(Tokenizer* tokenizer,
                     Binding* bindings,
                     int num_bindings,
                     const char* desc) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    const char* p = t.range.start.pos;
    const char* end = t.range.end.pos;
    if (end - p >= 1 && p[0] == '$') {
      /* var name */
      int i;
      for (i = 0; i < num_bindings; ++i) {
        const char* name = bindings[i].name;
        if (name && strncmp(name, p, end - p) == 0)
          return i;
      }
      FATAL("%d:%d: undefined %s variable \"%.*s\"\n", t.range.start.line,
            t.range.start.col, desc, (int)(end - p), p);
    } else {
      /* var index */
      uint32_t index;
      if (!read_uint32(&p, t.range.end.pos, &index) || p != t.range.end.pos) {
        FATAL("%d:%d: invalid var index\n", t.range.start.line,
              t.range.start.col);
      }

      if (index >= num_bindings) {
        FATAL("%d:%d: %s variable out of range (max %d)\n", t.range.start.line,
              t.range.start.col, desc, num_bindings);
      }

      return index;
    }
  } else {
    unexpected_token(t);
    return -1;
  }
}

static int parse_function_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, module->functions, module->num_functions,
                   "function");
}

static int parse_global_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, module->globals, module->num_globals, "global");
}

static int parse_arg_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->locals, function->num_args,
                   "function argument");
}

static int parse_local_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->locals, function->num_locals, "local");
}

static int parse_label_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->labels, function->num_labels, "label");
}

static void parse_type(Tokenizer* tokenizer, Type* type) {
  Token t = read_token(tokenizer);
  if (!match_type(t, type)) {
    FATAL("%d:%d: expected type\n", t.range.start.line, t.range.start.col);
  }
}

static Type parse_expr(Tokenizer* tokenizer,
                       Module* module,
                       Function* function);

static Type parse_block(Tokenizer* tokenizer,
                        Module* module,
                        Function* function) {
  Type type;
  while (1) {
    type = parse_expr(tokenizer, module, function);
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(tokenizer, t);
  }
  return type;
}

static void parse_literal(Tokenizer* tokenizer, Type type) {
  Token t = read_token(tokenizer);
  expect_atom(t);
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  switch (type) {
    case TYPE_I32: {
      uint32_t value;
      if (!read_uint32(&p, end, &value)) {
        FATAL("%d:%d: invalid unsigned 32-bit int\n", t.range.start.line,
              t.range.start.col);
      }
      break;
    }

    case TYPE_I64: {
      uint64_t value;
      if (!read_uint64(&p, end, &value)) {
        FATAL("%d:%d: invalid unsigned 64-bit int\n", t.range.start.line,
              t.range.start.col);
      }
      break;
    }

    case TYPE_F32:
    case TYPE_F64: {
      double value;
      if (!read_double(&p, end, &value)) {
        FATAL("%d:%d: invalid double\n", t.range.start.line, t.range.start.col);
      }
      break;
    }

    default:
      assert(0);
  }
}

static Type parse_switch(Tokenizer* tokenizer,
                         Module* module,
                         Function* function,
                         Type in_type) {
  int num_cases = 0;
  Type cond_type = parse_expr(tokenizer, module, function);
  check_type(tokenizer->loc, cond_type, in_type, " in switch condition");
  Type type = TYPE_VOID;
  Token t = read_token(tokenizer);
  while (1) {
    expect_open(t);
    Token open = t;
    t = read_token(tokenizer);
    expect_atom(t);
    if (match_atom(t, "case")) {
      parse_literal(tokenizer, in_type);
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        Type value_type;
        while (1) {
          value_type = parse_expr(tokenizer, module, function);
          t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
            break;
          } else if (t.type == TOKEN_TYPE_ATOM &&
                     match_atom(t, "fallthrough")) {
            expect_close(read_token(tokenizer));
            break;
          }
          rewind_token(tokenizer, t);
        }

        if (++num_cases == 1) {
          type = value_type;
        } else if (value_type == TYPE_VOID) {
          type = TYPE_VOID;
        } else {
          check_type(t.range.start, value_type, type, " in switch case");
        }
      }
    } else {
      /* default case */
      rewind_token(tokenizer, open);
      Type value_type = parse_expr(tokenizer, module, function);
      if (value_type == TYPE_VOID) {
        type = TYPE_VOID;
      } else {
        check_type(t.range.start, value_type, type, " in switch default case");
      }
      expect_close(read_token(tokenizer));
      break;
    }
    t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
  }
  return type;
}

static Type parse_expr(Tokenizer* tokenizer,
                       Module* module,
                       Function* function) {
  Type type = TYPE_VOID;
  expect_open(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    Type in_type;
    MemType mem_type;
    if (match_atom(t, "nop")) {
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "block")) {
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "if")) {
      Type cond_type = parse_expr(tokenizer, module, function);
      check_type(tokenizer->loc, cond_type, TYPE_I32, " of condition");
      Type true_type = parse_expr(tokenizer, module, function);
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        Type false_type = parse_expr(tokenizer, module, function);
        if (true_type == TYPE_VOID || false_type == TYPE_VOID) {
          type = TYPE_VOID;
        } else {
          check_type(tokenizer->loc, false_type, true_type,
                     " between true and false branches");
        }
        expect_close(read_token(tokenizer));
      } else {
        type = true_type;
      }
    } else if (match_atom(t, "loop")) {
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "label")) {
      t = read_token(tokenizer);
      realloc_list((void**)&function->labels, &function->num_labels,
                   sizeof(Binding));
      Binding* binding = &function->labels[function->num_labels - 1];
      binding->name = NULL;

      if (t.type == TOKEN_TYPE_ATOM) {
        /* label */
        expect_var_name(t);
        binding->name =
            strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
      } else if (t.type == TOKEN_TYPE_OPEN_PAREN) {
        /* no label */
        rewind_token(tokenizer, t);
      } else {
        unexpected_token(t);
      }
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "break")) {
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        /* TODO(binji): how to check that the given label is a parent? */
        parse_label_var(tokenizer, function);
        expect_close(read_token(tokenizer));
      }
    } else if (match_switch(t, &in_type)) {
      type = parse_switch(tokenizer, module, function, in_type);
    } else if (match_atom(t, "call")) {
      int index = parse_function_var(tokenizer, module);
      Function* callee = &module->functions[index];

      int num_args = 0;
      while (1) {
        Token t = read_token(tokenizer);
        if (t.type == TOKEN_TYPE_CLOSE_PAREN)
          break;
        rewind_token(tokenizer, t);
        if (++num_args > callee->num_args) {
          FATAL("%d:%d: too many arguments to function. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_args,
                callee->num_args);
        }
        Type arg_type = parse_expr(tokenizer, module, function);
        Type expected = callee->locals[num_args - 1].type;
        check_type_arg(t.range.start, arg_type, expected, "call", num_args - 1);
      }

      if (num_args < callee->num_args) {
        FATAL("%d:%d: too few arguments to function. got %d, expected %d\n",
              t.range.start.line, t.range.start.col, num_args,
              callee->num_args);
      }

      type = get_result_type(t.range.start, callee);
    } else if (match_atom(t, "dispatch")) {
      /* TODO(binji) */
    } else if (match_atom(t, "return")) {
      int num_results = 0;
      while (1) {
        t = read_token(tokenizer);
        if (t.type == TOKEN_TYPE_CLOSE_PAREN)
          break;
        if (++num_results > function->num_results) {
          FATAL("%d:%d: too many return values. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_results,
                function->num_results);
        }
        rewind_token(tokenizer, t);
        Type result_type = parse_expr(tokenizer, module, function);
        Type expected = function->result_types[num_results - 1];
        check_type_arg(t.range.start, result_type, expected, "return",
                       num_results - 1);
      }

      if (num_results < function->num_results) {
        FATAL("%d:%d: too few return values. got %d, expected %d\n",
              t.range.start.line, t.range.start.col, num_results,
              function->num_results);
      }

      type = get_result_type(t.range.start, function);
    } else if (match_atom(t, "destruct")) {
      /* TODO(binji) */
    } else if (match_atom(t, "getparam")) {
      int index = parse_arg_var(tokenizer, function);
      type = function->locals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "getlocal")) {
      int index = parse_local_var(tokenizer, function);
      type = function->locals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "setlocal")) {
      int index = parse_local_var(tokenizer, function);
      Binding* binding = &function->locals[index];
      type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, type, binding->type, "");
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "load_global")) {
      int index = parse_global_var(tokenizer, module);
      type = module->globals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "store_global")) {
      int index = parse_global_var(tokenizer, module);
      Binding* binding = &module->globals[index];
      type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, type, binding->type, "");
      expect_close(read_token(tokenizer));
    } else if (match_load_store(t, "load", &mem_type)) {
      Type index_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, index_type, TYPE_I32, " of load index");
      type = mem_type_to_type(mem_type);
      expect_close(read_token(tokenizer));
    } else if (match_load_store(t, "store", &mem_type)) {
      Type index_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, index_type, TYPE_I32, " of store index");
      type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, type, mem_type_to_type(mem_type),
                 " of store value");
      expect_close(read_token(tokenizer));
    } else if (match_const(t, &type)) {
      parse_literal(tokenizer, type);
      expect_close(read_token(tokenizer));
    } else if (match_unary(t, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, type, " of unary op");
      type = value_type;
      expect_close(read_token(tokenizer));
    } else if (match_binary(t, &type)) {
      Type value0_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value0_type, type, "binary op", 0);
      Type value1_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value1_type, type, "binary op", 1);
      assert(value0_type == value1_type);
      type = value0_type;
      expect_close(read_token(tokenizer));
    } else if (match_compare(t, &in_type)) {
      Type value0_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value0_type, in_type, "compare op", 0);
      Type value1_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value1_type, in_type, "compare op", 1);
      type = TYPE_I32;
      expect_close(read_token(tokenizer));
    } else if (match_convert(t, &in_type, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, in_type, " of convert op");
      expect_close(read_token(tokenizer));
    } else if (match_cast(t, &in_type, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, in_type, " of cast op");
      expect_close(read_token(tokenizer));
    } else {
      unexpected_token(t);
    }
  }
  return type;
}

static void parse_func(Tokenizer* tokenizer,
                       Module* module,
                       Function* function) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(tokenizer);
  }

  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      Token open = t;
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "param") || match_atom(t, "result") ||
            match_atom(t, "local")) {
          /* Skip, this has already been pre-parsed */
          parse_generic(tokenizer);
        } else {
          rewind_token(tokenizer, open);
          Type type = parse_expr(tokenizer, module, function);
          Type result_type = get_result_type(t.range.start, function);
          if (result_type != TYPE_VOID) {
            check_type(t.range.start, type, result_type, " in function result");
          }
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
}

static void preparse_binding_list(Tokenizer* tokenizer,
                                  Binding** bindings,
                                  int* num_bindings,
                                  const char* desc) {
  Token t = read_token(tokenizer);
  Type type;
  if (match_type(t, &type)) {
    while (1) {
      realloc_list((void**)bindings, num_bindings, sizeof(Binding));
      Binding* binding = &(*bindings)[*num_bindings - 1];
      binding->name = NULL;
      binding->type = type;

      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_CLOSE_PAREN)
        break;
      else if (!match_type(t, &type))
        unexpected_token(t);
    }
  } else {
    expect_var_name(t);
    parse_type(tokenizer, &type);
    expect_close(read_token(tokenizer));

    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_binding_by_name(*bindings, *num_bindings, name) != -1) {
      FATAL("%d:%d: redefinition of %s \"%s\"\n", t.range.start.line,
            t.range.start.col, desc, name);
    }

    realloc_list((void**)bindings, num_bindings, sizeof(Binding));
    Binding* binding = &(*bindings)[*num_bindings - 1];
    binding->name = name;
    binding->type = type;
  }
}

static void preparse_func(Tokenizer* tokenizer, Module* module) {
  realloc_list((void**)&module->functions, &module->num_functions,
               sizeof(Function));
  Function* function = &module->functions[module->num_functions - 1];
  memset(function, 0, sizeof(Function));

  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_function_by_name(module, name) != -1) {
      FATAL("%d:%d: redefinition of function \"%s\"\n", t.range.start.line,
            t.range.start.col, name);
    }

    function->name = name;
    t = read_token(tokenizer);
  }

  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      Type type;
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "param")) {
          /* Don't allow adding params after locals */
          if (function->num_args != function->num_locals) {
            FATAL("%d:%d: parameters must come before locals\n",
                  t.range.start.line, t.range.start.col);
          }
          preparse_binding_list(tokenizer, &function->locals,
                                &function->num_args, "function argument");
          function->num_locals = function->num_args;
        } else if (match_atom(t, "result")) {
          t = read_token(tokenizer);
          if (match_type(t, &type)) {
            while (1) {
              realloc_list((void**)&function->result_types,
                           &function->num_results, sizeof(Type));
              function->result_types[function->num_results - 1] = type;

              t = read_token(tokenizer);
              if (t.type == TOKEN_TYPE_CLOSE_PAREN)
                break;
              else if (!match_type(t, &type))
                unexpected_token(t);
            }
          } else {
            unexpected_token(t);
          }
        } else if (match_atom(t, "local")) {
          preparse_binding_list(tokenizer, &function->locals,
                                &function->num_locals, "local");
        } else {
          rewind_token(tokenizer, t);
          parse_generic(tokenizer);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
}

static void preparse_module(Tokenizer* tokenizer, Module* module) {
  Token t = read_token(tokenizer);
  Token first = t;
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          preparse_func(tokenizer, module);
        } else if (match_atom(t, "global")) {
          preparse_binding_list(tokenizer, &module->globals,
                                &module->num_globals, "global");
        } else {
          parse_generic(tokenizer);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
  rewind_token(tokenizer, first);
}

static void destroy_binding_list(Binding* bindings, int num_bindings) {
  int i;
  for (i = 0; i < num_bindings; ++i)
    free(bindings[i].name);
  free(bindings);
}

static void destroy_module(Module* module) {
  int i;
  for (i = 0; i < module->num_functions; ++i) {
    Function* function = &module->functions[i];
    free(function->name);
    free(function->result_types);
    destroy_binding_list(function->locals, function->num_locals);
    destroy_binding_list(function->labels, function->num_labels);
  }
  destroy_binding_list(module->globals, module->num_globals);
  free(module->exports);
}

static void parse_module(Tokenizer* tokenizer) {
  Module module = {};
  preparse_module(tokenizer, &module);

  int function_index = 0;
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          Function* function = &module.functions[function_index++];
          parse_func(tokenizer, &module, function);
        } else if (match_atom(t, "global")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "export")) {
          expect_string(read_token(tokenizer));
          parse_function_var(tokenizer, &module);
          expect_close(read_token(tokenizer));
        } else if (match_atom(t, "table")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "memory")) {
          parse_generic(tokenizer);
        } else {
          unexpected_token(t);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
  destroy_module(&module);
}

static void parse(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "module")) {
          parse_module(tokenizer);
        } else if (match_atom(t, "asserteq")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "invoke")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "assertinvalid")) {
          parse_generic(tokenizer);
        } else {
          unexpected_token(t);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_EOF) {
      break;
    } else {
      unexpected_token(t);
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [file.wasm]\n", argv[0]);
    return 0;
  }

  const char* filename = argv[1];
  FILE* f = fopen(filename, "rb");
  if (!f) {
    FATAL("unable to read %s\n", filename);
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  void* data = malloc(fsize);
  if (!data) {
    FATAL("unable to alloc %zd bytes\n", fsize);
  }
  if (fread(data, 1, fsize, f) != fsize) {
    FATAL("unable to read %zd bytes from %s\n", fsize, filename);
  }
  fclose(f);

  Source source;
  source.start = data;
  source.end = data + fsize;

  Tokenizer tokenizer;
  tokenizer.source = source;
  tokenizer.loc.pos = source.start;
  tokenizer.loc.line = 1;
  tokenizer.loc.col = 1;

  parse(&tokenizer);
  return 0;
}
