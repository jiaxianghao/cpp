#include "database/database_connection.h"
#include "database/database_config.h"
#include <iostream>
#include <cstring>

DatabaseConnection::PreparedParam DatabaseConnection::PreparedParam::fromInt(int value)
{
    PreparedParam param;
    param.type = PreparedParam::Type::Int;
    param.int_value = value;
    return param;
}

DatabaseConnection::PreparedParam DatabaseConnection::PreparedParam::fromDouble(double value)
{
    PreparedParam param;
    param.type = PreparedParam::Type::Double;
    param.double_value = value;
    return param;
}

DatabaseConnection::PreparedParam DatabaseConnection::PreparedParam::fromString(std::string value)
{
    PreparedParam param;
    param.type = PreparedParam::Type::String;
    param.string_value = std::move(value);
    return param;
}

DatabaseConnection::DatabaseConnection()
    : mysql_(nullptr), stmt_(nullptr), connected_(false)
{
    mysql_ = mysql_init(nullptr);
    if (!mysql_)
    {
        setError("Failed to initialize MySQL");
    }
}

DatabaseConnection::~DatabaseConnection()
{
    disconnect();
}

bool DatabaseConnection::connect()
{
    if (!mysql_)
    {
        setError("MySQL not initialized");
        return false;
    }
    
    DatabaseConfig& config = DatabaseConfig::getInstance();
    
    if (!mysql_real_connect(mysql_,
                           config.getHost().c_str(),
                           config.getUser().c_str(),
                           config.getPassword().c_str(),
                           config.getDatabase().c_str(),
                           config.getPort(),
                           nullptr,
                           0))
    {
        setError(mysql_error(mysql_));
        return false;
    }
    
    connected_ = true;
    return true;
}

void DatabaseConnection::disconnect()
{
    if (stmt_)
    {
        mysql_stmt_close(stmt_);
        stmt_ = nullptr;
    }
    
    if (connected_)
    {
        mysql_close(mysql_);
        connected_ = false;
    }
}

bool DatabaseConnection::executeQuery(const std::string& query)
{
    if (!connected_)
    {
        setError("Not connected to database");
        return false;
    }
    
    if (mysql_query(mysql_, query.c_str()) != 0)
    {
        setError(mysql_error(mysql_));
        return false;
    }
    
    return true;
}

std::vector<std::map<std::string, std::string>> DatabaseConnection::fetchAll(const std::string& query)
{
    std::vector<std::map<std::string, std::string>> results;
    
    if (!executeQuery(query))
    {
        return results;
    }
    
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (!result)
    {
        setError(mysql_error(mysql_));
        return results;
    }
    
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(result);
    int num_fields = mysql_num_fields(result);
    
    while ((row = mysql_fetch_row(result)))
    {
        std::map<std::string, std::string> row_data;
        for (int i = 0; i < num_fields; ++i)
        {
            std::string field_name = fields[i].name;
            std::string value = row[i] ? row[i] : "";
            row_data[field_name] = value;
        }
        results.push_back(row_data);
    }
    
    mysql_free_result(result);
    return results;
}

std::map<std::string, std::string> DatabaseConnection::fetchOne(const std::string& query)
{
    auto results = fetchAll(query);
    return results.empty() ? std::map<std::string, std::string>() : results[0];
}

bool DatabaseConnection::executePreparedQuery(const std::string& query,
    const std::vector<PreparedParam>& params)
{
    if (!connected_)
    {
        setError("Not connected to database");
        return false;
    }

    MYSQL_STMT* stmt = mysql_stmt_init(mysql_);
    if (!stmt)
    {
        setError("Failed to initialize prepared statement");
        return false;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    if (!params.empty())
    {
        std::vector<MYSQL_BIND> binds(params.size());
        std::vector<int> int_values(params.size());
        std::vector<double> double_values(params.size());
        std::vector<std::string> string_values(params.size());
        std::vector<unsigned long> string_lengths(params.size());
        std::vector<my_bool> null_flags(params.size(), 0);

        for (size_t i = 0; i < params.size(); ++i)
        {
            std::memset(&binds[i], 0, sizeof(MYSQL_BIND));
            binds[i].is_null = &null_flags[i];

            const auto& param = params[i];
            if (param.type == PreparedParam::Type::Int)
            {
                int_values[i] = param.int_value;
                binds[i].buffer_type = MYSQL_TYPE_LONG;
                binds[i].buffer = &int_values[i];
            }
            else if (param.type == PreparedParam::Type::Double)
            {
                double_values[i] = param.double_value;
                binds[i].buffer_type = MYSQL_TYPE_DOUBLE;
                binds[i].buffer = &double_values[i];
            }
            else
            {
                string_values[i] = param.string_value;
                string_lengths[i] = static_cast<unsigned long>(string_values[i].size());
                binds[i].buffer_type = MYSQL_TYPE_STRING;
                binds[i].buffer = const_cast<char*>(string_values[i].c_str());
                binds[i].buffer_length = string_lengths[i];
                binds[i].length = &string_lengths[i];
            }
        }

        if (mysql_stmt_bind_param(stmt, binds.data()) != 0)
        {
            setError(mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

std::vector<std::map<std::string, std::string>> DatabaseConnection::fetchAllPrepared(
    const std::string& query,
    const std::vector<PreparedParam>& params)
{
    std::vector<std::map<std::string, std::string>> results;

    if (!connected_)
    {
        setError("Not connected to database");
        return results;
    }

    MYSQL_STMT* stmt = mysql_stmt_init(mysql_);
    if (!stmt)
    {
        setError("Failed to initialize prepared statement");
        return results;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return results;
    }

    if (!params.empty())
    {
        std::vector<MYSQL_BIND> binds(params.size());
        std::vector<int> int_values(params.size());
        std::vector<double> double_values(params.size());
        std::vector<std::string> string_values(params.size());
        std::vector<unsigned long> string_lengths(params.size());
        std::vector<my_bool> null_flags(params.size(), 0);

        for (size_t i = 0; i < params.size(); ++i)
        {
            std::memset(&binds[i], 0, sizeof(MYSQL_BIND));
            binds[i].is_null = &null_flags[i];

            const auto& param = params[i];
            if (param.type == PreparedParam::Type::Int)
            {
                int_values[i] = param.int_value;
                binds[i].buffer_type = MYSQL_TYPE_LONG;
                binds[i].buffer = &int_values[i];
            }
            else if (param.type == PreparedParam::Type::Double)
            {
                double_values[i] = param.double_value;
                binds[i].buffer_type = MYSQL_TYPE_DOUBLE;
                binds[i].buffer = &double_values[i];
            }
            else
            {
                string_values[i] = param.string_value;
                string_lengths[i] = static_cast<unsigned long>(string_values[i].size());
                binds[i].buffer_type = MYSQL_TYPE_STRING;
                binds[i].buffer = const_cast<char*>(string_values[i].c_str());
                binds[i].buffer_length = string_lengths[i];
                binds[i].length = &string_lengths[i];
            }
        }

        if (mysql_stmt_bind_param(stmt, binds.data()) != 0)
        {
            setError(mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return results;
        }
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return results;
    }

    if (mysql_stmt_store_result(stmt) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return results;
    }

    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (!result)
    {
        mysql_stmt_close(stmt);
        return results;
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    std::vector<MYSQL_BIND> result_binds(num_fields);
    std::vector<std::vector<char>> buffers(static_cast<size_t>(num_fields));
    std::vector<unsigned long> lengths(static_cast<size_t>(num_fields), 0);
    std::vector<my_bool> is_null(static_cast<size_t>(num_fields), 0);

    for (int i = 0; i < num_fields; ++i)
    {
        unsigned long buffer_size = fields[i].max_length > 0 ? fields[i].max_length + 1 : 256;
        buffers[static_cast<size_t>(i)].resize(buffer_size);

        std::memset(&result_binds[static_cast<size_t>(i)], 0, sizeof(MYSQL_BIND));
        result_binds[static_cast<size_t>(i)].buffer_type = MYSQL_TYPE_STRING;
        result_binds[static_cast<size_t>(i)].buffer = buffers[static_cast<size_t>(i)].data();
        result_binds[static_cast<size_t>(i)].buffer_length = buffer_size;
        result_binds[static_cast<size_t>(i)].length = &lengths[static_cast<size_t>(i)];
        result_binds[static_cast<size_t>(i)].is_null = &is_null[static_cast<size_t>(i)];
    }

    if (mysql_stmt_bind_result(stmt, result_binds.data()) != 0)
    {
        setError(mysql_stmt_error(stmt));
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        return results;
    }

    int fetch_status = 0;
    while ((fetch_status = mysql_stmt_fetch(stmt)) == 0 || fetch_status == MYSQL_DATA_TRUNCATED)
    {
        std::map<std::string, std::string> row_data;
        for (int i = 0; i < num_fields; ++i)
        {
            if (is_null[static_cast<size_t>(i)])
            {
                row_data[fields[i].name] = "";
            }
            else
            {
                row_data[fields[i].name] = std::string(
                    buffers[static_cast<size_t>(i)].data(),
                    lengths[static_cast<size_t>(i)]);
            }
        }
        results.push_back(row_data);
    }

    if (fetch_status != MYSQL_NO_DATA && fetch_status != 0)
    {
        setError(mysql_stmt_error(stmt));
    }

    mysql_free_result(result);
    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return results;
}

std::map<std::string, std::string> DatabaseConnection::fetchOnePrepared(
    const std::string& query,
    const std::vector<PreparedParam>& params)
{
    auto results = fetchAllPrepared(query, params);
    return results.empty() ? std::map<std::string, std::string>() : results[0];
}

bool DatabaseConnection::prepareStatement(const std::string& query)
{
    if (!connected_)
    {
        setError("Not connected to database");
        return false;
    }
    
    if (stmt_)
    {
        mysql_stmt_close(stmt_);
    }
    
    stmt_ = mysql_stmt_init(mysql_);
    if (!stmt_)
    {
        setError("Failed to initialize prepared statement");
        return false;
    }
    
    if (mysql_stmt_prepare(stmt_, query.c_str(), query.length()) != 0)
    {
        setError(mysql_stmt_error(stmt_));
        return false;
    }
    
    return true;
}

bool DatabaseConnection::bindParameter(int index, const std::string& value)
{
    if (!stmt_)
    {
        setError("No prepared statement");
        return false;
    }
    
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer = const_cast<char*>(value.c_str());
    bind.buffer_length = value.length();
    bind.is_null = 0;
    
    if (mysql_stmt_bind_param(stmt_, &bind) != 0)
    {
        setError(mysql_stmt_error(stmt_));
        return false;
    }
    
    return true;
}

bool DatabaseConnection::bindParameter(int index, int value)
{
    if (!stmt_)
    {
        setError("No prepared statement");
        return false;
    }
    
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.buffer = &value;
    bind.is_null = 0;
    
    if (mysql_stmt_bind_param(stmt_, &bind) != 0)
    {
        setError(mysql_stmt_error(stmt_));
        return false;
    }
    
    return true;
}

std::vector<std::map<std::string, std::string>> DatabaseConnection::executePrepared()
{
    std::vector<std::map<std::string, std::string>> results;
    
    if (!stmt_)
    {
        setError("No prepared statement");
        return results;
    }
    
    if (mysql_stmt_execute(stmt_) != 0)
    {
        setError(mysql_stmt_error(stmt_));
        return results;
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt_);
    if (!result)
    {
        return results;
    }
    
    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);
    
    MYSQL_BIND* bind_result = new MYSQL_BIND[num_fields];
    char** buffer = new char*[num_fields];
    unsigned long* length = new unsigned long[num_fields];
    
    for (int i = 0; i < num_fields; ++i)
    {
        buffer[i] = new char[256];
        length[i] = 0;
        bind_result[i].buffer_type = MYSQL_TYPE_STRING;
        bind_result[i].buffer = buffer[i];
        bind_result[i].buffer_length = 256;
        bind_result[i].length = &length[i];
    }
    
    if (mysql_stmt_bind_result(stmt_, bind_result) != 0)
    {
        setError(mysql_stmt_error(stmt_));
        mysql_free_result(result);
        delete[] bind_result;
        for (int i = 0; i < num_fields; ++i)
        {
            delete[] buffer[i];
        }
        delete[] buffer;
        delete[] length;
        return results;
    }
    
    while (mysql_stmt_fetch(stmt_) == 0)
    {
        std::map<std::string, std::string> row_data;
        for (int i = 0; i < num_fields; ++i)
        {
            std::string field_name = fields[i].name;
            std::string value(buffer[i], length[i]);
            row_data[field_name] = value;
        }
        results.push_back(row_data);
    }
    
    mysql_free_result(result);
    delete[] bind_result;
    for (int i = 0; i < num_fields; ++i)
    {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] length;
    
    return results;
}

bool DatabaseConnection::beginTransaction()
{
    return executeQuery("START TRANSACTION");
}

bool DatabaseConnection::commit()
{
    return executeQuery("COMMIT");
}

bool DatabaseConnection::rollback()
{
    return executeQuery("ROLLBACK");
}

void DatabaseConnection::setError(const std::string& error)
{
    last_error_ = error;
    std::cerr << "Database Error: " << error << std::endl;
}
