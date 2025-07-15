//
// Created by dingjing on 25-6-9.
//

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <bits/socket.h>

#include "ipc.h"
#include "proc-list.h"
#include "macros/macros.h"

struct ProcName
{
    pid_t       pid;
    const char* name;
};


static bool check_is_dynamic_library (const char* path);
static bool check_has_gui (int pid, const char* procPath, int uid, bool isGui, void* udata);
static bool check_has_daemon (int pid, const char* procPath, int uid, bool isGui, void* udata);
static bool get_proc_pid (int pid, const char* procPath, int uid, bool isGui, void* udata);

bool utils_check_sec_gui_exists()
{
    bool result = false;
    proc_list_all(check_has_gui, &result);
    return result;
}

bool utils_check_sec_daemon_exists()
{
    bool result = false;
    proc_list_all(check_has_daemon, &result);
    return result;
}

int utils_get_pid_by_name(const char * name)
{
    char procName[256] = {0};
    snprintf(procName, sizeof(procName), "/%s", name);
    struct ProcName data = { .name = procName };

    proc_list_all(get_proc_pid, &data);

    return data.pid;
}

bool utils_check_proc_instance_exists (const char* lockFile)
{
    int fd = open(lockFile, O_RDONLY | O_CREAT, 0666);
    if (fd >= 0) {
        if (flock (fd, LOCK_EX | LOCK_NB) >= 0) {
            close(fd);
            return true;
        }
        close(fd);
    }

    return false;
}

bool utils_check_proc_library_exists(int pid, const char * libraryPath)
{
    FILE* fr = NULL;
    bool ret = false;
    char mapsPath[128] = {0};
    char lineBuf[1024] = {0};

    snprintf(mapsPath, sizeof(mapsPath) - 1, "/proc/%d/maps", pid);

    fr = fopen(mapsPath, "r");
    if (fr) {
        while (fgets(lineBuf, sizeof(lineBuf), fr)) {
            lineBuf[strcspn(lineBuf, "\n")] = 0;

            // /proc/self/maps 格式：
            // 地址范围 权限 偏移量 设备 inode 路径名
            // 例如：7f1234567000-7f1234568000 r-xp 00000000 08:01 1234567 /lib/x86_64-linux-gnu/libc.so.6
            char* tokens[8] = { NULL };
            char* token = strtok(lineBuf, " ");
            int tokenCount = 0;

            // 分割行内容
            while (token && tokenCount < 6) {
                tokens[tokenCount++] = token;
                token = strtok(NULL, " ");
            }

            // 如果有路径字段(第6列)
            if (tokenCount >= 6) {
                char* path = tokens[5];
                // 跳过剩余的空格，获取完整的路径
                while (*path == ' ') { path++; }
                // 检查是否是动态库
                if (check_is_dynamic_library(path) && 0 == c_strcmp(libraryPath, path)) {
                    ret = true;
                    break;
                }
            }
        }
        fclose(fr);
    }

    return ret;
}

bool utils_check_proc_environ_exists_key_value(int pid, const char * key, const char * value)
{
    char buffer[81920] = {0};
    char path[32] = {0};

    snprintf(path, sizeof(path), "/proc/%d/environ", pid);

    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        size_t len = read(fd, buffer, sizeof(buffer));
        close(fd);
        if (len > 0) {
            buffer[len] = '\0';
            char* p = buffer;
            while (p < buffer + len) {
                char* eq = strchr(p, '=');
                if (eq) {
                    *eq = '\0';
                    char* val = eq + 1;
                    char* end = strchr(val, '\0');
                    printf("'%s' = '%s'", p, val);
                    if (0 == strcmp(p, key) && strstr(val, value)) {
                        return true;
                    }
                    p = end + 1;
                }
            }
        }
    }

    return false;
}

void utils_get_proc_by_pid_array(int pid, char fileBuf[], size_t fileBufSize)
{
    C_RETURN_IF_FAIL(pid >= 0 && fileBuf && fileBufSize > 0);

    char exe[1024] = {0};
    snprintf(exe, sizeof(exe), "/proc/%d/exe", pid);

    (void) readlink (exe, fileBuf, fileBufSize);
}

void utils_send_data_to_local_socket(const char* localSocket, const char * data, size_t dataSize)
{
    utils_send_data_to_local_socket_with_response(localSocket, data, dataSize, NULL);
}

uint64_t utils_send_data_to_local_socket_with_response(const char * localSocket, const char * data, size_t dataSize, char ** response)
{
    int fd = 0;
    uint64_t recvBufLen = 0;

    do {
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        C_BREAK_IF_FAIL(fd >= 0);

        int reuse = 1;
        int timeout = 2000;
        int recvTimeout = 30 * 1000;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout));

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_LOCAL;
        strncpy(addr.sun_path, localSocket, sizeof(addr.sun_path) - 1);

        C_BREAK_IF_FAIL_SYSLOG_WARN((0 == connect(fd, (struct sockaddr*) &addr, sizeof(addr))),
            "connect error: %s", strerror(errno));

        C_BREAK_IF_FAIL_SYSLOG_WARN((dataSize == write(fd, data, dataSize)),
            "write error: %s", strerror(errno));

        if (response) {
            C_FREE_FUNC_NULL(*response, free);
#define COPY_BUFFER(buf, size) \
    *response = realloc(*response, recvBufLen + size); \
    memcpy(&((*response)[recvBufLen]), buf, size); \
    recvBufLen += size;
            char buf[1024] = {0};
            ssize_t readSize = 0;
            while (true) {
                readSize = read(fd, buf, sizeof(buf));
                if (readSize < sizeof(buf)) {
                    COPY_BUFFER(buf, readSize)
                    break;
                }
                COPY_BUFFER(buf, sizeof(buf));
            }
        }
    } while (false);

    return recvBufLen;
}

uint64_t utils_send_data_to_local_socket_with_response_data(const char* localSocket, int ipcType, const char* data, size_t dataSize, char** response)
{
    C_RETURN_VAL_IF_FAIL(localSocket && data, 0);
    const uint64_t allDataLen = sizeof(struct IpcMessage) + dataSize + 1;
    char* allData = malloc(allDataLen);
    C_RETURN_VAL_IF_FAIL(allData, false);
    memset(allData, 0, allDataLen);
    struct IpcMessage* ipcMsg = (void*) allData;
    ipcMsg->type = ipcType;
    ipcMsg->dataLen = dataSize;
    memcpy(ipcMsg->data, data, dataSize);

    char* resp = NULL;
    uint64_t recvBufLen = 0;
    const uint64_t respDataLen = utils_send_data_to_local_socket_with_response(localSocket, allData, allDataLen, &resp);
    C_FREE_FUNC_NULL(allData, free);
    if (respDataLen >= sizeof(struct IpcMessage)) {
        const struct IpcMessage* respMsg = (struct IpcMessage*) resp;
        C_FREE_FUNC_NULL(*response, free);
        if (respMsg->dataLen > 0) {
            *response = malloc(respMsg->dataLen);
            recvBufLen = respMsg->dataLen;
            memcpy(*response, respMsg->data, recvBufLen);
        }
    }
    C_FREE_FUNC_NULL(resp, free);

    return recvBufLen;
}

static bool check_has_gui (int pid, const char* procPath, int uid, bool isGui, void* udata)
{
    if (0 == strcmp("/usr/local/DocSecManager/bin/sec_gui", procPath)) {
        *(bool*) udata = true;
        return false;
    }

    return true;
    (void) pid;
    (void) uid;
    (void) isGui;
}

static bool check_has_daemon (int pid, const char* procPath, int uid, bool isGui, void* udata)
{
    if (0 == strcmp("/usr/local/DocSecManager/bin/sec_daemon", procPath)) {
        *(bool*) udata = true;
        return false;
    }

    return true;
    (void) pid;
    (void) uid;
    (void) isGui;
}

static bool check_is_dynamic_library (const char* path)
{
    C_RETURN_VAL_IF_FAIL(path && strlen(path) > 0, false);

    const char* ext = strrchr(path, '.');
    C_RETURN_VAL_IF_OK(ext && 0 == c_strcmp(ext, ".so"), true);
    C_RETURN_VAL_IF_OK(strstr(path, ".so."), true);

    return false;
}

static bool get_proc_pid (int pid, const char* procPath, int uid, bool isGui, void* udata)
{
    C_RETURN_VAL_IF_FAIL(procPath && strlen(procPath) > 0, false);

    struct ProcName* data = (struct ProcName*) udata;

    if (strstr(procPath, data->name)) {
        data->pid = pid;
        return false;
    }

    (void) uid;
    (void) isGui;

    return true;
}
