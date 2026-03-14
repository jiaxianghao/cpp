#include "TableManager.h"
#include <iostream>
#include <vector>

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

    std::cout << "=== 简单易用的数据库操作示例 ===" << std::endl;

    // 1. 创建表 - 不需要写SQL
    std::cout << "\n1. 创建用户表..." << std::endl;
    std::map<std::string, std::string> userFields = {
        {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
        {"name", "VARCHAR(100) NOT NULL"},
        {"email", "VARCHAR(100) UNIQUE NOT NULL"},
        {"age", "INT"},
        {"is_active", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
    };
    
    if (tableManager.createTable("easy_users", userFields))
    {
        std::cout << "用户表创建成功！" << std::endl;
    }
    else
    {
        std::cout << "创建表失败: " << tableManager.getLastError() << std::endl;
    }

    // 2. 插入数据 - 像操作对象一样简单
    std::cout << "\n2. 插入用户数据..." << std::endl;
    
    // 创建用户记录
    TableRecord user1;
    user1.set("name", "张三");
    user1.set("email", "zhangsan@example.com");
    user1.set("age", 25);
    user1.set("is_active", true);
    
    if (tableManager.insert("easy_users", user1))
    {
        std::cout << "用户张三插入成功！" << std::endl;
    }

    // 批量插入
    std::vector<TableRecord> users;
    
    TableRecord user2;
    user2.set("name", "李四");
    user2.set("email", "lisi@example.com");
    user2.set("age", 30);
    user2.set("is_active", true);
    users.push_back(user2);
    
    TableRecord user3;
    user3.set("name", "王五");
    user3.set("email", "wangwu@example.com");
    user3.set("age", 28);
    user3.set("is_active", false);
    users.push_back(user3);
    
    if (tableManager.insertBatch("easy_users", users))
    {
        std::cout << "批量插入成功！" << std::endl;
    }

    // 3. 查询数据 - 多种简单方式
    std::cout << "\n3. 查询用户数据..." << std::endl;
    
    // 查询所有用户
    std::cout << "\n所有用户:" << std::endl;
    auto allUsers = tableManager.select("easy_users");
    for (const auto& user : allUsers)
    {
        std::cout << "ID: " << user.getInt("id") 
                  << ", 姓名: " << user.getString("name")
                  << ", 邮箱: " << user.getString("email")
                  << ", 年龄: " << user.getInt("age")
                  << ", 状态: " << (user.getBool("is_active") ? "活跃" : "非活跃")
                  << std::endl;
    }
    
    // 查询特定字段
    std::cout << "\n只查询姓名和邮箱:" << std::endl;
    auto nameEmails = tableManager.select("easy_users", {"name", "email"});
    for (const auto& user : nameEmails)
    {
        std::cout << "姓名: " << user.getString("name")
                  << ", 邮箱: " << user.getString("email") << std::endl;
    }
    
    // 条件查询
    std::cout << "\n查询活跃用户:" << std::endl;
    auto activeUsers = tableManager.selectWhere("easy_users", "is_active = 1");
    for (const auto& user : activeUsers)
    {
        std::cout << "活跃用户: " << user.getString("name") << std::endl;
    }
    
    // 查询单个用户
    std::cout << "\n查询ID为1的用户:" << std::endl;
    auto user = tableManager.selectById("easy_users", "id", 1);
    if (user.hasField("name"))
    {
        std::cout << "找到用户: " << user.getString("name") << std::endl;
    }
    
    // 查询年龄大于25的用户
    std::cout << "\n查询年龄大于25的用户:" << std::endl;
    auto olderUsers = tableManager.selectWhere("easy_users", "age > 25");
    for (const auto& u : olderUsers)
    {
        std::cout << u.getString("name") << " 年龄: " << u.getInt("age") << std::endl;
    }

    // 4. 更新数据 - 简单直观
    std::cout << "\n4. 更新用户数据..." << std::endl;
    
    // 更新特定用户
    TableRecord updateData;
    updateData.set("age", 26);
    updateData.set("is_active", false);
    
    if (tableManager.updateById("easy_users", updateData, "id", 1))
    {
        std::cout << "用户ID 1 更新成功！" << std::endl;
    }
    
    // 批量更新（更新所有非活跃用户为活跃）
    TableRecord activateData;
    activateData.set("is_active", true);
    
    if (tableManager.update("easy_users", activateData, "is_active = 0"))
    {
        std::cout << "所有非活跃用户已激活！" << std::endl;
    }

    // 5. 删除数据 - 简单安全
    std::cout << "\n5. 删除用户数据..." << std::endl;
    
    // 删除特定用户
    if (tableManager.deleteById("easy_users", "id", 3))
    {
        std::cout << "用户ID 3 删除成功！" << std::endl;
    }
    
    // 条件删除
    if (tableManager.deleteWhere("easy_users", "age < 25"))
    {
        std::cout << "删除年龄小于25的用户成功！" << std::endl;
    }

    // 6. 统计查询 - 内置函数
    std::cout << "\n6. 统计信息..." << std::endl;
    
    int totalUsers = tableManager.count("easy_users");
    std::cout << "总用户数: " << totalUsers << std::endl;
    
    int activeCount = tableManager.countWhere("easy_users", "is_active = 1");
    std::cout << "活跃用户数: " << activeCount << std::endl;

    // 7. 复杂查询 - 使用查询构建器
    std::cout << "\n7. 复杂查询示例..." << std::endl;
    
    // 构建复杂查询：查询年龄大于25的活跃用户，按年龄降序排列，限制2条
    auto query = tableManager.query()
        .select({"name", "email", "age"})
        .from("easy_users")
        .where("age > 25")
        .andWhere("is_active = 1")
        .orderBy("age", false)  // 降序
        .limit(2);
    
    std::string complexQuery = query.build();
    std::cout << "生成的SQL: " << complexQuery << std::endl;
    
    // 执行查询
    auto result = dbManager.executeSelect(complexQuery);
    if (result)
    {
        std::cout << "查询结果:" << std::endl;
        while (result->next())
        {
            std::cout << "姓名: " << result->getString("name")
                      << ", 邮箱: " << result->getString("email")
                      << ", 年龄: " << result->getInt("age") << std::endl;
        }
    }

    // 8. 事务处理 - 保证数据一致性
    std::cout << "\n8. 事务处理示例..." << std::endl;
    
    if (tableManager.beginTransaction())
    {
        std::cout << "开始事务..." << std::endl;
        
        // 在事务中执行多个操作
        TableRecord newUser1;
        newUser1.set("name", "事务用户1");
        newUser1.set("email", "transaction1@example.com");
        newUser1.set("age", 35);
        
        TableRecord newUser2;
        newUser2.set("name", "事务用户2");
        newUser2.set("email", "transaction2@example.com");
        newUser2.set("age", 40);
        
        bool success = tableManager.insert("easy_users", newUser1) &&
                      tableManager.insert("easy_users", newUser2);
        
        if (success)
        {
            tableManager.commitTransaction();
            std::cout << "事务提交成功！" << std::endl;
        }
        else
        {
            tableManager.rollbackTransaction();
            std::cout << "事务回滚！" << std::endl;
        }
    }

    // 9. 最终查询结果
    std::cout << "\n9. 最终数据状态..." << std::endl;
    auto finalUsers = tableManager.select("easy_users");
    std::cout << "最终用户列表:" << std::endl;
    for (const auto& user : finalUsers)
    {
        std::cout << "ID: " << user.getInt("id") 
                  << ", 姓名: " << user.getString("name")
                  << ", 邮箱: " << user.getString("email")
                  << ", 年龄: " << user.getInt("age")
                  << ", 状态: " << (user.getBool("is_active") ? "活跃" : "非活跃")
                  << std::endl;
    }

    // 10. 清理 - 删除测试表
    std::cout << "\n10. 清理测试数据..." << std::endl;
    if (tableManager.dropTable("easy_users"))
    {
        std::cout << "测试表删除成功！" << std::endl;
    }

    std::cout << "\n=== 示例完成 ===" << std::endl;
    std::cout << "\n总结：通过TableManager，团队成员可以：\n";
    std::cout << "✅ 不需要写SQL就能操作数据库\n";
    std::cout << "✅ 使用简单直观的API\n";
    std::cout << "✅ 自动处理SQL注入防护\n";
    std::cout << "✅ 支持事务处理\n";
    std::cout << "✅ 提供类型安全的数据访问\n";
    std::cout << "✅ 支持批量操作\n";
    std::cout << "✅ 内置统计功能\n";

    dbManager.disconnect();
    return 0;
}
