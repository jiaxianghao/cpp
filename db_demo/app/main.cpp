#include "DatabaseService.h"
#include "UserModule.h"
#include "OrderModule.h"
#include <iostream>
#include <iomanip>

/**
 * @brief Print separator line
 */
void printSeparator()
{
    std::cout << "\n" << std::string(60, '=') << "\n" << std::endl;
}

/**
 * @brief Print section header
 */
void printSection(const std::string& title)
{
    printSeparator();
    std::cout << "  " << title << std::endl;
    printSeparator();
}

int main()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   分层架构数据库应用示例 - Layered Architecture Demo   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝\n" << std::endl;

    // Step 1: Initialize DatabaseService
    printSection("1. 初始化数据库服务");

    auto& dbService = core::DatabaseService::getInstance();

    // Update these with your actual database credentials
    if (!dbService.initialize("localhost", "testuser", "testpass", "test_db", 3306))
    {
        std::cerr << "❌ 数据库服务初始化失败" << std::endl;
        return 1;
    }

    std::cout << "✅ 数据库服务初始化成功" << std::endl;

    // Step 2: Initialize database tables
    printSection("2. 创建数据库表结构");

    if (!dbService.initializeTables())
    {
        std::cerr << "❌ 表结构创建失败" << std::endl;
        return 1;
    }

    std::cout << "✅ 所有表创建成功" << std::endl;

    // Step 3: Use UserModule
    printSection("3. 用户模块操作演示");

    modules::UserModule userModule;

    // Register users
    std::cout << "\n→ 注册新用户..." << std::endl;
    userModule.registerUser("张三", "zhangsan@example.com", 25);
    userModule.registerUser("李四", "lisi@example.com", 30);
    userModule.registerUser("王五", "wangwu@example.com", 28);

    // Batch register
    std::cout << "\n→ 批量注册用户..." << std::endl;
    std::vector<std::tuple<std::string, std::string, int>> batchUsers =
    {
        {"赵六", "zhaoliu@example.com", 35},
        {"孙七", "sunqi@example.com", 22},
        {"周八", "zhouba@example.com", 40}
    };
    int registered = userModule.batchRegisterUsers(batchUsers);
    std::cout << "批量注册成功: " << registered << " 个用户" << std::endl;

    // Get active users
    std::cout << "\n→ 查询所有活跃用户..." << std::endl;
    auto activeUsers = userModule.getActiveUsers();
    std::cout << "活跃用户数量: " << activeUsers.size() << std::endl;
    for (const auto& user : activeUsers)
    {
        std::cout << "  - ID: " << user.id
                  << ", 姓名: " << user.name
                  << ", 邮箱: " << user.email
                  << ", 年龄: " << user.age << std::endl;
    }

    // Search by age
    std::cout << "\n→ 搜索年龄在25-35之间的用户..." << std::endl;
    auto searchResults = userModule.searchUsersByAge(25, 35);
    std::cout << "找到 " << searchResults.size() << " 个用户" << std::endl;
    for (const auto& user : searchResults)
    {
        std::cout << "  - " << user.name << " (年龄: " << user.age << ")" << std::endl;
    }

    // Step 4: Use OrderModule
    printSection("4. 订单模块操作演示");

    modules::OrderModule orderModule;

    // Create orders
    std::cout << "\n→ 创建订单..." << std::endl;
    orderModule.createOrder(1, "笔记本电脑", 1, 5999.99);
    orderModule.createOrder(1, "无线鼠标", 2, 199.98);
    orderModule.createOrder(2, "机械键盘", 1, 899.00);
    orderModule.createOrder(3, "显示器", 1, 1899.00);

    // Get user orders
    std::cout << "\n→ 查询用户ID=1的所有订单..." << std::endl;
    auto userOrders = orderModule.getUserOrders(1);
    std::cout << "用户订单数量: " << userOrders.size() << std::endl;
    for (const auto& order : userOrders)
    {
        std::cout << "  - 订单ID: " << order.id
                  << ", 商品: " << order.productName
                  << ", 数量: " << order.quantity
                  << ", 总价: ¥" << std::fixed << std::setprecision(2) << order.totalPrice
                  << ", 状态: " << order.status << std::endl;
    }

    // Get pending orders
    std::cout << "\n→ 查询所有待处理订单..." << std::endl;
    auto pendingOrders = orderModule.getPendingOrders();
    std::cout << "待处理订单数量: " << pendingOrders.size() << std::endl;

    // Complete some orders
    std::cout << "\n→ 完成订单..." << std::endl;
    if (!pendingOrders.empty())
    {
        orderModule.completeOrder(pendingOrders[0].id);
        if (pendingOrders.size() > 1)
        {
            orderModule.completeOrder(pendingOrders[1].id);
        }
    }

    // Get completed orders
    std::cout << "\n→ 查询已完成订单..." << std::endl;
    auto completedOrders = orderModule.getCompletedOrders();
    std::cout << "已完成订单数量: " << completedOrders.size() << std::endl;

    // Step 5: Cross-module operations
    printSection("5. 跨模块业务操作演示");

    std::cout << "\n→ 获取用户资料和订单统计..." << std::endl;
    auto user = userModule.getUserProfile(1);
    if (user.has_value())
    {
        std::cout << "用户信息:" << std::endl;
        std::cout << "  姓名: " << user->name << std::endl;
        std::cout << "  邮箱: " << user->email << std::endl;
        std::cout << "  年龄: " << user->age << std::endl;

        // Get user's total spending (only completed orders)
        double totalSpending = orderModule.getUserTotalSpending(user->id);
        std::cout << "  总消费金额: ¥" << std::fixed << std::setprecision(2) << totalSpending << std::endl;
    }

    // Step 6: Transaction example
    printSection("6. 事务处理演示");

    std::cout << "\n→ 使用事务批量更新..." << std::endl;
    if (dbService.beginTransaction())
    {
        std::cout << "事务开始..." << std::endl;

        bool success = true;

        // Create multiple orders in a transaction
        success &= orderModule.createOrder(2, "平板电脑", 1, 3999.00);
        success &= orderModule.createOrder(2, "手机壳", 3, 89.97);

        if (success)
        {
            dbService.commitTransaction();
            std::cout << "✅ 事务提交成功" << std::endl;
        }
        else
        {
            dbService.rollbackTransaction();
            std::cout << "❌ 事务回滚" << std::endl;
        }
    }

    // Step 7: Statistics
    printSection("7. 统计信息");

    std::cout << "\n→ 系统统计数据..." << std::endl;
    int activeUserCount = userModule.getActiveUserCount();
    int pendingOrderCount = orderModule.getOrderCountByStatus("pending");
    int completedOrderCount = orderModule.getOrderCountByStatus("completed");

    std::cout << "  活跃用户总数: " << activeUserCount << std::endl;
    std::cout << "  待处理订单数: " << pendingOrderCount << std::endl;
    std::cout << "  已完成订单数: " << completedOrderCount << std::endl;

    // Summary
    printSection("总结");

    std::cout << "✅ 架构优势展示:\n" << std::endl;
    std::cout << "  1. 职责分离 - 业务模块只关注业务逻辑" << std::endl;
    std::cout << "  2. 易于维护 - 数据访问逻辑集中在DAO层" << std::endl;
    std::cout << "  3. 代码复用 - 多个模块共享同一套DAO" << std::endl;
    std::cout << "  4. 统一管理 - DatabaseService统一管理连接" << std::endl;
    std::cout << "  5. 类型安全 - 使用业务实体而非通用记录" << std::endl;
    std::cout << "  6. 易于测试 - 可以mock DAO进行单元测试" << std::endl;

    printSeparator();
    std::cout << "示例程序执行完成！\n" << std::endl;

    return 0;
}
