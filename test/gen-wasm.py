#!/usr/bin/env python3
#
# Copyright 2016 WebAssembly Community Group participants
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
import struct
import sys

from utils import Error, Hexdump

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
OUT_DIR = os.path.join(ROOT_DIR, 'out')
PLY_DIR = os.path.join(ROOT_DIR, 'third_party', 'ply')

sys.path.append(PLY_DIR)

try:
    import ply.lex as lex
    import ply.yacc as yacc
except ImportError:
    raise Error('Unable to import ply. Did you run "git submodule update"?')

# ply stuff ###################################################################
NAMED_VALUES = {
    'i32': 0x7f,    # -1
    'i64': 0x7e,    # -2
    'f32': 0x7d,    # -3
    'f64': 0x7c,    # -4
    'v128': 0x7b,    # -5
    'anyfunc': 0x70,    # -0x10
    'function': 0x60,    # -0x20
    'struct': 0x5f,    # -0x21
    'array': 0x5e,    # -0x22
    'void': 0x40,    # -0x40
    'magic': (0, 0x61, 0x73, 0x6d),
    'version': (1, 0, 0, 0),

    # section codes
    'USER': 0,
    'TYPE': 1,
    'IMPORT': 2,
    'FUNCTION': 3,
    'TABLE': 4,
    'MEMORY': 5,
    'GLOBAL': 6,
    'EXPORT': 7,
    'START': 8,
    'ELEM': 9,
    'CODE': 10,
    'DATA': 11,
    'DATACOUNT': 12,
    'EVENT': 13,

    # name subsection codes
    'NAME_MODULE': 0,
    'NAME_FUNCTION': 1,
    'NAME_LOCALS': 2,

    # linking subsection codes
    'LINKING_SEGMENT_INFO': 5,
    'LINKING_INIT_FUNCTIONS': 6,
    'LINKING_COMDAT_INFO': 7,
    'LINKING_SYMBOL_TABLE': 8,

    # external kinds
    'func_kind': 0,
    'table_kind': 1,
    'memory_kind': 2,
    'global_kind': 3,
    "unreachable": 0x00,
    "nop": 0x01,
    "block": 0x02,
    "loop": 0x03,
    "if": 0x04,
    "else": 0x05,
    "end": 0x0b,
    "br": 0x0c,
    "br_if": 0x0d,
    "br_table": 0x0e,
    "return": 0x0f,
    "call": 0x10,
    "call_indirect": 0x11,
    "return_call": 0x12,
    "return_call_indirect": 0x13,
    "drop": 0x1a,
    "select": 0x1b,
    "get_local": 0x20,
    "set_local": 0x21,
    "tee_local": 0x22,
    "get_global": 0x23,
    "set_global": 0x24,
    "i32.load": 0x28,
    "i64.load": 0x29,
    "f32.load": 0x2a,
    "f64.load": 0x2b,
    "i32.load8_s": 0x2c,
    "i32.load8_u": 0x2d,
    "i32.load16_s": 0x2e,
    "i32.load16_u": 0x2f,
    "i64.load8_s": 0x30,
    "i64.load8_u": 0x31,
    "i64.load16_s": 0x32,
    "i64.load16_u": 0x33,
    "i64.load32_s": 0x34,
    "i64.load32_u": 0x35,
    "i32.store": 0x36,
    "i64.store": 0x37,
    "f32.store": 0x38,
    "f64.store": 0x39,
    "i32.store8": 0x3a,
    "i32.store16": 0x3b,
    "i64.store8": 0x3c,
    "i64.store16": 0x3d,
    "i64.store32": 0x3e,
    "memory.size": 0x3f,
    "memory.grow": 0x40,
    "i32.const": 0x41,
    "i64.const": 0x42,
    "f32.const": 0x43,
    "f64.const": 0x44,
    "i32.eqz": 0x45,
    "i32.eq": 0x46,
    "i32.ne": 0x47,
    "i32.lt_s": 0x48,
    "i32.lt_u": 0x49,
    "i32.gt_s": 0x4a,
    "i32.gt_u": 0x4b,
    "i32.le_s": 0x4c,
    "i32.le_u": 0x4d,
    "i32.ge_s": 0x4e,
    "i32.ge_u": 0x4f,
    'i64.eqz': 0x50,
    "i64.eq": 0x51,
    "i64.ne": 0x52,
    "i64.lt_s": 0x53,
    "i64.lt_u": 0x54,
    "i64.gt_s": 0x55,
    "i64.gt_u": 0x56,
    "i64.le_s": 0x57,
    "i64.le_u": 0x58,
    "i64.ge_s": 0x59,
    "i64.ge_u": 0x5a,
    "f32.eq": 0x5b,
    "f32.ne": 0x5c,
    "f32.lt": 0x5d,
    "f32.gt": 0x5e,
    "f32.le": 0x5f,
    "f32.ge": 0x60,
    "f64.eq": 0x61,
    "f64.ne": 0x62,
    "f64.lt": 0x63,
    "f64.gt": 0x64,
    "f64.le": 0x65,
    "f64.ge": 0x66,
    "i32.clz": 0x67,
    "i32.ctz": 0x68,
    "i32.popcnt": 0x69,
    "i32.add": 0x6a,
    "i32.sub": 0x6b,
    "i32.mul": 0x6c,
    "i32.div_s": 0x6d,
    "i32.div_u": 0x6e,
    "i32.rem_s": 0x6f,
    "i32.rem_u": 0x70,
    "i32.and": 0x71,
    "i32.or": 0x72,
    "i32.xor": 0x73,
    "i32.shl": 0x74,
    "i32.shr_s": 0x75,
    "i32.shr_u": 0x76,
    "i32.rotl": 0x77,
    "i32.rotr": 0x78,
    "i64.clz": 0x79,
    "i64.ctz": 0x7a,
    "i64.popcnt": 0x7b,
    "i64.add": 0x7c,
    "i64.sub": 0x7d,
    "i64.mul": 0x7e,
    "i64.div_s": 0x7f,
    "i64.div_u": 0x80,
    "i64.rem_s": 0x81,
    "i64.rem_u": 0x82,
    "i64.and": 0x83,
    "i64.or": 0x84,
    "i64.xor": 0x85,
    "i64.shl": 0x86,
    "i64.shr_s": 0x87,
    "i64.shr_u": 0x88,
    "i64.rotl": 0x89,
    "i64.rotr": 0x8a,
    "f32.abs": 0x8b,
    "f32.neg": 0x8c,
    "f32.copysign": 0x8d,
    "f32.ceil": 0x8e,
    "f32.floor": 0x8f,
    "f32.trunc": 0x90,
    "f32.nearest": 0x91,
    "f32.sqrt": 0x92,
    "f32.add": 0x93,
    "f32.sub": 0x94,
    "f32.mul": 0x95,
    "f32.div": 0x96,
    "f32.min": 0x97,
    "f32.max": 0x98,
    "f64.abs": 0x99,
    "f64.neg": 0x9a,
    "f64.copysign": 0x9b,
    "f64.ceil": 0x9c,
    "f64.floor": 0x9d,
    "f64.trunc": 0x9e,
    "f64.nearest": 0x9f,
    "f64.sqrt": 0xa0,
    "f64.add": 0xa1,
    "f64.sub": 0xa2,
    "f64.mul": 0xa3,
    "f64.div": 0xa4,
    "f64.min": 0xa5,
    "f64.max": 0xa6,
    "i32.wrap/i64": 0xa7,
    "i32.trunc_s/f32": 0xa8,
    "i32.trunc_u/f32": 0xa9,
    "i32.trunc_s/f64": 0xaa,
    "i32.trunc_u/f64": 0xab,
    "i64.extend_s/i32": 0xac,
    "i64.extend_u/i32": 0xad,
    "i64.trunc_s/f32": 0xae,
    "i64.trunc_u/f32": 0xaf,
    "i64.trunc_s/f64": 0xb0,
    "i64.trunc_u/f64": 0xb1,
    "f32.convert_s/i32": 0xb2,
    "f32.convert_u/i32": 0xb3,
    "f32.convert_s/i64": 0xb4,
    "f32.convert_u/i64": 0xb5,
    "f32.demote/f64": 0xb6,
    "f64.convert_s/i32": 0xb7,
    "f64.convert_u/i32": 0xb8,
    "f64.convert_s/i64": 0xb9,
    "f64.convert_u/i64": 0xba,
    "f64.promote/f32": 0xbb,
    "i32.reinterpret/f32": 0xbc,
    "i64.reinterpret/f64": 0xbd,
    "f32.reinterpret/i32": 0xbe,
    "f64.reinterpret/i64": 0xbf,

    # bulk memory
    "memory.init": (0xfc, 0x08),
    "data.drop": (0xfc, 0x09),
    "memory.copy": (0xfc, 0x0a),
    "memory.fill": (0xfc, 0x0b),
    "table.init": (0xfc, 0x0c),
    "elem.drop": (0xfc, 0x0d),
    "table.copy": (0xfc, 0x0e),

    # exceptions
    "try": 0x06,
    "catch_all": 0x19,
}

keywords = {
    'func': 'FUNC',
    'section': 'SECTION',
    'leb_i32': 'LEB_I32',
    'leb_i64': 'LEB_I64',
    'leb_u32': 'LEB_U32',
    'f32': 'F32',
    'f64': 'F64',
    'str': 'STR',
}

# lexer ###

tokens = tuple(keywords.values()) + (
    'BYTE',
    'INT',
    'FLOAT',
    'STRING',
    'NAME',
    'NAMED_VALUE',
    'LPAREN',
    'RPAREN',
    'LBRACE',
    'RBRACE',
    'LBRACKET',
    'RBRACKET',)

t_LPAREN = r'\('
t_RPAREN = r'\)'
t_LBRACE = r'{'
t_RBRACE = r'}'
t_LBRACKET = r'\['
t_RBRACKET = r'\]'
t_ignore = ' \t'


def t_COMMENT(t):
    r';;.*'
    pass


def t_INT(t):
    r'\-?(0[xX][0-9a-fA-F]+|[0-9]+)'
    if t.value.lower().startswith('0x'):
        t.value = int(t.value, 16)
    else:
        t.value = int(t.value)

    if 0 <= t.value < 256:
        t.type = 'BYTE'
    return t


def t_FLOAT(t):
    r'\-?([0-9]*\.?[0-9]+|[0-9]+\.?[0-9]*)([eE][0-9]+)?'
    t.value = float(t.value)
    return t


def t_STRING(t):
    r'\'[^\']*\'|\"[^\"]*\"'
    t.value = t.value[1:-1]
    return t


def t_NAME(t):
    r'[a-zA-Z][a-zA-Z0-9_\.\/]*'
    if t.value in NAMED_VALUES:
        t.type = 'NAMED_VALUE'
        t.value = NAMED_VALUES[t.value]
    elif t.value in keywords:
        t.type = keywords[t.value]
    return t


def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)


def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)


lexer = lex.lex()

# parser ###


def LebLoop(data, v, cond):
    while True:
        byte = v & 0x7f
        v >>= 7
        if cond(v, byte):
            data.append(byte)
            break
        else:
            data.append(byte | 0x80)


def WriteUnsignedLeb(data, v, max_size):
    result = []
    LebLoop(result, v, lambda v, byte: v == 0)
    assert len(result) <= max_size
    data.extend(result)


def WriteLebU32(data, v):
    WriteUnsignedLeb(data, v, 5)


def WriteSignedLeb(data, v, max_size):
    result = []
    if v < 0:
        LebLoop(result, v, lambda v, byte: v == -1 and byte & 0x40)
    else:
        LebLoop(result, v, lambda v, byte: v == 0 and not byte & 0x40)
    assert len(result) <= max_size
    data.extend(result)


def WriteLebI32(data, v):
    WriteSignedLeb(data, v, 5)


def WriteLebI64(data, v):
    WriteSignedLeb(data, v, 10)


def WriteF32(data, v):
    data.extend(ord(b) for b in struct.pack('<f', v))


def WriteF64(data, v):
    data.extend(ord(b) for b in struct.pack('<d', v))


def WriteString(data, s):
    data.extend(ord(c) for c in s)


def p_data_byte(p):
    'data : data BYTE'
    p[0] = p[1]
    p[0].append(p[2])


def p_data_name(p):
    '''data : data NAME LBRACKET data RBRACKET
          | data FUNC LBRACKET data RBRACKET'''
    p[0] = p[1]
    # name is only used for documentation
    p[0].extend(p[4])


def p_data_named_value(p):
    'data : data NAMED_VALUE'
    p[0] = p[1]
    if type(p[2]) is tuple:
        p[0].extend(p[2])
    else:
        p[0].append(p[2])


def p_data_section(p):
    'data : data SECTION LPAREN NAMED_VALUE RPAREN LBRACE data RBRACE'
    p[0] = p[1]
    section_data = p[7]
    p[0].append(p[4])
    WriteLebU32(p[0], len(section_data))
    p[0].extend(section_data)


def p_data_user_section(p):
    'data : data SECTION LPAREN STRING RPAREN LBRACE data RBRACE'
    p[0] = p[1]
    name = p[4]
    section_data = p[7]
    p[0].append(0)  # 0 is the section code for "user"
    section_name_data = []
    WriteLebU32(section_name_data, len(name))
    WriteString(section_name_data, name)
    WriteLebU32(p[0], len(section_name_data) + len(section_data))
    p[0].extend(section_name_data)
    p[0].extend(section_data)


def p_data_func(p):
    'data : data FUNC LBRACE data RBRACE'
    p[0] = p[1]
    func_data = p[4]
    func_data.append(0xb)  # end opcode
    WriteLebU32(p[0], len(func_data))
    p[0].extend(func_data)


def p_data_str(p):
    'data : data STR LPAREN STRING RPAREN'
    p[0] = p[1]
    s = p[4]
    WriteLebU32(p[0], len(s))
    WriteString(p[0], s)


def p_data_leb_i32(p):
    '''data : data LEB_I32 LPAREN INT RPAREN
            | data LEB_I32 LPAREN BYTE RPAREN'''
    p[0] = p[1]
    WriteLebI32(p[0], p[4])


def p_data_leb_i64(p):
    '''data : data LEB_I64 LPAREN INT RPAREN
            | data LEB_I64 LPAREN BYTE RPAREN'''
    p[0] = p[1]
    WriteLebI64(p[0], p[4])


def p_data_leb_u32(p):
    '''data : data LEB_U32 LPAREN INT RPAREN
            | data LEB_U32 LPAREN BYTE RPAREN'''
    p[0] = p[1]
    WriteLebU32(p[0], p[4])


def p_data_f32(p):
    'data : data F32 LPAREN FLOAT RPAREN'
    p[0] = p[1]
    WriteF32(p[0], p[4])


def p_data_f64(p):
    'data : data F64 LPAREN FLOAT RPAREN'
    p[0] = p[1]
    WriteF64(p[0], p[4])


def p_data_string(p):
    'data : data STRING'
    p[0] = p[1]
    WriteString(p[0], p[2])


def p_data_empty(p):
    'data :'
    p[0] = []


def p_error(p):
    raise Error('%d: syntax error, %s' % (p.lineno, p))


parser = yacc.yacc(debug=False, tabmodule='gen_wasm',
                   debugfile='gen_wasm_debug.txt', outputdir=OUT_DIR)

################################################################################


def Run(input_file_name):
    with open(input_file_name) as input_file:
        input_data = input_file.read()
    data = parser.parse(input_data)
    # convert to bytes
    data = bytearray(data)
    return data


def main(args):
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-o', '--output', metavar='PATH',
                            help='output file.')
    arg_parser.add_argument('-v', '--verbose',
                            help='print more diagnotic messages.',
                            action='store_true')
    arg_parser.add_argument('file', help='input file.')
    options = arg_parser.parse_args(args)
    data = Run(options.file)
    if options.output:
        with open(options.output, 'wb') as output_file:
            output_file.write(data)
    else:
        sys.stdout.writelines(Hexdump(data))
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except Error as e:
        sys.stderr.write(str(e) + '\n')
        sys.exit(1)
