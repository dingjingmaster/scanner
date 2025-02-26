//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_UTILS_H
#define andsec_scanner_UTILS_H
#include <QString>



class Utils
{
public:
    static QString formatPath(const QString& path);
    static QString getFileMD5(const QString& path);
private:
    Utils(){}
    Utils(const Utils&) {};
};



#endif // andsec_scanner_UTILS_H
