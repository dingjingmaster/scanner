//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_DATA_BASE_H
#define andsec_scanner_DATA_BASE_H
#include <QSet>
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
    void updateFinishedFileAdd(const QString& taskId, qint64 finishedFile) const;
    void updateStartTime(const QString& taskId, const QDateTime& startTime) const;
    void updateStopTime(const QString& taskId, const QDateTime& stopTime) const;
    void updateTaskStatusPause(const QString& taskId) const;
    void updateTaskStatusRunning(const QString& taskId) const;
    void updateTaskStatusStopped(const QString& taskId) const;
    void updateTaskStatusFinished(const QString& taskId) const;

    // 保存 rule_id
    void updateRuleId (const QString& ruleId) const;
    bool checkRuleIdExists(const QString& ruleId) const;
    void showRuleId () const;

    // show scanTask
    void showScanTask() const;

    // scan result
    bool checkScanResultItemExists(const QString& filePath) const;
    QPair<QString, QString> getScanResultPolicyIdAndMd5(const QString& filePath) const;
    void updateScanResultItems(const QString& filePath, const QString& ruleId, const QString& fileType, const QString& content) const;

    // 扫描任务临时文件
    bool checkTempTaskFileExist(const QString& taskId) const;
    void saveTempTaskFileFirst(const QString& taskId, const QSet<QString>& files) const;
    void loadTempTaskFile(const QString& taskId, QSet<QString>& filesForScan, QSet<QString>& filesScanned) const;
    void updateTempTaskFile(const QString& taskId, const QSet<QString>& filesForScan, QSet<QString>& filesScanned) const;
    // bool get100FileByTaskId(const QString& taskId, QMap<QString, QString>& files) const;

    // task table
    void createPolicyIdTable() const;
    QString getTaskFileMd5(const QString& taskId, const QString& filePath) const; // 获取已保存的md5, 根据结果中的策略id检查是否无须扫描
    // bool checkTaskTableFileExists(const QString& taskId, const QString& filePath) const;
    // void insertTaskTable(const QString& taskId, const QString& filePath, const QString& md5) const;
    // void updateTaskTable(const QString& taskId, const QString& filePath, const QString& md5, bool isFinished) const;

private:
    explicit DataBase(QObject *parent = nullptr);

private:
    static DataBase                         gInstance;
    std::shared_ptr<sqlite3_wrap::Sqlite3>  mDB;
};



#endif // andsec_scanner_DATA_BASE_H
