#include "bot/BotController.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <thread>
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

    {std::lock_guard<std::mutex> lock(algMutex_);
        orderAlgorithm_.initializeBaristas(2); // Пусть у нас работают 2 баристы
        orderAlgorithm_.setBaristaStatus(0, true); // Выводим первого на смену
        orderAlgorithm_.setBaristaStatus(1, true); // Выводим второго на смену
        orderAlgorithm_.setWorkingDayStart(0.0);   // Время старта (в минутах от начала смены)
    }

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

    // Оборачиваем каждую команду в std::thread
    bot_->getEvents().onCommand("start", [this](const TgBot::Message::Ptr& message) {
        std::thread([this, message]() { handleStart(message); }).detach();
    });

    bot_->getEvents().onCommand("menu", [this](const TgBot::Message::Ptr& message) {
        std::thread([this, message]() { handleMenu(message); }).detach();
    });

    bot_->getEvents().onCommand("help", [this](const TgBot::Message::Ptr& message) {
        std::thread([this, message]() { handleHelp(message); }).detach();
    });

    bot_->getEvents().onCommand("cart", [this](const TgBot::Message::Ptr& message) {
        std::thread([this, message]() { handleCart(message); }).detach();
    });

    bot_->getEvents().onCommand("profile", [this](const TgBot::Message::Ptr& message) {
        std::thread([this, message]() { handleProfile(message); }).detach();
    });

    bot_->getEvents().onCallbackQuery([this](const TgBot::CallbackQuery::Ptr& query) {
        std::thread([this, query]() { handleCallback(query); }).detach();
    });

    std::cout << "[Bot] Handlers registered!" << std::endl;
}

void BotController::handleStart(const TgBot::Message::Ptr& message) {
    try {
        auto user = ensureUserExists(message);

        if (!user.has_value()) {
            std::cerr << "[ERROR] База данных вернула nullopt для пользователя!" << std::endl;
            answerMessage(message->chat->id, "⚠️ Ошибка базы данных. Не удалось загрузить профиль. Напишите админу.");
            return;
        }

        std::string text = "Привет, " + user->username + "!\n\n";
        text += "Добро пожаловать в CoffeeHSE!\n\n";
        text += "Выберите кофейню ниже:";

        answerMessage(message->chat->id, text, buildCafeKeyboard());

    } catch (const std::exception& e) {
        std::cerr << "[Bot] Критическая ошибка в handleStart: " << e.what() << std::endl;
    }
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
    if (parts.size() < 2) return;

    try {
        int cafeId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;

        {
            std::lock_guard<std::mutex> lock(botMutex_);
            userSelectedCafe_[chatId] = cafeId;
        }

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю...");
        answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleCategorySelection(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) return;

    try {
        int categoryId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;
        int cafeId = -1;

        {
            std::lock_guard<std::mutex> lock(botMutex_);
            userSelectedCategory_[chatId] = categoryId;
            auto cafeIt = userSelectedCafe_.find(chatId);
            if (cafeIt != userSelectedCafe_.end()) {
                cafeId = cafeIt->second;
            }
        }

        bot_->getApi().answerCallbackQuery(query->id, "Загружаю...");
        answerMessage(chatId, buildCategoryItemsText(cafeId, categoryId), buildItemKeyboard(cafeId, categoryId));
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleAddToCart(const TgBot::CallbackQuery::Ptr& query) {
    auto parts = split(query->data, ':');
    if (parts.size() < 2) return;

    try {
        int menuItemId = std::stoi(parts[1]);
        auto chatId = query->message->chat->id;

        // Сначала достаем товар из БД (это долго, делаем БЕЗ блокировки)
        auto itemOpt = menuRepository_.findItemById(menuItemId);
        if (!itemOpt.has_value()) {
            bot_->getApi().answerCallbackQuery(query->id, "Товар не найден!");
            return;
        }

        std::string itemName = itemOpt->get_name();
        double itemPrice = itemOpt->get_price();


        {
            std::lock_guard<std::mutex> lock(botMutex_);
            userCarts_[chatId].addItem(menuItemId, itemName, itemPrice, 1);
        }

        bot_->getApi().answerCallbackQuery(query->id, itemName + " добавлен!");
        answerMessage(chatId, buildCartText(chatId), buildCartKeyboard());
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleCheckout(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    std::vector<std::pair<int, int>> itemsToOrder;

    // 1. БЕЗОПАСНО ДОСТАЕМ КОРЗИНУ
    {
        std::lock_guard<std::mutex> lock(botMutex_);
        auto& cart = userCarts_[chatId];

        if (cart.isEmpty()) {
            bot_->getApi().answerCallbackQuery(query->id, "Корзина пуста");
            return;
        }
        itemsToOrder = cart.getItemPairs();
    }

    auto telegramId = query->from->id;
    auto user = userRepository_.findByTelegramId(telegramId);
    if (!user.has_value()) {
        bot_->getApi().answerCallbackQuery(query->id, "Пользователь не найден");
        return;
    }

    try {
        // 2. ОФОРМЛЯЕМ ЗАКАЗ В БАЗЕ ДАННЫХ
        bool success = orderRepository_.createOrderWithItems(user->id, itemsToOrder);

        if (success) {
            // 3. СОЗДАЕМ ЗАКАЗ
            Order algOrder;
            // Устанавливаем ID
            algOrder.set_id(user->id + std::rand() % 1000);
            algOrder.set_user_id(user->id);

            // Наполняем заказ товарами
            for (const auto& [itemId, qty] : itemsToOrder) {
                auto itemOpt = menuRepository_.findItemById(itemId);
                if (itemOpt.has_value()) {
                    for (int i = 0; i < qty; ++i) {
                        algOrder.add_item(std::make_shared<Item>(itemOpt.value()));
                    }
                }
            }

            // 4. ЗАПУСКАЕМ АЛГОРИТМ БАРИСТ
            double waitTimeMinutes = 0.0;
            int assignedBarista = -1;

            {
                std::lock_guard<std::mutex> algLock(algMutex_);

                orderAlgorithm_.addOrderToCommonQueue(algOrder);
                orderAlgorithm_.distributeOrders();

                // Ищем наш заказ, используя геттеры
                for (const auto& barista : orderAlgorithm_.getBaristas()) {
                    for (const auto& bOrder : barista.getOrderQueue()) {
                        if (bOrder.get_id() == algOrder.get_id()) {
                            waitTimeMinutes = bOrder.get_estimated_ready_time() - orderAlgorithm_.getTotalProcessingTime();
                            assignedBarista = barista.getId(); // Метод getId() у Barista
                        }
                    }
                }
            }

            // 5. ОЧИЩАЕМ КОРЗИНУ
            {
                std::lock_guard<std::mutex> lock(botMutex_);
                userCarts_[chatId].clear();
            }

            // 6. ОТПРАВЛЯЕМ КРАСИВЫЙ ОТВЕТ
            bot_->getApi().answerCallbackQuery(query->id, "Заказ принят в работу!");

            std::ostringstream text;
            text << "🎉 *Заказ успешно оформлен!*\n\n";

            if (assignedBarista != -1) {
                text << "👨‍🍳 Его готовит бариста №" << assignedBarista << ".\n";
                // Округляем время ожидания, чтобы не было "1.453 минут"
                int minutes = std::max(1, static_cast<int>(waitTimeMinutes));
                text << "⏳ Примерное время ожидания: *" << minutes << " мин.*";
            } else {
                text << "⏳ Ваш заказ добавлен в очередь. Ожидайте готовности!";
            }

            answerMessage(chatId, text.str());

        } else {
            bot_->getApi().answerCallbackQuery(query->id, "Ошибка оформления в БД");
        }
    } catch (const std::exception& e) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: " + std::string(e.what()));
    }
}

void BotController::handleClearCart(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;

    {
        // очищаем корзину с мьютексом
        std::lock_guard<std::mutex> lock(botMutex_);
        userCarts_[chatId].clear();
    }

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
    Cart cartCopy;

    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(botMutex_));
        auto it = userCarts_.find(chatId);
        if (it == userCarts_.end() || it->second.isEmpty()) {
            return "🛒 Корзина пуста\n\nДобавь товары из меню!";
        }
        cartCopy = it->second;
    }

    std::ostringstream text;
    text << "🛒 Корзина:\n\n";

    for (const auto& item : cartCopy.getItems()) {
        text << "- " << item.name << " x" << item.quantity
             << " = " << static_cast<int>(item.price * item.quantity) << " руб\n";
    }

    text << "\n----------------\n";
    text << "💰 Итого: " << static_cast<int>(cartCopy.getTotalAmount()) << " руб\n";
    text << "📦 Товаров: " << cartCopy.getItemCount();

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