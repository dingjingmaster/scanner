//
// Created by dingjing on 25-7-14.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/limits.h>

#include "hook-common.h"
#include "common/utils.h"
#include "hook-func/src/hook-func.h"


void hook_main_init() __attribute__((constructor()));
void hook_main_cleanup() __attribute__((destructor()));

typedef int (*Connect) (int sockFd, const struct sockaddr *addr, socklen_t addrLen);

static int check_special_ip (const char* ip);
static int check_special_proc (const char* proc);
static char* get_ip (const struct sockaddr_in* addr);
static int get_port (const struct sockaddr_in* addr);
static char* get_port_str (const struct sockaddr_in* addr);
int hook_connect (int sockFd, const void* addr, int addrLen);

static Connect      gsConnect = connect;
static HookFunc*    gsHookFunc = NULL;


void hook_main_init()
{
    gsHookFunc = hook_func_create();
    C_RETURN_IF_FAIL_SYSLOG_WARN(gsHookFunc, "hook_func_create() error!");

    C_RETURN_IF_FAIL_SYSLOG_WARN(0 == hook_func_prepare(gsHookFunc, (void**) &gsConnect, hook_connect), "hook_connect hook_func_prepare() error!");
    C_RETURN_IF_FAIL_SYSLOG_WARN(0 == hook_func_install(gsHookFunc, 0), "hook_func_install() error!");

    syslog(LOG_INFO, "[HOOK] install: connect done!");
}

void hook_main_cleanup()
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

int hook_connect (int sockFd, const void* addr, int addrLen)
{
    int ret = -101;

    char* ip = NULL;            // free
    char* portStr = NULL;       // free
    char proc[PATH_MAX] = {0};

    const struct sockaddr* addrS = (const struct sockaddr*) addr;

    sa_family_t family = 0;
    if (addrS) {
        family = addrS->sa_family;
    }
    else {
        syslog(LOG_ERR, "addr is null allow!");
        goto out;
    }

    utils_get_proc_by_pid_array(getpid(), proc, sizeof(proc) - 1);

    // 自动放过
    if (0 == check_special_proc (proc)) {
        syslog(LOG_ERR, "proc: %s\n", proc);
        goto out;
    }

    // ip && port
    if (AF_INET != family) {
        syslog(LOG_ERR, "is not AF_INET\n");
        goto out;
    }

    ip = get_ip((struct sockaddr_in*) addrS);
    if (!ip) {
        syslog (LOG_ERR, "ip is null, allow");
        goto out;
    }

    portStr = get_port_str((const struct sockaddr_in*) addrS);
    if (!portStr) {
        syslog(LOG_ERR, "port is null, allow");
        goto out;
    }

    if (!ip) {
        syslog(LOG_ERR, "!proc || !ip\n");
        goto out;
    }

    if (0 == check_special_ip (ip)) {
        syslog(LOG_ERR, "ip: %s special\n", ip);
        goto out;
    }

    syslog(LOG_ERR, "proc: %s, ip: %s, port: %s\n", proc, ip, portStr);
    if (hc_check_process_network_control_forbidden(proc, getpid(), ip, (int) strtol (portStr, NULL, 10))) {
        goto err;
    }

out:
    syslog(LOG_ERR, "proc: %s, ip: %s, port: %s check OK!\n", proc, ip, portStr);
    ret = gsConnect(sockFd, addrS, addrLen);
    goto out2;

err:
    syslog(LOG_ERR, "proc: %s, ip: %s, port: %s check deny!\n", proc, ip, portStr);

out2:
    C_FREE_FUNC_NULL(ip, free);
    C_FREE_FUNC_NULL(portStr, free);

    return ret;
}

static int check_special_proc (const char* proc)
{
    if (NULL == proc) {
        return 1;
    }

    if ((0 == strcmp(proc, "sandbox-mate-terminal"))
        || (0 == strcmp(proc, "sandbox-nemo"))
        || (0 == strcmp(proc, "andsec-sandbox"))
        || (0 == strcmp(proc, "sandbox-scanner"))
        || (0 == strcmp(proc, "andsec"))
        || (strstr(proc, "gsd-"))
        || (strstr(proc, "systemd"))
        || (strstr(proc, "polkited"))
        || (strstr(proc, "gnome-shell"))
        || (strstr(proc, "dbus-daemon"))
        || (0 == strcmp(proc, "sec_daemon"))
        || (0 == strcmp(proc, "andsec-copy-files"))) {
        return 0;
    }

    return 1;
}

static int check_special_ip (const char* ip)
{
    if (NULL == ip) {
        return 1;
    }

    if ((0 == strncmp(ip, "127.0.0.", 8)) || (0 == strcmp(ip, "localhost"))) {
        return 0;
    }

    return 1;
}

static int get_port (const struct sockaddr_in* addr)
{
    return ntohs(addr->sin_port);
}

static char* get_ip (const struct sockaddr_in* addr)
{
#define IP_LEN 256

    char* ip = NULL;

    char* buf1 = (char*) malloc (IP_LEN);
    if (!buf1) {
        goto out;
    }
    memset(buf1, 0, IP_LEN);

    inet_ntop(AF_INET, &(addr->sin_addr), buf1, IP_LEN);
    if (strlen(buf1) > 0) {
        ip = strdup(buf1);
    }

out:
    C_FREE_FUNC_NULL(buf1, free);

    return ip;
}

static char* get_port_str (const struct sockaddr_in* addr)
{
    char portStr[32] = {0};

    snprintf(portStr, sizeof(portStr), "%d", get_port(addr));

    return strdup(portStr);
}
