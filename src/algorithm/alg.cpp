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

