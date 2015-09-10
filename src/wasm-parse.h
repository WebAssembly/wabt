#ifndef WASM_PARSE_H
#define WASM_PARSE_H
//#include "wasm.h"

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

extern int g_verbose;
extern const char* g_outfile;
extern int g_dump_module;

typedef struct Module Module;
typedef struct Function Function;

typedef struct Parser {
  void* user_data;
  void (*before_parse_module)(Module* m, void* user_data);
  void (*after_parse_module)(Module* m, void* user_data);
  void (*before_parse_function)(Module* m, Function* f, void* user_data);
  void (*after_parse_function)(Module* m,
                               Function* f,
                               int num_exprs,
                               void* user_data);
  void (*before_parse_export)(Module* m, void* user_data);
  void (*after_parse_export)(Module* m, int function_index, void* user_data);
} Parser;

size_t copy_string_contents(Token t, char* dest, size_t size);
void parse_module(Parser* parser, Tokenizer* tokenizer);
void parse_file(Parser* parser, Tokenizer* tokenizer);

#endif /* WASM_PARSE_H */
