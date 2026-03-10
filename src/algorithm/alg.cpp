#include "../menu/MenuItem.h"
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

struct Barista {
    int id;
    bool isWorking;
    std::deque<Order> orderQueue;  // очередь заказов
    double busyUntilTime;           // время, до которого бариста занят
    int totalOrdersCompleted;       // статистика

    Barista() : id(0), isWorking(false), busyUntilTime(0), totalOrdersCompleted(0) {}
};

class OrderDistributionAlgorithm {
private:
    std::vector<Barista> baristas;
    std::deque<Order> commonQueue;  // общая очередь заказов
    double currentTime;              // текущее время симуляции
    double workingDayStartTime;      // начало рабочего дня (в минутах от 00:00)

    // Расчет времени приготовления одного элемента меню
    double calculateItemPreparationTime(const Item& item);

    // Поиск бариста с наименьшей загрузкой
    int findLeastLoadedBarista();

    // Поиск бариста с самой короткой очередью
    int findBaristaWithShortestQueue();

public:
    OrderDistributionAlgorithm();

    // ========== ИНИЦИАЛИЗАЦИЯ ==========

    // Установка количества бариста
    void initializeBaristas(int totalBaristas);

    // Установка статуса работы для бариста
    void setBaristaStatus(int baristaId, bool isWorking);

    // Установка времени начала рабочего дня
    void setWorkingDayStart(double startTimeInMinutes);

    // ========== УПРАВЛЕНИЕ ЗАКАЗАМИ ==========

    // Добавление заказа в общую очередь
    void addOrderToCommonQueue(const Order& order);

    // Распределение заказов из общей очереди по бариста
    void distributeOrders();

    // Распределение одного заказа конкретному бариста
    void assignOrderToBarista(Order& order, int baristaId);

    // ========== РАСЧЕТ ВРЕМЕНИ ==========

    // Расчет времени приготовления заказа
    double calculateOrderPreparationTime(const Order& order);

    // Получение общего времени выполнения всех заказов
    double getTotalProcessingTime() const;

    // Получение среднего времени ожидания
    double getAverageWaitTime() const;

    // ========== СТАТИСТИКА ==========

    // Получение статистики по бариста
    std::vector<Barista> getBaristas() const;

    // Получение очереди конкретного бариста
    std::deque<Order> getBaristaQueue(int baristaId) const;

    // Получение размера общей очереди
    size_t getCommonQueueSize() const;

    // Количество работающих бариста
    int getWorkingBaristasCount() const;

    // ========== УТИЛИТЫ ==========

    // Сброс состояния алгоритма
    void reset();

    // Вывод статистики в консоль
    void printStatistics() const;
};
