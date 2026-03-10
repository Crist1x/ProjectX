#ifndef PROJECTX_Category_H
#define PROJECTX_Category_H

#include "MenuItem.h"
#include <vector>
#include <string>
#include "../../cmake/json.hpp"

using json = nlohmann::json;

class Category {
private:
    int id{};
    std::string name;
    std::string description;
    int cafe_id{};
    std::vector<Item> items;

public:
    Category() = default;
    Category(int id, const std::string& name, const std::string& description, int cafe_id);

    // Геттеры
    int get_id() const;
    const std::string& get_name() const;
    const std::string& get_description() const;
    int get_cafe_id() const;
    const std::vector<Item>& get_items() const;

    // Сеттеры
    void set_name(const std::string& new_name);
    void set_description(const std::string& new_description);

    // Добавление/удаление и всякое такое
    void add_item(const Item& item);
    void remove_item(int item_id);
    Item* find_item(int item_id);
    const Item* find_item(int item_id) const;

    // Чуть-чуть информации
    size_t get_items_count() const;
    double get_min_price() const;
    double get_max_price() const;
    int get_max_preparation_time() const;

    // JSON и валидация
    json toJson() const;
    static Category fromJson(const json& j);

    bool isValid() const;
};

#endif