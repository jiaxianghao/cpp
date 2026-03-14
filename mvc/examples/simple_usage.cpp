#include <iostream>
#include <memory>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "repositories/user_repository.h"
#include "services/user_service.h"
#include "controllers/user_controller.h"

/**
 * 简单使用示例 - 展示最基础的使用方法
 * 这是你作为上层开发者最常用的方式
 */

int main()
{
    std::cout << "=== 简单使用示例 ===" << std::endl;
    std::cout << "展示最基础的使用方法" << std::endl;
    
    try
    {
        // 第一步: 配置数据库 (只需要做一次)
        DatabaseConfig& config = DatabaseConfig::getInstance();
        config.setHost("localhost");
        config.setUser("root");
        config.setPassword("password");
        config.setDatabase("myapp");
        
        // 第二步: 创建数据库连接
        auto connection = std::make_shared<DatabaseConnection>();
        if (!connection->connect())
        {
            std::cerr << "数据库连接失败: " << connection->getLastError() << std::endl;
            return 1;
        }
        
        // 第三步: 创建Repository (数据访问层)
        auto user_repo = std::make_shared<UserRepository>(connection);
        
        // 第四步: 创建Service (业务逻辑层)
        auto user_service = std::make_shared<UserService>(user_repo);
        
        // 第五步: 创建Controller (控制器层) - 这是你主要使用的接口
        auto user_controller = std::make_shared<UserController>(user_service);
        
        std::cout << "✅ 系统初始化完成!" << std::endl;
        
        // ===== 现在你可以开始使用系统了 =====
        
        std::cout << "\n--- 基础操作示例 ---" << std::endl;
        
        // 1. 创建用户 - 这是你主要使用的功能
        std::cout << "\n1. 创建用户" << std::endl;
        auto result = user_controller->createUser("张三", "zhangsan@example.com");
        if (result.success)
        {
            std::cout << "✅ 用户创建成功: " << result.user.toString() << std::endl;
        }
        else
        {
            std::cout << "❌ 用户创建失败: " << result.error_message << std::endl;
        }
        
        // 2. 获取用户信息
        std::cout << "\n2. 获取用户信息" << std::endl;
        User user = user_controller->getUserById(1);
        if (user.getId() > 0)
        {
            std::cout << "✅ 找到用户: " << user.toString() << std::endl;
        }
        else
        {
            std::cout << "❌ 用户不存在" << std::endl;
        }
        
        // 3. 搜索用户
        std::cout << "\n3. 搜索用户" << std::endl;
        auto users = user_controller->searchUsers("张");
        std::cout << "🔍 搜索结果: 找到 " << users.size() << " 个用户" << std::endl;
        for (const auto& u : users)
        {
            std::cout << "  - " << u.toString() << std::endl;
        }
        
        // 4. 更新用户信息
        std::cout << "\n4. 更新用户信息" << std::endl;
        if (user.getId() > 0)
        {
            user.setEmail("zhangsan.new@example.com");
            auto update_result = user_controller->updateUser(user);
            if (update_result.success)
            {
                std::cout << "✅ 用户信息更新成功" << std::endl;
            }
            else
            {
                std::cout << "❌ 用户信息更新失败: " << update_result.error_message << std::endl;
            }
        }
        
        // 5. 获取统计信息
        std::cout << "\n5. 获取统计信息" << std::endl;
        std::string stats = user_controller->getUserStatistics();
        std::cout << "📊 " << stats << std::endl;
        
        std::cout << "\n🎉 示例完成!" << std::endl;
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ 系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
