//
// Created by dingjing on 2/18/25.
//

#include "task-base.h"

#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QRegExp>
#include <QFileInfo>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <unistd.h>
#include <macros/macros.h>

#include "utils.h"

#define TASK_SCAN_LOG_INFO       qInfo() << "[TaskId: " << getTaskId() << " TaskName: " << getTaskName() << "] "


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
    // @TODO:// 暂时把所有要扫描的文件路径保存到内存，后续保存到文件中，避免占用太多内存
    TASK_SCAN_LOG_INFO << "Start scan files";

    auto getDirFiles = [=] (const QString & dir) ->QSet<QString> {
        QSet<QString> files;
        QStringList dirs;
        dirs << dir;
        for (int i = 0; i < dirs.size(); ++i) {
            QFileInfo fi(dirs.at(i));
            if (!fi.exists()) {continue;}
            if (fi.isDir()) {
                QDir ddir(dirs.at(i));
                auto ds = ddir.entryList();
                for (auto& j : ds) {
                    if ("." == j || ".." == j) { continue; }
                    QString p = QString("%1/%2").arg(fi.absoluteFilePath(), j);
                    QFileInfo fii(p);
                    if (fii.isDir()) { dirs.append(p); } else { files << Utils::formatPath(p); }
                }
            }
            else {
                files << Utils::formatPath(fi.absoluteFilePath());
            }
        }
        return files;
    };

    // 检查是否存在，存在则读取，否则扫描
    for (const auto& d : mTaskScanPath) {
        if (d.startsWith("/bin/")
            || d.startsWith("/boot/")
            || d.startsWith("/dev/")
            || d.startsWith("/efi/")
            || d.startsWith("/etc/")
            || d.startsWith("/lib/")
            || d.startsWith("/lib64/")
            || d.startsWith("/opt/")
            || d.startsWith("/proc/")
            || d.startsWith("/run/")
            || d.startsWith("/sbin/")
            || d.startsWith("/snap/")
            || d.startsWith("/srv/")
            || d.startsWith("/usr/")
            || d.startsWith("/tmp/")
            || d.startsWith("/var/")) {
            TASK_SCAN_LOG_INFO << "跳过特殊路径: " << d;
            continue;
        }
        TASK_SCAN_LOG_INFO << "遍历扫描路径: " << d;
        mFilesForScan += getDirFiles(d);
    }

    QRegExp tpReg;
    QRegExp tpExpReg;
    QRegExp dirExpReg;

    // 去掉非扫描目录
    QStringList lwDir;
    for (const auto& d : mTaskBypassPath) {
        lwDir << Utils::formatPath(d);
    }
    if (!lwDir.empty()) {
        dirExpReg.setPattern(QString("(%1)").arg(lwDir.join("|")));
    }
    lwDir.clear();

    // 去掉非关联扩展名
    if (!mTaskBypassFileTypes.isEmpty()) {
        tpExpReg.setPattern(QString("\\.(%1)$").arg(QStringList(mTaskBypassFileTypes.begin(), mTaskBypassFileTypes.end()).join("|")));
    }

    // 保留关联扩展名
    if (!mTaskScanFileTypes.isEmpty()) {
        tpReg.setPattern(QString("\\.(%1)$").arg(QStringList(mTaskScanFileTypes.begin(), mTaskScanFileTypes.end()).join("|")));
    }

    QStringList allFiles(mFilesForScan.begin(), mFilesForScan.end());
    mFilesForScan.clear();
    for (const auto& d : allFiles) {
        if (dirExpReg.exactMatch(d)) {
            TASK_SCAN_LOG_INFO << "不需要扫描(例外路径): " << d;
            mFilesForScan.remove(d);
        }
        else if (tpExpReg.exactMatch(d)) {
            TASK_SCAN_LOG_INFO << "不需要扫描(例外类型): " << d;
            mFilesForScan.remove(d);
        }
        else if (!tpReg.exactMatch(d)) {
            TASK_SCAN_LOG_INFO << "不需要扫描(非扫描类型): " << d;
            mFilesForScan.remove(d);
        }
        else {
            TASK_SCAN_LOG_INFO << "需要扫描: " << d;
        }
    }
    mFilesForScan = QSet<QString>(allFiles.begin(), allFiles.end());
    allFiles.clear();

    // 解析文件内容 并 扫描
    TASK_SCAN_LOG_INFO << "Finish scan files";
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
    C_RETURN_IF_OK(mIsRunning);

    TASK_SCAN_LOG_INFO << "Running";
    mTaskStatus = ScanTaskStatus::Running;
    scanFiles();

    while (true) {
        usleep(1000 * 1000 * 5);
        if (mTaskStatus == ScanTaskStatus::Stop) {
            mTaskStatus = ScanTaskStatus::Stopped;
            mIsRunning = false;
            TASK_SCAN_LOG_INFO << "Stopped";
            break;
        }
        else if (mTaskStatus == ScanTaskStatus::Running) {
            mIsRunning = true;
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



