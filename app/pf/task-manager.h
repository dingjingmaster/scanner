//
// Created by dingjing on 2/18/25.
//

#ifndef andsec_scanner_TASK_MANAGER_H
#define andsec_scanner_TASK_MANAGER_H
#include <QObject>
#include "task-base.h"


class TaskManagerPrivate;
class TaskManager final : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskManager)
public:
    static TaskManager* getInstance();
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    std::shared_ptr<TaskBase> getTaskById(const QString& id);

    bool parseScanTask(const QString &scanTask);
    QString parseTaskId(const QString &scanTask);
    void removeScanTask(const QString &scanTaskId);
    void startScanTask(std::shared_ptr<TaskBase> task);
    static QString getTaskIdByPolicyFile(const QString & policyFile);

    void startRunTaskAll();
    void startScanTask(const QString &scanTaskId);
    void stopScanTask(const QString &scanTaskId);
    void pauseScanTask(const QString &scanTaskId);

private:
    TaskManager();
    ~TaskManager() override = default;

private:
    TaskManagerPrivate*             d_ptr;
    static TaskManager              gInstance;
};


#endif // andsec_scanner_TASK_MANAGER_H
