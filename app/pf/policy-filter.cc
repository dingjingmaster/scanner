//
// Created by dingjing on 2/26/25.
//

#include "policy-filter.h"

#include <QFile>
#include <QDebug>
#include <QCryptographicHash>

#include "utils.h"
#include "data-base.h"
#include "gen-event.h"
#include "task-manager.h"
#include "../macros/macros.h"


PolicyFilter::PolicyFilter(int argc, char ** argv)
    : QCoreApplication(argc, argv), mTimer(new QTimer(this)),
        mPolicyDir(QString("%1/scan").arg(INSTALL_PATH))
{
    DataBase::getInstance().initDB();

    /**
     * @TODO:// 是否把定时器修改为IPC机制，这样节省性能
     */
    connect(mTimer, &QTimer::timeout, this, [this]() {
        QString taskId;
        bool hasError = false;
        bool isUpdateTask = false;
        const auto dirs = mPolicyDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const auto &f : dirs) {
            if (f.startsWith("scan-task-")) {
                QString policyFile = Utils::formatPath(QString("%1/%2")
                    .arg(mPolicyDir.absolutePath()).arg(f));
                taskId = TaskManager::getInstance()->parseTaskId(policyFile);
                if (checkFileNeedParse(policyFile)) {
                    qInfo() << "need parse!";
                    isUpdateTask = true;
                    if (TaskManager::getInstance()->parseScanTask(policyFile)) {
                        auto task = TaskManager::getInstance()->getTaskById(taskId);
                        auto scanTask = dynamic_cast<ScanTask*>(task.get());
                        scanTask->initRun();
                        updatePolicyFile(policyFile);
                        qInfo() << "Success parse scan task: " << policyFile;
                    }
                    else {
                        hasError = true;
                        qCritical() << "Failed to parse scan task: " << policyFile;
                    }
                }
                else {
                    hasError = true;
                    qCritical() << "Failed to parse scan task: " << policyFile;
                }
            }
            else {
                hasError = true;
                qWarning() << "Unrecognized policy file: " << f;
            }
        }

        if (isUpdateTask && !taskId.isEmpty()) {
            auto ids = DataBase::getInstance().queryTaskIds();
            for (auto& id : ids) {
                if (id.isEmpty() || id.isNull()) { continue; }
                if (id == taskId) {
                    qInfo () << "Current task id: " << id;
                    continue;
                }
                qInfo() << "DEL old task: " << id;
                auto task = TaskManager::getInstance()->getTaskById(taskId);
                auto scanTask = dynamic_cast<ScanTask*>(task.get());
                scanTask->stopRun();
                TaskManager::getInstance()->removeScanTask(id);
            }
        }

        // 处理错误数据
        if (hasError) {
            auto ids = DataBase::getInstance().queryTaskIds();
            for (auto& id : ids) {
                if (id.isEmpty() || id.isNull()) { continue; }
                if (id == taskId) {
                    qInfo () << "Current task id: " << id;
                    continue;
                }
                qInfo() << "DEL old task: " << id;
                TaskManager::getInstance()->removeScanTask(id);
            }
        }

        // 开始运行
        TaskManager::getInstance()->startRunTaskAll();
    });
    mTimer->setInterval(1000 * 5);
}

void PolicyFilter::start() const
{
    mTimer->start();
}

bool PolicyFilter::checkFileNeedParse(const QString & filePath) const
{
    C_RETURN_VAL_IF_OK(filePath.isNull() || filePath.isEmpty(), false);

    if (mPolicyFile.contains(filePath)) {
        const QString md5 = Utils::getFileMD5(filePath);
        C_RETURN_VAL_IF_OK(md5.isEmpty() || mPolicyFile[filePath] == md5, false);
    }

    return true;
}

void PolicyFilter::updatePolicyFile(const QString & filePath)
{
    C_RETURN_IF_OK(filePath.isNull() || filePath.isEmpty());

    mPolicyFile[filePath] = Utils::getFileMD5(filePath);
}

