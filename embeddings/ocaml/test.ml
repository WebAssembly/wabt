open Ctypes;;
open Wasmer;;
open Wasmer.Util;;

open Input;;

let () =
  let engine = Engine.new_ () in
  let store = Store.new_ engine in
  let module_  = Module.new_ store (load_wasm_file "../../out/wasi/Debug/libwabt.wasm") in
  (*let config = Wasi.WasiConfig.new_ Sys.argv.(0) in
  let env = Wasi.Env.new_ store config in
  let errfunc funname ext =
    Extern.of_func (Func.new_ store Functype.([] %-> [])
      (fun _ -> failwith ("Error: " ^ funname))) in*)
  let errfunc n t = Extern.of_func (Func.new_ store t (fun _ -> failwith ("Error: " ^ n))) in
  (*let imports = new importsBuilder in
  imports#add_default "wasi_snapshot_preview1" errfunc;*)
  (*imports#add "wasi_snapshot_preview1" "environ_get" errfunc;
  imports#add "wasi_snapshot_preview1" "environ_sizes_get" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_close" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_fdstat_get" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_seek" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_write" errfunc;
  imports#add "wasi_snapshot_preview1" "proc_exit" errfunc;
  
  imports#add "wasi_snapshot_preview1" "args_sizes_get" errfunc;
  imports#add "wasi_snapshot_preview1" "clock_time_get" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_advise" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_allocate" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_fdstat_set_rights" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_filestat_set_size" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_prestat_get" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_prestat_dir_name" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_pwrite" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_read" errfunc;
  imports#add "wasi_snapshot_preview1" "fd_readdir" errfunc;
  imports#add "wasi_snapshot_preview1" "path_create_directory" errfunc;
  imports#add "wasi_snapshot_preview1" "path_filestat_get" errfunc;
  imports#add "wasi_snapshot_preview1" "path_filestat_set_times" errfunc;
  imports#add "wasi_snapshot_preview1" "path_link" errfunc;
  imports#add "wasi_snapshot_preview1" "path_open" errfunc;
  imports#add "wasi_snapshot_preview1" "path_rename" errfunc;
  imports#add "wasi_snapshot_preview1" "path_symlink" errfunc;
  imports#add "wasi_snapshot_preview1" "path_unlink_file" errfunc;
  imports#add "wasi_snapshot_preview1" "proc_raise" errfunc;
  imports#add "wasi_snapshot_preview1" "random_get" errfunc;
  imports#add "wasi_snapshot_preview1" "sched_yield" errfunc;
  imports#add "wasi_snapshot_preview1" "sock_send" errfunc;*)
  (*let _instance, wabt = wabt_of_module store env module_ imports in*)
  let exports = Extern.Vec.make_uninit 7 in
  Extern.Vec.set_element exports 0 (errfunc "environ_get" Functype.([I32; I32] %-> [I32]));
  Extern.Vec.set_element exports 1 (errfunc "environ_sizes_get" Functype.([I32; I32] %-> [I32]));
  Extern.Vec.set_element exports 2 (errfunc "fd_close" Functype.([I32] %-> [I32]));
  Extern.Vec.set_element exports 3 (errfunc "fd_fdstat_get" Functype.([I32; I32] %-> [I32]));
  Extern.Vec.set_element exports 4 (errfunc "fd_seek" Functype.([I32; I64; I32; I32] %-> [I32]));
  Extern.Vec.set_element exports 5 (errfunc "fd_write" Functype.([I32; I32; I32; I32] %-> [I32]));
  Extern.Vec.set_element exports 6 (errfunc "proc_exit" Functype.([I32] %-> []));
  match Instance.new_ store module_ exports with
  | Error (Some t) ->
    let msg = Message.make_new () in Trap.message t msg;
    failwith ("Failed to instanciate module: " ^ Message.to_string msg)
  | Error None ->
    failwith ("Failed to instanciate module: " ^ (Util.retrieve_last_error ()))
  | Ok instance ->
  let wabt = new wabt module_ instance in
  
  let wat2wasm contents =
    print_endline ("Trying \"" ^ contents ^ "\"");
    try match wabt#wat2wasm contents WasmFeatures.[MutableGlobals] with
    | Error t ->
      let msg = Message.make_new () in Trap.message t msg;
      print_endline ("Caught trap: " ^ (Message.to_string msg))
    | Ok res -> match res with
      | Ok lst -> print_string "Got output: ";
        List.iter (fun c -> print_string (Char.escaped c)) lst;
        print_newline ()
      | Error err -> print_endline ("Got error: " ^ err)
    with e ->
      print_endline ("Caught exception: " ^ Printexc.to_string e) in
  let wasm2wat contents =
    print_string ("Trying \"");
    Bytes.iter (fun b -> print_string (Char.escaped b)) contents;
    print_newline ();
    try match wabt#wasm2wat contents WasmFeatures.[MutableGlobals] with
      | Error t ->
        let msg = Message.make_new () in Trap.message t msg;
        print_endline ("Caught trap: " ^ (Message.to_string msg))
      | Ok res -> match res with
        | Ok str -> print_endline ("Got output: " ^ str)
        | Error err -> print_endline ("Got error: " ^ err)
    with e ->
      print_endline ("Caught exception: " ^ Printexc.to_string e) in
  
  let bytes_of_list l =
    let len = List.length l in
    let bs = Bytes.create len in
    List.iteri (fun i c -> Bytes.set bs i (Char.chr c)) l;
    bs in
  
  wat2wasm "(module)";
  wat2wasm "xyz";
  wasm2wat (bytes_of_list [
      0;  97; 115; 109;   1;   0;   0;   0;   0;
      8;   4; 110;  97; 109; 101;   2;   1;   0;
      0;   9;   7; 108; 105; 110; 107; 105; 110;
    103;   2
  ]);;
