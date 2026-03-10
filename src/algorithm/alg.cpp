#include "menu/MenuItem.h"
#include <vector>
#include <deque>
#include <string>
#include <chrono>

struct Order {
    std::string id;
    std::vector<Item> items;
    double totalPreparationTime;  // общее время приготовления заказа
    double estimatedReadyTime;    // предполагаемое время готовности
    int assignedBaristaId;        // ID бариста, который готовит
    std::chrono::system_clock::time_point orderTime;  // время заказа

    Order() : totalPreparationTime(0), estimatedReadyTime(0), assignedBaristaId(-1) {}
};