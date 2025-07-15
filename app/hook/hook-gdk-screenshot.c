//
// Created by dingjing on 25-7-14.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hook-common.h"
#include "common/utils.h"
#include "hook-func/src/hook-func.h"


#define LOGI

#ifdef LOGI
#include <syslog.h>
#define logi(...) syslog(LOG_ERR, __VA_ARGS__)
#else
#define logi(...)
#endif


void hook_gdk_init() __attribute__((constructor()));
void hook_gdk_cleanup() __attribute__((destructor()));

typedef void (*GDbusConnectionCall) (void* conn, const char* busName, const char* objPath, const char* interName,
    const char* methName, void* parameters, const void* replyType, int flags, int timeoutSec, void* cancelable, void* cb, void* uData);
extern void g_dbus_connection_call (void* conn, const char* busName, const char* objPath, const char* interName,
    const char* methName, void* parameters, const void* replyType, int flags, int timeoutSec, void* cancelable, void* cb, void* uData);
static GDbusConnectionCall gsDbusConnectionCall = NULL;
static HookFunc* gsHookFunc = NULL;
void hook_dbus_connection_call (void* conn, const char* busName, const char* objPath, const char* interName,
                                const char* methName, void* parameters, const void* replyType, int flags,
                                int timeoutSec, void* cancelable, void* cb, void* uData)
{

    logi("busName: %s, methodName: %s", busName, methName);
    if (0 == strcmp(busName, "org.gnome.Shell.Screenshot") && (0 == strcmp(methName, "ScreenshotArea") || 0 == strcmp(methName, "Screenshot"))) {
        logi("screenshot");
        char procBuf[4096] = {0};
        utils_get_proc_by_pid_array(getpid(), procBuf, sizeof(procBuf));

        if (hc_check_screenshot_control_forbidden()) {
            syslog(LOG_INFO, "forbid screenshot!");
            return;
        }
    }

    gsDbusConnectionCall(conn, busName, objPath, interName, methName, parameters, replyType, flags, timeoutSec, cancelable, cb, uData);
}


void hook_gdk_init()
{
    gsDbusConnectionCall = dlsym(RTLD_DEFAULT, "g_dbus_connection_call");
    C_RETURN_IF_FAIL_SYSLOG_WARN(gsDbusConnectionCall, "get g_dbus_connection_call error!");

    gsHookFunc = hook_func_create();
    C_RETURN_IF_FAIL_SYSLOG_WARN(gsHookFunc, "hook_func_create() error!");

    C_RETURN_IF_FAIL_SYSLOG_WARN(0 == hook_func_prepare(gsHookFunc, (void**) &gsDbusConnectionCall, hook_dbus_connection_call), "hook_func_prepare() error!");
    C_RETURN_IF_FAIL_SYSLOG_WARN(0 == hook_func_install(gsHookFunc, 0), "hook_func_install() error!");

    syslog(LOG_INFO, "[HOOK] install: g_dbus_connection_call done!");
}

void hook_gdk_cleanup()
{
    if (gsHookFunc) {
        if (0 != hook_func_uninstall(gsHookFunc, 0)) {
            syslog(LOG_INFO, "[HOOK] uninstall failed!");
        }
        if (0 != hook_func_destroy(&gsHookFunc)) {
            syslog(LOG_ERR, "[HOOK] destroy failed!]");
        }
    }
}

