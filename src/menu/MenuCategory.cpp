#include "Category.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

Category::Category(int id, const std::string& name, 
                           const std::string& description, int cafe_id)
    : id(id), name(name), description(description), cafe_id(cafe_id) {}

// Геттеры
int Category::get_id() const { return id; }

const std::string& Category::get_name() const { return name; }

const std::string& Category::get_description() const { return description; }

int Category::get_cafe_id() const { return cafe_id; }

const std::vector<Item>& Category::get_items() const { return items; }

// Сеттеры
void Category::set_name(const std::string& new_name) {
    if (new_name.empty()) {
        throw std::invalid_argument("Category name cannot be empty");
    }
    name = new_name;
}

void Category::set_description(const std::string& new_description) {
    description = new_description;
}

// Добавление/удаление и всякое такое
void Category::add_item(const Item& item) {
    if (!item.isValid()) {
        throw std::invalid_argument("Cannot add invalid item");
    }
    if (item.get_cafe_id() != cafe_id) {
        throw std::invalid_argument("Item cafe_id does not match category cafe_id");
    }
    items.push_back(item);
}

void Category::remove_item(int item_id) {
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [item_id](const Item& item) { return item.get_id() == item_id; }),
        items.end()
    );
}

Item* Category::find_item(int item_id) {
    for (auto& item : items) {
        if (item.get_id() == item_id) {
            return &item;
        }
    }
    return nullptr;
}

const Item* Category::find_item(int item_id) const {
    for (const auto& item : items) {
        if (item.get_id() == item_id) {
            return &item;
        }
    }
    return nullptr;
}

// Чуть чуть информации
size_t Category::get_items_count() const {
    return items.size();
}

double Category::get_min_price() const {
    if (items.empty()) return 0.0;
    auto it = std::min_element(items.begin(), items.end(),
        [](const Item& a, const Item& b) { return a.get_price() < b.get_price(); });
    return it->get_price();
}

double Category::get_max_price() const {
    if (items.empty()) return 0.0;
    auto it = std::max_element(items.begin(), items.end(),
        [](const Item& a, const Item& b) { return a.get_price() < b.get_price(); });
    return it->get_price();
}

int Category::get_max_preparation_time() const {
    if (items.empty()) return 0;
    auto it = std::max_element(items.begin(), items.end(),
        [](const Item& a, const Item& b) { return a.get_preparation_time() < b.get_preparation_time(); });
    return it->get_preparation_time();
}

// JSON
json Category::toJson() const {
    json j = {
        {"id", id},
        {"name", name},
        {"description", description},
        {"cafe_id", cafe_id},
        {"items", json::array()}
    };
    for (const auto& item : items) {
        j["items"].push_back(item.toJson());
    }
    return j;
}

Category Category::fromJson(const json& j) {
    Category category;
    category.id = j.value("id", -1);
    category.name = j.value("name", "");
    category.description = j.value("description", "");
    category.cafe_id = j.value("cafe_id", -1);
    
    if (j.contains("items")) {
        for (const auto& item_json : j["items"]) {
            category.items.push_back(Item::fromJson(item_json));
        }
    }
    return category;
}

bool Category::isValid() const {
    return id > 0 && !name.empty() && cafe_id > 0 && !items.empty();
}