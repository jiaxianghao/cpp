#include "services/application_service.h"
#include "utils/logger.h"
#include <algorithm>
#include <numeric>

ApplicationService::ApplicationService(std::shared_ptr<UserRepository> user_repo,
                                     std::shared_ptr<ProductRepository> product_repo,
                                     std::shared_ptr<OrderRepository> order_repo)
    : user_repo_(user_repo), product_repo_(product_repo), order_repo_(order_repo)
{
}

ApplicationService::UserProfile ApplicationService::getUserProfile(int user_id)
{
    UserProfile profile;
    
    try
    {
        // 获取用户信息
        profile.user = user_repo_->getById(user_id);
        if (profile.user.getId() <= 0)
        {
            setError("用户不存在");
            return profile;
        }
        
        // 获取用户订单
        profile.orders = order_repo_->getByUserId(user_id);
        
        // 计算总消费
        profile.total_spent = order_repo_->getTotalAmountByUserId(user_id);
        
        // 计算订单数量
        profile.total_orders = order_repo_->getCountByUserId(user_id);
        
        logBusinessOperation("getUserProfile", true);
    }
    catch (const std::exception& e)
    {
        setError("获取用户档案失败: " + std::string(e.what()));
        logBusinessOperation("getUserProfile", false);
    }
    
    return profile;
}

bool ApplicationService::deleteUserAndOrders(int user_id)
{
    try
    {
        // 检查用户是否存在
        if (!user_repo_->exists(user_id))
        {
            setError("用户不存在");
            return false;
        }
        
        // 删除用户的所有订单
        auto orders = order_repo_->getByUserId(user_id);
        for (const auto& order : orders)
        {
            if (!order_repo_->deleteById(order.getId()))
            {
                setError("删除用户订单失败");
                return false;
            }
        }
        
        // 删除用户
        if (!user_repo_->deleteById(user_id))
        {
            setError("删除用户失败");
            return false;
        }
        
        logBusinessOperation("deleteUserAndOrders", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("删除用户和订单失败: " + std::string(e.what()));
        logBusinessOperation("deleteUserAndOrders", false);
        return false;
    }
}

ApplicationService::ProductInfo ApplicationService::getProductInfo(int product_id)
{
    ProductInfo info;
    
    try
    {
        // 获取产品信息
        info.product = product_repo_->getById(product_id);
        if (info.product.getId() <= 0)
        {
            setError("产品不存在");
            return info;
        }
        
        // 计算总订单数（简化版本）
        info.total_orders = 0; // 这里需要更复杂的查询
        
        // 计算总收入（简化版本）
        info.total_revenue = 0.0; // 这里需要更复杂的查询
        
        logBusinessOperation("getProductInfo", true);
    }
    catch (const std::exception& e)
    {
        setError("获取产品信息失败: " + std::string(e.what()));
        logBusinessOperation("getProductInfo", false);
    }
    
    return info;
}

bool ApplicationService::deleteProductAndUpdateOrders(int product_id)
{
    try
    {
        // 检查产品是否存在
        if (!product_repo_->exists(product_id))
        {
            setError("产品不存在");
            return false;
        }
        
        // 获取包含该产品的订单
        auto orders = order_repo_->getAll();
        for (const auto& order : orders)
        {
            for (const auto& item : order.getItems())
            {
                if (item.getProductId() == product_id)
                {
                    // 从订单中移除该产品
                    order_repo_->removeOrderItem(order.getId(), product_id);
                }
            }
        }
        
        // 删除产品
        if (!product_repo_->deleteById(product_id))
        {
            setError("删除产品失败");
            return false;
        }
        
        logBusinessOperation("deleteProductAndUpdateOrders", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("删除产品和更新订单失败: " + std::string(e.what()));
        logBusinessOperation("deleteProductAndUpdateOrders", false);
        return false;
    }
}

ApplicationService::OrderSummary ApplicationService::getOrderSummary(int order_id)
{
    OrderSummary summary;
    
    try
    {
        // 获取订单信息
        summary.order = order_repo_->getById(order_id);
        if (summary.order.getId() <= 0)
        {
            setError("订单不存在");
            return summary;
        }
        
        // 获取用户信息
        summary.user = user_repo_->getById(summary.order.getUserId());
        
        // 获取产品信息
        for (const auto& item : summary.order.getItems())
        {
            Product product = product_repo_->getById(item.getProductId());
            if (product.getId() > 0)
            {
                summary.products.push_back(product);
            }
        }
        
        logBusinessOperation("getOrderSummary", true);
    }
    catch (const std::exception& e)
    {
        setError("获取订单摘要失败: " + std::string(e.what()));
        logBusinessOperation("getOrderSummary", false);
    }
    
    return summary;
}

bool ApplicationService::createOrderWithValidation(int user_id, const std::vector<OrderItem>& items)
{
    try
    {
        // 验证用户是否存在
        if (!user_repo_->exists(user_id))
        {
            setError("用户不存在");
            return false;
        }
        
        // 验证订单项
        if (!validateOrderItems(items))
        {
            setError("订单项验证失败");
            return false;
        }
        
        // 检查产品可用性
        for (const auto& item : items)
        {
            if (!checkProductAvailability(item.getProductId(), item.getQuantity()))
            {
                setError("产品库存不足: " + std::to_string(item.getProductId()));
                return false;
            }
        }
        
        // 创建订单
        Order order(user_id, items);
        int order_id = order_repo_->create(order);
        if (order_id <= 0)
        {
            setError("创建订单失败");
            return false;
        }
        
        // 减少库存
        for (const auto& item : items)
        {
            if (!product_repo_->decreaseStock(item.getProductId(), item.getQuantity()))
            {
                setError("更新库存失败");
                return false;
            }
        }
        
        logBusinessOperation("createOrderWithValidation", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("创建订单失败: " + std::string(e.what()));
        logBusinessOperation("createOrderWithValidation", false);
        return false;
    }
}

bool ApplicationService::cancelOrderWithRefund(int order_id)
{
    try
    {
        // 获取订单
        Order order = order_repo_->getById(order_id);
        if (order.getId() <= 0)
        {
            setError("订单不存在");
            return false;
        }
        
        // 检查订单状态
        if (!order.canCancel())
        {
            setError("订单无法取消");
            return false;
        }
        
        // 取消订单
        if (!order_repo_->cancelOrder(order_id))
        {
            setError("取消订单失败");
            return false;
        }
        
        // 恢复库存
        for (const auto& item : order.getItems())
        {
            product_repo_->increaseStock(item.getProductId(), item.getQuantity());
        }
        
        logBusinessOperation("cancelOrderWithRefund", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("取消订单失败: " + std::string(e.what()));
        logBusinessOperation("cancelOrderWithRefund", false);
        return false;
    }
}

bool ApplicationService::processOrderPayment(int order_id)
{
    try
    {
        // 获取订单
        Order order = order_repo_->getById(order_id);
        if (order.getId() <= 0)
        {
            setError("订单不存在");
            return false;
        }
        
        // 检查订单状态
        if (order.getStatus() != "PENDING")
        {
            setError("订单状态不正确");
            return false;
        }
        
        // 确认订单
        if (!order_repo_->confirmOrder(order_id))
        {
            setError("确认订单失败");
            return false;
        }
        
        logBusinessOperation("processOrderPayment", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("处理订单支付失败: " + std::string(e.what()));
        logBusinessOperation("processOrderPayment", false);
        return false;
    }
}

bool ApplicationService::updateProductStock(int product_id, int new_quantity)
{
    try
    {
        if (!product_repo_->exists(product_id))
        {
            setError("产品不存在");
            return false;
        }
        
        if (!product_repo_->updateStock(product_id, new_quantity))
        {
            setError("更新库存失败");
            return false;
        }
        
        logBusinessOperation("updateProductStock", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("更新产品库存失败: " + std::string(e.what()));
        logBusinessOperation("updateProductStock", false);
        return false;
    }
}

bool ApplicationService::reserveProductStock(int product_id, int quantity)
{
    try
    {
        if (!checkProductAvailability(product_id, quantity))
        {
            setError("产品库存不足");
            return false;
        }
        
        if (!product_repo_->decreaseStock(product_id, quantity))
        {
            setError("预留库存失败");
            return false;
        }
        
        logBusinessOperation("reserveProductStock", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("预留产品库存失败: " + std::string(e.what()));
        logBusinessOperation("reserveProductStock", false);
        return false;
    }
}

bool ApplicationService::releaseProductStock(int product_id, int quantity)
{
    try
    {
        if (!product_repo_->increaseStock(product_id, quantity))
        {
            setError("释放库存失败");
            return false;
        }
        
        logBusinessOperation("releaseProductStock", true);
        return true;
    }
    catch (const std::exception& e)
    {
        setError("释放产品库存失败: " + std::string(e.what()));
        logBusinessOperation("releaseProductStock", false);
        return false;
    }
}

std::vector<Product> ApplicationService::getLowStockProducts(int threshold)
{
    try
    {
        auto products = product_repo_->getLowStockProducts(threshold);
        logBusinessOperation("getLowStockProducts", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("获取低库存产品失败: " + std::string(e.what()));
        logBusinessOperation("getLowStockProducts", false);
        return std::vector<Product>();
    }
}

ApplicationService::BusinessStatistics ApplicationService::getBusinessStatistics()
{
    BusinessStatistics stats;
    
    try
    {
        stats.total_users = user_repo_->getCount();
        stats.total_products = product_repo_->getCount();
        stats.total_orders = order_repo_->getCount();
        stats.pending_orders = order_repo_->getCountByStatus("PENDING");
        stats.low_stock_products = product_repo_->getLowStockProducts(10).size();
        
        // 计算总收入（简化版本）
        stats.total_revenue = 0.0; // 这里需要更复杂的查询
        
        logBusinessOperation("getBusinessStatistics", true);
    }
    catch (const std::exception& e)
    {
        setError("获取业务统计失败: " + std::string(e.what()));
        logBusinessOperation("getBusinessStatistics", false);
    }
    
    return stats;
}

std::vector<Order> ApplicationService::getOrdersByDateRange(const std::string& start_date, const std::string& end_date)
{
    try
    {
        auto orders = order_repo_->searchByDateRange(start_date, end_date);
        logBusinessOperation("getOrdersByDateRange", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("获取订单失败: " + std::string(e.what()));
        logBusinessOperation("getOrdersByDateRange", false);
        return std::vector<Order>();
    }
}

std::vector<Product> ApplicationService::getTopSellingProducts(int limit)
{
    // 简化版本，实际需要更复杂的查询
    return product_repo_->getAll();
}

std::vector<User> ApplicationService::getTopCustomers(int limit)
{
    // 简化版本，实际需要更复杂的查询
    return user_repo_->getAll();
}

bool ApplicationService::validateOrderData(const Order& order)
{
    return order.isValid();
}

bool ApplicationService::validateProductData(const Product& product)
{
    return product.isValid();
}

bool ApplicationService::validateUserData(const User& user)
{
    return user.isValid();
}

void ApplicationService::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("ApplicationService Error: {}", error);
}

bool ApplicationService::validateOrderItems(const std::vector<OrderItem>& items)
{
    if (items.empty())
    {
        return false;
    }
    
    for (const auto& item : items)
    {
        if (!item.isValid())
        {
            return false;
        }
    }
    
    return true;
}

bool ApplicationService::checkProductAvailability(int product_id, int quantity)
{
    Product product = product_repo_->getById(product_id);
    return product.canPurchase(quantity);
}

void ApplicationService::logBusinessOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("Business operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("Business operation failed: {}", operation);
    }
}

