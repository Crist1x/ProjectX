#include "bot/BotController.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        const char* token = "8212901743:AAG18i5JT3bVXADIRJLQqAob0UA-18RL6jQ";
        if (token == nullptr || std::string(token).empty()) {
            throw std::runtime_error("Set PROJECTX_BOT_TOKEN before запуском бота");
        }

        const std::filesystem::path projectRoot = PROJECTX_ROOT;
        const std::filesystem::path databaseDirectory = projectRoot / "databases";
        const std::filesystem::path databasePath = databaseDirectory / "projectx.db";

        std::filesystem::create_directories(databaseDirectory);

        db::Database database(databasePath.string());
        database.initializeSchema();

        db::UserRepository userRepository(database);
        db::MenuRepository menuRepository(database);
        db::OrderRepository orderRepository(database);

        BotController botController(token, database, userRepository, menuRepository, orderRepository);
        botController.run();
    } catch (const std::exception& e) {
        std::cerr << "Bot startup failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}