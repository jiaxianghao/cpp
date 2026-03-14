#ifndef DB_UTIL_DATABASE_MANAGER_H
#define DB_UTIL_DATABASE_MANAGER_H

/**
 * @file DatabaseManager.h
 * @brief MySQL数据库连接和操作管理器
 * @version 1.0.0
 * @author db_util Team
 */

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <string>
#include <vector>
#include <memory>

namespace db_util {

/**
 * @brief MySQL数据库管理器类
 * 
 * 提供MySQL数据库的连接管理、查询执行、事务处理等功能
 */
class DatabaseManager
{
public:
    /**
     * @brief 构造函数
     */
    DatabaseManager();

    /**
     * @brief 析构函数
     */
    ~DatabaseManager();

    /**
     * @brief 连接到数据库
     * @param host 数据库主机地址
     * @param user 用户名
     * @param password 密码
     * @param database 数据库名
     * @param port 端口号，默认3306
     * @return 连接是否成功
     */
    bool connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& database, 
                 int port = 3306);

    /**
     * @brief 断开数据库连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     * @return 连接状态
     */
    bool isConnected() const;

    /**
     * @brief 执行SQL查询
     * @param query SQL查询语句
     * @return 执行是否成功
     */
    bool executeQuery(const std::string& query);

    /**
     * @brief 执行SELECT查询
     * @param query SELECT查询语句
     * @return 结果集指针
     */
    std::unique_ptr<sql::ResultSet> executeSelect(const std::string& query);
    
    /**
     * @brief 准备预处理语句
     * @param query SQL语句
     * @return 预处理语句指针
     */
    std::unique_ptr<sql::PreparedStatement> prepareStatement(const std::string& query);

    /**
     * @brief 执行预处理语句
     * @param stmt 预处理语句指针
     * @return 执行是否成功
     */
    bool executePreparedStatement(sql::PreparedStatement* stmt);

    /**
     * @brief 执行预处理SELECT语句
     * @param stmt 预处理语句指针
     * @return 结果集指针
     */
    std::unique_ptr<sql::ResultSet> executePreparedSelect(sql::PreparedStatement* stmt);

    /**
     * @brief 开始事务
     * @return 是否成功开始事务
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     * @return 是否成功提交
     */
    bool commitTransaction();

    /**
     * @brief 回滚事务
     * @return 是否成功回滚
     */
    bool rollbackTransaction();

    /**
     * @brief 获取最后错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;

    /**
     * @brief 获取最后插入的ID
     * @return 插入的ID
     */
    int getLastInsertId() const;

    /**
     * @brief 获取受影响的行数
     * @return 受影响的行数
     */
    int getAffectedRows() const;

private:
    sql::mysql::MySQL_Driver* driver_;           ///< MySQL驱动指针
    std::unique_ptr<sql::Connection> connection_; ///< 数据库连接
    std::string lastError_;                      ///< 最后错误信息
    bool connected_;                             ///< 连接状态
};

} // namespace db_util

#endif // DB_UTIL_DATABASE_MANAGER_H
