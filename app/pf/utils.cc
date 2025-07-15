//
// Created by dingjing on 2/26/25.
//

#include "utils.h"

#include <QSet>
#include <QFile>
#include <QMutex>
#include <opencc.h>
#include <QCryptographicHash>

#include "proc-inject.h"
#include "../macros/macros.h"
#include "../common/proc-list.h"


struct ProcName
{
    pid_t       pid;
    QString     name;
};

static bool proc_pid_name (pid_t pid, const char* procPath, int uid, bool isGui, void* data);
static bool check_is_dynamic_library (const char* path);

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

QString Utils::getProcNameByPid(int pid)
{
    const QString exe = QString("/proc/%1/exe").arg(pid);
    if (QFile::exists(exe)) {
        const QFile f(exe);
        const QString target = f.symLinkTarget();
        if (!target.isNull() && !target.isEmpty()) {
            return QFile(target).fileName();
        }
    }

    return "";
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

QString Utils::getFileName(const QString& filePath)
{
    const auto arr = filePath.split("/");
    return arr.last();
}

QString Utils::simpleToTradition(const QString& str)
{
    const opencc::SimpleConverter cov("s2t.json");

    return cov.Convert(str.toUtf8().toStdString()).data();
}

QString Utils::getFileExtName(const QString& filePath)
{
    initFileExtName();

    const auto& name = Utils::getFileName(filePath);
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

    return (extName.size() > 1) ? extName : "";
}

int Utils::getPidByProcName(const QString& procName)
{
    struct ProcName procNameS = {
        .pid = 0,
        .name = QString("/%1").arg(procName),
    };

    proc_list_all(proc_pid_name, &procNameS);

    return procNameS.pid;
}

bool Utils::checkProcLibraryExists(int pid, const char* libraryPath)
{
    FILE* fr = nullptr;
    bool ret = false;
    char mapsPath[128] = {0};
    char lineBuf[1024] = {0};

    snprintf(mapsPath, sizeof(mapsPath) - 1, "/proc/%d/maps", pid);

    fr = fopen(mapsPath, "r");
    if (fr) {
        while (fgets(lineBuf, sizeof(lineBuf), fr)) {
            lineBuf[strcspn(lineBuf, "\n")] = 0;

            // /proc/self/maps 格式：
            // 地址范围 权限 偏移量 设备 inode 路径名
            // 例如：7f1234567000-7f1234568000 r-xp 00000000 08:01 1234567 /lib/x86_64-linux-gnu/libc.so.6
            char* tokens[8] = { nullptr };
            char* token = strtok(lineBuf, " ");
            int tokenCount = 0;

            // 分割行内容
            while (token && tokenCount < 6) {
                tokens[tokenCount++] = token;
                token = strtok(nullptr, " ");
            }

            // 如果有路径字段(第6列)
            if (tokenCount >= 6) {
                char* path = tokens[5];
                // 跳过剩余的空格，获取完整的路径
                while (*path == ' ') { path++; }
                // 检查是否是动态库
                if (check_is_dynamic_library(path) && 0 == c_strcmp(libraryPath, path)) {
                    ret = true;
                    break;
                }
            }
        }
        fclose(fr);
    }

    return ret;
}

static bool check_is_dynamic_library (const char* path)
{
    C_RETURN_VAL_IF_FAIL(path && strlen(path) > 0, false);

    const char* ext = strrchr(path, '.');
    C_RETURN_VAL_IF_OK(ext && 0 == c_strcmp(ext, ".so"), true);
    C_RETURN_VAL_IF_OK(strstr(path, ".so."), true);

    return false;
}

static bool proc_pid_name (pid_t pid, const char* procPath, int uid, bool isGui, void* data)
{
    struct ProcName* procName = static_cast<struct ProcName*>(data);

    if (QString(procPath).endsWith(procName->name)) {
        procName->pid = pid;
        return false;
    }

    return true;
}
