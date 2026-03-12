#include "db/UserRepository.h"
#include <sqlite3.h>
#include <iostream>

namespace db {
UserRepository::UserRepository(Database& database) : db_(database) {}

bool UserRepository::create(long long telegramId, const std::string& username) {
    const char* sql = "INSERT INTO users (telegram_id, username) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, telegramId);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::optional<User> UserRepository::findById(int id) {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    User user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.telegramId = sqlite3_column_int64(stmt, 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return user;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> UserRepository::findByTelegramId(long long telegramId) {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users WHERE telegram_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int64(stmt, 1, telegramId);

    User user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.telegramId = sqlite3_column_int64(stmt, 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return user;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<User> UserRepository::findAll() {
    const char* sql = "SELECT id, telegram_id, username, created_at FROM users;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<User> users;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return users;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.telegramId = sqlite3_column_int64(stmt, 1);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        users.push_back(user);
    }

    sqlite3_finalize(stmt);
    return users;
}

bool UserRepository::updateUsername(int id, const std::string& newUsername) {
    const char* sql = "UPDATE users SET username = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, newUsername.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool UserRepository::remove(int id) {
    const char* sql = "DELETE FROM users WHERE id = ?;";
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