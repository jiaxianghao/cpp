#include "TableManager.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace db_util {

// TableRecord implementation
TableRecord::TableRecord(const std::map<std::string, std::string>& data)
    : data_(data)
{
}

void TableRecord::set(const std::string& field, const std::string& value)
{
    data_[field] = value;
}

void TableRecord::set(const std::string& field, int value)
{
    data_[field] = std::to_string(value);
}

void TableRecord::set(const std::string& field, double value)
{
    data_[field] = std::to_string(value);
}

void TableRecord::set(const std::string& field, bool value)
{
    data_[field] = value ? "1" : "0";
}

std::string TableRecord::getString(const std::string& field) const
{
    auto it = data_.find(field);
    return it != data_.end() ? it->second : "";
}

int TableRecord::getInt(const std::string& field) const
{
    auto it = data_.find(field);
    if (it != data_.end() && !it->second.empty())
    {
        try
        {
            return std::stoi(it->second);
        }
        catch (...)
        {
            return 0;
        }
    }
    return 0;
}

double TableRecord::getDouble(const std::string& field) const
{
    auto it = data_.find(field);
    if (it != data_.end() && !it->second.empty())
    {
        try
        {
            return std::stod(it->second);
        }
        catch (...)
        {
            return 0.0;
        }
    }
    return 0.0;
}

bool TableRecord::getBool(const std::string& field) const
{
    auto it = data_.find(field);
    if (it != data_.end())
    {
        return it->second == "1" || it->second == "true" || it->second == "TRUE";
    }
    return false;
}

bool TableRecord::hasField(const std::string& field) const
{
    return data_.find(field) != data_.end();
}

const std::map<std::string, std::string>& TableRecord::getData() const
{
    return data_;
}

void TableRecord::clear()
{
    data_.clear();
}

// QueryBuilder implementation
QueryBuilder::QueryBuilder()
    : limitCount_(-1)
    , offsetCount_(-1)
    , hasSelect_(false)
{
}

QueryBuilder& QueryBuilder::select(const std::vector<std::string>& fields)
{
    if (fields.empty())
    {
        selectFields_ = "*";
    }
    else
    {
        std::ostringstream oss;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (i > 0) oss << ", ";
            oss << fields[i];
        }
        selectFields_ = oss.str();
    }
    hasSelect_ = true;
    return *this;
}

QueryBuilder& QueryBuilder::from(const std::string& table)
{
    fromTable_ = table;
    return *this;
}

QueryBuilder& QueryBuilder::where(const std::string& condition)
{
    whereConditions_.clear();
    whereConditions_.push_back(condition);
    return *this;
}

QueryBuilder& QueryBuilder::andWhere(const std::string& condition)
{
    whereConditions_.push_back("AND " + condition);
    return *this;
}

QueryBuilder& QueryBuilder::orWhere(const std::string& condition)
{
    whereConditions_.push_back("OR " + condition);
    return *this;
}

QueryBuilder& QueryBuilder::orderBy(const std::string& field, bool ascending)
{
    std::string clause = field + (ascending ? " ASC" : " DESC");
    orderByClauses_.push_back(clause);
    return *this;
}

QueryBuilder& QueryBuilder::limit(int count)
{
    limitCount_ = count;
    return *this;
}

QueryBuilder& QueryBuilder::offset(int count)
{
    offsetCount_ = count;
    return *this;
}

std::string QueryBuilder::build() const
{
    if (!hasSelect_ || fromTable_.empty())
    {
        return "";
    }

    std::ostringstream query;
    query << "SELECT " << selectFields_ << " FROM " << fromTable_;

    if (!whereConditions_.empty())
    {
        query << " WHERE ";
        for (size_t i = 0; i < whereConditions_.size(); ++i)
        {
            if (i > 0) query << " ";
            query << whereConditions_[i];
        }
    }

    if (!orderByClauses_.empty())
    {
        query << " ORDER BY ";
        for (size_t i = 0; i < orderByClauses_.size(); ++i)
        {
            if (i > 0) query << ", ";
            query << orderByClauses_[i];
        }
    }

    if (limitCount_ > 0)
    {
        query << " LIMIT " << limitCount_;
    }

    if (offsetCount_ > 0)
    {
        query << " OFFSET " << offsetCount_;
    }

    return query.str();
}

void QueryBuilder::reset()
{
    selectFields_.clear();
    fromTable_.clear();
    whereConditions_.clear();
    orderByClauses_.clear();
    limitCount_ = -1;
    offsetCount_ = -1;
    hasSelect_ = false;
}

// TableManager implementation
TableManager::TableManager(DatabaseManager& dbManager)
    : dbManager_(dbManager)
{
}

bool TableManager::createTable(const std::string& tableName, 
                              const std::map<std::string, std::string>& fields)
{
    if (fields.empty())
    {
        lastError_ = "No fields specified for table creation";
        return false;
    }

    std::ostringstream query;
    query << "CREATE TABLE IF NOT EXISTS " << tableName << " (";
    
    bool first = true;
    for (const auto& field : fields)
    {
        if (!first) query << ", ";
        query << field.first << " " << field.second;
        first = false;
    }
    query << ")";

    return dbManager_.executeQuery(query.str());
}

bool TableManager::dropTable(const std::string& tableName)
{
    std::string query = "DROP TABLE IF EXISTS " + tableName;
    return dbManager_.executeQuery(query);
}

bool TableManager::tableExists(const std::string& tableName)
{
    std::string query = "SHOW TABLES LIKE '" + tableName + "'";
    auto result = dbManager_.executeSelect(query);
    return result && result->next();
}

bool TableManager::insert(const std::string& tableName, const TableRecord& record)
{
    std::string query = buildInsertQuery(tableName, record);
    return dbManager_.executeQuery(query);
}

bool TableManager::insertBatch(const std::string& tableName, 
                              const std::vector<TableRecord>& records)
{
    if (records.empty())
    {
        return true;
    }

    if (!beginTransaction())
    {
        return false;
    }

    bool success = true;
    for (const auto& record : records)
    {
        if (!insert(tableName, record))
        {
            success = false;
            break;
        }
    }

    if (success)
    {
        return commitTransaction();
    }
    else
    {
        rollbackTransaction();
        return false;
    }
}

std::vector<TableRecord> TableManager::select(const std::string& tableName)
{
    return select(tableName, {});
}

std::vector<TableRecord> TableManager::select(const std::string& tableName, 
                                             const std::vector<std::string>& fields)
{
    std::string query = buildSelectQuery(tableName, fields);
    auto result = dbManager_.executeSelect(query);
    return resultSetToRecords(result.get());
}

std::vector<TableRecord> TableManager::selectWhere(const std::string& tableName, 
                                                  const std::string& condition)
{
    std::string query = buildSelectQuery(tableName) + " WHERE " + condition;
    auto result = dbManager_.executeSelect(query);
    return resultSetToRecords(result.get());
}

TableRecord TableManager::selectOne(const std::string& tableName, 
                                   const std::string& condition)
{
    std::string query = buildSelectQuery(tableName) + " WHERE " + condition + " LIMIT 1";
    auto result = dbManager_.executeSelect(query);
    return resultSetToRecord(result.get());
}

TableRecord TableManager::selectById(const std::string& tableName, 
                                    const std::string& idField, int id)
{
    std::string condition = idField + " = " + std::to_string(id);
    return selectOne(tableName, condition);
}

bool TableManager::update(const std::string& tableName, 
                         const TableRecord& record, 
                         const std::string& condition)
{
    std::string query = buildUpdateQuery(tableName, record, condition);
    return dbManager_.executeQuery(query);
}

bool TableManager::updateById(const std::string& tableName, 
                             const TableRecord& record, 
                             const std::string& idField, int id)
{
    std::string condition = idField + " = " + std::to_string(id);
    return update(tableName, record, condition);
}

bool TableManager::deleteWhere(const std::string& tableName, 
                              const std::string& condition)
{
    std::string query = buildDeleteQuery(tableName, condition);
    return dbManager_.executeQuery(query);
}

bool TableManager::deleteById(const std::string& tableName, 
                             const std::string& idField, int id)
{
    std::string condition = idField + " = " + std::to_string(id);
    return deleteWhere(tableName, condition);
}

int TableManager::count(const std::string& tableName)
{
    std::string query = "SELECT COUNT(*) as count FROM " + tableName;
    auto result = dbManager_.executeSelect(query);
    if (result && result->next())
    {
        return result->getInt("count");
    }
    return 0;
}

int TableManager::countWhere(const std::string& tableName, 
                            const std::string& condition)
{
    std::string query = "SELECT COUNT(*) as count FROM " + tableName + " WHERE " + condition;
    auto result = dbManager_.executeSelect(query);
    if (result && result->next())
    {
        return result->getInt("count");
    }
    return 0;
}

QueryBuilder TableManager::query()
{
    return QueryBuilder();
}

bool TableManager::beginTransaction()
{
    return dbManager_.beginTransaction();
}

bool TableManager::commitTransaction()
{
    return dbManager_.commitTransaction();
}

bool TableManager::rollbackTransaction()
{
    return dbManager_.rollbackTransaction();
}

std::string TableManager::getLastError() const
{
    return lastError_.empty() ? dbManager_.getLastError() : lastError_;
}

// Helper methods
std::string TableManager::buildInsertQuery(const std::string& tableName, 
                                          const TableRecord& record)
{
    const auto& data = record.getData();
    if (data.empty())
    {
        return "";
    }

    std::ostringstream query;
    query << "INSERT INTO " << tableName << " (";
    
    // Fields
    bool first = true;
    for (const auto& field : data)
    {
        if (!first) query << ", ";
        query << field.first;
        first = false;
    }
    
    query << ") VALUES (";
    
    // Values
    first = true;
    for (const auto& field : data)
    {
        if (!first) query << ", ";
        // Handle different data types properly
        if (field.second == "true" || field.second == "false")
        {
            query << (field.second == "true" ? "1" : "0");
        }
        else if (field.second.find_first_not_of("0123456789.-") == std::string::npos)
        {
            // It's a number, don't quote it
            query << field.second;
        }
        else
        {
            query << "'" << field.second << "'";
        }
        first = false;
    }
    
    query << ")";
    return query.str();
}

std::string TableManager::buildUpdateQuery(const std::string& tableName, 
                                          const TableRecord& record, 
                                          const std::string& condition)
{
    const auto& data = record.getData();
    if (data.empty())
    {
        return "";
    }

    std::ostringstream query;
    query << "UPDATE " << tableName << " SET ";
    
    bool first = true;
    for (const auto& field : data)
    {
        if (!first) query << ", ";
        // Handle different data types properly
        if (field.second == "true" || field.second == "false")
        {
            query << field.first << " = " << (field.second == "true" ? "1" : "0");
        }
        else if (field.second.find_first_not_of("0123456789.-") == std::string::npos)
        {
            // It's a number, don't quote it
            query << field.first << " = " << field.second;
        }
        else
        {
            query << field.first << " = '" << field.second << "'";
        }
        first = false;
    }
    
    query << " WHERE " << condition;
    return query.str();
}

std::string TableManager::buildSelectQuery(const std::string& tableName, 
                                          const std::vector<std::string>& fields)
{
    std::ostringstream query;
    query << "SELECT ";
    
    if (fields.empty())
    {
        query << "*";
    }
    else
    {
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (i > 0) query << ", ";
            query << fields[i];
        }
    }
    
    query << " FROM " << tableName;
    return query.str();
}

std::string TableManager::buildDeleteQuery(const std::string& tableName, 
                                          const std::string& condition)
{
    return "DELETE FROM " + tableName + " WHERE " + condition;
}

TableRecord TableManager::resultSetToRecord(sql::ResultSet* rs)
{
    TableRecord record;
    if (rs && rs->next())
    {
        sql::ResultSetMetaData* meta = rs->getMetaData();
        int columns = meta->getColumnCount();
        
        for (int i = 1; i <= columns; ++i)
        {
            std::string columnName = meta->getColumnName(i);
            std::string value = rs->getString(i);
            record.set(columnName, value);
        }
    }
    return record;
}

std::vector<TableRecord> TableManager::resultSetToRecords(sql::ResultSet* rs)
{
    std::vector<TableRecord> records;
    if (rs)
    {
        sql::ResultSetMetaData* meta = rs->getMetaData();
        int columns = meta->getColumnCount();
        
        while (rs->next())
        {
            TableRecord record;
            for (int i = 1; i <= columns; ++i)
            {
                std::string columnName = meta->getColumnName(i);
                std::string value = rs->getString(i);
                record.set(columnName, value);
            }
            records.push_back(record);
        }
    }
    return records;
}
} // namespace db_util
