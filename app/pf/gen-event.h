//
// Created by dingjing on 25-7-7.
//

#ifndef andsec_scanner_GEN_EVENT_H
#define andsec_scanner_GEN_EVENT_H
#include <QObject>


class GenEvent final : public QObject
{
    Q_OBJECT
public:
    typedef enum
    {
        SCAN_TASK_NO_LAUNCH = 0,                    // 初始状态
        SCAN_TASK_SCANNING,                         // 扫描中
        SCAN_TASK_STOP,                             // 停止
        SCAN_TASK_OVER,                             // 扫描完成
        SCAN_TASK_ERROR,                            // 异常
        SCAN_TASK_SUSPEND,                          // 暂停状态
    } ScanStatus;
    static GenEvent& getInstance();

    // 走 IPC
    // {
    //      "taskId":"197e2462b601dju5dpff",
    //      "taskState":3,
    //      "userId":"197e245726fe2f205rcj",
    //      "computerName":"DESKTOP-HDAJGGP",
    //      "mac":"00-0C-29-61-F6-4E",
    //      "ip":"192.168.85.191",
    //      "fileScanedNum":"1881",
    //      "fileTotalNum":"1881",
    //      "times":"1",
    //      "isScheduled":0,
    //      "execTimes":1,
    //      "lastUpdatedStamp":"2025-07-07 08:26:27"
    // }
    void genScanProgress(const QString& taskId, ScanStatus scanStatus, qint64 scannedNum, qint64 totalNum, qint32 times, bool isScheduled, qint32 execTimes) const;

private:
    QByteArray sendServerTypeAndWaitResp (int serverIpcType, const QByteArray& data, bool waitResp) const;
    QByteArray sendDataAndWaitResp (const QString& sockPath, const QByteArray& data, bool waitResp) const;

    QString getScanStatusString(ScanStatus scanStatus) const;
    explicit GenEvent(QObject* parent = nullptr);
    static GenEvent         gInstance;
};



#endif // andsec_scanner_GEN_EVENT_H
