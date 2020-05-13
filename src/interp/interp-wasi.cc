/*
 * Copyright 2020 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/interp/interp-wasi.h"
#include "src/interp/interp-util.h"

#ifdef WITH_WASI

#include "uvwasi.h"

#include <cinttypes>
#include <unordered_map>

using namespace wabt;
using namespace wabt::interp;

namespace {

// Types that align with WASI specis on size and alignment.
// TODO(sbc): Auto-generate this from witx.
typedef uint32_t __wasi_size_t;
typedef uint32_t __wasi_ptr_t;

_Static_assert(sizeof(__wasi_size_t) == 4, "witx calculated size");
_Static_assert(_Alignof(__wasi_size_t) == 4, "witx calculated align");
_Static_assert(sizeof(__wasi_ptr_t) == 4, "witx calculated size");
_Static_assert(_Alignof(__wasi_ptr_t) == 4, "witx calculated align");

struct __wasi_iovec_t {
  __wasi_ptr_t buf;
  __wasi_size_t buf_len;
};

_Static_assert(sizeof(__wasi_iovec_t) == 8, "witx calculated size");
_Static_assert(_Alignof(__wasi_iovec_t) == 4, "witx calculated align");
_Static_assert(offsetof(__wasi_iovec_t, buf) == 0, "witx calculated offset");
_Static_assert(offsetof(__wasi_iovec_t, buf_len) == 4, "witx calculated offset");

class WasiInstance {
 public:
  WasiInstance(Instance::Ptr instance,
               uvwasi_s* uvwasi,
               Memory* memory,
               Stream* trace_stream)
      : trace_stream(trace_stream),
        instance(instance),
        uvwasi(uvwasi),
        memory(memory) {}

  Result proc_exit(const Values& params, Values& results, Trap::Ptr* trap) {
    const Value arg0 = params[0];
    uvwasi_proc_exit(uvwasi, arg0.i32_);
    return Result::Ok;
  }

  Result fd_write(const Values& params, Values& results, Trap::Ptr* trap) {
    const Value arg0 = params[0];
    int32_t iovptr = params[1].i32_;
    int32_t iovcnt = params[2].i32_;
    __wasi_iovec_t* wasm_iovs = getMemPtr<__wasi_iovec_t>(iovptr, iovcnt, trap);
    if (!wasm_iovs) {
      return Result::Error;
    }
    uvwasi_ciovec_t* iovs = new uvwasi_ciovec_t[iovcnt];
    for (int i = 0; i < iovcnt; i++) {
      iovs[0].buf_len = wasm_iovs[0].buf_len;
      iovs[0].buf =
          getMemPtr<uint8_t>(wasm_iovs[0].buf, wasm_iovs[0].buf_len, trap);
      if (!iovs[0].buf) {
        return Result::Error;
      }
    }
    __wasi_ptr_t* out_addr = getMemPtr<__wasi_ptr_t>(params[3].i32_, 1, trap);
    if (!out_addr) {
      return Result::Error;
    }
    uvwasi_fd_write(uvwasi, arg0.i32_, iovs, iovcnt, out_addr);
    delete[] iovs;
    return Result::Ok;
  }

  // The trace stream accosiated with the instance.
  Stream* trace_stream;

  Instance::Ptr instance;
 private:
  template <typename T>
  T* getMemPtr(uint32_t address, uint32_t size, Trap::Ptr* trap) {
    if (!memory->IsValidAccess(address, 0, size * sizeof(T))) {
      *trap = Trap::New(*instance.store(),
                        StringPrintf("out of bounds memory access: "
                                     "[%u, %" PRIu64 ") >= max value %u",
                                     address, u64{address} + size * sizeof(T),
                                     memory->ByteSize()));
      return nullptr;
    }
    return reinterpret_cast<T*>(memory->UnsafeData() + address);
  }

  uvwasi_s* uvwasi;
  // The memory accociated with the instance.  Looked up once on startup
  // and cached here.
  Memory* memory;
};

std::unordered_map<Instance*, WasiInstance*> wasiInstances;

// TODO(sbc): Auto-generate this.

#define WASI_CALLBACK(NAME)                                          \
  Result NAME(Thread& thread, const Values& params, Values& results, \
              Trap::Ptr* trap) {                                     \
    Instance* instance = thread.GetCallerInstance();                 \
    assert(instance);                                                \
    WasiInstance* wasi_instance = wasiInstances[instance];           \
    if (wasi_instance->trace_stream) {                               \
      wasi_instance->trace_stream->Writef(                           \
          ">>> running wasi function \"%s\":\n", #NAME);             \
    }                                                                \
    return wasi_instance->NAME(params, results, trap);               \
  }

WASI_CALLBACK(proc_exit);
WASI_CALLBACK(fd_write);

}  // namespace

namespace wabt {
namespace interp {

Result WasiBindImports(const Module::Ptr& module,
                       RefVec& imports,
                       Stream* stream,
                       Stream* trace_stream) {
  Store* store = module.store();
  for (auto&& import : module->desc().imports) {
    if (import.type.type->kind != ExternKind::Func) {
      stream->Writef("wasi error: invalid import type: %s\n",
                     import.type.name.c_str());
      return Result::Error;
    }

    if (import.type.module != "wasi_snapshot_preview1") {
      stream->Writef("wasi error: unknown module import: `%s`\n",
                     import.type.module.c_str());
      return Result::Error;
    }

    auto func_type = *cast<FuncType>(import.type.type.get());
    auto import_name = StringPrintf("%s.%s", import.type.module.c_str(),
                                    import.type.name.c_str());
    HostFunc::Ptr host_func;

    // TODO(sbc): Auto-generate this.
    if (import.type.name == "proc_exit") {
      host_func = HostFunc::New(*store, func_type, proc_exit);
    } else if (import.type.name == "fd_write") {
      host_func = HostFunc::New(*store, func_type, fd_write);
    } else {
      stream->Writef("unknown wasi_snapshot_preview1 import: %s\n",
                     import.type.name.c_str());
      return Result::Error;
    }
    imports.push_back(host_func.ref());
  }

  return Result::Ok;
}

Result WasiRunStart(const Instance::Ptr& instance,
                    uvwasi_s* uvwasi,
                    Stream* err_stream,
                    Stream* trace_stream) {
  Store* store = instance.store();
  auto module = store->UnsafeGet<Module>(instance->module());
  auto&& module_desc = module->desc();

  Func::Ptr start;
  Memory::Ptr memory;
  for (auto&& export_ : module_desc.exports) {
    if (export_.type.name == "memory") {
      if (export_.type.type->kind != ExternalKind::Memory) {
        err_stream->Writef("wasi error: memory export has incorrect type\n");
        return Result::Error;
      }
      memory = store->UnsafeGet<Memory>(instance->memories()[export_.index]);
    }
    if (export_.type.name == "_start") {
      if (export_.type.type->kind != ExternalKind::Func) {
        err_stream->Writef("wasi error: _start export is not a function\n");
        return Result::Error;
      }
      start = store->UnsafeGet<Func>(instance->funcs()[export_.index]);
    }
    if (start && memory) {
      break;
    }
  }

  if (!start) {
    err_stream->Writef("wasi error: _start export not found\n");
    return Result::Error;
  }

  if (!memory) {
    err_stream->Writef("wasi error: memory export not found\n");
    return Result::Error;
  }

  if (start->type().params.size() || start->type().results.size()) {
    err_stream->Writef("wasi error: invalid _start signature\n");
    return Result::Error;
  }

  // Register memory
  auto* wasi = new WasiInstance(instance, uvwasi, memory.get(), trace_stream);
  wasiInstances[instance.get()] = wasi;

  // Call start ([] -> [])
  Values params;
  Values results;
  Trap::Ptr trap;
  Result res = start->Call(*store, params, results, &trap, trace_stream);
  if (trap) {
    WriteTrap(err_stream, "error", trap);
  }

  // Unregister memory
  wasiInstances.erase(instance.get());
  delete wasi;
  return res;
}

}  // namespace interp
}  // namespace wabt

#endif
