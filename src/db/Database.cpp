#include "db/Database.h"
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