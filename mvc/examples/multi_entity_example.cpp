#include <iostream>
#include <memory>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "repositories/user_repository.h"
#include "repositories/product_repository.h"
#include "repositories/order_repository.h"
#include "services/application_service.h"

/**
 * 多实体示例 - 展示如何使用多个实体类型
 * 这个示例展示了如何在实际业务中使用多个实体
 */

class MultiEntityApplication
{
private:
    std::shared_ptr<ApplicationService> app_service_;
    
public:
    MultiEntityApplication()
    {
        initializeDatabase();
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
        
        std::cout << "多实体应用系统初始化完成" << std::endl;
    }
    
    /**
     * 演示用户管理功能
     */
    void demonstrateUserManagement()
    {
        std::cout << "\n=== 用户管理演示 ===" << std::endl;
        
        // 获取用户档案
        auto profile = app_service_->getUserProfile(2);
        if (profile.user.getId() > 0)
        {
            std::cout << "用户档案:" << std::endl;
            std::cout << "  用户: " << profile.user.toString() << std::endl;
            std::cout << "  订单数量: " << profile.total_orders << std::endl;
            std::cout << "  总消费: " << profile.total_spent << " 元" << std::endl;
        }
        else
        {
            std::cout << "获取用户档案失败: " << app_service_->getLastError() << std::endl;
        }
    }
    
    /**
     * 演示产品管理功能
     */
    void demonstrateProductManagement()
    {
        std::cout << "\n=== 产品管理演示 ===" << std::endl;
        
        // 获取产品信息
        auto product_info = app_service_->getProductInfo(1);
        if (product_info.product.getId() > 0)
        {
            std::cout << "产品信息:" << std::endl;
            std::cout << "  产品: " << product_info.product.toString() << std::endl;
            std::cout << "  总订单数: " << product_info.total_orders << std::endl;
            std::cout << "  总收入: " << product_info.total_revenue << " 元" << std::endl;
        }
        else
        {
            std::cout << "获取产品信息失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 检查低库存产品
        auto low_stock_products = app_service_->getLowStockProducts(30);
        std::cout << "\n低库存产品 (" << low_stock_products.size() << " 个):" << std::endl;
        for (const auto& product : low_stock_products)
        {
            std::cout << "  - " << product.getName() 
                      << " (库存: " << product.getStockQuantity() << ")" << std::endl;
        }
    }
    
    /**
     * 演示订单管理功能
     */
    void demonstrateOrderManagement()
    {
        std::cout << "\n=== 订单管理演示 ===" << std::endl;
        
        // 获取订单摘要
        auto order_summary = app_service_->getOrderSummary(1);
        if (order_summary.order.getId() > 0)
        {
            std::cout << "订单摘要:" << std::endl;
            std::cout << "  订单: " << order_summary.order.toString() << std::endl;
            std::cout << "  用户: " << order_summary.user.getUsername() << std::endl;
            std::cout << "  产品数量: " << order_summary.products.size() << std::endl;
        }
        else
        {
            std::cout << "获取订单摘要失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 处理订单支付
        std::cout << "\n处理订单支付..." << std::endl;
        if (app_service_->processOrderPayment(2))
        {
            std::cout << "✅ 订单支付处理成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 订单支付处理失败: " << app_service_->getLastError() << std::endl;
        }
    }
    
    /**
     * 演示库存管理功能
     */
    void demonstrateInventoryManagement()
    {
        std::cout << "\n=== 库存管理演示 ===" << std::endl;
        
        // 更新产品库存
        std::cout << "更新产品库存..." << std::endl;
        if (app_service_->updateProductStock(1, 100))
        {
            std::cout << "✅ 产品库存更新成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 产品库存更新失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 预留库存
        std::cout << "预留产品库存..." << std::endl;
        if (app_service_->reserveProductStock(1, 5))
        {
            std::cout << "✅ 产品库存预留成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 产品库存预留失败: " << app_service_->getLastError() << std::endl;
        }
    }
    
    /**
     * 演示业务统计功能
     */
    void demonstrateBusinessStatistics()
    {
        std::cout << "\n=== 业务统计演示 ===" << std::endl;
        
        auto stats = app_service_->getBusinessStatistics();
        std::cout << "业务统计信息:" << std::endl;
        std::cout << "  总用户数: " << stats.total_users << std::endl;
        std::cout << "  总产品数: " << stats.total_products << std::endl;
        std::cout << "  总订单数: " << stats.total_orders << std::endl;
        std::cout << "  待处理订单: " << stats.pending_orders << std::endl;
        std::cout << "  低库存产品: " << stats.low_stock_products << std::endl;
        std::cout << "  总收入: " << stats.total_revenue << " 元" << std::endl;
    }
    
    /**
     * 演示跨实体业务操作
     */
    void demonstrateCrossEntityOperations()
    {
        std::cout << "\n=== 跨实体业务操作演示 ===" << std::endl;
        
        // 创建订单（包含验证）
        std::cout << "创建新订单..." << std::endl;
        std::vector<OrderItem> items;
        items.push_back(OrderItem(1, "iPhone 15", 7999.00, 1));
        items.push_back(OrderItem(3, "AirPods Pro", 1999.00, 1));
        
        if (app_service_->createOrderWithValidation(2, items))
        {
            std::cout << "✅ 订单创建成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 订单创建失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 取消订单（包含退款）
        std::cout << "\n取消订单..." << std::endl;
        if (app_service_->cancelOrderWithRefund(3))
        {
            std::cout << "✅ 订单取消成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 订单取消失败: " << app_service_->getLastError() << std::endl;
        }
    }
    
    /**
     * 演示数据一致性操作
     */
    void demonstrateDataConsistency()
    {
        std::cout << "\n=== 数据一致性演示 ===" << std::endl;
        
        // 删除用户和关联订单
        std::cout << "删除用户和关联订单..." << std::endl;
        if (app_service_->deleteUserAndOrders(4))
        {
            std::cout << "✅ 用户和关联订单删除成功" << std::endl;
        }
        else
        {
            std::cout << "❌ 用户和关联订单删除失败: " << app_service_->getLastError() << std::endl;
        }
        
        // 删除产品和更新订单
        std::cout << "\n删除产品和更新订单..." << std::endl;
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
 * 演示多实体业务场景
 */
void demonstrateMultiEntityScenarios()
{
    try
    {
        MultiEntityApplication app;
        
        std::cout << "🚀 开始多实体业务场景演示" << std::endl;
        
        // 演示各种功能
        app.demonstrateUserManagement();
        app.demonstrateProductManagement();
        app.demonstrateOrderManagement();
        app.demonstrateInventoryManagement();
        app.demonstrateBusinessStatistics();
        app.demonstrateCrossEntityOperations();
        app.demonstrateDataConsistency();
        
        std::cout << "\n🎉 多实体业务场景演示完成!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ 应用程序错误: " << e.what() << std::endl;
    }
}

int main()
{
    std::cout << "=== 多实体示例 ===" << std::endl;
    std::cout << "这个示例展示了如何使用多个实体类型" << std::endl;
    
    demonstrateMultiEntityScenarios();
    
    return 0;
}

