/* Link this with wasm2c output and uvwasi runtime to build a standalone app */
#include <stdio.h>
#include <stdlib.h>
#include "uvwasi.h"
#include "uvwasi-rt.h"

#define MODULE_NAME hello
#define MODULE_HEADER "hello.wasm.h"
#include MODULE_HEADER

//force pre-processor expansion of m_name
#define __module_init(m_name) Z_## m_name ##_init_module()
#define module_init(m_name) __module_init(m_name)

#define __module_instantiate(m_name, instance_p, wasi_p) Z_## m_name ##_instantiate(instance_p, wasi_p)
#define module_instantiate(m_name ,instance_p, wasi_p) __module_instantiate(m_name ,instance_p, wasi_p)

#define __module_free(m_name, instance_p)  Z_## m_name ##_free(instance_p)
#define module_free(m_name, instance_p) __module_free(m_name, instance_p)

#define __module_start(m_name, instance_p) Z_ ## m_name ## Z__start(instance_p)
#define module_start(m_name, instance_p) __module_start(m_name, instance_p)

int main(int argc, const char** argv)
{
    Z_hello_instance_t local_instance;
    uvwasi_t local_uvwasi_state;

    struct Z_wasi_snapshot_preview1_instance_t wasi_state = {
	.uvwasi = &local_uvwasi_state,
	.instance_memory = &local_instance.w2c_memory
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

    module_init(MODULE_NAME);
    module_instantiate(MODULE_NAME, &local_instance, (struct Z_wasi_snapshot_preview1_instance_t *) &wasi_state);
    module_start(MODULE_NAME, &local_instance);
    module_free(MODULE_NAME, &local_instance);

    uvwasi_destroy(&local_uvwasi_state);
    wasm_rt_free();

    return 0;
}
