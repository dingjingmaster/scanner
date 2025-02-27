//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_DATA_BASE_H
#define andsec_scanner_DATA_BASE_H
#include <QObject>
#include <QDateTime>

#include "../sqlite3-wrap/src/sqlite3-wrap.h"

/**
 * @brief 扫描任务和扫描结果
 */
class DataBase final : public QObject
{
    Q_OBJECT
public:
    static DataBase& getInstance();
    DataBase(DataBase &&) = delete;
    DataBase(DataBase const &) = delete;
    ~DataBase() override;

    /**
     * @brief 初始化数据库
     */
    void initDB() const;

    void insertTask(const QString& taskId, const QString& taskName,
                    const QString& scanDir, const QString& scanDirExp,
                    const QString& scanExt, const QString& scanExtExp,
                    int taskStatus, int scanMode) const;
    void updateTotalFile(const QString& taskId, qint64 totalFile) const;
    void updateFinishedFile(const QString& taskId, qint64 finishedFile) const;
    void updateStartTime(const QString& taskId, const QDateTime& startTime) const;
    void updateStopTime(const QString& taskId, const QDateTime& stopTime) const;
    void updateTaskStatusPause(const QString& taskId) const;
    void updateTaskStatusRunning(const QString& taskId) const;
    void updateTaskStatusStopped(const QString& taskId) const;
    void updateTaskStatusFinished(const QString& taskId) const;

    // task table
    void createTaskTable(const QString& taskId) const;
    bool get100FileByTaskId(const QString& taskId, QMap<QString, QString> files) const;
    bool checkTaskTableFileExists(const QString& taskId, const QString& filePath) const;
    void updateTaskTable(const QString& taskId, const QString& filePath, const QString& md5) const;

private:
    explicit DataBase(QObject *parent = nullptr);
private:
    static DataBase                         gInstance;
    std::shared_ptr<sqlite3_wrap::Sqlite3>  mDB;
};



#endif // andsec_scanner_DATA_BASE_H
