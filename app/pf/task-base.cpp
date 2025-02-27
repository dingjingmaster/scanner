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

#include "data-base.h"
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
        const auto ts = ff.toObject()["v"].toString().split("|");
        for (const auto& t : ts) {
            mTaskScanFileTypes << t;
        }
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
        const auto ts = ff.toObject()["v"].toString().split("|");
        for (const auto& t : ts) {
            mTaskBypassFileTypes << t;
        }
    }
}

const QSet<QString> & ScanTask::getBypassFileType() const
{
    return mTaskBypassFileTypes;
}

void ScanTask::setTaskScanPath(const QString & taskScanPath)
{
    if (taskScanPath.isNull() || taskScanPath.isEmpty()) {
        mTaskScanPath << "/";
        return;
    }
    const auto ls = taskScanPath.split("|");
    for (const auto& t : ls) {
        mTaskScanPath << Utils::formatPath(t);
    }
}

const QSet<QString> & ScanTask::getTaskScanPath() const
{
    return mTaskScanPath;
}

void ScanTask::setTaskBypassPath(const QString & taskBypassPath)
{
    const auto ls = taskBypassPath.split("|");
    for (const auto& t : ls) {
        if (!t.isNull() && !t.isEmpty()) {
            mTaskBypassPath << Utils::formatPath(t);
        }
    }
}

const QSet<QString> & ScanTask::getTaskBypassPath() const
{
    return mTaskBypassPath;
}

void ScanTask::scanFiles()
{
    // @TODO:// 暂时把所有要扫描的文件路径保存到内存，后续保存到文件中，避免占用太多内存
    TASK_SCAN_LOG_INFO << "Start scan files";

    auto isInScanDir = [&](const QString & path) -> bool {
        C_RETURN_VAL_IF_OK("/" == path, true);
        QString dir = path;
        if (!dir.endsWith("/")) {
            dir += "/";
        }
        if (dir.startsWith("/bin/")
            || dir.startsWith("/boot/")
            || dir.startsWith("/dev/")
            || dir.startsWith("/efi/")
            || dir.startsWith("/etc/")
            || dir.startsWith("/lib/")
            || dir.startsWith("/lib64/")
            || dir.startsWith("/opt/")
            || dir.startsWith("/proc/")
            || dir.startsWith("/run/")
            || dir.startsWith("/sbin/")
            || dir.startsWith("/snap/")
            || dir.startsWith("/srv/")
            || dir.startsWith("/sys/")
            || dir.startsWith("/usr/")
            || dir.startsWith("/tmp/")
            || dir.startsWith("/var/")) {
            TASK_SCAN_LOG_INFO << "跳过特殊文件夹: " << path;
            return false;
        }
        return true;
    };

    auto isInScan = [&] (const QString& path) ->bool {
        if (path.startsWith("/bin/")
            || path.startsWith("/boot/")
            || path.startsWith("/dev/")
            || path.startsWith("/efi/")
            || path.startsWith("/etc/")
            || path.startsWith("/lib/")
            || path.startsWith("/lib64/")
            || path.startsWith("/opt/")
            || path.startsWith("/proc/")
            || path.startsWith("/run/")
            || path.startsWith("/sbin/")
            || path.startsWith("/snap/")
            || path.startsWith("/srv/")
            || path.startsWith("/sys/")
            || path.startsWith("/usr/")
            || path.startsWith("/tmp/")
            || path.startsWith("/var/")) {
            TASK_SCAN_LOG_INFO << "跳过特殊文件: " << path;
            return false;
        }
        // 检查是否是例外文件夹
        for (const auto& t : mTaskBypassPath) {
            if (path.startsWith(t)) {
                TASK_SCAN_LOG_INFO << "例外路径: " << t;
                return false;
            }
        }
        return true;
    };

    auto getDirFiles = [=] (const QString & dir) ->QSet<QString> {
        QSet<QString> files;
        QStringList dirs;
        dirs << dir;
        for (int i = 0; i < dirs.size(); ++i) {
            QFileInfo fi(dirs.at(i));
            if (!fi.exists()) { TASK_SCAN_LOG_INFO << "文件夹不存在: " << fi.absolutePath(); continue;}
            if (!isInScanDir(fi.absolutePath())) { continue; }
            if (fi.isDir() && !fi.isSymbolicLink()) {
                QDir ddir(dirs.at(i));
                // TASK_SCAN_LOG_INFO << "正在遍历文件夹: " << ddir.absolutePath();
                auto ds = ddir.entryList();
                for (auto& j : ds) {
                    if ("." == j || ".." == j) { continue; }
                    QString p = Utils::formatPath(QString("%1/%2").arg(fi.absoluteFilePath(), j));
                    QFileInfo fii(p);
                    if (fii.isDir()) { if (isInScanDir(p)) { dirs.append(p); }} else { if (isInScan(p)) files << p; }
                }
            }
            else {
                auto ppp = Utils::formatPath(fi.absoluteFilePath());
                if (isInScan(ppp)) { files << ppp; }
            }
        }
        return files;
    };

    QSet<QString> filesForScan;

    // 检查是否存在，存在则读取，否则扫描
    for (const auto& d : mTaskScanPath) {
        // TASK_SCAN_LOG_INFO << "遍历扫描路径: " << d;
        if (isInScanDir(d)) {
            filesForScan += getDirFiles(d);
        }
        else {
            TASK_SCAN_LOG_INFO << "例外文件夹: " << d;
        }
    }

    // 保存 所有 要扫描的文件
    DataBase::getInstance().createTaskTable(getTaskId());
    for (const auto& d : filesForScan) {
        DataBase::getInstance().updateTaskTable(getTaskId(), Utils::formatPath(d), "");
    }
    DataBase::getInstance().updateTotalFile(getTaskId(), filesForScan.size());
    TASK_SCAN_LOG_INFO << "All files: " << filesForScan.size();

    // 解析文件内容 并 扫描
    TASK_SCAN_LOG_INFO << "Finish scan files";
}

void ScanTask::taskFinished()
{
    mTaskStatus = ScanTaskStatus::Finished;
    DataBase::getInstance().updateTaskStatusFinished(getTaskId());
    DataBase::getInstance().updateStopTime(getTaskId(), QDateTime::currentDateTime());
}

bool ScanTask::pop100File(QMap<QString, QString> & fileMap) const
{
    fileMap.clear();

    return DataBase::getInstance().get100FileByTaskId(getTaskId(), fileMap);
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
        if (mTaskStatus == ScanTaskStatus::Stop) {
            mTaskStatus = ScanTaskStatus::Stopped;
            mIsRunning = false;
            DataBase::getInstance().updateTaskStatusStopped(getTaskId());
            TASK_SCAN_LOG_INFO << "Stopped";
            break;
        }
        else if (mTaskStatus == ScanTaskStatus::Running) {
            mIsRunning = true;
            QMap<QString, QString> fileMap;
            if (pop100File(fileMap)) {
                if (fileMap.size() <= 0) {
                    taskFinished();
                    TASK_SCAN_LOG_INFO << "Finish scan files";
                    continue;
                }

                for (const auto& f : fileMap.keys()) {
                    TASK_SCAN_LOG_INFO << "Scann file: " << f;
                    // 取xxx文件， 开始扫描
                    // 获取要扫描的文件 并 开始扫描
                }
            }
        }
        else if (mTaskStatus == ScanTaskStatus::Finished) {
            // 增量扫描
            usleep(1000 * 1000 * 10);
        }
        else {
            usleep(1000 * 1000 * 5);
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

QString ScanTask::getTaskScanPathStr() const
{
    return QStringList(mTaskScanPath.begin(), mTaskScanPath.end()).join("{]");
}

QString ScanTask::getPolicyIdListStr() const
{
    return QStringList(mPolicyIdList.begin(), mPolicyIdList.end()).join("{]");
}

QString ScanTask::getFileTypeListStr() const
{
    return QStringList(mTaskScanFileTypes.begin(), mTaskScanFileTypes.end()).join("{]");
}

QString ScanTask::getBypassFileTypeStr() const
{
    return QStringList(mTaskBypassFileTypes.begin(), mTaskBypassFileTypes.end()).join("{]");
}

QString ScanTask::getTaskBypassPathStr() const
{
    return QStringList(mTaskBypassPath.begin(), mTaskBypassPath.end()).join("{]");
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

    DataBase::getInstance().updateStartTime(getTaskId(), QDateTime::currentDateTime());
}

