#ifndef WASM_PARSE_H
#define WASM_PARSE_H

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

struct Module;
struct Function;
enum Opcode;

typedef uintptr_t Cookie;

typedef struct Parser {
  void* user_data;
  void (*before_module)(struct Module* m, void* user_data);
  void (*after_module)(struct Module* m, void* user_data);
  void (*before_function)(struct Module* m,
                          struct Function* f,
                          void* user_data);
  void (*after_function)(struct Module* m,
                         struct Function* f,
                         int num_exprs,
                         void* user_data);
  void (*before_export)(struct Module* m, void* user_data);
  void (*after_export)(struct Module* m, int function_index, void* user_data);

  void (*before_binary)(enum Opcode opcode, void* user_data);
  Cookie (*before_block)(void* user_data);
  void (*after_block)(int num_exprs, Cookie cookie, void* user_data);
  void (*after_break)(int depth, void* user_data);
  void (*before_call)(int function_index, void* user_data);
  void (*before_compare)(enum Opcode opcode, void* user_data);
  void (*before_const)(enum Opcode opcode, void* user_data);
  void (*before_convert)(enum Opcode opcode, void* user_data);
  Cookie (*before_label)(void* user_data);
  void (*after_label)(int num_exprs, Cookie cookie, void* user_data);
  void (*after_get_local)(int remapped_index, void* user_data);
  Cookie (*before_loop)(void* user_data);
  void (*after_loop)(int num_exprs, Cookie cookie, void* user_data);
  Cookie (*before_if)(void* user_data);
  void (*after_if)(int with_else, Cookie cookie, void* user_data);
  void (*before_load)(enum Opcode opcode, uint8_t access, void* user_data);
  void (*after_load_global)(int index, void* user_data);
  void (*after_nop)(void* user_data);
  void (*before_return)(void* user_data);
  void (*before_set_local)(int index, void* user_data);
  void (*before_store)(enum Opcode opcode, uint8_t access, void* user_data);
  void (*before_store_global)(int index, void* user_data);
  void (*before_unary)(enum Opcode opcode, void* user_data);

  void (*u32_literal)(uint32_t value, void* user_data);
  void (*u64_literal)(uint64_t value, void* user_data);
  void (*f32_literal)(float value, void* user_data);
  void (*f64_literal)(double value, void* user_data);
} Parser;

size_t copy_string_contents(Token t, char* dest, size_t size);
void parse_module(Parser* parser, Tokenizer* tokenizer);
void parse_file(Parser* parser, Tokenizer* tokenizer);

#endif /* WASM_PARSE_H */
