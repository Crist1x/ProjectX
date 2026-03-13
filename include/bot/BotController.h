#ifndef PROJECTX_BOTCONTROLLER_H
#define PROJECTX_BOTCONTROLLER_H

#include "db/Database.h"
#include "db/MenuRepository.h"
#include "db/OrderRepository.h"
#include "db/UserRepository.h"
#include <memory>
#include <string>
#include <map>
#include <vector>
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

    // Registration
    void registerHandlers();

    // Commands
    void handleStart(const TgBot::Message::Ptr& message);
    void handleMenu(const TgBot::Message::Ptr& message);
    void handleHelp(const TgBot::Message::Ptr& message);
    void handleCart(const TgBot::Message::Ptr& message);

    // Callbacks
    void handleCallback(const TgBot::CallbackQuery::Ptr& query);
    void handleCafeSelection(const TgBot::CallbackQuery::Ptr& query);
    void handleCategorySelection(const TgBot::CallbackQuery::Ptr& query);
    void handleAddToCart(const TgBot::CallbackQuery::Ptr& query);
    void handleCheckout(const TgBot::CallbackQuery::Ptr& query);
    void handleClearCart(const TgBot::CallbackQuery::Ptr& query);
    void handleBackToCategories(const TgBot::CallbackQuery::Ptr& query);
    void handleBackToCafes(const TgBot::CallbackQuery::Ptr& query);

    // Helpers
    void ensureUserExists(const TgBot::Message::Ptr& message);
    std::vector<CafeView> fetchCafes() const;
    std::vector<Category> categoriesByCafe(int cafeId) const;
    std::vector<Item> itemsByCategory(int categoryId) const;

    // Text builders
    std::string buildCafeSelectionText() const;
    std::string buildCafeMenuText(int cafeId) const;
    std::string buildCategoryItemsText(int cafeId, int categoryId) const;
    std::string buildCartText(std::int64_t chatId) const;

    // Keyboard builders
    TgBot::InlineKeyboardMarkup::Ptr buildCafeKeyboard() const;
    TgBot::InlineKeyboardMarkup::Ptr buildCategoryKeyboard(int cafeId) const;
    TgBot::InlineKeyboardMarkup::Ptr buildItemKeyboard(int cafeId, int categoryId) const;
    TgBot::InlineKeyboardMarkup::Ptr buildCartKeyboard() const;

    // Utilities
    void answerMessage(std::int64_t chatId, const std::string& text,
                       const TgBot::GenericReply::Ptr& markup = nullptr) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;

    std::string token_;
    db::Database& database_;
    db::UserRepository& userRepository_;
    db::MenuRepository& menuRepository_;
    db::OrderRepository& orderRepository_;
    std::shared_ptr<TgBot::Bot> bot_;

    // Cart storage (per chat): chatId → [(menuItemId, quantity)]
    std::map<std::int64_t, std::vector<std::pair<int, int>>> userCarts_;

    // Selected cafe/category per user
    std::map<std::int64_t, int> userSelectedCafe_;
    std::map<std::int64_t, int> userSelectedCategory_;
};

#endif