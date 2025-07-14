//
// Created by dingjing on 25-7-7.
//

#ifndef andsec_scanner_IPC_H
#define andsec_scanner_IPC_H
#include <QMap>
#include <QObject>
#include <gio/gio.h>

#include "../common/ipc.h"

class Ipc;

typedef void(*IpcClientProcess)(Ipc*, const QByteArray& data, GSocket& clientSock);

class Ipc final : public QObject
{
    Q_OBJECT
    friend void process_client_request (gpointer data, gpointer udata);
    friend gboolean new_request (GSocketService* ls, GSocketConnection* conn, GObject* srcObj, gpointer uData);
public:
    static Ipc& getInstance();

private:
    void initConnection();
    IpcClientProcess getClientProcess(IpcServerType type);
    explicit Ipc(QObject* parent = nullptr);

private:
    GSocketService*                         mServer = nullptr;
    GSocketAddress*                         mServerAddr = nullptr;
    GThreadPool*                            mServerWorker = nullptr;
    GSocket*                                mServerSocket = nullptr;
    QMap<IpcServerType, IpcClientProcess>   mClientProcessor;
    static Ipc                              gInstance;
};



#endif // andsec_scanner_IPC_H
