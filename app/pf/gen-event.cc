//
// Created by dingjing on 25-7-7.
//

#include "gen-event.h"

#include <QDebug>

#include <sys/un.h>
#include <gio/gio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#ifndef IPC_SERVER_SOCKET_PATH
#define IPC_SERVER_SOCKET_PATH "/usr/local/andsec/start/sec_daemon.sock"
#endif


GenEvent GenEvent::gInstance;

static gsize read_all_data(GSocket* fr, QByteArray& out/*out*/);


GenEvent& GenEvent::getInstance()
{
    return gInstance;
}

void GenEvent::genScanProgress(const QString& taskId, ScanStatus scanStatus, qint64 scannedNum, qint64 totalNum, qint32 times, bool isScheduled, qint32 execTimes) const
{
#define SCAN_PROGRESS_TYPE      230
    const QString ctx = QString("%1|%2|%3|%4|%5|%6|%7")
                .arg(taskId)
                .arg(getScanStatusString(scanStatus))
                .arg(scannedNum)
                .arg(totalNum)
                .arg(times)
                .arg(isScheduled ? 1 : 0)
                .arg(execTimes);

    auto resp = sendServerTypeAndWaitResp(SCAN_PROGRESS_TYPE, ctx.toUtf8(), false);

#undef SCAN_PROGRESS_TYPE
}

QByteArray GenEvent::sendServerTypeAndWaitResp(int serverIpcType, const QByteArray& data, bool waitResp) const
{
    typedef struct __attribute__((packed)) _IpcMessage
    {
        unsigned int        type;    // 处理类型：IpcServerType、IpcClientType
        unsigned long       dataLen;
        char                data[];
    } IpcMessage;


    IpcMessage dataBuffer;
    dataBuffer.type = serverIpcType;
    dataBuffer.dataLen = data.size();

    QByteArray buffer;
    buffer.append(reinterpret_cast<char*>(&dataBuffer), sizeof(IpcMessage));
    buffer.append(data);

    return sendDataAndWaitResp(IPC_SERVER_SOCKET_PATH, buffer, waitResp);
}

QByteArray GenEvent::sendDataAndWaitResp(const QString& sockPath, const QByteArray& data, bool waitResp) const
{
    QByteArray resp;
    GError* error = nullptr;

    do {
        struct sockaddr_un addrT = {0};
        memset (&addrT, 0, sizeof (addrT));
        addrT.sun_family = AF_LOCAL;
        strncpy (addrT.sun_path, sockPath.toUtf8().constData(), sizeof(addrT.sun_path) - 1);
        GSocketAddress* addr = g_socket_address_new_from_native(&addrT, sizeof (addrT));
        if (addr) {
            GSocket* sock = g_socket_new (G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &error);
            if (sock) {
                if (g_socket_connect(sock, addr, nullptr, &error)) {
                    if (g_socket_condition_wait(sock, G_IO_OUT, nullptr, &error)) {
                        if (g_socket_send_with_blocking(sock, data.data(), data.size(), true, nullptr, &error)) {
                            qDebug() << "send data to client OK!";
                            if (waitResp) {
                                if (g_socket_condition_wait(sock, G_IO_IN, nullptr, &error)) {
                                    read_all_data(sock, resp);
                                }
                            }
                        }
                        else {
                            qWarning() << "sendDataToClient error: " << error->message;
                        }
                    }
                    else {
                        qWarning() << "g_socket_condition_wait error: " << error->message;
                    }
                }
                else {
                    qWarning() << "g_socket_connect error: " << error->message;
                }
                g_object_unref(sock);
            }
            else {
                qWarning() << "g_socket_new error: " << error->message;
            }
            g_object_unref(addr);
        }
    } while (false);

    if (error) { g_error_free(error); }

    return resp;
}

QString GenEvent::getScanStatusString(ScanStatus scanStatus) const
{
    switch (scanStatus) {
        default: { break; }
        case SCAN_TASK_NO_LAUNCH: {
            return "0";
        }
        case SCAN_TASK_SCANNING: {
            return "1";
        }
        case SCAN_TASK_STOP: {
            return "2";
        }
        case SCAN_TASK_OVER: {
            return "3";
        }
        case SCAN_TASK_ERROR: {
            return "4";
        }
        case SCAN_TASK_SUSPEND: {
            return "5";
        }
    }

    return "0";
}

GenEvent::GenEvent(QObject* parent)
    : QObject(parent)
{
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

        out.append(buf, static_cast<int>(readLen));
        if (readLen < sizeof (buf) - 1) {
            break;
        }
    }

    if(error) g_error_free(error);

    return out.length();
}