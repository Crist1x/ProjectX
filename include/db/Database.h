#ifndef PROJECT_DATABASE_H
#define PROJECT_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <memory>
#include <stdexcept>

namespace db {
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& msg)
        : std::runtime_error(msg) {}
};

struct SqliteDeleter {
    void operator()(sqlite3* ptr) const noexcept {
        if (ptr) {
            sqlite3_close(ptr);
        }
    }
};

struct StatementDeleter {
    void operator()(sqlite3_stmt* ptr) const noexcept {
        if (ptr) {
            sqlite3_finalize(ptr);
        }
    }
};

using StatementPtr = std::unique_ptr<sqlite3_stmt, StatementDeleter>;

class Database {
public:
    static constexpr const char* PRAGMA_FOREIGN_KEYS = "PRAGMA foreign_keys = ON;";
    static constexpr const char* DEFAULT_PATH = ":memory:";

    explicit Database(const std::string& path);
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    Database(Database&&) noexcept = default;
    Database& operator=(Database&&) noexcept = default;

    bool isOpen() const noexcept;

    void execute(const std::string& sql);

    StatementPtr prepare(const std::string& sql);

    sqlite3* getHandle() noexcept;

    bool tryExecute(const std::string& sql) noexcept;

private:
    std::unique_ptr<sqlite3, SqliteDeleter> db_;
};
}

#endif //PROJECT_DATABASE_H
