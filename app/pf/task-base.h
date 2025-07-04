//
// Created by dingjing on 2/18/25.
//

#ifndef andsec_scanner_TASK_BASE_H
#define andsec_scanner_TASK_BASE_H
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
    Stop,
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

private:
    QString                 mTaskId;
    QString                 mTaskName;
    TaskType                mTaskType;
    bool                    mIsUseOCR;
};

class ScanTask final : public TaskBase
{
public:
    explicit ScanTask(const QString& taskId, const QString& taskName);

    int getProgressRate() const;
    int getAttachmentReport() const;
    TaskScanMode getTaskScanMode() const;
    ScanTaskStatus getTaskStatus() const;
    int getTaskStatusInt() const;
    int getTaskScanModeInt() const;
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

    void stop();
    void run() override;

// private:
    void setProgressRate(int rate);
    void setTaskScanMode (int mode);
    void setAttachmentReport(int size);
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
    void taskFinished();
    // QString getTaskTmpMd5(const QString& filePath) const;
    void pop100File(QStringList& fileMap) const;
    void update100FileStatus(QStringList& fileMap);
    QPair<QString, QString> getScanFileResult (const QString& filePath);
    void fileScanFinished(const QString& path, const QString& md5, bool isHit, const QList<QString>& ctx);

    // 使用此任务中的扫描规则进行文件扫描
    void scanFile(const QString& filePath);

private:
    bool                                            mFinishedOneTime = false;
    int                                             mTotalNum;
    int                                             mFinishedNum;
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
