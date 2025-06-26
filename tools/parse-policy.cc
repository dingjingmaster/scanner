//
// Created by dingjing on 25-6-26.
//

#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>


int main (int argc, char* argv[])
{
    if (argc != 2) {
        qInfo() << "Usage: " << argv[1] << " <policy file>";
        return 0;
    }

    QFile file(argv[1]);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "open file: " << argv[1] << "err!";
        return -1;
    }
    const auto json = QJsonDocument::fromJson(file.readAll());
    file.close();

    const auto dataTask = json["dataDiscoveryTask"];
    if (!dataTask.isNull()) {
        const auto dlpContentDataList = json["dlpContentDataList"];
        if (!dlpContentDataList.isNull()) {
            qInfo() << "" \
                << "Task ID                  : " << dataTask["taskId"].toString() << "\n" \
                << "Task Name                : " << dataTask["taskName"].toString() << "\n" \
                << "Task Status              : " << dataTask["taskStatus"].toString() << "\n" \
                << "Scan path                : " << dataTask["scanPath"].toString() << "\n" \
                << "Result Reuse             : " << dataTask["reuseFlag"].toInt() << "\n" \
                << "Policy ID                : " << dataTask["policyIdList"].toString().split(",");
            qInfo() << "";
            if (dlpContentDataList.isArray()) {
                const auto arr = dlpContentDataList.toArray();
                for (auto obj : arr) {
                    qInfo() << "" \
                << "  Rule ID                : " << obj["id"].toString() << "\n" \
                << "  Rule Name              : " << obj["name"].toString() << "\n" \
                << "  Order Num              : " << obj["order_num"].toInt();
                    const auto ruleList = obj["ruleList"];
                    if (ruleList.isArray()) {
                        const auto rules = ruleList.toArray();
                        for (auto rule : rules) {
                            const auto isExcept = rule["exception_flag"].toInt();
                            const auto ruleParam = rule["ruleParam"].toObject();
                            const auto clueList = rule["clueItemDataList"].toArray();
                            qInfo() << "" \
                << "    Rule ID              : " << rule["rule_id"].toString();
                            qInfo() << "" \
                << "    Type                 : " << rule["clue_key"].toString();
                            if (ruleParam.contains("exactMatch")) {
                                qInfo() << "" \
                << "    Exact Match          : " << ruleParam["exactMatch"].toBool();
                            }
                            if (ruleParam.contains("ignoreCase")) {
                                qInfo() << "" \
                << "    Ignore Case          : " << ruleParam["ignoreCase"].toBool();
                            }
                            if (ruleParam.contains("ignoreConfuse")) {
                                qInfo() << "" \
                << "    Ignore Confuse       : " << ruleParam["ignoreConfuse"].toBool();
                            }
                            if (ruleParam.contains("ignoreZhTw")) {
                                qInfo() << "" \
                << "    Ignore ZhTw          : " << ruleParam["ignoreZhTw"].toBool();
                            }
                            if (ruleParam.contains("minMatchItems")) {
                                qInfo() << "" \
                << "    Min Match Items      : " << ruleParam["minMatchItems"].toInt();
                            }
                            if (ruleParam.contains("minMatchItemsScore")) {
                                qInfo() << "" \
                << "    Min Match Items Score: " << ruleParam["minMatchItemsScore"].toInt();
                            }
                            if (ruleParam.contains("totalScore")) {
                                qInfo() << "" \
                << "    Total Score          : " << ruleParam["totalScore"].toInt();
                            }
                            if (ruleParam.contains("weightFlag")) {
                                qInfo() << "" \
                << "    Weight Flag          : " << ruleParam["weightFlag"].toInt();
                            }
                            if (ruleParam.contains("wildcard")) {
                                qInfo() << "" \
                << "    Wildcard             : " << ruleParam["wildcard"].toBool();
                            }
                            QStringList ruleT;
                            for (auto c : clueList) {
                                ruleT << c.toObject()["itemValue"].toString();
                            }
                            if (!isExcept) {
                                qInfo() << "" \
                << "    Rule List            : " << ruleT;
                            }
                            else {
                                qInfo() << "" \
                << "    Rule List(N)         : " << ruleT;
                            }
                            qInfo() << "";
                        }
                        qInfo() << "";
                    }
                }
            }
        }
    }



    return 0;
}
