#include "db/MenuRepository.h"
#include <sqlite3.h>

namespace db {
namespace {
Category makeCategory(sqlite3_stmt* stmt) {
    return Category(
        sqlite3_column_int(stmt, 0),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
        "",
        sqlite3_column_int(stmt, 2)
    );
}

Item makeItem(sqlite3_stmt* stmt) {
    return Item(
        sqlite3_column_int(stmt, 0),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
        sqlite3_column_double(stmt, 3),
        1,
        sqlite3_column_int(stmt, 4),
        sqlite3_column_int(stmt, 5),
        true
    );
}
}

MenuRepository::MenuRepository(Database& database) : db_(database) {}

bool MenuRepository::createCategory(const std::string& name, int cafeId) {
    const char* sql = "INSERT INTO menu_categories (name, cafe_id) VALUES (?, ?);";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 2, cafeId);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to create category: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::optional<Category> MenuRepository::findCategoryById(int id) {
    const char* sql = "SELECT id, name, cafe_id FROM menu_categories WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        return makeCategory(stmt.get());
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find category by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<Category> MenuRepository::findAllCategories() {
    const char* sql = "SELECT id, name, cafe_id FROM menu_categories;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<Category> categories;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        categories.push_back(makeCategory(stmt.get()));
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

std::optional<Item> MenuRepository::findItemById(int id) {
    const char* sql =
        "SELECT mi.id, mi.name, mi.description, mi.price, mi.category_id, mc.cafe_id, mi.created_at "
        "FROM menu_items mi "
        "JOIN menu_categories mc ON mc.id = mi.category_id "
        "WHERE mi.id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        return makeItem(stmt.get());
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find item by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<Item> MenuRepository::findAllItems() {
    const char* sql =
        "SELECT mi.id, mi.name, mi.description, mi.price, mi.category_id, mc.cafe_id, mi.created_at "
        "FROM menu_items mi "
        "JOIN menu_categories mc ON mc.id = mi.category_id;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<Item> items;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        items.push_back(makeItem(stmt.get()));
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch menu items: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return items;
}

std::vector<Item> MenuRepository::findItemsByCategory(int categoryId) {
    const char* sql =
        "SELECT mi.id, mi.name, mi.description, mi.price, mi.category_id, mc.cafe_id, mi.created_at "
        "FROM menu_items mi "
        "JOIN menu_categories mc ON mc.id = mi.category_id "
        "WHERE mi.category_id = ?;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<Item> items;

    sqlite3_bind_int(stmt.get(), 1, categoryId);

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        items.push_back(makeItem(stmt.get()));
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
