//
// Created by dingjing on 2/26/25.
//

#include "data-base.h"

#include <QFile>
#include <QDebug>

#include <glib.h>

#include "sqlite3-wrap.h"

#define TASK_TMP_SCAN_FILE                      DATA_DIR"/andsec-task-"

#define SELECT_SCAN_TASK_TABLE \
    "SELECT task_id, task_name, scan_task_dir, scan_task_dir_exception, " \
    "scan_task_file_ext, scan_task_file_ext_exception, " \
    "start_time, stop_time, total_file, finished_file, " \
    "task_status, scan_mode, round FROM scan_task;"

#define SCAN_TASK_TABLE \
    "CREATE TABLE scan_task (" \
    "   `task_id`                       VARCHAR(255)                    NOT NULL," \
    "   `task_name`                     TEXT                            NOT NULL," \
    "   `scan_task_dir`                 TEXT                            NOT NULL," \
    "   `scan_task_dir_exception`       TEXT                            NOT NULL," \
    "   `scan_task_file_ext`            TEXT                            NOT NULL," \
    "   `scan_task_file_ext_exception`  TEXT                            NOT NULL," \
    "   `start_time`                    INTEGER         DEFAULT 0       NOT NULL," \
    "   `stop_time`                     INTEGER         DEFAULT 0       NOT NULL," \
    "   `total_file`                    INTEGER         DEFAULT 0       NOT NULL," \
    "   `finished_file`                 INTEGER         DEFAULT 0       NOT NULL," \
    "   `task_status`                   TINYINT         DEFAULT 0       NOT NULL," \
    "   `scan_mode`                     TINYINT         DEFAULT 0       NOT NULL," \
    "   `round`                         INT             DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY (task_id) " \
    ");"

#define SCAN_RESULT_TABLE \
    "CREATE TABLE scan_result (" \
    "   `file_path`                     TEXT                            NOT NULL," \
    "   `file_md5`                      TEXT                            NOT NULL," \
    "   `policy_id`                     TEXT            DEFAULT ''      NOT NULL," \
    "   `mis_policy_id`                 TEXT            DEFAULT ''      NOT NULL," \
    "   `scan_finished_time`            INTEGER         DEFAULT 0       NOT NULL," \
    "   `file_type`                     VARCHAR         DEFAULT ''      NOT NULL," \
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

void DataBase::insertTask(const QString & taskId, const QString & taskName, const QString & scanDir, const QString & scanDirExp, const QString & scanExt, const QString & scanExtExp, int taskStatus, int scanMode) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "INSERT INTO scan_task VALUES ("
                                           ":task_id, :task_name,"
                                           ":scan_task_dir, :scan_task_dir_exception,"
                                           ":scan_task_file_ext, :scan_task_file_ext_exception,"
                                           ":start_time, :stop_time,"
                                           ":total_file, :finished_file,"
                                           ":task_status, :scan_mode, :round"
                                           ");");
    try {
        cmd.bind(":task_id", taskId);
        cmd.bind(":task_name", taskName);
        cmd.bind(":scan_task_dir", scanDir);
        cmd.bind(":scan_task_dir_exception", scanDirExp);
        cmd.bind(":scan_task_file_ext", scanExt);
        cmd.bind(":scan_task_file_ext_exception", scanExtExp);
        cmd.bind(":start_time", QDateTime::currentDateTime().toMSecsSinceEpoch());
        cmd.bind(":stop_time", 0);
        cmd.bind(":total_file", 0);
        cmd.bind(":finished_file", 0);
        cmd.bind(":task_status", taskStatus);
        cmd.bind(":scan_mode", scanMode);
        cmd.bind(":round", 0);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to insert scan_task: " << e.what();
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

#if 0
// bool DataBase::checkTaskTableFileExists(const QString & taskId, const QString & filePath) const
// {
//     return mDB->checkKeyExist("T" + taskId, "file_path", filePath);
// }

void DataBase::insertTaskTable(const QString & taskId, const QString & filePath, const QString& md5) const
{
    if (checkTaskTableFileExists(taskId, filePath)) {
        if (!md5.isNull() && !md5.isEmpty() && "" != md5) {
            sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_md5 FROM T%1 WHERE file_path = ?;").arg(taskId));
            query.bind(0, filePath);
            const auto iter = query.begin();
            const auto md5S = (*iter).get<QString>(0);
            if (md5S == md5) {
                query.finish();
                return;
            }
            query.finish();

            const sqlite3_wrap::Sqlite3Command cmd(*mDB, QString("UPDATE T%1 SET file_md5 = ?, is_finished = 0 WHERE file_path = ?;")
                                                .arg(taskId).toUtf8().constData());
            try {
                cmd.bind(1, md5);
                cmd.bind(2, filePath);
                cmd.execute();
            }
            catch (std::exception& e) {
                qWarning() << __FUNCTION__ << __LINE__ << "Failed to update task_table: " << e.what();
            }
            return;
        }
        return;
    }
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, QString("INSERT INTO T%1 VALUES (:file_path, :file_md5, :is_finished);").arg(taskId).toUtf8().constData());
    try {
        cmd.bind(1, filePath);
        cmd.bind(2, md5);
        cmd.bind(3, 0);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update insert: " << e.what();
    }
}

void DataBase::updateTaskTable(const QString& taskId, const QString& filePath, const QString& md5, bool isFinished) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, QString("UPDATE T%1 SET file_md5 = ?, is_finished = ? WHERE file_path = ?;")
                                        .arg(taskId).toUtf8().constData());
    try {
        cmd.bind(1, md5);
        cmd.bind(2, isFinished ? 1 : 0);
        cmd.bind(3, filePath);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update task_table: " << e.what();
    }
}

bool DataBase::get100FileByTaskId(const QString & taskId, QMap<QString, QString>& files) const
{
    //
    QMap<QString, short> ctx;

    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to open file: " << path;
        return false;
    }
    file.close();



    /*
    sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_path, file_md5 FROM T%1 WHERE is_finished = 0 LIMIT 100;").arg(taskId));
    for (auto it = query.begin(); it != query.end(); ++it) {
        const auto filePath = (*it).get<QString>(0);
        const auto md5S = (*it).get<QString>(1);
        files[filePath] = md5S;
    }
    return (SQLITE_OK == query.finish());
    */

    return false;
}
#endif

void DataBase::updateTotalFile(const QString & taskId, qint64 totalFile) const
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

void DataBase::updateFinishedFile(const QString & taskId, qint64 finishedFile) const
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

void DataBase::updateStartTime(const QString & taskId, const QDateTime& startTime) const
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

void DataBase::updateStopTime(const QString & taskId, const QDateTime& stopTime) const
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

void DataBase::updateTaskStatusPause(const QString & taskId) const
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

void DataBase::updateTaskStatusRunning(const QString & taskId) const
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

void DataBase::updateTaskStatusStopped(const QString & taskId) const
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

void DataBase::updateTaskStatusFinished(const QString & taskId) const
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
        for (;iter != query.end(); ++iter) {
            qInfo() << "===========\n" \
                << "Policy ID       : " << (*iter).get<QString>(0) << "\n" \
                << "dirty           : " << (*iter).get<QString>(1) << "\n";
        }
        query.finish();
    }
    catch (std::exception &ex) {
        qWarning() << ex.what();
    }
}

void DataBase::showScanTask() const
{
    auto getTaskStatusString = [=] (int taskStatus) -> QString {
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

    auto getTaskModeString = [=] (int taskMode) -> QString {
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
        for (;iter != query.end(); ++iter) {
            const qint64 startTime = (*iter).get<qint64>(6);
            const qint64 stopTime = (*iter).get<qint64>(7);
            qInfo() << "===========\n" \
                << "Task ID         : " << (*iter).get<QString>(0) << "\n" \
                << "Task Name       : " << (*iter).get<QString>(1) << "\n" \
                << "Scan Dir        : " << (*iter).get<QString>(2).split("{]") << "\n" \
                << "Scan Dir(N)     : " << (*iter).get<QString>(3).split("{]") << "\n" \
                << "Scan File ext   : " << (*iter).get<QString>(4).split("{]") << "\n" \
                << "Scan File ext(N): " << (*iter).get<QString>(5).split("{]") << "\n" \
                << "Start Time      : " << (startTime > 0 ? QDateTime::fromMSecsSinceEpoch(startTime).toLocalTime().toString("yyyy-mm-dd HH:MM:ss") : "null") << "\n" \
                << "Stop Time       : " << (stopTime > 0 ? QDateTime::fromMSecsSinceEpoch(stopTime).toLocalTime().toString("yyyy-mm-dd HH:MM:ss") : "null") << "\n" \
                << "Total File      : " << (*iter).get<qint64>(8) << "\n" \
                << "Finished File   : " << (*iter).get<qint64>(9) << "\n" \
                << "Task Status     : " << getTaskStatusString((*iter).get<int>(10)) << "\n" \
                << "Task Mode       : " << getTaskModeString((*iter).get<int>(11)) << "\n" \
                << "Round           : " << (*iter).get<int>(12) << "\n\n\n";
        }
        query.finish();
    }
    catch (std::exception &ex) {
        qWarning() << ex.what();
    }
}

QPair<QString, QString> DataBase::getScanResultPolicyIdAndMd5(const QString& filePath) const
{
    QPair<QString, QString> ret("", "");
    try {
        sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_md5, policy_id FROM scan_result WHERE file_path = ?;"));
        query.bind(0, filePath);
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

bool DataBase::checkTempTaskFileExist(const QString& taskId) const
{
    const QString path = QString("%1%2").arg(TASK_TMP_SCAN_FILE).arg(taskId);

    return QFile::exists(path);
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
