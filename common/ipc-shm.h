//
// Created by dingjing on 25-7-14.
//

#ifndef client_IPC_SHM_H
#define client_IPC_SHM_H
#include "macros/macros.h"

C_BEGIN_EXTERN_C

/**
 * @brief 共享内存数据，保存需要重定向的进程名
 */
typedef struct __attribute__((__packed__))
{
    char                processName[8192000];           // 8MB 足够了
} ShmRedirectProcess;
C_STRUCT_SIZE_CHECK(ShmRedirectProcess, 8192000);

void ipc_shm_redirect_proc_info_free                    (void);
bool ipc_shm_redirect_proc_info_is_redirect_this_proc   (void);
bool ipc_shm_redirect_proc_info_is_redirect_by_pid      (int32_t pid);
bool ipc_shm_redirect_proc_info_init                    (bool isCreate);
bool ipc_shm_redirect_proc_info_is_redirect_by_name     (const char* procName);

/**
 * @param data 进程字符串， |proc1|proc2|proc3|...|
 * @param dataLen 字符串长度
 * @return 成功返回 true
 */
bool ipc_shm_redirect_proc_info_set_data                (const char* data, int64_t dataLen);


/**
 * @brief 共享内存数据，需要Hook的进程名
 */
typedef struct __attribute__((__packed__))
{
    char                processName[8192000];           // 8MB 足够了
} ShmHookProcess;
C_STRUCT_SIZE_CHECK(ShmHookProcess, 8192000);

bool ipc_shm_hook_proc_info_init                        (bool isCreate);
void ipc_shm_hook_proc_info_free                        (void);
bool ipc_shm_hook_proc_info_is_hook_this_proc           (void);
bool ipc_shm_hook_proc_info_is_hook_by_pid              (int32_t pid);
bool ipc_shm_hook_proc_info_is_hook_by_name             (const char* procName);
/**
 * @param data 进程字符串， |proc1|proc2|proc3|...|
 * @param dataLen 字符串长度
 * @return 成功返回 true
 */
bool ipc_shm_hook_proc_info_set_data                    (const char* data, int64_t dataLen);
/**
 * @brief 显示数据
 */
void ipc_shm_hook_show_data                             (void);


C_END_EXTERN_C

#endif // client_IPC_SHM_H
