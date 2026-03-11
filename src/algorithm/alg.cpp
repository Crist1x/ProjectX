#include "algorithm/OrderDistributionAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <limits>

OrderDistributionAlgorithm::OrderDistributionAlgorithm() 
    : currentTime(0), workingDayStartTime(0) {}

void OrderDistributionAlgorithm::initializeBaristas(int totalBaristas) {
    baristas.clear();
    baristas.resize(totalBaristas);
    
    for (int i = 0; i < totalBaristas; ++i) {
        baristas[i].id = i;
        baristas[i].isWorking = false;
        baristas[i].busyUntilTime = workingDayStartTime;
        baristas[i].totalOrdersCompleted = 0;
    }
}

void OrderDistributionAlgorithm::setBaristaStatus(int baristaId, bool isWorking) {
    if (baristaId >= 0 && baristaId < static_cast<int>(baristas.size())) {
        baristas[baristaId].isWorking = isWorking;
        std::cout << "Barista " << baristaId << " status: "
                  << (isWorking ? "WORKING" : "NOT WORKING") << std::endl;
    }
}

void OrderDistributionAlgorithm::setWorkingDayStart(double startTimeInMinutes) {
    workingDayStartTime = startTimeInMinutes;
    currentTime = startTimeInMinutes;

    for (auto& barista : baristas) {
        barista.busyUntilTime = startTimeInMinutes;
    }
}

void OrderDistributionAlgorithm::addOrderToCommonQueue(const Order& order) {
    Order newOrder = order;
    newOrder.orderTime = std::chrono::system_clock::now();
    commonQueue.push_back(newOrder);

    std::cout << "Order " << order.id << " added to common queue. "
              << "Queue size: " << commonQueue.size() << std::endl;
}

void OrderDistributionAlgorithm::distributeOrders() {
    std::cout << "\n=== Distributing orders ===" << std::endl;
    std::cout << "Orders in queue: " << commonQueue.size() << std::endl;
    std::cout << "Working baristas: " << getWorkingBaristasCount() << std::endl;

    while (!commonQueue.empty()) {
        Order order = commonQueue.front();
        commonQueue.pop_front();

        order.totalPreparationTime = calculateOrderPreparationTime(order);

        int bestBaristaId = findLeastLoadedBarista();

        if (bestBaristaId != -1) {
            assignOrderToBarista(order, bestBaristaId);
        } else {
            std::cerr << "No working baristas available!" << std::endl;
            commonQueue.push_front(order);
            break;
        }
    }

    std::cout << "=== Distribution complete ===\n" << std::endl;
}

void OrderDistributionAlgorithm::assignOrderToBarista(Order& order, int baristaId) {
    if (baristaId < 0 || baristaId >= static_cast<int>(baristas.size())) {
        std::cerr << "Invalid barista ID: " << baristaId << std::endl;
        return;
    }

    Barista& barista = baristas[baristaId];

    if (!barista.isWorking) {
        std::cerr << "Barista " << baristaId << " is not working!" << std::endl;
        return;
    }

    double startTime = std::max(currentTime, barista.busyUntilTime);
    order.estimatedReadyTime = startTime + order.totalPreparationTime;
    order.assignedBaristaId = baristaId;

    barista.orderQueue.push_back(order);
    barista.busyUntilTime = order.estimatedReadyTime;

    std::cout << "Order " << order.id << " assigned to Barista " << baristaId
              << ". Ready at: " << order.estimatedReadyTime << " min"
              << " (prep time: " << order.totalPreparationTime << " min)" << std::endl;
}

double OrderDistributionAlgorithm::calculateItemPreparationTime(const Item& item) {
    // Берем время приготовления из Item
    return static_cast<double>(item.get_preparation_time());
}

double OrderDistributionAlgorithm::calculateOrderPreparationTime(const Order& order) {
    double totalTime = 0.0;

    for (const auto& item : order.items) {
        totalTime += calculateItemPreparationTime(item);
    }

    if (order.items.size() > 3) {
        totalTime *= 1.2;
    }

    return totalTime;
}

int OrderDistributionAlgorithm::findLeastLoadedBarista() {
    int bestBaristaId = -1;
    double minBusyTime = std::numeric_limits<double>::max();

    for (const auto& barista : baristas) {
        if (barista.isWorking) {
            if (barista.busyUntilTime < minBusyTime) {
                minBusyTime = barista.busyUntilTime;
                bestBaristaId = barista.id;
            }
        }
    }

    return bestBaristaId;
}


int OrderDistributionAlgorithm::findBaristaWithShortestQueue() {
    int bestBaristaId = -1;
    size_t minQueueSize = std::numeric_limits<size_t>::max();

    for (const auto& barista : baristas) {
        if (barista.isWorking) {
            if (barista.orderQueue.size() < minQueueSize) {
                minQueueSize = barista.orderQueue.size();
                bestBaristaId = barista.id;
            }
        }
    }

    return bestBaristaId;
}

double OrderDistributionAlgorithm::getTotalProcessingTime() const {
    double maxTime = currentTime;

    for (const auto& barista : baristas) {
        if (barista.isWorking && barista.busyUntilTime > maxTime) {
            maxTime = barista.busyUntilTime;
        }
    }

    return maxTime - currentTime;
}

