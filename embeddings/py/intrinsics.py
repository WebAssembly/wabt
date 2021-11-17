def encode_utf8(string, realloc, memory):
    string_bytes = string.encode('utf8')
    string_len = len(string_bytes)
    ptr = realloc(0, 0, 1, string_len) & 0xffffffff
    if ptr + string_len > memory.data_size:
        raise IndexError("The string is out of bounds")
    buffer = memory.int8_view()
    buffer[ptr:ptr+string_len] = string_bytes
    return ptr, string_len

def decode_utf8(memory, ptr, string_len):
    ptr = ptr & 0xffffffff 
    string_len = string_len & 0xffffffff
    if ptr + string_len > memory.data_size:
        raise IndexError('The string is out of bounds')
    buffer = memory.int8_view()
    bytes = bytearray(buffer[ptr:ptr+string_len])
    return bytes.decode('utf8')
