//
// Created by dingjing on 25-7-14.
//

#ifndef andsec_scanner_IPC_C_H
#define andsec_scanner_IPC_C_H
#include "macros/macros.h"


C_BEGIN_EXTERN_C

#ifndef IPC_SERVER_SOCKET_PATH
#define IPC_SERVER_SOCKET_PATH              "/usr/local/andsec/start/sec_daemon.sock"
#endif

#ifndef HOOK_CORE
#define HOOK_CORE                           "/usr/local/andsec/lib/hook-main.so"
#endif

#ifndef HOOK_PRELOAD
#define HOOK_PRELOAD                        "/usr/local/andsec/lib/hook-ld_preload.so"
#endif


typedef enum
{
    IPC_TYPE_NONE = 0,

    IPC_TYPE_SERVER_STOP_TASK = 1,                  // 1 停止
    IPC_TYPE_SERVER_PAUSE_TASK = 2,                 // 2 暂停
    IPC_TYPE_SERVER_START_TASK = 3,                 // 3 扫描中

    IPC_TYPE_INJECT_LIB_BY_PID = 10,                // 10 通过 pid 注入动态库
    IPC_TYPE_INJECT_LIB_BY_PROC_NAME,               // 11 通过 进程名 注入动态库
    IPC_TYPE_INJECT_LIB_ALL_GUI_PROC,               // 12 所有图形进程中注入动态库
    IPC_TYPE_INJECT_ALL_ALL_GUI_PROC,               // 13 所有图形进程中注入默认动态库


    // from sec_daemon
    IPC_TYPE_SCREENSHOT = 31,                       // 31 截屏控制
    IPC_TYPE_CHECK_PROC_NETWORK_FORBIDDEN = 32,     // 32 检测是否禁止联网

    IPC_TYPE_NUM
} IpcServerType;

struct __attribute__((packed)) IpcMessage
{
    unsigned int        type;                       // 处理类型：IpcServerType、IpcClientType
    unsigned long       dataLen;
    char                data[];
};


C_END_EXTERN_C

#endif // andsec_scanner_IPC_C_H
