project(common LANGUAGES C)
add_library(common STATIC
        utils.h utils.c
        ipc-shm.h ipc-shm.c
        proc-list.h proc-list.c
        utils-ipc.h utils-ipc.c
)
target_compile_options(common PUBLIC -fPIC)
set_target_properties(common PROPERTIES LINKER_LANGUAGE C)