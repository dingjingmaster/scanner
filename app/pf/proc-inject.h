//
// Created by dingjing on 25-6-9.
//

#ifndef client_PROC_INJECT_H
#define client_PROC_INJECT_H

#include <gio/gio.h>

#include "macros/macros.h"
#include "../common/proc-list.h"


typedef struct _ProcInject      ProcInject;
// typedef bool (*ProcIterator)    (int pid, const char* procPath, int uid, void* uData);

/**
 * @brief 将动态库libraryPath，注入pid指定进程
 * @param pid
 * @param libraryPath
 * @return
 */
bool            proc_inject_inject_so_by_pid        (int32_t pid, const char* libraryPath);
bool            proc_inject_inject_so_by_proc_name  (const char* procName, const char* libraryPath);
void            proc_inject_inject_all_gui_proc     (void);


ProcInject*     proc_inject_create_by_pid           (uint32_t pid);
void            proc_inject_destroy                 (ProcInject**injector);
bool            proc_inject_attach_to               (ProcInject* injector);
bool            proc_inject_detach_from             (ProcInject* injector);
bool            proc_inject_create_by_pid2          (ProcInject* injector, uint32_t pid);
// void            proc_list_all                       (ProcIterator iter, void* data);

/**
 * @brief 获取模块的起始地址
 * @param injector
 * @param moduleName
 * @param isLocal
 * @return
 */
uintptr_t       proc_inject_get_base                (ProcInject* injector, const char* moduleName, bool isLocal);

/**
 * @brief 计算目标地址中，相关模块的地址
 * @param injector
 * @param moduleName
 * @param localFuncAddr 本地函数地址
 * @return
 */
uintptr_t       proc_inject_get_remote_func_addr    (ProcInject* injector, char* moduleName, void* localFuncAddr);

/**
 * @brief 调用远程进程的指定函数
 * @param injector
 * @param functionPointer
 * @param count
 * @param ...
 * @return >1 才说明调用成功
 */
uintptr_t       proc_inject_remote_call             (ProcInject* injector, void* functionPointer, int count, ...);


/**
 * @brief 从进程中的 address 处读取数据
 * @param injector
 * @param address 要读取的地址(此进程地址)
 * @param out 本地buffer
 * @param length 要读取的字节数
 * @return
 */
bool            proc_inject_read_memory             (ProcInject* injector, uintptr_t address, uintptr_t out, size_t length);

/**
 * @brief 将内存数据写到指定地址
 * @param injector
 * @param address 远程地址(远程地址)
 * @param data 本地数据指针
 * @param length 要写的数据长度
 * @return
 */
bool            proc_inject_write_memory            (ProcInject* injector, uintptr_t address, uintptr_t data, size_t length);

/**
 * @brief
 * @param injector
 * @param address 要查找到地址
 * @param moduleName
 * @param moduleNameBufLen moduleName保存区域的最大大小
 * @return
 */
bool            proc_inject_get_local_module_name   (ProcInject* injector, void* address, char* moduleName, int moduleNameBufLen);


#endif // client_PROC_INJECT_H
