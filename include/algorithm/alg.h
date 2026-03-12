#ifndef PROJECTX_ALG_H
#define PROJECTX_ALG_H

#include "menu/MenuItem.h"
#include <vector>
#include <deque>
#include <string>
#include <chrono>
#include <limits>
#include "menu/Order.h"




class Barista {
private:
    int id;
    bool isWorking;
    std::deque<Order> orderQueue;
    double busyUntilTime;
    int totalOrdersCompleted;

public:
    Barista();
    Barista(int baristaId, bool working = false);

    // Геттеры
    int getId() const;
    bool getIsWorking() const;
    const std::deque<Order>& getOrderQueue() const;
    double getBusyUntilTime() const;
    int getTotalOrdersCompleted() const;
    size_t getQueueSize() const;

    // Сеттеры
    void setIsWorking(bool working);
    void setBusyUntilTime(double time);
    void incrementOrdersCompleted();

    // Управление очередью
    void addOrderToQueue(const Order& order);
    Order getNextOrder();
    bool hasOrders() const;
    void clearQueue();
};

class alg {
private:
    std::vector<Barista> baristas;
    std::deque<Order> commonQueue;
    double currentTime;
    double workingDayStartTime;

    double calculateItemPreparationTime(const Item& item);
    int findLeastLoadedBarista();
    int findBaristaWithShortestQueue();

public:
    alg();

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
    const std::vector<Barista>& getBaristas() const;
    const Barista& getBarista(int baristaId) const;
    size_t getCommonQueueSize() const;
    int getWorkingBaristasCount() const;

    // Утилиты
    void reset();
    void printStatistics() const;
};

#endif