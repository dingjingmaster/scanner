//
// Created by dingjing on 2/26/25.
//

#include "data-base.h"

#include <QFile>
#include <QDebug>

#include <glib.h>
#include <QMutex>

#include "utils.h"
#include "sqlite3-wrap.h"
#include "macros/macros.h"


#define TASK_TMP_SCAN_FILE                      DATA_DIR"/andsec-task-"

#define SELECT_SCAN_TASK_TABLE \
    "SELECT task_id, task_name, scan_task_dir, scan_task_dir_exception, " \
    "scan_task_file_ext, scan_task_file_ext_exception, " \
    "start_time, stop_time, total_file, finished_file, " \
    "task_status, scan_mode, round, times, is_scheduled FROM scan_task;"

#define SCAN_TASK_TABLE \
    "CREATE TABLE scan_task (" \
    "   `task_id`                       VARCHAR(255)                    NOT NULL," \
    "   `task_name`                     TEXT                            NOT NULL," \
    "   `scan_task_dir`                 TEXT                            NOT NULL," \
    "   `scan_task_dir_exception`       TEXT                            NOT NULL," \
    "   `scan_task_file_ext`            TEXT                            NOT NULL," \
    "   `scan_task_file_ext_exception`  TEXT                            NOT NULL," \
    "   `policy_ids`                    TEXT                            NOT NULL," \
    "   `start_time`                    INTEGER         DEFAULT 0       NOT NULL," \
    "   `stop_time`                     INTEGER         DEFAULT 0       NOT NULL," \
    "   `total_file`                    INTEGER         DEFAULT 0       NOT NULL," \
    "   `finished_file`                 INTEGER         DEFAULT 0       NOT NULL," \
    "   `is_scheduled`                  TINYINT         DEFAULT 0       NOT NULL," \
    "   `times`                         INT             DEFAULT 0       NOT NULL," \
    "   `scheduling_cron`               VARCHAR         DEFAULT ''      NOT NULL," \
    "   `task_status`                   TINYINT         DEFAULT 0       NOT NULL," \
    "   `scan_mode`                     TINYINT         DEFAULT 0       NOT NULL," \
    "   `round`                         INT             DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY (task_id) " \
    ");"

#define SELECT_SCAN_RESULT_TABLE \
    "SELECT file_path, file_md5, policy_id, scan_finished_time, " \
    "file_type, file_ext_name, file_size, content " \
    " FROM scan_result;"

#define SCAN_RESULT_TABLE \
    "CREATE TABLE scan_result (" \
    "   `file_path`                     TEXT                            NOT NULL," \
    "   `file_md5`                      TEXT                            NOT NULL," \
    "   `policy_id`                     TEXT            DEFAULT ''      NOT NULL," \
    "   `scan_finished_time`            INTEGER         DEFAULT 0       NOT NULL," \
    "   `file_type`                     TEXT            DEFAULT ''      NOT NULL," \
    "   `file_ext_name`                 VARCHAR                         NOT NULL," \
    "   `file_size`                     INTEGER                         NOT NULL," \
    "   `content`                       TEXT            DEFAULT ''      NOT NULL," \
    "   PRIMARY KEY (file_path)" \
    ");"

#define SELECT_POLICY_ID_TABLE \
    "SELECT rid, dirty FROM policy_id;"

#define POLICY_ID \
    "CREATE TABLE policy_id (" \
    "   `rid`                           VARCHAR                         NOT NULL," \
    "   `is_checked`                    TINYINT         DEFAULT 1       NOT NULL," \
    "   `dirty`                         TINYINT         DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY (rid)" \
    ");"



DataBase DataBase::gInstance;

DataBase& DataBase::getInstance()
{
    return gInstance;
}

DataBase::~DataBase()
{
}

void DataBase::initDB() const
{
    if (!QFile::exists(DATA_DIR)) {
        g_mkdir_with_parents(DATA_DIR, 0755);
    }

    const QString dbName = QString("%1/andsec-db").arg(DATA_DIR);
    if (SQLITE_OK != mDB->connect(dbName)) {
        qWarning() << "Failed to connect to database " << dbName;
        ::exit(-1);
    }

    int ret = mDB->execute(SCAN_TASK_TABLE);
    if (SQLITE_OK != ret && (!(SQLITE_ERROR == ret && mDB->lastError().contains("exists")))) {
        qWarning() << "Failed to execute scan_task: " << ret << "  error: " << mDB->lastError();
        ::exit(-1);
    }

    ret = mDB->execute(SCAN_RESULT_TABLE);
    if (SQLITE_OK != ret && (!(SQLITE_ERROR == ret && mDB->lastError().contains("exists")))) {
        qWarning() << "Failed to execute scan_task: " << ret << "  error: " << mDB->lastError();
        ::exit(-1);
    }

    ret = mDB->execute(POLICY_ID);
    if (SQLITE_OK != ret && (!(SQLITE_ERROR == ret && mDB->lastError().contains("exists")))) {
        qWarning() << "Failed to execute policy_id: " << ret << "  error: " << mDB->lastError();
        ::exit(-1);
    }
}

bool DataBase::checkTaskExists(const QString& taskId) const
{
    bool ret = false;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT task_id FROM scan_task WHERE task_id = ?;"));
        query.bind(1, taskId);
        ret = (query.begin() != query.end());
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

void DataBase::insertTask(const QString& taskId, const QString& taskName, const QString& scanDir, const QString& scanDirExp, const QString& scanExt, const QString& scanExtExp, const QString& schedulingCron, const QStringList& policyIds,
                          int taskStatus, int scanMode, int times, bool isScheduling) const
{
    if (!checkTaskExists(taskId)) {
        try {
            const sqlite3_wrap::Sqlite3Command cmd(
                *mDB, "INSERT INTO scan_task VALUES (" ":task_id, :task_name," ":scan_task_dir, :scan_task_dir_exception," ":scan_task_file_ext, :scan_task_file_ext_exception," ":policy_ids," ":start_time, :stop_time,"
                ":total_file, :finished_file," ":is_scheduled, :times," ":scheduling_cron," ":task_status, :scan_mode, :round" ");");

            cmd.bind(":task_id", taskId);
            cmd.bind(":task_name", taskName);
            cmd.bind(":scan_task_dir", scanDir);
            cmd.bind(":scan_task_dir_exception", scanDirExp);
            cmd.bind(":scan_task_file_ext", scanExt);
            cmd.bind(":scan_task_file_ext_exception", scanExtExp);
            cmd.bind(":policy_ids", policyIds.join("{]"));
            cmd.bind(":start_time", QDateTime::currentDateTime().toMSecsSinceEpoch());
            cmd.bind(":stop_time", 0);
            cmd.bind(":total_file", 0);
            cmd.bind(":finished_file", 0);
            cmd.bind(":is_scheduled", isScheduling ? 1 : 0);
            cmd.bind(":times", times);
            cmd.bind(":scheduling_cron", schedulingCron);
            cmd.bind(":task_status", taskStatus);
            cmd.bind(":scan_mode", scanMode);
            cmd.bind(":round", 0);
            cmd.execute();
        }
        catch (std::exception& e) {
            qWarning() << __FUNCTION__ << __LINE__ << "Failed to insert scan_task: " << e.what();
        }
    }
    else {
        try {
            const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET " "times = ?, round = ? WHERE task_id = ?");
            cmd.bind(1, times);
            cmd.bind(2, 1 + getExecTimes(taskId));
            cmd.bind(3, taskId);
            const int ret = cmd.execute();
            qInfo() << __FUNCTION__ << __LINE__ << "UPDATE scan_task: " << taskId << ", ret: " << ret;
        }
        catch (std::exception& e) {
            qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
        }
    }
}

QString DataBase::getTaskFileMd5(const QString& taskId, const QString& filePath) const
{
    QString md5S = "";
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_md5 FROM T%1 WHERE file_path = ?;").arg(taskId));
        query.bind(0, filePath);
        const auto iter = query.begin();
        if (iter != query.end()) {
            md5S = (*iter).get<QString>(0);
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return md5S;
}

void DataBase::updateTotalFile(const QString& taskId, qint64 totalFile) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET total_file=? WHERE task_id=?;");
        cmd.bind(1, totalFile);
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateFinishedFile(const QString& taskId, qint64 finishedFile) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET finished_file=? WHERE task_id=?;");
        cmd.bind(1, finishedFile);
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateFinishedFileAdd(const QString& taskId, qint64 finishedFile) const
{
    qint64 total = 0;
    qint64 finished = 0;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, "SELECT finished_file, total_file FROM scan_task WHERE task_id=?;");
        query.bind(1, taskId);
        const auto iter = query.begin();
        if (iter != query.end()) {
            finished = (*iter).get<qint64>(0);
            total = (*iter).get<qint64>(1);
        }
        query.finish();
    }
    catch (std::exception& e) {
        qWarning() << "Failed to update scan_task: " << e.what();
    }

    if (total <= 0 || finished + finishedFile > total) {
        qWarning() << "Scan Task Total File Is Error!";
    }

    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET finished_file=? WHERE task_id=?;");
        cmd.bind(1, finished + finishedFile);
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateStartTime(const QString& taskId, const QDateTime& startTime) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET start_time=? WHERE task_id=?;");
        cmd.bind(1, startTime.toMSecsSinceEpoch());
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateStopTime(const QString& taskId, const QDateTime& stopTime) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET stop_time=? WHERE task_id=?;");
        cmd.bind(1, stopTime.toMSecsSinceEpoch());
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusPause(const QString& taskId) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=4 WHERE task_id=?;");
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusRunning(const QString& taskId) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=1 WHERE task_id=?;");
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusStopped(const QString& taskId) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=2 WHERE task_id=?;");
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusFinished(const QString& taskId) const
{
    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=3 WHERE task_id=?;");
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

qint64 DataBase::getFinishedFileNum(const QString& taskId) const
{
    int ret = 0;
    int execTimes = 0;

    try {
        sqlite3_wrap::Sqlite3Query cmd(*mDB, "SELECT finished_file FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        auto iter = cmd.begin();
        if (iter != cmd.end()) {
            execTimes = (*iter).get<qint64>(0);
        }
        ret = cmd.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
    qInfo() << "ret: " << ret << ", execTimes: " << execTimes;

    return execTimes;
}

qint64 DataBase::getTotalFileNum(const QString& taskId) const
{
    int ret = 0;
    int execTimes = 0;

    try {
        sqlite3_wrap::Sqlite3Query cmd(*mDB, "SELECT total_file FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        auto iter = cmd.begin();
        if (iter != cmd.end()) {
            execTimes = (*iter).get<qint64>(0);
        }
        ret = cmd.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
    qInfo() << "ret: " << ret << ", execTimes: " << execTimes;

    return execTimes;
}

bool DataBase::getIsScheduled(const QString& taskId) const
{
    int ret = 0;
    int execTimes = 0;

    try {
        sqlite3_wrap::Sqlite3Query cmd(*mDB, "SELECT is_scheduled FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        auto iter = cmd.begin();
        if (iter != cmd.end()) {
            execTimes = (*iter).get<int>(0);
        }
        ret = cmd.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
    qInfo() << "ret: " << ret << ", scheduled: " << execTimes;

    return (execTimes == 1);
}

enum DataBase::TaskStatus DataBase::getTaskStatus(const QString& taskId) const
{
    int ret = 0;
    int status = 0;

    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, "SELECT task_status FROM scan_task WHERE task_id=?;");
        query.bind(1, taskId);
        auto iter = query.begin();
        if (iter != query.end()) {
            status = (*iter).get<int>(0);
        }
        ret = query.finish();
        qInfo() << "status: " << status << ", ret: " << ret;
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }

    if (TASK_STATUS_RUNNING == status) {
        return TASK_STATUS_RUNNING;
    }
    else if (TASK_STATUS_STOPPED == status) {
        return TASK_STATUS_STOPPED;
    }
    else if (TASK_STATUS_FINISHED == status) {
        return TASK_STATUS_FINISHED;
    }
    else if (TASK_STATUS_PAUSE == status) {
        return TASK_STATUS_PAUSE;
    }

    return TASK_STATUS_UNKNOWN;
}

bool DataBase::checkNeedRun(const QString& taskId) const
{
    int ret = 0;
    int times = 0;
    int round = 0;

    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, "SELECT times, round FROM scan_task WHERE task_id=?;");
        query.bind(1, taskId);
        auto iter = query.begin();
        if (iter != query.end()) {
            times = (*iter).get<int>(0);
            round = (*iter).get<int>(1);
        }
        ret = query.finish();
        qInfo() << "times: " << times << "round: " << round << ", ret: " << ret;
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }

    return round <= times;
}

QStringList DataBase::queryTaskIds() const
{
    QStringList res;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, "SELECT task_id FROM scan_task;");
        for (auto it = query.begin(); it != query.end(); ++it) {
            res << (*it).get<QString>(0);
        }
    }
    catch (std::exception& e) {
        qWarning() << "Failed to update scan_task: " << e.what();
    }

    return res;
}

int DataBase::getExecTimes(const QString& taskId) const
{
    int ret = 0;
    int execTimes = 0;

    try {
        sqlite3_wrap::Sqlite3Query cmd(*mDB, "SELECT round FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        auto iter = cmd.begin();
        if (iter != cmd.end()) {
            execTimes = (*iter).get<int>(0);
        }
        ret = cmd.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
    qInfo() << "ret: " << ret << ", execTimes: " << execTimes;

    return execTimes;
}

int DataBase::getTimes(const QString& taskId) const
{
    int ret = 0;
    int execTimes = 0;

    try {
        sqlite3_wrap::Sqlite3Query cmd(*mDB, "SELECT times FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        auto iter = cmd.begin();
        if (iter != cmd.end()) {
            execTimes = (*iter).get<int>(0);
        }
        ret = cmd.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
    qInfo() << "ret: " << ret << ", execTimes: " << execTimes;

    return execTimes;
}

void DataBase::setTimes(const QString& taskId, int times) const
{
    try {
        int ret = 0;
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET times = ? WHERE task_id=?;");
        cmd.bind(1, times);
        cmd.bind(1, taskId);
        ret = cmd.execute();
        qInfo() << "ret: " << ret << ", times: " << times;
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }
}

void DataBase::deleteTask(const QString& taskId) const
{
    QSet<QString> res;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, "SELECT policy_ids FROM scan_task WHERE task_id=?;");
        query.bind(1, taskId);
        for (auto it = query.begin(); it != query.end(); ++it) {
            res += (*it).get<QString>(0).split("{]").toSet();
        }

        sqlite3_wrap::Sqlite3Command cmd(*mDB, "DELETE FROM scan_task WHERE task_id=?;");
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to delete scan_task: " << e.what();
    }

    for (auto& rid : res) {
        updateRuleIdDirty(rid);
    }
}

void DataBase::updateRuleId(const QString& ruleId) const
{
    int ret = 0;
    if (checkRuleIdExists(ruleId)) {
        try {
            const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE policy_id SET dirty=0 WHERE rid=?;");
            cmd.bind(1, ruleId);
            ret = cmd.execute();
        }
        catch (std::exception& ex) {
            qWarning() << "err: " << ex.what();
        }
    }
    else {
        try {
            const sqlite3_wrap::Sqlite3Command cmd(*mDB, "INSERT INTO policy_id VALUES (:rid, 1, 0);");
            cmd.bind(":rid", ruleId);
            ret = cmd.execute();
        }
        catch (std::exception& ex) {
            qWarning() << "err: " << ex.what();
        }
    }
    qInfo() << "ruleId: " << ruleId << ", ret: " << ret;
}

void DataBase::updateRuleIdDirty(const QString& ruleId) const
{
    int ret = 0;

    try {
        const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE policy_id SET dirty=1 WHERE rid=?;");
        cmd.bind(1, ruleId);
        ret = cmd.execute();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    qInfo() << "UPDATE ruleId: " << ruleId << ", dirty! ret: " << ret;
}

bool DataBase::checkRuleIdExists(const QString& ruleId) const
{
    bool ret = false;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT rid FROM policy_id WHERE rid = ?;"));
        query.bind(1, ruleId);
        ret = (query.begin() != query.end());
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

void DataBase::showRuleId() const
{
    qInfo() << SELECT_POLICY_ID_TABLE;

    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString(SELECT_POLICY_ID_TABLE));
        auto iter = query.begin();
        for (; iter != query.end(); ++iter) {
            qInfo() << "===========\n" << "Policy ID       : " << (*iter).get<QString>(0) << "\n" << "dirty           : " << (*iter).get<QString>(1) << "\n";
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << ex.what();
    }
}

void DataBase::showScanTask() const
{
    auto getTaskStatusString = [=](int taskStatus) -> QString {
        switch (taskStatus) {
        case 0: {
            return "未开始";
        }
        case 1: {
            return "扫描中";
        }
        case 2: {
            return "已停止";
        }
        case 3: {
            return "已完成";
        }
        case 4: {
            return "已暂停";
        }
        case 5: {
            return "出错";
        }
        default: break;
        }
        return "未知";
    };

    auto getTaskModeString = [=](int taskMode) -> QString {
        switch (taskMode) {
        case 0: return "默认模式";
        case 1: return "免打扰模式";
        case 2: return "快速模式";
        }
        return "未知";
    };

    qInfo() << SELECT_SCAN_TASK_TABLE;

    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString(SELECT_SCAN_TASK_TABLE));
        auto iter = query.begin();
        for (; iter != query.end(); ++iter) {
            const qint64 startTime = (*iter).get<qint64>(6);
            const qint64 stopTime = (*iter).get<qint64>(7);
            qInfo() << "===========\n" << "Task ID         : " << (*iter).get<QString>(0) << "\n" << "Task Name       : " << (*iter).get<QString>(1) << "\n" << "Scan Dir        : " << (*iter).get<QString>(2).split("{]") << "\n" <<
                "Scan Dir(N)     : " << (*iter).get<QString>(3).split("{]") << "\n" << "Scan File ext   : " << (*iter).get<QString>(4).split("{]") << "\n" << "Scan File ext(N): " << (*iter).get<QString>(5).split("{]") << "\n" <<
                "Start Time      : " << (startTime > 0 ? QDateTime::fromMSecsSinceEpoch(startTime).toLocalTime().toString("yyyy-mm-dd HH:MM:ss") : "null") << "\n" << "Stop Time       : " << (
                    stopTime > 0 ? QDateTime::fromMSecsSinceEpoch(stopTime).toLocalTime().toString("yyyy-mm-dd HH:MM:ss") : "null") << "\n" << "Total File      : " << (*iter).get<qint64>(8) << "\n" << "Finished File   : " << (*iter).get<
                    qint64>(9) << "\n" << "Task Status     : " << getTaskStatusString((*iter).get<int>(10)) << "\n" << "Task Mode       : " << getTaskModeString((*iter).get<int>(11)) << "\n" << "Round           : " << (*iter).get<int>(12)
                << "\n" << "Times           : " << (*iter).get<int>(13) << "\n" << "Is scheduled    : " << (*iter).get<int>(14) << "\n\n\n";
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << ex.what();
    }
}

void DataBase::showScanResult() const
{
    qInfo() << SELECT_SCAN_RESULT_TABLE;

    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString(SELECT_SCAN_RESULT_TABLE));
        auto iter = query.begin();
        for (; iter != query.end(); ++iter) {
            qInfo() << "===========\n" << "File Path       : " << (*iter).get<QString>(0) << "\n" << "File MD5        : " << (*iter).get<QString>(1) << "\n" << "Policy ID       : " << (*iter).get<QString>(2).split("{]") << "\n" <<
                "Finished Time   : " << QDateTime::fromMSecsSinceEpoch((*iter).get<qint64>(3)).toLocalTime().toString("yyyy-mm-dd HH:MM:ss") << "\n" << "File Type       : " << (*iter).get<QString>(4).split("{]") << "\n" <<
                "File Ext Name   : " << (*iter).get<QString>(5) << "\n" << "File Size       : " << (*iter).get<qint64>(6) << "\n" << "Content         :";
            const auto ctArr = (*iter).get<QString>(7).split("{|]");
            for (auto& c : ctArr) {
                const auto arr = c.split("{]");
                if (arr.size() != 3) {
                    qWarning() << arr.size();
                    continue;
                }
                qInfo() << "" << "    ruleId      : " << arr.at(0) << "\n" << "    keyword     : " << arr.at(1) << "\n" << "    content     : " << arr.at(2) << "\n";
            }
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << ex.what();
    }
}

QString DataBase::getScanResultItemMd5(const QString& filePath) const
{
    QString ret = "";
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_md5 FROM scan_result WHERE file_path = ?;"));
        query.bind(1, filePath);
        const auto iter = query.begin();
        if (iter != query.end()) {
            ret = (*iter).get<QString>(0);
            query.finish();
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

QString DataBase::getScanResultItemType(const QString& filePath) const
{
    QString ret = "";
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_type FROM scan_result WHERE file_path = ?;"));
        query.bind(1, filePath);
        const auto iter = query.begin();
        if (iter != query.end()) {
            ret = (*iter).get<QString>(0);
            query.finish();
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

bool DataBase::checkScanResultItemExists(const QString& filePath) const
{
    bool ret = false;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_path FROM scan_result WHERE file_path = ?;"));
        query.bind(1, filePath);
        ret = (query.begin() != query.end());
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

QList<DataBase::ResultContent> DataBase::getScanResultContent(const QString& filePath) const
{
    QList<ResultContent> ret;
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT content FROM scan_result WHERE file_path = ?;"));
        query.bind(1, filePath);
        const auto iter = query.begin();
        if (iter != query.end()) {
            const auto content = (*iter).get<QString>(0);
            query.finish();

            const auto arrCT = content.split("{|]");
            for (auto& a : arrCT) {
                const auto arr1CT = a.split("{]");
                if (arr1CT.length() != 3) {
                    continue;
                }
                ResultContent ctx;
                ctx.ruleId = arr1CT.at(0);
                ctx.key = arr1CT.at(1);
                ctx.content = arr1CT.at(2);
                ret.append(ctx);
            }
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

QPair<QString, QString> DataBase::getScanResultPolicyIdAndMd5(const QString& filePath) const
{
    QPair<QString, QString> ret("", "");
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_md5, policy_id FROM scan_result WHERE file_path = ?;"));
        query.bind(1, filePath);
        const auto iter = query.begin();
        if (iter != query.end()) {
            const auto md5S = (*iter).get<QString>(0);
            const auto policyId = (*iter).get<QString>(1);
            query.finish();
            ret = QPair<QString, QString>(md5S, policyId);
        }
        query.finish();
    }
    catch (std::exception& ex) {
        qWarning() << "err: " << ex.what();
    }

    return ret;
}

void DataBase::updateScanResultItems(const QString& filePath, const QString& ruleId, const QString& fileType, const QString& content) const
{
    static QMutex lock;
    QMutexLocker locker(&lock);

    const QString md5 = Utils::getFileMD5(filePath);
    const qint64 finishedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    const QString fileExt = Utils::getFileExtName(filePath);
    const qint64 fileSize = Utils::getFileSize(filePath);

    QMap<QString, QSet<QString>> ctx;
    QSet<QString> ctxV;
    ctxV << content;
    ctx[ruleId] = ctxV;

    // file type
    QSet<QString> ft = fileType.split("{]").toSet();

    // rule id
    QSet<QString> ruleIds;
    ruleIds << ruleId;

    // get rule_id
    auto getRuleIds = [=](const QSet<QString>& rules) -> QString {
        return rules.toList().join("{]");
    };

    // get file_type
    auto getFileTypes = [=](const QSet<QString>& fts) -> QString {
        return fts.toList().join("{]");
    };

    // get content
    auto getContent = [=](const QMap<QString, QSet<QString>>& gc) -> QString {
        QStringList ctxT;
        for (auto it = gc.keyValueBegin(); it != gc.keyValueEnd(); ++it) {
            ctxT.append(QString("%1{]%2").arg(it->first).arg(it->second.toList().join("{]")));
        }
        return ctxT.join("{|]");
    };

    if (checkScanResultItemExists(filePath)) {
        try {
            sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT content, file_type, policy_id FROM scan_result WHERE file_path = ?;"));
            query.bind(1, filePath);
            auto iter = query.begin();
            const QString cT = (*iter).get<QString>(0);
            const QString ftT = (*iter).get<QString>(1);
            const QString piT = (*iter).get<QString>(2);

            // update context
            const auto arrCT = cT.split("{|]");
            for (auto& a : arrCT) {
                const auto arr1CT = a.split("{]");
                if (arr1CT.length() != 3) {
                    continue;
                }
                if (ctx.contains(arr1CT[0])) {
                    QSet<QString> v = ctx[arr1CT[0]];
                    v << QString("%1{]%2").arg(arr1CT[1]).arg(arr1CT[2]);
                    ctx[arr1CT[0]] = v;
                }
                else {
                    QSet<QString> v;
                    v << QString("%1{]%2").arg(arr1CT[1]).arg(arr1CT[2]);
                    ctx[arr1CT[0]] = v;
                }
            }

            // update file_type
            const auto ftTT = ftT.split("{]");
            for (auto& f : ftTT) { ft << f; }

            // update policy_id
            const auto piTT = piT.split("{]");
            for (auto& p : piTT) { ruleIds << p; }

            query.finish();
        }
        catch (std::exception& ex) {
            qWarning() << "err: " << ex.what();
        }

        // update
        try {
            sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_result SET " "file_md5 = ?," "policy_id = ?," "scan_finished_time = ?," "file_type = ?," "file_size = ?," "content = ? WHERE file_path = ?;");
            cmd.bind(1, md5);
            cmd.bind(2, getRuleIds(ruleIds));
            cmd.bind(3, finishedTime);
            cmd.bind(4, getFileTypes(ft));
            cmd.bind(5, fileSize);
            cmd.bind(6, getContent(ctx));
            cmd.bind(7, filePath);
            const int ret = cmd.execute();

            qInfo() << "Update:\n" << "file path       : " << filePath << "\n" << "rule ids        : " << getRuleIds(ruleIds) << "\n" << "finished time   : " << finishedTime << "\n" << "file type       : " << getFileTypes(ft) << "\n" <<
                "file size       : " << fileSize << "\n" << "content         : " << getContent(ctx) << "\n" << "ret             : " << ret << "\n";
        }
        catch (std::exception& ex) {
            qWarning() << "err: " << ex.what();
        }
    }
    else {
        try {
            const sqlite3_wrap::Sqlite3Command cmd(*mDB, "INSERT INTO scan_result VALUES (" ":file_path, :file_md5," ":policy_id, :scan_finished_time," ":file_type, :file_ext_name," ":file_size, :content" ");");
            cmd.bind(":file_path", filePath);
            cmd.bind(":file_md5", md5);
            cmd.bind(":policy_id", getRuleIds(ruleIds));
            cmd.bind(":scan_finished_time", finishedTime);
            cmd.bind(":file_type", getFileTypes(ft));
            cmd.bind(":file_ext_name", fileExt);
            cmd.bind(":file_size", fileSize);
            cmd.bind(":content", getContent(ctx));
            const int ret = cmd.execute();

            qInfo() << "Insert:\n" << "file path       : " << filePath << "\n" << "md5             : " << md5 << "\n" << "rule ids        : " << getRuleIds(ruleIds) << "\n" << "finished time   : " << finishedTime << "\n" <<
                "file type       : " << getFileTypes(ft) << "\n" << "file size       : " << fileSize << "\n" << "content         : " << getContent(ctx) << "\n" << "ret             : " << ret << "\n";
        }
        catch (std::exception& ex) {
            qWarning() << "err: " << ex.what();
        }
    }
}

bool DataBase::checkTempTaskFileExist(const QString& taskId) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);

    return QFile::exists(path);
}

void DataBase::deleteTempTaskFile(const QString& taskId) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    QFile::remove(path);
}

void DataBase::saveTempTaskFileFirst(const QString& taskId, const QSet<QString>& files) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to open file: " << path;
        return;
    }
    for (auto& f : files) {
        const QString line = QString("%1{|]0\n").arg(f);
        file.write(line.toUtf8());
    }
    file.close();
}

void DataBase::loadTempTaskFile(const QString& taskId, QSet<QString>& filesForScan, QSet<QString>& filesScanned) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to open file: " << path;
        return;
    }

    while (!file.atEnd()) {
        const QString line = file.readLine();
        const auto arr = line.split("{|]");
        if (arr.count() != 2) {
            continue;
        }
        if (arr.at(1).toInt()) {
            filesScanned << arr.at(0);
        }
        else {
            filesForScan << arr.at(0);
        }
    }

    file.close();
}

void DataBase::updateTempTaskFile(const QString& taskId, const QSet<QString>& filesForScan, QSet<QString>& filesScanned) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    const QString pathTmp = QString("%1%2.tmp").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    QFile file(pathTmp);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to open file: " << pathTmp;
        return;
    }

    for (auto& f : filesForScan) {
        const QString line = QString("%1{|]0\n").arg(f);
        file.write(line.toUtf8());
    }
    for (auto& f : filesScanned) {
        const QString line = QString("%1{|]1\n").arg(f);
        file.write(line.toUtf8());
    }
    file.close();

    QFile::remove(path);
    QFile::rename(pathTmp, path);
}

void DataBase::createPolicyIdTable() const
{
    if (!mDB->checkTableIsExist("policy_id")) {
        mDB->execute(POLICY_ID);
    }
}

DataBase::DataBase(QObject* parent)
    : QObject(parent), mDB(std::make_shared<sqlite3_wrap::Sqlite3>())
{
    initDB();
}
