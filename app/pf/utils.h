//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_UTILS_H
#define andsec_scanner_UTILS_H
#include <QString>


class Utils
{
public:
    static QString getProcNameByPid(int pid);
    static qint64 getFileSize(const QString& path);
    static QString formatPath(const QString& path);
    static QString getFileMD5(const QString& path);
    static QString getFileName(const QString& filePath);
    static QString simpleToTradition(const QString& str);
    static QString getFileExtName(const QString& filePath);
    static int getPidByProcName(const QString& procName);
    static bool checkProcLibraryExists (int pid, const char* libraryPath);
private:
    Utils(){}
    Utils(const Utils&) {};
};



#endif // andsec_scanner_UTILS_H
