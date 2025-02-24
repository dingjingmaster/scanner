//
// Created by dingjing on 2/18/25.
//

#include "task-manager.h"

#include <QMap>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QJsonDocument>
#include <QWaitCondition>

#include <pthread.h>

#include "task-base.h"
#include "macros/macros.h"

typedef struct _ThreadPool      ThreadPool;
typedef struct _ThreadWorker    ThreadWorker;

typedef void* (*ThreadWorkerFunc)(void* args);

struct _ThreadPool
{
    bool                shutdown;
    int                 workerNum;
    pthread_t*          threadID;
    ThreadWorkerFunc*   threadWorkerList;
    QMutex              lock;
    QWaitCondition      cond;
};

TaskManager TaskManager::gInstance;

class TaskManagerPrivate
{
    Q_DECLARE_PUBLIC(TaskManager)
public:
    explicit TaskManagerPrivate(TaskManager* q);

    bool parseScanTask(const QString& scanTask);

private:
    TaskManager*                                q_ptr = nullptr;
    QMap<QString, std::shared_ptr<ScanTask>>    mScanTasks;
    QMap<QString, std::shared_ptr<TaskBase>>    mDlpTasks;
};

TaskManagerPrivate::TaskManagerPrivate(TaskManager * q)
    : q_ptr(q)
{
}

bool TaskManagerPrivate::parseScanTask(const QString & scanTask)
{
    C_RETURN_VAL_IF_FAIL(QFile::exists(scanTask), false);

    QFile file(scanTask);
    if (file.open(QIODevice::ReadOnly)) {
        const auto json = QJsonDocument::fromJson(file.readAll());
        file.close();

        const auto dataTask = json["dataDiscoveryTask"];
        if (!dataTask.isNull()) {
            const auto dlpContentDataList = json["dlpContentDataList"];
            if (!dlpContentDataList.isNull()) {
                const auto taskId = dataTask["taskId"].toString();
                const auto taskName = dataTask["taskName"].toString();
                const auto taskStatus = dataTask["taskStatus"].toString();
                const auto taskOCRFlag = dataTask["ocrFlag"].toInt();
                const auto fileTypeList = dataTask["fileTypeList"].toString();
                const auto bypassFileTypeList = dataTask["bypassFileTypeList"].toString();
                const auto bypassScanPath = dataTask["bypassScanPath"].toString();
                const auto execAction = dataTask["execAction"].toString();
                const auto policyIdList = dataTask["policyIdList"].toString();
                const auto progressRate = dataTask["progressRate"].toInt();
                const auto times = dataTask["times"].toInt();
                const auto terminalQuery = dataTask["terminalQuery"].toInt();
                const auto schedulingMechanism = dataTask["schedulingMechanism"].toString();
                const auto schedulingCron = dataTask["schedulingCron"].toString();
                const auto scanPath = dataTask["scanPath"].toString();
                const auto scanPathType = dataTask["scanPathType"].toInt();
                const auto scanMode = dataTask["scanMode"].toInt();
                const auto scanIntervalTimer = dataTask["scanIntervalTimer"].toInt();
                const auto reuseFlag = dataTask["reuseFlag"].toInt();
                const auto attachmentReportSize = dataTask["attachmentReportSize"].toInt();
                const auto attachmentReportFlag = dataTask["attachmentReportFlag"].toInt();

                if (!taskId.isEmpty() && !taskName.isEmpty()) {
                    const auto scanTaskPtr = std::make_shared<ScanTask>(taskId, taskName);
                    scanTaskPtr->setTaskStatus(taskStatus);
                    scanTaskPtr->setUseOCR(taskOCRFlag);
                    scanTaskPtr->setFileTypeList(fileTypeList);
                    scanTaskPtr->setBypassFileType(bypassFileTypeList);
                    scanTaskPtr->setTaskScanPath(scanPath);
                    scanTaskPtr->setTaskBypassPath(bypassScanPath);
                    scanTaskPtr->setPolicyIdList(policyIdList);
                    scanTaskPtr->setProgressRate(progressRate);
                    scanTaskPtr->setTaskScanMode(scanMode);
                    scanTaskPtr->setAttachmentReport(attachmentReportFlag ? attachmentReportSize : -1);
                    scanTaskPtr->parseRules(dlpContentDataList.toArray());
                    mScanTasks[taskId] = scanTaskPtr;
                }
            }
        }
    }

    return true;
}

TaskManager* TaskManager::getInstance()
{
    return &gInstance;
}

bool TaskManager::parseScanTask(const QString & scanTask)
{
    Q_D(TaskManager);

    return d->parseScanTask(scanTask);
}

void TaskManager::startScanTask()
{

}

TaskManager::TaskManager()
    : d_ptr(new TaskManagerPrivate(this))
{
}

