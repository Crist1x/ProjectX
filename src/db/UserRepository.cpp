#include "db/UserRepository.h"
#include <sqlite3.h>

namespace db {
UserRepository::UserRepository(Database& database) : db_(database) {}

bool UserRepository::create(long long telegramId, const std::string& username) {
    const char* sql = "INSERT INTO users (telegram_id, username) VALUES (?, ?);";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int64(stmt.get(), 1, telegramId);
    sqlite3_bind_text(stmt.get(), 2, username.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to create user: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::optional<User> UserRepository::findById(int id) {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    User user;
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt.get(), 0);
        user.telegramId = sqlite3_column_int64(stmt.get(), 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        return user;
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find user by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::optional<User> UserRepository::findByTelegramId(long long telegramId) {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users WHERE telegram_id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int64(stmt.get(), 1, telegramId);

    User user;
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt.get(), 0);
        user.telegramId = sqlite3_column_int64(stmt.get(), 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        return user;
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find user by telegram id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<User> UserRepository::findAll() {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<User> users;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt.get(), 0);
        user.telegramId = sqlite3_column_int64(stmt.get(), 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        users.push_back(user);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch users: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return users;
}

bool UserRepository::updateUsername(int id, const std::string& newUsername) {
    const char* sql = "UPDATE users SET username = ? WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, newUsername.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 2, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to update username: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}

bool UserRepository::remove(int id) {
    const char* sql = "DELETE FROM users WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to remove user: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}
}
