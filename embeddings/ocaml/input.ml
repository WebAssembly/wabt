open Wasmer;;

exception Invalid_variant of string;;

module WasmFeatures = struct
  type flag =
    | Exceptions
    | MutableGlobals
    | SatFloatToInt
    | SignExtension
    | SIMD
    | Threads
    | MultiValue
    | TailCall
    | BulkMemory
    | ReferenceTypes
    | Annotations
    | GC
  
  type flags = flag list
  let flag_to_wa = function
    | Exceptions -> 1
    | MutableGlobals -> 2
    | SatFloatToInt -> 4
    | SignExtension -> 8
    | SIMD -> 16
    | Threads -> 32
    | MultiValue -> 64
    | TailCall -> 128
    | BulkMemory -> 256
    | ReferenceTypes -> 512
    | Annotations -> 1024
    | GC -> 2048
  let rec to_wa f = match f with
    | [] -> 0
    | f1 :: fs -> to_wa fs lor (flag_to_wa f1)
  let from_wa f =
    let ret, rest = List.fold_left (fun (acc, rest) v ->
      let flagv = flag_to_wa v in
      if (rest land flagv) <> 0 then (v :: acc, rest - flagv)
      else (acc, rest)) ([], f) [
        GC;
        Annotations;
        ReferenceTypes;
        BulkMemory;
        TailCall;
        MultiValue;
        Threads;
        SIMD;
        SignExtension;
        SatFloatToInt;
        MutableGlobals;
        Exceptions;
      ] in
    if rest = 0 then Ok ret
    else Error (ret, rest)
end;;

class wabt module_ instance =
  let exports = Extern.Vec.make_new () in
    let () = Instance.exports instance exports in
  let mem = match Util.get_export_of_name module_ instance "memory" with
    | None -> failwith "Invalid arguments: instance and/or module does not implement wabt"
    | Some ext -> Extern.to_memory ext in
  let realloc = match Util.get_export_of_name module_ instance "canonical_abi_realloc" with
    | None -> failwith "Invalid arguments: instance and/or module does not implement wabt"
    | Some ext -> Extern.to_func ext in
  let reallocator ptr origsz align newsz =
    let ret_holder = Val.new_ () in
    match Func.call
      realloc
      Val.(Vec.of_list [of_i32 ptr; of_i32 origsz; of_i32 align; of_i32 newsz])
      Val.(Vec.of_list [ret_holder]) with
    | None ->
print_endline Valkind.(match Val.get_kind ret_holder with I32->"I32"|I64->"I64"|F32->"F32"|F64->"F64"|AnyRef->"AnyRef"|FuncRef->"FuncRef");
Ctypes.(!@(Val.get_ptr_const ret_holder|->Val.fof|->Val.Anon0.fi32))
    | Some t ->
      let msg = Message.make_new () in Trap.message t msg;
      failwith (Message.to_string msg) in
  let free = match Util.get_export_of_name module_ instance "canonical_abi_free" with
    | None -> failwith "Invalid arguments: instance and/or module does not implement wabt"
    | Some ext -> Extern.to_func ext in
  let freer ptr sz align =
    match Func.call
      free
      Val.(Vec.of_list [of_i32 ptr; of_i32 sz; of_i32 align])
      Val.(Vec.of_list []) with
    | None -> ()
    | Some t ->
      let msg = Message.make_new () in Trap.message t msg;
      failwith (Message.to_string msg) in
  
  let _encode_utf8 str =
    let encoded = Bytes.of_string str in
    let len = String.length str in
    let ptr = reallocator 0l 0l 1l (Int32.of_int len) in
    if (Int32.to_int ptr) + len > Memory.data_size mem then 
      failwith "The string is out of bounds";
    Memory.set_data mem (Int32.to_int ptr) len encoded;
    ptr, len in
  
  let _decode_utf8 ptr len =
    if (Int32.to_int ptr) + len > Memory.data_size mem then 
      failwith "The string is out of bounds";
    let encoded = Memory.get_data mem (Int32.to_int ptr) len in
    String.of_bytes encoded in
  
  let import_wasm2wat = match Util.get_export_of_name module_ instance "wasm2wat" with
    | None -> failwith "Invalid arguments: instance and/or module does not implement wabt"
    | Some ext -> Extern.to_func ext in
  let import_wat2wasm = match Util.get_export_of_name module_ instance "wat2wasm" with
    | None -> failwith "Invalid arguments: instance and/or module does not implement wabt"
    | Some ext -> Extern.to_func ext in
object
  
  method wasm2wat arg0 arg1 =
    let val0 = arg0 in
    let len0 = Bytes.length val0 in
    let ptr0 = reallocator 0l 0l 1l (Int32.of_int (len0 * 1)) in
    Memory.set_data mem (Int32.to_int ptr0) (len0 * 1) val0;
    let val1 = WasmFeatures.to_wa arg1 in
    let args = Val.(Vec.of_list [
      of_i32 ptr0; of_i32 (Int32.of_int len0);
      of_i32 (Int32.of_int val1);
    ]) in
    let rets = Val.Vec.make_uninit 1 in
    match Func.call import_wasm2wat args rets with
    | Some t -> Error t
    | None -> Ok (
      let ret0 = Val.get_i32 (Val.Vec.get_element_const rets 0) in
      match Int32.to_int (Memory.get_i32 mem (Int32.to_int ret0)) with
      | 0 -> Ok (
        let load4i32 = Memory.get_i32 mem (Int32.to_int ret0 + 4) in
        let load8i32 = Memory.get_i32 mem (Int32.to_int ret0 + 8) in
        let ptr6 = load4i32 in
        let len6 = load8i32 in
        let data6 = Memory.get_data mem (Int32.to_int ptr6) (Int32.to_int len6) in
        freer ptr6 len6 1l;
        Bytes.to_string data6
      )
      | 1 -> Error (
        let load4i32 = Memory.get_i32 mem (Int32.to_int ret0 + 4) in
        let load8i32 = Memory.get_i32 mem (Int32.to_int ret0 + 8) in
        let ptr9 = load4i32 in
        let len9 = load8i32 in
        let data9 = Memory.get_data mem (Int32.to_int ptr9) (Int32.to_int len9) in
        freer ptr9 len9 1l;
        Bytes.to_string data9
      )
      | _ -> raise (Invalid_variant "expected")
    )
  
  method wat2wasm arg0 arg1 =
    let ptr0, len0 = _encode_utf8 arg0 in
    let val1 = WasmFeatures.to_wa arg1 in
    let args = Val.(Vec.of_list [
      of_i32 ptr0; of_i32 (Int32.of_int len0);
      of_i32 (Int32.of_int val1);
    ]) in
    let rets = Val.Vec.make_uninit 1 in
    match Func.call import_wat2wasm args rets with
    | Some t -> Error t
    | None -> Ok (
      let ret0 = Val.get_i32 (Val.Vec.get_element_const rets 0) in
      match Int32.to_int (Memory.get_i32 mem (Int32.to_int ret0)) with
      | 0 -> Ok (
        let load4i32 = Memory.get_i32 mem (Int32.to_int ret0 + 4) in
        let load8i32 = Memory.get_i32 mem (Int32.to_int ret0 + 8) in
        let ptr6 = load4i32 in
        let len6 = load8i32 in
        let data6 = Memory.get_data mem (Int32.to_int ptr6) (Int32.to_int len6) in
        freer ptr6 len6 1l;
        List.of_seq (Bytes.to_seq data6)
      )
      | 1 -> Error (
        let load4i32 = Memory.get_i32 mem (Int32.to_int ret0 + 4) in
        let load8i32 = Memory.get_i32 mem (Int32.to_int ret0 + 8) in
        let ptr9 = load4i32 in
        let len9 = load8i32 in
        let data9 = Memory.get_data mem (Int32.to_int ptr9) (Int32.to_int len9) in
        freer ptr9 len9 1l;
        Bytes.to_string data9
      )
      | _ -> raise (Invalid_variant "expected")
    )
end;;

(* let wabt_of_module store env module_ (imports: Util.importsBuilder) =
  match Instance.new_ store module_ (imports#to_exports env module_) with
  | Ok instance -> instance, new wabt module_ instance
  | Error (Some t) ->
    let msg = Message.make_new () in Trap.message t msg;
    failwith ("Failed to instanciate module: " ^ Message.to_string msg)
  | Error None ->
    failwith ("Failed to instanciate module: " ^ (Util.retrieve_last_error ()));; *)
