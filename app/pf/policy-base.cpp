//
// Created by dingjing on 2/17/25.
//

#include "policy-base.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <macros/macros.h>

#include "data-base.h"
#include "regex-matcher.h"
#include "utils.h"


RuleBase::RuleBase(const QString& ruleId, RuleType type)
    : mRuleId(ruleId), mType(type)
{
}

RuleBase::~RuleBase()
{
}

RuleType RuleBase::getRuleType()
{
    return mType;
}

void RuleBase::setExactMatch(bool exactMatch)
{
    mExactMatch = exactMatch;
}

bool RuleBase::getExactMatch() const
{
    return mExactMatch;
}

const QString & RuleBase::getRuleId() const
{
    return mRuleId;
}

void RuleBase::parseRule(const QJsonValue & rule)
{
    qWarning() << "RuleBase::parseRule, This is Not supported, Please check you code!";
}

bool RuleBase::matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath)
{
    return false;
}

void RuleBase::saveResult(const QString& filePath, const QString& metaPath, const QString& context)
{
    QString fileType = "";
    QFile f(metaPath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!f.atEnd()) {
            QString line = f.readLine();
            if (line.startsWith("Content-Type")) {
                const auto arr = line.split("{]");
                if (arr.length() != 2) {
                    continue;
                }
                fileType = arr[1].trimmed();
            }
        }
    }

    // 保存扫描结果
    DataBase::getInstance().updateScanResultItems(filePath, getRuleId(), fileType, context);
}

QString RuleBase::getTaskTypeString() const
{
    switch (mType) {
        case RuleType::GeneralFileAttribute: {
            return "FileAttribute";
        }
        case RuleType::GeneralFileFingerprint: {
            return "FileFingerprint";
        }
        case RuleType::GeneralKeyword: {
            return "Keyword";
        }
        case RuleType::GeneralRegular: {
            return "Regular";
        }
    }
    return "Unknown";
}

IdmRule::IdmRule(const QString & id)
    : RuleBase(id, RuleType::GeneralFileFingerprint)
{
}

void IdmRule::parseRule(const QJsonValue& rule)
{
    const auto clueItemDataList = rule["clueItemDataList"].toArray();
    C_RETURN_IF_OK(clueItemDataList.isEmpty());
    for (const auto& clueItemData : clueItemDataList) {
        const auto itemVal = clueItemData["itemValue"].toString();
        const auto weight = clueItemData["weight"].toInt();
        addIdmAndWeight(itemVal, weight);
    }
}

void IdmRule::addIdmAndWeight(const QString & idm, int weight)
{
    C_RETURN_IF_OK(idm.isNull() || idm.isEmpty() || weight < 0);

    mIsCheckIdm = true;
    mIdmAndWeight[idm] = weight;
}

FileTypeRule::FileTypeRule(const QString & id)
    : RuleBase(id, RuleType::GeneralFileAttribute)
{
}

void FileTypeRule::parseRule(const QJsonValue & rule)
{
    C_RETURN_IF_OK(rule.isNull());

    // const auto ruleParam = rule["ruleParam"].toObject();
    // const auto clueItemDataList = rule["clueItemDataList"].toArray();
    const auto ruleAttrDto = rule["ruleAttrDto"].toObject();
    C_RETURN_IF_OK(ruleAttrDto.isEmpty());

    // qDebug() << "Start parse fileType rule: " << getRuleId();

    const auto detailAttrList = ruleAttrDto["detailAttrList"].toArray();
    const auto typeAttrList = ruleAttrDto["typeAttrList"].toArray();

    for (auto ta : typeAttrList) {
        const auto key = ta["clue_key"].toString("");
        if ("modifytime" == key) {
            const auto val = ta["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                const auto start = v.toObject()["rangeStart"].toString("");
                const auto end = v.toObject()["rangeEnd"].toString("");
                setModifyTimes(start, end);
            }
        }
        else if ("createtime" == key) {
            const auto val = ta["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                const auto start = v.toObject()["rangeStart"].toString("");
                const auto end = v.toObject()["rangeEnd"].toString("");
                setCreateTimes(start, end);
            }
        }
        else if ("filetypes" == key) {
            const auto val = ta["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                const auto vvv = v.toObject()["v"].toString("");
                addFileTypes(vvv);
            }
        }
        else if ("fileName" == key) {
            const auto val = ta["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                const auto vvv = v.toObject()["v"].toString("");
                addFileNames(vvv);
            }
        }
        else if ("filesize" == key) {
            const auto val = ta["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                const auto start = v.toObject()["rangeStart"].toInt();
                const auto end = v.toObject()["rangeEnd"].toInt();
                const auto type = v.toObject()["type"].toInt();
                setFileSize(start, end, type);
            }
        }
        else {
            qWarning() << "Unknown key: " << key;
        }
    }

    for (auto da : detailAttrList) {
        const auto key = da["clue_key"].toString("");
        if ("subject" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailSubject(v.toObject()["v"].toString(""));
            }
        }
        else if ("company" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailCompany(v.toObject()["v"].toString(""));
            }
        }
        else if ("manager" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailManager(v.toObject()["v"].toString(""));
            }
        }
        else if ("tags" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailTags(v.toObject()["v"].toString(""));
            }
        }
        else if ("category" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailCategory(v.toObject()["v"].toString(""));
            }
        }
        else if ("comments" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailComments(v.toObject()["v"].toString(""));
            }
        }
        else if ("preserver" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailPreserver(v.toObject()["v"].toString(""));
            }
        }
        else if ("author" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailAuthor(v.toObject()["v"].toString(""));
            }
        }
        else if ("title" == key) {
            const auto val = da["clue_value"];
            const auto valJson = QJsonDocument::fromJson(val.toString().toUtf8());
            for (auto v : valJson.array()) {
                addDetailTitle(v.toObject()["v"].toString(""));
            }
        }
        else {
            qWarning() << "Unknown key: " << key;
        }
    }
}

bool FileTypeRule::matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath)
{
#define TASK_SCAN_LOG_INFO       qInfo() \
    << "[文件类型]"

    C_RETURN_VAL_IF_FAIL(QFile::exists(filePath) && QFile::exists(metaPath) && QFile::exists(ctxPath), ret);

    QFile file(filePath);
    C_RETURN_VAL_IF_FAIL(file.open(QIODevice::ReadOnly | QIODevice::Text), false);

    bool ret = true;

    // 文件类型
    const QString fileNameExt = Utils::getFileExtName(filePath);
    const QString fileName = Utils::getFileName(filePath).replace(fileNameExt, "");
    ret = mFileTypes.contains(fileNameExt);
    TASK_SCAN_LOG_INFO << "文件类型: " << ret << " (" << fileNameExt << ")";
    C_RETURN_VAL_IF_FAIL(ret, false);

    // 文件名称
    ret = mFileNames.contains(fileName);
    TASK_SCAN_LOG_INFO << "文件类型: " << ret << " (" << fileName << ")";
    C_RETURN_VAL_IF_FAIL(ret, false);

    // 文件大小
    const QFileInfo fi(filePath);
    const qint64 fileSize = fi.size();
    ret = (fileSize >= mMinFileSize && fileSize <= mMaxFileSize);
    TASK_SCAN_LOG_INFO << "文件大小: " << ret << " (" << mMinFileSize << " -- " << fileSize << " -- " << mMaxFileSize << ")";
    C_RETURN_VAL_IF_FAIL(ret, false);

    // 创建时间
    const qint64 fileCreateTime = fi.fileTime(QFileDevice::FileBirthTime).toMSecsSinceEpoch();
    ret = (fileCreateTime >= mCreateTimeStart && fileCreateTime <= mCreateTimeEnd);
    TASK_SCAN_LOG_INFO << "文件创建: " << ret << " (" << mCreateTimeStart << " -- " << fileCreateTime << " -- " << mCreateTimeEnd << ")";
    C_RETURN_VAL_IF_FAIL(ret, false);

    // 修改时间
    const qint64 fileModifyTime = fi.fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch();
    ret = (fileModifyTime >= mModifyTimeStart && fileModifyTime <= mModifyTimeEnd);
    TASK_SCAN_LOG_INFO << "文件修改: " << ret << " (" << mModifyTimeStart << " -- " << fileModifyTime << " -- " << mModifyTimeEnd << ")";
    C_RETURN_VAL_IF_FAIL(ret, false);

    // 详细信息
    {

    }
    C_RETURN_VAL_IF_FAIL(ret, false);

    TASK_SCAN_LOG_INFO << "命中";

#undef TASK_SCAN_LOG_INFO

    return ret;
}

void FileTypeRule::setModifyTimes(const QString & start, const QString & end)
{
    C_RETURN_IF_OK(start.isEmpty() || end.isEmpty());

    mIsCheckModifyTime = true;

    mModifyTimeStart = QDateTime::fromString(start, "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch();
    mModifyTimeEnd = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch();
}

void FileTypeRule::setCreateTimes(const QString & start, const QString & end)
{
    C_RETURN_IF_OK(start.isEmpty() || end.isEmpty());

    mIsCheckCreateTime = true;

    mCreateTimeStart = QDateTime::fromString(start, "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch();
    mCreateTimeEnd = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch();
}

void FileTypeRule::addFileTypes(const QString & fileType)
{
    C_RETURN_IF_OK(fileType.isNull() || fileType.isEmpty() || "" == fileType);

    mIsCheckFileTypes = true;
    mFileTypes << fileType;
}

void FileTypeRule::addFileNames(const QString & fileNames)
{
    C_RETURN_IF_OK(fileNames.isNull() || fileNames.isEmpty() || "" == fileNames);

    mIsCheckFileNames = true;
    mFileNames << fileNames;
}

void FileTypeRule::setFileSize(qint64 start, qint64 end, int type)
{
    C_RETURN_IF_OK(start < 0 || end < 0 || type <= 0);

    mIsCheckFileSize = true;

#define B           2^0
#define K           2^10
#define M           2^20
#define G           2^30

    switch (type) {
        default:
        case 1: {
            start *= B;
            end *= B;
            break;
        }
        case 2: {
            start *= K;
            end *= K;
            break;
        }
        case 3: {
            start *= M;
            end *= M;
            break;
        }
        case 4: {
            start *= G;
            end *= G;
            break;
        }
    }

    mMinFileSize = qMin(start, end);
    mMaxFileSize = qMax(start, end);
}

void FileTypeRule::addDetailSubject(const QString & subject)
{
    C_RETURN_IF_OK(subject.isNull() || subject.isEmpty() || "" == subject);

    mIsCheckDetailSubject = true;
    mDetailSubject << subject;
}

void FileTypeRule::addDetailCompany(const QString & company)
{
    C_RETURN_IF_OK(company.isNull() || company.isEmpty() || "" == company);

    mIsCheckDetailCompany = true;
    mDetailCompany << company;
}

void FileTypeRule::addDetailManager(const QString & manager)
{
    C_RETURN_IF_OK(manager.isNull() || manager.isEmpty() || "" == manager);

    mIsCheckDetailManager = true;
    mDetailManager << manager;
}

void FileTypeRule::addDetailTags(const QString & tags)
{
    C_RETURN_IF_OK(tags.isNull() || tags.isEmpty() || "" == tags);

    mIsCheckDetailTags = true;
    mDetailTags << tags;
}

void FileTypeRule::addDetailCategory(const QString & cate)
{
    C_RETURN_IF_OK(cate.isNull() || cate.isEmpty() || "" == cate);
    mIsCheckDetailCategory = true;
    mDetailCategory << cate;
}

void FileTypeRule::addDetailComments(const QString & comments)
{
    C_RETURN_IF_OK(comments.isNull() || comments.isEmpty() || "" == comments);

    mIsCheckDetailComments = true;
    mDetailComments << comments;
}

void FileTypeRule::addDetailPreserver(const QString & preserver)
{
    C_RETURN_IF_OK(preserver.isNull() || preserver.isEmpty() || "" == preserver);

    mIsCheckDetailPreserver = true;
    mDetailPreserver << preserver;
}

void FileTypeRule::addDetailAuthor(const QString & author)
{
    C_RETURN_IF_OK(author.isNull() || author.isEmpty() || "" == author);

    mIsCheckDetailAuthor = true;
    mDetailAuthor << author;
}

void FileTypeRule::addDetailTitle(const QString & title)
{
    C_RETURN_IF_OK(title.isNull() || title.isEmpty() || "" == title);

    mIsCheckDetailTitle = true;
    mDetailTitle << title;
}

RegexpRule::RegexpRule(const QString & id)
    :RuleBase(id, RuleType::GeneralRegular)
{
}

void RegexpRule::setRegexAndWeight(const QString & regex, int weight)
{
    if (mRegexAndWeight.contains(regex)) {
        mRegexAndWeight[regex] += weight;
    }
    else {
        mRegexAndWeight[regex] = weight;
    }
}

int RegexpRule::getWightByRegex(const QString &regex) const
{
    if (mRegexAndWeight.contains(regex)) {
        return mRegexAndWeight[regex];
    }

    return 0;
}

bool RegexpRule::getIsPreciseSearch() const
{
    return mIsPreciseSearch;
}

void RegexpRule::parseRule(const QJsonValue & rule)
{
    const auto ruleParam = rule["ruleParam"].toObject();
    const auto ruleAttrDto = rule["ruleAttrDto"].toObject();
    const auto clueItemDataList = rule["clueItemDataList"].toArray();

    C_RETURN_IF_OK(ruleParam.isEmpty() || ruleAttrDto.isEmpty() || clueItemDataList.isEmpty());

    // const bool ignoreCase = ruleParam["ignoreCase"].toBool();
    // const bool ignoreConfuse = ruleParam["ignoreConfuse"].toBool();
    // const bool wildcard = ruleParam["wildcard"].toBool();
    // const bool ignoreZhTw = ruleParam["ignoreZhTw"].toBool();
    const int wightFlag = ruleParam["wightFlag"].toInt();
    const int minMatchItems = ruleParam["minMatchItems"].toInt();
    const int minMatchItemsScore = ruleParam["minMatchItemsScore"].toInt();
    const bool exactMatch = ruleParam["exactMatch"].toBool(); // true 匹配到后、显示时候把前后非数字去掉，否则不去掉

#if 0
    qInfo() << "exactMatch          : " << exactMatch;
    qInfo() << "miniMatchItems      : " << minMatchItems;
    qInfo() << "miniMatchItemsScore : " << minMatchItemsScore;
    exit(0);
#endif

    setExactMatch(exactMatch);
    // setWildcard(wildcard);
    // setIgnoreCase(ignoreCase);
    // setIgnoreConfuse(ignoreConfuse);
    // setIgnoreZhTw(ignoreZhTw);
    setRecognitionMode(wightFlag);

    if (getRecognitionMode() == RuleBase::RecognitionTimes) {
        setMinMatchCount(minMatchItems);
    }
    else if (getRecognitionMode() == RuleBase::RecognitionScore) {
        setMinMatchCount(minMatchItemsScore);
    }
    for (auto k : clueItemDataList) {
        const auto kkk = k.toObject()["itemValue"].toString("");
        const auto weight = k.toObject()["weight"].toInt(0);
        setRegexAndWeight(kkk, weight);
    }
}

bool RegexpRule::matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath)
{
#define TASK_SCAN_LOG_INFO       qInfo() \
    << "[正则规则]" \
    << "[精确查找: " << mIsPreciseSearch \
    << " 最小匹配个数: " << getMinMatchCount() \
    << "] "

    bool ret = false;
    C_RETURN_VAL_IF_FAIL(QFile::exists(filePath) && QFile::exists(metaPath) && QFile::exists(ctxPath), ret);

    QFile file(filePath);
    C_RETURN_VAL_IF_FAIL(file.open(QIODevice::ReadOnly | QIODevice::Text), false);

#if 0
    auto regSpecialChar = [=] (const QString& str) -> QString {
        // TODO:// 后续优化性能
        QString str0 = str;
        QString str1 = str0.replace("\\", "\\\\");
        QString str2 = str1.replace("|", "\\|");
        QString str3 = str2.replace("(", "\\(");
        QString str4 = str3.replace(")", "\\)");
        QString str5 = str4.replace("[", "\\[");
        QString str6 = str5.replace("]", "\\]");
        QString str7 = str6.replace("$", "\\$");
        QString str8 = str7.replace("*", "\\*");
        QString str9 = str8.replace("+", "\\+");
        QString str10 = str9.replace(".", "\\.");
        QString str11 = str10.replace("?", "\\?");
        QString str12 = str11.replace("^", "\\^");
        QString str13 = str12.replace("{", "\\{");
        QString str14 = str13.replace("}", "\\}");
        return str14;
    };
#endif

    auto getContent = [=] (const RegexMatcher& rm) -> QString {
        auto iter = rm.getResultIterator();
        while (iter.hasNext()) {
            auto kv = iter.next();
            if (!kv.first.isEmpty() && !kv.second.isEmpty()) {
                return QString("%1{]%2").arg(kv.first).arg(kv.second);
            }
        }
        return "";
    };

    qint64 score = 0;
    qint64 timesScore = 0;
    // QMap<QString, qint64> score;
    for (auto kv = mRegexAndWeight.begin(); kv != mRegexAndWeight.end(); ++kv) {
        RegexMatcher rm (kv.key());
        timesScore += ((rm.getMatchedCount() >= kv.value()) ? 1 : 0);
        score += (kv.value() * rm.getMatchedCount());
        // TODO:// 保存上下文
    }
    file.close();

    // 权重类型
    if (RecognitionScore == mMode) {
        if (score >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[积分权重] matched count: " << score << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            // saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }
    else if (RecognitionTimes == mMode) {
        if (timesScore >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[次数权重] matched count: " << timesScore << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            // saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }
    else {
        if (timesScore >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[关闭权重]matched count: " << timesScore << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            // saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }

#undef TASK_SCAN_LOG_INFO

    return ret;
}

#if 0
void RegexpRule::setIgnoreCase(bool ignoreCase)
{
    mIgnoreCase = ignoreCase;
}

bool RegexpRule::getIgnoreCase() const
{
    return mIgnoreCase;
}

void RegexpRule::setIgnoreConfuse(bool c)
{
    mIgnoreConfuse = c;
}

bool RegexpRule::getIgnoreConfuse() const
{
    return mIgnoreConfuse;
}

void RegexpRule::setIgnoreZhTw(bool c)
{
    mIgnoreZhTw = c;
}

bool RegexpRule::getIgnoreZhTw() const
{
    return mIgnoreZhTw;
}

void RegexpRule::setWildcard(bool c)
{
    mWildcard = c;
}

bool RegexpRule::getWildcard() const
{
    return mWildcard;
}

#endif


void RegexpRule::setRecognitionMode(int m) { mMode = (RecognitionMode)m; }
void RegexpRule::setIsPreciseSearch(bool p)
{
    mIsPreciseSearch = p;
}

RuleBase::RecognitionMode RegexpRule::getRecognitionMode() const
{
    return mMode;
}

void RegexpRule::setMinMatchCount(qint64 c)
{
    mCount = c;
}

qint64 RegexpRule::getMinMatchCount() const
{
    return mCount;
}

void RegexpRule::addCurrentMatchCount(qint64 c)
{
    mCurCount += c;
}

qint64 RegexpRule::getCurrentMatchCount() const
{
    return mCurCount;
}

KeywordRule::KeywordRule(const QString & id)
    : RuleBase(id, RuleType::GeneralKeyword)
{
}

void KeywordRule::setKeywordAndWeight(const QString & keyword, int weight)
{
    if (mKeywordAndWeight.contains(keyword)) {
        mKeywordAndWeight[keyword] += weight;
    }
    else {
        mKeywordAndWeight[keyword] = weight;
    }
}

int KeywordRule::getWightByKeyword(const QString & keyword) const
{
    if (mKeywordAndWeight.contains(keyword)) {
        return mKeywordAndWeight[keyword];
    }

    return 0;
}

bool KeywordRule::matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath)
{
#define TASK_SCAN_LOG_INFO       qInfo() \
    << "[KeywordRule]" \
    << "[ignore case: " << mIgnoreCase \
    << " zhTw: " << mIgnoreZhTw \
    << " ignore confuse: " << mIgnoreConfuse \
    << " wildcard: " << mWildcard \
    << " mincount: " << getMinMatchCount() \
    << "] "

    bool ret = false;
    C_RETURN_VAL_IF_FAIL(QFile::exists(filePath) && QFile::exists(metaPath) && QFile::exists(ctxPath), ret);

    QFile file(filePath);
    C_RETURN_VAL_IF_FAIL(file.open(QIODevice::ReadOnly | QIODevice::Text), false);

#if 0
    auto regSpecialChar = [=] (const QString& str) -> QString {
        // TODO:// 后续优化性能
        QString str0 = str;
        QString str1 = str0.replace("\\", "\\\\");
        QString str2 = str1.replace("|", "\\|");
        QString str3 = str2.replace("(", "\\(");
        QString str4 = str3.replace(")", "\\)");
        QString str5 = str4.replace("[", "\\[");
        QString str6 = str5.replace("]", "\\]");
        QString str7 = str6.replace("$", "\\$");
        QString str8 = str7.replace("*", "\\*");
        QString str9 = str8.replace("+", "\\+");
        QString str10 = str9.replace(".", "\\.");
        QString str11 = str10.replace("?", "\\?");
        QString str12 = str11.replace("^", "\\^");
        QString str13 = str12.replace("{", "\\{");
        QString str14 = str13.replace("}", "\\}");
        return str14;
    };
#endif

    auto getContent = [=] (const RegexMatcher& rm) -> QString {
        auto iter = rm.getResultIterator();
        while (iter.hasNext()) {
            auto kv = iter.next();
            if (!kv.first.isEmpty() && !kv.second.isEmpty()) {
                return QString("%1{]%2").arg(kv.first).arg(kv.second);
            }
        }
        return "";
    };

    auto getRegexpStrByKeyword = [=] (const QString& kw) -> QString {
        QStringList regs;
        regs << kw;

        if (mIgnoreZhTw) {
            QString regTr = Utils::simpleToTradition(kw);
            if (kw != regTr) {
                regs << regTr;
            }
        }

        QStringList ls;
        for (auto& ll : regs) {
            QString reg = ll;
            if (mIgnoreConfuse) {
                auto arr = ll.split("");
                arr.removeAll("");
                reg = arr.join(".{0,15}");
                reg = reg.replace("|", "\\|");
            }
            ls << reg;
        }
        return QString("(%1)").arg(ls.join("|"));
    };

    QStringList ls;
    for (auto& l : mKeywordAndWeight.keys()) {
        QStringList regs;
        regs << l;

        if (mIgnoreZhTw) {
            QString regTr = Utils::simpleToTradition(l);
            if (l != regTr) {
                regs << regTr;
            }
        }

        for (auto& ll : regs) {
            QString reg = ll;
            if (mIgnoreConfuse) {
                auto arr = ll.split("");
                arr.removeAll("");
                reg = arr.join(".{0,15}");
                reg = reg.replace("|", "\\|");
            }
            ls << reg;
        }
    }
    TASK_SCAN_LOG_INFO << "keywords: " << mKeywordAndWeight.keys();

    QList<QString> ctx;
    const QString regStr = QString("(%1)").arg(ls.join("|"));
    TASK_SCAN_LOG_INFO << "keywords regs 2: " << regStr;
    RegexMatcher rm (regStr, !mIgnoreCase);
    rm.match(file);
    file.close();

    // 权重类型
    if (RecognitionScore == mMode) {
        qint64 retTmp = 0;

        QMap<QString, QRegExp> keyReg;
        for (auto& l : mKeywordAndWeight.keys()) {
            keyReg[l] = QRegExp(getRegexpStrByKeyword(l),
                mIgnoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive);
        }

        auto iter = rm.getResultIterator();
        while (iter.hasNext()) {
            auto kv = iter.next();
            if (!kv.first.isEmpty() && !kv.second.isEmpty()) {
                for (auto kr = keyReg.keyValueBegin(); kr != keyReg.keyValueEnd(); ++kr) {
                    if (kr->second.exactMatch(kv.first)) {
                        retTmp += mKeywordAndWeight[kr->first];
                    }
                }
            }
        }

        if (retTmp >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[积分权重] matched count: " << retTmp << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }
    else if (RecognitionTimes == mMode) {
        QMap<QString, qint64> c;
        QMap<QString, QRegExp> keyReg;
        for (auto& l : mKeywordAndWeight.keys()) {
            keyReg[l] = QRegExp(getRegexpStrByKeyword(l),
                mIgnoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive);
            c[l] = 0;
        }

        auto iter = rm.getResultIterator();
        while (iter.hasNext()) {
            auto kv = iter.next();
            if (!kv.first.isEmpty() && !kv.second.isEmpty()) {
                for (auto kr = keyReg.keyValueBegin(); kr != keyReg.keyValueEnd(); ++kr) {
                    if (kr->second.exactMatch(kv.first)) {
                        c[kr->first]++;
                    }
                }
            }
        }
#if 1
        QString debugStr = "";
        for (auto l = c.keyValueBegin(); l != c.keyValueEnd(); l++) {
            debugStr += QString("keyword: %1, count: %2\n").arg(l->first).arg(l->second);
        }
        TASK_SCAN_LOG_INFO << "\n" << debugStr << "\n";
#endif
        qint64 retTmp = 0;
        for (auto kv = c.keyValueBegin(); kv != c.keyValueEnd(); kv++) {
            if (mKeywordAndWeight[kv->first] <= kv->second) {
                retTmp++;
            }
        }
        if (retTmp >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[次数权重] matched count: " << c << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }
    else {
        const auto c = rm.getMatchedCount();
        if (c >= getMinMatchCount()) {
            TASK_SCAN_LOG_INFO << "[关闭权重]matched count: " << c << " greater then mini count: " << getMinMatchCount();
            // FIXME:// 多保存几个上下文？每个关键词一个？
            saveResult(filePath, metaPath, getContent(rm));
            ret = true;
        }
    }

#undef TASK_SCAN_LOG_INFO

    return ret;
}

void KeywordRule::setIgnoreCase(bool ignoreCase)
{
    mIgnoreCase = ignoreCase;
}

bool KeywordRule::getIgnoreCase() const
{
    return mIgnoreCase;
}

void KeywordRule::setIgnoreConfuse(bool c)
{
    mIgnoreConfuse = c;
}

bool KeywordRule::getIgnoreConfuse() const
{
    return mIgnoreConfuse;
}

void KeywordRule::setIgnoreZhTw(bool c)
{
    mIgnoreZhTw = c;
}

bool KeywordRule::getIgnoreZhTw() const
{
    return mIgnoreZhTw;
}

void KeywordRule::setWildcard(bool c)
{
    mWildcard = c;
}

bool KeywordRule::getWildcard() const
{
    return mWildcard;
}

void KeywordRule::setRecognitionMode(int m)
{
    mMode = (RecognitionMode) m;
}

RuleBase::RecognitionMode KeywordRule::getRecognitionMode() const
{
    return mMode;
}

void KeywordRule::setMinMatchCount(qint64 c)
{
    mCount = c;
}

qint64 KeywordRule::getMinMatchCount() const
{
    return mCount;
}

void KeywordRule::addCurrentMatchCount(qint64 c)
{
    mCurCount += c;
}

qint64 KeywordRule::getCurrentMatchCount() const
{
    return mCurCount;
}

void KeywordRule::parseRule(const QJsonValue & rule)
{
    const auto ruleParam = rule["ruleParam"].toObject();
    const auto ruleAttrDto = rule["ruleAttrDto"].toObject();
    const auto clueItemDataList = rule["clueItemDataList"].toArray();

    C_RETURN_IF_OK(ruleParam.isEmpty() || ruleAttrDto.isEmpty() || clueItemDataList.isEmpty());

    const bool ignoreCase = ruleParam["ignoreCase"].toBool();
    const bool ignoreConfuse = ruleParam["ignoreConfuse"].toBool();
    const bool wildcard = ruleParam["wildcard"].toBool();
    const bool ignoreZhTw = ruleParam["ignoreZhTw"].toBool();
    const int weightFlag = ruleParam["weightFlag"].toInt();
    const int minMatchItems = ruleParam["minMatchItems"].toInt();
    const int minMatchItemsScore = ruleParam["minMatchItemsScore"].toInt();
    const bool exactMatch = ruleParam["exactMatch"].toBool(); // true 匹配到后、显示时候把前后非数字去掉，否则不去掉

    setExactMatch(exactMatch);
    setWildcard(wildcard);
    setIgnoreCase(ignoreCase);
    setIgnoreConfuse(ignoreConfuse);
    setIgnoreZhTw(ignoreZhTw);
    setRecognitionMode(weightFlag);

    qInfo() << "1";
    if (getRecognitionMode() == RuleBase::RecognitionTimes
        || getRecognitionMode() == RuleBase::RecognitionClose) {
        setMinMatchCount(minMatchItems);
        qInfo() << "setMinMatchCount: " << getMinMatchCount();
    }
    else if (getRecognitionMode() == RuleBase::RecognitionScore) {
        setMinMatchCount(minMatchItemsScore);
    }

    for (auto k : clueItemDataList) {
        const auto kkk = k.toObject()["itemValue"].toString("");
        const auto weight = k.toObject()["weight"].toInt(0);
        setKeywordAndWeight(kkk, weight);
    }

#if 1
    qInfo() << "\nKeyWord:\n" \
            << "ignoreCase          : " << ignoreCase << "\n" \
            << "ignoreConfuse       : " << ignoreConfuse << "\n" \
            << "wildcard            : " << wildcard << "\n" \
            << "ignoreZhTW          : " << ignoreZhTw << "\n" \
            << "weightFlag          : " << weightFlag << "\n" \
            << "exactMatch          : " << exactMatch << "\n" \
            << "miniMatchItems      : " << minMatchItems << "\n" \
            << "miniMatchItemsScore : " << minMatchItemsScore << "\n" \
            << "getMinMatchCount    : " << getMinMatchCount();
#endif
}

PolicyGroup::PolicyGroup(const QString & id, const QString & name)
    : mId(id), mName(name)
{
}

PolicyGroup::~PolicyGroup()
{
}

void PolicyGroup::parseRule(const QJsonArray& rule)
{
    for (auto r : rule) {
        const auto ruleType = r["clue_key"].toString("");
        const auto ruleId = r["rule_id"].toString();
        const int exceptFlag = r["exception_flag"].toInt();

        const auto ruleParam = r["ruleParam"].toObject();
        const auto ruleAttrDto = r["ruleAttrDto"].toObject();
        const auto clueItemDataList = r["clueItemDataList"].toArray();

        if (!ruleType.isEmpty() && !ruleId.isEmpty()) {
            std::shared_ptr<RuleBase> rT = nullptr;
            if ("regex" == ruleType.toLower()) {
                rT = std::make_shared<RegexpRule>(ruleId);
                rT->parseRule(r);
                qInfo() << "Parse regex!";
            }
            else if ("keyword" == ruleType.toLower()) {
                rT = std::make_shared<KeywordRule>(ruleId);
                rT->parseRule(r);
                qInfo() << "Parse keyword!";
            }
            else if ("filetype" == ruleType.toLower()) {
                rT = std::make_shared<FileTypeRule>(ruleId);
                rT->parseRule(r);
                qInfo() << "Parse filetype!";
            }
            else if ("idm" == ruleType.toLower()) {
                rT = std::make_shared<IdmRule>(ruleId);
                rT->parseRule(r);
                qInfo() << "Parse idm!";
            }
            else {
                qWarning() << "Rule type " << ruleType << " does not exist";
                continue;
            }

            if (1 == exceptFlag) {
                mExceptRules[ruleId] = rT;
            }
            else {
                mRules[ruleId] = rT;
            }

            // 保存 rule_id
            DataBase::getInstance().updateRuleId(ruleId);
        }
    }
}

QString PolicyGroup::getPolicyGroupName() const
{
    return mName;
}

void PolicyGroup::setDescription(const QString & desc)
{
    mDescription = desc;
}

const QString & PolicyGroup::getDescription() const
{
    return mDescription;
}

void PolicyGroup::setRiskLevel(RiskLevel level)
{
    mRiskLevel = level;
}

void PolicyGroup::setRiskLevel(int level)
{
    switch (level) {
        default:
        case 1: {
            mRiskLevel = RiskLevel::Low;
            break;
        }
        case 2: {
            mRiskLevel = RiskLevel::Middle;
            break;
        }
        case 3: {
            mRiskLevel = RiskLevel::High;
            break;
        }
    }
}

RiskLevel PolicyGroup::getRiskLevel() const
{
    return mRiskLevel;
}

QString PolicyGroup::getRiskLevelString() const
{
    switch (mRiskLevel) {
        default:
        case RiskLevel::Low: {
            return "Low";
        }
        case RiskLevel::Middle: {
            return "Middle";
        }
        case RiskLevel::High: {
            return "High";
        }
    }

    return "Unknown";
}

void PolicyGroup::setRuleHitCount(qint64 count)
{
    mRuleHitCount = count;
}

qint64 PolicyGroup::getRuleHitCount() const
{
    return mRuleHitCount;
}

void PolicyGroup::setRuleExceptCount(qint64 count)
{
    mRuleExceptCount = count;
}

qint64 PolicyGroup::getRuleExceptCount() const
{
    return mRuleExceptCount;
}

void PolicyGroup::setOrder(int order)
{
    mOrder = order;
}

int PolicyGroup::getOrder() const
{
    return mOrder;
}

MatchResult PolicyGroup::match(const QString& filePath, const QString& metaPath, const QString& ctxPath)
{
#define TASK_SCAN_LOG_INFO       qInfo() \
    << "[TaskId: " << mId \
    << " TaskName: " << mName \
    << " RiskLevel: " << getRiskLevelString() \
    << " RuleHitCount: " << mRuleHitCount \
    << " RuleExceptCount: " << mRuleExceptCount \
    << "] "
#if 0
    QFile fileM(metaPath);
    if (fileM.open(QFile::ReadOnly | QFile::Text)) {
        qInfo() << "\nmeta:\n" << QString(fileM.readAll());
        fileM.close();
    }

    QFile file(ctxPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qInfo() << "\ncontent:\n" << QString(file.readAll());
        file.close();
    }
#endif

    // 例外
    quint64 expInt = 0;         // 例外命中数量
    const qint64 exceptCount = mExceptRules.count();
    if (exceptCount > 0) {
        const qint64 minExceptCount = getRuleExceptCount();
        for (auto kv = mExceptRules.keyValueBegin(); kv != mExceptRules.keyValueEnd(); ++kv) {
            if (kv->second->matchRule(filePath, metaPath, ctxPath)) {
                TASK_SCAN_LOG_INFO << "命中例外策略组ID: " << kv->first << " file: " << filePath;
                expInt++;
            }

            // 如果配置非全量匹配
            if (minExceptCount >= 0 && expInt >= minExceptCount) {
                TASK_SCAN_LOG_INFO << "命中例外策略, file: " << filePath;
                return MatchResult::PG_MATCH_EXCEPT;
            }
        }

        // 如果配置全量匹配
        if (minExceptCount < 0 && expInt >= exceptCount) {
            TASK_SCAN_LOG_INFO << "命中例外策略, file: " << filePath << " expInt: " << expInt << " except rule count: " << exceptCount;
            return MatchResult::PG_MATCH_EXCEPT;
        }
    }


    // 匹配
    quint64 matchInt = 0;       // 规则命中数量
    const qint64 ruleCount = mRules.count();
    if (ruleCount > 0) {
        const qint64 minMatchCount = getRuleHitCount();
        for (auto kv = mRules.keyValueBegin(); kv != mRules.keyValueEnd(); ++kv) {
            if (kv->second->matchRule(filePath, metaPath, ctxPath)) {
                TASK_SCAN_LOG_INFO << "命中策略组ID: " << kv->first << " file: " << filePath;
                matchInt++;
            }

            // 如果配置非全量匹配
            if (minMatchCount >= 0 && matchInt >= minMatchCount) {
                TASK_SCAN_LOG_INFO << "命中策略, file: " << filePath;
                return MatchResult::PG_MATCH_OK;
            }
        }

        // 如果配置全量匹配
        if (minMatchCount < 0 && matchInt >= minMatchCount) {
            TASK_SCAN_LOG_INFO << "命中策略, file: " << filePath << " matchInt: " << matchInt << " rule count: " << ruleCount;
            return MatchResult::PG_MATCH_OK;
        }
    }

    TASK_SCAN_LOG_INFO << "Hit exception policy: " << expInt << " matched policy: " << matchInt;

#undef TASK_SCAN_LOG_INFO

    return MatchResult::PG_MATCH_NO;
}

