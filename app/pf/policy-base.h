//
// Created by dingjing on 2/17/25.
//

#ifndef andsec_scanner_POLICY_BASE_H
#define andsec_scanner_POLICY_BASE_H
#include <memory>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QString>

enum class RuleType
{
    GeneralKeyword,
    GeneralRegular,
    GeneralFileAttribute,
    GeneralFileFingerprint
};

enum class RiskLevel
{
    Low,
    Middle,
    High
};

enum class MatchResult
{
    PG_MATCH_ERR,       // 出错
    PG_MATCH_EXCEPT,    // 命中例外
    PG_MATCH_OK,        // 命中策略
    PG_MATCH_NO,        // 没有命中
};

class RuleBase
{
public:
    enum RecognitionMode
    {
        RecognitionTimes = 1,
        RecognitionScore,
        RecognitionClose
    };

    explicit RuleBase(const QString &id, RuleType type);
    virtual ~RuleBase();

    virtual RuleType getRuleType();

    void setExactMatch(bool exactMatch);
    bool getExactMatch() const;

    const QString &getRuleId() const;

    virtual void parseRule(const QJsonValue& rule);
    virtual bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath);
    virtual void saveResult(const QString& filePath, const QString& metaPath, const QString& context);

    QString getTaskTypeString() const;

private:
    bool                                        mExactMatch;
    QString                                     mRuleId;        // rule id
    RuleType                                    mType;          // 检测类型
};

class IdmRule final : public RuleBase
{
public:
    explicit IdmRule(const QString &id);
    ~IdmRule() override = default;

    void parseRule(const QJsonValue& rule) override;

private:
    void addIdmAndWeight(const QString& idm, int weight);

private:
    bool                        mIsCheckIdm{};
    QMap<QString, int>          mIdmAndWeight;
};

class FileTypeRule final : public RuleBase
{
public:
    explicit FileTypeRule(const QString &id);
    ~FileTypeRule() override = default;

    void parseRule(const QJsonValue& rule) override;
    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath) override;

private:
    void setModifyTimes(const QString& start, const QString& end);
    void setCreateTimes(const QString& start, const QString& end);
    void addFileTypes(const QString& fileType);
    void addFileNames(const QString& fileNames);
    void setFileSize(qint64 start, qint64 end, int type);

    void addDetailSubject(const QString& subject);
    void addDetailCompany(const QString& company);
    void addDetailManager(const QString& manager);
    void addDetailTags(const QString& tags);
    void addDetailCategory(const QString& cate);
    void addDetailComments(const QString& comments);
    void addDetailPreserver(const QString& preserver);
    void addDetailAuthor(const QString& author);
    void addDetailTitle(const QString& title);

private:
    bool                                            mIsCheckCreateTime;
    qint64                                          mCreateTimeStart;
    qint64                                          mCreateTimeEnd;

    bool                                            mIsCheckModifyTime;
    qint64                                          mModifyTimeStart;
    qint64                                          mModifyTimeEnd;

    bool                                            mIsCheckFileTypes;
    QSet<QString>                                   mFileTypes;

    bool                                            mIsCheckFileNames;
    QSet<QString>                                   mFileNames;

    bool                                            mIsCheckFileSize;
    qint64                                          mMinFileSize;
    qint64                                          mMaxFileSize;

    bool                                            mIsCheckDetailSubject;
    QSet<QString>                                   mDetailSubject;

    bool                                            mIsCheckDetailCompany;
    QSet<QString>                                   mDetailCompany;

    bool                                            mIsCheckDetailManager;
    QSet<QString>                                   mDetailManager;

    bool                                            mIsCheckDetailTags;
    QSet<QString>                                   mDetailTags;

    bool                                            mIsCheckDetailCategory;
    QSet<QString>                                   mDetailCategory;

    bool                                            mIsCheckDetailComments;
    QSet<QString>                                   mDetailComments;

    bool                                            mIsCheckDetailPreserver;
    QSet<QString>                                   mDetailPreserver;

    bool                                            mIsCheckDetailAuthor;
    QSet<QString>                                   mDetailAuthor;

    bool                                            mIsCheckDetailTitle;
    QSet<QString>                                   mDetailTitle;
};

class RegexpRule final : public RuleBase
{
public:
    explicit RegexpRule(const QString &id);

    // bool getWildcard() const;
    // bool getIgnoreCase() const;
    // bool getIgnoreZhTw() const;
    // bool getIgnoreConfuse() const;
    qint64 getMinMatchCount() const;
    qint64 getCurrentMatchCount() const;
    RecognitionMode getRecognitionMode() const;
    int getWightByRegex (const QString &regex) const;
    bool getIsPreciseSearch() const;

    void parseRule(const QJsonValue& rule) override;
    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath) override;

private:
    void addCurrentMatchCount(qint64 c=1);
    void setMinMatchCount(qint64 c);
    void setRecognitionMode(int m);
    // void setWildcard(bool c);
    // void setIgnoreZhTw(bool c);
    // void setIgnoreConfuse(bool c);
    // void setIgnoreCase(bool ignoreCase);
    void setIsPreciseSearch(bool p);
    void setRegexAndWeight(const QString &regex, int wight);

private:
    QMap<QString, int>                          mRegexAndWeight;            // keyword + wight

    // 属性
    bool                                        mIsPreciseSearch = false;   // 精确查找

    // bool                                        mIgnoreCase = false;        // 忽略大小写
    // bool                                        mIgnoreConfuse = false;     // 支持恶意混淆
    // bool                                        mIgnoreZhTw = false;        // 忽略中文简体、繁体
    // bool                                        mWildcard = false;          // 支持通配符

    // 匹配模式
    enum RecognitionMode                        mMode;
    qint64                                      mCount;                     // 关键字权重次数、积分权重分数、关闭权重表示匹配到的关键词个数
    qint64                                      mCurCount;                  // 当前得分
};

class KeywordRule final : public RuleBase
{
public:
    explicit KeywordRule(const QString &id);

    bool getWildcard() const;
    bool getIgnoreZhTw() const;
    bool getIgnoreCase() const;
    bool getIgnoreConfuse() const;
    qint64 getMinMatchCount() const;
    qint64 getCurrentMatchCount() const;
    RecognitionMode getRecognitionMode() const;
    void parseRule(const QJsonValue & rule) override;
    int getWightByKeyword(const QString &keyword) const;

    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath) override;

private:
    void setWildcard(bool c);
    void setIgnoreZhTw(bool c);
    void setIgnoreConfuse(bool c);
    void setRecognitionMode(int m);
    void setMinMatchCount(qint64 c);
    void setIgnoreCase(bool ignoreCase);
    void addCurrentMatchCount(qint64 c=1);
    void setKeywordAndWeight(const QString &keyword, int wight);

private:
    // 当规则命中文件时候，将相关信息缓存起来

    // 当扫描的时候，先根据文件、文件MD5、策略ID来确定是否已经完成扫描

private:
    QMap<QString, int>                          mKeywordAndWeight;          // keyword + wight

    // 属性
    bool                                        mIgnoreCase = false;        // 忽略大小写
    bool                                        mIgnoreConfuse = false;     // 支持恶意混淆
    bool                                        mIgnoreZhTw = false;        // 忽略中文简体、繁体
    bool                                        mWildcard = false;          // 支持通配符

    // 匹配模式
    enum RecognitionMode                        mMode;
    qint64                                      mCount;                     // 关键字权重次数、积分权重分数、关闭权重表示匹配到的关键词个数
    qint64                                      mCurCount;                  // 当前得分
};


/**
 * @brief 策略
 */
class PolicyGroup
{
public:
    PolicyGroup(const QString& id, const QString& name);
    ~PolicyGroup();

    void parseRule(const QJsonArray& rule);
    QString getPolicyGroupName() const;

    void setDescription(const QString& desc);
    const QString& getDescription() const;

    void setFinallyHitCount(qint64 c);
    qint64 getFinallyHitCount() const;

    void setRiskLevel(RiskLevel level);
    void setRiskLevel(int level);
    RiskLevel getRiskLevel() const;
    int getRiskLevelInt() const;
    QString getRiskLevelString() const;

    void setRuleHitCount(qint64 count);
    qint64 getRuleHitCount() const;

    void setRuleExceptCount(qint64 count);
    qint64 getRuleExceptCount() const;

    void setOrder(int order);
    int getOrder() const;

    bool containsRule(const QString& key) const;

    MatchResult match (const QString& filePath, const QString& metaPath, const QString& ctxPath);

private:
    int                                         mOrder;
    QString                                     mId;
    QString                                     mName;                  // 策略名称
    QString                                     mDescription;           // 策略描述
    RiskLevel                                   mRiskLevel;             // 风险等级
    qint64                                      mRuleHitCount;          // 组内需要命中策略数，-1表示全部命中，策略命中个数
    qint64                                      mRuleExceptCount;       // 组内例外命中策略数，-1表示需要全部命中
    QMap<QString, std::shared_ptr<RuleBase>>    mExceptRules;
    QMap<QString, std::shared_ptr<RuleBase>>    mRules;                 // 规则

    qint64                                      mRuleHitResultCount;    // 最终命中次数(策略组没命中则为0)
};


#endif // andsec_scanner_POLICY_BASE_H
