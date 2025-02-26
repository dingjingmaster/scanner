//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_DATA_BASE_H
#define andsec_scanner_DATA_BASE_H
#include <QObject>
#include "../sqlite3-wrap/src/sqlite3-wrap.h"

/**
 * @brief 扫描任务和扫描结果
 */
class DataBase final : public QObject
{
    Q_OBJECT
public:
    static DataBase& getInstance();
    DataBase(DataBase &&) = delete;
    DataBase(DataBase const &) = delete;
    ~DataBase() override;

    /**
     * @brief 初始化数据库
     */
    void initDB() const;
private:
    explicit DataBase(QObject *parent = nullptr);
private:
    static DataBase                         gInstance;
    std::shared_ptr<sqlite3_wrap::Sqlite3>  mDB;
};



#endif // andsec_scanner_DATA_BASE_H
