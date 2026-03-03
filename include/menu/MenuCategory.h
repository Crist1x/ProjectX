#ifndef PROJECTX_MENUCATEGORY_H
#define PROJECTX_MENUCATEGORY_H

#include "MenuItem.h"
#include <vector>
#include <memory>
#include <algorithm>

class Category {
private:
    int id;
    std::string name;
    std::string description;
    std::vector<std::shared_ptr<Item>> products;
    std::map<std::string, std::string> attributes;

public:
    Category();
    Category(int id, const std::string& name, const std::string& description = "");

    // Геттеры
    int getId() const;
    const std::string& getName() const;
    const std::string& getDescription() const;
    const std::vector<std::shared_ptr<Item>>& getProducts() const;
    size_t getProductCount() const;

    // Работа с продуктами
    bool addProduct(std::shared_ptr<Item> Item);
    bool removeProduct(int product_id);
    std::shared_ptr<Item> getProduct(int product_id) const;
    std::shared_ptr<Item> getProductByName(const std::string& name) const;

    // Поиск продуктов
    std::vector<std::shared_ptr<Item>> searchProducts(
            const std::string& query) const; // РЕЛИЗНУТЬ
    std::vector<std::shared_ptr<Item>> getAvailableProducts() const;

    // Работа с атрибутами
    void setAttribute(const std::string& key, const std::string& value);
    std::string getAttribute(const std::string& key) const;

    // Утилиты
    void print() const;
    bool isEmpty() const;

    // Статистика
    struct Statistics {
        size_t total_products;
        size_t available_products;
        double min_price;
        double max_price;
        double average_price;
    };

    Statistics getStatistics() const;
};

#endif // CATEGORY_HPP
