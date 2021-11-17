from wasmer import Store, Module, wasi, ImportObject, Function, FunctionType, Type
from input import WABT, WasmFeature
import os

__dir__ = os.path.dirname(os.path.realpath(__file__))
contents = open(__dir__ + '/../../out/wasi/Debug/libwabt.wasm', 'rb').read()

store = Store()
module = Module(store, contents)

# def not_implemented():
#     raise NotImplementedError("The import function is not ")

# import_object = ImportObject()

# import_object.register(
#     "wasi_snapshot_preview1",
#     {
#         "environ_get": Function(store, not_implemented, FunctionType([Type.I32, Type.I32], [Type.I32])),
#         "environ_sizes_get": Function(store, not_implemented, FunctionType([Type.I32, Type.I32], [Type.I32])),
#         "fd_close": Function(store, not_implemented, FunctionType([Type.I32], [Type.I32])),
#         "fd_fdstat_get": Function(store, not_implemented, FunctionType([Type.I32, Type.I32], [Type.I32])),
#         "fd_seek": Function(store, not_implemented, FunctionType([Type.I32, Type.I64, Type.I32, Type.I32], [Type.I32])),
#         "fd_write": Function(store, not_implemented, FunctionType([Type.I32, Type.I32, Type.I32, Type.I32], [Type.I32])),
#         "proc_exit": Function(store, not_implemented, FunctionType([Type.I32], [])),
#     }
# )

wasi_version = wasi.get_version(module, strict=True)
wasi_env = wasi.StateBuilder('wasi_test_program').finalize()
import_object = wasi_env.generate_import_object(store, wasi_version)


wabt = WABT()
wabt.instantiate(module, import_object)


def wat2wasm(contents):
    print(f"trying {contents}")
    try:
        result = wabt.wat2wasm(contents, WasmFeature.MutableGlobals)
        print(result)
    except Exception as e:
        print("error", e)

def wasm2wat(contents):
    print(f"trying {contents}")
    try:
        result = wabt.wasm2wat(contents, WasmFeature.MutableGlobals)
        print(result)
    except Exception as e:
        print("error", e)


wat2wasm("(module)");
wat2wasm("xyz");
wasm2wat(bytearray([
    0, 97, 115, 109,   1,   0,   0,   0,   0,
    8,  4, 110,  97, 109, 101,   2,   1,   0,
    0,  9,   7, 108, 105, 110, 107, 105, 110,
  103,  2
]))
