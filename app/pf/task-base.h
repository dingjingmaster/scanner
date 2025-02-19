//
// Created by dingjing on 2/18/25.
//

#ifndef andsec_scanner_TASK_BASE_H
#define andsec_scanner_TASK_BASE_H
#include <QSet>
#include <QJsonArray>
#include <QStringList>

#include "policy-base.h"


enum class TaskType
{
    ScanTaskType,
    DlpTaskType
};

enum class ScanTaskStatus
{
    Running,
};

enum class TaskScanMode
{
    TaskScanNoDisturbMode,
    TaskScanNormalMode,
    TaskScanFastMode
};

class TaskBase
{
public:
    explicit TaskBase(TaskType tp, const QString& taskId, const QString& taskName);
    virtual ~TaskBase();

    void setUseOCR(int useOCR);
    bool getUseOCR() const;

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

    void setTaskStatus(const QString& taskStatus);
    void setTaskStatus(ScanTaskStatus taskStatus);
    ScanTaskStatus getTaskStatus() const;

    void setFileTypeList(const QString& fileTypeList);
    const QSet<QString>& getFileTypeList() const;

    void setBypassFileType(const QString& bypassFileType);
    const QSet<QString>& getBypassFileType() const;

    void setTaskScanPath(const QString& taskScanPath);
    const QSet<QString>& getTaskScanPath() const;

    void setTaskBypassPath(const QString& taskBypassPath);
    const QSet<QString>& getTaskBypassPath() const;

    void setPolicyIdList(const QString& policyIdList);
    const QSet<QString>& getPolicyIdList() const;

    void setProgressRate(int rate);
    int getProgressRate() const;

    void setTaskScanMode (int mode);
    void setTaskScanMode (TaskScanMode mode);
    TaskScanMode getTaskScanMode() const;

    void setAttachmentReport(int size);
    int getAttachmentReport() const;

    void parseRules(const QJsonArray& arr);

private:
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
};



#endif // andsec_scanner_TASK_BASE_H
