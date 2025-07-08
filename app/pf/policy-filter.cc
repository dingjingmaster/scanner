//
// Created by dingjing on 2/26/25.
//

#include "policy-filter.h"

#include <QFile>
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonDocument>

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
        const auto dirs = mPolicyDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const auto &f : dirs) {
            if (f.startsWith("scan-task-")) {
                QString policyFile = Utils::formatPath(QString("%1/%2")
                    .arg(mPolicyDir.absolutePath()).arg(f));
                if (checkFileNeedParse(policyFile)) {
                    qInfo() << "need parse!";
                    if (TaskManager::getInstance()->parseScanTask(policyFile)) {
                        qInfo() << "Success parse scan task: " << policyFile;
                    }
                    else {
                        qCritical() << "Failed to parse scan task: " << policyFile;
                    }
                }
                else {
                    qCritical() << "Not need to parse scan task: " << policyFile;
                }
            }
        }

        // 处理错误数据
        {
            auto ids = DataBase::getInstance().queryTaskIds();
            for (auto& id : ids) {
                if (id.isEmpty() || id.isNull()) { continue; }
                // 判断是否是当前的扫描任务
                if (!TaskManager::getInstance()->isValidScanTaskId(id)) {
                    qInfo() << "DEL old task: " << id;
                    TaskManager::getInstance()->removeScanTask(id);
                }
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

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        const auto json = QJsonDocument::fromJson(file.readAll());
        file.close();

        const auto dataTask = json["dataDiscoveryTask"];
        if (!dataTask.isNull()) {
            const auto dlpContentDataList = json["dlpContentDataList"];
            if (!dlpContentDataList.isNull()) {
                const auto taskId = dataTask["taskId"].toString();
                const auto times = dataTask["times"].toInt();
                const auto dbTimes = DataBase::getInstance().getTimes(taskId);
                auto task = TaskManager::getInstance()->getTaskById(taskId);
                C_RETURN_VAL_IF_FAIL(task, true);
                auto scanTask = dynamic_cast<ScanTask*>(task.get());
                C_RETURN_VAL_IF_FAIL(scanTask, true);
                qInfo() << "times: " << times << ", taskId: " << taskId << ", times: " << scanTask->getTimes() << ", dbTimes: " << dbTimes;
                if (times != dbTimes) {
                    scanTask->setTimes(times);
                    scanTask->taskForceReload();
                    return true;
                }
            }
        }
    }

    return false;
}

void PolicyFilter::updatePolicyFile(const QString & filePath, const QString & md5)
{
    C_RETURN_IF_OK(filePath.isNull() || filePath.isEmpty());

    mPolicyFile[filePath] = md5;
}

