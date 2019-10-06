/*
 * Copyright 2019 WebAssembly Community Group participants
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

#include <wasm.h>

#include "src/binary-reader.h"
#include "src/error-formatter.h"
#include "src/error.h"
#include "src/interp/binary-reader-interp.h"
#include "src/interp/binary-reader-metadata.h"
#include "src/interp/interp.h"
#include "src/ir.h"
#include "src/stream.h"

using namespace wabt;
using namespace wabt::interp;

#ifndef NDEBUG
#define TRACE(str, ...) fprintf(stderr, "CAPI: " str, ##__VA_ARGS__)
#else
#define TRACE(...)
#endif

static Features s_features;
static Stream* s_trace_stream;
static Thread::Options s_thread_options;
static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_stdout_stream;

struct wasm_valtype_t {
  wasm_valkind_t kind;
};

struct wasm_config_t {};

struct wasm_externtype_t {
  wasm_externkind_t kind;

 protected:
  wasm_externtype_t(wasm_externkind_t kind) : kind(kind) {}
};

struct wasm_globaltype_t : wasm_externtype_t {
  wasm_globaltype_t(wasm_valtype_t type, bool mutable_)
      : wasm_externtype_t(WASM_EXTERN_GLOBAL), type(type), mutable_(mutable_) {}
  wasm_valtype_t type;
  bool mutable_;
};

struct wasm_tabletype_t : wasm_externtype_t {
  wasm_tabletype_t(wasm_valtype_t elemtype, wasm_limits_t limits)
      : wasm_externtype_t(WASM_EXTERN_TABLE),
        elemtype(elemtype),
        limits(limits) {}
  wasm_valtype_t elemtype;
  wasm_limits_t limits;
};

struct wasm_memorytype_t : wasm_externtype_t {
  wasm_memorytype_t(wasm_limits_t limits)
      : wasm_externtype_t(WASM_EXTERN_MEMORY), limits(limits) {}
  wasm_limits_t limits;
};

struct wasm_importtype_t {};

struct wasm_exporttype_t {
  ~wasm_exporttype_t() { delete type; }

  wasm_name_t name;
  wasm_externtype_t* type;
};

struct wasm_engine_t {};

struct wasm_store_t {
  wasm_store_t(Environment* env, Executor* executor)
      : env(env), executor(executor) {}

  ~wasm_store_t() {
    TRACE("~store\n");
    delete executor;
  }

  Environment* env;
  Executor* executor;
};

enum class WasmRefType { Extern, Module, Instance, Trap, Foreign };

std::string WasmRefTypeToString(WasmRefType t) {
  switch (t) {
    case WasmRefType::Extern:
      return "extern";
    case WasmRefType::Module:
      return "module";
    case WasmRefType::Instance:
      return "instance";
    case WasmRefType::Trap:
      return "trap";
    case WasmRefType::Foreign:
      return "foreign";
  }
  WABT_UNREACHABLE;
}

struct wasm_ref_t {
  wasm_ref_t(WasmRefType kind) : kind(kind) {}
  wasm_ref_t(const wasm_ref_t& other)
      : kind(other.kind),
        host_info(other.host_info),
        finalizer(other.finalizer) {}

  virtual ~wasm_ref_t() {
    if (finalizer) {
      finalizer(host_info);
    }
  }

  virtual wasm_ref_t* Copy() const { return new wasm_ref_t(kind); }

  bool Same(const wasm_ref_t& other) const;

  WasmRefType kind;
  void* host_info = nullptr;
  void (*finalizer)(void*) = nullptr;
};

struct wasm_extern_ref_t : wasm_ref_t {
  wasm_extern_ref_t(ExternalKind kind, Index index)
      : wasm_ref_t(WasmRefType::Extern), kind(kind), index(index) {}

  wasm_extern_ref_t(const wasm_extern_ref_t& other)
      : wasm_ref_t(other), kind(other.kind), index(other.index) {}

  wasm_ref_t* Copy() const override { return new wasm_extern_ref_t(*this); }

  bool Same(const wasm_extern_ref_t& other) const {
    return kind == other.kind && index == other.index;
  }

  ExternalKind kind;
  Index index;
};

struct wasm_foreign_ref_t : wasm_ref_t {
  wasm_foreign_ref_t(Index index) : wasm_ref_t(WasmRefType::Foreign), index(index) {}
  wasm_foreign_ref_t(const wasm_foreign_ref_t& other) : wasm_ref_t(other), index(other.index) {}

  wasm_ref_t* Copy() const override { return new wasm_foreign_ref_t(*this); }

  Index index;
};

struct wasm_foreign_t : wasm_foreign_ref_t {
  wasm_foreign_t(Index index) : wasm_foreign_ref_t(index) {}

  bool Same(const wasm_foreign_t& other) const {
    TRACE("wasm_foreign_t %u %u\n", index, other.index);
    return index == other.index;
  }
};

struct WasmInstance {
  WasmInstance(wasm_store_t* store, DefinedModule* module)
      : store(store), module(module) {}

  ~WasmInstance() {
    TRACE("~WasmInstance\n");
    delete module;
  }
  wasm_store_t* store;
  DefinedModule* module;
};

struct wasm_instance_t : wasm_ref_t {
  wasm_instance_t(wasm_store_t* store, interp::DefinedModule* module)
      : wasm_ref_t(WasmRefType::Instance),
        ptr(std::make_shared<WasmInstance>(store, module)) {}

  wasm_instance_t(std::shared_ptr<WasmInstance> ptr)
      : wasm_ref_t(WasmRefType::Instance), ptr(ptr) {}

  wasm_instance_t(const wasm_instance_t& other)
      : wasm_ref_t(other), ptr(other.ptr) {}

  wasm_ref_t* Copy() const override { return new wasm_instance_t(*this); }

  bool Same(const wasm_instance_t& other) const { return ptr == other.ptr; }

  std::shared_ptr<WasmInstance> ptr;
};

struct wasm_module_t : wasm_ref_t {
  wasm_module_t(wasm_store_t* store,
                const wasm_byte_vec_t* in,
                ModuleMetadata* metadata)
      : wasm_ref_t(WasmRefType::Module), store(store), metadata(metadata) {
    wasm_byte_vec_copy(&binary, in);
  }

  bool Same(const wasm_module_t& other) const {
    assert(false);
    return true;
  }

  ~wasm_module_t() {
    TRACE("~module\n");
    wasm_byte_vec_delete(&binary);
    delete metadata;
  }

  wasm_store_t* store;
  wasm_byte_vec_t binary;
  ModuleMetadata* metadata;
};

struct wasm_shared_module_t : wasm_module_t {};

bool wasm_ref_t::Same(const wasm_ref_t& other) const {
  TRACE("wasm_ref_t::Same kind=%d other=%d\n", static_cast<int>(kind),
        static_cast<int>(other.kind));
  if (kind != other.kind)
    return false;
  TRACE("wasm_ref_t::Same x\n");
  if (other.host_info != host_info && other.finalizer != finalizer)
    return false;
  TRACE("wasm_ref_t::Same 2\n");
  switch (kind) {
    case WasmRefType::Extern:
      return static_cast<const wasm_extern_ref_t*>(this)->Same(
          static_cast<const wasm_extern_ref_t&>(other));
    case WasmRefType::Instance:
      return static_cast<const wasm_instance_t*>(this)->Same(
          static_cast<const wasm_instance_t&>(other));
    case WasmRefType::Module:
      return static_cast<const wasm_module_t*>(this)->Same(
          static_cast<const wasm_module_t&>(other));
    case WasmRefType::Foreign:
      return static_cast<const wasm_foreign_t*>(this)->Same(
          static_cast<const wasm_foreign_t&>(other));
    case WasmRefType::Trap:
      return true;
  }
  WABT_UNREACHABLE;
}

struct wasm_frame_t {
  wasm_instance_t* instance;
  size_t offset;
  uint32_t func_index;
};

// Type conversion utilities
static Type ToWabtType(wasm_valkind_t kind);
static wasm_valkind_t FromWabtType(Type type);

Limits ToWabtLimits(const wasm_limits_t& limits) {
  return Limits(limits.min, limits.max);
}

wasm_limits_t FromWabtLimits(const Limits& limits) {
  return wasm_limits_t{(uint32_t)limits.initial, (uint32_t)limits.max};
}

// Value conversion utilities
static TypedValue ToWabtValue(const wasm_val_t& value);
static void ToWabtValues(TypedValues& out_values,
                         const wasm_val_t values[],
                         size_t count);
static wasm_val_t FromWabtValue(std::shared_ptr<WasmInstance> instance,
                                const TypedValue& value);
static void FromWabtValues(std::shared_ptr<WasmInstance> instance,
                           wasm_val_t values[],
                           const TypedValues& wabt_values);

struct wasm_functype_t : wasm_externtype_t {
  wasm_functype_t(interp::FuncSignature& sig)
      : wasm_externtype_t(WASM_EXTERN_FUNC) {
    TRACE("new wasm_functype_t %" PRIzx " -> %" PRIzx "\n", sig.param_types.size(),
          sig.result_types.size());
    wasm_valtype_vec_new_uninitialized(&params, sig.param_types.size());
    wasm_valtype_vec_new_uninitialized(&results, sig.result_types.size());

    for (size_t i = 0; i < sig.param_types.size(); i++) {
      params.data[i] = new wasm_valtype_t{FromWabtType(sig.param_types[i])};
    }
    for (size_t i = 0; i < sig.result_types.size(); i++) {
      results.data[i] = new wasm_valtype_t{FromWabtType(sig.result_types[i])};
    }
  }

  wasm_functype_t(wasm_valtype_vec_t* params, wasm_valtype_vec_t* results)
      : wasm_externtype_t(WASM_EXTERN_FUNC),
        params(*params),
        results(*results) {}

  ~wasm_functype_t() {
    TRACE("~wasm_functype_t\n");
    wasm_valtype_vec_delete(&params);
    wasm_valtype_vec_delete(&results);
  }

  interp::FuncSignature Sig() const {
    std::vector<Type> param_vec;
    std::vector<Type> result_vec;
    for (size_t i = 0; i < params.size; i++) {
      param_vec.push_back(ToWabtType(wasm_valtype_kind(params.data[i])));
    }
    for (size_t i = 0; i < results.size; i++) {
      result_vec.push_back(ToWabtType(wasm_valtype_kind(results.data[i])));
    }
    return interp::FuncSignature{param_vec, result_vec};
  }

  wasm_valtype_vec_t params;
  wasm_valtype_vec_t results;
};

struct wasm_extern_t : wasm_extern_ref_t {
  virtual ~wasm_extern_t() = default;

  interp::Environment* Env() const { return instance.get()->store->env; }

  wasm_extern_t(const wasm_extern_t& other)
      : wasm_extern_ref_t(other), instance(other.instance) {}

  virtual wasm_externtype_t* ExternType() const = 0;

  std::shared_ptr<WasmInstance> instance;

 protected:
  wasm_extern_t(std::shared_ptr<WasmInstance> instance,
                ExternalKind kind,
                Index index)
      : wasm_extern_ref_t(kind, index), instance(instance) {}
};

struct wasm_func_t : wasm_extern_t {
  wasm_func_t(std::shared_ptr<WasmInstance> instance, Index index)
      : wasm_extern_t(instance, ExternalKind::Func, index) {}

  interp::Func* GetFunc() const { return Env()->GetFunc(index); }

  interp::FuncSignature* GetSig() const {
    return Env()->GetFuncSignature(GetFunc()->sig_index);
  }

  wasm_externtype_t* ExternType() const override {
    return new wasm_functype_t(*GetSig());
  }
};

struct wasm_global_t : wasm_extern_t {
  wasm_global_t(std::shared_ptr<WasmInstance> instance, Index index)
      : wasm_extern_t(instance, ExternalKind::Global, index) {}

  interp::Global* GetGlobal() const { return Env()->GetGlobal(index); }

  wasm_externtype_t* ExternType() const override {
    auto* g = GetGlobal();
    return new wasm_globaltype_t({FromWabtType(g->type)}, g->mutable_);
  }
};

struct wasm_table_t : wasm_extern_t {
  wasm_table_t(std::shared_ptr<WasmInstance> instance, Index index)
      : wasm_extern_t(instance, ExternalKind::Table, index) {}

  interp::Table* GetTable() const { return Env()->GetTable(index); }

  wasm_externtype_t* ExternType() const override {
    auto* t = GetTable();
    return new wasm_tabletype_t({FromWabtType(t->elem_type)},
                                FromWabtLimits(t->limits));
  }
};

struct wasm_memory_t : wasm_extern_t {
  wasm_memory_t(std::shared_ptr<WasmInstance> instance, Index index)
      : wasm_extern_t(instance, ExternalKind::Memory, index) {}

  interp::Memory* GetMemory() const {
    auto* env = instance.get()->store->env;
    return env->GetMemory(index);
  }

  wasm_externtype_t* ExternType() const override {
    auto* mem = GetMemory();
    return new wasm_memorytype_t(FromWabtLimits(mem->page_limits));
  }
};

struct wasm_trap_t : wasm_ref_t {
  wasm_trap_t(const wasm_message_t* msg) : wasm_ref_t(WasmRefType::Trap) {
    wasm_name_copy(&message, msg);
  }
  wasm_message_t message;
};

// Helper functions

static Type ToWabtType(wasm_valkind_t kind) {
  switch (kind) {
    case WASM_I32:
      return Type::I32;
    case WASM_I64:
      return Type::I64;
    case WASM_F32:
      return Type::F32;
    case WASM_F64:
      return Type::F64;
    case WASM_ANYREF:
      return Type::Anyref;
    case WASM_FUNCREF:
      return Type::Funcref;
    default:
      TRACE("unexpected wasm_valkind_t: %d\n", kind);
      WABT_UNREACHABLE;
  }
  WABT_UNREACHABLE;
}

static wasm_valkind_t FromWabtType(Type type) {
  switch (type) {
    case Type::I32:
      return WASM_I32;
    case Type::I64:
      return WASM_I64;
    case Type::F32:
      return WASM_F32;
    case Type::F64:
      return WASM_F64;
    case Type::Anyref:
      return WASM_ANYREF;
    case Type::Funcref:
      return WASM_FUNCREF;
    default:
      WABT_UNREACHABLE;
  }
}

static TypedValue ToWabtValue(const wasm_val_t& value) {
  TypedValue out(ToWabtType(value.kind));
  switch (value.kind) {
    case WASM_I32:
      out.set_i32(value.of.i32);
      break;
    case WASM_I64:
      out.set_i64(value.of.i64);
      break;
    case WASM_F32:
      out.set_f32(value.of.f32);
      break;
    case WASM_F64:
      out.set_f64(value.of.f64);
      break;
    case WASM_FUNCREF: {
      auto* ext = static_cast<wasm_extern_t*>(value.of.ref);
      assert(ext->kind == ExternalKind::Func);
      out.set_ref(Ref{RefType::Func, ext->index});
      break;
    }
    case WASM_ANYREF: {
      wasm_ref_t* ref = value.of.ref;
      if (!ref) {
        out.set_ref(Ref{RefType::Null, kInvalidIndex});
        break;
      }
      if (ref->kind == WasmRefType::Foreign) {
        auto* f = static_cast<wasm_foreign_t*>(ref);
        out.set_ref(Ref{RefType::Host, f->index});
        out.type = Type::Hostref;
        break;
      }
      printf("unexpected WASM_ANYREF in ToWabtValue: %d\n",
             static_cast<int>(ref->kind));
      WABT_UNREACHABLE;
      break;
    }
    default:
      TRACE("unexpected wasm type: %d\n", value.kind);
      assert(false);
  }
  TRACE("ToWabtValue -> %s\n", TypedValueToString(out).c_str());
  return out;
}

static void ToWabtValues(TypedValues& wabt_values,
                         const wasm_val_t values[],
                         size_t count) {
  for (size_t i = 0; i < count; i++) {
    wabt_values.push_back(ToWabtValue(values[i]));
  }
}

// wasm_byte_vec

static wasm_val_t FromWabtValue(std::shared_ptr<WasmInstance> instance,
                                const TypedValue& value) {
  TRACE("FromWabtValue: %s\n", TypedValueToString(value).c_str());
  wasm_val_t out_value;
  switch (value.type) {
    case Type::I32:
      out_value.kind = WASM_I32;
      out_value.of.i32 = value.get_i32();
      break;
    case Type::I64:
      out_value.kind = WASM_I64;
      out_value.of.i64 = value.get_i64();
      break;
    case Type::F32:
      out_value.kind = WASM_F32;
      out_value.of.f32 = value.get_f32();
      break;
    case Type::F64:
      out_value.kind = WASM_F64;
      out_value.of.f64 = value.get_f64();
      break;
    case Type::Funcref:
      out_value.kind = WASM_FUNCREF;
      out_value.of.ref = new wasm_func_t(instance, value.get_ref().index);
      break;
    case Type::Hostref:
      out_value.kind = WASM_ANYREF;
      out_value.of.ref = new wasm_foreign_t(value.get_ref().index);
      break;
    case Type::Nullref:
      out_value.kind = WASM_ANYREF;
      out_value.of.ref = nullptr;
      break;
    default:
      TRACE("unexpected wabt type: %d\n", static_cast<int>(value.type));
      assert(false);
  }
  return out_value;
}

static void FromWabtValues(std::shared_ptr<WasmInstance> instance,
                           wasm_val_t values[],
                           const TypedValues& wabt_values) {
  for (size_t i = 0; i < wabt_values.size(); i++) {
    values[i] = FromWabtValue(instance, wabt_values[i]);
  }
}

static wasm_externkind_t FromWabtExternalKind(ExternalKind kind) {
  switch (kind) {
    case ExternalKind::Func:
      return WASM_EXTERN_FUNC;
    case ExternalKind::Global:
      return WASM_EXTERN_GLOBAL;
    case ExternalKind::Table:
      return WASM_EXTERN_TABLE;
    case ExternalKind::Memory:
      return WASM_EXTERN_MEMORY;
    case ExternalKind::Event:
      WABT_UNREACHABLE;
  }
  WABT_UNREACHABLE;
}

static ReadBinaryOptions GetOptions() {
  const bool kReadDebugNames = true;
  const bool kStopOnFirstError = true;
  const bool kFailOnCustomSectionError = true;
  s_features.EnableAll();
  if (getenv("WASM_API_DEBUG") != nullptr) {
    s_log_stream = FileStream::CreateStdout();
  }
  return ReadBinaryOptions(s_features, s_log_stream.get(), kReadDebugNames,
                           kStopOnFirstError, kFailOnCustomSectionError);
}

extern "C" {

// wasm_valtype

wasm_valtype_t* wasm_valtype_new(wasm_valkind_t kind) {
  return new wasm_valtype_t{kind};
}

wasm_valkind_t wasm_valtype_kind(const wasm_valtype_t* type) {
  assert(type);
  return type->kind;
}

// Helpers

static void print_sig(const interp::FuncSignature& sig) {
#ifndef NDEBUG
  fprintf(stderr, "(");
  bool first = true;
  for (auto Type : sig.param_types) {
    if (!first) {
      fprintf(stderr, ", ");
    }
    first = false;
    fprintf(stderr, "%s", GetTypeName(Type));
  }
  fprintf(stderr, ") -> (");
  first = true;
  for (auto Type : sig.result_types) {
    if (!first) {
      fprintf(stderr, ", ");
    }
    first = false;
    fprintf(stderr, "%s", GetTypeName(Type));
  }
  fprintf(stderr, ")\n");
#endif
}

// wasm_val

void wasm_val_delete(wasm_val_t* val) {
  assert(val);
  if (wasm_valkind_is_ref(val->kind)) {
    delete val->of.ref;
    val->of.ref = nullptr;
  }
}

void wasm_val_copy(wasm_val_t* out, const wasm_val_t* in) {
  TRACE("wasm_val_copy %s\n", TypedValueToString(ToWabtValue(*in)).c_str());
  *out = *in;
  TRACE("wasm_val_copy -> %p %s\n", out,
        TypedValueToString(ToWabtValue(*out)).c_str());
}

// wasm_trap

wasm_trap_t* wasm_trap_new(wasm_store_t* store, const wasm_message_t* msg) {
  return new wasm_trap_t(msg);
}

void wasm_trap_message(const wasm_trap_t* trap, wasm_message_t* out) {
  assert(trap);
  wasm_name_copy(out, &trap->message);
}

wasm_frame_t* wasm_trap_origin(const wasm_trap_t* trap) {
  // TODO(sbc): Implement stack traces
  return nullptr;
}

void wasm_trap_trace(const wasm_trap_t* trap, wasm_frame_vec_t* out) {
  assert(trap);
  assert(out);
  wasm_frame_vec_new_empty(out);
  // std::string msg(trap->message.data, trap->message.size);
  // fTRACE(stderr, "error: %s\n", msg.c_str());
  return;
}

// wasm_engine

wasm_engine_t* wasm_engine_new() {
  return new wasm_engine_t();
}

wasm_engine_t* wasm_engine_new_with_config(wasm_config_t*) {
  assert(false);
  return nullptr;
}

// wasm_store

wasm_store_t* wasm_store_new(wasm_engine_t* engine) {
  assert(engine);
  if (!s_trace_stream) {
    s_trace_stream = s_stdout_stream.get();
  }
  Environment* env = new Environment(s_features);
  Executor* executor = new Executor(env, s_trace_stream, s_thread_options);
  return new wasm_store_t(env, executor);
}

// wasm_module

wasm_module_t* wasm_module_new(wasm_store_t* store,
                               const wasm_byte_vec_t* binary) {
  Errors errors;
  ModuleMetadata* metadata = nullptr;
  wabt::Result result = ReadBinaryMetadata(binary->data, binary->size,
                                           GetOptions(), &errors, &metadata);
  if (!Succeeded(result)) {
    return nullptr;
  }
  return new wasm_module_t(store, binary, metadata);
}

void wasm_module_exports(const wasm_module_t* module,
                         wasm_exporttype_vec_t* out) {
  size_t num_exports = module->metadata->exports.size();
  TRACE("wasm_module_exports: %" PRIzx "\n", num_exports);
  wasm_exporttype_vec_new_uninitialized(out, num_exports);
  auto* env = module->store->env;
  for (size_t i = 0; i < num_exports; i++) {
    interp::Export& exp = module->metadata->exports[i];
    wasm_name_t name;
    wasm_name_new_from_string(&name, exp.name.c_str());
    TRACE("module_export: %s\n", exp.name.c_str());
    switch (exp.kind) {
      case ExternalKind::Func: {
        interp::Func* func = env->GetFunc(exp.index);
        auto* type =
            new wasm_functype_t(*env->GetFuncSignature(func->sig_index));
        out->data[i] = new wasm_exporttype_t{name, type};
        break;
      }
      case ExternalKind::Global: {
        auto* g = env->GetGlobal(exp.index);
        auto* type =
            new wasm_globaltype_t({FromWabtType(g->type)}, g->mutable_);
        out->data[i] = new wasm_exporttype_t{name, type};
        break;
      }
      case ExternalKind::Table: {
        auto* t = env->GetTable(exp.index);
        auto* type = new wasm_tabletype_t({FromWabtType(t->elem_type)},
                                          FromWabtLimits(t->limits));
        out->data[i] = new wasm_exporttype_t{name, type};
        break;
      }
      case ExternalKind::Memory: {
        auto* mem = env->GetMemory(exp.index);
        auto* type = new wasm_memorytype_t(FromWabtLimits(mem->page_limits));
        out->data[i] = new wasm_exporttype_t{name, type};
        break;
      }
      case ExternalKind::Event:
        WABT_UNREACHABLE;
    }
  }
}

void wasm_module_serialize(const wasm_module_t* module, wasm_byte_vec_t* out) {
  wasm_byte_vec_copy(out, &module->binary);
}

wasm_module_t* wasm_module_deserialize(wasm_store_t* store,
                                       const wasm_byte_vec_t* bytes) {
  return wasm_module_new(store, bytes);
}

// wasm_exporttype

wasm_exporttype_t* wasm_exporttype_new(wasm_name_t* name,
                                       wasm_externtype_t* type) {
  return new wasm_exporttype_t{*name, type};
}

const wasm_name_t* wasm_exporttype_name(const wasm_exporttype_t* ex) {
  return &ex->name;
}

const wasm_externtype_t* wasm_exporttype_type(const wasm_exporttype_t* ex) {
  TRACE("wasm_exporttype_type %p\n", ex);
  assert(ex);
  return ex->type;
}

// wasm_instance

wasm_instance_t* wasm_instance_new(wasm_store_t* store,
                                   const wasm_module_t* module,
                                   const wasm_extern_t* const imports[],
                                   wasm_trap_t** trap_out) {
  TRACE("wasm_instance_new: %p %p\n", store, module);
  assert(module);
  assert(module->metadata);
  assert(store);
  assert(store->env);

  Errors errors;
  interp::DefinedModule* interp_module = nullptr;
  std::vector<interp::Export> wabt_imports;
  std::vector<interp::Export*> wabt_import_ptrs;

  for (size_t i = 0; i < module->metadata->imports.size(); i++) {
    wabt_imports.emplace_back("", imports[i]->kind, imports[i]->index);
  }

  for (auto& i : wabt_imports) {
    wabt_import_ptrs.push_back(&i);
  }

  wabt::Result result =
      ReadBinaryInterp(store->env, module->binary.data, module->binary.size,
                       GetOptions(), wabt_import_ptrs, &errors, &interp_module);

  FormatErrorsToFile(errors, Location::Type::Binary);
  if (!Succeeded(result)) {
    TRACE("ReadBinaryInterp failed!\n");
    return nullptr;
  }

  ExecResult exec_result = store->executor->Initialize(interp_module);
  if (!exec_result.ok()) {
    TRACE("Initialize failed!\n");
    wasm_name_t msg;
    wasm_name_new_from_string(&msg, ResultToString(exec_result.result).c_str());
    wasm_trap_t* trap = wasm_trap_new(store, &msg);
    *trap_out = trap;
    return nullptr;
  }
  return new wasm_instance_t(store, interp_module);
}

void wasm_instance_exports(const wasm_instance_t* instance,
                           wasm_extern_vec_t* out) {
  WasmInstance& wasm_instance = *instance->ptr.get();
  interp::DefinedModule* module =
      const_cast<interp::DefinedModule*>(wasm_instance.module);
  size_t num_exports = module->exports.size();
  wasm_extern_vec_new_uninitialized(out, num_exports);
  TRACE("wasm_instance_exports: %" PRIzx "\n", num_exports);

  for (size_t i = 0; i < num_exports; i++) {
    interp::Export* export_ = &module->exports[i];
    TRACE("getexport: '%s' %d\n", export_->name.c_str(),
          static_cast<int>(export_->kind));
    switch (export_->kind) {
      case ExternalKind::Func:
        out->data[i] = new wasm_func_t(instance->ptr, export_->index);
        break;
      case ExternalKind::Global:
        out->data[i] = new wasm_global_t(instance->ptr, export_->index);
        break;
      case ExternalKind::Table:
        out->data[i] = new wasm_table_t(instance->ptr, export_->index);
        break;
      case ExternalKind::Memory:
        out->data[i] = new wasm_memory_t(instance->ptr, export_->index);
        break;
      case ExternalKind::Event:
        WABT_UNREACHABLE;
    }
  }
}

// wasm_functype

wasm_functype_t* wasm_functype_new(wasm_valtype_vec_t* params,
                                   wasm_valtype_vec_t* results) {
  TRACE("wasm_functype_new params=%" PRIzx " args=%" PRIzx "\n", params->size, results->size);
  auto* res = new wasm_functype_t(params, results);
  interp::FuncSignature sig = res->Sig();
  TRACE("wasm_functype_new -> ");
  print_sig(sig);
  return res;
}

const wasm_valtype_vec_t* wasm_functype_params(const wasm_functype_t* f) {
  TRACE("wasm_functype_params: %" PRIzx "\n", f->params.size);
  return &f->params;
}

const wasm_valtype_vec_t* wasm_functype_results(const wasm_functype_t* f) {
  return &f->results;
}

// wasm_func

static wasm_func_t* do_wasm_func_new(
    wasm_store_t* store,
    const wasm_functype_t* type,
    wasm_func_callback_t host_callback,
    wasm_func_callback_with_env_t host_callback_with_env,
    void* env,
    void (*finalizer)(void*)) {
  TRACE("do_wasm_func_new\n");
  type->Sig();
  auto instance = std::make_shared<WasmInstance>(store, nullptr);

  HostFunc::Callback callback =
      [instance, host_callback, host_callback_with_env, env](
          const HostFunc* func, const interp::FuncSignature* sig,
          const TypedValues& args, TypedValues& results) -> interp::Result {
    TRACE("calling host function: ");
    print_sig(*sig);
    wasm_val_t* host_args = new wasm_val_t[args.size()];
    wasm_val_t* host_results = new wasm_val_t[sig->result_types.size()];
    FromWabtValues(instance, host_args, args);
    wasm_trap_t* trap;
    if (host_callback) {
      trap = host_callback(host_args, host_results);
    } else {
      assert(host_callback_with_env);
      trap = host_callback_with_env(env, host_args, host_results);
    }
    if (trap) {
      TRACE("host function trapped\n");
      delete[] host_args;
      delete[] host_results;
      return interp::ResultType::TrapHostTrapped;
    }
    TRACE("adding result types: %" PRIzx "\n", sig->result_types.size());
    for (size_t i = 0; i < sig->result_types.size(); i++) {
      TRACE("result: %p\n", &host_results[i]);
      results[i] = ToWabtValue(host_results[i]);
      TRACE("added result value: %s\n", TypedValueToString(results[i]).c_str());
    }
    delete[] host_args;
    delete[] host_results;
    return interp::ResultType::Ok;
  };

  type->Sig();
  type->Sig();
  static int function_count = 0;
  std::string name = std::string("function") + std::to_string(function_count++);
  store->env->EmplaceBackFuncSignature(type->Sig());
  Index sig_index = store->env->GetFuncSignatureCount() - 1;
  auto* host_func = new HostFunc("extern", name, sig_index, callback);
  store->env->EmplaceBackFunc(host_func);
  Index func_index = store->env->GetFuncCount() - 1;
  return new wasm_func_t(instance, func_index);
}

wasm_func_t* wasm_func_new(wasm_store_t* store,
                           const wasm_functype_t* type,
                           wasm_func_callback_t callback) {
  return do_wasm_func_new(store, type, callback, nullptr, nullptr, nullptr);
}

wasm_func_t* wasm_func_new_with_env(wasm_store_t* store,
                                    const wasm_functype_t* type,
                                    wasm_func_callback_with_env_t callback,
                                    void* env,
                                    void (*finalizer)(void*)) {
  return do_wasm_func_new(store, type, nullptr, callback, env, finalizer);
}

wasm_functype_t* wasm_func_type(const wasm_func_t* func) {
  TRACE("wasm_func_type\n");
  return new wasm_functype_t(*func->GetSig());
}

size_t wasm_func_result_arity(const wasm_func_t* func) {
  interp::Func* wabt_func = func->GetFunc();
  auto* env = func->instance.get()->store->env;
  interp::FuncSignature* sig = env->GetFuncSignature(wabt_func->sig_index);
  return sig->result_types.size();
}

size_t wasm_func_param_arity(const wasm_func_t* func) {
  interp::Func* wabt_func = func->GetFunc();
  auto* env = func->instance.get()->store->env;
  interp::FuncSignature* sig = env->GetFuncSignature(wabt_func->sig_index);
  return sig->param_types.size();
}

wasm_trap_t* wasm_func_call(const wasm_func_t* f,
                            const wasm_val_t args[],
                            wasm_val_t results[]) {
  TRACE("wasm_func_call %d\n", f->index);
  assert(f);
  wasm_store_t* store = f->instance.get()->store;
  assert(store);
  Executor* exec = store->executor;
  wasm_functype_t* functype = wasm_func_type(f);
  TypedValues wabt_args;
  ToWabtValues(wabt_args, args, functype->params.size);
  wasm_functype_delete(functype);
  assert(f->kind == ExternalKind::Func);
  ExecResult res = exec->RunFunction(f->index, wabt_args);
  if (!res.ok()) {
    std::string msg = ResultToString(res.result);
    TRACE("wasm_func_call failed: %s\n", msg.c_str());
    wasm_name_t message;
    wasm_name_new_from_string(&message, msg.c_str());
    wasm_trap_t* trap = wasm_trap_new(store, &message);
    wasm_name_delete(&message);
    return trap;
  }
  FromWabtValues(f->instance, results, res.values);
  return nullptr;
}

// wasm_globaltype

wasm_globaltype_t* wasm_globaltype_new(wasm_valtype_t* type,
                                       wasm_mutability_t mut) {
  assert(type);
  return new wasm_globaltype_t(*type, mut == WASM_VAR);
}

wasm_mutability_t wasm_globaltype_mutability(const wasm_globaltype_t* type) {
  assert(type);
  return type->mutable_;
}

const wasm_valtype_t* wasm_globaltype_content(const wasm_globaltype_t* type) {
  assert(type);
  return &type->type;
}

// wasm_tabletype

wasm_tabletype_t* wasm_tabletype_new(wasm_valtype_t* type,
                                     const wasm_limits_t* limits) {
  return new wasm_tabletype_t(*type, *limits);
}

const wasm_valtype_t* wasm_tabletype_element(const wasm_tabletype_t* type) {
  return &type->elemtype;
}

const wasm_limits_t* wasm_tabletype_limits(const wasm_tabletype_t* type) {
  return &type->limits;
}

// wasm_memorytype

wasm_memorytype_t* wasm_memorytype_new(const wasm_limits_t* limits) {
  return new wasm_memorytype_t(*limits);
}

const wasm_limits_t* wasm_memorytype_limits(const wasm_memorytype_t* t) {
  return &t->limits;
}

// wasm_global

wasm_global_t* wasm_global_new(wasm_store_t* store,
                               const wasm_globaltype_t* type,
                               const wasm_val_t* val) {
  assert(store && store && type);
  TypedValue value = ToWabtValue(*val);
  TRACE("wasm_global_new: %s\n", TypedValueToString(value).c_str());
  interp::Global* g = store->env->EmplaceBackGlobal(value.type, type->mutable_);
  g->typed_value = value;
  Index global_index = store->env->GetGlobalCount() - 1;
  auto instance = std::make_shared<WasmInstance>(store, nullptr);
  return new wasm_global_t(instance, global_index);
}

wasm_globaltype_t* wasm_global_type(const wasm_global_t* global) {
  assert(global);
  assert(false);
  WABT_UNREACHABLE;
}

void wasm_global_get(const wasm_global_t* global, wasm_val_t* out) {
  assert(global);
  TRACE("wasm_global_get");
  interp::Global* interp_global = global->GetGlobal();
  TRACE(" -> %s\n", TypedValueToString(interp_global->typed_value).c_str());
  *out = FromWabtValue(global->instance, interp_global->typed_value);
  return;
}

void wasm_global_set(wasm_global_t* global, const wasm_val_t* val) {
  TRACE("wasm_global_set\n");
  interp::Global* g = global->GetGlobal();
  g->typed_value = ToWabtValue(*val);
}

// wasm_table

wasm_table_t* wasm_table_new(wasm_store_t* store,
                             const wasm_tabletype_t* type,
                             wasm_ref_t* init) {
  store->env->EmplaceBackTable(ToWabtType(type->elemtype.kind),
                               ToWabtLimits(type->limits));
  Index index = store->env->GetTableCount() - 1;
  auto instance = std::make_shared<WasmInstance>(store, nullptr);
  return new wasm_table_t(instance, index);
}

wasm_tabletype_t* wasm_table_type(const wasm_table_t*) {
  assert(false);
  return nullptr;
}

wasm_table_size_t wasm_table_size(const wasm_table_t* table) {
  return table->GetTable()->size();
}

wasm_ref_t* wasm_table_get(const wasm_table_t* t, wasm_table_size_t index) {
  interp::Table* table = t->GetTable();
  // TODO(sbc): This duplicates code from the CallIndirect handler.  I imagine
  // we will refactor this when we implment the TableGet opcode.
  if (index >= table->size())
    return nullptr;
  switch (table->entries[index].kind) {
    case RefType::Func: {
      Index func_index = table->entries[index].index;
      if (func_index == kInvalidIndex)
        return nullptr;
      TRACE("wasm_table_get: %u -> %u\n", index, func_index);
      return new wasm_extern_ref_t(ExternalKind::Func, func_index);
    }
    case RefType::Host:
      return new wasm_foreign_ref_t(table->entries[index].index);
    case RefType::Null:
      return nullptr;
  }
  WABT_UNREACHABLE;
}

bool wasm_table_set(wasm_table_t* t, wasm_table_size_t index, wasm_ref_t* ref) {
  interp::Table* table = t->GetTable();
  if (index >= table->size()) {
    return false;
  }
  TRACE("wasm_table_set %p\n", ref);
  if (!ref) {
    table->entries[index] = Ref{RefType::Null, kInvalidIndex};
    return true;
  }
  TRACE("wasm_table_set %s\n", WasmRefTypeToString(ref->kind).c_str());
  switch (ref->kind) {
    case WasmRefType::Extern: {
      auto* ext = static_cast<wasm_extern_ref_t*>(ref);
      switch (ext->kind) {
        case ExternalKind::Func:
          table->entries[index] = Ref{RefType::Func, ext->index};
          break;
        case ExternalKind::Table:
        case ExternalKind::Memory:
        case ExternalKind::Global:
        case ExternalKind::Event:
          return false;
      }
      break;
    }
    case WasmRefType::Foreign: {
      auto* f = static_cast<wasm_foreign_ref_t*>(ref);
      table->entries[index] = Ref{RefType::Host, f->index};
      break;
    }
    case WasmRefType::Module:
    case WasmRefType::Instance:
    case WasmRefType::Trap:
      return false;
  }
  return true;
}

bool wasm_table_grow(wasm_table_t* t,
                     wasm_table_size_t delta,
                     wasm_ref_t* init) {
  interp::Table* table = t->GetTable();
  size_t cursize = table->size();
  size_t newsize = cursize + delta;
  if (newsize > table->limits.max) {
    return false;
  }
  TRACE("wasm_table_grow %" PRIzx " -> %" PRIzx "\n", cursize, newsize);
  auto* ext = static_cast<wasm_extern_ref_t*>(init);
  if (ext && ext->kind != ExternalKind::Func) {
    return false;
  }
  Ref init_ref{RefType::Null, kInvalidIndex};
  if (ext) {
    init_ref = Ref{RefType::Func, ext->index};
  }
  table->entries.resize(newsize, init_ref);
  return true;
}

// wams_memory

wasm_memory_t* wasm_memory_new(wasm_store_t* store,
                               const wasm_memorytype_t* type) {
  TRACE("wasm_memory_new\n");
  store->env->EmplaceBackMemory(Limits(type->limits.min, type->limits.max));
  Index index = store->env->GetMemoryCount() - 1;
  auto instance = std::make_shared<WasmInstance>(store, nullptr);
  return new wasm_memory_t(instance, index);
}

byte_t* wasm_memory_data(wasm_memory_t* m) {
  interp::Memory* memory = m->GetMemory();
  return memory->data.data();
}

wasm_memory_pages_t wasm_memory_size(const wasm_memory_t* m) {
  interp::Memory* memory = m->GetMemory();
  return memory->data.size() / WABT_PAGE_SIZE;
}

size_t wasm_memory_data_size(const wasm_memory_t* m) {
  interp::Memory* memory = m->GetMemory();
  return memory->data.size();
}

bool wasm_memory_grow(wasm_memory_t* m, wasm_memory_pages_t delta) {
  interp::Memory* memory = m->GetMemory();
  size_t cursize = memory->data.size() / WABT_PAGE_SIZE;
  size_t newsize = cursize + delta;
  if (newsize > memory->page_limits.max) {
    return false;
  }
  TRACE("wasm_memory_grow %" PRIzx " -> %" PRIzx "\n", cursize, newsize);
  memory->data.resize(newsize * WABT_PAGE_SIZE);
  return true;
}

// wasm_frame

wasm_instance_t* wasm_frame_instance(const wasm_frame_t* frame) {
  assert(frame);
  return frame->instance;
}

size_t wasm_frame_module_offset(const wasm_frame_t* frame) {
  assert(frame);
  return frame->offset;
}

size_t wasm_frame_func_offset(const wasm_frame_t* frame) {
  assert(false);
  return 0;
}

uint32_t wasm_frame_func_index(const wasm_frame_t* frame) {
  assert(frame);
  return frame->func_index;
}

// wasm_externtype

wasm_externkind_t wasm_externtype_kind(const wasm_externtype_t* type) {
  assert(type);
  return type->kind;
}

// wasm_extern

wasm_externtype_t* wasm_extern_type(const wasm_extern_t* extern_) {
  return extern_->ExternType();
}

wasm_externkind_t wasm_extern_kind(const wasm_extern_t* extern_) {
  return FromWabtExternalKind(extern_->kind);
}

// wasm_foreign

wasm_foreign_t* wasm_foreign_new(wasm_store_t* store) {
  store->env->EmplaceBackHostObject();
  Index new_index = store->env->GetHostCount() - 1;
  return new wasm_foreign_t(new_index);
}

// vector types

#define WASM_IMPL_OWN(name)                       \
  void wasm_##name##_delete(wasm_##name##_t* t) { \
    assert(t);                                    \
    TRACE("wasm_" #name "_delete\n");             \
    delete t;                                     \
  }

WASM_IMPL_OWN(frame);
WASM_IMPL_OWN(config);
WASM_IMPL_OWN(engine);
WASM_IMPL_OWN(store);

#define WASM_IMPL_VEC(name, ptr_or_none)                                \
  void wasm_##name##_vec_new(wasm_##name##_vec_t* vec, size_t size,     \
                             wasm_##name##_t ptr_or_none const src[]) { \
    TRACE("wasm_" #name "_vec_new\n");                                  \
    wasm_##name##_vec_new_uninitialized(vec, size);                     \
    memcpy(vec->data, src, size * sizeof(wasm_##name##_t ptr_or_none)); \
  }                                                                     \
  void wasm_##name##_vec_new_empty(wasm_##name##_vec_t* out) {          \
    TRACE("wasm_" #name "_vec_new_empty\n");                            \
    out->data = nullptr;                                                \
    out->size = 0;                                                      \
  }                                                                     \
  void wasm_##name##_vec_new_uninitialized(wasm_##name##_vec_t* vec,    \
                                           size_t size) {               \
    TRACE("wasm_" #name "_vec_new_uninitialized %" PRIzx "\n", size);   \
    vec->size = size;                                                   \
    if (size)                                                           \
      vec->data = new wasm_##name##_t ptr_or_none[size];                \
    else                                                                \
      vec->data = nullptr;                                              \
  }                                                                     \
  void wasm_##name##_vec_copy(wasm_##name##_vec_t* out,                 \
                              const wasm_##name##_vec_t* vec) {         \
    TRACE("wasm_" #name "_vec_copy %" PRIzx "\n", vec->size);           \
    wasm_##name##_vec_new_uninitialized(out, vec->size);                \
    memcpy(out->data, vec->data,                                        \
           vec->size * sizeof(wasm_##name##_t ptr_or_none));            \
  }                                                                     \
  void wasm_##name##_vec_delete(wasm_##name##_vec_t* vec) {             \
    TRACE("wasm_" #name "_vec_delete\n");                               \
    if (vec->data) {                                                    \
      delete vec->data;                                                 \
    }                                                                   \
    vec->size = 0;                                                      \
  }

WASM_IMPL_VEC(byte, );
WASM_IMPL_VEC(val, );
WASM_IMPL_VEC(frame, *);
WASM_IMPL_VEC(extern, *);

#define WASM_IMPL_TYPE(name)                                    \
  WASM_IMPL_OWN(name)                                           \
  WASM_IMPL_VEC(name, *)                                        \
  wasm_##name##_t* wasm_##name##_copy(wasm_##name##_t* other) { \
    return new wasm_##name##_t(*other);                         \
  }

WASM_IMPL_TYPE(valtype);
WASM_IMPL_TYPE(functype);
WASM_IMPL_TYPE(globaltype);
WASM_IMPL_TYPE(tabletype);
WASM_IMPL_TYPE(memorytype);
WASM_IMPL_TYPE(externtype);
WASM_IMPL_TYPE(importtype);
WASM_IMPL_TYPE(exporttype);

#define WASM_IMPL_REF_BASE(name)                                       \
  WASM_IMPL_OWN(name)                                                  \
  wasm_##name##_t* wasm_##name##_copy(const wasm_##name##_t* ref) {    \
    TRACE("wasm_" #name "_copy\n");                                    \
    return static_cast<wasm_##name##_t*>(ref->Copy());                 \
  }                                                                    \
  bool wasm_##name##_same(const wasm_##name##_t* ref,                  \
                          const wasm_##name##_t* other) {              \
    TRACE("wasm_" #name "_same\n");                                    \
    return ref->Same(*other);                                          \
  }                                                                    \
  void* wasm_##name##_get_host_info(const wasm_##name##_t* ref) {      \
    return ref->host_info;                                             \
  }                                                                    \
  void wasm_##name##_set_host_info(wasm_##name##_t* ref, void* info) { \
    ref->host_info = info;                                             \
  }                                                                    \
  void wasm_##name##_set_host_info_with_finalizer(                     \
      wasm_##name##_t* ref, void* info, void (*finalizer)(void*)) {    \
    ref->host_info = info;                                             \
    ref->finalizer = finalizer;                                        \
  }

#define WASM_IMPL_REF(name)                                                  \
  WASM_IMPL_REF_BASE(name)                                                   \
  wasm_ref_t* wasm_##name##_as_ref(wasm_##name##_t* subclass) {              \
    return subclass;                                                         \
  }                                                                          \
  wasm_##name##_t* wasm_ref_as_##name(wasm_ref_t* ref) {                     \
    return static_cast<wasm_##name##_t*>(ref);                               \
  }                                                                          \
  const wasm_ref_t* wasm_##name##_as_ref_const(                              \
      const wasm_##name##_t* subclass) {                                     \
    return subclass;                                                         \
  }                                                                          \
  const wasm_##name##_t* wasm_ref_as_##name##_const(const wasm_ref_t* ref) { \
    return static_cast<const wasm_##name##_t*>(ref);                         \
  }

WASM_IMPL_REF_BASE(ref);

WASM_IMPL_REF(extern);
WASM_IMPL_REF(foreign);
WASM_IMPL_REF(func);
WASM_IMPL_REF(global);
WASM_IMPL_REF(instance);
WASM_IMPL_REF(memory);
WASM_IMPL_REF(table);
WASM_IMPL_REF(trap);

#define WASM_IMPL_SHARABLE_REF(name)                                       \
  WASM_IMPL_REF(name)                                                      \
  WASM_IMPL_OWN(shared_##name)                                             \
  wasm_shared_##name##_t* wasm_##name##_share(const wasm_##name##_t* t) {  \
    return static_cast<wasm_shared_##name##_t*>(                           \
        const_cast<wasm_##name##_t*>(t));                                  \
  }                                                                        \
  wasm_##name##_t* wasm_##name##_obtain(wasm_store_t*,                     \
                                        const wasm_shared_##name##_t* t) { \
    return static_cast<wasm_##name##_t*>(                                  \
        const_cast<wasm_shared_##name##_t*>(t));                           \
  }

WASM_IMPL_SHARABLE_REF(module)

#define WASM_IMPL_EXTERN(name)                                                 \
  const wasm_##name##type_t* wasm_externtype_as_##name##type_const(            \
      const wasm_externtype_t* t) {                                            \
    return static_cast<const wasm_##name##type_t*>(t);                         \
  }                                                                            \
  wasm_##name##type_t* wasm_externtype_as_##name##type(wasm_externtype_t* t) { \
    return static_cast<wasm_##name##type_t*>(t);                               \
  }                                                                            \
  wasm_externtype_t* wasm_##name##type_as_externtype(wasm_##name##type_t* t) { \
    return static_cast<wasm_externtype_t*>(t);                                 \
  }                                                                            \
  const wasm_externtype_t* wasm_##name##type_as_externtype_const(              \
      const wasm_##name##type_t* t) {                                          \
    return static_cast<const wasm_externtype_t*>(t);                           \
  }                                                                            \
  wasm_extern_t* wasm_##name##_as_extern(wasm_##name##_t* name) {              \
    return static_cast<wasm_extern_t*>(name);                                  \
  }                                                                            \
  const wasm_extern_t* wasm_##name##_as_extern_const(                          \
      const wasm_##name##_t* name) {                                           \
    return static_cast<const wasm_extern_t*>(name);                            \
  }                                                                            \
  wasm_##name##_t* wasm_extern_as_##name(wasm_extern_t* ext) {                 \
    return static_cast<wasm_##name##_t*>(ext);                                 \
  }

WASM_IMPL_EXTERN(table);
WASM_IMPL_EXTERN(func);
WASM_IMPL_EXTERN(global);
WASM_IMPL_EXTERN(memory);

}  // extern "C"
