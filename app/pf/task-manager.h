//
// Created by dingjing on 2/18/25.
//

#ifndef andsec_scanner_TASK_MANAGER_H
#define andsec_scanner_TASK_MANAGER_H
#include <QObject>


class TaskManagerPrivate;
class TaskManager final : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskManager)
public:
    static TaskManager* getInstance();
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    bool parseScanTask(const QString &scanTask);

private:
    TaskManager();
    ~TaskManager() override = default;

private:
    TaskManagerPrivate*             d_ptr;
    static TaskManager              gInstance;
};


#endif // andsec_scanner_TASK_MANAGER_H
