#include "DatabaseManager.h"
#include <iostream>
#include <string>

using namespace db_util;

int main()
{
    DatabaseManager dbManager;

    // Connect to database
    if (!dbManager.connect("localhost", "testuser", "testpass", "test_db", 3306))
    {
        std::cout << "Connection failed: " << dbManager.getLastError() << std::endl;
        return 1;
    }

    std::cout << "Connected successfully!" << std::endl;

    // Test simple query
    std::cout << "Testing simple query..." << std::endl;
    if (dbManager.executeQuery("SELECT 1"))
    {
        std::cout << "Simple query succeeded" << std::endl;
    }
    else
    {
        std::cout << "Simple query failed: " << dbManager.getLastError() << std::endl;
    }

    // Test create table
    std::cout << "Testing create table..." << std::endl;
    if (dbManager.executeQuery("CREATE TABLE debug_test (id INT)"))
    {
        std::cout << "Create table succeeded" << std::endl;
    }
    else
    {
        std::cout << "Create table failed: " << dbManager.getLastError() << std::endl;
    }

    // Test insert
    std::cout << "Testing insert..." << std::endl;
    if (dbManager.executeQuery("INSERT INTO debug_test VALUES (1)"))
    {
        std::cout << "Insert succeeded" << std::endl;
    }
    else
    {
        std::cout << "Insert failed: " << dbManager.getLastError() << std::endl;
    }

    // Test select
    std::cout << "Testing select..." << std::endl;
    auto result = dbManager.executeSelect("SELECT * FROM debug_test");
    if (result)
    {
        std::cout << "Select succeeded" << std::endl;
        while (result->next())
        {
            std::cout << "ID: " << result->getInt("id") << std::endl;
        }
    }
    else
    {
        std::cout << "Select failed: " << dbManager.getLastError() << std::endl;
    }

    // Clean up
    dbManager.executeQuery("DROP TABLE IF EXISTS debug_test");
    dbManager.disconnect();

    return 0;
}
