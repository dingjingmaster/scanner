//
// Created by dingjing on 25-7-14.
//

#ifndef andsec_scanner_HOOK_COMMON_H
#define andsec_scanner_HOOK_COMMON_H
#include "macros/macros.h"

#include <syslog.h>


#define LOGI

#ifdef LOGI
#define logi(...) syslog(LOG_ERR, __VA_ARGS__)
#else
#define logi(...)
#endif

C_BEGIN_EXTERN_C

// 截屏管控
bool hc_check_screenshot_control_forbidden          ();

// 网络管控, 是否允许联网
bool hc_check_process_network_control_forbidden     (const char* proc, int pid, const char* ip, int port);

C_END_EXTERN_C

#endif // andsec_scanner_HOOK_COMMON_H
