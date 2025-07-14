//
// Created by dingjing on 25-6-10.
//

#include "utils-ipc.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"


void utils_ipc_send_data_to_local_socket(const char * clientSock, int ipcType, const char * data, uint64_t dataLen)
{
    const cuint64 len = sizeof (struct IpcMessage) + dataLen + 1;
    char* buffer = malloc(len);
    memset(buffer, 0, len);
    C_RETURN_IF_FAIL(buffer);

    struct IpcMessage msg;
    msg.type = ipcType;
    msg.dataLen = dataLen;

    memcpy(buffer, &msg, sizeof(struct IpcMessage));
    memcpy(buffer + sizeof(struct IpcMessage), data, dataLen);

    utils_send_data_to_local_socket(clientSock, buffer, len);

    C_FREE_FUNC_NULL(buffer, free);
}

void utils_ipc_send_data_to_local_socket_with_response(const char * clientSock, int ipcType, const char * data, uint64_t dataLen, char ** response, uint64_t* responseLen)
{
    const cuint64 len = sizeof (struct IpcMessage) + dataLen + 1;
    char* buffer = malloc(len);
    memset(buffer, 0, len);
    C_RETURN_IF_FAIL(buffer);

    struct IpcMessage msg;
    msg.type = ipcType;
    msg.dataLen = dataLen;

    memcpy(buffer, &msg, sizeof(struct IpcMessage));
    memcpy(buffer + sizeof(struct IpcMessage), data, dataLen);

    if (responseLen) {
        *responseLen = utils_send_data_to_local_socket_with_response(clientSock, buffer, len, response);
    }

    C_FREE_FUNC_NULL(buffer, free);
}
