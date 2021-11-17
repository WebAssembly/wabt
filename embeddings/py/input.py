from wasmer import Store, Module, Instance
from intrinsics import encode_utf8, decode_utf8
from enum import IntFlag

class WasmFeature(IntFlag):
    Exceptions=1
    MutableGlobals=2
    SatFloatToInt=4
    SignExtension=8
    SIMD=16
    Threads=32
    MultiValue=64
    TailCall=128
    BulkMemory=256
    ReferenceTypes=512
    Annotations=1024
    GC=2048

    Phase5=1|2|4|8|16|BulkMemory|ReferenceTypes
    Phase4=Phase5
    Phase3=Phase4|MultiValue|TailCall|Annotations

class WABT:
    def instantiate(self, module, imports):
        self.instance = Instance(module, imports)

    def wasm2wat(self, wasm, flags):
        memory = self.instance.exports.memory
        realloc = self.instance.exports.canonical_abi_realloc
        free = self.instance.exports.canonical_abi_free
        val0 = wasm
        len0 = len(val0)
        ptr0 = realloc(0, 0, 1, len0 * 1)
        buffer = memory.int8_view()
        buffer[ptr0:ptr0+len0*1] = val0
        ret = self.instance.exports.wasm2wat(ptr0, len0, flags)
        ret = int(ret / 4)
        buf32 = memory.int32_view()
        if buf32[ret] == 0:
            ptr1 = buf32[ret + 2]
            len1 = buf32[ret + 4]
            list1 = decode_utf8(memory, ptr1, len1)
            free(ptr1, len1, 1)
            return list1
        elif buf32[ret] == 1:
            ptr2 = buf32[ret + 2]
            len2 = buf32[ret + 4]
            list2 = decode_utf8(memory, ptr2, len2)
            free(ptr2, len2, 1)
            raise Exception(list2)

    def wat2wasm(self, wat, flags):
        memory = self.instance.exports.memory
        realloc = self.instance.exports.canonical_abi_realloc
        free = self.instance.exports.canonical_abi_free
        ptr0, len0 = encode_utf8(wat, realloc, memory)
        ret = self.instance.exports.wat2wasm(ptr0, len0, flags)
        ret = int(ret / 4)
        buf32 = memory.int32_view()
        if buf32[ret] == 0:
            ptr1 = buf32[ret + 2]
            len1 = buf32[ret + 4]
            buf8 = memory.int8_view()
            wasm = bytearray(buf8[ptr1:ptr1+len1])
            free(ptr1, len1, 1)
            return wasm
        elif buf32[ret] == 1:
            ptr2 = buf32[ret + 2]
            len2 = buf32[ret + 4]
            message = decode_utf8(memory, ptr2, len2)
            free(ptr2, len2, 1)
            raise Exception(message)
