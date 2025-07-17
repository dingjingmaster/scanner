//
// Created by dingjing on 25-7-7.
//

#ifndef andsec_scanner_GEN_EVENT_H
#define andsec_scanner_GEN_EVENT_H
#include <QObject>

#include "policy-base.h"


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

    /*
      {
          "logType":2,
          "scanId":"197e27cc1b81kdjkk3hn",
          "time":"2025-07-07 09:25:32",
          "times":"1",
          "isScheduled":0,
          "execTimes":1,
          "fileName":"C:\\Users\\Administrator\\Documents\\xwechat_files\\WMPF\\emotion_search\\26\\main",
          "attachmentZip":"",
          "hash":"dc6d1538e2e29fc272383409884e496c",
          "fileType":"application/zip",
          "recognized":"0",
          "deviceId":"f1983414d6a1c4f978b940ed22a505131bee57be",
          "deviceName":"DESKTOP-HDAJGGP",
          "ip":"192.168.85.191",
          "mac":"00-0C-29-61-F6-4E",
          "userId":"197e245726fe2f205rcj",
          "fileSize":"294934",
          "gradeIds":[],
          "policyInfo":[
              {
                  "policyId":"197e245e6b6atb3iukn3",
                  "trip":1,
                  "level":1,
                  "action":4,
                  "context":[
                      {
                          "info":""
                      }
                  ],
                  "count":112,
                  "extActionInfo":[ { }, { }, { } ]
               }
          ],
          "fileInfo": [
                {
                    "xxxx": "xxx",
                    "名称" : "",
                    "大小": "xxxKB",
                    "项目类型" : "文件",
                    "修改日期" : "",
                    "创建日期" : "",
                    "访问日期" : "",
                    "属性": "",
                    "计算机": "",
                }
          ]
      }
     */
    void genScanContentEvent(const QString& taskId, const std::shared_ptr<PolicyGroup>& pg, const QString& fileName, bool isScheduled) const;
private:
    QByteArray sendServerTypeAndWaitResp (int serverIpcType, const QByteArray& data, bool waitResp) const;
    QByteArray sendDataAndWaitResp (const QString& sockPath, const QByteArray& data, bool waitResp) const;

    QString getScanStatusString(ScanStatus scanStatus) const;
    explicit GenEvent(QObject* parent = nullptr);
    static GenEvent         gInstance;
};



#endif // andsec_scanner_GEN_EVENT_H
