//
// Created by dingjing on 2/18/25.
//

#ifndef andsec_scanner_TASK_BASE_H
#define andsec_scanner_TASK_BASE_H
#include <QMutex>
#include <QRunnable>

#include "policy-base.h"

class ScanTaskManager;
class ScanTaskManagerPrivate;

enum class TaskType
{
    ScanTaskType,
    DlpTaskType
};

enum class ScanTaskStatus
{
    UnStarted,
    Running,
    Stopped,
    Finished,
    Paused,
    Error,
};

enum class TaskScanMode
{
    TaskScanNoDisturbMode,
    TaskScanNormalMode,
    TaskScanFastMode
};

enum class ScanStatus
{
    SCAN_TASK_NO_LAUNCH = 0,                    // 初始状态
    SCAN_TASK_SCANNING,                         // 扫描中
    SCAN_TASK_STOP,                             // 停止
    SCAN_TASK_OVER,                             // 扫描完成
    SCAN_TASK_ERROR,                            // 异常
    SCAN_TASK_SUSPEND,                          // 暂停状态
};

class TaskBase : public QRunnable
{
public:
    explicit TaskBase(TaskType tp, const QString& taskId, const QString& taskName);
    ~TaskBase() override;

    void run() override;

    QString getTaskId() const;
    QString getTaskName() const;

    bool getUseOCR() const;
    void setUseOCR(int useOCR);

    void setParseOK();
    void setParseFail();
    bool getIsParseOK() const;

private:
    QString                 mTaskId;
    QString                 mTaskName;
    TaskType                mTaskType;
    bool                    mIsUseOCR;
    bool                    mParseOK = false;
};

class ScanTask final : public TaskBase
{
public:
    explicit ScanTask(const QString& taskId, const QString& taskName);

    qint64 getScannedFileNum () const;
    void setScannedFileNum (qint64 fileNum);

    qint64 getTotalFileNum () const;
    void setTotalFileNum (qint64 fileNum);

    int getTimes() const;
    int getProgressRate() const;
    int getAttachmentReport() const;
    TaskScanMode getTaskScanMode() const;
    ScanTaskStatus getTaskStatus() const;
    int getTaskStatusInt() const;
    int getTaskScanModeInt() const;
    bool getIsScheduled() const;
    void parseRules(const QJsonArray& arr);
    QString getTaskScanPathStr() const;
    QString getPolicyIdListStr() const;
    QString getFileTypeListStr() const;
    QString getBypassFileTypeStr() const;
    QString getTaskBypassPathStr() const;
    const QSet<QString>& getTaskScanPath() const;
    const QSet<QString>& getPolicyIdList() const;
    const QSet<QString>& getFileTypeList() const;
    const QSet<QString>& getBypassFileType() const;
    const QSet<QString>& getTaskBypassPath() const;

    bool checkNeedRun() const;
    void taskForceReload() const;

    void errorRun();                            // 出错
    void initRun() const;                       // 初始运行
    void progress() const;                      // 记录进度
    void stopRun();                             // 停止运行
    void pauseRun();                            // 暂停任务
    void startRun();                            // 开始运行
    void finishedRun();                         // 扫描完成

    bool getIsForceRestart() const;
    void setIsForceRestart(bool forceRestart);

    static qint64 getScannedFileNum(const QString& taskId);
    static qint64 getTotalFileNum(const QString& taskId);
    static int getTimes(const QString& taskId);
    static bool getIsScheduled(const QString& taskId);

    void run() override;

// private:
    void setTimes(int times);
    void setProgressRate(int rate);
    void setTaskScanMode (int mode);
    void setAttachmentReport(int size);
    void setIsScheduled(bool isScheduled);
    void setTaskScanMode (TaskScanMode mode);
    void setTaskStatus(ScanTaskStatus taskStatus);
    void setTaskStatus(const QString& taskStatus);
    void setTaskScanPath(const QString& taskScanPath);
    void setPolicyIdList(const QString& policyIdList);
    void setFileTypeList(const QString& fileTypeList);
    void setBypassFileType(const QString& bypassFileType);
    void setTaskBypassPath(const QString& taskBypassPath);

private:
    void scanFiles();
    void pop100File(QStringList& fileMap);
    void update100FileStatus(QStringList& fileMap);
    QPair<QString, QString> getScanFileResult (const QString& filePath);
    void fileScanFinished(const QString& path, const QString& md5, bool isHit, const QList<QString>& ctx);

    // 使用此任务中的扫描规则进行文件扫描
    void scanFile(const QString& filePath);

private:
    bool                                            mForceRestart = false;
    bool                                            mFinishedOneTime = false;
    int                                             mTimes;
    bool                                            mIsScheduled;
    qint64                                          mTotalNum;
    qint64                                          mFinishedNum;
    std::atomic_bool                                mIsRunning;
    int                                             mAttachmentReport;      // 附件上报 MB
    int                                             mProgressRate;
    ScanTaskStatus                                  mTaskStatus;
    TaskScanMode                                    mTaskScanMode;
    QSet<QString>                                   mPolicyIdList;
    QSet<QString>                                   mTaskScanPath;
    QSet<QString>                                   mTaskBypassPath;
    QSet<QString>                                   mTaskScanFileTypes;
    QSet<QString>                                   mTaskBypassFileTypes;
    QMap<QString, std::shared_ptr<PolicyGroup>>     mPolicies;
    QMap<int, std::shared_ptr<PolicyGroup>>         mPoliciesIdx;
    QList<int>                                      mPoliciesOrderIdx;

    // 扫描任务相关
    QSet<QString>                                   mFilesForScan;
    QSet<QString>                                   mFilesScanned;
};




#endif // andsec_scanner_TASK_BASE_H
