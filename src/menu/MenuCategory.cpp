#include "MenuCategory.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>

// Конструкторы
Category::Category() : id(0), name(""), description("") {}
Category::Category(int id, const std::string& name, const std::string& description)
        : id(id), name(name), description(description) {}

// Геттеры
int Category::getId() const { return id; }
const std::string& Category::getName() const { return name; }
const std::string& Category::getDescription() const { return description; }
const std::vector<std::shared_ptr<Item>>& Category::getProducts() const {
    return products;
}
size_t Category::getProductCount() const { return products.size(); }

// Работа с продуктами
bool Category::addProduct(std::shared_ptr<Item> item) {
    // Проверяем, нет ли уже продукта с таким ID
    auto it = std::find_if(products.begin(), products.end(),
                           [item](const std::shared_ptr<Item>& p) {
                               return p->get_id() == item->get_id();
                           });

    if (it != products.end()) {
        return false; // Продукт с таким ID уже существует
    }

    products.push_back(item);
    return true;
}

bool Category::removeProduct(int product_id) {
    auto it = std::remove_if(products.begin(), products.end(),
                             [product_id](const std::shared_ptr<Item>& Item) {
                                 return Item->get_id() == product_id;
                             });

    if (it != products.end()) {
        products.erase(it, products.end());
        return true;
    }
    return false;
}

std::shared_ptr<Item> Category::getProduct(int product_id) const {
    auto it = std::find_if(products.begin(), products.end(),
                           [product_id](const std::shared_ptr<Item>& Item) {
                               return Item->get_id() == product_id;
                           });

    return (it != products.end()) ? *it : nullptr;
}

std::shared_ptr<Item> Category::getProductByName(const std::string& name) const {
    auto it = std::find_if(products.begin(), products.end(),
                           [&name](const std::shared_ptr<Item>& Item) {
                               return Item->get_name() == name;
                           });

    return (it != products.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Item>> Category::getAvailableProducts() const {
    std::vector<std::shared_ptr<Item>> results;

    for (const auto& Item : products) {
        if (Item->is_available()) {
            results.push_back(Item);
        }
    }

    return results;
}

// Работа с атрибутами
void Category::setAttribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

std::string Category::getAttribute(const std::string& key) const {
    auto it = attributes.find(key);
    return (it != attributes.end()) ? it->second : "";
}

// Утилиты
void Category::print() const {
    std::cout << "\n=== " << name << " ===" << std::endl;
    if (!description.empty()) {
        std::cout << description << std::endl;
    }

    if (products.empty()) {
        std::cout << "(пусто)" << std::endl;
    } else {
        for (const auto& Item : products) {
            std::cout << "  ";
            // Item->print(); //TODO РЕАЛИЗОВАТЬ
        }
    }
}

bool Category::isEmpty() const {
    return products.empty();
}

// Статистика
Category::Statistics Category::getStatistics() const {
    Statistics stats;
    //TODO РЕАЛИЗОВАТЬ
    return stats;
}