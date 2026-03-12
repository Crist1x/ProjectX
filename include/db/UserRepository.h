#ifndef PROJECT_USERREPOSITORY_H
#define PROJECT_USERREPOSITORY_H

#include "db/Database.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
struct User {
    int id;
    long long telegramId;
    std::string username;
    std::string createdAt;
};

class UserRepository {
public:
    explicit UserRepository(Database& database);

    bool create(long long telegramId, const std::string& username);
    std::optional<User> findById(int id);
    std::optional<User> findByTelegramId(long long telegramId);
    std::vector<User> findAll();
    bool updateUsername(int id, const std::string& newUsername);
    bool remove(int id);

private:
    Database& db_;
};
}

#endif //PROJECT_USERREPOSITORY_H