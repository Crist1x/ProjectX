
#ifndef PROJECTX_ORDER_H
#define PROJECTX_ORDER_H

#include "menu/MenuItem.h"
#include <vector>
#include <memory>

enum class OrderStatus {
    PENDING,     // В очереди
    COOKING,     // Готовится баристой
    READY,       // Готов к выдаче
    COMPLETED    // Выдан клиенту
};

struct Order {
    int id{};
    long long telegram_user_id{};
    
    std::vector<std::shared_ptr<Item>> items;
    
    double totalPreparationTime{0.0};
    double estimatedReadyTime{0.0};
    int assignedBaristaId{-1};
    
    OrderStatus status{OrderStatus::PENDING};
};

#endif // PROJECTX_ORDER_H

