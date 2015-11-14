%{
#include "wasm-tokens.h"
#include "wasm-parser.h"

void yyerror(WasmLocation*, void*, const char*, ...);

#define YYSTYPE WasmToken
#define YYLTYPE WasmLocation

typedef struct WasmScannerExtra {
  int column;
  int comment_nesting;
} WasmScannerExtra;

#define EXTRA(scanner) ((WasmScannerExtra*)yyget_extra(scanner))
#define COMMENT_NESTING(scanner) (EXTRA(scanner)->comment_nesting)
#define COLUMN(scanner) (EXTRA(scanner)->column)
#define YY_USER_ACTION                                 \
  {                                                    \
    WasmScannerExtra* extra = EXTRA(yyscanner);        \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = extra->column;              \
    extra->column += yyleng;                           \
    yylloc->last_column = extra->column;               \
    yylval->start = yytext;                            \
    yylval->end = yytext + yyleng;                     \
  }

#define TYPE(name) yylval->type = WASM_TYPE_##name
#define SIGN(struct_, sign_) \
  yylval->struct_.sign = sign_ ? WASM_SIGNED : WASM_UNSIGNED
#define MEMSIZE(size_) yylval->mem.size = WASM_MEM_SIZE_##size_
#define MEMSIZESIGN(size_, sign_) \
  MEMSIZE(size_);                 \
  SIGN(mem, sign_)
#define UNOP(name) yylval->unary.op = WASM_UNARY_OP_##name
#define BINOP(name) yylval->binary.op = WASM_BINARY_OP_##name
#define BINOPSIGN(name, sign_) \
  BINOP(name);                 \
  SIGN(binary, sign_)
#define CMPOP(name) yylval->compare.op = WASM_COMPARE_OP_##name
#define CMPOPSIGN(name, sign_) \
  CMPOP(name);                 \
  SIGN(compare, sign_)
#define CONVTYPE(type) yylval->convert.type2 = WASM_TYPE_##type
#define CONVTYPESIGN(type, sign_) \
  CONVTYPE(type);                 \
  SIGN(convert, sign_)
#define CASTTYPE(type) yylval->cast.type2 = WASM_TYPE_##type

#define TOK(name) WASM_TOKEN_TYPE_##name

%}

%option bison-bridge
%option bison-locations
%option yylineno
%option reentrant

%x LINE_COMMENT
%x BLOCK_COMMENT

SPACE     [ \t]
DIGIT     [0-9]
DIGITS    [0-9]+
HEXDIGIT  [0-9a-fA-F]
LETTER    [a-zA-Z]
SYMBOL    [+\-*/\\\^~=<>!?@#$%&|:`.]
TICK      "`"
ESCAPE    [nt\\'"]
CHARACTER [^"\\\x00-\x1f\x7f]|"\"{ESCAPE}|"\"{HEXDIGIT}{HEXDIGIT}
SIGN      [+-]?
NUM       {SIGN}{DIGIT}+
HEXNUM    {SIGN}"0x"{HEXDIGIT}+
INT       {NUM}|{HEXNUM}
FLOAT0    {NUM}"."{DIGIT}+
FLOAT1    {NUM}("."{DIGIT}+)?[eE]{NUM}
HEXFLOAT  {SIGN}"0x"{HEXDIGIT}+"."?{HEXDIGIT}*"p"{SIGN}{DIGIT}+
INFINITY  {SIGN}"infinity"
NAN       {SIGN}"nan"|{SIGN}"nan:0x"{HEXDIGIT}+
FLOAT     {FLOAT0}|{FLOAT1}|{HEXFLOAT}|{INFINITY}|{NAN}
TEXT      "\""{CHARACTER}*"\""
ATOM      ({LETTER}|{DIGIT}|"_"|{TICK}|{SYMBOL})+
NAME      "$"{ATOM}

%%

"("                   { return TOK(LPAR); }
")"                   { return TOK(RPAR); }
{INT}                 { return TOK(INT); }
{FLOAT}               { return TOK(FLOAT); }
{TEXT}                { return TOK(TEXT); }
"i32"                 { TYPE(I32); return TOK(VALUE_TYPE); }
"i64"                 { TYPE(I64); return TOK(VALUE_TYPE); }
"f32"                 { TYPE(F32); return TOK(VALUE_TYPE); }
"f64"                 { TYPE(F64); return TOK(VALUE_TYPE); }
"nop"                 { return TOK(NOP); }
"block"               { return TOK(BLOCK); }
"if"                  { return TOK(IF); }
"if_else"             { return TOK(IF_ELSE); }
"loop"                { return TOK(LOOP); }
"label"               { return TOK(LABEL); }
"br"                  { return TOK(BR); }
"br_if"               { return TOK(BR_IF); }
"tableswitch"         { return TOK(TABLESWITCH); }
"case"                { return TOK(CASE); }
"call"                { return TOK(CALL); }
"call_import"         { return TOK(CALL_IMPORT); }
"call_indirect"       { return TOK(CALL_INDIRECT); }
"return"              { return TOK(RETURN); }
"get_local"           { return TOK(GET_LOCAL); }
"set_local"           { return TOK(SET_LOCAL); }
"i32.load"            { TYPE(I32); return TOK(LOAD); }
"i64.load"            { TYPE(I64); return TOK(LOAD); }
"f32.load"            { TYPE(F32); return TOK(LOAD); }
"f64.load"            { TYPE(F64); return TOK(LOAD); }
"i32.store"           { TYPE(I32); return TOK(STORE); }
"i64.store"           { TYPE(I64); return TOK(STORE); }
"f32.store"           { TYPE(F32); return TOK(STORE); }
"f64.store"           { TYPE(F64); return TOK(STORE); }
"i32.load8_s"         { TYPE(I32); MEMSIZESIGN(8, 1); return TOK(LOAD_EXTEND); }
"i64.load8_s"         { TYPE(I64); MEMSIZESIGN(8, 1); return TOK(LOAD_EXTEND); }
"i32.load8_u"         { TYPE(I32); MEMSIZESIGN(8, 0); return TOK(LOAD_EXTEND); }
"i64.load8_u"         { TYPE(I64); MEMSIZESIGN(8, 0); return TOK(LOAD_EXTEND); }
"i32.load16_s"        { TYPE(I32); MEMSIZESIGN(16, 1); return TOK(LOAD_EXTEND); }
"i64.load16_s"        { TYPE(I64); MEMSIZESIGN(16, 1); return TOK(LOAD_EXTEND); }
"i32.load16_u"        { TYPE(I32); MEMSIZESIGN(16, 0); return TOK(LOAD_EXTEND); }
"i64.load16_u"        { TYPE(I64); MEMSIZESIGN(16, 0); return TOK(LOAD_EXTEND); }
"i64.load32_s"        { TYPE(I64); MEMSIZESIGN(32, 1); return TOK(LOAD_EXTEND); }
"i64.load32_u"        { TYPE(I64); MEMSIZESIGN(32, 0); return TOK(LOAD_EXTEND); }
"i32.store8"          { TYPE(I32); MEMSIZE(8); return TOK(STORE_WRAP); }
"i64.store8"          { TYPE(I64); MEMSIZE(8); return TOK(STORE_WRAP); }
"i32.store16"         { TYPE(I32); MEMSIZE(16); return TOK(STORE_WRAP); }
"i64.store16"         { TYPE(I64); MEMSIZE(16); return TOK(STORE_WRAP); }
"i64.store32"         { TYPE(I64); MEMSIZE(32); return TOK(STORE_WRAP); }
"offset="{DIGITS}     { return TOK(OFFSET); }
"align="{DIGITS}      { return TOK(ALIGN); }
"i32.const"           { TYPE(I32); return TOK(CONST); }
"i64.const"           { TYPE(I64); return TOK(CONST); }
"f32.const"           { TYPE(F32); return TOK(CONST); }
"f64.const"           { TYPE(F64); return TOK(CONST); }
"i32.clz"             { TYPE(I32); UNOP(CLZ); return TOK(UNARY); }
"i64.clz"             { TYPE(I64); UNOP(CLZ); return TOK(UNARY); }
"i32.ctz"             { TYPE(I32); UNOP(CTZ); return TOK(UNARY); }
"i64.ctz"             { TYPE(I64); UNOP(CTZ); return TOK(UNARY); }
"i32.popcnt"          { TYPE(I32); UNOP(POPCNT); return TOK(UNARY); }
"i64.popcnt"          { TYPE(I64); UNOP(POPCNT); return TOK(UNARY); }
"f32.neg"             { TYPE(F32); UNOP(NEG); return TOK(UNARY); }
"f64.neg"             { TYPE(F64); UNOP(NEG); return TOK(UNARY); }
"f32.abs"             { TYPE(F32); UNOP(ABS); return TOK(UNARY); }
"f64.abs"             { TYPE(F64); UNOP(ABS); return TOK(UNARY); }
"f32.sqrt"            { TYPE(F32); UNOP(SQRT); return TOK(UNARY); }
"f64.sqrt"            { TYPE(F64); UNOP(SQRT); return TOK(UNARY); }
"f32.ceil"            { TYPE(F32); UNOP(CEIL); return TOK(UNARY); }
"f64.ceil"            { TYPE(F64); UNOP(CEIL); return TOK(UNARY); }
"f32.floor"           { TYPE(F32); UNOP(FLOOR); return TOK(UNARY); }
"f64.floor"           { TYPE(F64); UNOP(FLOOR); return TOK(UNARY); }
"f32.trunc"           { TYPE(F32); UNOP(TRUNC); return TOK(UNARY); }
"f64.trunc"           { TYPE(F64); UNOP(TRUNC); return TOK(UNARY); }
"f32.nearest"         { TYPE(F32); UNOP(NEAREST); return TOK(UNARY); }
"f64.nearest"         { TYPE(F64); UNOP(NEAREST); return TOK(UNARY); }
"i32.add"             { TYPE(I32); BINOP(ADD); return TOK(BINARY); }
"i64.add"             { TYPE(I64); BINOP(ADD); return TOK(BINARY); }
"i32.sub"             { TYPE(I32); BINOP(SUB); return TOK(BINARY); }
"i64.sub"             { TYPE(I64); BINOP(SUB); return TOK(BINARY); }
"i32.mul"             { TYPE(I32); BINOP(MUL); return TOK(BINARY); }
"i64.mul"             { TYPE(I64); BINOP(MUL); return TOK(BINARY); }
"i32.div_s"           { TYPE(I32); BINOPSIGN(DIV, 1); return TOK(BINARY); }
"i64.div_s"           { TYPE(I64); BINOPSIGN(DIV, 1); return TOK(BINARY); }
"i32.div_u"           { TYPE(I32); BINOPSIGN(DIV, 0); return TOK(BINARY); }
"i64.div_u"           { TYPE(I64); BINOPSIGN(DIV, 0); return TOK(BINARY); }
"i32.rem_s"           { TYPE(I32); BINOPSIGN(REM, 1); return TOK(BINARY); }
"i64.rem_s"           { TYPE(I64); BINOPSIGN(REM, 1); return TOK(BINARY); }
"i32.rem_u"           { TYPE(I32); BINOPSIGN(REM, 0); return TOK(BINARY); }
"i64.rem_u"           { TYPE(I64); BINOPSIGN(REM, 0); return TOK(BINARY); }
"i32.and"             { TYPE(I32); BINOP(AND); return TOK(BINARY); }
"i64.and"             { TYPE(I64); BINOP(AND); return TOK(BINARY); }
"i32.or"              { TYPE(I32); BINOP(OR); return TOK(BINARY); }
"i64.or"              { TYPE(I64); BINOP(OR); return TOK(BINARY); }
"i32.xor"             { TYPE(I32); BINOP(XOR); return TOK(BINARY); }
"i64.xor"             { TYPE(I64); BINOP(XOR); return TOK(BINARY); }
"i32.shl"             { TYPE(I32); BINOP(SHL); return TOK(BINARY); }
"i64.shl"             { TYPE(I64); BINOP(SHL); return TOK(BINARY); }
"i32.shr_s"           { TYPE(I32); BINOPSIGN(SHR, 1); return TOK(BINARY); }
"i64.shr_s"           { TYPE(I64); BINOPSIGN(SHR, 1); return TOK(BINARY); }
"i32.shr_u"           { TYPE(I32); BINOPSIGN(SHR, 0); return TOK(BINARY); }
"i64.shr_u"           { TYPE(I64); BINOPSIGN(SHR, 0); return TOK(BINARY); }
"f32.add"             { TYPE(F32); BINOP(ADD); return TOK(BINARY); }
"f64.add"             { TYPE(F64); BINOP(ADD); return TOK(BINARY); }
"f32.sub"             { TYPE(F32); BINOP(SUB); return TOK(BINARY); }
"f64.sub"             { TYPE(F64); BINOP(SUB); return TOK(BINARY); }
"f32.mul"             { TYPE(F32); BINOP(MUL); return TOK(BINARY); }
"f64.mul"             { TYPE(F64); BINOP(MUL); return TOK(BINARY); }
"f32.div"             { TYPE(F32); BINOP(DIV); return TOK(BINARY); }
"f64.div"             { TYPE(F64); BINOP(DIV); return TOK(BINARY); }
"f32.min"             { TYPE(F32); BINOP(MIN); return TOK(BINARY); }
"f64.min"             { TYPE(F64); BINOP(MIN); return TOK(BINARY); }
"f32.max"             { TYPE(F32); BINOP(MAX); return TOK(BINARY); }
"f64.max"             { TYPE(F64); BINOP(MAX); return TOK(BINARY); }
"f32.copysign"        { TYPE(F32); BINOP(COPYSIGN); return TOK(BINARY); }
"f64.copysign"        { TYPE(F64); BINOP(COPYSIGN); return TOK(BINARY); }
"i32.eq"              { TYPE(I32); CMPOP(EQ); return TOK(COMPARE); }
"i64.eq"              { TYPE(I64); CMPOP(EQ); return TOK(COMPARE); }
"i32.ne"              { TYPE(I32); CMPOP(EQ); return TOK(COMPARE); }
"i64.ne"              { TYPE(I64); CMPOP(EQ); return TOK(COMPARE); }
"i32.lt_s"            { TYPE(I32); CMPOPSIGN(LT, 1); return TOK(COMPARE); }
"i64.lt_s"            { TYPE(I64); CMPOPSIGN(LT, 1); return TOK(COMPARE); }
"i32.lt_u"            { TYPE(I32); CMPOPSIGN(LT, 0); return TOK(COMPARE); }
"i64.lt_u"            { TYPE(I64); CMPOPSIGN(LT, 0); return TOK(COMPARE); }
"i32.le_s"            { TYPE(I32); CMPOPSIGN(LE, 1); return TOK(COMPARE); }
"i64.le_s"            { TYPE(I64); CMPOPSIGN(LE, 1); return TOK(COMPARE); }
"i32.le_u"            { TYPE(I32); CMPOPSIGN(LE, 0); return TOK(COMPARE); }
"i64.le_u"            { TYPE(I64); CMPOPSIGN(LE, 0); return TOK(COMPARE); }
"i32.gt_s"            { TYPE(I32); CMPOPSIGN(GT, 1); return TOK(COMPARE); }
"i64.gt_s"            { TYPE(I64); CMPOPSIGN(GT, 1); return TOK(COMPARE); }
"i32.gt_u"            { TYPE(I32); CMPOPSIGN(GT, 0); return TOK(COMPARE); }
"i64.gt_u"            { TYPE(I64); CMPOPSIGN(GT, 0); return TOK(COMPARE); }
"i32.ge_s"            { TYPE(I32); CMPOPSIGN(GE, 1); return TOK(COMPARE); }
"i64.ge_s"            { TYPE(I64); CMPOPSIGN(GE, 1); return TOK(COMPARE); }
"i32.ge_u"            { TYPE(I32); CMPOPSIGN(GE, 0); return TOK(COMPARE); }
"i64.ge_u"            { TYPE(I64); CMPOPSIGN(GE, 0); return TOK(COMPARE); }
"f32.eq"              { TYPE(F32); CMPOP(EQ); return TOK(COMPARE); }
"f64.eq"              { TYPE(F64); CMPOP(EQ); return TOK(COMPARE); }
"f32.ne"              { TYPE(F32); CMPOP(NE); return TOK(COMPARE); }
"f64.ne"              { TYPE(F64); CMPOP(NE); return TOK(COMPARE); }
"f32.lt"              { TYPE(F32); CMPOP(LT); return TOK(COMPARE); }
"f64.lt"              { TYPE(F64); CMPOP(LT); return TOK(COMPARE); }
"f32.le"              { TYPE(F32); CMPOP(LE); return TOK(COMPARE); }
"f64.le"              { TYPE(F64); CMPOP(LE); return TOK(COMPARE); }
"f32.gt"              { TYPE(F32); CMPOP(GT); return TOK(COMPARE); }
"f64.gt"              { TYPE(F64); CMPOP(GT); return TOK(COMPARE); }
"f32.ge"              { TYPE(F32); CMPOP(GE); return TOK(COMPARE); }
"f64.ge"              { TYPE(F64); CMPOP(GE); return TOK(COMPARE); }
"i64.extend_s/i32"    { TYPE(I64); CONVTYPESIGN(I32, 1); return TOK(CONVERT); }
"i64.extend_u/i32"    { TYPE(I64); CONVTYPESIGN(I32, 0); return TOK(CONVERT); }
"i32.wrap/i64"        { TYPE(I32); CONVTYPE(I64); return TOK(CONVERT); }
"i32.trunc_s/f32"     { TYPE(I32); CONVTYPESIGN(F32, 1); return TOK(CONVERT); }
"i64.trunc_s/f32"     { TYPE(I64); CONVTYPESIGN(F32, 1); return TOK(CONVERT); }
"i32.trunc_s/f64"     { TYPE(I32); CONVTYPESIGN(F64, 1); return TOK(CONVERT); }
"i64.trunc_s/f64"     { TYPE(I64); CONVTYPESIGN(F64, 1); return TOK(CONVERT); }
"i32.trunc_u/f32"     { TYPE(I32); CONVTYPESIGN(F32, 0); return TOK(CONVERT); }
"i64.trunc_u/f32"     { TYPE(I64); CONVTYPESIGN(F32, 0); return TOK(CONVERT); }
"i32.trunc_u/f64"     { TYPE(I32); CONVTYPESIGN(F64, 0); return TOK(CONVERT); }
"i64.trunc_u/f64"     { TYPE(I64); CONVTYPESIGN(F64, 0); return TOK(CONVERT); }
"f32.convert_s/i32"   { TYPE(F32); CONVTYPESIGN(I32, 1); return TOK(CONVERT); }
"f64.convert_s/i32"   { TYPE(F64); CONVTYPESIGN(I32, 1); return TOK(CONVERT); }
"f32.convert_s/i64"   { TYPE(F32); CONVTYPESIGN(I64, 1); return TOK(CONVERT); }
"f64.convert_s/i64"   { TYPE(F64); CONVTYPESIGN(I64, 1); return TOK(CONVERT); }
"f32.convert_u/i32"   { TYPE(F32); CONVTYPESIGN(I32, 0); return TOK(CONVERT); }
"f64.convert_u/i32"   { TYPE(F64); CONVTYPESIGN(I32, 0); return TOK(CONVERT); }
"f32.convert_u/i64"   { TYPE(F32); CONVTYPESIGN(I64, 0); return TOK(CONVERT); }
"f64.convert_u/i64"   { TYPE(F64); CONVTYPESIGN(I64, 0); return TOK(CONVERT); }
"f64.promote/f32"     { TYPE(F64); CONVTYPE(F32); return TOK(CONVERT); }
"f32.demote/f64"      { TYPE(F32); CONVTYPE(F64); return TOK(CONVERT); }
"f32.reinterpret/i32" { TYPE(F32); CASTTYPE(I32); return TOK(CAST); }
"i32.reinterpret/f32" { TYPE(I32); CASTTYPE(F32); return TOK(CAST); }
"f64.reinterpret/i64" { TYPE(F64); CASTTYPE(I64); return TOK(CAST); }
"i64.reinterpret/f64" { TYPE(I64); CASTTYPE(F64); return TOK(CAST); }
"i32.select"          { TYPE(I32); return TOK(SELECT); }
"i64.select"          { TYPE(I64); return TOK(SELECT); }
"f32.select"          { TYPE(F32); return TOK(SELECT); }
"f64.select"          { TYPE(F64); return TOK(SELECT); }
"unreachable"         { return TOK(UNREACHABLE); }
"memory_size"         { return TOK(MEMORY_SIZE); }
"grow_memory"         { return TOK(GROW_MEMORY); }
"has_feature"         { return TOK(HAS_FEATURE); }
"type"                { return TOK(TYPE); }
"func"                { return TOK(FUNC); }
"param"               { return TOK(PARAM); }
"result"              { return TOK(RESULT); }
"local"               { return TOK(LOCAL); }
"module"              { return TOK(MODULE); }
"memory"              { return TOK(MEMORY); }
"segment"             { return TOK(SEGMENT); }
"import"              { return TOK(IMPORT); }
"export"              { return TOK(EXPORT); }
"table"               { return TOK(TABLE); }
"assert_invalid"      { return TOK(ASSERT_INVALID); }
"assert_return"       { return TOK(ASSERT_RETURN); }
"assert_return_nan"   { return TOK(ASSERT_RETURN_NAN); }
"assert_trap"         { return TOK(ASSERT_TRAP); }
"invoke"              { return TOK(INVOKE); }
{NAME}                { return TOK(VAR); }

";;"                  { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\n      { COLUMN(yyscanner) = 1; BEGIN(INITIAL); }
<LINE_COMMENT><<EOF>> { return TOK(EOF); }
<LINE_COMMENT>.
"(;"                  { BEGIN(BLOCK_COMMENT); COMMENT_NESTING(yyscanner) = 1; }
<BLOCK_COMMENT>"(;"   { COMMENT_NESTING(yyscanner)++; }
<BLOCK_COMMENT>";)"   { if (--COMMENT_NESTING(yyscanner) == 0) BEGIN(INITIAL); }
<BLOCK_COMMENT>\n     { COLUMN(yyscanner) = 1; }
<BLOCK_COMMENT><<EOF>> { yyerror(yylloc, &yyscanner, "unexpected EOF"); }
<BLOCK_COMMENT>.
\n                    { COLUMN(yyscanner) = 1; }
[ \t\r]
<<EOF>>               { return TOK(EOF); }
{ATOM}                { yyerror(yylloc, &yyscanner, "unexpected token \"%.*s\"",
                                yyleng, yytext); }
.                     { yyerror(yylloc, &yyscanner, "unexpected char"); }

%%

int yyparse(void* scanner);

int main(int argc, char **argv) {
  ++argv, --argc; /* skip over program name */
  yyscan_t scanner;
  WasmScannerExtra extra = {};
  extra.column = 1;

#if 0
  yydebug = 1;
#endif

  yylex_init(&scanner);
  if (argc > 0)
    yyset_in(fopen(argv[0], "r"), scanner);
  else
    yyset_in(stdin, scanner);
  yyset_extra(&extra, scanner);
  yyparse(scanner);
  yylex_destroy(scanner);
}
