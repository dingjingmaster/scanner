//
// Created by dingjing on 25-6-9.
//

#ifndef client_PROC_LIST_H
#define client_PROC_LIST_H
#include "macros/macros.h"

C_BEGIN_EXTERN_C

typedef bool (*ProcIterator) (int pid, const char* procPath, int uid, bool isGui, void* uData);

void proc_list_all (ProcIterator iter, void* data);

C_END_EXTERN_C

#endif // client_PROC_LIST_H
