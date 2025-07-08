//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_POLICY_FILTER_H
#define andsec_scanner_POLICY_FILTER_H
#include <QDir>
#include <QMap>
#include <QTimer>
#include <QCoreApplication>


class PolicyFilter final : public QCoreApplication
{
    Q_OBJECT
public:
    explicit PolicyFilter(int argc, char** argv);

    void start() const;

private:
    bool checkFileNeedParse(const QString &filePath) const;
    void updatePolicyFile(const QString &filePath, const QString& md5);

Q_SIGNALS:
    void scanStart(const QString& taskId, QPrivateSignal);

private:
    QTimer*                             mTimer;
    QDir                                mPolicyDir;
    QMap<QString, QString>              mPolicyFile;
};



#endif // andsec_scanner_POLICY_FILTER_H
