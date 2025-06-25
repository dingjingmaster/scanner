//
// Created by dingjing on 2/26/25.
//

#include "policy-filter.h"

#include <QFile>
#include <QDebug>
#include <QCryptographicHash>

#include "utils.h"
#include "data-base.h"
#include "task-manager.h"
#include "../macros/macros.h"


PolicyFilter::PolicyFilter(int argc, char ** argv)
    : QCoreApplication(argc, argv), mTimer(new QTimer(this)), mPolicyDir(QString("%1/scan").arg(INSTALL_PATH))
{
    DataBase::getInstance().initDB();

    /**
     * @TODO:// 是否把定时器修改为IPC机制，这样节省性能
     */
    connect(mTimer, &QTimer::timeout, this, [this]() {
        const auto dirs = mPolicyDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const auto &f : dirs) {
            if (f.startsWith("scan-task-")) {
                QString policyFile = Utils::formatPath(QString("%1/%2").arg(mPolicyDir.absolutePath()).arg(f));

                // TODO://如果是空文件，则删除此任务相关的所有数据

                if (checkFileNeedParse(policyFile)) {
                    if (TaskManager::getInstance()->parseScanTask(policyFile)) {
                        // TODO:// 换个地方保存???
                        updatePolicyFile(policyFile);
                        qInfo() << "Success parse scan task: " << policyFile;
                        TaskManager::getInstance()->startScanTask(TaskManager::getInstance()->parseTaskId(policyFile));
                    }
                    else {
                        qCritical() << "Failed to parse scan task: " << policyFile;
                    }
                }
            }
            else {
                qWarning() << "Unrecognized policy file: " << f;
            }
        }

        // TODO://删除不需要的任务，后续如果下发内容改变，则清空旧的文件，此处解析到后要删除旧任务相关数据
        for (const auto &f : mPolicyFile.keys()) {
            if (!QFile::exists(f)) {
                TaskManager::getInstance()->stopScanTask(TaskManager::getTaskIdByPolicyFile(f));
                TaskManager::getInstance()->removeScanTask(TaskManager::getTaskIdByPolicyFile(f));
            }
        }
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

