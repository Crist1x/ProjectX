#include "db/Database.h"
#include <array>
#include <sstream>

namespace db {
Database::Database(const std::string& path) {
    sqlite3* rawPtr = nullptr;

    int result = sqlite3_open(path.c_str(), &rawPtr);

    if (result != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Failed to open database '" << path << "': "
            << sqlite3_errmsg(rawPtr);

        if (rawPtr) {
            sqlite3_close(rawPtr);
        }

        throw DatabaseException(oss.str());
    }

    db_.reset(rawPtr);

    execute(PRAGMA_FOREIGN_KEYS);
}

bool Database::isOpen() const noexcept {
    return db_ != nullptr;
}

void Database::initializeSchema() {
    static constexpr std::array<const char*, 10> schemaStatements = {
        "CREATE TABLE IF NOT EXISTS cafes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE,"
        "location TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "telegram_id INTEGER NOT NULL UNIQUE,"
        "username TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");",

        "CREATE TABLE IF NOT EXISTS menu_categories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "cafe_id INTEGER NOT NULL,"
        "FOREIGN KEY(cafe_id) REFERENCES cafes(id) ON DELETE CASCADE"
        ");",

        "CREATE TABLE IF NOT EXISTS menu_items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "price REAL NOT NULL CHECK(price >= 0),"
        "category_id INTEGER NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(category_id) REFERENCES menu_categories(id) ON DELETE CASCADE"
        ");",

        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "total_amount REAL NOT NULL CHECK(total_amount >= 0),"
        "status TEXT NOT NULL DEFAULT 'new',"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");",

        "CREATE TABLE IF NOT EXISTS order_items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "order_id INTEGER NOT NULL,"
        "menu_item_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "price REAL NOT NULL CHECK(price >= 0),"
        "FOREIGN KEY(order_id) REFERENCES orders(id) ON DELETE CASCADE,"
        "FOREIGN KEY(menu_item_id) REFERENCES menu_items(id) ON DELETE CASCADE"
        ");",

        "CREATE INDEX IF NOT EXISTS idx_menu_categories_cafe_id ON menu_categories(cafe_id);",
        "CREATE INDEX IF NOT EXISTS idx_menu_items_category_id ON menu_items(category_id);",
        "CREATE INDEX IF NOT EXISTS idx_orders_user_id ON orders(user_id);",
        "CREATE INDEX IF NOT EXISTS idx_order_items_order_id ON order_items(order_id);"
    };

    for (const char* statement : schemaStatements) {
        execute(statement);
    }

    execute(
        "INSERT OR IGNORE INTO cafes (id, name, location) VALUES "
        "(1, 'Груша', 'Главный корпус'),"
        "(2, 'Джефрис', 'Корпус Джефрис'),"
        "(3, 'Стекляшка', 'Стекляшка');"
    );
}

void Database::execute(const std::string& sql) {
    if (!db_) {
        throw DatabaseException("Database not open");
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_.get(), sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::string errorMsg = errMsg ? errMsg : "Unknown SQLite error";

        sqlite3_free(errMsg);

        throw DatabaseException("SQL execution failed: " + errorMsg);
    }
}

StatementPtr Database::prepare(const std::string& sql) {
    if (!db_) {
        throw DatabaseException("Database not open");
    }

    sqlite3_stmt* rawStmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &rawStmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_.get())));
    }

    return StatementPtr(rawStmt);
}

bool Database::tryExecute(const std::string& sql) noexcept {
    if (!db_) {
        return false;
    }

    char* errMsg = nullptr;
    int result = sqlite3_exec(db_.get(), sql.c_str(), nullptr, nullptr, &errMsg);

    if (result != SQLITE_OK) {
        if (errMsg) {
            sqlite3_free(errMsg);
        }
        return false;
    }

    return true;
}

sqlite3* Database::getHandle() noexcept {
    return db_.get();
}

}
