//
// Created by dingjing on 2/26/25.
//

#include "utils.h"

#include <QFile>
#include <QCryptographicHash>

#include "../macros/macros.h"


QString Utils::formatPath(const QString & path)
{
    QString res = path;
    if (res.isNull() || res.isEmpty()) {
        return res;
    }

    while (res.contains("//")) {
        res = res.replace("//", "/");
    }

    if (res.endsWith("/")) {
        res.chop(1);
    }

    return res;
}

QString Utils::getFileMD5(const QString & filePath)
{
    C_RETURN_VAL_IF_OK(filePath.isNull() || filePath.isEmpty() || !QFile::exists(filePath), "");

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(&file);
        file.close();
        return hash.result().toHex();
    }

    return "";
}
