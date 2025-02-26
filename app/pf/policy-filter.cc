//
// Created by dingjing on 2/26/25.
//

#include "policy-filter.h"

#include "data-base.h"
#include "task-manager.h"


PolicyFilter::PolicyFilter(int argc, char ** argv)
    : QCoreApplication(argc, argv)
{
    DataBase::getInstance().initDB();

    // TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-195170debff1mkl6ft6n");
    // TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-19517dced85e88mji6qh");
    // TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-1951bccedd4lgrc94odb");
    // TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-1951bf89fe816mkpt3hq");
}
