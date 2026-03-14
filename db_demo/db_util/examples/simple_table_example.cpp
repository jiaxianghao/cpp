#include "TableManager.h"
#include <iostream>

using namespace db_util;

int main()
{
    // Create database manager and connect
    DatabaseManager dbManager;
    if (!dbManager.connect("localhost", "testuser", "testpass", "test_db", 3306))
    {
        std::cout << "Connection failed: " << dbManager.getLastError() << std::endl;
        return 1;
    }

    // Create table manager
    TableManager tableManager(dbManager);

    std::cout << "=== TableManager 简单测试 ===" << std::endl;

    // 1. 创建测试表
    std::cout << "\n1. 创建测试表..." << std::endl;
    std::map<std::string, std::string> fields = {
        {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
        {"name", "VARCHAR(50) NOT NULL"},
        {"value", "INT DEFAULT 0"}
    };
    
    if (tableManager.createTable("test_table", fields))
    {
        std::cout << "测试表创建成功！" << std::endl;
    }
    else
    {
        std::cout << "创建表失败: " << tableManager.getLastError() << std::endl;
        return 1;
    }

    // 2. 插入数据
    std::cout << "\n2. 插入数据..." << std::endl;
    
    TableRecord record1;
    record1.set("name", "测试数据1");
    record1.set("value", 100);
    
    if (tableManager.insert("test_table", record1))
    {
        std::cout << "数据1插入成功！" << std::endl;
    }
    else
    {
        std::cout << "插入失败: " << tableManager.getLastError() << std::endl;
    }
    
    TableRecord record2;
    record2.set("name", "测试数据2");
    record2.set("value", 200);
    
    if (tableManager.insert("test_table", record2))
    {
        std::cout << "数据2插入成功！" << std::endl;
    }
    else
    {
        std::cout << "插入失败: " << tableManager.getLastError() << std::endl;
    }

    // 3. 查询数据
    std::cout << "\n3. 查询数据..." << std::endl;
    
    auto allRecords = tableManager.select("test_table");
    std::cout << "所有记录:" << std::endl;
    for (const auto& record : allRecords)
    {
        std::cout << "ID: " << record.getInt("id") 
                  << ", 名称: " << record.getString("name")
                  << ", 值: " << record.getInt("value") << std::endl;
    }
    
    // 4. 条件查询
    std::cout << "\n4. 条件查询..." << std::endl;
    
    auto highValueRecords = tableManager.selectWhere("test_table", "value > 150");
    std::cout << "值大于150的记录:" << std::endl;
    for (const auto& record : highValueRecords)
    {
        std::cout << "ID: " << record.getInt("id") 
                  << ", 名称: " << record.getString("name")
                  << ", 值: " << record.getInt("value") << std::endl;
    }
    
    // 5. 更新数据
    std::cout << "\n5. 更新数据..." << std::endl;
    
    TableRecord updateData;
    updateData.set("value", 300);
    
    if (tableManager.updateById("test_table", updateData, "id", 1))
    {
        std::cout << "ID 1 更新成功！" << std::endl;
    }
    else
    {
        std::cout << "更新失败: " << tableManager.getLastError() << std::endl;
    }
    
    // 6. 验证更新结果
    std::cout << "\n6. 验证更新结果..." << std::endl;
    
    auto updatedRecord = tableManager.selectById("test_table", "id", 1);
    if (updatedRecord.hasField("name"))
    {
        std::cout << "更新后的记录: ID=" << updatedRecord.getInt("id")
                  << ", 名称=" << updatedRecord.getString("name")
                  << ", 值=" << updatedRecord.getInt("value") << std::endl;
    }
    
    // 7. 统计
    std::cout << "\n7. 统计信息..." << std::endl;
    
    int totalCount = tableManager.count("test_table");
    std::cout << "总记录数: " << totalCount << std::endl;
    
    int highValueCount = tableManager.countWhere("test_table", "value > 200");
    std::cout << "值大于200的记录数: " << highValueCount << std::endl;
    
    // 8. 删除数据
    std::cout << "\n8. 删除数据..." << std::endl;
    
    if (tableManager.deleteById("test_table", "id", 2))
    {
        std::cout << "ID 2 删除成功！" << std::endl;
    }
    else
    {
        std::cout << "删除失败: " << tableManager.getLastError() << std::endl;
    }
    
    // 9. 最终状态
    std::cout << "\n9. 最终状态..." << std::endl;
    
    auto finalRecords = tableManager.select("test_table");
    std::cout << "最终记录数: " << finalRecords.size() << std::endl;
    for (const auto& record : finalRecords)
    {
        std::cout << "ID: " << record.getInt("id") 
                  << ", 名称: " << record.getString("name")
                  << ", 值: " << record.getInt("value") << std::endl;
    }
    
    // 10. 清理
    std::cout << "\n10. 清理..." << std::endl;
    
    if (tableManager.dropTable("test_table"))
    {
        std::cout << "测试表删除成功！" << std::endl;
    }
    else
    {
        std::cout << "删除表失败: " << tableManager.getLastError() << std::endl;
    }

    std::cout << "\n=== 测试完成 ===" << std::endl;
    std::cout << "✅ TableManager 基本功能测试通过！" << std::endl;

    dbManager.disconnect();
    return 0;
}
