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
    "   `is_finished`                   TINYINT         DEFAULT 0       NOT NULL," \
    "   PRIMARY KEY (file_path)" \
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

DataBase::DataBase(QObject* parent)
    : QObject(parent), mDB(std::make_shared<sqlite3_wrap::Sqlite3>())
{
}
