//
// Created by dingjing on 2/17/25.
//

#include "policy-base.h"

#include <QJsonDocument>
#include <macros/macros.h>

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

bool RuleBase::matchRule(const QString & stringLine)
{
    return false;
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

bool FileTypeRule::matchRule(const QString & lineString)
{
    return false;
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

int RegexpRule::getWightByRegex(const QString & regex) const
{
    if (mRegexAndWeight.contains(regex)) {
        return mRegexAndWeight[regex];
    }

    return 0;
}

void RegexpRule::parseRule(const QJsonValue & rule)
{
    const auto ruleParam = rule["ruleParam"].toObject();
    const auto ruleAttrDto = rule["ruleAttrDto"].toObject();
    const auto clueItemDataList = rule["clueItemDataList"].toArray();

    C_RETURN_IF_OK(ruleParam.isEmpty() || ruleAttrDto.isEmpty() || clueItemDataList.isEmpty());

    const bool ignoreCase = ruleParam["ignoreCase"].toBool();
    const bool ignoreConfuse = ruleParam["ignoreConfuse"].toBool();
    const bool wildcard = ruleParam["wildcard"].toBool();
    const bool ignoreZhTw = ruleParam["ignoreZhTw"].toBool();
    const int wightFlag = ruleParam["wightFlag"].toInt();
    const int minMatchItems = ruleParam["minMatchItems"].toInt();
    const int minMatchItemsScore = ruleParam["minMatchItemsScore"].toInt();
    const bool exactMatch = ruleParam["exactMatch"].toBool(); // true 匹配到后、显示时候把前后非数字去掉，否则不去掉

    setExactMatch(exactMatch);
    setWildcard(wildcard);
    setIgnoreCase(ignoreCase);
    setIgnoreConfuse(ignoreConfuse);
    setIgnoreZhTw(ignoreZhTw);
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

void RegexpRule::setRecognitionMode(int m)
{
    mMode = (RecognitionMode) m;
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
    const int wightFlag = ruleParam["wightFlag"].toInt();
    const int minMatchItems = ruleParam["minMatchItems"].toInt();
    const int minMatchItemsScore = ruleParam["minMatchItemsScore"].toInt();
    const bool exactMatch = ruleParam["exactMatch"].toBool(); // true 匹配到后、显示时候把前后非数字去掉，否则不去掉

    setExactMatch(exactMatch);
    setWildcard(wildcard);
    setIgnoreCase(ignoreCase);
    setIgnoreConfuse(ignoreConfuse);
    setIgnoreZhTw(ignoreZhTw);
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
        setKeywordAndWeight(kkk, weight);
    }
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
            }
            else if ("keyword" == ruleType.toLower()) {
                rT = std::make_shared<KeywordRule>(ruleId);
                rT->parseRule(r);
            }
            else if ("filetype" == ruleType.toLower()) {
                rT = std::make_shared<FileTypeRule>(ruleId);
                rT->parseRule(r);
            }
            else if ("idm" == ruleType.toLower()) {
                rT = std::make_shared<IdmRule>(ruleId);
                rT->parseRule(r);
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

void PolicyGroup::setRuleHitCount(int count)
{
    mRuleHitCount = count;
}

int PolicyGroup::getRuleHitCount() const
{
    return mRuleHitCount;
}

void PolicyGroup::setRuleExceptCount(int count)
{
    mRuleExceptCount = count;
}

int PolicyGroup::getRuleExceptCount() const
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

bool PolicyGroup::match(const QString& filePath, QList<QString>& ctx, QMap<QString, QString>& res)
{
    return false;
}

