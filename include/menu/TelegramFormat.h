#ifndef PROJECTX_TELEGRAMFORMAT_H
#define PROJECTX_TELEGRAMFORMAT_H

#include <string>

class TelegramFormat {
public:
    virtual ~TelegramFormat() = default;
    
    // Чисто виртуальный метод, который обязаны реализовать наследники
    virtual std::string to_telegram_format() const = 0;
};

#endif //PROJECTX_TELEGRAMFORMAT_H