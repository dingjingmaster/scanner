//
// Created by dingjing on 25-7-14.
//

#ifndef andsec_scanner_IPC_C_H
#define andsec_scanner_IPC_C_H
#include "macros/macros.h"

C_BEGIN_EXTERN_C

typedef enum
{
    IPC_TYPE_NONE = 0,

    IPC_TYPE_SERVER_STOP_TASK,          // 1 停止
    IPC_TYPE_SERVER_PAUSE_TASK,         // 2 暂停
    IPC_TYPE_SERVER_START_TASK,         // 3 扫描中

    IPC_TYPE_INJECT_LIB_BY_PID = 10,    // 10 通过 pid 注入动态库
    IPC_TYPE_INJECT_LIB_BY_PROC_NAME,   // 11 通过 进程名 注入动态库

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
