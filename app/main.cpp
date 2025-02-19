//
// Created by dingjing on 2/17/25.
//

#include "pf/task-manager.h"

int main (int argc, char* argv[])
{
    TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-195170debff1mkl6ft6n");
    TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-19517dced85e88mji6qh");
    TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-1951bccedd4lgrc94odb");
    TaskManager::getInstance()->parseScanTask("/usr/local/andsec/scan/scan-task-1951bf89fe816mkpt3hq");

    return 0;
}