#include "db/Database.h"
#include <iostream>

namespace db {
    Database::Database(const std::string& path) : db_(nullptr) {
        int result = sqlite3_open(path.c_str(), &db_);

        if (result != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("Failed to open database");
        }

        execute("PRAGMA foreign_keys = ON;");
    }

    Database::~Database() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    bool Database::isOpen() const noexcept {
        return db_ != nullptr;
    }

    bool Database::execute(const std::string& sql) {
        if (!db_) {
            return false;
        }

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);

        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        return true;
    }

    sqlite3* Database::getHandle() noexcept {
        return db_;
    }
}