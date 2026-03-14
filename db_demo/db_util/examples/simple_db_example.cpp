#include "DatabaseManager.h"
#include <iostream>
#include <string>

using namespace db_util;

int main()
{
    // Create database manager instance
    DatabaseManager dbManager;

    // Connect to database
    std::string host = "localhost";
    std::string user = "testuser";
    std::string password = "testpass";
    std::string database = "test_db";
    int port = 3306;

    std::cout << "Connecting to MySQL database..." << std::endl;
    
    if (!dbManager.connect(host, user, password, database, port))
    {
        std::cout << "Connection failed: " << dbManager.getLastError() << std::endl;
        return 1;
    }

    std::cout << "Successfully connected to database!" << std::endl;

    // Example 1: Execute a simple query
    std::cout << "\n--- Example 1: Creating a test table ---" << std::endl;
    
    std::string createTableQuery = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "name VARCHAR(100) NOT NULL,"
        "email VARCHAR(100) UNIQUE NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (dbManager.executeQuery(createTableQuery))
    {
        std::cout << "Table 'users' created successfully" << std::endl;
    }
    else
    {
        std::cout << "Failed to create table: " << dbManager.getLastError() << std::endl;
    }

    // Example 2: Insert data using prepared statement
    std::cout << "\n--- Example 2: Inserting data with prepared statement ---" << std::endl;
    
    std::string insertQuery = "INSERT INTO users (name, email) VALUES (?, ?)";
    auto stmt = dbManager.prepareStatement(insertQuery);
    
    if (stmt)
    {
        stmt->setString(1, "John Doe");
        stmt->setString(2, "john.doe@example.com");
        
        if (dbManager.executePreparedStatement(stmt.get()))
        {
            std::cout << "User inserted successfully. ID: " << dbManager.getLastInsertId() << std::endl;
        }
        else
        {
            std::cout << "Failed to insert user: " << dbManager.getLastError() << std::endl;
        }
    }

    // Example 3: Select data
    std::cout << "\n--- Example 3: Selecting data ---" << std::endl;
    
    std::string selectQuery = "SELECT id, name, email, created_at FROM users";
    auto result = dbManager.executeSelect(selectQuery);
    
    if (result)
    {
        std::cout << "Users in database:" << std::endl;
        std::cout << "ID\tName\t\tEmail\t\t\tCreated At" << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        
        while (result->next())
        {
            int id = result->getInt("id");
            std::string name = result->getString("name");
            std::string email = result->getString("email");
            std::string createdAt = result->getString("created_at");
            
            std::cout << id << "\t" << name << "\t" << email << "\t" << createdAt << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to select data: " << dbManager.getLastError() << std::endl;
    }

    // Example 4: Transaction example
    std::cout << "\n--- Example 4: Transaction example ---" << std::endl;
    
    if (dbManager.beginTransaction())
    {
        std::cout << "Transaction started" << std::endl;
        
        // Insert multiple users in transaction
        std::string insertQuery2 = "INSERT INTO users (name, email) VALUES (?, ?)";
        auto stmt2 = dbManager.prepareStatement(insertQuery2);
        
        if (stmt2)
        {
            bool allSuccess = true;
            
            // Insert user 1
            stmt2->setString(1, "Jane Smith");
            stmt2->setString(2, "jane.smith@example.com");
            if (!dbManager.executePreparedStatement(stmt2.get()))
            {
                allSuccess = false;
            }
            
            // Insert user 2
            stmt2->setString(1, "Bob Johnson");
            stmt2->setString(2, "bob.johnson@example.com");
            if (!dbManager.executePreparedStatement(stmt2.get()))
            {
                allSuccess = false;
            }
            
            if (allSuccess)
            {
                if (dbManager.commitTransaction())
                {
                    std::cout << "Transaction committed successfully" << std::endl;
                }
                else
                {
                    std::cout << "Failed to commit transaction: " << dbManager.getLastError() << std::endl;
                }
            }
            else
            {
                if (dbManager.rollbackTransaction())
                {
                    std::cout << "Transaction rolled back due to errors" << std::endl;
                }
                else
                {
                    std::cout << "Failed to rollback transaction: " << dbManager.getLastError() << std::endl;
                }
            }
        }
    }
    else
    {
        std::cout << "Failed to begin transaction: " << dbManager.getLastError() << std::endl;
    }

    // Example 5: Update data
    std::cout << "\n--- Example 5: Updating data ---" << std::endl;
    
    std::string updateQuery = "UPDATE users SET name = ? WHERE email = ?";
    auto updateStmt = dbManager.prepareStatement(updateQuery);
    
    if (updateStmt)
    {
        updateStmt->setString(1, "John Updated Doe");
        updateStmt->setString(2, "john.doe@example.com");
        
        if (dbManager.executePreparedStatement(updateStmt.get()))
        {
            std::cout << "User updated successfully. Affected rows: " << dbManager.getAffectedRows() << std::endl;
        }
        else
        {
            std::cout << "Failed to update user: " << dbManager.getLastError() << std::endl;
        }
    }

    // Example 6: Delete data
    std::cout << "\n--- Example 6: Deleting data ---" << std::endl;
    
    std::string deleteQuery = "DELETE FROM users WHERE email = ?";
    auto deleteStmt = dbManager.prepareStatement(deleteQuery);
    
    if (deleteStmt)
    {
        deleteStmt->setString(1, "bob.johnson@example.com");
        
        if (dbManager.executePreparedStatement(deleteStmt.get()))
        {
            std::cout << "User deleted successfully. Affected rows: " << dbManager.getAffectedRows() << std::endl;
        }
        else
        {
            std::cout << "Failed to delete user: " << dbManager.getLastError() << std::endl;
        }
    }

    // Disconnect from database
    std::cout << "\nDisconnecting from database..." << std::endl;
    dbManager.disconnect();
    std::cout << "Disconnected successfully" << std::endl;

    return 0;
}
