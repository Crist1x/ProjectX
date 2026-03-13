#include "bot/BotController.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

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
    std::cout << "[BotController] Constructor called, token length: " << token.length() << std::endl;
    registerHandlers();
    std::cout << "[BotController] Handlers registered" << std::endl;
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
        std::cout << "[Bot] >>> /start received from chat " << message->chat->id << std::endl;
        handleStart(message);
    });

    bot_->getEvents().onCommand("menu", [this](const TgBot::Message::Ptr& message) {
        std::cout << "[Bot] >>> /menu received from chat " << message->chat->id << std::endl;
        handleMenu(message);
    });

    bot_->getEvents().onCommand("help", [this](const TgBot::Message::Ptr& message) {
        std::cout << "[Bot] >>> /help received from chat " << message->chat->id << std::endl;
        handleHelp(message);
    });

    bot_->getEvents().onCommand("cart", [this](const TgBot::Message::Ptr& message) {
        std::cout << "[Bot] >>> /cart received from chat " << message->chat->id << std::endl;
        handleCart(message);
    });

    bot_->getEvents().onCallbackQuery([this](const TgBot::CallbackQuery::Ptr& query) {
        std::cout << "[Bot] >>> Callback received: " << query->data << std::endl;
        handleCallback(query);
    });

    std::cout << "[Bot] All handlers registered successfully" << std::endl;
}

void BotController::handleStart(const TgBot::Message::Ptr& message) {
    std::cout << "[Bot] handleStart called for chat " << message->chat->id << std::endl;
    ensureUserExists(message);

    std::string text = "☕ *Привет! Я бот университетской кофейни.*\n\n"
                       "Помогу выбрать кофейню и сделать заказ.\n\n"
                       "*Команды:*\n"
                       "/menu - посмотреть меню\n"
                       "/cart - ваша корзина\n"
                       "/help - помощь";

    answerMessage(message->chat->id, text, buildCafeKeyboard());
    std::cout << "[Bot] handleStart completed" << std::endl;
}

void BotController::handleMenu(const TgBot::Message::Ptr& message) {
    std::cout << "[Bot] handleMenu called for chat " << message->chat->id << std::endl;
    ensureUserExists(message);
    answerMessage(message->chat->id, buildCafeSelectionText(), buildCafeKeyboard());
    std::cout << "[Bot] handleMenu completed" << std::endl;
}

void BotController::handleHelp(const TgBot::Message::Ptr& message) {
    std::cout << "[Bot] handleHelp called for chat " << message->chat->id << std::endl;

    std::string text = "📖 *Помощь*\n\n"
                       "*Команды:*\n"
                       "/start - начать работу\n"
                       "/menu - выбрать кофейню и посмотреть меню\n"
                       "/cart - посмотреть корзину\n"
                       "/help - показать эту справку\n\n"
                       "*Как сделать заказ:*\n"
                       "1. Нажмите /menu\n"
                       "2. Выберите кофейню\n"
                       "3. Выберите категорию\n"
                       "4. Нажмите на товар для добавления в корзину\n"
                       "5. Откройте /cart и нажмите *Оформить заказ*";

    answerMessage(message->chat->id, text);
    std::cout << "[Bot] handleHelp completed" << std::endl;
}

void BotController::handleCart(const TgBot::Message::Ptr& message) {
    std::cout << "[Bot] handleCart called for chat " << message->chat->id << std::endl;
    ensureUserExists(message);
    answerMessage(message->chat->id, buildCartText(message->chat->id), buildCartKeyboard());
    std::cout << "[Bot] handleCart completed" << std::endl;
}

void BotController::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    std::cout << "[Bot] handleCallback called with  " << query->data << std::endl;

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
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка формата");
        return;
    }

    try {
        int cafeId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;
        userSelectedCafe_[chatId] = cafeId;

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю меню");
        answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error in handleCafeSelection: " << e.what() << std::endl;
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: " + std::string(e.what()));
    }
}

void BotController::handleCategorySelection(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка формата");
        return;
    }

    try {
        int categoryId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;
        userSelectedCategory_[chatId] = categoryId;

        auto cafeIt = userSelectedCafe_.find(chatId);
        int cafeId = (cafeIt != userSelectedCafe_.end()) ? cafeIt->second : -1;

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю товары");
        answerMessage(chatId, buildCategoryItemsText(cafeId, categoryId), buildItemKeyboard(cafeId, categoryId));
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error in handleCategorySelection: " << e.what() << std::endl;
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: " + std::string(e.what()));
    }
}

void BotController::handleAddToCart(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка формата");
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

        bot_->getApi().answerCallbackQuery(query->id, "✓ Добавлено в корзину");
        answerMessage(chatId, buildCartText(chatId), buildCartKeyboard());
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error in handleAddToCart: " << e.what() << std::endl;
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка при добавлении");
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

            std::string text = "✅ *Заказ оформлен!*\n\n"
                               "Ожидайте подтверждения от бариста.\n"
                               "Вы можете продолжить делать заказы через /menu";
            answerMessage(chatId, text);
        } else {
            bot_->getApi().answerCallbackQuery(query->id, "Ошибка при оформлении");
        }
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error in handleCheckout: " << e.what() << std::endl;
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

    if (cafeIt == userSelectedCafe_.end()) {
        answerMessage(chatId, buildCafeSelectionText(), buildCafeKeyboard());
        return;
    }

    int cafeId = cafeIt->second;
    answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
}

void BotController::handleBackToCafes(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    answerMessage(chatId, buildCafeSelectionText(), buildCafeKeyboard());
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
    std::cout << "[Bot] New user registered: " << username << " (TG ID: " << telegramId << ")" << std::endl;
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

std::string BotController::buildCafeSelectionText() const {
    std::ostringstream text;
    text << "☕ *Выберите кофейню:*\n\n";

    for (const auto& cafe : fetchCafes()) {
        text << "• " << cafe.name << " (" << cafe.location << ")\n";
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

    std::ostringstream text;
    text << "📍 *" << selectedCafe->name << "*\n";
    text << "📌 " << selectedCafe->location << "\n\n";
    text << "*Категории:*\n";

    if (categories.empty()) {
        text << "\nПока нет доступных категорий.";
        return text.str();
    }

    for (const auto& category : categories) {
        text << "• " << category.get_name() << "\n";
    }

    text << "\nВыберите категорию ниже:";

    return text.str();
}

std::string BotController::buildCategoryItemsText(int cafeId, int categoryId) const {
    std::optional<CafeView> selectedCafe;
    for (const auto& cafe : fetchCafes()) {
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
    text << "📍 *" << selectedCafe->name << "*\n";
    text << "📋 *" << selectedCategory->get_name() << "*\n\n";

    if (items.empty()) {
        text << "Пока пусто в этой категории.\n\n";
        text << "Выберите другую категорию или вернитесь назад.";
        return text.str();
    }

    for (const auto& item : items) {
        text << "• *" << item.get_name() << "* — " << item.get_price() << "₽\n";
        if (!item.get_description().empty()) {
            text << "  _" << item.get_description() << "_\n";
        }
    }

    text << "\nНажмите на товар, чтобы добавить в корзину.";

    return text.str();
}

std::string BotController::buildCartText(std::int64_t chatId) const {
    auto it = userCarts_.find(chatId);

    if (it == userCarts_.end() || it->second.empty()) {
        return "🛒 *Ваша корзина пуста*\n\nДобавьте товары из меню!";
    }

    std::ostringstream text;
    text << "🛒 *Ваша корзина*\n\n";

    double total = 0.0;
    int itemCount = 0;

    for (const auto& [menuItemId, qty] : it->second) {
        auto itemOpt = menuRepository_.findItemById(menuItemId);
        if (itemOpt.has_value()) {
            double itemTotal = itemOpt->get_price() * qty;
            total += itemTotal;
            itemCount += qty;
            text << "• " << itemOpt->get_name() << " × " << qty
                 << " = " << itemTotal << "₽\n";
        } else {
            text << "• Товар #" << menuItemId << " × " << qty << "\n";
        }
    }

    text << "\n*Итого: " << total << "₽*\n";
    text << "Товаров: " << itemCount;

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
    backButton->text = "🔙 Назад к кофейням";
    backButton->callbackData = "back:cafes:0";
    keyboard->inlineKeyboard.push_back({backButton});

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildItemKeyboard(int cafeId, int categoryId) const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    const auto items = itemsByCategory(categoryId);

    for (const auto& item : items) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = "🛒 " + item.get_name() + " — " + std::to_string(static_cast<int>(item.get_price())) + "₽";
        button->callbackData = "add:" + std::to_string(item.get_id());
        keyboard->inlineKeyboard.push_back({button});
    }

    auto backButton = std::make_shared<TgBot::InlineKeyboardButton>();
    backButton->text = "🔙 Назад к категориям";
    backButton->callbackData = "back:cat:0";
    keyboard->inlineKeyboard.push_back({backButton});

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCartKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto buttonCheckout = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonCheckout->text = "✅ Оформить заказ";
    buttonCheckout->callbackData = "checkout";

    auto buttonClear = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonClear->text = "🗑 Очистить";
    buttonClear->callbackData = "cart_clear";

    auto buttonMenu = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonMenu->text = "📋 В меню";
    buttonMenu->callbackData = "back:cafes:0";

    keyboard->inlineKeyboard.push_back({buttonCheckout, buttonClear});
    keyboard->inlineKeyboard.push_back({buttonMenu});

    return keyboard;
}

void BotController::answerMessage(std::int64_t chatId, const std::string& text,
                                  const TgBot::GenericReply::Ptr& markup) const {
    try {
        std::cout << "[Bot] Sending message to chat " << chatId << std::endl;
        bot_->getApi().sendMessage(chatId, text, nullptr, nullptr, markup);
        std::cout << "[Bot] Message sent successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error sending message: " << e.what() << std::endl;
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