//
// Created by dingjing on 25-6-9.
//

#ifndef client_UTILS_H
#define client_UTILS_H
#include "macros/macros.h"

C_BEGIN_EXTERN_C

bool        utils_check_sec_gui_exists                      ();
bool        utils_check_sec_daemon_exists                   ();
int         utils_get_pid_by_name                           (const char* name);
bool        utils_check_proc_instance_exists                (const char* lockFile);
bool        utils_check_proc_library_exists                 (int pid, const char* libraryPath);
bool        utils_check_proc_environ_exists_key_value       (int pid, const char* key, const char* value);
void        utils_get_proc_by_pid_array                     (int pid, char fileBuf[], size_t fileBufSize);
void        utils_send_data_to_local_socket                 (const char* localSocket, const char* data, size_t dataSize);
uint64_t    utils_send_data_to_local_socket_with_response   (const char* localSocket, const char* data, size_t dataSize, char** response/* OUT */);

C_END_EXTERN_C

#endif // client_UTILS_H
