//
// Created by dingjing on 25-7-14.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>


#define LOGI

#ifdef LOGI
#include <syslog.h>
#define logi(...) syslog(LOG_ERR, __VA_ARGS__)
#else
#define logi(...)
#endif


void hook_gdk_init() __attribute__((constructor()));
void hook_gdk_cleanup() __attribute__((destructor()));

void hook_gdk_init()
{

}

void hook_gdk_cleanup()
{
}

