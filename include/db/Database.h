#ifndef PROJECT_DATABASE_H
#define PROJECT_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace db {
class Database {
private: sqlite3* db_;

public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool isOpen() const noexcept;
    bool execute(const std::string& sql);
    sqlite3* getHandle() noexcept;
};
}

#endif //PROJECT_DATABASE_H