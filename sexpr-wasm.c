#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABS_TO_SPACES 8

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
              fprintf(stderr, "newline in string\n");
              exit(1);
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
    fprintf(stderr, "%d:%d: expected '(', not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}

static void expect_close(Token t) {
  if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    fprintf(stderr, "%d:%d: expected ')', not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}


static void expect_atom(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    fprintf(stderr, "%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}

static int match_atom(Token t, const char* s) {
  return strncmp(t.range.start.pos, s, t.range.end.pos - t.range.start.pos) ==
         0;
}

static int match_atom_prefix(Token t, const char* s) {
  int slen = strlen(s);
  int len = (int)(t.range.end.pos - t.range.start.pos);
  if (slen < len)
    len = slen;
  return strncmp(t.range.start.pos, s, slen) == 0;
}

static int match_int_type(const char* s) {
  return s[0] == 'i' &&
         ((s[1] == '3' && s[2] == '2') || (s[1] == '6' && s[2] == '4'));
}

static int match_float_type(const char* s) {
  return s[0] == 'f' &&
         ((s[1] == '3' && s[2] == '2') || (s[1] == '6' && s[2] == '4'));
}

static int match_int_float_type(const char* s) {
  return (s[0] == 'i' || s[0] == 'f') &&
         ((s[1] == '3' && s[2] == '2') || (s[1] == '6' && s[2] == '4'));
}

static int match_atom_int_float(Token t, const char* prefix) {
  int plen = strlen(prefix);
  const char* p = t.range.start.pos;
  int len = (int)(t.range.end.pos - p);
  if (len != plen + 3)
    return 0;
  if (strncmp(p, prefix, plen) != 0)
    return 0;
  return match_int_float_type(&p[plen]);
}

static int match_atom_int(Token t, const char* prefix) {
  int plen = strlen(prefix);
  const char* p = t.range.start.pos;
  int len = (int)(t.range.end.pos - p);
  if (len != plen + 3)
    return 0;
  if (strncmp(p, prefix, plen) != 0)
    return 0;
  return match_int_type(&p[plen]);
}

static int match_atom_float(Token t, const char* prefix) {
  int plen = strlen(prefix);
  const char* p = t.range.start.pos;
  int len = (int)(t.range.end.pos - p);
  if (len != plen + 3)
    return 0;
  if (strncmp(p, prefix, plen) != 0)
    return 0;
  return match_float_type(&p[plen]);
}

static int match_unary(Token t) {
  return match_atom_int_float(t, "neg.") || match_atom_int_float(t, "abs.") ||
         match_atom_int_float(t, "not.") || match_atom_int(t, "clz.") ||
         match_atom_int(t, "ctz.") || match_atom_float(t, "ceil.") ||
         match_atom_float(t, "floor.") || match_atom_float(t, "trunc.") ||
         match_atom_float(t, "round.");
}

static int match_binary(Token t) {
  return match_atom_int_float(t, "add.") || match_atom_int_float(t, "sub.") ||
         match_atom_int_float(t, "mul.") || match_atom_int(t, "divs.") ||
         match_atom_int(t, "divu.") || match_atom_int(t, "mods.") ||
         match_atom_int(t, "modu.") || match_atom_int(t, "and.") ||
         match_atom_int(t, "or.") || match_atom_int(t, "xor.") ||
         match_atom_int(t, "shl.") || match_atom_int(t, "shr.") ||
         match_atom_int(t, "sar.") || match_atom_float(t, "div.") ||
         match_atom_float(t, "copysign.");
}

static int match_compare(Token t) {
  return match_atom_int_float(t, "eq.") || match_atom_int_float(t, "neq.") ||
         match_atom_int(t, "lts.") || match_atom_int(t, "ltu.") ||
         match_atom_int(t, "les.") || match_atom_int(t, "leu.") ||
         match_atom_int(t, "gts.") || match_atom_int(t, "gtu.") ||
         match_atom_float(t, "lt.") || match_atom_float(t, "le.") ||
         match_atom_float(t, "gt.") || match_atom_float(t, "ge.");
}

static int match_convert(Token t) {
  const char prefix[] = "convert";
  int plen = strlen(prefix);
  const char* p = t.range.start.pos;
  int len = (int)(t.range.end.pos - p);
  if (len < plen + 1)
    return 0;
  if (strncmp(p, prefix, plen) != 0)
    return 0;

  switch (p[plen]) {
    case '.':
      if (len != plen + 8)
        return 0;
      p += plen + 1;
      return match_float_type(p) && match_float_type(p + 4);

    case 's':
    case 'u':
      if (len != plen + 9)
        return 0;
      if (p[plen + 1] != '.')
        return 0;
      p += plen + 2;
      return (match_int_type(p) && match_int_type(p + 4)) ||
             (match_int_type(p) && match_float_type(p + 4)) ||
             (match_float_type(p) && match_int_type(p + 4));

    default:
      return 0;
  }
}

static void unexpected_token(Token t) {
  fprintf(stderr, "%d:%d: unexpected token \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  exit(1);
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
      fprintf(stderr, "unexpected eof.\n");
      exit(1);
    }
  }
}

static void parse_var(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    if (t.range.end.pos - t.range.start.pos >= 1 &&
        t.range.start.pos[0] == '$') {
      /* var name */
    } else {
      /* var index */
      char* endptr;
      errno = 0;
      int len = (int)(t.range.end.pos - t.range.start.pos);
      long int value = strtoul(t.range.start.pos, &endptr, 10);
      if (endptr - t.range.start.pos != len || errno != 0) {
        fprintf(stderr, "%d:%d: invalid var index\n", t.range.start.line,
                t.range.start.col);
        exit(1);
      }
      (void)value;
    }
  }
}

static void parse_expr(Tokenizer* tokenizer);

static void parse_block(Tokenizer* tokenizer) {
  while (1) {
    parse_expr(tokenizer);
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(tokenizer, t);
  }
}

static void parse_expr(Tokenizer* tokenizer) {
  expect_open(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    if (match_atom(t, "nop")) {
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "block")) {
      parse_block(tokenizer);
    } else if (match_atom(t, "if")) {
      parse_expr(tokenizer); /* condition */
      parse_expr(tokenizer); /* true */
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        parse_expr(tokenizer); /* false */
        expect_close(read_token(tokenizer));
      }
    } else if (match_atom(t, "loop")) {
      parse_block(tokenizer);
    } else if (match_atom(t, "label")) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        /* label */
      } else if (t.type == TOKEN_TYPE_OPEN_PAREN) {
        /* no label */
        rewind_token(tokenizer, t);
      } else {
        unexpected_token(t);
      }
      parse_block(tokenizer);
    } else if (match_atom(t, "break")) {
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        parse_var(tokenizer);
        expect_close(read_token(tokenizer));
      }
    } else if (match_atom_prefix(t, "switch")) {
      /* TODO(binji) */
    } else if (match_atom(t, "call")) {
      parse_var(tokenizer);
      parse_block(tokenizer);
    } else if (match_atom(t, "dispatch")) {
      parse_var(tokenizer);
    } else if (match_atom(t, "return")) {
      parse_block(tokenizer);
    } else if (match_atom(t, "destruct")) {
      parse_var(tokenizer);
    } else if (match_atom(t, "getparam")) {
      parse_var(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "getlocal")) {
      parse_var(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "setlocal")) {
      parse_var(tokenizer);
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "loadglobal")) {
      parse_var(tokenizer);
    } else if (match_atom(t, "storeglobal")) {
      parse_var(tokenizer);
    } else if (match_atom(t, "load")) {
      /* TODO(binji) */
    } else if (match_atom(t, "store")) {
      /* TODO(binji) */
    } else if (match_atom_prefix(t, "const")) {
      expect_atom(read_token(tokenizer));
      expect_close(read_token(tokenizer));
    } else if (match_unary(t)) {
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_binary(t)) {
      parse_expr(tokenizer);
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_compare(t)) {
      parse_expr(tokenizer);
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_convert(t)) {
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else if (match_atom_prefix(t, "cast")) {
      parse_expr(tokenizer);
      expect_close(read_token(tokenizer));
    } else {
      unexpected_token(t);
    }
  }
}

static void parse_func(Tokenizer* tokenizer) {
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
        if (match_atom(t, "param")) {
          expect_atom(read_token(tokenizer));
          Token t = read_token(tokenizer);
          if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
            expect_atom(t);
            expect_close(read_token(tokenizer));
          }
        } else if (match_atom(t, "result")) {
          expect_atom(read_token(tokenizer));
          expect_close(read_token(tokenizer));
        } else if (match_atom(t, "local")) {
          expect_atom(read_token(tokenizer));
          Token t = read_token(tokenizer);
          if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
            expect_atom(t);
            expect_close(read_token(tokenizer));
          }
        } else {
          rewind_token(tokenizer, open);
          parse_expr(tokenizer);
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

static void parse_module(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          parse_func(tokenizer);
        } else if (match_atom(t, "global")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "export")) {
          parse_generic(tokenizer);
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
    fprintf(stderr, "unable to read %s\n", filename);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  void* data = malloc(fsize);
  if (!data) {
    fprintf(stderr, "unable to alloc %zd bytes\n", fsize);
    return 1;
  }
  if (fread(data, 1, fsize, f) != fsize) {
    fprintf(stderr, "unable to read %zd bytes from %s\n", fsize, filename);
    return 1;
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
