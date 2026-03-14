# MySQL Database Utility

A C++ class for MySQL database operations using MySQL Connector/C++.

## Features

- Connection management
- Basic query execution
- Prepared statements support
- Transaction management
- Error handling
- Result set processing

## Prerequisites

### Ubuntu/Debian
```bash
# Install MySQL Connector/C++
sudo apt-get update
sudo apt-get install libmysqlcppconn-dev

# Install build tools
sudo apt-get install build-essential cmake pkg-config
```

### CentOS/RHEL
```bash
# Install MySQL Connector/C++
sudo yum install mysql-connector-c++-devel

# Install build tools
sudo yum install gcc-c++ cmake pkgconfig
```

## Building the Project

1. Clone or download the project files
2. Create a build directory:
```bash
mkdir build
cd build
```

3. Configure and build:
```bash
cmake ..
make
```

## Usage

### Basic Usage

```cpp
#include "DatabaseManager.h"

int main()
{
    DatabaseManager dbManager;
    
    // Connect to database
    if (!dbManager.connect("localhost", "username", "password", "database_name"))
    {
        std::cout << "Connection failed: " << dbManager.getLastError() << std::endl;
        return 1;
    }
    
    // Execute a simple query
    if (dbManager.executeQuery("CREATE TABLE test (id INT, name VARCHAR(50))"))
    {
        std::cout << "Table created successfully" << std::endl;
    }
    
    // Disconnect
    dbManager.disconnect();
    return 0;
}
```

### Prepared Statements

```cpp
// Prepare a statement
auto stmt = dbManager.prepareStatement("INSERT INTO users (name, email) VALUES (?, ?)");
if (stmt)
{
    stmt->setString(1, "John Doe");
    stmt->setString(2, "john@example.com");
    
    if (dbManager.executePreparedStatement(stmt.get()))
    {
        std::cout << "Inserted with ID: " << dbManager.getLastInsertId() << std::endl;
    }
}
```

### Transactions

```cpp
// Begin transaction
if (dbManager.beginTransaction())
{
    // Execute multiple operations
    // ...
    
    // Commit or rollback
    if (success)
    {
        dbManager.commitTransaction();
    }
    else
    {
        dbManager.rollbackTransaction();
    }
}
```

### Selecting Data

```cpp
auto result = dbManager.executeSelect("SELECT * FROM users");
if (result)
{
    while (result->next())
    {
        int id = result->getInt("id");
        std::string name = result->getString("name");
        std::cout << "ID: " << id << ", Name: " << name << std::endl;
    }
}
```

## Configuration

Before running the example, update the database connection parameters in `main.cpp`:

```cpp
std::string host = "localhost";
std::string user = "your_username";
std::string password = "your_password";
std::string database = "your_database";
int port = 3306;
```

## Error Handling

The class provides comprehensive error handling:

```cpp
if (!dbManager.executeQuery("SELECT * FROM non_existent_table"))
{
    std::cout << "Error: " << dbManager.getLastError() << std::endl;
}
```

## Class Methods

### Connection Management
- `connect(host, user, password, database, port)` - Connect to database
- `disconnect()` - Close connection
- `isConnected()` - Check connection status

### Query Execution
- `executeQuery(query)` - Execute a simple query
- `executeSelect(query)` - Execute SELECT query and return result set
- `prepareStatement(query)` - Create a prepared statement
- `executePreparedStatement(stmt)` - Execute a prepared statement
- `executePreparedSelect(stmt)` - Execute a prepared SELECT statement

### Transaction Management
- `beginTransaction()` - Start a transaction
- `commitTransaction()` - Commit current transaction
- `rollbackTransaction()` - Rollback current transaction

### Utility Methods
- `getLastError()` - Get the last error message
- `getLastInsertId()` - Get the last inserted ID
- `getAffectedRows()` - Get the number of affected rows

## License

This project is provided as-is for educational and development purposes.
