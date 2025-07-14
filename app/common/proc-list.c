//
// Created by dingjing on 25-6-9.
//

#include "proc-list.h"

#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>


static int proc_get_uid (int pid);
static bool proc_get_proc_path  (int pid, char buf[], int size);
static bool proc_is_gui_process (const char* environData, size_t size);

void proc_list_all(ProcIterator iter, void* data)
{
    C_RETURN_IF_FAIL_SYSLOG_WARN(iter, "iter is null!");

    struct dirent* entry = NULL;
    DIR* dir = opendir("/proc");
    C_RETURN_IF_FAIL(dir);

    size_t readSize = 0;
    char procPath[PATH_MAX];
    char procBuf[128] = {0};
    char environData[8192] = {0};

    while ((entry = readdir(dir)) != NULL) {
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            pid_t pid = (pid_t) strtol (entry->d_name, NULL, 10);
            if (pid > 0) {
                memset(procBuf, 0, sizeof(procBuf));
                snprintf(procBuf, sizeof(procBuf), "/proc/%s/environ", entry->d_name);
                if (0 == access(procBuf, F_OK)) {
                    FILE* fp = fopen(procBuf, "r");
                    if (fp) {
                        readSize = fread(environData, sizeof(char), sizeof(environData) - 1, fp);
                        environData[readSize] = '\0';
                        fclose(fp);
                        if (proc_get_proc_path(pid, procPath, sizeof(procPath))) {
                            if (proc_is_gui_process(environData, readSize)) {
                                if (iter(pid, procPath, proc_get_uid(pid), true, data)) { continue; }
                            }
                            else {
                                if (iter(pid, procPath, proc_get_uid(pid), false, data)) { continue; }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}

static int proc_get_uid (int pid)
{
    uid_t uid = 0;
    char line[256];
    FILE* fp = NULL;
    char filePath[32] = {0};

    snprintf(filePath, sizeof(filePath) - 1, "/proc/%d/status", pid);

    // 打开 /proc/self/status 文件
    fp = fopen(filePath, "r");
    if (fp == NULL) {
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

static bool proc_is_gui_process (const char* environData, size_t size)
{
    static const char* guiIndicators[] = {
        "DISPLAY=",
        "WAYLAND_DISPLAY=",
        "XDG_SESSION_TYPE=",
        "GDMSESSION=",
        "GNOME_DESKTOP_SESSION_ID=",
        "KDE_FULL_SESSION=",
        "DESKTOP_SESSION=",
        NULL,
    };

    const char* env = environData;
    while (env < environData + size) {
        for (int i = 0; guiIndicators[i] != NULL; i++) {
            if (0 == strncmp(env, guiIndicators[i], strlen(guiIndicators[i]))) {
                return true;
            }
        }
        env += strlen(env) + 1;
    }
    return false;
}

static bool proc_get_proc_path (int pid, char buf[], int size)
{
    memset(buf, 0 , size);

    char exe[128] = {0};
    snprintf(exe, sizeof(exe), "/proc/%d/exe", pid);

    return readlink(exe, buf, size - 1) > 0;
}
