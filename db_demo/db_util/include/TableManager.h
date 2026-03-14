#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

/**
 * @file TableManager.h
 * @brief 数据库表操作管理器
 * @version 1.0.0
 * @author db_util Team
 */

#include "DatabaseManager.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "DatabaseManager.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace db_util {

// Forward declarations
class TableRecord;
class QueryBuilder;

// Record class for table operations
class TableRecord
{
public:
    TableRecord() = default;
    explicit TableRecord(const std::map<std::string, std::string>& data);
    
    // Set field value
    void set(const std::string& field, const std::string& value);
    void set(const std::string& field, int value);
    void set(const std::string& field, double value);
    void set(const std::string& field, bool value);
    
    // Get field value
    std::string getString(const std::string& field) const;
    int getInt(const std::string& field) const;
    double getDouble(const std::string& field) const;
    bool getBool(const std::string& field) const;
    
    // Check if field exists
    bool hasField(const std::string& field) const;
    
    // Get all data
    const std::map<std::string, std::string>& getData() const;
    
    // Clear all data
    void clear();

private:
    std::map<std::string, std::string> data_;
};

// Query builder for complex queries
class QueryBuilder
{
public:
    QueryBuilder();
    
    // Basic query methods
    QueryBuilder& select(const std::vector<std::string>& fields = {});
    QueryBuilder& from(const std::string& table);
    QueryBuilder& where(const std::string& condition);
    QueryBuilder& andWhere(const std::string& condition);
    QueryBuilder& orWhere(const std::string& condition);
    QueryBuilder& orderBy(const std::string& field, bool ascending = true);
    QueryBuilder& limit(int count);
    QueryBuilder& offset(int count);
    
    // Build the query
    std::string build() const;
    
    // Reset the builder
    void reset();

private:
    std::string selectFields_;
    std::string fromTable_;
    std::vector<std::string> whereConditions_;
    std::vector<std::string> orderByClauses_;
    int limitCount_;
    int offsetCount_;
    bool hasSelect_;
};

// Main table manager class
class TableManager
{
public:
    explicit TableManager(DatabaseManager& dbManager);
    
    // Table operations
    bool createTable(const std::string& tableName, 
                    const std::map<std::string, std::string>& fields);
    bool dropTable(const std::string& tableName);
    bool tableExists(const std::string& tableName);
    
    // Insert operations
    bool insert(const std::string& tableName, const TableRecord& record);
    bool insertBatch(const std::string& tableName, 
                     const std::vector<TableRecord>& records);
    
    // Select operations
    std::vector<TableRecord> select(const std::string& tableName);
    std::vector<TableRecord> select(const std::string& tableName, 
                                   const std::vector<std::string>& fields);
    std::vector<TableRecord> selectWhere(const std::string& tableName, 
                                        const std::string& condition);
    TableRecord selectOne(const std::string& tableName, 
                         const std::string& condition);
    TableRecord selectById(const std::string& tableName, 
                          const std::string& idField, int id);
    
    // Update operations
    bool update(const std::string& tableName, 
                const TableRecord& record, 
                const std::string& condition);
    bool updateById(const std::string& tableName, 
                   const TableRecord& record, 
                   const std::string& idField, int id);
    
    // Delete operations
    bool deleteWhere(const std::string& tableName, 
                    const std::string& condition);
    bool deleteById(const std::string& tableName, 
                   const std::string& idField, int id);
    
    // Count operations
    int count(const std::string& tableName);
    int countWhere(const std::string& tableName, 
                  const std::string& condition);
    
    // Query builder
    QueryBuilder query();
    
    // Transaction support
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // Error handling
    std::string getLastError() const;

private:
    DatabaseManager& dbManager_;
    std::string lastError_;
    
    // Helper methods
    std::string buildInsertQuery(const std::string& tableName, 
                                const TableRecord& record);
    std::string buildUpdateQuery(const std::string& tableName, 
                                const TableRecord& record, 
                                const std::string& condition);
    std::string buildSelectQuery(const std::string& tableName, 
                                const std::vector<std::string>& fields = {});
    std::string buildDeleteQuery(const std::string& tableName, 
                                const std::string& condition);
    
    // Convert result set to TableRecord
    TableRecord resultSetToRecord(sql::ResultSet* rs);
    std::vector<TableRecord> resultSetToRecords(sql::ResultSet* rs);
};

} // namespace db_util

#endif // DB_UTIL_TABLE_MANAGER_H
