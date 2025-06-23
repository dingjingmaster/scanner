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
    virtual bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res);
    virtual bool exceptRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res);

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
    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res) override;

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

    bool getWildcard() const;
    bool getIgnoreCase() const;
    bool getIgnoreZhTw() const;
    bool getIgnoreConfuse() const;
    qint64 getMinMatchCount() const;
    qint64 getCurrentMatchCount() const;
    RecognitionMode getRecognitionMode() const;
    int getWightByRegex (const QString &regex) const;

    void parseRule(const QJsonValue& rule) override;
    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res) override;

private:
    void addCurrentMatchCount(qint64 c=1);
    void setMinMatchCount(qint64 c);
    void setRecognitionMode(int m);
    void setWildcard(bool c);
    void setIgnoreZhTw(bool c);
    void setIgnoreConfuse(bool c);
    void setIgnoreCase(bool ignoreCase);
    void setRegexAndWeight(const QString &regex, int wight);

private:
    QMap<QString, int>                          mRegexAndWeight;            // keyword + wight

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

    bool matchRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res) override;
    bool exceptRule(const QString& filePath, const QString& metaPath, const QString& ctxPath, const QList<QString>& ctx, QMap<QString, QString>& res) override;

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

    void setRiskLevel(RiskLevel level);
    void setRiskLevel(int level);
    RiskLevel getRiskLevel() const;
    QString getRiskLevelString() const;

    void setRuleHitCount(int count);
    int getRuleHitCount() const;

    void setRuleExceptCount(int count);
    int getRuleExceptCount() const;

    void setOrder(int order);
    int getOrder() const;

    bool match (const QString& filePath, const QString& metaPath, const QString& ctxPath, QList<QString>& ctx, QMap<QString, QString>& res);

private:
    int                                         mOrder;
    QString                                     mId;
    QString                                     mName;              // 策略名称
    QString                                     mDescription;       // 策略描述
    RiskLevel                                   mRiskLevel;         // 风险等级
    int                                         mRuleHitCount;      // 策略命中个数
    int                                         mRuleExceptCount;   // 例外策略个数
    QMap<QString, std::shared_ptr<RuleBase>>    mExceptRules;
    QMap<QString, std::shared_ptr<RuleBase>>    mRules;             // 规则
};


#endif // andsec_scanner_POLICY_BASE_H
