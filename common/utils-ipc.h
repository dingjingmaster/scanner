//
// Created by dingjing on 25-6-10.
//

#ifndef client_UTILS_IPC_H
#define client_UTILS_IPC_H
#include "ipc.h"

C_BEGIN_EXTERN_C

void utils_ipc_send_data_to_local_socket                (const char* clientSock, int ipcType, const char* data, uint64_t dataLen);
void utils_ipc_send_data_to_local_socket_with_response  (const char* clientSock, int ipcType, const char* data, uint64_t dataLen, char** response, uint64_t* responseLen);

C_END_EXTERN_C

#endif // client_UTILS_IPC_H
