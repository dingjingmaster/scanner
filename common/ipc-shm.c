//
// Created by dingjing on 25-7-14.
//

#include "ipc-shm.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>
#include <linux/limits.h>


#define IPC_PROC_SHM_KEY            986689


typedef struct
{
    bool                            isOwner;
    bool                            isInited;
    int32_t                         shmHandle;
    void*                           data;
    uint64_t                        dataLen;
    pthread_rwlock_t                lock;
} ShmPrivate;

static void clean_shm_ipc           (void);
static void clean_shm_ipc_sig       (int sig);
static char* get_process_name       (pid_t pid);
static void shm_destroy             (ShmPrivate* shmPrivate);
static bool shm_set_data            (ShmPrivate* shmPrivate, const void* data, int64_t dataLen);
static bool shm_init                (bool isCreate, key_t shmKey, ShmPrivate* shmPrivate, size_t dataLen);


static bool shm_redirect_proc_info_check_proc   (const char* procName/*|procName|*/);
static bool shm_hook_proc_info_check_proc       (const char* procName/*|procName|*/);


static ShmPrivate gsRedirectInfoPrivate;


bool ipc_shm_redirect_proc_info_init(bool isCreate)
{
    return shm_init(isCreate, IPC_PROC_SHM_KEY, &gsRedirectInfoPrivate, sizeof(ShmRedirectProcess));
}

void ipc_shm_redirect_proc_info_free(void)
{
    shm_destroy(&gsRedirectInfoPrivate);
}

bool ipc_shm_redirect_proc_info_is_redirect_this_proc(void)
{
    return ipc_shm_redirect_proc_info_is_redirect_by_pid(getpid());
}

bool ipc_shm_redirect_proc_info_is_redirect_by_pid(int32_t pid)
{
    char* buf = get_process_name(pid);
    C_RETURN_VAL_IF_FAIL(buf, false);

    const bool ret = ipc_shm_redirect_proc_info_is_redirect_by_name(buf);

    C_FREE_FUNC(buf, free);

    return ret;
}

bool ipc_shm_redirect_proc_info_set_data(const char * data, int64_t dataLen)
{
    C_RETURN_VAL_IF_FAIL(data && dataLen > 0, false);

    int64_t dataLenTrue = dataLen;
    const size_t mallocSize = dataLen + 12;
    char* dataT = malloc (mallocSize);
    C_RETURN_VAL_IF_FAIL(dataT, false);
    memset(dataT, 0, mallocSize);

    const bool prefix = c_str_has_prefix(data, "|");
    const bool suffix = c_str_has_suffix(data, "|");

    if (!prefix && !suffix) {
        dataLenTrue += 2;
        snprintf(dataT, mallocSize, "|%s|", data);
    }
    else if (!prefix) {
        dataLenTrue += 1;
        snprintf(dataT, mallocSize, "|%s", data);
    }
    else if (!suffix) {
        dataLenTrue += 1;
        snprintf(dataT, mallocSize, "%s|", data);
    }

    const bool ret = shm_set_data(&gsRedirectInfoPrivate, dataT, dataLenTrue);

    C_FREE_FUNC_NULL(dataT, free);

    return ret;
}

bool ipc_shm_redirect_proc_info_is_redirect_by_name(const char* procName)
{
    C_RETURN_VAL_IF_FAIL(procName, false);

    const uint64_t dataLen = strlen(procName);
    const uint64_t mallocSize = dataLen + 12;
    char* buf = malloc (mallocSize);
    C_RETURN_VAL_IF_FAIL(buf, false);
    memset(buf, 0, mallocSize);

    const bool prefix = c_str_has_prefix(procName, "|");
    const bool suffix = c_str_has_suffix(procName, "|");

    if (!prefix && !suffix) {
        snprintf(buf, mallocSize, "|%s|", procName);
    }
    else if (!prefix) {
        snprintf(buf, mallocSize, "|%s", procName);
    }
    else if (!suffix) {
        snprintf(buf, mallocSize, "%s|", procName);
    }

    c_str_ascii_to_lower(buf);

    return shm_redirect_proc_info_check_proc(buf);
}

static bool shm_redirect_proc_info_check_proc (const char* procName/*|procName|*/)
{
    C_RETURN_VAL_IF_FAIL(procName, false);

    bool found = false;

    pthread_rwlock_rdlock(&gsRedirectInfoPrivate.lock);
    const ShmRedirectProcess* data = gsRedirectInfoPrivate.data;
    if (gsRedirectInfoPrivate.data && data->processName[0] != '\0') {
        found = (NULL != strstr(data->processName, procName)) ? true : false;
    }
    pthread_rwlock_unlock(&gsRedirectInfoPrivate.lock);

    return found;
}



#define IPC_HOOK_SHM_KEY            989898
static ShmPrivate gsHookInfoPrivate;

bool ipc_shm_hook_proc_info_init (bool isCreate)
{
    const key_t shmKey = ftok("/tmp/client-ipc-shm", IPC_HOOK_SHM_KEY);
    return shm_init(isCreate, shmKey, &gsHookInfoPrivate, sizeof(ShmHookProcess));
}

void ipc_shm_hook_proc_info_free (void)
{
    shm_destroy(&gsHookInfoPrivate);
}

bool ipc_shm_hook_proc_info_is_hook_this_proc (void)
{
    return ipc_shm_hook_proc_info_is_hook_by_pid(getpid());
}

bool ipc_shm_hook_proc_info_is_hook_by_pid(int32_t pid)
{
    char* buf = get_process_name(pid);
    C_RETURN_VAL_IF_FAIL(buf, false);

    const bool ret = ipc_shm_hook_proc_info_is_hook_by_name(buf);

    C_FREE_FUNC(buf, free);

    return ret;
}

bool ipc_shm_hook_proc_info_is_hook_by_name(const char* procName)
{
    C_RETURN_VAL_IF_FAIL(procName, false);

    const uint64_t dataLen = strlen(procName);
    const uint64_t mallocSize = dataLen + 12;
    char* buf = malloc (mallocSize);
    C_RETURN_VAL_IF_FAIL(buf, false);

    memset(buf, 0, mallocSize);

    const bool prefix = c_str_has_prefix(procName, "|");
    const bool suffix = c_str_has_suffix(procName, "|");

    if (!prefix && !suffix) {
        snprintf(buf, mallocSize, "|%s|", procName);
    }
    else if (!prefix) {
        snprintf(buf, mallocSize, "|%s", procName);
    }
    else if (!suffix) {
        snprintf(buf, mallocSize, "%s|", procName);
    }

    c_str_ascii_to_lower(buf);

    return shm_hook_proc_info_check_proc(buf);
}

bool ipc_shm_hook_proc_info_set_data (const char* data, int64_t dataLen)
{
    C_RETURN_VAL_IF_FAIL(data && dataLen > 0, false);

    int64_t dataLenTrue = dataLen;
    const size_t mallocSize = dataLen + 12;
    char* dataT = malloc (mallocSize);
    C_RETURN_VAL_IF_FAIL(dataT, false);
    memset(dataT, 0, mallocSize);

    const bool prefix = c_str_has_prefix(data, "|");
    const bool suffix = c_str_has_suffix(data, "|");

    if (!prefix && !suffix) {
        dataLenTrue += 2;
        snprintf(dataT, mallocSize, "|%s|", data);
    }
    else if (!prefix) {
        dataLenTrue += 1;
        snprintf(dataT, mallocSize, "|%s", data);
    }
    else if (!suffix) {
        dataLenTrue += 1;
        snprintf(dataT, mallocSize, "%s|", data);
    }

    c_str_ascii_to_lower(dataT);

    const bool ret = shm_set_data(&gsHookInfoPrivate, dataT, dataLenTrue);

    C_FREE_FUNC_NULL(dataT, free);

    return ret;
}

void ipc_shm_hook_show_data(void)
{
    pthread_rwlock_wrlock(&gsHookInfoPrivate.lock);
    char* shmData = gsHookInfoPrivate.data;
    if (shmData && shmData[0] != '\0') {
        printf("%s\n", shmData);
    }

    pthread_rwlock_unlock(&gsHookInfoPrivate.lock);
}


static bool shm_init (bool isCreate, key_t shmKey, ShmPrivate* shmPrivate, size_t dataLen)
{
    C_RETURN_VAL_IF_FAIL(shmPrivate && dataLen > 0, false);

    int err = 0;
    memset(shmPrivate, 0, sizeof(ShmPrivate));
    shmPrivate->isInited = false;
    shmPrivate->isOwner = isCreate;
    shmPrivate->dataLen = dataLen;

    // syslog(LOG_INFO, "key %x, dataLen: %ld", IPC_PROC_SHM_KEY, dataLen);

    do {
        // shm
        errno = 0;
        const int flags = (shmPrivate->isOwner ? (IPC_CREAT | 0666) : 0);
        int ret = shmget(shmKey, dataLen, flags);
        C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(ret >= 0, false, "shmget error: %d, %s", errno, strerror(errno));
        shmPrivate->shmHandle = ret;

        // attaches memory
        const int attachesFlags = (isCreate ? 0 : SHM_RDONLY);
        shmPrivate->data = shmat (shmPrivate->shmHandle, NULL, attachesFlags);
        C_BREAK_IF_FAIL_SYSLOG_WARN(shmPrivate->data != NULL, "shmat error: %s", strerror(errno));
        {
            atexit(clean_shm_ipc);
            signal(SIGINT, clean_shm_ipc_sig);
            signal(SIGABRT, clean_shm_ipc_sig);
            signal(SIGSEGV, clean_shm_ipc_sig);
            signal(SIGKILL, clean_shm_ipc_sig);
        }

        // 初始化读写锁，以允许进程间共享
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_rwlock_init(&shmPrivate->lock, NULL);
        ret = pthread_rwlock_init(&shmPrivate->lock, &attr);
        err = errno;
        pthread_rwlockattr_destroy(&attr);
        C_BREAK_IF_FAIL_SYSLOG_WARN(ret == 0, false, "pthread_rwlock_init error: %s", strerror(err));

        // 初始化共享内存
        pthread_rwlock_wrlock(&shmPrivate->lock);
        if (isCreate) {
            memset(shmPrivate->data, 0, shmPrivate->dataLen);
        }
        pthread_rwlock_unlock(&shmPrivate->lock);
        shmPrivate->isInited = true;
    } while (false);


    return shmPrivate->isInited;
}

static void shm_destroy (ShmPrivate* shmPrivate)
{
    C_RETURN_IF_FAIL(shmPrivate && (shmPrivate->isInited || shmPrivate->isOwner));

    pthread_rwlock_wrlock(&shmPrivate->lock);
    if (shmPrivate->isOwner) {
        memset(shmPrivate->data, 0, shmPrivate->dataLen);
        if (0 != shmdt(shmPrivate->data)) { syslog(LOG_ERR, "shmdt error: %s", strerror(errno)); }
        if (0 != shmctl(shmPrivate->shmHandle, IPC_RMID, NULL)) { syslog(LOG_ERR, "shmctl error: %s", strerror(errno)); }
    }
    else {
        if (0 != shmdt(shmPrivate->data)) { syslog(LOG_ERR, "shmdt error: %s", strerror(errno)); }
    }
    pthread_rwlock_unlock(&shmPrivate->lock);
    pthread_rwlock_destroy(&shmPrivate->lock);
}

static bool shm_set_data (ShmPrivate* shmPrivate, const void* data, int64_t dataLen)
{
    C_RETURN_VAL_IF_FAIL(shmPrivate && data && dataLen > 0, false);

    bool ret = false;

    pthread_rwlock_wrlock(&shmPrivate->lock);
    void* shmData = shmPrivate->data;
    C_RETURN_VAL_IF_FAIL(shmData, false);

    do {
        C_BREAK_IF_FAIL_SYSLOG_WARN(dataLen < shmPrivate->dataLen, "data too long %ld < %ld!", dataLen, shmPrivate->dataLen);
        memset(shmData, 0, shmPrivate->dataLen);
        memcpy(shmData, data, dataLen);
        ret = true;
    } while (false);

    pthread_rwlock_unlock(&shmPrivate->lock);

    return ret;
}

static char* get_process_name (pid_t pid)
{
    char path[128] = {0};
    char buf[PATH_MAX] = {0};

    snprintf(path, sizeof(path) - 1, "/proc/%d/exe", pid);

    C_RETURN_VAL_IF_FAIL_SYSLOG_WARN(0 <= readlink(path, buf, sizeof (buf) - 1), false, "readlink error: %s", strerror(errno));

    int i = 0;
    char* ptr = NULL;
    for (i = 0; buf[i]; ++i) {
        if (buf[i] == '/') {
            ptr = &buf[i];
        }
    }

    C_RETURN_VAL_IF_FAIL(ptr, false);

    return strdup(ptr);
}

static bool shm_hook_proc_info_check_proc (const char* procName/*|procName|*/)
{
    C_RETURN_VAL_IF_FAIL(procName, false);

    bool found = false;

    pthread_rwlock_rdlock(&gsHookInfoPrivate.lock);
    const ShmHookProcess* data = gsHookInfoPrivate.data;
    if (gsHookInfoPrivate.data && data->processName[0] != '\0') {
        found = (NULL != strstr(data->processName, procName)) ? true : false;
    }
    pthread_rwlock_unlock(&gsHookInfoPrivate.lock);

    return found;
}

static void clean_shm_ipc (void)
{
    if (gsHookInfoPrivate.isOwner) {
        shm_destroy(&gsHookInfoPrivate);
    }

    if (gsRedirectInfoPrivate.isOwner) {
        shm_destroy(&gsRedirectInfoPrivate);
    }
}

static void clean_shm_ipc_sig (int sig)
{
    clean_shm_ipc();

    (void) sig;
}
