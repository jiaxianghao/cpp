#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "repositories/user_repository.h"
#include "services/user_service.h"
#include "controllers/user_controller.h"

/**
 * 业务应用示例 - 展示如何在实际项目中使用分层架构
 * 这个示例模拟了一个用户管理系统的实际使用场景
 */
class BusinessApplication
{
private:
    std::shared_ptr<UserController> user_controller_;
    
public:
    BusinessApplication()
    {
        initializeDatabase();
        setupUserController();
    }
    
    /**
     * 初始化数据库连接
     */
    void initializeDatabase()
    {
        // 配置数据库连接参数
        DatabaseConfig& config = DatabaseConfig::getInstance();
        config.setHost("localhost");
        config.setUser("root");
        config.setPassword("password");
        config.setDatabase("myapp");
        config.setPort(3306);
        
        std::cout << "数据库配置已加载" << std::endl;
    }
    
    /**
     * 设置用户控制器
     */
    void setupUserController()
    {
        // 创建数据库连接
        auto connection = std::make_shared<DatabaseConnection>();
        if (!connection->connect())
        {
            throw std::runtime_error("数据库连接失败: " + connection->getLastError());
        }
        
        // 创建Repository层
        auto user_repo = std::make_shared<UserRepository>(connection);
        
        // 创建Service层
        auto user_service = std::make_shared<UserService>(user_repo);
        
        // 创建Controller层
        user_controller_ = std::make_shared<UserController>(user_service);
        
        std::cout << "用户管理系统初始化完成" << std::endl;
    }
    
    /**
     * 用户注册功能
     */
    bool registerUser(const std::string& username, const std::string& email)
    {
        std::cout << "\n=== 用户注册 ===" << std::endl;
        std::cout << "用户名: " << username << std::endl;
        std::cout << "邮箱: " << email << std::endl;
        
        // 检查用户名是否可用
        if (!user_controller_->isUsernameAvailable(username))
        {
            std::cout << "❌ 用户名已存在: " << username << std::endl;
            return false;
        }
        
        // 检查邮箱是否可用
        if (!user_controller_->isEmailAvailable(email))
        {
            std::cout << "❌ 邮箱已存在: " << email << std::endl;
            return false;
        }
        
        // 创建用户
        auto result = user_controller_->createUser(username, email);
        if (result.success)
        {
            std::cout << "✅ 用户注册成功!" << std::endl;
            std::cout << "用户ID: " << result.user.getId() << std::endl;
            std::cout << "创建时间: " << result.user.getCreatedAt() << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ 用户注册失败: " << result.error_message << std::endl;
            return false;
        }
    }
    
    /**
     * 用户登录功能
     */
    bool loginUser(const std::string& username)
    {
        std::cout << "\n=== 用户登录 ===" << std::endl;
        std::cout << "尝试登录用户: " << username << std::endl;
        
        User user = user_controller_->getUserByUsername(username);
        if (user.getId() > 0)
        {
            std::cout << "✅ 登录成功!" << std::endl;
            std::cout << "欢迎, " << user.getUsername() << "!" << std::endl;
            std::cout << "邮箱: " << user.getEmail() << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ 登录失败: 用户不存在" << std::endl;
            return false;
        }
    }
    
    /**
     * 更新用户信息
     */
    bool updateUserProfile(int user_id, const std::string& new_email)
    {
        std::cout << "\n=== 更新用户信息 ===" << std::endl;
        std::cout << "用户ID: " << user_id << std::endl;
        std::cout << "新邮箱: " << new_email << std::endl;
        
        // 获取现有用户信息
        User user = user_controller_->getUserById(user_id);
        if (user.getId() <= 0)
        {
            std::cout << "❌ 用户不存在" << std::endl;
            return false;
        }
        
        // 检查新邮箱是否可用
        if (!user_controller_->isEmailAvailable(new_email))
        {
            std::cout << "❌ 邮箱已被使用: " << new_email << std::endl;
            return false;
        }
        
        // 更新用户信息
        user.setEmail(new_email);
        auto result = user_controller_->updateUser(user);
        
        if (result.success)
        {
            std::cout << "✅ 用户信息更新成功!" << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ 用户信息更新失败: " << result.error_message << std::endl;
            return false;
        }
    }
    
    /**
     * 搜索用户功能
     */
    void searchUsers(const std::string& keyword)
    {
        std::cout << "\n=== 搜索用户 ===" << std::endl;
        std::cout << "搜索关键词: " << keyword << std::endl;
        
        auto users = user_controller_->searchUsers(keyword);
        
        if (users.empty())
        {
            std::cout << "🔍 未找到匹配的用户" << std::endl;
        }
        else
        {
            std::cout << "🔍 找到 " << users.size() << " 个匹配的用户:" << std::endl;
            for (const auto& user : users)
            {
                std::cout << "  - ID: " << user.getId() 
                         << ", 用户名: " << user.getUsername()
                         << ", 邮箱: " << user.getEmail() << std::endl;
            }
        }
    }
    
    /**
     * 显示用户统计信息
     */
    void showUserStatistics()
    {
        std::cout << "\n=== 用户统计信息 ===" << std::endl;
        std::string stats = user_controller_->getUserStatistics();
        std::cout << stats << std::endl;
    }
    
    /**
     * 删除用户功能
     */
    bool deleteUser(int user_id)
    {
        std::cout << "\n=== 删除用户 ===" << std::endl;
        std::cout << "要删除的用户ID: " << user_id << std::endl;
        
        // 确认用户存在
        if (!user_controller_->userExists(user_id))
        {
            std::cout << "❌ 用户不存在" << std::endl;
            return false;
        }
        
        // 获取用户信息用于确认
        User user = user_controller_->getUserById(user_id);
        std::cout << "确认删除用户: " << user.getUsername() << " (" << user.getEmail() << ")" << std::endl;
        
        // 执行删除
        auto result = user_controller_->deleteUser(user_id);
        if (result.success)
        {
            std::cout << "✅ 用户删除成功!" << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ 用户删除失败: " << result.error_message << std::endl;
            return false;
        }
    }
    
    /**
     * 显示所有用户
     */
    void listAllUsers()
    {
        std::cout << "\n=== 所有用户列表 ===" << std::endl;
        
        auto users = user_controller_->getAllUsers();
        if (users.empty())
        {
            std::cout << "📋 暂无用户" << std::endl;
        }
        else
        {
            std::cout << "📋 共有 " << users.size() << " 个用户:" << std::endl;
            for (const auto& user : users)
            {
                std::cout << "  " << user.toString() << std::endl;
            }
        }
    }
};

/**
 * 演示业务场景
 */
void demonstrateBusinessScenarios()
{
    try
    {
        BusinessApplication app;
        
        std::cout << "🚀 开始业务场景演示" << std::endl;
        
        // 场景1: 用户注册
        app.registerUser("alice", "alice@example.com");
        app.registerUser("bob", "bob@example.com");
        app.registerUser("charlie", "charlie@example.com");
        
        // 场景2: 用户登录
        app.loginUser("alice");
        app.loginUser("nonexistent_user");
        
        // 场景3: 更新用户信息
        app.updateUserProfile(1, "alice.new@example.com");
        
        // 场景4: 搜索用户
        app.searchUsers("alice");
        app.searchUsers("example.com");
        
        // 场景5: 显示统计信息
        app.showUserStatistics();
        
        // 场景6: 列出所有用户
        app.listAllUsers();
        
        // 场景7: 删除用户
        app.deleteUser(3);
        
        // 场景8: 最终统计
        app.showUserStatistics();
        
        std::cout << "\n🎉 业务场景演示完成!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ 应用程序错误: " << e.what() << std::endl;
    }
}

int main()
{
    std::cout << "=== 业务应用示例 ===" << std::endl;
    std::cout << "这个示例展示了如何在实际业务中使用分层架构" << std::endl;
    
    demonstrateBusinessScenarios();
    
    return 0;
}
