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

