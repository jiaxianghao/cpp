#include "controllers/order_controller.h"
#include "utils/logger.h"

OrderController::OrderController(std::shared_ptr<OrderService> order_service)
    : order_service_(order_service)
{
}

OrderController::CreateOrderResult OrderController::createOrder(int user_id, const std::vector<OrderItem>& items)
{
    CreateOrderResult result;
    result.success = false;
    
    try
    {
        Order order = order_service_->createOrder(user_id, items);
        
        if (order.getId() > 0)
        {
            result.success = true;
            result.order = order;
            logOperation("createOrder", true);
        }
        else
        {
            result.error_message = order_service_->getLastError();
            setError(result.error_message);
            logOperation("createOrder", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("createOrder", false);
    }
    
    return result;
}

Order OrderController::getOrderById(int id)
{
    try
    {
        Order order = order_service_->getOrderById(id);
        logOperation("getOrderById", order.getId() > 0);
        return order;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getOrderById", false);
        return Order();
    }
}

std::vector<Order> OrderController::getOrdersByUserId(int user_id)
{
    try
    {
        auto orders = order_service_->getOrdersByUserId(user_id);
        logOperation("getOrdersByUserId", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getOrdersByUserId", false);
        return std::vector<Order>();
    }
}

std::vector<Order> OrderController::getOrdersByStatus(const std::string& status)
{
    try
    {
        auto orders = order_service_->getOrdersByStatus(status);
        logOperation("getOrdersByStatus", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getOrdersByStatus", false);
        return std::vector<Order>();
    }
}

std::vector<Order> OrderController::getAllOrders()
{
    try
    {
        auto orders = order_service_->getAllOrders();
        logOperation("getAllOrders", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getAllOrders", false);
        return std::vector<Order>();
    }
}

OrderController::UpdateOrderResult OrderController::updateOrder(const Order& order)
{
    UpdateOrderResult result;
    result.success = false;
    
    try
    {
        result.success = order_service_->updateOrder(order);
        
        if (result.success)
        {
            logOperation("updateOrder", true);
        }
        else
        {
            result.error_message = order_service_->getLastError();
            setError(result.error_message);
            logOperation("updateOrder", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("updateOrder", false);
    }
    
    return result;
}

OrderController::DeleteOrderResult OrderController::deleteOrder(int id)
{
    DeleteOrderResult result;
    result.success = false;
    
    try
    {
        result.success = order_service_->deleteOrder(id);
        
        if (result.success)
        {
            logOperation("deleteOrder", true);
        }
        else
        {
            result.error_message = order_service_->getLastError();
            setError(result.error_message);
            logOperation("deleteOrder", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("deleteOrder", false);
    }
    
    return result;
}

OrderController::CancelOrderResult OrderController::cancelOrder(int order_id)
{
    CancelOrderResult result;
    result.success = false;
    
    try
    {
        result.success = order_service_->cancelOrder(order_id);
        
        if (result.success)
        {
            logOperation("cancelOrder", true);
        }
        else
        {
            result.error_message = order_service_->getLastError();
            setError(result.error_message);
            logOperation("cancelOrder", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("cancelOrder", false);
    }
    
    return result;
}

bool OrderController::confirmOrder(int order_id)
{
    try
    {
        bool success = order_service_->confirmOrder(order_id);
        logOperation("confirmOrder", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("confirmOrder", false);
        return false;
    }
}

bool OrderController::shipOrder(int order_id)
{
    try
    {
        bool success = order_service_->shipOrder(order_id);
        logOperation("shipOrder", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("shipOrder", false);
        return false;
    }
}

bool OrderController::deliverOrder(int order_id)
{
    try
    {
        bool success = order_service_->deliverOrder(order_id);
        logOperation("deliverOrder", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("deliverOrder", false);
        return false;
    }
}

bool OrderController::addOrderItem(int order_id, const OrderItem& item)
{
    try
    {
        bool success = order_service_->addOrderItem(order_id, item);
        logOperation("addOrderItem", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("addOrderItem", false);
        return false;
    }
}

bool OrderController::removeOrderItem(int order_id, int product_id)
{
    try
    {
        bool success = order_service_->removeOrderItem(order_id, product_id);
        logOperation("removeOrderItem", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("removeOrderItem", false);
        return false;
    }
}

bool OrderController::updateOrderItem(int order_id, int product_id, int quantity)
{
    try
    {
        bool success = order_service_->updateOrderItem(order_id, product_id, quantity);
        logOperation("updateOrderItem", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("updateOrderItem", false);
        return false;
    }
}

std::vector<OrderItem> OrderController::getOrderItems(int order_id)
{
    try
    {
        auto items = order_service_->getOrderItems(order_id);
        logOperation("getOrderItems", true);
        return items;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getOrderItems", false);
        return std::vector<OrderItem>();
    }
}

std::vector<Order> OrderController::searchOrdersByDateRange(const std::string& start_date, const std::string& end_date)
{
    try
    {
        auto orders = order_service_->searchOrdersByDateRange(start_date, end_date);
        logOperation("searchOrdersByDateRange", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchOrdersByDateRange", false);
        return std::vector<Order>();
    }
}

std::vector<Order> OrderController::searchOrdersByAmountRange(double min_amount, double max_amount)
{
    try
    {
        auto orders = order_service_->searchOrdersByAmountRange(min_amount, max_amount);
        logOperation("searchOrdersByAmountRange", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchOrdersByAmountRange", false);
        return std::vector<Order>();
    }
}

std::vector<Order> OrderController::searchOrdersByUser(int user_id)
{
    try
    {
        auto orders = order_service_->getOrdersByUserId(user_id);
        logOperation("searchOrdersByUser", true);
        return orders;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchOrdersByUser", false);
        return std::vector<Order>();
    }
}

int OrderController::getOrderCount()
{
    try
    {
        return order_service_->getOrderCount();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return 0;
    }
}

int OrderController::getOrderCountByUserId(int user_id)
{
    try
    {
        return order_service_->getOrderCountByUserId(user_id);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return 0;
    }
}

int OrderController::getOrderCountByStatus(const std::string& status)
{
    try
    {
        return order_service_->getOrderCountByStatus(status);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return 0;
    }
}

double OrderController::getTotalAmountByUserId(int user_id)
{
    try
    {
        return order_service_->getTotalAmountByUserId(user_id);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return 0.0;
    }
}

double OrderController::getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date)
{
    try
    {
        return order_service_->getTotalAmountByDateRange(start_date, end_date);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return 0.0;
    }
}

std::string OrderController::getOrderStatistics()
{
    try
    {
        return order_service_->getOrderStatistics();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return "Error retrieving statistics";
    }
}

bool OrderController::orderExists(int id)
{
    try
    {
        return order_service_->orderExists(id);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool OrderController::canCancelOrder(int order_id)
{
    try
    {
        Order order = order_service_->getOrderById(order_id);
        return order.canCancel();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool OrderController::canShipOrder(int order_id)
{
    try
    {
        Order order = order_service_->getOrderById(order_id);
        return order.canShip();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool OrderController::canDeliverOrder(int order_id)
{
    try
    {
        Order order = order_service_->getOrderById(order_id);
        return order.getStatus() == "SHIPPED";
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

std::string OrderController::getLastError() const
{
    return last_error_;
}

void OrderController::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("OrderController Error: {}", error);
}

void OrderController::logOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("Order operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("Order operation failed: {}", operation);
    }
}
