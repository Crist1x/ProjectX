#ifndef PROJECTX_MENU_H
#define PROJECTX_MENU_H

#include "MenuCategory.h"
#include <vector>
#include <string>
#include <memory>
#include "../../cmake/json.hpp"
#include "TelegramFormat.h"

using json = nlohmann::json;

class Logger;
class Database;

class Menu: TelegramFormat {
private:
    int cafe_id{};
    std::string cafe_name;
    // Применили тот же фикс: теперь категории живут в куче, указатели безопасны
    std::vector<std::shared_ptr<Category>> categories;
    std::weak_ptr<Logger> logger;
    std::weak_ptr<Database> database;

public:
    Menu() = default;
    Menu(int cafe_id, const std::string& cafe_name);
    Menu(int cafe_id, const std::string& cafe_name,
         std::shared_ptr<Logger> logger, std::shared_ptr<Database> db);

    // Геттеры и сеттеры
    int get_cafe_id() const;
    const std::string& get_cafe_name() const;
    const std::vector<std::shared_ptr<Category>>& get_categories() const;

    void set_cafe_name(const std::string& name);

    // Управление
    void add_category(const Category& category);
    void remove_category(int category_id);
    Category* find_category(int category_id);
    const Category* find_category(int category_id) const;

    Item* find_item(int item_id);
    const Item* find_item(int item_id) const;

    // Загрузка/сохранение
    void load_from_database();
    void save_to_database() const;

    std::string to_telegram_format() const override;

    // JSON
    json toJson() const;
    static Menu fromJson(const json& j);

    // Информация
    size_t get_categories_count() const;
    size_t get_total_items_count() const;
    double get_total_price_range_min() const;
    double get_total_price_range_max() const;

    bool isValid() const;

private:
    void log_info(const std::string& message) const;
    void log_error(const std::string& message) const;
};

#endif