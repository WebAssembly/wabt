/* Link this with wasm2c output and uvwasi runtime to build a standalone app */
#include <stdio.h>
#include <stdlib.h>
#include "uvwasi.h"
#include "uvwasi-rt.h"
#include "main.h"

int main(int argc, const char** argv)
{
    Z_main_instance_t local_instance;
    uvwasi_t local_uvwasi_state;

    struct Z_wasi_snapshot_preview1_instance_t wasi_state = {
	.uvwasi = &local_uvwasi_state,
	.instance_memory = Z_mainZ_memory(&local_instance),
    };

    uvwasi_options_t init_options;

    //pass in standard descriptors
    init_options.in = 0;
    init_options.out = 1;
    init_options.err = 2;
    init_options.fd_table_size = 3;

    //pass in args and environement
    extern const char ** environ;
    init_options.argc = argc;
    init_options.argv = argv;
    init_options.envp = (const char **) environ;

    //no sandboxing enforced, binary has access to everything user does
    init_options.preopenc = 2;
    init_options.preopens = calloc(2, sizeof(uvwasi_preopen_t));

    init_options.preopens[0].mapped_path = "/";
    init_options.preopens[0].real_path = "/";
    init_options.preopens[1].mapped_path = "./";
    init_options.preopens[1].real_path = ".";

    init_options.allocator = NULL;

    wasm_rt_init();
    uvwasi_errno_t ret = uvwasi_init(&local_uvwasi_state, &init_options);

    if (ret != UVWASI_ESUCCESS) {
        printf("uvwasi_init failed with error %d\n", ret);
        exit(1);
    }

    Z_main_init_module();
    Z_main_instantiate(&local_instance, (struct Z_wasi_snapshot_preview1_instance_t *) &wasi_state);
    Z_mainZ__start(&local_instance);
    Z_main_free(&local_instance);

    uvwasi_destroy(&local_uvwasi_state);
    wasm_rt_free();

    return 0;
}
