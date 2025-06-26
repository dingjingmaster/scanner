//
// Created by dingjing on 2/26/25.
//

#include "utils.h"

#include <QSet>
#include <QFile>
#include <QMutex>
#include <opencc.h>
#include <QCryptographicHash>

#include "../macros/macros.h"


static const char* sDict[] = {
    ".tar.bz2",
    ".tar.gz",
    ".tar.xz",
    ".conf",
    ".docx",
    ".html",
    ".java",
    ".pptx",
    ".xlsx",
    ".bz2",
    ".cfg",
    ".cpp",
    ".csv",
    ".doc",
    ".deb",
    ".exe",
    ".gif",
    ".jar",
    ".htm",
    ".iso",
    ".jpg",
    ".hpp",
    ".ini",
    ".log",
    ".odp",
    ".odt",
    ".ods",
    ".pdf",
    ".png",
    ".ppt",
    ".rar",
    ".rpm",
    ".run",
    ".tar",
    ".tbz",
    ".tgz",
    ".txt",
    ".tmp",
    ".wav",
    ".wps",
    ".wps",
    ".xls",
    ".xpm",
    ".zip",
    ".bz",
    ".cc",
    ".gz",
    ".ko",
    ".py",
    ".ps",
    ".sh",
    ".so",
    ".xz",
    ".c",
    ".h",
    ".a",
    ".o",
    nullptr
};
static QSet<QString> sDictIdx;

static void initFileExtName()
{
    static qint64 init = 0;
    C_RETURN_IF_FAIL(init);

    static QMutex lock;
    lock.lock();
    for (int i = 0; sDict[i]; ++i) {
        sDictIdx << sDict[i];
    }
    lock.unlock();
}

qint64 Utils::getFileSize(const QString& path)
{
    const QFile f(path);
    return f.size();
}

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
        while (!file.atEnd()) {
            QByteArray data = file.read(40960);
            hash.addData(data);
        }
        file.close();
        return hash.result().toHex();
    }

    return "";
}

QString Utils::simpleToTradition(const QString& str)
{
    const opencc::SimpleConverter cov("s2t.json");

    return cov.Convert(str.toUtf8().toStdString()).data();
}

QString Utils::getFileExtName(const QString& filePath)
{
    initFileExtName();

    const auto arr = filePath.split("/");
    const auto& name = arr.last();
    QStringList extArr = name.split(".");
    extArr.pop_front();
    QString extName = QString(".%1").arg(extArr.join("."));

    if (extName.size() > 9) {
        if (!sDictIdx.contains(extName)) {
            for (int i = 0; sDict[i]; ++i) {
                if (name.endsWith(sDict[i])) {
                    extName = sDict[i];
                }
            }
        }
    }

    return extName;
}