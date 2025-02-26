//
// Created by dingjing on 2/18/25.
//

#include "task-base.h"

#include <QTimer>
#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <unistd.h>


TaskBase::TaskBase(TaskType tp, const QString & taskId, const QString & taskName)
    : mTaskId(taskId), mTaskName(taskName), mTaskType(tp)
{
}

TaskBase::~TaskBase()
{
}

void TaskBase::run()
{
}

QString TaskBase::getTaskId() const
{
    return mTaskId;
}

QString TaskBase::getTaskName() const
{
    return mTaskName;
}

void TaskBase::setUseOCR(int useOCR)
{
    mIsUseOCR = (useOCR == 1);
}

bool TaskBase::getUseOCR() const
{
    return mIsUseOCR;
}

ScanTask::ScanTask(const QString & taskId, const QString & taskName)
    : TaskBase(TaskType::ScanTaskType, taskId, taskName), mIsRunning(false)
{
    setAutoDelete(false);
}

void ScanTask::setTaskStatus(const QString & taskStatus)
{
    if (taskStatus.toLower() == "running") {
        mTaskStatus = ScanTaskStatus::Running;
    }
}

void ScanTask::setTaskStatus(ScanTaskStatus taskStatus)
{
    mTaskStatus = taskStatus;
}

ScanTaskStatus ScanTask::getTaskStatus() const
{
    return mTaskStatus;
}

int ScanTask::getTaskStatusInt() const
{
    switch (mTaskStatus) {
    default:
    case ScanTaskStatus::UnStarted:
        return 0;
    case ScanTaskStatus::Running:
        return 1;
    case ScanTaskStatus::Stopped:
        return 2;
    case ScanTaskStatus::Finished:
        return 3;
    case ScanTaskStatus::Paused:
        return 4;
    case ScanTaskStatus::Error:
        return 5;
    }

    return 0;
}

int ScanTask::getTaskScanModeInt() const
{
    switch (mTaskScanMode) {
    default:
    case TaskScanMode::TaskScanNormalMode:
        return 0;
    case TaskScanMode::TaskScanFastMode:
        return 2;
    case TaskScanMode::TaskScanNoDisturbMode:
        return 3;
    }
}

void ScanTask::setFileTypeList(const QString & fileTypeList)
{
    const auto jsonDoc = QJsonDocument::fromJson(fileTypeList.toUtf8());

    for (auto ff : jsonDoc.array()) {
        mTaskScanFileTypes << ff.toObject()["v"].toString();
    }
}

const QSet<QString> & ScanTask::getFileTypeList() const
{
    return mTaskScanFileTypes;
}

void ScanTask::setBypassFileType(const QString & bypassFileType)
{
    const auto jsonDoc = QJsonDocument::fromJson(bypassFileType.toUtf8());

    for (auto ff : jsonDoc.array()) {
        mTaskBypassFileTypes << ff.toObject()["v"].toString();
    }
}

const QSet<QString> & ScanTask::getBypassFileType() const
{
    return mTaskBypassFileTypes;
}

void ScanTask::setTaskScanPath(const QString & taskScanPath)
{
    const auto ls = taskScanPath.split("|");
    mTaskScanPath = QSet<QString>(ls.begin(), ls.end());
}

const QSet<QString> & ScanTask::getTaskScanPath() const
{
    return mTaskScanPath;
}

void ScanTask::setTaskBypassPath(const QString & taskBypassPath)
{
    const auto ls = taskBypassPath.split("|");
    mTaskBypassPath = QSet<QString>(ls.begin(), ls.end());
}

const QSet<QString> & ScanTask::getTaskBypassPath() const
{
    return mTaskBypassPath;
}

void ScanTask::scanFiles()
{
    // 检查是否存在，存在则读取，否则扫描
    qInfo() << "ScanTask::scanFiles";

    // 保存解析到的文件信息 sqlite3

    // 解析文件内容 并 扫描
}

void ScanTask::stop()
{
    mTaskStatus = ScanTaskStatus::Stop;
    QEventLoop loop;
    QTimer timer;
    loop.connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (mTaskStatus == ScanTaskStatus::Stopped) {
            loop.exit(0);
        }
    });
    timer.setInterval(1000);
    timer.start();
    loop.exec();
}

void ScanTask::run()
{
    qInfo() << "Task: " << getTaskId() << " name: " << getTaskName() << " RUNNING!";
    mTaskStatus = ScanTaskStatus::Running;
    scanFiles();

    while (true) {
        usleep(1000 * 1000 * 5);
        if (mTaskStatus == ScanTaskStatus::Stop) {
            mTaskStatus = ScanTaskStatus::Stopped;
            qInfo() << "Task: " << getTaskId() << " name: " << getTaskName() << " STOPPED!";
            break;
        }
        else if (mTaskStatus == ScanTaskStatus::Running) {
            // 取xxx文件， 开始扫描
            // 获取要扫描的文件 并 开始扫描
        }
    }
}

void ScanTask::setPolicyIdList(const QString& policyIdList)
{
    const auto ls = policyIdList.split(",");
    mPolicyIdList = QSet<QString>(ls.begin(), ls.end());
}

const QSet<QString> & ScanTask::getPolicyIdList() const
{
    return mPolicyIdList;
}

void ScanTask::setProgressRate(int rate)
{
    mProgressRate = rate;
}

int ScanTask::getProgressRate() const
{
    return mProgressRate;
}

void ScanTask::setTaskScanMode(int mode)
{
    switch (mode) {
        default:
        case 1: {
            mTaskScanMode = TaskScanMode::TaskScanNoDisturbMode;
            break;
        }
        case 2: {
            mTaskScanMode = TaskScanMode::TaskScanNormalMode;
        }
        case 3: {
            mTaskScanMode = TaskScanMode::TaskScanFastMode;
        }
    }
}

void ScanTask::setTaskScanMode(TaskScanMode mode)
{
    mTaskScanMode = mode;
}

TaskScanMode ScanTask::getTaskScanMode() const
{
    return mTaskScanMode;
}

void ScanTask::setAttachmentReport(int size)
{
    mAttachmentReport = size;
}

int ScanTask::getAttachmentReport() const
{
    return mAttachmentReport;
}

void ScanTask::parseRules(const QJsonArray & arr)
{
    for (auto f : arr) {
        const QString id = f["id"].toString();
        const QString name = f["name"].toString();
        const int matchCount = f["matchDetectionRuleCount"].toInt();
        const int exceptionCount = f["matchExceptionRuleCount"].toInt();
        const int orderNum = f["order_num"].toInt();
        const int riskLevel = f["risk_level"].toInt();
        // qInfo() << "id: " << id << " name: " << name;
        if (!id.isEmpty() && !name.isEmpty()) {
            const auto val = std::make_shared<PolicyGroup>(id, name);

            val->setRiskLevel(riskLevel);
            val->setRuleHitCount(matchCount);
            val->setRuleExceptCount(exceptionCount);

            mPolicies[id] = val;
            mPoliciesIdx[orderNum] = val;
            mPoliciesOrderIdx << orderNum;
            std::sort(mPoliciesOrderIdx.begin(), mPoliciesOrderIdx.end());

            const auto ruleList = f["ruleList"];
            if (ruleList.isArray()) {
                val->parseRule(ruleList.toArray());
            }
        }
    }
}



