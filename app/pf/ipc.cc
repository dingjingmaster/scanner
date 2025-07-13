//
// Created by dingjing on 25-7-7.
//

#include "ipc.h"

#include <sys/un.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <QDebug>

#include "proc-inject.h"
#include "task-manager.h"
#include "utils.h"
#include "macros/macros.h"


#define IPC_SCAN_TASK_SOCKET_PATH "/usr/local/andsec/start/andsec-scan-task.sock"


static gsize read_all_data(GSocket* fr, QByteArray& out/*out*/);
void process_client_request (gpointer data, gpointer udata);
gboolean new_request (GSocketService* ls, GSocketConnection* conn, GObject* srcObj, gpointer uData);


static void process_client_scan_task_stop_task      (Ipc* ipc, const QByteArray& data, GSocket& clientSock);
static void process_client_scan_task_pause_task     (Ipc* ipc, const QByteArray& data, GSocket& clientSock);
static void process_client_scan_task_start_task     (Ipc* ipc, const QByteArray& data, GSocket& clientSock);

static void process_client_inject_lib_by_pid        (Ipc* ipc, const QByteArray& data, GSocket& clientSock);
static void process_client_inject_lib_by_proc_name  (Ipc* ipc, const QByteArray& data, GSocket& clientSock);


Ipc Ipc::gInstance;

Ipc& Ipc::getInstance()
{
    return gInstance;
}

void Ipc::initConnection()
{
    GError* error = nullptr;

    do {
        const QString socketPath(IPC_SCAN_TASK_SOCKET_PATH);
        const GSocketFamily family = G_SOCKET_FAMILY_UNIX;

        struct sockaddr_un addrT = {0};
        memset (&addrT, 0, sizeof (addrT));
        addrT.sun_family = AF_LOCAL;
        strncpy (addrT.sun_path, socketPath.toUtf8().constData(), sizeof(addrT.sun_path) - 1);
        mServerSocket = g_socket_new (family, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &error);
        if (error) {
            qWarning() << "g_socket_new error: " << error->message;
            break;
        }

        g_socket_set_blocking (mServerSocket, true);
        if (0 == access(socketPath.toUtf8().data(), R_OK | W_OK)) {
            remove (socketPath.toUtf8().data());
        }

        mServerAddr = g_socket_address_new_from_native(&addrT, sizeof (addrT));

        if (!g_socket_bind (mServerSocket, mServerAddr, false, &error)) {
            qWarning() << "bind error: " << error->message;
            break;
        }

        if (!g_socket_listen (mServerSocket, &error)) {
            qWarning() << "listen error: " << error->message;
            break;
        }

        mServer = g_socket_service_new();
        g_socket_listener_add_socket (G_SOCKET_LISTENER(mServer), mServerSocket, nullptr, &error);
        if (error) {
            qWarning() << "g_socket_listener_add_socket error: " << error->message;
            break;
        }

        mServerWorker = g_thread_pool_new (process_client_request, this, 100, false, &error);
        if (error) {
            qWarning() << "g_thread_pool_new error: " << error->message;
            break;
        }

        chmod (socketPath.toUtf8().data(), 0777);
        g_signal_connect (G_SOCKET_LISTENER(mServer), "incoming", reinterpret_cast<GCallback>(new_request), this);
    } while (false);

    if (error) { g_error_free(error); }
}

IpcClientProcess Ipc::getClientProcess(IpcServerType type)
{
    if (mClientProcessor.contains(type)) {
        return mClientProcessor[type];
    }

    return nullptr;
}

Ipc::Ipc(QObject* parent)
    : QObject(parent)
{
    initConnection();

    mClientProcessor[IPC_TYPE_SERVER_STOP_TASK]                 = process_client_scan_task_stop_task;
    mClientProcessor[IPC_TYPE_SERVER_PAUSE_TASK]                = process_client_scan_task_pause_task;
    mClientProcessor[IPC_TYPE_SERVER_START_TASK]                = process_client_scan_task_start_task;
    mClientProcessor[IPC_TYPE_INJECT_LIB_BY_PID]                = process_client_inject_lib_by_pid;
    mClientProcessor[IPC_TYPE_INJECT_LIB_BY_PROC_NAME]          = process_client_inject_lib_by_proc_name;
}

gboolean new_request (GSocketService* ls, GSocketConnection* conn, GObject* srcObj, gpointer uData)
{
    (void) ls;
    (void) srcObj;

    qDebug() << "new request!";

    g_return_val_if_fail(uData, false);

    g_object_ref(conn);

    GError* error = nullptr;
    g_thread_pool_push(static_cast<Ipc*>(uData)->mServerWorker, conn, &error);
    if (error) {
        qDebug() << error->message;
        g_object_unref(conn);
        return false;
    }

    return true;
}

void process_client_request (gpointer data, gpointer udata)
{
    g_return_if_fail(data);

    if (!udata) {
        qWarning() << "client udata null";
        g_object_unref(data);
        return;
    }

    QByteArray binStr;
    IpcMessage* msg = nullptr;
    auto* conn = static_cast<GSocketConnection*>(data);
    auto sc = static_cast<Ipc*>(udata);

    GSocket* socket = g_socket_connection_get_socket (conn);

    const guint64 strLen = read_all_data (socket, binStr);
    if (strLen <= 0) {
        qWarning() << "read client data null";
        goto out;
    }

    if (binStr.length() >= static_cast<int>(sizeof(IpcMessage))) {
        msg = reinterpret_cast<IpcMessage*>(binStr.data());
        const int dataLen = static_cast<int>(msg->dataLen);
        if (dataLen + static_cast<int>(sizeof(IpcMessage)) >= binStr.length()) {
            IpcClientProcess processor = sc->getClientProcess(static_cast<IpcServerType>(msg->type));
            if (processor) {
                QByteArray dataT = "";
                if (msg->dataLen > 0) {
                    dataT = QByteArray(msg->data, static_cast<int>(msg->dataLen));
                }
                (*processor)(sc, dataT, *socket);
            }
            else {
                qWarning() << "unknow request type: " << msg->type;
            }
        }
        else {
            qWarning() << "Invalid request data, dataLen: " << dataLen << " IpcMessage len: " << sizeof(IpcMessage) << " all len: " << binStr.length();
        }
    }
    else {
        qWarning() << "unknow request size";
    }

out:
    if (conn)       { g_object_unref (conn); }

    g_thread_pool_stop_unused_threads();

    (void) data;
    (void) udata;
}

static gsize read_all_data(GSocket* fr, QByteArray& out/*out*/)
{
    GError* error = nullptr;

    gsize readLen = 0;
    char buf[1024] = {0};

    while ((readLen = g_socket_receive(fr, buf, sizeof(buf) - 1, nullptr, &error)) > 0) {
        if (error) {
            qWarning() << error->message;
            break;
        }
        out.append(buf, readLen);
        if (readLen < sizeof (buf) - 1) {
            break;
        }
    }

    if(error) g_error_free(error);

    return out.length();
}

//
static void process_client_scan_task_stop_task (Ipc* ipc, const QByteArray& data, GSocket& clientSock)
{
    const QString taskId(data);

    TaskManager::getInstance()->stopScanTask(taskId);

    Q_UNUSED(ipc);
    Q_UNUSED(clientSock);
}

static void process_client_scan_task_pause_task (Ipc* ipc, const QByteArray& data, GSocket& clientSock)
{
    const QString taskId(data);

    TaskManager::getInstance()->pauseScanTask(taskId);

    Q_UNUSED(ipc);
    Q_UNUSED(clientSock);
}

static void process_client_scan_task_start_task (Ipc* ipc, const QByteArray& data, GSocket& clientSock)
{
    const QString taskId(data);

    TaskManager::getInstance()->startScanTask(taskId);

    Q_UNUSED(ipc);
    Q_UNUSED(clientSock);
}

static void process_client_inject_lib_by_pid (Ipc* ipc, const QByteArray& data, GSocket& clientSock)
{
    const auto arr = data.split('|');
    C_RETURN_IF_FAIL(arr.size() == 2);
    const int pid = arr.at(0).toInt();
    const QString procName = Utils::getProcNameByPid(pid);
    const bool ret = proc_inject_inject_so_by_pid(pid, arr[1]);
    qInfo() << "Inject to pid: " << arr[0] << ", procName: " << procName << ", library: " << arr[1] << ", ret: " << ret;

    Q_UNUSED(ipc);
    Q_UNUSED(clientSock);
}

static void process_client_inject_lib_by_proc_name  (Ipc* ipc, const QByteArray& data, GSocket& clientSock)
{
    const auto arr = data.split('|');
    C_RETURN_IF_FAIL(arr.size() == 2);
    const QString procName = arr.at(0);
    const bool ret = proc_inject_inject_so_by_proc_name(procName.toUtf8().constData(), arr[1]);
    qInfo() << "Inject to procName: " << procName << ", library: " << arr[1] << ", ret: " << ret;

    Q_UNUSED(ipc);
    Q_UNUSED(clientSock);
}
