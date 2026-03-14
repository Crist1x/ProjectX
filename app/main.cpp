#include "bot/BotController.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Простая функция для чтения токена из файла .env
std::string readTokenFromEnvFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(
            "File .env not found in project root.\n"
            "Create .env with: PROJECTX_BOT_TOKEN=your_token_here"
        );
    }

    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) {
            continue;
        }

        // Пропускаем комментарии (строки начинающиеся с #)
        if (line[0] == '#') {
            continue;
        }

        // Ищем строку PROJECTX_BOT_TOKEN=
        const std::string prefix = "PROJECTX_BOT_TOKEN=";
        if (line.size() > prefix.size() && line.substr(0, prefix.size()) == prefix) {
            // Возвращаем всё после знака =
            return line.substr(prefix.size());
        }
    }

    throw std::runtime_error(
        "PROJECTX_BOT_TOKEN not found in .env file.\n"
        "Add line: PROJECTX_BOT_TOKEN=your_token_here"
    );
}

int main() {
    try {
        std::cout << "   CoffeeHSE Bot - University Coffee" << std::endl;
        std::cout << std::endl;

        // Читаем токен из файла .env (в корне проекта)
        const std::filesystem::path projectRoot = PROJECTX_ROOT;
        const std::filesystem::path envPath = projectRoot / ".env";

        std::cout << "[1/5] Loading token from .env..." << std::endl;

        std::string token = readTokenFromEnvFile(envPath);

        // Отладка токена
        std::cout << "       Bot token loaded" << std::endl;
        std::cout << "      Token length: " << token.length() << std::endl;
        std::cout << std::endl;

        // Database path
        const std::filesystem::path databaseDirectory = projectRoot / "databases";
        const std::filesystem::path databasePath = databaseDirectory / "projectx.db";

        std::cout << "[2/5] Initializing database..." << std::endl;
        std::filesystem::create_directories(databaseDirectory);

        db::Database database(databasePath.string());
        database.initializeSchema();
        std::cout << "       Database initialized" << std::endl;

        std::cout << "[3/5] Creating repositories..." << std::endl;
        db::UserRepository userRepository(database);
        db::MenuRepository menuRepository(database);
        db::OrderRepository orderRepository(database);
        std::cout << "       Repositories created" << std::endl;

        std::cout << "[4/5] Starting bot controller..." << std::endl;
        BotController botController(token, database, userRepository, menuRepository, orderRepository);
        std::cout << "       Bot controller initialized" << std::endl;

        std::cout << "[5/5] Bot is running!" << std::endl;
        std::cout << std::endl;
        std::cout << "   Bot is online" << std::endl;
        std::cout << std::endl;
        std::cout << "Commands: /start /menu /cart /help" << std::endl;
        std::cout << std::endl;

        botController.run();

    } catch (const std::exception& e) {
        std::cerr << std::endl;
        std::cerr << "   ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}