#include "bot/BotController.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iomanip>

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
    std::cout << "[BotController] Initializing..." << std::endl;

    cachedCafes_ = fetchCafes();

    registerHandlers();
    std::cout << "[BotController] Ready!" << std::endl;
}

void BotController::run() {
    std::cout << "[Bot] Starting long poll..." << std::endl;
    TgBot::TgLongPoll longPoll(*bot_);

    while (true) {
        try {
            longPoll.start();
        } catch (const std::exception& e) {
            std::cerr << "[Bot] Long poll error: " << e.what() << std::endl;
        }
    }
}

void BotController::registerHandlers() {
    std::cout << "[Bot] Registering handlers..." << std::endl;

    bot_->getEvents().onCommand("start", [this](const TgBot::Message::Ptr& message) {
        handleStart(message);
    });

    bot_->getEvents().onCommand("menu", [this](const TgBot::Message::Ptr& message) {
        handleMenu(message);
    });

    bot_->getEvents().onCommand("help", [this](const TgBot::Message::Ptr& message) {
        handleHelp(message);
    });

    bot_->getEvents().onCommand("cart", [this](const TgBot::Message::Ptr& message) {
        handleCart(message);
    });

    bot_->getEvents().onCommand("profile", [this](const TgBot::Message::Ptr& message) {
        handleProfile(message);
    });

    bot_->getEvents().onCallbackQuery([this](const TgBot::CallbackQuery::Ptr& query) {
        handleCallback(query);
    });

    std::cout << "[Bot] Handlers registered!" << std::endl;
}

void BotController::handleStart(const TgBot::Message::Ptr& message) {
    auto user = ensureUserExists(message);

    std::string text = "Привет, " + user->username + "!\n\n";
    text += "Добро пожаловать в CoffeeHSE!\n\n";
    text += "Выберите кофейню ниже:";

    answerMessage(message->chat->id, text, buildCafeKeyboard());
}

void BotController::handleMenu(const TgBot::Message::Ptr& message) {
    ensureUserExists(message);
    answerMessage(message->chat->id, "Выберите кофейню:", buildCafeKeyboard());
}

void BotController::handleHelp(const TgBot::Message::Ptr& message) {
    ensureUserExists(message);

    std::string text = "Помощь\n\n";
    text += "Команды:\n";
    text += "/start - главное меню\n";
    text += "/menu - выбрать кофейню\n";
    text += "/cart - корзина\n";
    text += "/profile - твой профиль\n";
    text += "/help - справка\n\n";
    text += "Как сделать заказ:\n";
    text += "1. Нажми /menu\n";
    text += "2. Выбери кофейню\n";
    text += "3. Выбери категорию\n";
    text += "4. Нажми на товар\n";
    text += "5. Открой /cart и оформи";

    answerMessage(message->chat->id, text);
}

void BotController::handleCart(const TgBot::Message::Ptr& message) {
    ensureUserExists(message);
    answerMessage(message->chat->id, buildCartText(message->chat->id), buildCartKeyboard());
}

void BotController::handleProfile(const TgBot::Message::Ptr& message) {
    auto user = ensureUserExists(message);
    auto orders = orderRepository_.findOrdersByUser(user->id);

    std::string text = "Твой профиль\n\n";
    text += "Имя: " + user->username + "\n";
    text += "Telegram ID: " + std::to_string(user->telegramId) + "\n";
    text += "Всего заказов: " + std::to_string(orders.size()) + "\n\n";

    if (!orders.empty()) {
        text += "Последние заказы:\n";
        int count = 0;
        for (auto it = orders.rbegin(); it != orders.rend() && count < 5; ++it, ++count) {
            text += "- Заказ #" + std::to_string(it->get_id()) + ": " + it->get_status() + "\n";
        }
    } else {
        text += "У тебя пока нет заказов\n";
    }

    answerMessage(message->chat->id, text);
}

void BotController::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    const std::string& data = query->data;

    if (data.rfind("cafe:", 0) == 0) {
        handleCafeSelection(query);
    }
    else if (data.rfind("category:", 0) == 0) {
        handleCategorySelection(query);
    }
    else if (data.rfind("add:", 0) == 0) {
        handleAddToCart(query);
    }
    else if (data == "checkout") {
        handleCheckout(query);
    }
    else if (data == "cart_clear") {
        handleClearCart(query);
    }
    else if (data.rfind("back:cat:", 0) == 0) {
        handleBackToCategories(query);
    }
    else if (data.rfind("back:cafes:", 0) == 0) {
        handleBackToCafes(query);
    }
    else {
        bot_->getApi().answerCallbackQuery(query->id, "Неизвестное действие");
    }
}

void BotController::handleCafeSelection(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
        return;
    }

    try {
        int cafeId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;
        userSelectedCafe_[chatId] = cafeId;

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю...");
        answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleCategorySelection(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
        return;
    }

    try {
        int categoryId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;
        userSelectedCategory_[chatId] = categoryId;

        auto cafeIt = userSelectedCafe_.find(chatId);
        int cafeId = (cafeIt != userSelectedCafe_.end()) ? cafeIt->second : -1;

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю...");
        answerMessage(chatId, buildCategoryItemsText(cafeId, categoryId), buildItemKeyboard(cafeId, categoryId));
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleAddToCart(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
        return;
    }

    try {
        int menuItemId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;

        auto& cart = userCarts_[chatId];
        bool found = false;
        for (auto& item : cart) {
            if (item.first == menuItemId) {
                item.second++;
                found = true;
                break;
            }
        }
        if (!found) {
            cart.emplace_back(menuItemId, 1);
        }

        auto itemOpt = menuRepository_.findItemById(menuItemId);
        std::string itemName = itemOpt.has_value() ? itemOpt->get_name() : "Товар";

        bot_->getApi().answerCallbackQuery(query->id, itemName + " добавлен!");
        answerMessage(chatId, buildCartText(chatId), buildCartKeyboard());
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleCheckout(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    auto& cart = userCarts_[chatId];

    if (cart.empty()) {
        bot_->getApi().answerCallbackQuery(query->id, "Корзина пуста");
        return;
    }

    auto telegramId = query->from->id;
    auto user = userRepository_.findByTelegramId(telegramId);
    if (!user.has_value()) {
        bot_->getApi().answerCallbackQuery(query->id, "Пользователь не найден");
        return;
    }

    try {
        bool success = orderRepository_.createOrderWithItems(user->id, cart);

        if (success) {
            cart.clear();
            bot_->getApi().answerCallbackQuery(query->id, "Заказ оформлен!");
            answerMessage(chatId, "Заказ оформлен!\nОжидайте подтверждения.");
        } else {
            bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
        }
    } catch (const std::exception& e) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: " + std::string(e.what()));
    }
}

void BotController::handleClearCart(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    userCarts_[chatId].clear();

    bot_->getApi().answerCallbackQuery(query->id, "Корзина очищена");
    answerMessage(chatId, buildCartText(chatId), buildCartKeyboard());
}

void BotController::handleBackToCategories(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    auto cafeIt = userSelectedCafe_.find(chatId);

    if (cafeIt != userSelectedCafe_.end()) {
        int cafeId = cafeIt->second;
        answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
    } else {
        answerMessage(chatId, "Выберите кофейню:", buildCafeKeyboard());
    }
}

void BotController::handleBackToCafes(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    answerMessage(chatId, "Выберите кофейню:", buildCafeKeyboard());
}

std::optional<db::User> BotController::ensureUserExists(const TgBot::Message::Ptr& message) {
    const long long telegramId = message->from->id;

    auto existingUser = userRepository_.findByTelegramId(telegramId);
    if (existingUser.has_value()) {
        return existingUser;
    }

    std::string username = message->from->username;
    if (username.empty()) {
        username = message->from->firstName;
    }
    if (username.empty()) {
        username = "user_" + std::to_string(telegramId);
    }

    userRepository_.create(telegramId, username);
    return userRepository_.findByTelegramId(telegramId);
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

std::vector<Item> BotController::itemsByCategory(int categoryId) const {
    return menuRepository_.findItemsByCategory(categoryId);
}

std::string BotController::buildCafeMenuText(int cafeId) const {
    std::optional<CafeView> selectedCafe;
    for (const auto& cafe : cachedCafes_) {
        if (cafe.id == cafeId) {
            selectedCafe = cafe;
            break;
        }
    }

    if (!selectedCafe.has_value()) {
        return "Кофейня не найдена.";
    }

    const auto categories = categoriesByCafe(cafeId);

    std::ostringstream text;
    text << selectedCafe->name << "\n";
    text << selectedCafe->location << "\n\n";
    text << "Категории:\n";

    if (categories.empty()) {
        text << "Пока нет категорий.";
        return text.str();
    }

    for (const auto& category : categories) {
        text << "- " << category.get_name() << "\n";
    }

    text << "\nВыберите категорию:";
    return text.str();
}

std::string BotController::buildCategoryItemsText(int cafeId, int categoryId) const {
    std::optional<CafeView> selectedCafe;
    for (const auto& cafe : cachedCafes_) {
        if (cafe.id == cafeId) {
            selectedCafe = cafe;
            break;
        }
    }

    std::optional<Category> selectedCategory;
    for (const auto& cat : menuRepository_.findAllCategories()) {
        if (cat.get_id() == categoryId) {
            selectedCategory = cat;
            break;
        }
    }

    if (!selectedCafe.has_value() || !selectedCategory.has_value()) {
        return "Не найдено.";
    }

    const auto items = itemsByCategory(categoryId);

    std::ostringstream text;
    text << selectedCafe->name << "\n";
    text << selectedCategory->get_name() << "\n\n";

    if (items.empty()) {
        text << "Пока пусто.";
        return text.str();
    }

    for (const auto& item : items) {
        text << "- " << item.get_name() << ": " << static_cast<int>(item.get_price()) << " руб\n";
        if (!item.get_description().empty()) {
            text << "  " << item.get_description() << "\n";
        }
    }

    text << "\nНажми на товар для добавления.";
    return text.str();
}

std::string BotController::buildCartText(std::int64_t chatId) const {
    auto it = userCarts_.find(chatId);

    if (it == userCarts_.end() || it->second.empty()) {
        return "Корзина пуста\n\nДобавь товары из меню!";
    }

    std::ostringstream text;
    text << "Корзина:\n\n";

    double total = 0.0;
    int itemCount = 0;

    for (const auto& [menuItemId, qty] : it->second) {
        auto itemOpt = menuRepository_.findItemById(menuItemId);
        if (itemOpt.has_value()) {
            double itemTotal = itemOpt->get_price() * qty;
            total += itemTotal;
            itemCount += qty;
            text << "- " << itemOpt->get_name() << " x" << qty << " = " << static_cast<int>(itemTotal) << " руб\n";
        }
    }

    text << "\n----------------\n";
    text << "Итого: " << static_cast<int>(total) << " руб\n";
    text << "Товаров: " << itemCount;

    return text.str();
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCafeKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    for (const auto& cafe : cachedCafes_) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = cafe.name;
        button->callbackData = "cafe:" + std::to_string(cafe.id);
        keyboard->inlineKeyboard.push_back({button});
    }

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCategoryKeyboard(int cafeId) const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    const auto categories = categoriesByCafe(cafeId);

    for (const auto& category : categories) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = category.get_name();
        button->callbackData = "category:" + std::to_string(category.get_id());
        keyboard->inlineKeyboard.push_back({button});
    }

    auto backButton = std::make_shared<TgBot::InlineKeyboardButton>();
    backButton->text = "<< Назад к кофейням";
    backButton->callbackData = "back:cafes:0";
    keyboard->inlineKeyboard.push_back({backButton});

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildItemKeyboard(int cafeId, int categoryId) const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    const auto items = itemsByCategory(categoryId);

    for (const auto& item : items) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = item.get_name() + " - " + std::to_string(static_cast<int>(item.get_price())) + " руб";
        button->callbackData = "add:" + std::to_string(item.get_id());
        keyboard->inlineKeyboard.push_back({button});
    }

    auto backButton = std::make_shared<TgBot::InlineKeyboardButton>();
    backButton->text = "<< Назад к категориям";
    backButton->callbackData = "back:cat:0";
    keyboard->inlineKeyboard.push_back({backButton});

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCartKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto buttonCheckout = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonCheckout->text = "Оформить заказ";
    buttonCheckout->callbackData = "checkout";

    auto buttonClear = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonClear->text = "Очистить";
    buttonClear->callbackData = "cart_clear";

    auto buttonMenu = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonMenu->text = "В меню";
    buttonMenu->callbackData = "back:cafes:0";

    keyboard->inlineKeyboard.push_back({buttonCheckout, buttonClear});
    keyboard->inlineKeyboard.push_back({buttonMenu});

    return keyboard;
}

void BotController::answerMessage(std::int64_t chatId, const std::string& text,
                                  const TgBot::GenericReply::Ptr& markup) const {
    try {
        bot_->getApi().sendMessage(chatId, text, nullptr, nullptr, markup);
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error: " << e.what() << std::endl;
    }
}

std::vector<std::string> BotController::split(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}