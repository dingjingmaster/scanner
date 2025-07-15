//
// Created by dingjing on 25-7-15.
//
#include <stdlib.h>
#include <unistd.h>

#include "common/ipc.h"


void hook_ld_preload_init       (void) __attribute__((constructor()));
void hook_ld_preload_cleanup    (void) __attribute__((destructor()));

void hook_ld_preload_init (void)
{
    if (0 == access(HOOK_CORE, F_OK)) {
        setenv("LD_PRELOAD", HOOK_CORE, 1);
    }
}

void hook_ld_preload_cleanup (void)
{
    setenv("LD_PRELOAD", "", 1);
}