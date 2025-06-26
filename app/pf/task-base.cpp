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
#include <QTemporaryDir>
#include <QJsonDocument>

#include <unistd.h>
#include <macros/macros.h>

#include "data-base.h"
#include "utils.h"
#include "tika-wrap/src/java-env.h"

#define TASK_SCAN_LOG_INFO       qInfo()    << "[TaskId: " << getTaskId() << " TaskName: " << getTaskName() << "] "
#define TASK_SCAN_LOG_WARN       qWarning() << "[TaskId: " << getTaskId() << " TaskName: " << getTaskName() << "] "


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
    TASK_SCAN_LOG_INFO << "Start scan files";

    mFilesForScan.clear();
    mFilesScanned.clear();

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
            TASK_SCAN_LOG_INFO << "skip special directory: " << path;
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
            TASK_SCAN_LOG_INFO << "skip special directory: " << path;
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
            if (!fi.exists()) { TASK_SCAN_LOG_INFO << "Not exists directory: " << fi.absolutePath(); continue;}
            if (!isInScanDir(fi.absolutePath())) { TASK_SCAN_LOG_INFO << "Not absolute path"; continue; }
            if (fi.isDir() && !fi.isSymbolicLink()) {
                QDir ddir(dirs.at(i));
                TASK_SCAN_LOG_INFO << "scanning directory: " << ddir.absolutePath();
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

    // 第一次扫描 -- 检查是否存在，存在则读取，否则扫描
    if (!DataBase::getInstance().checkTempTaskFileExist(getTaskId())) {
        QSet<QString> filesForScan;
        for (const auto& d : mTaskScanPath) {
            TASK_SCAN_LOG_INFO << "遍历扫描路径: " << d;
            if (isInScanDir(d)) {
                filesForScan += getDirFiles(d);
            }
            else {
                TASK_SCAN_LOG_INFO << "例外文件夹: " << d;
            }
        }
        DataBase::getInstance().saveTempTaskFileFirst(getTaskId(), filesForScan);
        DataBase::getInstance().updateTotalFile(getTaskId(), filesForScan.size());
        mFilesForScan = filesForScan;
        TASK_SCAN_LOG_INFO << "All files: " << filesForScan.size();
    }
    else {
        DataBase::getInstance().loadTempTaskFile(getTaskId(), mFilesForScan, mFilesScanned);
    }
}

void ScanTask::taskFinished()
{
    mTaskStatus = ScanTaskStatus::Finished;
    DataBase::getInstance().updateTaskStatusFinished(getTaskId());
    DataBase::getInstance().updateStopTime(getTaskId(), QDateTime::currentDateTime());
}

void ScanTask::pop100File(QStringList & fileMap) const
{
    fileMap.clear();

    C_RETURN_IF_OK(mFilesForScan.isEmpty());

    for (auto& f : mFilesForScan) {
        fileMap << f;
        C_BREAK_IF_OK(fileMap.count() >= 100);
    }
}

void ScanTask::update100FileStatus(QStringList& fileMap)
{
    // 更新临时数据
    for (auto& f : fileMap) {
        mFilesForScan.remove(f);
        mFilesScanned << f;
    }
    DataBase::getInstance().updateTempTaskFile(getTaskId(), mFilesForScan, mFilesScanned);
}

QPair<QString, QString> ScanTask::getScanFileResult(const QString& filePath)
{
    // 扫描任务 -- 临时数据
    const QString md5 = DataBase::getInstance().getTaskFileMd5(getTaskId(), filePath);

    // 扫描结果 -- md5、policy_id
}

void ScanTask::fileScanFinished(const QString& path, const QString& md5, bool isHit, const QList<QString>& ctx)
{
    // 更新临时表状态
    // DataBase::getInstance().updateTaskTable(getTaskId(), path, md5, true);

    // TODO:// 更新 结果表状态
    if (isHit) {
        // 保存到扫描结果里
    }
}

void ScanTask::scanFile(const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        TASK_SCAN_LOG_INFO << "Not exists file: " << filePath;
        return;
    }

    if (mPoliciesIdx.empty()) {
        TASK_SCAN_LOG_WARN << "File " << filePath << " does not exist";
        return;
    }

    TASK_SCAN_LOG_INFO << "Scann file: " << filePath;

    // 提取文件内容
    const QTemporaryDir tmpDir;
    const QString tmpDirPath = tmpDir.path();
    if (!JavaEnv::getInstance()->parseFile(filePath, tmpDirPath)) {
        TASK_SCAN_LOG_INFO << "Failed to parse file: " << filePath;
        return;
    }

    const QString& meta = QString("%1/meta.txt").arg(tmpDirPath);
    const QString& ctx = QString("%1/ctx.txt").arg(tmpDirPath);

    for (auto i : mPoliciesOrderIdx) {
        if (!mPoliciesIdx.contains(i)) {
            TASK_SCAN_LOG_WARN << "Not found policy id: " << i;
            continue;
        }
        const auto p = mPoliciesIdx[i];

        const MatchResult matchResult = p->match(filePath, meta, ctx);
        if (matchResult == MatchResult::PG_MATCH_OK) {
            // TODO:// 生成 敏感 日志并保存



            TASK_SCAN_LOG_INFO << "file: " << filePath << " Matched(规则匹配)!";
        }
        else if (matchResult == MatchResult::PG_MATCH_ERR) {
            TASK_SCAN_LOG_INFO << "file: " << filePath << " err(出错)!";
        }
        else if (matchResult == MatchResult::PG_MATCH_NO) {
            TASK_SCAN_LOG_INFO << "file: " << filePath << " No(规则/例外规则未命中)!";
        }
        else if (matchResult == MatchResult::PG_MATCH_EXCEPT) {
            TASK_SCAN_LOG_INFO << "file: " << filePath << " Except(例外命中)!";
        }
        else {
            TASK_SCAN_LOG_WARN << "NOT SUPPORTED RESULT!";
        }
    }
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
            QStringList fileMap;
            pop100File(fileMap);
            if (fileMap.size() <= 0) {
                taskFinished();
                TASK_SCAN_LOG_INFO << "Finish scan Task: " << getTaskId();
                continue;
            }
            TASK_SCAN_LOG_INFO << "start scann '" << fileMap.size() << "' files ...";
            for (const auto& f : fileMap) {
                scanFile(f);
            }
            update100FileStatus(fileMap);
            DataBase::getInstance().updateFinishedFileAdd(getTaskId(), fileMap.count());
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

