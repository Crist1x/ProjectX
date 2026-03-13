#include "db/MenuRepository.h"
#include <sqlite3.h>

namespace db {
MenuRepository::MenuRepository(Database& database) : db_(database) {}

bool MenuRepository::createCategory(const std::string& name) {
    const char* sql = "INSERT INTO menu_categories (name) VALUES (?);";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to create category: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::optional<MenuCategory> MenuRepository::findCategoryById(int id) {
    const char* sql = "SELECT id, name FROM menu_categories WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    MenuCategory category;
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        category.id = sqlite3_column_int(stmt.get(), 0);
        category.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        return category;
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find category by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<MenuCategory> MenuRepository::findAllCategories() {
    const char* sql = "SELECT id, name FROM menu_categories;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<MenuCategory> categories;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        MenuCategory category;
        category.id = sqlite3_column_int(stmt.get(), 0);
        category.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        categories.push_back(category);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch categories: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return categories;
}

bool MenuRepository::deleteCategory(int id) {
    const char* sql = "DELETE FROM menu_categories WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to delete category: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}

bool MenuRepository::createItem(const std::string& name,
                                const std::string& description,
                                double price,
                                int categoryId) {
    const char* sql = "INSERT INTO menu_items (name, description, price, category_id) VALUES (?, ?, ?, ?);";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt.get(), 3, price);
    sqlite3_bind_int(stmt.get(), 4, categoryId);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to create item: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::optional<MenuItem> MenuRepository::findItemById(int id) {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    MenuItem item;
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        item.id = sqlite3_column_int(stmt.get(), 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        item.price = sqlite3_column_double(stmt.get(), 3);
        item.categoryId = sqlite3_column_int(stmt.get(), 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 5));
        return item;
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find item by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<MenuItem> MenuRepository::findAllItems() {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<MenuItem> items;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        MenuItem item;
        item.id = sqlite3_column_int(stmt.get(), 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        item.price = sqlite3_column_double(stmt.get(), 3);
        item.categoryId = sqlite3_column_int(stmt.get(), 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 5));
        items.push_back(item);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch menu items: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return items;
}

std::vector<MenuItem> MenuRepository::findItemsByCategory(int categoryId) {
    const char* sql = "SELECT id, name, description, price, category_id, created_at FROM menu_items WHERE category_id = ?;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<MenuItem> items;

    sqlite3_bind_int(stmt.get(), 1, categoryId);

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        MenuItem item;
        item.id = sqlite3_column_int(stmt.get(), 0);
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        item.price = sqlite3_column_double(stmt.get(), 3);
        item.categoryId = sqlite3_column_int(stmt.get(), 4);
        item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 5));
        items.push_back(item);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch items by category: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return items;
}

bool MenuRepository::updateItem(int id,
                                const std::string& name,
                                const std::string& description,
                                double price) {
    const char* sql = "UPDATE menu_items SET name = ?, description = ?, price = ? WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt.get(), 3, price);
    sqlite3_bind_int(stmt.get(), 4, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to update item: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}

bool MenuRepository::deleteItem(int id) {
    const char* sql = "DELETE FROM menu_items WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to delete item: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}
}
