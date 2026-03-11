#include "db/MenuRepository.h"
#include <sqlite3.h>
#include <iostream>

namespace db {
MenuRepository::MenuRepository(Database& database) : db_(database) {}

bool MenuRepository::createCategory(const std::string& name) {
    const char* sql = "INSERT INTO menu_categories (name) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::optional<MenuCategory> MenuRepository::findCategoryById(int id) {
    const char* sql = "SELECT id, name FROM menu_categories WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    MenuCategory category;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        category.id = sqlite3_column_int(stmt, 0);
        category.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);
        return category;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<MenuCategory> MenuRepository::findAllCategories() {
    const char* sql = "SELECT id, name FROM menu_categories;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<MenuCategory> categories;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return categories;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MenuCategory category;
        category.id = sqlite3_column_int(stmt, 0);
        category.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        categories.push_back(category);
    }

    sqlite3_finalize(stmt);
    return categories;
}

bool MenuRepository::deleteCategory(int id) {
    const char* sql = "DELETE FROM menu_categories WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool MenuRepository::createItem(const std::string& name,
                                const std::string& description,
                                double price,
                                int categoryId) {
    const char* sql = "INSERT INTO menu_items (name, description, price, category_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        return false;
    }

    // Привязываем 4 параметра
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, price);
    sqlite3_bind_int(stmt, 4, categoryId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::optional<MenuItem> MenuRepository::findItemById(int id) {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    MenuItem item;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        item.id = sqlite3_column_int(stmt, 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.price = sqlite3_column_double(stmt, 3);
        item.categoryId = sqlite3_column_int(stmt, 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sqlite3_finalize(stmt);
        return item;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<MenuItem> MenuRepository::findAllItems() {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<MenuItem> items;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return items;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MenuItem item;
        item.id = sqlite3_column_int(stmt, 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.price = sqlite3_column_double(stmt, 3);
        item.categoryId = sqlite3_column_int(stmt, 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        items.push_back(item);
    }

    sqlite3_finalize(stmt);
    return items;
}

std::vector<MenuItem> MenuRepository::findItemsByCategory(int categoryId) {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items WHERE category_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<MenuItem> items;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return items;
    }

    sqlite3_bind_int(stmt, 1, categoryId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MenuItem item;
        item.id = sqlite3_column_int(stmt, 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.price = sqlite3_column_double(stmt, 3);
        item.categoryId = sqlite3_column_int(stmt, 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        items.push_back(item);
    }

    sqlite3_finalize(stmt);
    return items;
}

bool MenuRepository::updateItem(int id,
                                const std::string& name,
                                const std::string& description,
                                double price) {
    const char* sql = "UPDATE menu_items SET name = ?, description = ?, price = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, price);
    sqlite3_bind_int(stmt, 4, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

//Dèlēté item
bool MenuRepository::deleteItem(int id) {
    const char* sql = "DELETE FROM menu_items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}
}