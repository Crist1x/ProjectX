#ifndef PROJECTX_BOTCONTROLLER_H
#define PROJECTX_BOTCONTROLLER_H

#include "db/Database.h"
#include "db/MenuRepository.h"
#include "db/OrderRepository.h"
#include "db/UserRepository.h"
#include <memory>
#include <string>
#include <tgbot/tgbot.h>

class BotController {
public:
    BotController(const std::string& token,
                  db::Database& database,
                  db::UserRepository& userRepository,
                  db::MenuRepository& menuRepository,
                  db::OrderRepository& orderRepository);

    void run();

private:
    struct CafeView {
        int id;
        std::string name;
        std::string location;
    };

    void registerHandlers();
    void handleStart(const TgBot::Message::Ptr& message);
    void handleMenu(const TgBot::Message::Ptr& message);
    void handleHelp(const TgBot::Message::Ptr& message);
    void handleCallback(const TgBot::CallbackQuery::Ptr& query);

    void ensureUserExists(const TgBot::Message::Ptr& message);
    std::vector<CafeView> fetchCafes() const;
    std::vector<Category> categoriesByCafe(int cafeId) const;
    std::string buildCafeSelectionText() const;
    std::string buildCafeMenuText(int cafeId) const;
    TgBot::InlineKeyboardMarkup::Ptr buildCafeKeyboard() const;
    void answerMessage(std::int64_t chatId, const std::string& text,
                       const TgBot::GenericReply::Ptr& markup = nullptr) const;

    std::string token_;
    db::Database& database_;
    db::UserRepository& userRepository_;
    db::MenuRepository& menuRepository_;
    db::OrderRepository& orderRepository_;
    std::shared_ptr<TgBot::Bot> bot_;
};

#endif