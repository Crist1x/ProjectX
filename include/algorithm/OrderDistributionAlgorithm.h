#ifndef PROJECTX_ORDERDISTRIBUTIONALGORITHM_H
#define PROJECTX_ORDERDISTRIBUTIONALGORITHM_H

#include "menu/MenuItem.h"
#include <vector>
#include <deque>
#include <string>
#include <chrono>
#include <map>

struct Order {
    std::string id;
    std::vector<Item> items;  // ← Используем Item, а не MenuItem!
    double totalPreparationTime;
    double estimatedReadyTime;
    int assignedBaristaId;
    std::chrono::system_clock::time_point orderTime;

    Order() : totalPreparationTime(0), estimatedReadyTime(0), assignedBaristaId(-1) {}
};

struct Barista {
    int id;
    bool isWorking;
    std::deque<Order> orderQueue;
    double busyUntilTime;
    int totalOrdersCompleted;

    Barista() : id(0), isWorking(false), busyUntilTime(0), totalOrdersCompleted(0) {}
};

class OrderDistributionAlgorithm {
private:
    std::vector<Barista> baristas;
    std::deque<Order> commonQueue;
    double currentTime;
    double workingDayStartTime;

    double calculateItemPreparationTime(const Item& item);
    int findLeastLoadedBarista();
    int findBaristaWithShortestQueue();

public:
    OrderDistributionAlgorithm();

    // Инициализация
    void initializeBaristas(int totalBaristas);
    void setBaristaStatus(int baristaId, bool isWorking);
    void setWorkingDayStart(double startTimeInMinutes);

    // Управление заказами
    void addOrderToCommonQueue(const Order& order);
    void distributeOrders();
    void assignOrderToBarista(Order& order, int baristaId);

    // Расчет времени
    double calculateOrderPreparationTime(const Order& order);
    double getTotalProcessingTime() const;
    double getAverageWaitTime() const;

    // Статистика
    std::vector<Barista> getBaristas() const;
    std::deque<Order> getBaristaQueue(int baristaId) const;
    size_t getCommonQueueSize() const;
    int getWorkingBaristasCount() const;

    // Утилиты
    void reset();
    void printStatistics() const;
};

#endif PROJECTX_ORDERDISTRIBUTIONALGORITHM_H