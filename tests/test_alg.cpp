#include "algorithm/alg.h"
#include <gtest/gtest.h>
#include <chrono>
#include <string>

Order createOrder(int id, int items_count = 1) {
    Order order;
    order.set_id(id);
    order.set_user_id(1);
    order.set_status("new");
    for (int i = 0; i < items_count; i++) {
        auto item = std::make_shared<Item>(i + 1, "Test Item", "Desc", 100.0, 5, 1, 1);
        order.add_item(item);
    }
    return order;
}

TEST(BaristaTest, DefaultConstructor) {
    Barista b;
    EXPECT_EQ(b.getId(), 0);
    EXPECT_FALSE(b.getIsWorking());
    EXPECT_EQ(b.getQueueSize(), 0);
}

TEST(BaristaTest, ConstructorWithParams) {
    Barista b(5, true);
    EXPECT_EQ(b.getId(), 5);
    EXPECT_TRUE(b.getIsWorking());
}

TEST(BaristaTest, AddOrderToQueue) {
    Barista b(1, true);
    EXPECT_EQ(b.getQueueSize(), 0);

    b.addOrderToQueue(createOrder(1));
    EXPECT_EQ(b.getQueueSize(), 1);

    b.addOrderToQueue(createOrder(2));
    EXPECT_EQ(b.getQueueSize(), 2);
}

TEST(BaristaTest, ChangeWorkingStatus) {
    Barista b(1, true);
    b.setIsWorking(false);
    EXPECT_FALSE(b.getIsWorking());

    b.setIsWorking(true);
    EXPECT_TRUE(b.getIsWorking());
}

TEST(BaristaTest, SetBusyTime) {
    Barista b(1, true);
    b.setBusyUntilTime(100.5);
    EXPECT_DOUBLE_EQ(b.getBusyUntilTime(), 100.5);
}

TEST(BaristaTest, ClearQueue) {
    Barista b(1, true);
    for (int i = 0; i < 5; i++) {
        b.addOrderToQueue(createOrder(i));
    }
    EXPECT_EQ(b.getQueueSize(), 5);
    b.clearQueue();
    EXPECT_EQ(b.getQueueSize(), 0);
}

TEST(AlgorithmTest, InitializeBaristas) {
    alg algorithm;
    algorithm.initializeBaristas(5);
    EXPECT_EQ(algorithm.getBaristas().size(), 5);
}

TEST(AlgorithmTest, SetBaristaStatus) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    algorithm.setBaristaStatus(0, true);
    algorithm.setBaristaStatus(1, false);

    EXPECT_TRUE(algorithm.getBarista(0).getIsWorking());
    EXPECT_FALSE(algorithm.getBarista(1).getIsWorking());
}

TEST(AlgorithmTest, CountWorkingBaristas) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    algorithm.setBaristaStatus(0, true);
    algorithm.setBaristaStatus(1, true);
    algorithm.setBaristaStatus(2, false);

    EXPECT_EQ(algorithm.getWorkingBaristasCount(), 2);
}

TEST(AlgorithmTest, AddOrderToQueue) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    EXPECT_EQ(algorithm.getCommonQueueSize(), 0);

    algorithm.addOrderToCommonQueue(createOrder(1));
    EXPECT_EQ(algorithm.getCommonQueueSize(), 1);
}

TEST(AlgorithmTest, DistributeOrders) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    algorithm.setBaristaStatus(0, true);
    algorithm.setBaristaStatus(1, true);

    algorithm.addOrderToCommonQueue(createOrder(1));
    algorithm.addOrderToCommonQueue(createOrder(2));
    algorithm.distributeOrders();

    EXPECT_EQ(algorithm.getCommonQueueSize(), 0);
}

TEST(AlgorithmTest, NoWorkingBaristas) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    algorithm.setBaristaStatus(0, false);
    algorithm.setBaristaStatus(1, false);
    algorithm.setBaristaStatus(2, false);

    algorithm.addOrderToCommonQueue(createOrder(1));
    algorithm.distributeOrders();

    EXPECT_EQ(algorithm.getCommonQueueSize(), 1);
}

TEST(AlgorithmTest, CalculatePrepTime) {
    alg algorithm;
    algorithm.initializeBaristas(3);

    Order o1 = createOrder(1, 1);
    Order o2 = createOrder(2, 3);

    double t1 = algorithm.calculateOrderPreparationTime(o1);
    double t2 = algorithm.calculateOrderPreparationTime(o2);

    EXPECT_LT(t1, t2);
}
TEST(AlgorithmTest, Reset) {
    alg algorithm;
    algorithm.initializeBaristas(3);
    algorithm.setBaristaStatus(0, true);
    algorithm.addOrderToCommonQueue(createOrder(1));
    algorithm.distributeOrders();

    algorithm.reset();
    EXPECT_EQ(algorithm.getCommonQueueSize(), 0);
}

TEST(AlgorithmTest, FullWorkflow) {
    alg algorithm;
    algorithm.initializeBaristas(4);

    algorithm.setBaristaStatus(0, true);
    algorithm.setBaristaStatus(1, true);
    algorithm.setBaristaStatus(2, true);
    algorithm.setBaristaStatus(3, false);

    for (int i = 0; i < 10; i++) {
        algorithm.addOrderToCommonQueue(createOrder(i));
    }

    auto start = std::chrono::high_resolution_clock::now();
    algorithm.distributeOrders();
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(algorithm.getCommonQueueSize(), 0);
    EXPECT_EQ(algorithm.getWorkingBaristasCount(), 3);

    std::cout << "Test completed in " << ms.count() << "ms" << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
