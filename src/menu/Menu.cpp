#include "menu/Menu.h"
//TODO: потом добавиьт сюда инклюд класса дб и дописать пару функций
#include <sstream>
#include <algorithm>
#include <stdexcept>

// Конструкторы
Menu::Menu(int cafe_id, const std::string& cafe_name)
    : cafe_id(cafe_id), cafe_name(cafe_name) {}

Menu::Menu(int cafe_id, const std::string& cafe_name,
           std::shared_ptr<Logger> logger, std::shared_ptr<Database> db)
    : cafe_id(cafe_id), cafe_name(cafe_name), logger(logger), database(db) {
    log_info("Menu created for cafe: " + cafe_name + " (ID: " + std::to_string(cafe_id) + ")");
}

// Геттеры
int Menu::get_cafe_id() const { return cafe_id; }

const std::string& Menu::get_cafe_name() const { return cafe_name; }

const std::vector<std::shared_ptr<Category>>& Menu::get_categories() const { return categories; }

// Сеттеры
void Menu::set_cafe_name(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Cafe name cannot be empty");
    }
    cafe_name = name;
}

// Управление категориями
void Menu::add_category(const Category& category) {
    if (!category.isValid()) {
        throw std::invalid_argument("Cannot add invalid category");
    }
    if (category.get_cafe_id() != cafe_id) {
        throw std::invalid_argument("Category cafe_id does not match menu cafe_id");
    }
    categories.push_back(std::make_shared<Category>(category));
    log_info("Category added: " + category.get_name());
}

void Menu::remove_category(int category_id) {
    categories.erase(
        std::remove_if(categories.begin(), categories.end(),
            [category_id](const std::shared_ptr<Category>& cat) { return cat->get_id() == category_id; }),
        categories.end()
    );
    log_info("Category removed: ID " + std::to_string(category_id));
}

Category* Menu::find_category(int category_id) {
    for (auto& cat : categories) {
        if (cat->get_id() == category_id) {
            return cat.get();
        }
    }
    return nullptr;
}

const Category* Menu::find_category(int category_id) const {
    for (const auto& cat : categories) {
        if (cat->get_id() == category_id) {
            return cat.get();
        }
    }
    return nullptr;
}

// Поиск блюда
Item* Menu::find_item(int item_id) {
    for (auto& cat : categories) {
        Item* item = cat->find_item(item_id);
        if (item) return item;
    }
    return nullptr;
}

const Item* Menu::find_item(int item_id) const {
    for (const auto& cat : categories) {
        const Item* item = cat->find_item(item_id);
        if (item) return item;
    }
    return nullptr;
}

// БД (Реализует ОЛЕГ)
void Menu::load_from_database() {
    log_info("Loading menu from database for cafe " + std::to_string(cafe_id));
    // TODO: реализовать
}

void Menu::save_to_database() const {
    log_info("Saving menu to database");
    // TODO: реализовать
}

// Форматирование
std::string Menu::to_telegram_format() const {
    std::ostringstream oss;
    oss << "📋 *Меню: " << cafe_name << "*\n\n";

    for (const auto& cat : categories) {
        oss << "🍽 *" << cat->get_name() << "*\n";
        oss << "_" << cat->get_description() << "_\n\n";

        // Здесь теперь item_ptr, так как Category возвращает вектор умных указателей
        for (const auto& item_ptr : cat->get_items()) {
            if (item_ptr->is_available()) {
                oss << "• " << item_ptr->get_name() << " - "
                    << item_ptr->get_price() << "₽\n";
                oss << "  _" << item_ptr->get_description() << "_\n";
                oss << "  ⏱ " << item_ptr->get_preparation_time() << " мин.\n\n";
            }
        }
    }

    return oss.str();
}

// JSON
json Menu::toJson() const {
    json j = {
        {"cafe_id", cafe_id},
        {"cafe_name", cafe_name},
        {"categories", json::array()}
    };
    for (const auto& cat : categories) {
        j["categories"].push_back(cat->toJson());
    }
    return j;
}

Menu Menu::fromJson(const json& j) {
    Menu menu;
    menu.cafe_id = j.value("cafe_id", -1);
    menu.cafe_name = j.value("cafe_name", "");

    if (j.contains("categories")) {
        for (const auto& cat_json : j["categories"]) {
            menu.categories.push_back(std::make_shared<Category>(Category::fromJson(cat_json)));
        }
    }
    return menu;
}

// Инфа
size_t Menu::get_categories_count() const {
    return categories.size();
}

size_t Menu::get_total_items_count() const {
    size_t total = 0;
    for (const auto& cat : categories) {
        total += cat->get_items_count();
    }
    return total;
}

double Menu::get_total_price_range_min() const {
    double min_price = std::numeric_limits<double>::max();
    for (const auto& cat : categories) {
        if (cat->get_items_count() > 0) {
            min_price = std::min(min_price, cat->get_min_price());
        }
    }
    return (min_price == std::numeric_limits<double>::max()) ? 0.0 : min_price;
}

double Menu::get_total_price_range_max() const {
    double max_price = 0.0;
    for (const auto& cat : categories) {
        max_price = std::max(max_price, cat->get_max_price());
    }
    return max_price;
}

bool Menu::isValid() const {
    return cafe_id > 0 && !cafe_name.empty() && !categories.empty();
}

void Menu::log_info(const std::string& message) const { }

void Menu::log_error(const std::string& message) const { }