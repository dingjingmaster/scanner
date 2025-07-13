//
// Created by dingjing on 25-6-9.
//

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "proc-inject.h"

#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syslog.h>

#include "utils.h"


struct _ProcInject
{
    bool                    isAttach;
    uint32_t                pid;
};

static int proc_get_uid (int pid);

static bool proc_get_proc_path (int pid, char buf[], int size);

bool proc_inject_inject_so_by_pid(int32_t pid, const char * libraryPath)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(pid > 0 && libraryPath, false, "Invalid pid or library path.");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN('/' == libraryPath[0], false, "Invalid path, Absolute library paths must be used.");
    C_RETURN_VAL_IF_FAIL(!Utils::checkProcLibraryExists(pid, libraryPath), true);

    bool ret = false;
    uintptr_t remoteAddr = 0;
    size_t libLen = strlen(libraryPath) + sizeof(char);
    ProcInject* injector = proc_inject_create_by_pid(pid);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "Failed to create injector.");

    do {
        C_BREAK_IF_FAIL_SYSLOG_WARN(proc_inject_attach_to(injector), "Could not attach to injector.");

        remoteAddr = proc_inject_remote_call(injector, (void*) malloc, 1, 1024);
        C_BREAK_IF_FAIL_SYSLOG_WARN(remoteAddr > 1, "Remote malloc failed!\n");
        C_BREAK_IF_FAIL_SYSLOG_WARN(proc_inject_write_memory(injector, remoteAddr, (uintptr_t)libraryPath, libLen + sizeof(char)), "Writing library path failed!\n");

        const uintptr_t dlopenRet = proc_inject_remote_call(injector, (void*) dlopen, 2, remoteAddr, RTLD_NOW | RTLD_GLOBAL);
        if (1 == dlopenRet) {
            break;
        }
        else {
            char errorStr[512] = {0};
            const uintptr_t errorAddr = proc_inject_remote_call(injector, (void*)dlerror, 0);
            C_BREAK_IF_FAIL_SYSLOG_WARN(1 != errorAddr, "dlerror called failed!\n");
            if ((uintptr_t) NULL == errorAddr) {
                ret = true;
                break;
            }
            C_BREAK_IF_FAIL_SYSLOG_WARN(!proc_inject_read_memory(injector, errorAddr, (uintptr_t)errorStr, sizeof(errorStr) - 1), "%s", errorStr);
        }
    } while (0);

    if (remoteAddr > 1) {
        proc_inject_remote_call(injector, (void*) free, 1, remoteAddr);
    }

    proc_inject_destroy(&injector);

    return ret;
}

bool proc_inject_inject_so_by_proc_name(const char* procName, const char* libraryPath)
{
    const int pid = Utils::getPidByProcName(procName);
    C_RETURN_VAL_IF_FAIL(pid > 0, false);

    return proc_inject_inject_so_by_pid(pid, libraryPath);
}

ProcInject* proc_inject_create_by_pid(uint32_t pid)
{
    ProcInject* injector = (ProcInject*)g_malloc0(sizeof(ProcInject));
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, NULL, "malloc error");

    injector->pid = pid;
    injector->isAttach = false;

    return injector;
}

void proc_inject_destroy(ProcInject** injector)
{
    if (injector && *injector) {
        proc_inject_detach_from(*injector);
        free(*injector);
        *injector = NULL;
    }
}

bool proc_inject_attach_to(ProcInject* injector)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(!injector->isAttach, true, "injector is already attached");

    for (int i = 0; i < 2; ++i) {
        if (0 == ptrace(PTRACE_ATTACH, (pid_t) injector->pid, NULL, NULL)) {
            injector->isAttach = true;
            break;
        }
        perror(strerror(errno));
        system("echo 0 > /proc/sys/kernel/yama/ptrace_scope");
    }

    return injector->isAttach;
}

bool proc_inject_detach_from(ProcInject* injector)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited!");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector->isAttach, false, "injector is not attached!");

    const long ret = ptrace(PTRACE_DETACH, (pid_t) injector->pid, NULL, NULL);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ret, false, "%s", strerror(errno));

    injector->isAttach = false;

    return true;
}

bool proc_inject_create_by_pid2(ProcInject* injector, uint32_t pid)
{
    C_RETURN_VAL_IF_FAIL(injector, false);

    memset(injector, 0, sizeof(ProcInject));

    injector->pid = pid;
    injector->isAttach = false;

    return true;
}

void proc_list_all(ProcIterator iter, void* data)
{
    C_RETURN_IF_FAIL_SYSLOG_WARN(iter, "iter is null!");

    struct dirent* entry = nullptr;
    DIR* dir = opendir("/proc");
    C_RETURN_IF_FAIL(dir);

    char procPath[PATH_MAX] = {0};

    while ((entry = readdir(dir)) != nullptr) {
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            const pid_t pid = static_cast<pid_t>(strtol(entry->d_name, nullptr, 10));
            if (pid > 0) {
                if (proc_get_proc_path(pid, procPath, sizeof(procPath))) {
                    if (!iter(pid, procPath, proc_get_uid(pid), data)) {
                        break;
                    }
                }
            }
        }
    }

    closedir(dir);
}

uintptr_t proc_inject_get_base(ProcInject* injector, const char * moduleName, bool isLocal)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(moduleName && moduleName[0] != '\0', false, "Invalid params");

    char filePath[64] = {0};
    char lineBuf[1024] = {0};

    uintptr_t startAddress = 0;
    if (isLocal) {
        snprintf(filePath, sizeof(filePath) - 1, "/proc/self/maps");
    }
    else {
        snprintf(filePath, sizeof(filePath) - 1, "/proc/%d/maps", injector->pid);
    }

    FILE* fp = fopen(filePath, "r");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(fp, startAddress, "%s", strerror(errno));

    while (fgets(lineBuf, sizeof(lineBuf), fp)) {
        if (strstr(lineBuf, moduleName)) {
            (void)sscanf(lineBuf, "%lx-", &startAddress);
            break;
        }
    }
    fclose(fp);

    return startAddress;
}

uintptr_t proc_inject_get_remote_func_addr(ProcInject* injector, char * moduleName, void * localFuncAddr)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, 0, "injector is not inited");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(moduleName && localFuncAddr, 0, "Invalid params");

    const uintptr_t localModuleAddr = proc_inject_get_base(injector, moduleName, true);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(localModuleAddr, 0, "Failed get local module base address");

    const uintptr_t remoteModuleAddr = proc_inject_get_base(injector, moduleName, false);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(remoteModuleAddr, 0, "Failed get remote module base address");

    const uintptr_t remoteFuncAddress = (uintptr_t) localFuncAddr - localModuleAddr + remoteModuleAddr;

    return remoteFuncAddress;
}

uintptr_t proc_inject_remote_call(ProcInject* injector, void * functionPointer, int count, ...)
{
    uintptr_t res = 1;
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, res, "injector is not inited");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(functionPointer, res, "Invalid params");

    int status = 0;
    char moduleName[1024] = {0};
    struct user_regs_struct returnRegisters, originalRegisters, tempRegisters;
    uintptr_t space = sizeof (uintptr_t), returnAddr = 0, remoteSymbolAddr = 0;

    bool ret = proc_inject_get_local_module_name(injector, functionPointer, moduleName, sizeof(moduleName));
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(ret, res, "injector_get_local_module_name failed");

    remoteSymbolAddr = proc_inject_get_remote_func_addr(injector, moduleName, functionPointer);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(remoteSymbolAddr > 0, res, "injector_get_remote_func_addr failed");

    long ptraceRet = ptrace(PTRACE_GETREGS, (pid_t) injector->pid, NULL, &tempRegisters);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptraceRet, res, "%s", strerror(errno));

    memcpy(&originalRegisters, &tempRegisters, sizeof(tempRegisters));
    while (0 != ((tempRegisters.rsp - space - 8) & 0xF)) {
        tempRegisters.rsp--;
    }

    va_list argList;
    va_start(argList, count);
    for (int i = 0; i < count && i < 6; i++) {
        uintptr_t arg = va_arg(argList, uintptr_t);
        switch (i) {
        case 0: tempRegisters.rdi = arg; break;
        case 1: tempRegisters.rsi = arg; break;
        case 2: tempRegisters.rdx = arg; break;
        case 3: tempRegisters.rcx = arg; break;
        case 4: tempRegisters.r8 = arg; break;
        case 5: tempRegisters.r9 = arg; break;
        default: break;
        }
    }
    va_end(argList);

    tempRegisters.rsp -= sizeof(uintptr_t);
    ret = proc_inject_write_memory(injector, tempRegisters.rsp, (uintptr_t) & returnAddr, sizeof(uintptr_t));
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(ret, res, "injector_write_memory failed\n");

    tempRegisters.rip = remoteSymbolAddr;
    tempRegisters.rax = 1;
    tempRegisters.orig_rax = 0;

    ptraceRet = ptrace(PTRACE_SETREGS, (pid_t) injector->pid, NULL, &tempRegisters);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptraceRet, res, "%s", strerror(errno));

    ptraceRet = ptrace(PTRACE_CONT, (pid_t) injector->pid, NULL, NULL);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptraceRet, res, "%s", strerror(errno));

    for (;;) {
        pid_t wp = waitpid((pid_t) injector->pid, &status, WUNTRACED);
        C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(wp == injector->pid, res, "Waitpid failed");

        C_BREAK_IF_OK(WIFSTOPPED(status) && (WSTOPSIG(status) == SIGSEGV || WSTOPSIG(status) == SIGKILL));
        C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(!WIFEXITED(status), res, "Error: process exited.\n");
        C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(!WIFSIGNALED(status), res, "Error: process terminated.\n");
        C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptrace(PTRACE_CONT, injector->pid, NULL, NULL), res, "Couldn't continue process.\n");
    }

    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptrace(PTRACE_GETREGS, injector->pid, NULL, &returnRegisters), res, "Couldn't get registers.\n");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 == ptrace(PTRACE_SETREGS, injector->pid, NULL, &originalRegisters), res, "Couldn't set registers.\n");

    return returnRegisters.rax;
}

bool proc_inject_read_memory(ProcInject* injector, uintptr_t address, uintptr_t out, size_t length)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited");

    struct iovec local = {
        .iov_base = (void*) out,
        .iov_len = length,
    };
    struct iovec remote = {
        .iov_base = (void*) address,
        .iov_len = length,
    };

    const ssize_t ret = process_vm_readv((pid_t) injector->pid, &local, 1, &remote, 1, 0);
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(ret >= 0, false, "%s", strerror(errno));

    return true;
}

bool proc_inject_write_memory(ProcInject* injector, uintptr_t address, uintptr_t data, size_t length)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited\n");

    struct iovec local = {
        .iov_base = (void*) data,
        .iov_len = length,
    };
    struct iovec remote = {
        .iov_base = (void*) address,
        .iov_len = length,
    };

    if ((ssize_t) length == process_vm_writev((pid_t) injector->pid, &local, 1, &remote, 1, 0)) {
        return true;
    }

    return false;
}

bool proc_inject_get_local_module_name(ProcInject* injector, void * address, char * moduleName, int moduleNameLen)
{
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(injector, false, "injector is not inited\n");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(address && moduleName && moduleNameLen > 0, false, "Invalid params\n");

    FILE* fp = NULL;
    char line[4096] = {0};
    char filePath[64] = {0};

    snprintf(filePath, sizeof(filePath), "/proc/%d/maps", getpid());
    fp = fopen(filePath, "r");
    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(fp, false, "%s", strerror(errno));

    while (fgets(line, sizeof(line), fp)) {
        uintptr_t endAddr = 0;
        uintptr_t startAddr = 0;
        (void) sscanf (line, "%lx-%lx ", &startAddr, &endAddr);
        if ((uintptr_t) address >= startAddr && (uintptr_t) address <= endAddr) {
            char* libPath = strchr(line, '/');
            if (libPath) {
                char* newLine = strchr(libPath, '\n');
                if (newLine) {
                    *newLine = '\0';
                }
                memset(moduleName, 0, moduleNameLen);
                strncpy(moduleName, libPath, moduleNameLen - 1);
                fclose(fp);
                return true;
            }
        }
    }

    fclose(fp);
    return false;
}

static int proc_get_uid (int pid)
{
    uid_t uid = 0;
    char line[256];
    FILE* fp = nullptr;
    char filePath[32] = {0};

    snprintf(filePath, sizeof(filePath) - 1, "/proc/%d/status", pid);

    // 打开 /proc/self/status 文件
    fp = fopen(filePath, "r");
    if (fp == nullptr) {
        return -1;
    }

    // 逐行读取文件内容
    while (fgets(line, sizeof(line), fp)) {
        // 查找包含 "Uid:" 的行
        if (strncmp(line, "Uid:", 4) == 0) {
            // 解析 UID 值
            sscanf(line, "Uid:\t%u", &uid);
            break;
        }
    }

    // 关闭文件
    fclose(fp);

    return uid;
}

static bool proc_get_proc_path (int pid, char buf[], int size)
{
    memset(buf, 0 , size);

    char exe[128] = {0};
    snprintf(exe, sizeof(exe), "/proc/%d/exe", pid);

    return readlink(exe, buf, size - 1) > 0;
}
