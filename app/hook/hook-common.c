//
// Created by dingjing on 25-7-15.
//
#include "hook-common.h"

#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "common/ipc.h"
#include "common/utils.h"


bool hc_check_screenshot_control_forbidden()
{
    bool res = false;

    char* buf = NULL;
    char sendBuf[8192] = {0};
    char procName[1024] = {0};

    utils_get_proc_by_pid_array(getpid(), procName, sizeof(procName));

    const int ret = snprintf(sendBuf, sizeof(sendBuf) - 1, "%d|%s", getpid(), procName);
    if ((ret > 0) && (0 < utils_send_data_to_local_socket_with_response_data(IPC_SERVER_SOCKET_PATH, IPC_TYPE_SCREENSHOT, sendBuf, ret, &buf))) {
        res = (strtol(buf, NULL, 10) == 0);
    }
    C_FREE_FUNC_NULL(buf, free);

    syslog(LOG_INFO, "[screenshot] %d", res);

    return res;
}
