//
// Created by dingjing on 2/18/25.
//

#include "task-manager.h"

#include <QMap>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QThreadPool>
#include <QJsonDocument>
#include <QWaitCondition>

#include <pthread.h>

#include "data-base.h"
#include "gen-event.h"
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
    static QString parseTaskId(const QString& scanTask);
    std::shared_ptr<ScanTask> getScanTask(const QString& scanTaskId);
    void removeScanTask(const QString& scanTaskId);

private:
    TaskManager*                                q_ptr = nullptr;
    QMap<QString, std::shared_ptr<ScanTask>>    mScanTasks;
    QMap<QString, std::shared_ptr<TaskBase>>    mDlpTasks;
    QThreadPool                                 mScanTaskThreadPool;
};

TaskManagerPrivate::TaskManagerPrivate(TaskManager * q)
    : q_ptr(q)
{
    mScanTaskThreadPool.setMaxThreadCount(100);
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
                    scanTaskPtr->setTimes(times);
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
                    DataBase::getInstance().insertTask(taskId, taskName,
                        scanTaskPtr->getTaskScanPathStr(), scanTaskPtr->getTaskBypassPathStr(),
                        scanTaskPtr->getFileTypeListStr(), scanTaskPtr->getBypassFileTypeStr(),
                        schedulingCron, policyIdList.split(","),
                        scanTaskPtr->getTaskStatusInt(), scanTaskPtr->getTaskScanModeInt(),
                        times, schedulingMechanism.toLower() == "schedule");
                    scanTaskPtr->setParseOK();
                    return true;
                }
            }
        }
    }

    return false;
}

QString TaskManagerPrivate::parseTaskId(const QString & scanTask)
{
    C_RETURN_VAL_IF_FAIL(QFile::exists(scanTask), "");

    QFile file(scanTask);
    if (file.open(QIODevice::ReadOnly)) {
        const auto json = QJsonDocument::fromJson(file.readAll());
        file.close();

        const auto dataTask = json["dataDiscoveryTask"];
        if (!dataTask.isNull()) {
            const auto dlpContentDataList = json["dlpContentDataList"];
            if (!dlpContentDataList.isNull()) {
                const auto taskId = dataTask["taskId"].toString();

                if (!taskId.isEmpty()) {
                    return taskId;
                }
            }
        }
    }

    return "";
}

std::shared_ptr<ScanTask> TaskManagerPrivate::getScanTask(const QString & scanTaskId)
{
    if (mScanTasks.contains(scanTaskId)) {
        return mScanTasks[scanTaskId];
    }
    return nullptr;
}

void TaskManagerPrivate::removeScanTask(const QString & scanTaskId)
{
    mScanTasks.remove(scanTaskId);

    GenEvent::getInstance().genScanProgress(scanTaskId, GenEvent::SCAN_TASK_STOP,
            ScanTask::getScannedFileNum(scanTaskId), ScanTask::getTotalFileNum(scanTaskId),
            ScanTask::getTimes(scanTaskId), ScanTask::getIsScheduled(scanTaskId),
            DataBase::getInstance().getExecTimes(scanTaskId));

    DataBase::getInstance().deleteTask(scanTaskId);
}

TaskManager* TaskManager::getInstance()
{
    return &gInstance;
}

bool TaskManager::isValidScanTaskId(const QString& scanTaskId)
{
    Q_D(TaskManager);
    return d->mScanTasks.contains(scanTaskId);
}

std::shared_ptr<TaskBase> TaskManager::getTaskById(const QString& id)
{
    Q_D(TaskManager);
    return d->getScanTask(id);
}

bool TaskManager::parseScanTask(const QString & scanTask)
{
    Q_D(TaskManager);

    return d->parseScanTask(scanTask);
}

QString TaskManager::parseTaskId(const QString & scanTask)
{
    Q_D(TaskManager);

    return d->parseTaskId(scanTask);
}

void TaskManager::startScanTask(const QString& scanTaskId)
{
    Q_D(TaskManager);

    const auto scanTask = d->getScanTask(scanTaskId);

    startScanTask(scanTask);
}

void TaskManager::startScanTask(std::shared_ptr<TaskBase> task)
{
    Q_D(TaskManager);

    if (task.get()) {
        qInfo() << "Start scan task:" << task->getTaskId();
        d->mScanTaskThreadPool.start(task.get());
    }
}

QString TaskManager::getTaskIdByPolicyFile(const QString & policyFile)
{
    QString taskId = policyFile.split("/").last();

    return taskId.replace("scan-task-", "");
}

void TaskManager::checkAllTask()
{
    Q_D(TaskManager);

    for (auto it = d->mScanTasks.keyValueBegin(); it != d->mScanTasks.keyValueEnd(); ++it) {
        qInfo() << "Schedule Task: " << it->first;
        if (it->second->checkNeedRun()) {
            startScanTask(it->second);
        }
        else {
            qInfo() << "Task: " << it->first << " Not Need Run!";
        }
    }
}

void TaskManager::startRunTaskAll()
{
    Q_D(TaskManager);

    for (auto it = d->mScanTasks.keyValueBegin(); it != d->mScanTasks.keyValueEnd(); ++it) {
        qInfo() << "Schedule Task: " << it->first;
        if (it->second->checkNeedRun()) {
            startScanTask(it->second);
        }
        else {
            qInfo() << "Task: " << it->first << " Not Need Run!";
        }
    }
}

void TaskManager::stopScanTask(const QString & scanTaskId)
{
    Q_D(TaskManager);

    const auto scanTask = d->getScanTask(scanTaskId);
    if (scanTask) {
        scanTask->stopRun();
    }
}

void TaskManager::pauseScanTask(const QString& scanTaskId)
{
    Q_D(TaskManager);

    const auto scanTask = d->getScanTask(scanTaskId);
    if (scanTask) {
        scanTask->pauseRun();
    }
}

void TaskManager::removeScanTask(const QString & scanTaskId)
{
    Q_D(TaskManager);

    d->removeScanTask(scanTaskId);
}

TaskManager::TaskManager()
    : d_ptr(new TaskManagerPrivate(this))
{
}

