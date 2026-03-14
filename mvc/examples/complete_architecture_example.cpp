#include <iostream>
#include <memory>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "repositories/user_repository.h"
#include "repositories/product_repository.h"
#include "repositories/order_repository.h"
#include "services/user_service.h"
#include "services/product_service.h"
#include "services/order_service.h"
#include "services/application_service.h"
#include "controllers/user_controller.h"
#include "controllers/product_controller.h"
#include "controllers/order_controller.h"

/**
 * 完整架构示例 - 展示如何使用所有层
 * 这个示例展示了完整的架构使用方式
 */

class CompleteArchitectureApplication
{
private:
    // Controllers
    std::shared_ptr<UserController> user_controller_;
    std::shared_ptr<ProductController> product_controller_;
    std::shared_ptr<OrderController> order_controller_;
    
    // Application Service
    std::shared_ptr<ApplicationService> app_service_;
    
public:
    CompleteArchitectureApplication()
    {
        initializeDatabase();
        setupControllers();
        setupApplicationService();
    }
    
    void initializeDatabase()
    {
        DatabaseConfig& config = DatabaseConfig::getInstance();
        config.setHost("localhost");
        config.setUser("root");
        config.setPassword("password");
        config.setDatabase("myapp");
        config.setPort(3306);
        
        std::cout << "数据库配置已加载" << std::endl;
    }
    
    void setupControllers()
    {
        // 创建数据库连接
        auto connection = std::make_shared<DatabaseConnection>();
        if (!connection->connect())
        {
            throw std::runtime_error("数据库连接失败: " + connection->getLastError());
        }
        
        // 创建Repository层
        auto user_repo = std::make_shared<UserRepository>(connection);
        auto product_repo = std::make_shared<ProductRepository>(connection);
        auto order_repo = std::make_shared<OrderRepository>(connection);
        
        // 创建Service层
        auto user_service = std::make_shared<UserService>(user_repo);
        auto product_service = std::make_shared<ProductService>(product_repo);
        auto order_service = std::make_shared<OrderService>(order_repo);
        
        // 创建Controller层
        user_controller_ = std::make_shared<UserController>(user_service);
        product_controller_ = std::make_shared<ProductController>(product_service);
        order_controller_ = std::make_shared<OrderController>(order_service);
        
        std::cout << "控制器层初始化完成" << std::endl;
    }
    
    void setupApplicationService()
    {
        // 创建数据库连接
        auto connection = std::make_shared<DatabaseConnection>();
        if (!connection->connect())
        {
            throw std::runtime_error("数据库连接失败: " + connection->getLastError());
        }
        
        // 创建Repository层
        auto user_repo = std::make_shared<UserRepository>(connection);
        auto product_repo = std::make_shared<ProductRepository>(connection);
        auto order_repo = std::make_shared<OrderRepository>(connection);
        
        // 创建应用服务层
        app_service_ = std::make_shared<ApplicationService>(user_repo, product_repo, order_repo);
        
        std::cout << "应用服务层初始化完成" << std::endl;
    }
    
    /**
     * 演示用户管理功能
     */
    void demonstrateUserManagement()
    {
        std::cout << "\n=== 用户管理演示 ===" << std::endl;
        
        // 创建用户
        auto result = user_controller_->createUser("alice", "alice@example.com");
        if (result.success)
        {
            std::cout << "✅ 用户创建成功: " << result.user.toString() << std::endl;
        }
        else
        {
            std::cout << "❌ 用户创建失败: " << result.error_message << std::endl;
        }
        
        // 获取用户
        User user = user_controller_->getUserById(1);
        if (user.getId() > 0)
        {
            std::cout << "✅ 找到用户: " << user.toString() << std::endl;
        }
        
        // 搜索用户
        auto users = user_controller_->searchUsers("alice");
        std::cout << "🔍 搜索到 " << users.size() << " 个用户" << std::endl;
    }
    
    /**
     * 演示产品管理功能
     */
    void demonstrateProductManagement()
    {
        std::cout << "\n=== 产品管理演示 ===" << std::endl;
        
        // 创建产品
        auto result = product_controller_->createProduct("iPhone 15", "最新款iPhone手机", 7999.00, 50);
        if (result.success)
        {
            std::cout << "✅ 产品创建成功: " << result.product.toString() << std::endl;
        }
        else
        {
            std::cout << "❌ 产品创建失败: " << result.error_message << std::endl;
        }
        
        // 获取产品
        Product product = product_controller_->getProductById(1);
        if (product.getId() > 0)
        {
            std::cout << "✅ 找到产品: " << product.toString() << std::endl;
        }
        
        // 更新库存
        if (product_controller_->updateStock(1, 100))
        {
            std::cout << "✅ 库存更新成功" << std::endl;
        }
        
        // 搜索产品
        auto products = product_controller_->searchProducts("iPhone");
        std::cout << "🔍 搜索到 " << products.size() << " 个产品" << std::endl;
    }
    
    /**
     * 演示订单管理功能
     */
    void demonstrateOrderManagement()
    {
        std::cout << "\n=== 订单管理演示 ===" << std::endl;
        
        // 创建订单项
        std::vector<OrderItem> items;
        items.push_back(OrderItem(1, "iPhone 15", 7999.00, 1));
        items.push_back(OrderItem(2, "AirPods Pro", 1999.00, 1));
        
        // 创建订单
        auto result = order_controller_->createOrder(1, items);
        if (result.success)
        {
            std::cout << "✅ 订单创建成功: " << result.order.toString() << std::endl;
        }
        else
        {
            std::cout << "❌ 订单创建失败: " << result.error_message << std::endl;
        }
        
        // 获取订单
        Order order = order_controller_->getOrderById(1);
        if (order.getId() > 0)
        {
            std::cout << "✅ 找到订单: " << order.toString() << std::endl;
        }
        
        // 确认订单
        if (order_controller_->confirmOrder(1))
        {
            std::cout << "✅ 订单确认成功" << std::endl;
        }
        
        // 发货
        if (order_controller_->shipOrder(1))
        {
            std::cout << "✅ 订单发货成功" << std::endl;
        }
    }
    
    /**
     * 演示跨实体业务功能
     */
    void demonstrateCrossEntityOperations()
    {
        std::cout << "\n=== 跨实体业务演示 ===" << std::endl;
        
        // 获取用户档案
        auto profile = app_service_->getUserProfile(1);
        if (profile.user.getId() > 0)
        {
            std::cout << "✅ 用户档案: " << profile.user.getUsername() 
                      << ", 订单数: " << profile.total_orders 
                      << ", 总消费: " << profile.total_spent << std::endl;
        }
        
        // 创建订单并验证
        std::vector<OrderItem> items;
        items.push_back(OrderItem(1, "iPhone 15", 7999.00, 1));
        
        if (app_service_->createOrderWithValidation(1, items))
        {
            std::cout << "✅ 订单创建并验证成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 订单创建失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 取消订单并退款
        if (app_service_->cancelOrderWithRefund(2))
        {
            std::cout << "✅ 订单取消并退款成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 订单取消失败: " << app_service_->getLastError() << std::endl;
        }
    }
    
    /**
     * 演示库存管理功能
     */
    void demonstrateInventoryManagement()
    {
        std::cout << "\n=== 库存管理演示 ===" << std::endl;
        
        // 更新产品库存
        if (app_service_->updateProductStock(1, 100))
        {
            std::cout << "✅ 产品库存更新成功" << std::endl;
        }
        
        // 预留库存
        if (app_service_->reserveProductStock(1, 5))
        {
            std::cout << "✅ 产品库存预留成功" << std::endl;
        }
        
        // 释放库存
        if (app_service_->releaseProductStock(1, 2))
        {
            std::cout << "✅ 产品库存释放成功" << std::endl;
        }
        
        // 获取低库存产品
        auto low_stock_products = app_service_->getLowStockProducts(10);
        std::cout << "📦 低库存产品: " << low_stock_products.size() << " 个" << std::endl;
    }
    
    /**
     * 演示业务统计功能
     */
    void demonstrateBusinessStatistics()
    {
        std::cout << "\n=== 业务统计演示 ===" << std::endl;
        
        auto stats = app_service_->getBusinessStatistics();
        std::cout << "📊 业务统计:" << std::endl;
        std::cout << "  总用户数: " << stats.total_users << std::endl;
        std::cout << "  总产品数: " << stats.total_products << std::endl;
        std::cout << "  总订单数: " << stats.total_orders << std::endl;
        std::cout << "  待处理订单: " << stats.pending_orders << std::endl;
        std::cout << "  低库存产品: " << stats.low_stock_products << std::endl;
    }
    
    /**
     * 演示数据一致性操作
     */
    void demonstrateDataConsistency()
    {
        std::cout << "\n=== 数据一致性演示 ===" << std::endl;
        
        // 删除用户和关联订单
        if (app_service_->deleteUserAndOrders(3))
        {
            std::cout << "✅ 用户和关联订单删除成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 用户和关联订单删除失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 删除产品和更新订单
        if (app_service_->deleteProductAndUpdateOrders(5))
        {
            std::cout << "✅ 产品和关联订单更新成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 产品和关联订单更新失败: " << app_service_->getLastError() << std::endl;
        }
    }
};

/**
 * 演示完整架构
 */
void demonstrateCompleteArchitecture()
{
    try
    {
        CompleteArchitectureApplication app;
        
        std::cout << "🚀 开始完整架构演示" << std::endl;
        
        // 演示各种功能
        app.demonstrateUserManagement();
        app.demonstrateProductManagement();
        app.demonstrateOrderManagement();
        app.demonstrateCrossEntityOperations();
        app.demonstrateInventoryManagement();
        app.demonstrateBusinessStatistics();
        app.demonstrateDataConsistency();
        
        std::cout << "\n🎉 完整架构演示完成!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ 应用程序错误: " << e.what() << std::endl;
    }
}

int main()
{
    std::cout << "=== 完整架构示例 ===" << std::endl;
    std::cout << "这个示例展示了如何使用完整的架构" << std::endl;
    
    demonstrateCompleteArchitecture();
    
    return 0;
}
