from abc import abstractmethod
import ctypes
from dataclasses import dataclass
from enum import Flag, auto
from typing import Any, Generic, List, Tuple, TypeVar, Union, cast
import wasmtime

try:
    from typing import Protocol
except ImportError:
    class Protocol: # type: ignore
        pass

T = TypeVar('T')

def _load(ty: Any, mem: wasmtime.Memory, store: wasmtime.Storelike, base: int, offset: int) -> Any:
    ptr = (base & 0xffffffff) + offset
    if ptr + ctypes.sizeof(ty) > mem.data_len(store):
        raise IndexError('out-of-bounds store')
    raw_base = mem.data_ptr(store)
    c_ptr = ctypes.POINTER(ty)(
        ty.from_address(ctypes.addressof(raw_base.contents) + ptr)
    )
    return c_ptr[0]

@dataclass
class Ok(Generic[T]):
    value: T
E = TypeVar('E')
@dataclass
class Err(Generic[E]):
    value: E

Expected = Union[Ok[T], Err[E]]

def _decode_utf8(mem: wasmtime.Memory, store: wasmtime.Storelike, ptr: int, len: int) -> str:
    ptr = ptr & 0xffffffff
    len = len & 0xffffffff
    if ptr + len > mem.data_len(store):
        raise IndexError('string out of bounds')
    base = mem.data_ptr(store)
    base = ctypes.POINTER(ctypes.c_ubyte)(
        ctypes.c_ubyte.from_address(ctypes.addressof(base.contents) + ptr)
    )
    return ctypes.string_at(base, len).decode('utf-8')

def _encode_utf8(val: str, realloc: wasmtime.Func, mem: wasmtime.Memory, store: wasmtime.Storelike) -> Tuple[int, int]:
    bytes = val.encode('utf8')
    ptr = realloc(store, 0, 0, 1, len(bytes))
    assert(isinstance(ptr, int))
    ptr = ptr & 0xffffffff
    if ptr + len(bytes) > mem.data_len(store):
        raise IndexError('string out of bounds')
    base = mem.data_ptr(store)
    base = ctypes.POINTER(ctypes.c_ubyte)(
        ctypes.c_ubyte.from_address(ctypes.addressof(base.contents) + ptr)
    )
    ctypes.memmove(base, bytes, len(bytes))
    return (ptr, len(bytes))

def _list_canon_lift(ptr: int, len: int, size: int, ty: Any, mem: wasmtime.Memory ,store: wasmtime.Storelike) -> Any:
    ptr = ptr & 0xffffffff
    len = len & 0xffffffff
    if ptr + len * size > mem.data_len(store):
        raise IndexError('list out of bounds')
    raw_base = mem.data_ptr(store)
    base = ctypes.POINTER(ty)(
        ty.from_address(ctypes.addressof(raw_base.contents) + ptr)
    )
    if ty == ctypes.c_uint8:
        return ctypes.string_at(base, len)
    return base[:len]

def _list_canon_lower(list: Any, ty: Any, size: int, align: int, realloc: wasmtime.Func, mem: wasmtime.Memory, store: wasmtime.Storelike) -> Tuple[int, int]:
    total_size = size * len(list)
    ptr = realloc(store, 0, 0, align, total_size)
    assert(isinstance(ptr, int))
    ptr = ptr & 0xffffffff
    if ptr + total_size > mem.data_len(store):
        raise IndexError('list realloc return of bounds')
    raw_base = mem.data_ptr(store)
    base = ctypes.POINTER(ty)(
        ty.from_address(ctypes.addressof(raw_base.contents) + ptr)
    )
    for i, val in enumerate(list):
        base[i] = val
    return (ptr, len(list))
class WasmFeature(Flag):
    EXCEPTIONS = auto()
    MUTABLE_GLOBALS = auto()
    SAT_FLOAT_TO_INT = auto()
    SIGN_EXTENSION = auto()
    SIMD = auto()
    THREADS = auto()
    MULTI_VALUE = auto()
    TAIL_CALL = auto()
    BULK_MEMORY = auto()
    REFERENCE_TYPES = auto()
    ANNOTATIONS = auto()
    GC = auto()

class Input:
    instance: wasmtime.Instance
    _canonical_abi_free: wasmtime.Func
    _canonical_abi_realloc: wasmtime.Func
    _memory: wasmtime.Memory
    _wasm2wat: wasmtime.Func
    _wat2wasm: wasmtime.Func
    def __init__(self, store: wasmtime.Store, linker: wasmtime.Linker, module: wasmtime.Module):
        self.instance = linker.instantiate(store, module)
        exports = self.instance.exports(store)
        
        canonical_abi_free = exports['canonical_abi_free']
        assert(isinstance(canonical_abi_free, wasmtime.Func))
        self._canonical_abi_free = canonical_abi_free
        
        canonical_abi_realloc = exports['canonical_abi_realloc']
        assert(isinstance(canonical_abi_realloc, wasmtime.Func))
        self._canonical_abi_realloc = canonical_abi_realloc
        
        memory = exports['memory']
        assert(isinstance(memory, wasmtime.Memory))
        self._memory = memory
        
        wasm2wat = exports['wasm2wat']
        assert(isinstance(wasm2wat, wasmtime.Func))
        self._wasm2wat = wasm2wat
        
        wat2wasm = exports['wat2wasm']
        assert(isinstance(wat2wasm, wasmtime.Func))
        self._wat2wasm = wat2wasm

    def wat2wasm(self, caller: wasmtime.Store, wat: str, features: WasmFeature) -> Expected[bytes, str]:
        memory = self._memory;
        realloc = self._canonical_abi_realloc
        free = self._canonical_abi_free
        ptr, len0 = _encode_utf8(wat, realloc, memory, caller)
        ret = self._wat2wasm(caller, ptr, len0, (features).value)
        assert(isinstance(ret, int))
        load = _load(ctypes.c_int32, memory, caller, ret, 0)
        load1 = _load(ctypes.c_int32, memory, caller, ret, 8)
        load2 = _load(ctypes.c_int32, memory, caller, ret, 16)
        variant: Expected[bytes, str]
        if load == 0:
            ptr3 = load1
            len4 = load2
            list = cast(bytes, _list_canon_lift(ptr3, len4, 1, ctypes.c_uint8, memory, caller))
            free(caller, ptr3, len4, 1)
            variant = Ok(list)
        elif load == 1:
            ptr5 = load1
            len6 = load2
            list7 = _decode_utf8(memory, caller, ptr5, len6)
            free(caller, ptr5, len6, 1)
            variant = Err(list7)
        else:
            raise TypeError("invalid variant discriminant for expected")
        return variant

    def wasm2wat(self, caller: wasmtime.Store, wasm: bytes, features: WasmFeature) -> Expected[str, str]:
        memory = self._memory;
        realloc = self._canonical_abi_realloc
        free = self._canonical_abi_free
        ptr, len0 = _list_canon_lower(wasm, ctypes.c_uint8, 1, 1, realloc, memory, caller)
        ret = self._wasm2wat(caller, ptr, len0, (features).value)
        assert(isinstance(ret, int))
        load = _load(ctypes.c_int32, memory, caller, ret, 0)
        load1 = _load(ctypes.c_int32, memory, caller, ret, 8)
        load2 = _load(ctypes.c_int32, memory, caller, ret, 16)
        variant: Expected[str, str]
        if load == 0:
            ptr3 = load1
            len4 = load2
            list = _decode_utf8(memory, caller, ptr3, len4)
            free(caller, ptr3, len4, 1)
            variant = Ok(list)
        elif load == 1:
            ptr5 = load1
            len6 = load2
            list7 = _decode_utf8(memory, caller, ptr5, len6)
            free(caller, ptr5, len6, 1)
            variant = Err(list7)
        else:
            raise TypeError("invalid variant discriminant for expected")
        return variant
