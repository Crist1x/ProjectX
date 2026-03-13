#include "algorithm/alg.h"
#include <algorithm>
#include <iostream>


Barista::Barista()
    : id(0), isWorking(false), busyUntilTime(0), totalOrdersCompleted(0) {}

Barista::Barista(int baristaId, bool working)
    : id(baristaId), isWorking(working), busyUntilTime(0), totalOrdersCompleted(0) {}

int Barista::getId() const { return id; }
bool Barista::getIsWorking() const { return isWorking; }
const std::deque<Order>& Barista::getOrderQueue() const { return orderQueue; }
double Barista::getBusyUntilTime() const { return busyUntilTime; }
int Barista::getTotalOrdersCompleted() const { return totalOrdersCompleted; }
size_t Barista::getQueueSize() const { return orderQueue.size(); }

void Barista::setIsWorking(bool working) { isWorking = working; }
void Barista::setBusyUntilTime(double time) { busyUntilTime = time; }
void Barista::incrementOrdersCompleted() { totalOrdersCompleted++; }

void Barista::addOrderToQueue(const Order& order) {
    orderQueue.push_back(order);
}

Order Barista::getNextOrder() {
    if (!orderQueue.empty()) {
        Order order = orderQueue.front();
        orderQueue.pop_front();
        return order;
    }
    return Order();
}

bool Barista::hasOrders() const {
    return !orderQueue.empty();
}

void Barista::clearQueue() {
    orderQueue.clear();
}

alg::alg()
    : currentTime(0), workingDayStartTime(0) {}

void alg::initializeBaristas(int totalBaristas) {
    baristas.clear();
    baristas.reserve(totalBaristas);

    for (int i = 0; i < totalBaristas; ++i) {
        baristas.emplace_back(i, false);
    }
}

void alg::setBaristaStatus(int baristaId, bool isWorking) {
    if (baristaId >= 0 && baristaId < static_cast<int>(baristas.size())) {
        baristas[baristaId].setIsWorking(isWorking);
        std::cout << "Barista " << baristaId << " status: "
                  << (isWorking ? "in work" : "not work") << std::endl;
    }
}

void alg::setWorkingDayStart(double startTimeInMinutes) {
    workingDayStartTime = startTimeInMinutes;
    currentTime = startTimeInMinutes;

    for (auto& barista : baristas) {
        barista.setBusyUntilTime(startTimeInMinutes);
    }
}

void alg::addOrderToCommonQueue(const Order& order) {
    commonQueue.push_back(order);
    std::cout << "Order " << order.get_id() << " added to common queue"
              << "Queue size: " << commonQueue.size() << std::endl;
}

void alg::distributeOrders() {
    std::cout << "\n Distributing orders" << std::endl;
    std::cout << "Orders in queue: " << commonQueue.size() << std::endl;
    std::cout << "Working baristas: " << getWorkingBaristasCount() << std::endl;

    while (!commonQueue.empty()) {
        Order order = commonQueue.front();
        commonQueue.pop_front();

        order.set_total_preparation_time(calculateOrderPreparationTime(order));

        int bestBaristaId = findLeastLoadedBarista();

        if (bestBaristaId != -1) {
            assignOrderToBarista(order, bestBaristaId);
        } else {
            std::cerr << "no available" << std::endl;
            commonQueue.push_front(order);
            break;
        }
    }

    std::cout << "complete\n" << std::endl;
}

void alg::assignOrderToBarista(Order& order, int baristaId) {
    if (baristaId < 0 || baristaId >= static_cast<int>(baristas.size())) {
        std::cerr << "Invalid barista ID: " << baristaId << std::endl;
        return;
    }

    Barista& barista = baristas[baristaId];

    if (!barista.getIsWorking()) {
        std::cerr << "Barista " << baristaId << " is not working!" << std::endl;
        return;
    }

    double startTime = std::max(currentTime, barista.getBusyUntilTime());
    order.set_estimated_ready_time(startTime + order.get_total_preparation_time());
    order.set_assigned_barista_id(baristaId);

    barista.addOrderToQueue(order);
    barista.setBusyUntilTime(order.get_estimated_ready_time());

    std::cout << "Order " << order.get_id() << " assigned to Barista " << baristaId
              << ". Ready at: " << order.get_estimated_ready_time() << " min"
              << " (prep time: " << order.get_total_preparation_time() << " min)" << std::endl;
}

double alg::calculateItemPreparationTime(const Item& item) {
    return static_cast<double>(item.get_preparation_time());
}

double alg::calculateOrderPreparationTime(const Order& order) {
    double totalTime = 0.0;


    for (const auto& item_ptr : order.get_items()) {
        totalTime += calculateItemPreparationTime(*item_ptr);
    }



    if (order.get_items().size() > 3) {
        totalTime *= 1.2;
    }

    return totalTime;
}

int alg::findLeastLoadedBarista() {
    int bestBaristaId = -1;
    double minBusyTime = std::numeric_limits<double>::max();

    for (const auto& barista : baristas) {
        if (barista.getIsWorking()) {
            if (barista.getBusyUntilTime() < minBusyTime) {
                minBusyTime = barista.getBusyUntilTime();
                bestBaristaId = barista.getId();
            }
        }
    }

    return bestBaristaId;
}

int alg::findBaristaWithShortestQueue() {
    int bestBaristaId = -1;
    size_t minQueueSize = std::numeric_limits<size_t>::max();

    for (const auto& barista : baristas) {
        if (barista.getIsWorking()) {
            if (barista.getQueueSize() < minQueueSize) {
                minQueueSize = barista.getQueueSize();
                bestBaristaId = barista.getId();
            }
        }
    }

    return bestBaristaId;
}

double alg::getTotalProcessingTime() const {
    double maxTime = currentTime;

    for (const auto& barista : baristas) {
        if (barista.getIsWorking() && barista.getBusyUntilTime() > maxTime) {
            maxTime = barista.getBusyUntilTime();
        }
    }

    return maxTime - currentTime;
}

double alg::getAverageWaitTime() const {
    double totalWaitTime = 0.0;
    int orderCount = 0;

    for (const auto& barista : baristas) {
        for (const auto& order : barista.getOrderQueue()) {
            double waitTime = order.get_estimated_ready_time() - currentTime;
            totalWaitTime += waitTime;
            orderCount++;
        }
    }

    return orderCount > 0 ? totalWaitTime / orderCount : 0.0;
}

const std::vector<Barista>& alg::getBaristas() const {
    return baristas;
}

const Barista& alg::getBarista(int baristaId) const {
    static Barista empty;
    if (baristaId >= 0 && baristaId < static_cast<int>(baristas.size())) {
        return baristas[baristaId];
    }
    return empty;
}

size_t alg::getCommonQueueSize() const {
    return commonQueue.size();
}

int alg::getWorkingBaristasCount() const {
    int count = 0;
    for (const auto& barista : baristas) {
        if (barista.getIsWorking()) count++;
    }
    return count;
}

void alg::reset() {
    currentTime = workingDayStartTime;
    commonQueue.clear();

    for (auto& barista : baristas) {
        barista.clearQueue();
        barista.setBusyUntilTime(workingDayStartTime);
    }

    std::cout << "Algorithm reset complete" << std::endl;
}

void alg::printStatistics() const {
    std::cout << "\nSTATISTICS" << std::endl;
    std::cout << "Working baristas: " << getWorkingBaristasCount() << std::endl;
    std::cout << "Orders in common queue: " << commonQueue.size() << std::endl;
    std::cout << "Total processing time: " << getTotalProcessingTime() << " min" << std::endl;
    std::cout << "Average wait time: " << getAverageWaitTime() << " min" << std::endl;

    std::cout << "\nBarista Details" << std::endl;
    for (const auto& barista : baristas) {
        std::cout << "Barista " << barista.getId() << ": "
                  << (barista.getIsWorking() ? "in work" : "no working")
                  << " | Queue: " << barista.getQueueSize() << " orders"
                  << " | Busy until: " << barista.getBusyUntilTime() << " min"
                  << std::endl;
    }
    std::cout << "\n" << std::endl;
}
