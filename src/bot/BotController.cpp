#include "bot/BotController.h"
#include <sstream>
#include <stdexcept>

BotController::BotController(const std::string& token,
                             db::Database& database,
                             db::UserRepository& userRepository,
                             db::MenuRepository& menuRepository,
                             db::OrderRepository& orderRepository)
    : token_(token),
      database_(database),
      userRepository_(userRepository),
      menuRepository_(menuRepository),
      orderRepository_(orderRepository),
      bot_(std::make_shared<TgBot::Bot>(token)) {
    registerHandlers();
}

void BotController::run() {
    TgBot::TgLongPoll longPoll(*bot_);

    while (true) {
        longPoll.start();
    }
}

void BotController::registerHandlers() {
    bot_->getEvents().onCommand("start", [this](const TgBot::Message::Ptr& message) {
        handleStart(message);
    });

    bot_->getEvents().onCommand("menu", [this](const TgBot::Message::Ptr& message) {
        handleMenu(message);
    });

    bot_->getEvents().onCommand("help", [this](const TgBot::Message::Ptr& message) {
        handleHelp(message);
    });

    bot_->getEvents().onCallbackQuery([this](const TgBot::CallbackQuery::Ptr& query) {
        handleCallback(query);
    });
}

void BotController::handleStart(const TgBot::Message::Ptr& message) {
    ensureUserExists(message);

    std::ostringstream text;
    text << "Привет! Я @" << "CoffeeHSEbot" << ".\n";
    text << "Помогу выбрать кофейню и посмотреть меню.\n\n";
    text << buildCafeSelectionText();

    answerMessage(message->chat->id, text.str(), buildCafeKeyboard());
}

void BotController::handleMenu(const TgBot::Message::Ptr& message) {
    ensureUserExists(message);
    answerMessage(message->chat->id, buildCafeSelectionText(), buildCafeKeyboard());
}

void BotController::handleHelp(const TgBot::Message::Ptr& message) {
    answerMessage(
        message->chat->id,
        "Команды:\n"
        "/start - начать работу\n"
        "/menu - выбрать кофейню и посмотреть меню\n"
        "/help - показать помощь"
    );
}

void BotController::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    const std::string prefix = "cafe:";
    if (query->data.rfind(prefix, 0) == 0) {
        int cafeId = std::stoi(query->data.substr(prefix.size()));
        bot_->getApi().answerCallbackQuery(query->id, "Загружаю меню");
        answerMessage(query->message->chat->id, buildCafeMenuText(cafeId));
        return;
    }

    bot_->getApi().answerCallbackQuery(query->id, "Неизвестное действие");
}

void BotController::ensureUserExists(const TgBot::Message::Ptr& message) {
    const long long telegramId = message->from->id;
    if (userRepository_.findByTelegramId(telegramId).has_value()) {
        return;
    }

    std::string username = message->from->username;
    if (username.empty()) {
        username = message->from->firstName;
    }
    if (username.empty()) {
        username = "user_" + std::to_string(telegramId);
    }

    userRepository_.create(telegramId, username);
}

std::vector<BotController::CafeView> BotController::fetchCafes() const {
    auto stmt = database_.prepare("SELECT id, name, location FROM cafes ORDER BY id;");
    std::vector<CafeView> cafes;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        cafes.push_back({
            sqlite3_column_int(stmt.get(), 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2))
        });
    }

    if (rc != SQLITE_DONE) {
        throw db::DatabaseException("Failed to fetch cafes");
    }

    return cafes;
}

std::vector<Category> BotController::categoriesByCafe(int cafeId) const {
    std::vector<Category> categories;
    for (const auto& category : menuRepository_.findAllCategories()) {
        if (category.get_cafe_id() == cafeId) {
            categories.push_back(category);
        }
    }
    return categories;
}

std::string BotController::buildCafeSelectionText() const {
    std::ostringstream text;
    text << "Выбери кофейню:\n";

    for (const auto& cafe : fetchCafes()) {
        text << cafe.id << ". " << cafe.name << " (" << cafe.location << ")\n";
    }

    return text.str();
}

std::string BotController::buildCafeMenuText(int cafeId) const {
    std::optional<CafeView> selectedCafe;
    for (const auto& cafe : fetchCafes()) {
        if (cafe.id == cafeId) {
            selectedCafe = cafe;
            break;
        }
    }

    if (!selectedCafe.has_value()) {
        return "Кофейня не найдена.";
    }

    const auto categories = categoriesByCafe(cafeId);
    const auto items = menuRepository_.findAllItems();

    std::ostringstream text;
    text << "Меню кофейни " << selectedCafe->name << ":\n";

    if (categories.empty()) {
        text << "\nПока нет доступных категорий.";
        return text.str();
    }

    for (const auto& category : categories) {
        text << "\n[" << category.get_name() << "]\n";
        bool hasItems = false;

        for (const auto& item : items) {
            if (item.get_cafe_id() == cafeId && item.get_category_id() == category.get_id()) {
                hasItems = true;
                text << "- " << item.get_name()
                     << " | " << item.get_price() << " руб.\n";
            }
        }

        if (!hasItems) {
            text << "- Пока пусто\n";
        }
    }

    return text.str();
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCafeKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    for (const auto& cafe : fetchCafes()) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = cafe.name;
        button->callbackData = "cafe:" + std::to_string(cafe.id);
        keyboard->inlineKeyboard.push_back({button});
    }

    return keyboard;
}

void BotController::answerMessage(std::int64_t chatId, const std::string& text,
                                  const TgBot::GenericReply::Ptr& markup) const {
    bot_->getApi().sendMessage(chatId, text, nullptr, nullptr, markup);
}