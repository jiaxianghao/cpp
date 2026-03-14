#include "repositories/product_repository.h"
#include <iostream>

using Param = DatabaseConnection::PreparedParam;

ProductRepository::ProductRepository(std::shared_ptr<DatabaseConnection> connection)
    : connection_(connection)
{
}

std::vector<Product> ProductRepository::getAll()
{
    std::vector<Product> products;
    std::string query = "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products ORDER BY id";
    
    auto results = connection_->fetchAll(query);
    for (const auto& row : results)
    {
        products.push_back(mapRowToProduct(row));
    }
    
    return products;
}

Product ProductRepository::getById(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE id = ?",
        { Param::fromInt(id) });
    
    if (result.empty())
    {
        return Product();
    }
    
    return mapRowToProduct(result);
}

Product ProductRepository::getByName(const std::string& name)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE name = ?",
        { Param::fromString(name) });
    
    if (result.empty())
    {
        return Product();
    }
    
    return mapRowToProduct(result);
}

int ProductRepository::create(const Product& product)
{
    if (connection_->executePreparedQuery(
        "INSERT INTO products (name, description, price, stock_quantity) VALUES (?, ?, ?, ?)",
        { Param::fromString(product.getName()), Param::fromString(product.getDescription()),
            Param::fromDouble(product.getPrice()),
            Param::fromInt(product.getStockQuantity()) }))
    {
        auto result = connection_->fetchOne("SELECT LAST_INSERT_ID() as id");
        if (!result.empty())
        {
            return std::stoi(result["id"]);
        }
    }
    
    return -1;
}

bool ProductRepository::update(const Product& product)
{
    return connection_->executePreparedQuery(
        "UPDATE products SET name = ?, description = ?, price = ?, stock_quantity = ? WHERE id = ?",
        { Param::fromString(product.getName()), Param::fromString(product.getDescription()),
            Param::fromDouble(product.getPrice()),
            Param::fromInt(product.getStockQuantity()),
            Param::fromInt(product.getId()) });
}

bool ProductRepository::deleteById(int id)
{
    return connection_->executePreparedQuery(
        "DELETE FROM products WHERE id = ?",
        { Param::fromInt(id) });
}

std::vector<Product> ProductRepository::searchByName(const std::string& pattern)
{
    std::vector<Product> products;
    std::string like_pattern = "%" + pattern + "%";
    auto results = connection_->fetchAllPrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE name LIKE ? ORDER BY name",
        { Param::fromString(like_pattern) });
    for (const auto& row : results)
    {
        products.push_back(mapRowToProduct(row));
    }
    
    return products;
}

std::vector<Product> ProductRepository::searchByDescription(const std::string& pattern)
{
    std::vector<Product> products;
    std::string like_pattern = "%" + pattern + "%";
    auto results = connection_->fetchAllPrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE description LIKE ? ORDER BY name",
        { Param::fromString(like_pattern) });
    for (const auto& row : results)
    {
        products.push_back(mapRowToProduct(row));
    }
    
    return products;
}

std::vector<Product> ProductRepository::searchByPriceRange(double min_price, double max_price)
{
    std::vector<Product> products;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE price BETWEEN ? AND ? ORDER BY price",
        { Param::fromDouble(min_price), Param::fromDouble(max_price) });
    for (const auto& row : results)
    {
        products.push_back(mapRowToProduct(row));
    }
    
    return products;
}

bool ProductRepository::updateStock(int product_id, int new_quantity)
{
    return connection_->executePreparedQuery(
        "UPDATE products SET stock_quantity = ? WHERE id = ?",
        { Param::fromInt(new_quantity), Param::fromInt(product_id) });
}

bool ProductRepository::decreaseStock(int product_id, int quantity)
{
    return connection_->executePreparedQuery(
        "UPDATE products SET stock_quantity = stock_quantity - ? WHERE id = ? AND stock_quantity >= ?",
        { Param::fromInt(quantity), Param::fromInt(product_id), Param::fromInt(quantity) });
}

bool ProductRepository::increaseStock(int product_id, int quantity)
{
    return connection_->executePreparedQuery(
        "UPDATE products SET stock_quantity = stock_quantity + ? WHERE id = ?",
        { Param::fromInt(quantity), Param::fromInt(product_id) });
}

int ProductRepository::getCount()
{
    auto result = connection_->fetchOne("SELECT COUNT(*) as count FROM products");
    if (!result.empty())
    {
        return std::stoi(result["count"]);
    }
    return 0;
}

bool ProductRepository::exists(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM products WHERE id = ?",
        { Param::fromInt(id) });
    return !result.empty();
}

bool ProductRepository::existsByName(const std::string& name)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM products WHERE name = ?",
        { Param::fromString(name) });
    return !result.empty();
}

std::vector<Product> ProductRepository::getLowStockProducts(int threshold)
{
    std::vector<Product> products;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, name, description, price, stock_quantity, created_at, updated_at FROM products WHERE stock_quantity <= ? ORDER BY stock_quantity",
        { Param::fromInt(threshold) });
    for (const auto& row : results)
    {
        products.push_back(mapRowToProduct(row));
    }
    
    return products;
}

std::vector<Product> ProductRepository::getOutOfStockProducts()
{
    return getLowStockProducts(0);
}

Product ProductRepository::mapRowToProduct(const std::map<std::string, std::string>& row)
{
    Product product;
    
    if (row.find("id") != row.end())
    {
        product.setId(std::stoi(row.at("id")));
    }
    
    if (row.find("name") != row.end())
    {
        product.setName(row.at("name"));
    }
    
    if (row.find("description") != row.end())
    {
        product.setDescription(row.at("description"));
    }
    
    if (row.find("price") != row.end())
    {
        product.setPrice(std::stod(row.at("price")));
    }
    
    if (row.find("stock_quantity") != row.end())
    {
        product.setStockQuantity(std::stoi(row.at("stock_quantity")));
    }
    
    if (row.find("created_at") != row.end())
    {
        product.setCreatedAt(row.at("created_at"));
    }
    
    if (row.find("updated_at") != row.end())
    {
        product.setUpdatedAt(row.at("updated_at"));
    }
    
    return product;
}

