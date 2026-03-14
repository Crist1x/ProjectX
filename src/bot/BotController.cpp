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

    try {
        cachedCafes_ = fetchCafes();
    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL ERROR] Ошибка при загрузке кофеен: " << e.what() << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(algMutex_);

        for (const auto& cafe : cachedCafes_) {
            alg localAlg;
            localAlg.initializeBaristas(2);
            localAlg.setBaristaStatus(0, true);
            localAlg.setBaristaStatus(1, true);
            localAlg.setWorkingDayStart(0.0);

            cafeAlgorithms_[cafe.id] = localAlg;
        }
    }

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

    // УБРАЛИ std::thread
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
    else if (data == "checkout") {
        handleCheckout(query);
    }
    else if (data == "time_menu") {
        auto chatId = query->message->chat->id;
        bot_->getApi().answerCallbackQuery(query->id);
        editMessage(chatId, query->message->messageId, "🕒 Когда вы хотите забрать заказ?", buildTimeSelectionKeyboard());
    }
    else if (data.rfind("schedule:", 0) == 0) {
        handleScheduledOrder(query);
    }

    else if (data.rfind("back:cart:", 0) == 0) {
        auto chatId = query->message->chat->id;
        bot_->getApi().answerCallbackQuery(query->id);
        editMessage(chatId, query->message->messageId, buildCartText(chatId), buildCartKeyboard());

    }

    else if (data == "view_cart") {
        auto chatId = query->message->chat->id;
        bot_->getApi().answerCallbackQuery(query->id);
        editMessage(chatId, query->message->messageId, buildCartText(chatId), buildCartKeyboard());
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
        editMessage(chatId, query->message->messageId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
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
        editMessage(chatId, query->message->messageId, buildCategoryItemsText(cafeId, categoryId), buildItemKeyboard(cafeId, categoryId));
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

        // товар из БД
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
    } catch (...) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка");
    }
}

void BotController::handleCheckout(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    std::vector<std::pair<int, int>> itemsToOrder;
    int currentCafeId = -1;

    {
        std::lock_guard<std::mutex> lock(botMutex_);
        auto& cart = userCarts_[chatId];

        if (cart.isEmpty()) {
            bot_->getApi().answerCallbackQuery(query->id, "Корзина пуста");
            return;
        }
        itemsToOrder = cart.getItemPairs();

        auto it = userSelectedCafe_.find(chatId);
        if (it != userSelectedCafe_.end()) {
            currentCafeId = it->second;
        }
    }

    if (currentCafeId == -1) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: Кофейня не выбрана");
        return;
    }

    auto telegramId = query->from->id;
    auto user = userRepository_.findByTelegramId(telegramId);
    if (!user.has_value()) {
        bot_->getApi().answerCallbackQuery(query->id, "Пользователь не найден");
        return;
    }

    try {
        bool success = orderRepository_.createOrderWithItems(user->id, itemsToOrder);

        if (success) {
            Order algOrder;
            algOrder.set_id(user->id + std::rand() % 1000);
            algOrder.set_user_id(user->id);

            for (const auto& [itemId, qty] : itemsToOrder) {
                auto itemOpt = menuRepository_.findItemById(itemId);
                if (itemOpt.has_value()) {
                    for (int i = 0; i < qty; ++i) {
                        algOrder.add_item(std::make_shared<Item>(itemOpt.value()));
                    }
                }
            }

            double waitTimeMinutes = 0.0;
            int assignedBarista = -1;

            {
                std::lock_guard<std::mutex> algLock(algMutex_);
                auto& activeAlg = cafeAlgorithms_[currentCafeId];

                activeAlg.addOrderToCommonQueue(algOrder);
                activeAlg.distributeOrders();

                for (const auto& barista : activeAlg.getBaristas()) {
                    for (const auto& bOrder : barista.getOrderQueue()) {
                        if (bOrder.get_id() == algOrder.get_id()) {
                            waitTimeMinutes = bOrder.get_estimated_ready_time() - activeAlg.getCurrentTime();
                            assignedBarista = barista.getId();
                        }
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(botMutex_);
                userCarts_[chatId].clear();
            }

            bot_->getApi().answerCallbackQuery(query->id, "Заказ принят в работу!");

            std::ostringstream text;
            text << "🎉 <b>Заказ успешно оформлен!</b>\n\n";

            if (assignedBarista != -1) {
                text << "👨‍🍳 Его готовит бариста №" << assignedBarista << ".\n";
                int minutes = std::max(1, static_cast<int>(waitTimeMinutes));
                text << "⏳ Примерное время ожидания: <b>" << minutes << " мин.</b>";
            } else {
                text << "⏳ Ваш заказ добавлен в очередь. Ожидайте готовности!";
            }

            // Заменяем корзину
            editMessage(chatId, query->message->messageId, text.str());

            if (waitTimeMinutes > 0) {

                int waitSeconds = static_cast<int>(waitTimeMinutes * 60);

                auto botPtr = bot_; 

                std::thread([botPtr, chatId, waitSeconds, assignedBarista]() {
                    std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
                    try {
                        std::string readyText = "🔔 <b>Дзинь!</b>\n\nВаш заказ готов!\nБариста №"
                                              + std::to_string(assignedBarista)
                                              + " уже ждет вас на выдаче.";

                        botPtr->getApi().sendMessage(chatId, readyText, 0, 0, nullptr, "HTML");
                    } catch (const std::exception& e) {
                        std::cerr << "[Thread Error]: " << e.what() << std::endl;
                    }
                }).detach();
            }

        } else {
            bot_->getApi().answerCallbackQuery(query->id, "Ошибка оформления в БД");
        }
    } catch (const std::exception& e) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: " + std::string(e.what()));
        std::cerr << "[Bot] Checkout Error: " << e.what() << std::endl;
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
    editMessage(chatId, query->message->messageId, buildCartText(chatId), buildCartKeyboard());
}

void BotController::handleBackToCategories(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    auto cafeIt = userSelectedCafe_.find(chatId);

    if (cafeIt != userSelectedCafe_.end()) {
        int cafeId = cafeIt->second;
        answerMessage(chatId, buildCafeMenuText(cafeId), buildCategoryKeyboard(cafeId));
    } else {
        editMessage(chatId, query->message->messageId, "Выберите кофейню:", buildCafeKeyboard());
    }
}

void BotController::handleBackToCafes(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    editMessage(chatId, query->message->messageId, "Выберите кофейню:", buildCafeKeyboard());
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

    auto cartButton = std::make_shared<TgBot::InlineKeyboardButton>();
    cartButton->text = "🛒 Перейти в корзину";
    cartButton->callbackData = "view_cart";
    keyboard->inlineKeyboard.push_back({cartButton});

    auto backButton = std::make_shared<TgBot::InlineKeyboardButton>();
    backButton->text = "<< Назад к категориям";
    backButton->callbackData = "back:cat:0";
    keyboard->inlineKeyboard.push_back({backButton});

    return keyboard;
}

TgBot::InlineKeyboardMarkup::Ptr BotController::buildCartKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto buttonCheckout = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonCheckout->text = "🚀 Приготовить сейчас";
    buttonCheckout->callbackData = "checkout";

    auto buttonTime = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonTime->text = "🕒 Заказать ко времени";
    buttonTime->callbackData = "time_menu";

    auto buttonClear = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonClear->text = "🗑 Очистить";
    buttonClear->callbackData = "cart_clear";

    auto buttonMenu = std::make_shared<TgBot::InlineKeyboardButton>();
    buttonMenu->text = "📖 В меню";
    buttonMenu->callbackData = "back:cafes:0";

    keyboard->inlineKeyboard.push_back({buttonCheckout});
    keyboard->inlineKeyboard.push_back({buttonTime}); // кнопка времени
    keyboard->inlineKeyboard.push_back({buttonClear, buttonMenu});

    return keyboard;
}

void BotController::answerMessage(std::int64_t chatId, const std::string& text,
                                  const TgBot::GenericReply::Ptr& markup) const {
    try {
        bot_->getApi().sendMessage(chatId, text, 0, 0, markup, "HTML");
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Error: " << e.what() << std::endl;
    }
}

void BotController::editMessage(std::int64_t chatId, std::int32_t messageId, const std::string& text,
                                const TgBot::GenericReply::Ptr& markup) const {
    try {
        auto inlineMarkup = std::dynamic_pointer_cast<TgBot::InlineKeyboardMarkup>(markup);
        bot_->getApi().editMessageText(text, chatId, messageId, "", "HTML", 0, inlineMarkup);
    } catch (const std::exception& e) {
        std::cerr << "[Bot] Edit Error: " << e.what() << std::endl;
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

TgBot::InlineKeyboardMarkup::Ptr BotController::buildTimeSelectionKeyboard() const {
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    std::vector<int> times = {15, 30, 45, 60, 90, 120}; // Минуты

    for (size_t i = 0; i < times.size(); i += 2) {
        auto btn1 = std::make_shared<TgBot::InlineKeyboardButton>();
        btn1->text = "Через " + std::to_string(times[i]) + " мин";
        btn1->callbackData = "schedule:" + std::to_string(times[i]);

        auto btn2 = std::make_shared<TgBot::InlineKeyboardButton>();
        btn2->text = "Через " + std::to_string(times[i+1]) + " мин";
        btn2->callbackData = "schedule:" + std::to_string(times[i+1]);

        keyboard->inlineKeyboard.push_back({btn1, btn2});
    }

    auto backBtn = std::make_shared<TgBot::InlineKeyboardButton>();
    backBtn->text = "<< Назад в корзину";
    backBtn->callbackData = "back:cart:0";
    keyboard->inlineKeyboard.push_back({backBtn});

    return keyboard;
}

void BotController::handleScheduledOrder(const TgBot::CallbackQuery::Ptr& query) {
    auto chatId = query->message->chat->id;
    auto telegramId = query->from->id;

    //  через сколько минут нужен кофе
    auto parts = split(query->data, ':');
    if (parts.size() < 2) return;
    int targetDelayMinutes = std::stoi(parts[1]);

    std::vector<std::pair<int, int>> itemsToOrder;
    int currentCafeId = -1;

    {
        std::lock_guard<std::mutex> lock(botMutex_);
        auto& cart = userCarts_[chatId];
        if (cart.isEmpty()) {
            bot_->getApi().answerCallbackQuery(query->id, "Корзина пуста!");
            return;
        }
        itemsToOrder = cart.getItemPairs();
        cart.clear();

        // ID кофейни
        auto it = userSelectedCafe_.find(chatId);
        if (it != userSelectedCafe_.end()) {
            currentCafeId = it->second;
        }
    }

    if (currentCafeId == -1) {
        bot_->getApi().answerCallbackQuery(query->id, "Ошибка: Кофейня не выбрана");
        return;
    }

    bot_->getApi().answerCallbackQuery(query->id, "Планирую заказ...");

    // на готовку и очередь 10 мин
    int prepBuffer = 10;
    int sleepTimeMinutes = targetDelayMinutes - prepBuffer;

    if (sleepTimeMinutes <= 0) {
        sleepTimeMinutes = 1;
    }

    std::ostringstream text;
    text << "✅ <b>Заказ запланирован!</b>\n\n";
    text << "Вы выбрали забрать заказ через " << targetDelayMinutes << " мин.\n";
    text << "Мы начнем готовить его через " << sleepTimeMinutes << " мин, чтобы он был горячим к вашему приходу!";
    answerMessage(chatId, text.str());

    auto botPtr = bot_;

    std::thread([this, telegramId, chatId, itemsToOrder, sleepTimeMinutes, currentCafeId]() {
        std::this_thread::sleep_for(std::chrono::minutes(sleepTimeMinutes));

        try {
            Order algOrder;

            {
                std::lock_guard<std::mutex> dbLock(botMutex_); 

                auto user = userRepository_.findByTelegramId(telegramId);
                if (!user.has_value()) return;

                bool success = orderRepository_.createOrderWithItems(user->id, itemsToOrder);
                if (!success) return;

                algOrder.set_id(user->id + std::rand() % 1000);
                algOrder.set_user_id(user->id);

                for (const auto& [itemId, qty] : itemsToOrder) {
                    auto itemOpt = menuRepository_.findItemById(itemId);
                    if (itemOpt.has_value()) {
                        for (int i = 0; i < qty; ++i) {
                            algOrder.add_item(std::make_shared<Item>(itemOpt.value()));
                        }
                    }
                }
            } // снятие блока

            int assignedBarista = -1;
            {
                std::lock_guard<std::mutex> algLock(algMutex_);
                auto& activeAlg = cafeAlgorithms_[currentCafeId];
                activeAlg.addOrderToCommonQueue(algOrder);
                activeAlg.distributeOrders();

                for (const auto& barista : activeAlg.getBaristas()) {
                    for (const auto& bOrder : barista.getOrderQueue()) {
                        if (bOrder.get_id() == algOrder.get_id()) {
                            assignedBarista = barista.getId();
                        }
                    }
                }
            }

            std::string alertText = "👨‍🍳 <b>Пора готовить!</b>\nБариста №" + std::to_string(assignedBarista) +
                                    " только что взялся за ваш заказ, который вы планировали.";
            answerMessage(chatId, alertText);

        } catch (const std::exception& e) {
            std::cerr << "[Bot] Ошибка в отложенном заказе: " << e.what() << std::endl;
        }

    }).detach();
}
