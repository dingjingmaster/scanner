//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_DATA_BASE_H
#define andsec_scanner_DATA_BASE_H
#include <QSet>
#include <memory>
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
    enum TaskStatus
    {
        TASK_STATUS_UNKNOWN = 0,
        TASK_STATUS_RUNNING = 1,
        TASK_STATUS_STOPPED = 2,
        TASK_STATUS_FINISHED = 3,
        TASK_STATUS_PAUSE = 4,
    };
    static DataBase& getInstance();
    DataBase(DataBase &&) = delete;
    DataBase(DataBase const &) = delete;
    ~DataBase() override;

    /**
     * @brief 初始化数据库
     */
    void initDB() const;

    bool checkTaskExists(const QString& taskId) const;

    void insertTask(const QString& taskId, const QString& taskName,
                    const QString& scanDir, const QString& scanDirExp,
                    const QString& scanExt, const QString& scanExtExp,
                    const QString& schedulingCron,
                    const QStringList& policyId,
                    int taskStatus, int scanMode,
                    int times,
                    bool isScheduled=false) const;
    void updateTotalFile(const QString& taskId, qint64 totalFile) const;
    void updateFinishedFile(const QString& taskId, qint64 finishedFile) const;
    void updateFinishedFileAdd(const QString& taskId, qint64 finishedFile) const;
    void updateStartTime(const QString& taskId, const QDateTime& startTime) const;
    void updateStopTime(const QString& taskId, const QDateTime& stopTime) const;

    void updateTaskStatusPause(const QString& taskId) const;
    void updateTaskStatusRunning(const QString& taskId) const;
    void updateTaskStatusStopped(const QString& taskId) const;
    void updateTaskStatusFinished(const QString& taskId) const;

    // scan_task
    qint64 getFinishedFileNum(const QString& taskId) const;
    qint64 getTotalFileNum(const QString& taskId) const;
    bool getIsScheduled(const QString& taskId) const;
    enum TaskStatus getTaskStatus(const QString& taskId) const;
    bool checkNeedRun(const QString& taskId) const;

    // 获取所有TaskId
    QStringList queryTaskIds() const;
    int getExecTimes(const QString& taskId) const;
    int getTimes(const QString& taskId) const;
    void setTimes(const QString& taskId, int times) const;

    // 删除扫描任务，标记已经删除的任务Id
    void deleteTask(const QString& taskId) const;

    // 保存 rule_id
    void updateRuleId (const QString& ruleId) const;
    void updateRuleIdDirty (const QString& ruleId) const;
    bool checkRuleIdExists(const QString& ruleId) const;
    void showRuleId () const;

    // show scanTask
    void showScanTask() const;

    // scan result
    void showScanResult() const;
    bool checkScanResultItemExists(const QString& filePath) const;
    QPair<QString, QString> getScanResultPolicyIdAndMd5(const QString& filePath) const;
    void updateScanResultItems(const QString& filePath, const QString& ruleId, const QString& fileType, const QString& content) const;

    // 扫描任务临时文件
    bool checkTempTaskFileExist(const QString& taskId) const;
    void deleteTempTaskFile(const QString& taskId) const;
    void saveTempTaskFileFirst(const QString& taskId, const QSet<QString>& files) const;
    void loadTempTaskFile(const QString& taskId, QSet<QString>& filesForScan, QSet<QString>& filesScanned) const;
    void updateTempTaskFile(const QString& taskId, const QSet<QString>& filesForScan, QSet<QString>& filesScanned) const;

    // task table
    void createPolicyIdTable() const;
    QString getTaskFileMd5(const QString& taskId, const QString& filePath) const; // 获取已保存的md5, 根据结果中的策略id检查是否无须扫描

private:
    explicit DataBase(QObject *parent = nullptr);

private:
    static DataBase                         gInstance;
    std::shared_ptr<sqlite3_wrap::Sqlite3>  mDB;
};



#endif // andsec_scanner_DATA_BASE_H
