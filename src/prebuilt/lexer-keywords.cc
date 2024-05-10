/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -m 50 -L C++ -N InWordSet -E -t -c --output-file=./prebuilt/lexer-keywords.cc ./lexer-keywords.txt  */
/* Computed positions: -k'1,3,5-8,10,12,15,17-19,23,27,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "./lexer-keywords.txt"
struct TokenInfo {
  TokenInfo(const char* name) : name(name) {}
  TokenInfo(const char* name, TokenType token_type)
      : name(name), token_type(token_type) {}
  TokenInfo(const char* name, Type value_type)
      : name(name), token_type(TokenType::ValueType), value_type(value_type) {}
  TokenInfo(const char* name, Type value_type, TokenType token_type)
      : name(name), token_type(token_type), value_type(value_type) {}
  TokenInfo(const char* name, TokenType token_type, Opcode opcode)
      : name(name), token_type(token_type), opcode(opcode) {}

  const char* name;
  TokenType token_type;
  union {
    Type value_type;
    Opcode opcode;
  };
};;
/* maximum key range = 2998, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct TokenInfo *InWordSet (const char *str, size_t len);
};

inline unsigned int
Perfect_Hash::hash (const char *str, size_t len)
{
  static unsigned short asso_values[] =
    {
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011,    2, 3011, 3011,  700,
       359,    1,   30,    0,  346,  541,  246, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011,  261,    0,    3,  217,   66,
        64,    6,  832,    4,  647,  235,    0,    2,   20,  116,
        53,   80,  753,  703,   19,    2,    0,    0,  390,  383,
       328,   83,  314, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011, 3011,
      3011, 3011, 3011, 3011, 3011, 3011, 3011
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[26])];
      /*FALLTHROUGH*/
      case 26:
      case 25:
      case 24:
      case 23:
        hval += asso_values[static_cast<unsigned char>(str[22])];
      /*FALLTHROUGH*/
      case 22:
      case 21:
      case 20:
      case 19:
        hval += asso_values[static_cast<unsigned char>(str[18])];
      /*FALLTHROUGH*/
      case 18:
        hval += asso_values[static_cast<unsigned char>(str[17])];
      /*FALLTHROUGH*/
      case 17:
        hval += asso_values[static_cast<unsigned char>(str[16])];
      /*FALLTHROUGH*/
      case 16:
      case 15:
        hval += asso_values[static_cast<unsigned char>(str[14])];
      /*FALLTHROUGH*/
      case 14:
      case 13:
      case 12:
        hval += asso_values[static_cast<unsigned char>(str[11])];
      /*FALLTHROUGH*/
      case 11:
      case 10:
        hval += asso_values[static_cast<unsigned char>(str[9])];
      /*FALLTHROUGH*/
      case 9:
      case 8:
        hval += asso_values[static_cast<unsigned char>(str[7])];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[static_cast<unsigned char>(str[6])];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[static_cast<unsigned char>(str[5])];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[static_cast<unsigned char>(str[4])];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2]+1)];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0]+1)];
        break;
    }
  return hval + asso_values[static_cast<unsigned char>(str[len - 1])];
}

struct TokenInfo *
Perfect_Hash::InWordSet (const char *str, size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 636,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 32,
      MIN_HASH_VALUE = 13,
      MAX_HASH_VALUE = 3010
    };

  static struct TokenInfo wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 45 "./lexer-keywords.txt"
      {"data", TokenType::Data},
#line 134 "./lexer-keywords.txt"
      {"f64.gt", TokenType::Compare, Opcode::F64Gt},
#line 73 "./lexer-keywords.txt"
      {"f32.gt", TokenType::Compare, Opcode::F32Gt},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 608 "./lexer-keywords.txt"
      {"ref.test", TokenType::RefTest},
      {""}, {""}, {""},
#line 133 "./lexer-keywords.txt"
      {"f64.ge", TokenType::Compare, Opcode::F64Ge},
#line 72 "./lexer-keywords.txt"
      {"f32.ge", TokenType::Compare, Opcode::F32Ge},
#line 578 "./lexer-keywords.txt"
      {"result", TokenType::Result},
      {""},
#line 137 "./lexer-keywords.txt"
      {"f64.lt", TokenType::Compare, Opcode::F64Lt},
#line 76 "./lexer-keywords.txt"
      {"f32.lt", TokenType::Compare, Opcode::F32Lt},
      {""},
#line 470 "./lexer-keywords.txt"
      {"i64", Type::I64},
      {""}, {""}, {""},
#line 150 "./lexer-keywords.txt"
      {"f64", Type::F64},
      {""}, {""}, {""}, {""},
#line 135 "./lexer-keywords.txt"
      {"f64.le", TokenType::Compare, Opcode::F64Le},
#line 74 "./lexer-keywords.txt"
      {"f32.le", TokenType::Compare, Opcode::F32Le},
      {""}, {""}, {""},
#line 338 "./lexer-keywords.txt"
      {"i32x4.gt_u", TokenType::Compare, Opcode::I32X4GtU},
      {""},
#line 99 "./lexer-keywords.txt"
      {"f32x4.gt", TokenType::Compare, Opcode::F32X4Gt},
      {""},
#line 337 "./lexer-keywords.txt"
      {"i32x4.gt_s", TokenType::Compare, Opcode::I32X4GtS},
      {""},
#line 336 "./lexer-keywords.txt"
      {"i32x4.ge_u", TokenType::Compare, Opcode::I32X4GeU},
      {""}, {""},
#line 561 "./lexer-keywords.txt"
      {"mut", TokenType::Mut},
#line 335 "./lexer-keywords.txt"
      {"i32x4.ge_s", TokenType::Compare, Opcode::I32X4GeS},
      {""}, {""}, {""},
#line 98 "./lexer-keywords.txt"
      {"f32x4.ge", TokenType::Compare, Opcode::F32X4Ge},
      {""},
#line 348 "./lexer-keywords.txt"
      {"i32x4.lt_u", TokenType::Compare, Opcode::I32X4LtU},
      {""},
#line 101 "./lexer-keywords.txt"
      {"f32x4.lt", TokenType::Compare, Opcode::F32X4Lt},
#line 370 "./lexer-keywords.txt"
      {"i32x4", TokenType::I32X4},
#line 347 "./lexer-keywords.txt"
      {"i32x4.lt_s", TokenType::Compare, Opcode::I32X4LtS},
      {""},
#line 340 "./lexer-keywords.txt"
      {"i32x4.le_u", TokenType::Compare, Opcode::I32X4LeU},
#line 120 "./lexer-keywords.txt"
      {"f32x4", TokenType::F32X4},
#line 446 "./lexer-keywords.txt"
      {"i64.ne", TokenType::Compare, Opcode::I64Ne},
#line 297 "./lexer-keywords.txt"
      {"i32.ne", TokenType::Compare, Opcode::I32Ne},
#line 339 "./lexer-keywords.txt"
      {"i32x4.le_s", TokenType::Compare, Opcode::I32X4LeS},
#line 586 "./lexer-keywords.txt"
      {"struct", Type::Struct, TokenType::Struct},
#line 143 "./lexer-keywords.txt"
      {"f64.ne", TokenType::Compare, Opcode::F64Ne},
#line 82 "./lexer-keywords.txt"
      {"f32.ne", TokenType::Compare, Opcode::F32Ne},
#line 100 "./lexer-keywords.txt"
      {"f32x4.le", TokenType::Compare, Opcode::F32X4Le},
#line 142 "./lexer-keywords.txt"
      {"f64.neg", TokenType::Unary, Opcode::F64Neg},
#line 81 "./lexer-keywords.txt"
      {"f32.neg", TokenType::Unary, Opcode::F32Neg},
#line 43 "./lexer-keywords.txt"
      {"code", TokenType::Code},
      {""},
#line 592 "./lexer-keywords.txt"
      {"struct.set", TokenType::StructSet, Opcode::StructSet                           },
#line 623 "./lexer-keywords.txt"
      {"table", TokenType::Table},
#line 589 "./lexer-keywords.txt"
      {"struct.get", TokenType::StructGet, Opcode::StructGet},
#line 607 "./lexer-keywords.txt"
      {"ref.cast", TokenType::RefCast},
#line 591 "./lexer-keywords.txt"
      {"struct.get_u",TokenType::StructGetU, Opcode::StructGetU},
#line 37 "./lexer-keywords.txt"
      {"br", TokenType::Br, Opcode::Br},
#line 48 "./lexer-keywords.txt"
      {"do", TokenType::Do},
      {""},
#line 590 "./lexer-keywords.txt"
      {"struct.get_s",TokenType::StructGetS, Opcode::StructGetS},
#line 621 "./lexer-keywords.txt"
      {"table.set", TokenType::TableSet, Opcode::TableSet},
      {""},
#line 618 "./lexer-keywords.txt"
      {"table.get", TokenType::TableGet, Opcode::TableGet},
      {""}, {""}, {""},
#line 560 "./lexer-keywords.txt"
      {"module", TokenType::Module},
#line 141 "./lexer-keywords.txt"
      {"f64.nearest", TokenType::Unary, Opcode::F64Nearest},
#line 80 "./lexer-keywords.txt"
      {"f32.nearest", TokenType::Unary, Opcode::F32Nearest},
      {""}, {""}, {""}, {""}, {""},
#line 355 "./lexer-keywords.txt"
      {"i32x4.neg", TokenType::Unary, Opcode::I32X4Neg},
#line 356 "./lexer-keywords.txt"
      {"i32x4.ne", TokenType::Compare, Opcode::I32X4Ne},
#line 32 "./lexer-keywords.txt"
      {"before", TokenType::Before},
      {""},
#line 106 "./lexer-keywords.txt"
      {"f32x4.neg", TokenType::Unary, Opcode::F32X4Neg},
#line 107 "./lexer-keywords.txt"
      {"f32x4.ne", TokenType::Compare, Opcode::F32X4Ne},
#line 46 "./lexer-keywords.txt"
      {"declare", TokenType::Declare},
      {""}, {""}, {""}, {""},
#line 460 "./lexer-keywords.txt"
      {"i64.store", TokenType::Store, Opcode::I64Store},
#line 310 "./lexer-keywords.txt"
      {"i32.store", TokenType::Store, Opcode::I32Store},
      {""}, {""},
#line 147 "./lexer-keywords.txt"
      {"f64.store", TokenType::Store, Opcode::F64Store},
#line 85 "./lexer-keywords.txt"
      {"f32.store", TokenType::Store, Opcode::F32Store},
      {""}, {""},
#line 447 "./lexer-keywords.txt"
      {"i64.or", TokenType::Binary, Opcode::I64Or},
#line 298 "./lexer-keywords.txt"
      {"i32.or", TokenType::Binary, Opcode::I32Or},
      {""},
#line 576 "./lexer-keywords.txt"
      {"ref.null", TokenType::RefNull, Opcode::RefNull},
      {""}, {""},
#line 105 "./lexer-keywords.txt"
      {"f32x4.nearest", TokenType::Unary, Opcode::F32X4Nearest},
      {""}, {""},
#line 582 "./lexer-keywords.txt"
      {"return", TokenType::Return, Opcode::Return},
      {""},
#line 22 "./lexer-keywords.txt"
      {"rec", TokenType::Rec},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 453 "./lexer-keywords.txt"
      {"i64.rotr", TokenType::Binary, Opcode::I64Rotr},
#line 304 "./lexer-keywords.txt"
      {"i32.rotr", TokenType::Binary, Opcode::I32Rotr},
#line 452 "./lexer-keywords.txt"
      {"i64.rotl", TokenType::Binary, Opcode::I64Rotl},
#line 303 "./lexer-keywords.txt"
      {"i32.rotl", TokenType::Binary, Opcode::I32Rotl},
#line 47 "./lexer-keywords.txt"
      {"delegate", TokenType::Delegate},
      {""},
#line 149 "./lexer-keywords.txt"
      {"f64.trunc", TokenType::Unary, Opcode::F64Trunc},
#line 87 "./lexer-keywords.txt"
      {"f32.trunc", TokenType::Unary, Opcode::F32Trunc},
      {""},
#line 186 "./lexer-keywords.txt"
      {"func", Type::FuncRef, TokenType::Func},
      {""}, {""}, {""}, {""},
#line 91 "./lexer-keywords.txt"
      {"f32x4.ceil", TokenType::Unary, Opcode::F32X4Ceil},
      {""}, {""}, {""},
#line 445 "./lexer-keywords.txt"
      {"i64.mul", TokenType::Binary, Opcode::I64Mul},
#line 296 "./lexer-keywords.txt"
      {"i32.mul", TokenType::Binary, Opcode::I32Mul},
      {""}, {""},
#line 140 "./lexer-keywords.txt"
      {"f64.mul", TokenType::Binary, Opcode::F64Mul},
#line 79 "./lexer-keywords.txt"
      {"f32.mul", TokenType::Binary, Opcode::F32Mul},
      {""},
#line 187 "./lexer-keywords.txt"
      {"none", Type::NoneRef, TokenType::None},
      {""}, {""},
#line 329 "./lexer-keywords.txt"
      {"i32x4.add", TokenType::Binary, Opcode::I32X4Add},
      {""}, {""}, {""},
#line 90 "./lexer-keywords.txt"
      {"f32x4.add", TokenType::Binary, Opcode::F32X4Add},
#line 354 "./lexer-keywords.txt"
      {"i32x4.mul", TokenType::Binary, Opcode::I32X4Mul},
      {""}, {""}, {""},
#line 104 "./lexer-keywords.txt"
      {"f32x4.mul", TokenType::Binary, Opcode::F32X4Mul},
      {""}, {""}, {""},
#line 118 "./lexer-keywords.txt"
      {"f32x4.trunc", TokenType::Unary, Opcode::F32X4Trunc},
      {""},
#line 583 "./lexer-keywords.txt"
      {"select", TokenType::Select, Opcode::Select},
      {""}, {""},
#line 381 "./lexer-keywords.txt"
      {"i64.and", TokenType::Binary, Opcode::I64And},
#line 246 "./lexer-keywords.txt"
      {"i32.and", TokenType::Binary, Opcode::I32And},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 380 "./lexer-keywords.txt"
      {"i64.add", TokenType::Binary, Opcode::I64Add},
#line 245 "./lexer-keywords.txt"
      {"i32.add", TokenType::Binary, Opcode::I32Add},
#line 40 "./lexer-keywords.txt"
      {"call", TokenType::Call, Opcode::Call},
      {""},
#line 122 "./lexer-keywords.txt"
      {"f64.add", TokenType::Binary, Opcode::F64Add},
#line 60 "./lexer-keywords.txt"
      {"f32.add", TokenType::Binary, Opcode::F32Add},
      {""}, {""},
#line 419 "./lexer-keywords.txt"
      {"i64.const", TokenType::Const, Opcode::I64Const},
#line 275 "./lexer-keywords.txt"
      {"i32.const", TokenType::Const, Opcode::I32Const},
      {""}, {""},
#line 124 "./lexer-keywords.txt"
      {"f64.const", TokenType::Const, Opcode::F64Const},
#line 62 "./lexer-keywords.txt"
      {"f32.const", TokenType::Const, Opcode::F32Const},
      {""}, {""}, {""},
#line 547 "./lexer-keywords.txt"
      {"local.set", TokenType::LocalSet, Opcode::LocalSet},
#line 569 "./lexer-keywords.txt"
      {"null", Type::Null, TokenType::Null},
#line 546 "./lexer-keywords.txt"
      {"local.get", TokenType::LocalGet, Opcode::LocalGet},
#line 585 "./lexer-keywords.txt"
      {"start", TokenType::Start},
#line 548 "./lexer-keywords.txt"
      {"local.tee", TokenType::LocalTee, Opcode::LocalTee},
      {""},
#line 549 "./lexer-keywords.txt"
      {"local", TokenType::Local},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 121 "./lexer-keywords.txt"
      {"f64.abs", TokenType::Unary, Opcode::F64Abs},
#line 59 "./lexer-keywords.txt"
      {"f32.abs", TokenType::Unary, Opcode::F32Abs},
      {""},
#line 357 "./lexer-keywords.txt"
      {"i32x4.relaxed_laneselect", TokenType::Ternary, Opcode::I32X4RelaxedLaneSelect},
#line 442 "./lexer-keywords.txt"
      {"i64.load", TokenType::Load, Opcode::I64Load},
#line 293 "./lexer-keywords.txt"
      {"i32.load", TokenType::Load, Opcode::I32Load},
      {""}, {""},
#line 136 "./lexer-keywords.txt"
      {"f64.load", TokenType::Load, Opcode::F64Load},
#line 75 "./lexer-keywords.txt"
      {"f32.load", TokenType::Load, Opcode::F32Load},
      {""}, {""},
#line 358 "./lexer-keywords.txt"
      {"i32x4.replace_lane", TokenType::SimdLaneOp, Opcode::I32X4ReplaceLane},
#line 321 "./lexer-keywords.txt"
      {"i8", Type::I8},
      {""}, {""},
#line 114 "./lexer-keywords.txt"
      {"f32x4.replace_lane", TokenType::SimdLaneOp, Opcode::F32X4ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 23 "./lexer-keywords.txt"
      {"after", TokenType::After},
#line 363 "./lexer-keywords.txt"
      {"i32x4.sub", TokenType::Binary, Opcode::I32X4Sub},
      {""}, {""},
#line 328 "./lexer-keywords.txt"
      {"i32x4.abs", TokenType::Unary, Opcode::I32X4Abs},
#line 117 "./lexer-keywords.txt"
      {"f32x4.sub", TokenType::Binary, Opcode::F32X4Sub},
      {""}, {""},
#line 89 "./lexer-keywords.txt"
      {"f32x4.abs", TokenType::Unary, Opcode::F32X4Abs},
      {""}, {""}, {""}, {""},
#line 433 "./lexer-keywords.txt"
      {"i64.gt_u", TokenType::Compare, Opcode::I64GtU},
#line 286 "./lexer-keywords.txt"
      {"i32.gt_u", TokenType::Compare, Opcode::I32GtU},
#line 325 "./lexer-keywords.txt"
      {"nofunc", Type::NoFunc, TokenType::NoFunc},
      {""},
#line 432 "./lexer-keywords.txt"
      {"i64.gt_s", TokenType::Compare, Opcode::I64GtS},
#line 285 "./lexer-keywords.txt"
      {"i32.gt_s", TokenType::Compare, Opcode::I32GtS},
#line 431 "./lexer-keywords.txt"
      {"i64.ge_u", TokenType::Compare, Opcode::I64GeU},
#line 284 "./lexer-keywords.txt"
      {"i32.ge_u", TokenType::Compare, Opcode::I32GeU},
      {""}, {""},
#line 430 "./lexer-keywords.txt"
      {"i64.ge_s", TokenType::Compare, Opcode::I64GeS},
#line 283 "./lexer-keywords.txt"
      {"i32.ge_s", TokenType::Compare, Opcode::I32GeS},
      {""},
#line 21 "./lexer-keywords.txt"
      {"sub", TokenType::Sub},
      {""}, {""},
#line 444 "./lexer-keywords.txt"
      {"i64.lt_u", TokenType::Compare, Opcode::I64LtU},
#line 295 "./lexer-keywords.txt"
      {"i32.lt_u", TokenType::Compare, Opcode::I32LtU},
      {""}, {""},
#line 443 "./lexer-keywords.txt"
      {"i64.lt_s", TokenType::Compare, Opcode::I64LtS},
#line 294 "./lexer-keywords.txt"
      {"i32.lt_s", TokenType::Compare, Opcode::I32LtS},
#line 435 "./lexer-keywords.txt"
      {"i64.le_u", TokenType::Compare, Opcode::I64LeU},
#line 288 "./lexer-keywords.txt"
      {"i32.le_u", TokenType::Compare, Opcode::I32LeU},
#line 559 "./lexer-keywords.txt"
      {"memory", TokenType::Memory},
      {""},
#line 434 "./lexer-keywords.txt"
      {"i64.le_s", TokenType::Compare, Opcode::I64LeS},
#line 287 "./lexer-keywords.txt"
      {"i32.le_s", TokenType::Compare, Opcode::I32LeS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 417 "./lexer-keywords.txt"
      {"i64.atomic.store", TokenType::AtomicStore, Opcode::I64AtomicStore},
#line 273 "./lexer-keywords.txt"
      {"i32.atomic.store", TokenType::AtomicStore, Opcode::I32AtomicStore},
#line 112 "./lexer-keywords.txt"
      {"f32x4.relaxed_min", TokenType::Binary, Opcode::F32X4RelaxedMin},
#line 397 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32SubU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 601 "./lexer-keywords.txt"
      {"array.set", TokenType::ArraySet, Opcode::ArraySet                                                                          },
      {""},
#line 598 "./lexer-keywords.txt"
      {"array.get", TokenType::ArrayGet, Opcode::ArrayGet                                                                          },
      {""}, {""},
#line 36 "./lexer-keywords.txt"
      {"br_table", TokenType::BrTable, Opcode::BrTable},
      {""},
#line 344 "./lexer-keywords.txt"
      {"i32x4.relaxed_trunc_f64x2_u_zero", TokenType::Unary, Opcode::I32X4RelaxedTruncF64X2UZero},
      {""},
#line 343 "./lexer-keywords.txt"
      {"i32x4.relaxed_trunc_f64x2_s_zero", TokenType::Unary, Opcode::I32X4RelaxedTruncF64X2SZero},
      {""}, {""},
#line 622 "./lexer-keywords.txt"
      {"table.size", TokenType::TableSize, Opcode::TableSize},
      {""}, {""}, {""},
#line 33 "./lexer-keywords.txt"
      {"binary", TokenType::Bin},
#line 113 "./lexer-keywords.txt"
      {"f32x4.relaxed_nmadd", TokenType::Ternary, Opcode::F32X4RelaxedNmadd},
      {""}, {""},
#line 410 "./lexer-keywords.txt"
      {"i64.atomic.rmw.or", TokenType::AtomicRmw, Opcode::I64AtomicRmwOr},
#line 267 "./lexer-keywords.txt"
      {"i32.atomic.rmw.or", TokenType::AtomicRmw, Opcode::I32AtomicRmwOr},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 463 "./lexer-keywords.txt"
      {"i64.trunc_f32_u", TokenType::Convert, Opcode::I64TruncF32U},
#line 313 "./lexer-keywords.txt"
      {"i32.trunc_f32_u", TokenType::Convert, Opcode::I32TruncF32U},
      {""}, {""},
#line 462 "./lexer-keywords.txt"
      {"i64.trunc_f32_s", TokenType::Convert, Opcode::I64TruncF32S},
#line 312 "./lexer-keywords.txt"
      {"i32.trunc_f32_s", TokenType::Convert, Opcode::I32TruncF32S},
      {""},
#line 330 "./lexer-keywords.txt"
      {"i32x4.all_true", TokenType::Unary, Opcode::I32X4AllTrue},
#line 584 "./lexer-keywords.txt"
      {"shared", TokenType::Shared},
      {""},
#line 123 "./lexer-keywords.txt"
      {"f64.ceil", TokenType::Unary, Opcode::F64Ceil},
#line 61 "./lexer-keywords.txt"
      {"f32.ceil", TokenType::Unary, Opcode::F32Ceil},
      {""}, {""},
#line 320 "./lexer-keywords.txt"
      {"i32", Type::I32},
      {""},
#line 394 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32AndU},
      {""},
#line 88 "./lexer-keywords.txt"
      {"f32", Type::F32},
      {""}, {""}, {""}, {""},
#line 620 "./lexer-keywords.txt"
      {"table.init", TokenType::TableInit, Opcode::TableInit},
      {""}, {""}, {""},
#line 393 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32AddU},
#line 159 "./lexer-keywords.txt"
      {"f64x2.gt", TokenType::Compare, Opcode::F64X2Gt},
      {""},
#line 479 "./lexer-keywords.txt"
      {"i64x2.gt_s", TokenType::Binary, Opcode::I64X2GtS},
      {""}, {""}, {""}, {""}, {""},
#line 481 "./lexer-keywords.txt"
      {"i64x2.ge_s", TokenType::Binary, Opcode::I64X2GeS},
      {""}, {""}, {""},
#line 158 "./lexer-keywords.txt"
      {"f64x2.ge", TokenType::Compare, Opcode::F64X2Ge},
#line 20 "./lexer-keywords.txt"
      {"array", Type::Array, TokenType::Array},
      {""},
#line 602 "./lexer-keywords.txt"
      {"array.len", TokenType::ArrayLen, Opcode::ArrayLen                                                                          },
#line 161 "./lexer-keywords.txt"
      {"f64x2.lt", TokenType::Compare, Opcode::F64X2Lt},
      {""},
#line 478 "./lexer-keywords.txt"
      {"i64x2.lt_s", TokenType::Binary, Opcode::I64X2LtS},
#line 616 "./lexer-keywords.txt"
      {"table.copy", TokenType::TableCopy, Opcode::TableCopy},
#line 110 "./lexer-keywords.txt"
      {"f32x4.relaxed_madd", TokenType::Ternary, Opcode::F32X4RelaxedMadd},
      {""}, {""},
#line 626 "./lexer-keywords.txt"
      {"try", TokenType::Try, Opcode::Try},
#line 480 "./lexer-keywords.txt"
      {"i64x2.le_s", TokenType::Binary, Opcode::I64X2LeS},
      {""},
#line 544 "./lexer-keywords.txt"
      {"invoke", TokenType::Invoke},
      {""},
#line 160 "./lexer-keywords.txt"
      {"f64x2.le", TokenType::Compare, Opcode::F64X2Le},
      {""},
#line 396 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32OrU},
      {""}, {""}, {""},
#line 451 "./lexer-keywords.txt"
      {"i64.rem_u", TokenType::Binary, Opcode::I64RemU},
#line 302 "./lexer-keywords.txt"
      {"i32.rem_u", TokenType::Binary, Opcode::I32RemU},
#line 450 "./lexer-keywords.txt"
      {"i64.rem_s", TokenType::Binary, Opcode::I64RemS},
#line 301 "./lexer-keywords.txt"
      {"i32.rem_s", TokenType::Binary, Opcode::I32RemS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 413 "./lexer-keywords.txt"
      {"i64.atomic.rmw.xor", TokenType::AtomicRmw, Opcode::I64AtomicRmwXor},
#line 270 "./lexer-keywords.txt"
      {"i32.atomic.rmw.xor", TokenType::AtomicRmw, Opcode::I32AtomicRmwXor},
#line 350 "./lexer-keywords.txt"
      {"i32x4.max_u", TokenType::Binary, Opcode::I32X4MaxU},
      {""},
#line 349 "./lexer-keywords.txt"
      {"i32x4.max_s", TokenType::Binary, Opcode::I32X4MaxS},
      {""},
#line 385 "./lexer-keywords.txt"
      {"i64.atomic.load", TokenType::AtomicLoad, Opcode::I64AtomicLoad},
#line 249 "./lexer-keywords.txt"
      {"i32.atomic.load", TokenType::AtomicLoad, Opcode::I32AtomicLoad},
      {""}, {""}, {""},
#line 483 "./lexer-keywords.txt"
      {"i64x2.neg", TokenType::Unary, Opcode::I64X2Neg},
#line 477 "./lexer-keywords.txt"
      {"i64x2.ne", TokenType::Binary, Opcode::I64X2Ne},
      {""}, {""},
#line 166 "./lexer-keywords.txt"
      {"f64x2.neg", TokenType::Unary, Opcode::F64X2Neg},
#line 167 "./lexer-keywords.txt"
      {"f64x2.ne", TokenType::Compare, Opcode::F64X2Ne},
#line 441 "./lexer-keywords.txt"
      {"i64.load8_u", TokenType::Load, Opcode::I64Load8U},
#line 292 "./lexer-keywords.txt"
      {"i32.load8_u", TokenType::Load, Opcode::I32Load8U},
#line 440 "./lexer-keywords.txt"
      {"i64.load8_s", TokenType::Load, Opcode::I64Load8S},
#line 291 "./lexer-keywords.txt"
      {"i32.load8_s", TokenType::Load, Opcode::I32Load8S},
#line 461 "./lexer-keywords.txt"
      {"i64.sub", TokenType::Binary, Opcode::I64Sub},
#line 311 "./lexer-keywords.txt"
      {"i32.sub", TokenType::Binary, Opcode::I32Sub},
#line 516 "./lexer-keywords.txt"
      {"i8x16.gt_u", TokenType::Compare, Opcode::I8X16GtU},
      {""},
#line 148 "./lexer-keywords.txt"
      {"f64.sub", TokenType::Binary, Opcode::F64Sub},
#line 86 "./lexer-keywords.txt"
      {"f32.sub", TokenType::Binary, Opcode::F32Sub},
#line 515 "./lexer-keywords.txt"
      {"i8x16.gt_s", TokenType::Compare, Opcode::I8X16GtS},
#line 103 "./lexer-keywords.txt"
      {"f32x4.min", TokenType::Binary, Opcode::F32X4Min},
#line 514 "./lexer-keywords.txt"
      {"i8x16.ge_u", TokenType::Compare, Opcode::I8X16GeU},
#line 581 "./lexer-keywords.txt"
      {"return_call", TokenType::ReturnCall, Opcode::ReturnCall},
#line 502 "./lexer-keywords.txt"
      {"i64.xor", TokenType::Binary, Opcode::I64Xor},
#line 379 "./lexer-keywords.txt"
      {"i32.xor", TokenType::Binary, Opcode::I32Xor},
#line 513 "./lexer-keywords.txt"
      {"i8x16.ge_s", TokenType::Compare, Opcode::I8X16GeS},
      {""}, {""},
#line 165 "./lexer-keywords.txt"
      {"f64x2.nearest", TokenType::Unary, Opcode::F64X2Nearest},
#line 609 "./lexer-keywords.txt"
      {"br_on_cast", TokenType::BrOnCast, Opcode::BrOnCast},
      {""},
#line 520 "./lexer-keywords.txt"
      {"i8x16.lt_u", TokenType::Compare, Opcode::I8X16LtU},
#line 572 "./lexer-keywords.txt"
      {"ref.extern", TokenType::RefExtern},
      {""}, {""},
#line 519 "./lexer-keywords.txt"
      {"i8x16.lt_s", TokenType::Compare, Opcode::I8X16LtS},
      {""},
#line 518 "./lexer-keywords.txt"
      {"i8x16.le_u", TokenType::Compare, Opcode::I8X16LeU},
#line 139 "./lexer-keywords.txt"
      {"f64.min", TokenType::Binary, Opcode::F64Min},
#line 78 "./lexer-keywords.txt"
      {"f32.min", TokenType::Binary, Opcode::F32Min},
      {""},
#line 517 "./lexer-keywords.txt"
      {"i8x16.le_s", TokenType::Compare, Opcode::I8X16LeS},
#line 458 "./lexer-keywords.txt"
      {"i64.store32", TokenType::Store, Opcode::I64Store32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 408 "./lexer-keywords.txt"
      {"i64.atomic.rmw.and", TokenType::AtomicRmw, Opcode::I64AtomicRmwAnd},
#line 265 "./lexer-keywords.txt"
      {"i32.atomic.rmw.and", TokenType::AtomicRmw, Opcode::I32AtomicRmwAnd},
#line 153 "./lexer-keywords.txt"
      {"f64x2.ceil", TokenType::Unary, Opcode::F64X2Ceil},
      {""}, {""},
#line 639 "./lexer-keywords.txt"
      {"v128.store", TokenType::Store, Opcode::V128Store},
      {""}, {""},
#line 102 "./lexer-keywords.txt"
      {"f32x4.max", TokenType::Binary, Opcode::F32X4Max},
      {""}, {""},
#line 407 "./lexer-keywords.txt"
      {"i64.atomic.rmw.add", TokenType::AtomicRmw, Opcode::I64AtomicRmwAdd},
#line 264 "./lexer-keywords.txt"
      {"i32.atomic.rmw.add", TokenType::AtomicRmw, Opcode::I32AtomicRmwAdd},
      {""}, {""}, {""},
#line 471 "./lexer-keywords.txt"
      {"i64x2.add", TokenType::Binary, Opcode::I64X2Add},
      {""},
#line 527 "./lexer-keywords.txt"
      {"i8x16.neg", TokenType::Unary, Opcode::I8X16Neg},
#line 529 "./lexer-keywords.txt"
      {"i8x16.ne", TokenType::Compare, Opcode::I8X16Ne},
#line 152 "./lexer-keywords.txt"
      {"f64x2.add", TokenType::Binary, Opcode::F64X2Add},
#line 475 "./lexer-keywords.txt"
      {"i64x2.mul", TokenType::Binary, Opcode::I64X2Mul},
      {""}, {""}, {""},
#line 164 "./lexer-keywords.txt"
      {"f64x2.mul", TokenType::Binary, Opcode::F64X2Mul},
#line 635 "./lexer-keywords.txt"
      {"v128.or", TokenType::Binary, Opcode::V128Or},
      {""},
#line 188 "./lexer-keywords.txt"
      {"function", TokenType::Function},
#line 178 "./lexer-keywords.txt"
      {"f64x2.trunc", TokenType::Unary, Opcode::F64X2Trunc},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 437 "./lexer-keywords.txt"
      {"i64.load16_u", TokenType::Load, Opcode::I64Load16U},
#line 290 "./lexer-keywords.txt"
      {"i32.load16_u", TokenType::Load, Opcode::I32Load16U},
#line 634 "./lexer-keywords.txt"
      {"v128.not", TokenType::Unary, Opcode::V128Not},
      {""},
#line 436 "./lexer-keywords.txt"
      {"i64.load16_s", TokenType::Load, Opcode::I64Load16S},
#line 289 "./lexer-keywords.txt"
      {"i32.load16_s", TokenType::Load, Opcode::I32Load16S},
      {""}, {""}, {""}, {""}, {""},
#line 653 "./lexer-keywords.txt"
      {"v128.store64_lane", TokenType::SimdStoreLane, Opcode::V128Store64Lane},
      {""},
#line 439 "./lexer-keywords.txt"
      {"i64.load32_u", TokenType::Load, Opcode::I64Load32U},
      {""}, {""}, {""},
#line 438 "./lexer-keywords.txt"
      {"i64.load32_s", TokenType::Load, Opcode::I64Load32S},
#line 558 "./lexer-keywords.txt"
      {"memory.size", TokenType::MemorySize, Opcode::MemorySize},
#line 636 "./lexer-keywords.txt"
      {"v128.any_true", TokenType::Unary, Opcode::V128AnyTrue},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 334 "./lexer-keywords.txt"
      {"i32x4.extract_lane", TokenType::SimdLaneOp, Opcode::I32X4ExtractLane},
      {""}, {""}, {""},
#line 96 "./lexer-keywords.txt"
      {"f32x4.extract_lane", TokenType::SimdLaneOp, Opcode::F32X4ExtractLane},
      {""}, {""},
#line 633 "./lexer-keywords.txt"
      {"v128.load", TokenType::Load, Opcode::V128Load},
      {""}, {""}, {""},
#line 490 "./lexer-keywords.txt"
      {"i64x2.relaxed_laneselect", TokenType::Ternary, Opcode::I64X2RelaxedLaneSelect},
      {""}, {""}, {""}, {""},
#line 506 "./lexer-keywords.txt"
      {"i8x16.add", TokenType::Binary, Opcode::I8X16Add},
#line 342 "./lexer-keywords.txt"
      {"i32x4.relaxed_trunc_f32x4_u", TokenType::Unary, Opcode::I32X4RelaxedTruncF32X4U},
      {""}, {""},
#line 491 "./lexer-keywords.txt"
      {"i64x2.replace_lane", TokenType::SimdLaneOp, Opcode::I64X2ReplaceLane},
#line 341 "./lexer-keywords.txt"
      {"i32x4.relaxed_trunc_f32x4_s", TokenType::Unary, Opcode::I32X4RelaxedTruncF32X4S},
      {""},
#line 630 "./lexer-keywords.txt"
      {"v128.and", TokenType::Binary, Opcode::V128And},
#line 174 "./lexer-keywords.txt"
      {"f64x2.replace_lane", TokenType::SimdLaneOp, Opcode::F64X2ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 600 "./lexer-keywords.txt"
      {"array.get_u", TokenType::ArrayGetU, Opcode::ArrayGetU                                                                        },
      {""},
#line 599 "./lexer-keywords.txt"
      {"array.get_s", TokenType::ArrayGetS, Opcode::ArrayGetS                                                                        },
#line 496 "./lexer-keywords.txt"
      {"i64x2.sub", TokenType::Binary, Opcode::I64X2Sub},
      {""}, {""},
#line 482 "./lexer-keywords.txt"
      {"i64x2.abs", TokenType::Unary, Opcode::I64X2Abs},
#line 177 "./lexer-keywords.txt"
      {"f64x2.sub", TokenType::Binary, Opcode::F64X2Sub},
      {""},
#line 632 "./lexer-keywords.txt"
      {"v128.const", TokenType::Const, Opcode::V128Const},
#line 151 "./lexer-keywords.txt"
      {"f64x2.abs", TokenType::Unary, Opcode::F64X2Abs},
#line 629 "./lexer-keywords.txt"
      {"v128.andnot", TokenType::Binary, Opcode::V128Andnot},
      {""}, {""}, {""},
#line 28 "./lexer-keywords.txt"
      {"assert_return", TokenType::AssertReturn},
      {""},
#line 459 "./lexer-keywords.txt"
      {"i64.store8", TokenType::Store, Opcode::I64Store8},
#line 309 "./lexer-keywords.txt"
      {"i32.store8", TokenType::Store, Opcode::I32Store8},
      {""}, {""}, {""},
#line 575 "./lexer-keywords.txt"
      {"ref.is_null", TokenType::RefIsNull, Opcode::RefIsNull},
      {""}, {""},
#line 588 "./lexer-keywords.txt"
      {"struct.new_default", TokenType::StructNewDefault, Opcode::StructNewDefault},
      {""},
#line 562 "./lexer-keywords.txt"
      {"nan:arithmetic", TokenType::NanArithmetic},
      {""}, {""}, {""},
#line 570 "./lexer-keywords.txt"
      {"any", Type::Any, TokenType::Any},
#line 331 "./lexer-keywords.txt"
      {"i32x4.bitmask", TokenType::Unary, Opcode::I32X4Bitmask},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 604 "./lexer-keywords.txt"
      {"array.copy", TokenType::ArrayCopy, Opcode::ArrayCopy                                                                        },
      {""},
#line 384 "./lexer-keywords.txt"
      {"i64.atomic.load8_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad8U},
#line 248 "./lexer-keywords.txt"
      {"i32.atomic.load8_u", TokenType::AtomicLoad, Opcode::I32AtomicLoad8U},
      {""}, {""}, {""},
#line 563 "./lexer-keywords.txt"
      {"nan:canonical", TokenType::NanCanonical},
#line 640 "./lexer-keywords.txt"
      {"v128", Type::V128},
      {""},
#line 531 "./lexer-keywords.txt"
      {"i8x16.relaxed_laneselect", TokenType::Ternary, Opcode::I8X16RelaxedLaneSelect},
      {""},
#line 172 "./lexer-keywords.txt"
      {"f64x2.relaxed_min", TokenType::Binary, Opcode::F64X2RelaxedMin},
#line 615 "./lexer-keywords.txt"
      {"i31.get_u", TokenType::I31GetU, Opcode::I31GetU},
      {""},
#line 614 "./lexer-keywords.txt"
      {"i31.get_s", TokenType::I31GetS, Opcode::I31GetS},
      {""}, {""},
#line 596 "./lexer-keywords.txt"
      {"array.new_data", TokenType::ArrayNewData, Opcode::ArrayNewData                                                                  },
#line 532 "./lexer-keywords.txt"
      {"i8x16.replace_lane", TokenType::SimdLaneOp, Opcode::I8X16ReplaceLane},
      {""},
#line 594 "./lexer-keywords.txt"
      {"array.new_default", TokenType::ArrayNewDefault, Opcode::ArrayNewDefault                                                            },
      {""}, {""},
#line 189 "./lexer-keywords.txt"
      {"get", TokenType::Get},
      {""}, {""}, {""},
#line 55 "./lexer-keywords.txt"
      {"tag", TokenType::Tag},
      {""},
#line 352 "./lexer-keywords.txt"
      {"i32x4.min_u", TokenType::Binary, Opcode::I32X4MinU},
      {""},
#line 351 "./lexer-keywords.txt"
      {"i32x4.min_s", TokenType::Binary, Opcode::I32X4MinS},
#line 539 "./lexer-keywords.txt"
      {"i8x16.sub", TokenType::Binary, Opcode::I8X16Sub},
      {""}, {""},
#line 503 "./lexer-keywords.txt"
      {"i8x16.abs", TokenType::Unary, Opcode::I8X16Abs},
      {""}, {""}, {""},
#line 173 "./lexer-keywords.txt"
      {"f64x2.relaxed_nmadd", TokenType::Ternary, Opcode::F64X2RelaxedNmadd},
      {""}, {""}, {""}, {""},
#line 401 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8AndU},
#line 258 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.and_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8AndU},
      {""}, {""}, {""}, {""},
#line 605 "./lexer-keywords.txt"
      {"array.init_data", TokenType::ArrayInitData, Opcode::ArrayInitData                                                                },
      {""}, {""},
#line 427 "./lexer-keywords.txt"
      {"i64.extend8_s", TokenType::Unary, Opcode::I64Extend8S},
#line 282 "./lexer-keywords.txt"
      {"i32.extend8_s", TokenType::Unary, Opcode::I32Extend8S},
#line 400 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8AddU},
#line 257 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.add_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8AddU},
#line 484 "./lexer-keywords.txt"
      {"i64x2.all_true", TokenType::Unary, Opcode::I64X2AllTrue},
      {""}, {""}, {""},
#line 556 "./lexer-keywords.txt"
      {"memory.grow", TokenType::MemoryGrow, Opcode::MemoryGrow},
      {""}, {""},
#line 557 "./lexer-keywords.txt"
      {"memory.init", TokenType::MemoryInit, Opcode::MemoryInit},
      {""}, {""},
#line 465 "./lexer-keywords.txt"
      {"i64.trunc_f64_u", TokenType::Convert, Opcode::I64TruncF64U},
#line 315 "./lexer-keywords.txt"
      {"i32.trunc_f64_u", TokenType::Convert, Opcode::I32TruncF64U},
#line 454 "./lexer-keywords.txt"
      {"i64.shl", TokenType::Binary, Opcode::I64Shl},
#line 305 "./lexer-keywords.txt"
      {"i32.shl", TokenType::Binary, Opcode::I32Shl},
#line 464 "./lexer-keywords.txt"
      {"i64.trunc_f64_s", TokenType::Convert, Opcode::I64TruncF64S},
#line 314 "./lexer-keywords.txt"
      {"i32.trunc_f64_s", TokenType::Convert, Opcode::I32TruncF64S},
      {""},
#line 420 "./lexer-keywords.txt"
      {"i64.ctz", TokenType::Unary, Opcode::I64Ctz},
#line 276 "./lexer-keywords.txt"
      {"i32.ctz", TokenType::Unary, Opcode::I32Ctz},
#line 577 "./lexer-keywords.txt"
      {"register", TokenType::Register},
#line 398 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32XchgU},
      {""}, {""}, {""},
#line 543 "./lexer-keywords.txt"
      {"input", TokenType::Input},
      {""}, {""},
#line 359 "./lexer-keywords.txt"
      {"i32x4.shl", TokenType::Binary, Opcode::I32X4Shl},
#line 538 "./lexer-keywords.txt"
      {"i8x16.sub_sat_u", TokenType::Binary, Opcode::I8X16SubSatU},
#line 627 "./lexer-keywords.txt"
      {"type", TokenType::Type},
      {""}, {""},
#line 537 "./lexer-keywords.txt"
      {"i8x16.sub_sat_s", TokenType::Binary, Opcode::I8X16SubSatS},
#line 399 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32XorU},
      {""}, {""}, {""},
#line 418 "./lexer-keywords.txt"
      {"i64.clz", TokenType::Unary, Opcode::I64Clz},
#line 274 "./lexer-keywords.txt"
      {"i32.clz", TokenType::Unary, Opcode::I32Clz},
#line 501 "./lexer-keywords.txt"
      {"i64x2", TokenType::I64X2},
      {""},
#line 170 "./lexer-keywords.txt"
      {"f64x2.relaxed_madd", TokenType::Ternary, Opcode::F64X2RelaxedMadd},
      {""},
#line 182 "./lexer-keywords.txt"
      {"f64x2", TokenType::F64X2},
#line 542 "./lexer-keywords.txt"
      {"import", TokenType::Import},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 94 "./lexer-keywords.txt"
      {"f32x4.div", TokenType::Binary, Opcode::F32X4Div},
#line 146 "./lexer-keywords.txt"
      {"f64.sqrt", TokenType::Unary, Opcode::F64Sqrt},
#line 84 "./lexer-keywords.txt"
      {"f32.sqrt", TokenType::Unary, Opcode::F32Sqrt},
#line 411 "./lexer-keywords.txt"
      {"i64.atomic.rmw.sub", TokenType::AtomicRmw, Opcode::I64AtomicRmwSub},
#line 268 "./lexer-keywords.txt"
      {"i32.atomic.rmw.sub", TokenType::AtomicRmw, Opcode::I32AtomicRmwSub},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 116 "./lexer-keywords.txt"
      {"f32x4.sqrt", TokenType::Unary, Opcode::F32X4Sqrt},
      {""},
#line 507 "./lexer-keywords.txt"
      {"i8x16.all_true", TokenType::Unary, Opcode::I8X16AllTrue},
#line 593 "./lexer-keywords.txt"
      {"array.new", TokenType::ArrayNew, Opcode::ArrayNew                                                                          },
#line 645 "./lexer-keywords.txt"
      {"v128.load8_splat", TokenType::Load, Opcode::V128Load8Splat},
      {""}, {""},
#line 566 "./lexer-keywords.txt"
      {"output", TokenType::Output},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 425 "./lexer-keywords.txt"
      {"i64.extend16_s", TokenType::Unary, Opcode::I64Extend16S},
#line 281 "./lexer-keywords.txt"
      {"i32.extend16_s", TokenType::Unary, Opcode::I32Extend16S},
      {""},
#line 565 "./lexer-keywords.txt"
      {"offset", TokenType::Offset},
#line 404 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8SubU},
#line 261 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.sub_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8SubU},
      {""},
#line 625 "./lexer-keywords.txt"
      {"throw", TokenType::Throw, Opcode::Throw},
#line 597 "./lexer-keywords.txt"
      {"array.new_elem", TokenType::ArrayNewElem, Opcode::ArrayNewElem                                                                  },
      {""}, {""},
#line 505 "./lexer-keywords.txt"
      {"i8x16.add_sat_u", TokenType::Binary, Opcode::I8X16AddSatU},
#line 163 "./lexer-keywords.txt"
      {"f64x2.min", TokenType::Binary, Opcode::F64X2Min},
#line 426 "./lexer-keywords.txt"
      {"i64.extend32_s", TokenType::Unary, Opcode::I64Extend32S},
#line 540 "./lexer-keywords.txt"
      {"i8x16", TokenType::I8X16},
#line 504 "./lexer-keywords.txt"
      {"i8x16.add_sat_s", TokenType::Binary, Opcode::I8X16AddSatS},
#line 646 "./lexer-keywords.txt"
      {"v128.load8_lane", TokenType::SimdLoadLane, Opcode::V128Load8Lane},
      {""}, {""}, {""},
#line 138 "./lexer-keywords.txt"
      {"f64.max", TokenType::Binary, Opcode::F64Max},
#line 77 "./lexer-keywords.txt"
      {"f32.max", TokenType::Binary, Opcode::F32Max},
#line 643 "./lexer-keywords.txt"
      {"v128.load32_splat", TokenType::Load, Opcode::V128Load32Splat},
#line 571 "./lexer-keywords.txt"
      {"quote", TokenType::Quote},
      {""}, {""}, {""}, {""}, {""},
#line 416 "./lexer-keywords.txt"
      {"i64.atomic.store8", TokenType::AtomicStore, Opcode::I64AtomicStore8},
#line 272 "./lexer-keywords.txt"
      {"i32.atomic.store8", TokenType::AtomicStore, Opcode::I32AtomicStore8},
      {""}, {""},
#line 27 "./lexer-keywords.txt"
      {"assert_malformed", TokenType::AssertMalformed},
      {""}, {""},
#line 362 "./lexer-keywords.txt"
      {"i32x4.splat", TokenType::Unary, Opcode::I32X4Splat},
#line 207 "./lexer-keywords.txt"
      {"i16x8.gt_u", TokenType::Compare, Opcode::I16X8GtU},
      {""}, {""},
#line 115 "./lexer-keywords.txt"
      {"f32x4.splat", TokenType::Unary, Opcode::F32X4Splat},
#line 206 "./lexer-keywords.txt"
      {"i16x8.gt_s", TokenType::Compare, Opcode::I16X8GtS},
      {""},
#line 205 "./lexer-keywords.txt"
      {"i16x8.ge_u", TokenType::Compare, Opcode::I16X8GeU},
#line 650 "./lexer-keywords.txt"
      {"v128.store8_lane", TokenType::SimdStoreLane, Opcode::V128Store8Lane},
      {""}, {""},
#line 204 "./lexer-keywords.txt"
      {"i16x8.ge_s", TokenType::Compare, Opcode::I16X8GeS},
      {""}, {""},
#line 30 "./lexer-keywords.txt"
      {"assert_unlinkable", TokenType::AssertUnlinkable},
      {""}, {""},
#line 213 "./lexer-keywords.txt"
      {"i16x8.lt_u", TokenType::Compare, Opcode::I16X8LtU},
      {""},
#line 162 "./lexer-keywords.txt"
      {"f64x2.max", TokenType::Binary, Opcode::F64X2Max},
#line 522 "./lexer-keywords.txt"
      {"i8x16.max_u", TokenType::Binary, Opcode::I8X16MaxU},
#line 212 "./lexer-keywords.txt"
      {"i16x8.lt_s", TokenType::Compare, Opcode::I16X8LtS},
#line 521 "./lexer-keywords.txt"
      {"i8x16.max_s", TokenType::Binary, Opcode::I8X16MaxS},
#line 209 "./lexer-keywords.txt"
      {"i16x8.le_u", TokenType::Compare, Opcode::I16X8LeU},
#line 648 "./lexer-keywords.txt"
      {"v128.load32_lane", TokenType::SimdLoadLane, Opcode::V128Load32Lane},
      {""},
#line 34 "./lexer-keywords.txt"
      {"block", TokenType::Block, Opcode::Block},
#line 208 "./lexer-keywords.txt"
      {"i16x8.le_s", TokenType::Compare, Opcode::I16X8LeS},
      {""}, {""}, {""},
#line 474 "./lexer-keywords.txt"
      {"v128.load32x2_u", TokenType::Load, Opcode::V128Load32X2U},
#line 541 "./lexer-keywords.txt"
      {"if", TokenType::If, Opcode::If},
      {""}, {""},
#line 473 "./lexer-keywords.txt"
      {"v128.load32x2_s", TokenType::Load, Opcode::V128Load32X2S},
      {""}, {""},
#line 641 "./lexer-keywords.txt"
      {"v128.xor", TokenType::Binary, Opcode::V128Xor},
#line 568 "./lexer-keywords.txt"
      {"ref", Type::Ref, TokenType::Ref},
#line 53 "./lexer-keywords.txt"
      {"else", TokenType::Else, Opcode::Else},
      {""}, {""},
#line 606 "./lexer-keywords.txt"
      {"array.init_elem", TokenType::ArrayInitElem, Opcode::ArrayInitElem  },
      {""}, {""}, {""}, {""}, {""}, {""},
#line 651 "./lexer-keywords.txt"
      {"v128.store16_lane", TokenType::SimdStoreLane, Opcode::V128Store16Lane},
      {""}, {""},
#line 508 "./lexer-keywords.txt"
      {"i8x16.avgr_u", TokenType::Binary, Opcode::I8X16AvgrU},
      {""},
#line 580 "./lexer-keywords.txt"
      {"return_call_indirect", TokenType::ReturnCallIndirect, Opcode::ReturnCallIndirect},
      {""}, {""},
#line 111 "./lexer-keywords.txt"
      {"f32x4.relaxed_max", TokenType::Binary, Opcode::F32X4RelaxedMax},
#line 221 "./lexer-keywords.txt"
      {"i16x8.neg", TokenType::Unary, Opcode::I16X8Neg},
#line 223 "./lexer-keywords.txt"
      {"i16x8.ne", TokenType::Compare, Opcode::I16X8Ne},
      {""},
#line 631 "./lexer-keywords.txt"
      {"v128.bitselect", TokenType::Ternary, Opcode::V128BitSelect},
#line 652 "./lexer-keywords.txt"
      {"v128.store32_lane", TokenType::SimdStoreLane, Opcode::V128Store32Lane},
#line 637 "./lexer-keywords.txt"
      {"v128.load32_zero", TokenType::Load, Opcode::V128Load32Zero},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 619 "./lexer-keywords.txt"
      {"table.grow", TokenType::TableGrow, Opcode::TableGrow},
#line 579 "./lexer-keywords.txt"
      {"rethrow", TokenType::Rethrow, Opcode::Rethrow},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 50 "./lexer-keywords.txt"
      {"either", TokenType::Either},
#line 472 "./lexer-keywords.txt"
      {"i64x2.extract_lane", TokenType::SimdLaneOp, Opcode::I64X2ExtractLane},
      {""}, {""}, {""},
#line 156 "./lexer-keywords.txt"
      {"f64x2.extract_lane", TokenType::SimdLaneOp, Opcode::F64X2ExtractLane},
      {""},
#line 624 "./lexer-keywords.txt"
      {"then", TokenType::Then},
#line 322 "./lexer-keywords.txt"
      {"i16", Type::I16},
#line 38 "./lexer-keywords.txt"
      {"call_indirect", TokenType::CallIndirect, Opcode::CallIndirect},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 587 "./lexer-keywords.txt"
      {"struct.new", TokenType::StructNew, Opcode::StructNew},
      {""}, {""}, {""}, {""}, {""},
#line 54 "./lexer-keywords.txt"
      {"end", TokenType::End, Opcode::End},
      {""}, {""}, {""}, {""},
#line 403 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8OrU},
#line 260 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.or_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8OrU},
      {""}, {""}, {""}, {""},
#line 429 "./lexer-keywords.txt"
      {"i64.extend_i32_u", TokenType::Convert, Opcode::I64ExtendI32U},
      {""},
#line 428 "./lexer-keywords.txt"
      {"i64.extend_i32_s", TokenType::Convert, Opcode::I64ExtendI32S},
      {""},
#line 128 "./lexer-keywords.txt"
      {"f64.convert_i64_u", TokenType::Convert, Opcode::F64ConvertI64U},
#line 66 "./lexer-keywords.txt"
      {"f32.convert_i64_u", TokenType::Convert, Opcode::F32ConvertI64U},
      {""}, {""},
#line 127 "./lexer-keywords.txt"
      {"f64.convert_i64_s", TokenType::Convert, Opcode::F64ConvertI64S},
#line 65 "./lexer-keywords.txt"
      {"f32.convert_i64_s", TokenType::Convert, Opcode::F32ConvertI64S},
      {""}, {""}, {""},
#line 196 "./lexer-keywords.txt"
      {"i16x8.add", TokenType::Binary, Opcode::I16X8Add},
      {""}, {""}, {""},
#line 44 "./lexer-keywords.txt"
      {"data.drop", TokenType::DataDrop, Opcode::DataDrop},
#line 218 "./lexer-keywords.txt"
      {"i16x8.mul", TokenType::Binary, Opcode::I16X8Mul},
      {""}, {""}, {""},
#line 456 "./lexer-keywords.txt"
      {"i64.shr_u", TokenType::Binary, Opcode::I64ShrU},
#line 307 "./lexer-keywords.txt"
      {"i32.shr_u", TokenType::Binary, Opcode::I32ShrU},
#line 455 "./lexer-keywords.txt"
      {"i64.shr_s", TokenType::Binary, Opcode::I64ShrS},
#line 306 "./lexer-keywords.txt"
      {"i32.shr_s", TokenType::Binary, Opcode::I32ShrS},
#line 567 "./lexer-keywords.txt"
      {"param", TokenType::Param},
      {""}, {""}, {""},
#line 485 "./lexer-keywords.txt"
      {"i64x2.bitmask", TokenType::Unary, Opcode::I64X2Bitmask},
      {""}, {""}, {""}, {""}, {""},
#line 545 "./lexer-keywords.txt"
      {"item", TokenType::Item},
      {""},
#line 361 "./lexer-keywords.txt"
      {"i32x4.shr_u", TokenType::Binary, Opcode::I32X4ShrU},
      {""},
#line 360 "./lexer-keywords.txt"
      {"i32x4.shr_s", TokenType::Binary, Opcode::I32X4ShrS},
      {""}, {""},
#line 422 "./lexer-keywords.txt"
      {"i64.div_u", TokenType::Binary, Opcode::I64DivU},
#line 278 "./lexer-keywords.txt"
      {"i32.div_u", TokenType::Binary, Opcode::I32DivU},
#line 421 "./lexer-keywords.txt"
      {"i64.div_s", TokenType::Binary, Opcode::I64DivS},
#line 277 "./lexer-keywords.txt"
      {"i32.div_s", TokenType::Binary, Opcode::I32DivS},
#line 56 "./lexer-keywords.txt"
      {"extern", Type::ExternRef, TokenType::Extern},
      {""}, {""}, {""}, {""}, {""},
#line 183 "./lexer-keywords.txt"
      {"field", TokenType::Field},
      {""}, {""}, {""}, {""}, {""},
#line 382 "./lexer-keywords.txt"
      {"i64.atomic.load16_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad16U},
#line 247 "./lexer-keywords.txt"
      {"i32.atomic.load16_u", TokenType::AtomicLoad, Opcode::I32AtomicLoad16U},
      {""},
#line 406 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8XorU},
#line 263 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.xor_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8XorU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 383 "./lexer-keywords.txt"
      {"i64.atomic.load32_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad32U},
      {""}, {""}, {""}, {""}, {""},
#line 224 "./lexer-keywords.txt"
      {"i16x8.relaxed_laneselect", TokenType::Ternary, Opcode::I16X8RelaxedLaneSelect},
#line 211 "./lexer-keywords.txt"
      {"v128.load8x8_u", TokenType::Load, Opcode::V128Load8X8U},
      {""},
#line 210 "./lexer-keywords.txt"
      {"v128.load8x8_s", TokenType::Load, Opcode::V128Load8X8S},
      {""},
#line 97 "./lexer-keywords.txt"
      {"f32x4.floor", TokenType::Unary, Opcode::F32X4Floor},
      {""}, {""},
#line 595 "./lexer-keywords.txt"
      {"array.new_fixed", TokenType::ArrayNewFixed, Opcode::ArrayNewFixed                                                                },
#line 226 "./lexer-keywords.txt"
      {"i16x8.replace_lane", TokenType::SimdLaneOp, Opcode::I16X8ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 390 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16SubU},
#line 254 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.sub_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16SubU},
      {""}, {""}, {""}, {""},
#line 509 "./lexer-keywords.txt"
      {"i8x16.bitmask", TokenType::Unary, Opcode::I8X16Bitmask},
#line 233 "./lexer-keywords.txt"
      {"i16x8.sub", TokenType::Binary, Opcode::I16X8Sub},
      {""}, {""},
#line 193 "./lexer-keywords.txt"
      {"i16x8.abs", TokenType::Unary, Opcode::I16X8Abs},
      {""},
#line 109 "./lexer-keywords.txt"
      {"f32x4.pmin", TokenType::Binary, Opcode::F32X4PMin},
#line 415 "./lexer-keywords.txt"
      {"i64.atomic.store32", TokenType::AtomicStore, Opcode::I64AtomicStore32},
#line 42 "./lexer-keywords.txt"
      {"catch_all", TokenType::CatchAll, Opcode::CatchAll},
      {""},
#line 412 "./lexer-keywords.txt"
      {"i64.atomic.rmw.xchg", TokenType::AtomicRmw, Opcode::I64AtomicRmwXchg},
#line 269 "./lexer-keywords.txt"
      {"i32.atomic.rmw.xchg", TokenType::AtomicRmw, Opcode::I32AtomicRmwXchg},
      {""}, {""}, {""},
#line 573 "./lexer-keywords.txt"
      {"ref.func", TokenType::RefFunc, Opcode::RefFunc},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 240 "./lexer-keywords.txt"
      {"i16x8", TokenType::I16X8},
#line 492 "./lexer-keywords.txt"
      {"i64x2.shl", TokenType::Binary, Opcode::I64X2Shl},
      {""}, {""}, {""}, {""},
#line 132 "./lexer-keywords.txt"
      {"f64.floor", TokenType::Unary, Opcode::F64Floor},
#line 71 "./lexer-keywords.txt"
      {"f32.floor", TokenType::Unary, Opcode::F32Floor},
      {""}, {""}, {""}, {""}, {""},
#line 326 "./lexer-keywords.txt"
      {"noextern", Type::NoExtern, TokenType::NoExtern},
      {""}, {""},
#line 524 "./lexer-keywords.txt"
      {"i8x16.min_u", TokenType::Binary, Opcode::I8X16MinU},
      {""},
#line 523 "./lexer-keywords.txt"
      {"i8x16.min_s", TokenType::Binary, Opcode::I8X16MinS},
      {""}, {""}, {""}, {""}, {""},
#line 323 "./lexer-keywords.txt"
      {"i31", Type::I31, TokenType::I31},
#line 154 "./lexer-keywords.txt"
      {"f64x2.div", TokenType::Binary, Opcode::F64X2Div},
#line 387 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16AndU},
#line 251 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.and_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16AndU},
      {""}, {""}, {""}, {""},
#line 232 "./lexer-keywords.txt"
      {"i16x8.sub_sat_u", TokenType::Binary, Opcode::I16X8SubSatU},
      {""}, {""}, {""},
#line 231 "./lexer-keywords.txt"
      {"i16x8.sub_sat_s", TokenType::Binary, Opcode::I16X8SubSatS},
#line 386 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16AddU},
#line 250 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.add_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16AddU},
      {""}, {""}, {""},
#line 176 "./lexer-keywords.txt"
      {"f64x2.sqrt", TokenType::Unary, Opcode::F64X2Sqrt},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 130 "./lexer-keywords.txt"
      {"f64.div", TokenType::Binary, Opcode::F64Div},
#line 69 "./lexer-keywords.txt"
      {"f32.div", TokenType::Binary, Opcode::F32Div},
      {""},
#line 555 "./lexer-keywords.txt"
      {"memory.fill", TokenType::MemoryFill, Opcode::MemoryFill},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 389 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16OrU},
#line 253 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.or_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16OrU},
      {""},
#line 533 "./lexer-keywords.txt"
      {"i8x16.shl", TokenType::Binary, Opcode::I8X16Shl},
      {""}, {""},
#line 197 "./lexer-keywords.txt"
      {"i16x8.all_true", TokenType::Unary, Opcode::I16X8AllTrue},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 554 "./lexer-keywords.txt"
      {"memory.copy", TokenType::MemoryCopy, Opcode::MemoryCopy},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 495 "./lexer-keywords.txt"
      {"i64x2.splat", TokenType::Unary, Opcode::I64X2Splat},
      {""}, {""},
#line 644 "./lexer-keywords.txt"
      {"v128.load64_splat", TokenType::Load, Opcode::V128Load64Splat},
#line 175 "./lexer-keywords.txt"
      {"f64x2.splat", TokenType::Unary, Opcode::F64X2Splat},
#line 195 "./lexer-keywords.txt"
      {"i16x8.add_sat_u", TokenType::Binary, Opcode::I16X8AddSatU},
      {""}, {""}, {""},
#line 194 "./lexer-keywords.txt"
      {"i16x8.add_sat_s", TokenType::Binary, Opcode::I16X8AddSatS},
      {""}, {""},
#line 395 "./lexer-keywords.txt"
      {"i64.atomic.rmw32.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw32CmpxchgU},
      {""}, {""}, {""}, {""}, {""},
#line 628 "./lexer-keywords.txt"
      {"unreachable", TokenType::Unreachable, Opcode::Unreachable},
      {""}, {""}, {""}, {""},
#line 68 "./lexer-keywords.txt"
      {"f32.demote_f64", TokenType::Convert, Opcode::F32DemoteF64},
      {""}, {""}, {""}, {""},
#line 457 "./lexer-keywords.txt"
      {"i64.store16", TokenType::Store, Opcode::I64Store16},
#line 308 "./lexer-keywords.txt"
      {"i32.store16", TokenType::Store, Opcode::I32Store16},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 655 "./lexer-keywords.txt"
      {"i8x16.swizzle", TokenType::Binary, Opcode::I8X16Swizzle},
      {""},
#line 649 "./lexer-keywords.txt"
      {"v128.load64_lane", TokenType::SimdLoadLane, Opcode::V128Load64Lane},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 215 "./lexer-keywords.txt"
      {"i16x8.max_u", TokenType::Binary, Opcode::I16X8MaxU},
      {""},
#line 214 "./lexer-keywords.txt"
      {"i16x8.max_s", TokenType::Binary, Opcode::I16X8MaxS},
      {""}, {""}, {""}, {""}, {""},
#line 171 "./lexer-keywords.txt"
      {"f64x2.relaxed_max", TokenType::Binary, Opcode::F64X2RelaxedMax},
      {""}, {""},
#line 617 "./lexer-keywords.txt"
      {"table.fill", TokenType::TableFill, Opcode::TableFill},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 536 "./lexer-keywords.txt"
      {"i8x16.splat", TokenType::Unary, Opcode::I8X16Splat},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 512 "./lexer-keywords.txt"
      {"i8x16.extract_lane_u", TokenType::SimdLaneOp, Opcode::I8X16ExtractLaneU},
#line 638 "./lexer-keywords.txt"
      {"v128.load64_zero", TokenType::Load, Opcode::V128Load64Zero},
#line 511 "./lexer-keywords.txt"
      {"i8x16.extract_lane_s", TokenType::SimdLaneOp, Opcode::I8X16ExtractLaneS},
#line 198 "./lexer-keywords.txt"
      {"i16x8.avgr_u", TokenType::Binary, Opcode::I16X8AvgrU},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 26 "./lexer-keywords.txt"
      {"assert_invalid", TokenType::AssertInvalid},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 126 "./lexer-keywords.txt"
      {"f64.convert_i32_u", TokenType::Convert, Opcode::F64ConvertI32U},
#line 64 "./lexer-keywords.txt"
      {"f32.convert_i32_u", TokenType::Convert, Opcode::F32ConvertI32U},
      {""}, {""},
#line 125 "./lexer-keywords.txt"
      {"f64.convert_i32_s", TokenType::Convert, Opcode::F64ConvertI32S},
#line 63 "./lexer-keywords.txt"
      {"f32.convert_i32_s", TokenType::Convert, Opcode::F32ConvertI32S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 29 "./lexer-keywords.txt"
      {"assert_trap", TokenType::AssertTrap},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 494 "./lexer-keywords.txt"
      {"i64x2.shr_u", TokenType::Binary, Opcode::I64X2ShrU},
      {""},
#line 493 "./lexer-keywords.txt"
      {"i64x2.shr_s", TokenType::Binary, Opcode::I64X2ShrS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 157 "./lexer-keywords.txt"
      {"f64x2.floor", TokenType::Unary, Opcode::F64X2Floor},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 610 "./lexer-keywords.txt"
      {"br_on_cast_fail", TokenType::BrOnCastFail, Opcode::BrOnCastFail},
      {""}, {""},
#line 129 "./lexer-keywords.txt"
      {"f64.copysign", TokenType::Binary, Opcode::F64Copysign},
#line 67 "./lexer-keywords.txt"
      {"f32.copysign", TokenType::Binary, Opcode::F32Copysign},
      {""},
#line 528 "./lexer-keywords.txt"
      {"i8x16.popcnt", TokenType::Unary, Opcode::I8X16Popcnt},
      {""},
#line 424 "./lexer-keywords.txt"
      {"i64.eqz", TokenType::Convert, Opcode::I64Eqz},
#line 280 "./lexer-keywords.txt"
      {"i32.eqz", TokenType::Convert, Opcode::I32Eqz},
      {""}, {""}, {""}, {""},
#line 169 "./lexer-keywords.txt"
      {"f64x2.pmin", TokenType::Binary, Opcode::F64X2PMin},
      {""},
#line 535 "./lexer-keywords.txt"
      {"i8x16.shr_u", TokenType::Binary, Opcode::I8X16ShrU},
      {""},
#line 534 "./lexer-keywords.txt"
      {"i8x16.shr_s", TokenType::Binary, Opcode::I8X16ShrS},
      {""}, {""},
#line 530 "./lexer-keywords.txt"
      {"i8x16.relaxed_swizzle", TokenType::Binary, Opcode::I8X16RelaxedSwizzle},
      {""}, {""}, {""}, {""}, {""},
#line 41 "./lexer-keywords.txt"
      {"catch", TokenType::Catch, Opcode::Catch},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 199 "./lexer-keywords.txt"
      {"i16x8.bitmask", TokenType::Unary, Opcode::I16X8Bitmask},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 391 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16XchgU},
#line 255 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.xchg_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16XchgU},
#line 353 "./lexer-keywords.txt"
      {"i32x4.dot_i16x8_s", TokenType::Binary, Opcode::I32X4DotI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 217 "./lexer-keywords.txt"
      {"i16x8.min_u", TokenType::Binary, Opcode::I16X8MinU},
      {""},
#line 216 "./lexer-keywords.txt"
      {"i16x8.min_s", TokenType::Binary, Opcode::I16X8MinS},
      {""},
#line 392 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16XorU},
#line 256 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.xor_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16XorU},
#line 423 "./lexer-keywords.txt"
      {"i64.eq", TokenType::Compare, Opcode::I64Eq},
#line 279 "./lexer-keywords.txt"
      {"i32.eq", TokenType::Compare, Opcode::I32Eq},
      {""},
#line 603 "./lexer-keywords.txt"
      {"array.fill", TokenType::ArrayFill, Opcode::ArrayFill                                                                        },
#line 131 "./lexer-keywords.txt"
      {"f64.eq", TokenType::Compare, Opcode::F64Eq},
#line 70 "./lexer-keywords.txt"
      {"f32.eq", TokenType::Compare, Opcode::F32Eq},
#line 574 "./lexer-keywords.txt"
      {"ref.eq", TokenType::RefEq, Opcode::RefEq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 191 "./lexer-keywords.txt"
      {"global.set", TokenType::GlobalSet, Opcode::GlobalSet},
      {""},
#line 190 "./lexer-keywords.txt"
      {"global.get", TokenType::GlobalGet, Opcode::GlobalGet},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 192 "./lexer-keywords.txt"
      {"global", TokenType::Global},
      {""}, {""}, {""},
#line 333 "./lexer-keywords.txt"
      {"i32x4.eq", TokenType::Compare, Opcode::I32X4Eq},
      {""}, {""}, {""},
#line 95 "./lexer-keywords.txt"
      {"f32x4.eq", TokenType::Compare, Opcode::F32X4Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 327 "./lexer-keywords.txt"
      {"i32.wrap_i64", TokenType::Convert, Opcode::I32WrapI64},
#line 227 "./lexer-keywords.txt"
      {"i16x8.shl", TokenType::Binary, Opcode::I16X8Shl},
      {""}, {""}, {""},
#line 24 "./lexer-keywords.txt"
      {"assert_exception", TokenType::AssertException},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 469 "./lexer-keywords.txt"
      {"i64.trunc_sat_f64_u", TokenType::Convert, Opcode::I64TruncSatF64U},
#line 319 "./lexer-keywords.txt"
      {"i32.trunc_sat_f64_u", TokenType::Convert, Opcode::I32TruncSatF64U},
      {""}, {""},
#line 468 "./lexer-keywords.txt"
      {"i64.trunc_sat_f64_s", TokenType::Convert, Opcode::I64TruncSatF64S},
#line 318 "./lexer-keywords.txt"
      {"i32.trunc_sat_f64_s", TokenType::Convert, Opcode::I32TruncSatF64S},
      {""}, {""}, {""},
#line 642 "./lexer-keywords.txt"
      {"v128.load16_splat", TokenType::Load, Opcode::V128Load16Splat},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 402 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw8CmpxchgU},
#line 259 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmw8CmpxchgU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 25 "./lexer-keywords.txt"
      {"assert_exhaustion", TokenType::AssertExhaustion},
#line 409 "./lexer-keywords.txt"
      {"i64.atomic.rmw.cmpxchg", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmwCmpxchg},
#line 266 "./lexer-keywords.txt"
      {"i32.atomic.rmw.cmpxchg", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmwCmpxchg},
      {""}, {""}, {""}, {""}, {""},
#line 49 "./lexer-keywords.txt"
      {"drop", TokenType::Drop, Opcode::Drop},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 647 "./lexer-keywords.txt"
      {"v128.load16_lane", TokenType::SimdLoadLane, Opcode::V128Load16Lane},
#line 552 "./lexer-keywords.txt"
      {"memory.atomic.wait32", TokenType::AtomicWait, Opcode::MemoryAtomicWait32},
      {""}, {""},
#line 612 "./lexer-keywords.txt"
      {"extern.convert_any", TokenType::ExternConvertAny, Opcode::ExternConvertAny},
      {""}, {""},
#line 346 "./lexer-keywords.txt"
      {"v128.load16x4_u", TokenType::Load, Opcode::V128Load16X4U},
      {""}, {""}, {""},
#line 345 "./lexer-keywords.txt"
      {"v128.load16x4_s", TokenType::Load, Opcode::V128Load16X4S},
#line 324 "./lexer-keywords.txt"
      {"eq", Type::Eq, TokenType::Eq},
      {""},
#line 564 "./lexer-keywords.txt"
      {"nop", TokenType::Nop, Opcode::Nop},
      {""}, {""},
#line 553 "./lexer-keywords.txt"
      {"memory.atomic.wait64", TokenType::AtomicWait, Opcode::MemoryAtomicWait64},
#line 611 "./lexer-keywords.txt"
      {"any.convert_extern", TokenType::AnyConvertExtern, Opcode::AnyConvertExtern},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 230 "./lexer-keywords.txt"
      {"i16x8.splat", TokenType::Unary, Opcode::I16X8Splat},
      {""},
#line 58 "./lexer-keywords.txt"
      {"export", TokenType::Export},
      {""}, {""}, {""}, {""},
#line 93 "./lexer-keywords.txt"
      {"f32x4.convert_i32x4_u", TokenType::Unary, Opcode::F32X4ConvertI32X4U},
      {""},
#line 92 "./lexer-keywords.txt"
      {"f32x4.convert_i32x4_s", TokenType::Unary, Opcode::F32X4ConvertI32X4S},
#line 203 "./lexer-keywords.txt"
      {"i16x8.extract_lane_u", TokenType::SimdLaneOp, Opcode::I16X8ExtractLaneU},
      {""},
#line 202 "./lexer-keywords.txt"
      {"i16x8.extract_lane_s", TokenType::SimdLaneOp, Opcode::I16X8ExtractLaneS},
      {""},
#line 108 "./lexer-keywords.txt"
      {"f32x4.pmax", TokenType::Binary, Opcode::F32X4PMax},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 372 "./lexer-keywords.txt"
      {"i32x4.trunc_sat_f32x4_u", TokenType::Unary, Opcode::I32X4TruncSatF32X4U},
      {""}, {""},
#line 365 "./lexer-keywords.txt"
      {"i32x4.extadd_pairwise_i16x8_u", TokenType::Unary, Opcode::I32X4ExtaddPairwiseI16X8U},
#line 371 "./lexer-keywords.txt"
      {"i32x4.trunc_sat_f32x4_s", TokenType::Unary, Opcode::I32X4TruncSatF32X4S},
#line 364 "./lexer-keywords.txt"
      {"i32x4.extadd_pairwise_i16x8_s", TokenType::Unary, Opcode::I32X4ExtaddPairwiseI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 405 "./lexer-keywords.txt"
      {"i64.atomic.rmw8.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8XchgU},
#line 262 "./lexer-keywords.txt"
      {"i32.atomic.rmw8.xchg_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8XchgU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 550 "./lexer-keywords.txt"
      {"loop", TokenType::Loop, Opcode::Loop},
#line 488 "./lexer-keywords.txt"
      {"i64x2.extend_low_i32x4_u", TokenType::Unary, Opcode::I64X2ExtendLowI32X4U},
      {""},
#line 486 "./lexer-keywords.txt"
      {"i64x2.extend_low_i32x4_s", TokenType::Unary, Opcode::I64X2ExtendLowI32X4S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 613 "./lexer-keywords.txt"
      {"ref.i31", TokenType::RefI31, Opcode::RefI31},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 448 "./lexer-keywords.txt"
      {"i64.popcnt", TokenType::Unary, Opcode::I64Popcnt},
#line 299 "./lexer-keywords.txt"
      {"i32.popcnt", TokenType::Unary, Opcode::I32Popcnt},
      {""}, {""}, {""}, {""}, {""},
#line 225 "./lexer-keywords.txt"
      {"i16x8.relaxed_q15mulr_s", TokenType::Binary, Opcode::I16X8RelaxedQ15mulrS},
#line 119 "./lexer-keywords.txt"
      {"f32x4.demote_f64x2_zero", TokenType::Unary, Opcode::F32X4DemoteF64X2Zero},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 499 "./lexer-keywords.txt"
      {"i64x2.extmul_low_i32x4_u", TokenType::Binary, Opcode::I64X2ExtmulLowI32X4U},
      {""},
#line 497 "./lexer-keywords.txt"
      {"i64x2.extmul_low_i32x4_s", TokenType::Binary, Opcode::I64X2ExtmulLowI32X4S},
#line 414 "./lexer-keywords.txt"
      {"i64.atomic.store16", TokenType::AtomicStore, Opcode::I64AtomicStore16},
#line 271 "./lexer-keywords.txt"
      {"i32.atomic.store16", TokenType::AtomicStore, Opcode::I32AtomicStore16},
      {""}, {""}, {""},
#line 378 "./lexer-keywords.txt"
      {"i32x4.trunc_sat_f64x2_u_zero", TokenType::Unary, Opcode::I32X4TruncSatF64X2UZero},
      {""},
#line 377 "./lexer-keywords.txt"
      {"i32x4.trunc_sat_f64x2_s_zero", TokenType::Unary, Opcode::I32X4TruncSatF64X2SZero},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 229 "./lexer-keywords.txt"
      {"i16x8.shr_u", TokenType::Binary, Opcode::I16X8ShrU},
      {""},
#line 228 "./lexer-keywords.txt"
      {"i16x8.shr_s", TokenType::Binary, Opcode::I16X8ShrS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 35 "./lexer-keywords.txt"
      {"br_if", TokenType::BrIf, Opcode::BrIf},
#line 145 "./lexer-keywords.txt"
      {"f64.reinterpret_i64", TokenType::Convert, Opcode::F64ReinterpretI64},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 551 "./lexer-keywords.txt"
      {"memory.atomic.notify", TokenType::AtomicNotify, Opcode::MemoryAtomicNotify},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 57 "./lexer-keywords.txt"
      {"externref", Type::ExternRef},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 184 "./lexer-keywords.txt"
      {"funcref", Type::FuncRef},
#line 476 "./lexer-keywords.txt"
      {"i64x2.eq", TokenType::Binary, Opcode::I64X2Eq},
      {""}, {""},
#line 52 "./lexer-keywords.txt"
      {"elem", TokenType::Elem},
#line 155 "./lexer-keywords.txt"
      {"f64x2.eq", TokenType::Compare, Opcode::F64X2Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 467 "./lexer-keywords.txt"
      {"i64.trunc_sat_f32_u", TokenType::Convert, Opcode::I64TruncSatF32U},
#line 317 "./lexer-keywords.txt"
      {"i32.trunc_sat_f32_u", TokenType::Convert, Opcode::I32TruncSatF32U},
      {""}, {""},
#line 466 "./lexer-keywords.txt"
      {"i64.trunc_sat_f32_s", TokenType::Convert, Opcode::I64TruncSatF32S},
#line 316 "./lexer-keywords.txt"
      {"i32.trunc_sat_f32_s", TokenType::Convert, Opcode::I32TruncSatF32S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 388 "./lexer-keywords.txt"
      {"i64.atomic.rmw16.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw16CmpxchgU},
#line 252 "./lexer-keywords.txt"
      {"i32.atomic.rmw16.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmw16CmpxchgU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 510 "./lexer-keywords.txt"
      {"i8x16.eq", TokenType::Compare, Opcode::I8X16Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 180 "./lexer-keywords.txt"
      {"f64x2.convert_low_i32x4_u", TokenType::Unary, Opcode::F64X2ConvertLowI32X4U},
      {""},
#line 179 "./lexer-keywords.txt"
      {"f64x2.convert_low_i32x4_s", TokenType::Unary, Opcode::F64X2ConvertLowI32X4S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 220 "./lexer-keywords.txt"
      {"i16x8.narrow_i32x4_u", TokenType::Binary, Opcode::I16X8NarrowI32X4U},
      {""},
#line 219 "./lexer-keywords.txt"
      {"i16x8.narrow_i32x4_s", TokenType::Binary, Opcode::I16X8NarrowI32X4S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 168 "./lexer-keywords.txt"
      {"f64x2.pmax", TokenType::Binary, Opcode::F64X2PMax},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 654 "./lexer-keywords.txt"
      {"i8x16.shuffle", TokenType::SimdShuffleOp, Opcode::I8X16Shuffle},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 144 "./lexer-keywords.txt"
      {"f64.promote_f32", TokenType::Convert, Opcode::F64PromoteF32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 376 "./lexer-keywords.txt"
      {"i32x4.extend_low_i16x8_u", TokenType::Unary, Opcode::I32X4ExtendLowI16X8U},
      {""},
#line 375 "./lexer-keywords.txt"
      {"i32x4.extend_low_i16x8_s", TokenType::Unary, Opcode::I32X4ExtendLowI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 83 "./lexer-keywords.txt"
      {"f32.reinterpret_i32", TokenType::Convert, Opcode::F32ReinterpretI32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 368 "./lexer-keywords.txt"
      {"i32x4.extmul_low_i16x8_u", TokenType::Binary, Opcode::I32X4ExtmulLowI16X8U},
      {""},
#line 366 "./lexer-keywords.txt"
      {"i32x4.extmul_low_i16x8_s", TokenType::Binary, Opcode::I32X4ExtmulLowI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 374 "./lexer-keywords.txt"
      {"i32x4.extend_high_i16x8_u", TokenType::Unary, Opcode::I32X4ExtendHighI16X8U},
      {""},
#line 373 "./lexer-keywords.txt"
      {"i32x4.extend_high_i16x8_s", TokenType::Unary, Opcode::I32X4ExtendHighI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 39 "./lexer-keywords.txt"
      {"call_ref", TokenType::CallRef, Opcode::CallRef},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 369 "./lexer-keywords.txt"
      {"i32x4.extmul_high_i16x8_u", TokenType::Binary, Opcode::I32X4ExtmulHighI16X8U},
      {""},
#line 367 "./lexer-keywords.txt"
      {"i32x4.extmul_high_i16x8_s", TokenType::Binary, Opcode::I32X4ExtmulHighI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 332 "./lexer-keywords.txt"
      {"i32x4.dot_i8x16_i7x16_add_s", TokenType::Ternary, Opcode::I32X4DotI8X16I7X16AddS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 31 "./lexer-keywords.txt"
      {"atomic.fence", TokenType::AtomicFence, Opcode::AtomicFence},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 489 "./lexer-keywords.txt"
      {"i64x2.extend_high_i32x4_u", TokenType::Unary, Opcode::I64X2ExtendHighI32X4U},
      {""},
#line 487 "./lexer-keywords.txt"
      {"i64x2.extend_high_i32x4_s", TokenType::Unary, Opcode::I64X2ExtendHighI32X4S},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 185 "./lexer-keywords.txt"
      {"anyref",Type::Any},
      {""},
#line 201 "./lexer-keywords.txt"
      {"i16x8.eq", TokenType::Compare, Opcode::I16X8Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 500 "./lexer-keywords.txt"
      {"i64x2.extmul_high_i32x4_u", TokenType::Binary, Opcode::I64X2ExtmulHighI32X4U},
      {""},
#line 498 "./lexer-keywords.txt"
      {"i64x2.extmul_high_i32x4_s", TokenType::Binary, Opcode::I64X2ExtmulHighI32X4S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 244 "./lexer-keywords.txt"
      {"i16x8.extend_low_i8x16_u", TokenType::Unary, Opcode::I16X8ExtendLowI8X16U},
      {""},
#line 243 "./lexer-keywords.txt"
      {"i16x8.extend_low_i8x16_s", TokenType::Unary, Opcode::I16X8ExtendLowI8X16S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 449 "./lexer-keywords.txt"
      {"i64.reinterpret_f64", TokenType::Convert, Opcode::I64ReinterpretF64},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 238 "./lexer-keywords.txt"
      {"i16x8.extmul_low_i8x16_u", TokenType::Binary, Opcode::I16X8ExtmulLowI8X16U},
      {""},
#line 236 "./lexer-keywords.txt"
      {"i16x8.extmul_low_i8x16_s", TokenType::Binary, Opcode::I16X8ExtmulLowI8X16S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 235 "./lexer-keywords.txt"
      {"i16x8.extadd_pairwise_i8x16_u", TokenType::Unary, Opcode::I16X8ExtaddPairwiseI8X16U},
      {""},
#line 234 "./lexer-keywords.txt"
      {"i16x8.extadd_pairwise_i8x16_s", TokenType::Unary, Opcode::I16X8ExtaddPairwiseI8X16S},
      {""},
#line 526 "./lexer-keywords.txt"
      {"i8x16.narrow_i16x8_u", TokenType::Binary, Opcode::I8X16NarrowI16X8U},
      {""},
#line 525 "./lexer-keywords.txt"
      {"i8x16.narrow_i16x8_s", TokenType::Binary, Opcode::I8X16NarrowI16X8S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 51 "./lexer-keywords.txt"
      {"elem.drop", TokenType::ElemDrop, Opcode::ElemDrop},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 222 "./lexer-keywords.txt"
      {"i16x8.q15mulr_sat_s", TokenType::Binary, Opcode::I16X8Q15mulrSatS},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 300 "./lexer-keywords.txt"
      {"i32.reinterpret_f32", TokenType::Convert, Opcode::I32ReinterpretF32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 181 "./lexer-keywords.txt"
      {"f64x2.promote_low_f32x4", TokenType::Unary, Opcode::F64X2PromoteLowF32X4},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 200 "./lexer-keywords.txt"
      {"i16x8.dot_i8x16_i7x16_s", TokenType::Binary, Opcode::I16X8DotI8X16I7X16S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 242 "./lexer-keywords.txt"
      {"i16x8.extend_high_i8x16_u", TokenType::Unary, Opcode::I16X8ExtendHighI8X16U},
      {""},
#line 241 "./lexer-keywords.txt"
      {"i16x8.extend_high_i8x16_s", TokenType::Unary, Opcode::I16X8ExtendHighI8X16S},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 239 "./lexer-keywords.txt"
      {"i16x8.extmul_high_i8x16_u", TokenType::Binary, Opcode::I16X8ExtmulHighI8X16U},
      {""},
#line 237 "./lexer-keywords.txt"
      {"i16x8.extmul_high_i8x16_s", TokenType::Binary, Opcode::I16X8ExtmulHighI8X16S}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
