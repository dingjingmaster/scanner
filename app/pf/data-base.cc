//
// Created by dingjing on 2/26/25.
//

#include "data-base.h"

#include <QFile>
#include <QDebug>

#include <glib.h>

#include "../sqlite3-wrap/src/sqlite3-wrap.h"

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
    "   `is_first`                      TINYINT         DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY (task_id) " \
    ");"

#define SCAN_RESULT_TABLE \
    "CREATE TABLE scan_result (" \
    "   `file_path`                     TEXT                            NOT NULL," \
    "   `file_md5`                      TEXT                            NOT NULL," \
    "   `policy_id`                     TEXT            DEFAULT ''      NOT NULL," \
    "   `scan_finished_time`            INTEGER         DEFAULT 0       NOT NULL," \
    "   `file_type`                     VARCHAR         DEFAULT ''      NOT NULL," \
    "   `file_ext_name`                 VARCHAR                         NOT NULL," \
    "   `file_size`                     INTEGER                         NOT NULL," \
    "   PRIMARY KEY (file_path)" \
    ");"

#define SCAN_TABLE \
    "CREATE TABLE T%1 (" \
    "   `file_path`                     TEXT                            NOT NULL," \
    "   `file_md5`                      TEXT                            NOT NULL," \
    "   `is_finished`                   TINYINT         DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY(file_path)" \
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
}

void DataBase::insertTask(const QString & taskId, const QString & taskName, const QString & scanDir, const QString & scanDirExp, const QString & scanExt, const QString & scanExtExp, int taskStatus, int scanMode) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "INSERT INTO scan_task VALUES ("
                                           ":task_id, :task_name,"
                                           ":scan_task_dir, :scan_task_dir_exception,"
                                           ":scan_task_file_ext, :scan_task_file_ext_exception,"
                                           ":start_time, :stop_time,"
                                           ":total_file, :finished_file,"
                                           ":task_status, :scan_mode, :is_first"
                                           ");");
    try {
        cmd.bind(":task_id", taskId);
        cmd.bind(":task_name", taskName);
        cmd.bind(":scan_task_dir", scanDir);
        cmd.bind(":scan_task_dir_exception", scanDirExp);
        cmd.bind(":scan_task_file_ext", scanExt);
        cmd.bind(":scan_task_file_ext_exception", scanExtExp);
        cmd.bind(":start_time", 0);
        cmd.bind(":stop_time", 0);
        cmd.bind(":total_file", 0);
        cmd.bind(":finished_file", 0);
        cmd.bind(":task_status", taskStatus);
        cmd.bind(":scan_mode", scanMode);
        cmd.bind(":is_first", 1);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to insert scan_task: " << e.what();
    }
}

void DataBase::createTaskTable(const QString & taskId) const
{
    if (!mDB->checkTableIsExist(taskId)) {
        mDB->execute(QString(SCAN_TABLE).arg(taskId).toUtf8().constData());
    }
}

bool DataBase::checkTaskTableFileExists(const QString & taskId, const QString & filePath) const
{
    return mDB->checkKeyExist("T" + taskId, "file_path", filePath);
}

void DataBase::updateTaskTable(const QString & taskId, const QString & filePath, const QString& md5) const
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

bool DataBase::get100FileByTaskId(const QString & taskId, QMap<QString, QString>& files) const
{
    sqlite3_wrap::Sqlite3Query query(*mDB, QString("SELECT file_path, file_md5 FROM T%1 WHERE is_finished = 0 LIMIT 100;").arg(taskId));
    for (auto it = query.begin(); it != query.end(); ++it) {
        const auto filePath = (*it).get<QString>(0);
        const auto md5S = (*it).get<QString>(1);
        files[filePath] = md5S;
    }
    return (SQLITE_OK == query.finish());
}

void DataBase::updateTotalFile(const QString & taskId, qint64 totalFile) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET total_file=? WHERE task_id=?;");
    try {
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
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET finished_file=? WHERE task_id=?;");
    try {
        cmd.bind(1, finishedFile);
        cmd.bind(2, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateStartTime(const QString & taskId, const QDateTime& startTime) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET start_time=? WHERE task_id=?;");
    try {
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
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET stop_time=? WHERE task_id=?;");
    try {
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
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=4 WHERE task_id=?;");
    try {
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusRunning(const QString & taskId) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=1 WHERE task_id=?;");
    try {
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusStopped(const QString & taskId) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=2 WHERE task_id=?;");
    try {
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

void DataBase::updateTaskStatusFinished(const QString & taskId) const
{
    const sqlite3_wrap::Sqlite3Command cmd(*mDB, "UPDATE scan_task SET task_status=3 WHERE task_id=?;");
    try {
        cmd.bind(1, taskId);
        cmd.execute();
    }
    catch (std::exception& e) {
        qWarning() << __FUNCTION__ << __LINE__ << "Failed to update scan_task: " << e.what();
    }
}

DataBase::DataBase(QObject* parent)
    : QObject(parent), mDB(std::make_shared<sqlite3_wrap::Sqlite3>())
{
}
