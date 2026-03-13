#ifndef PROJECT_ORDERREPOSITORY_H
#define PROJECT_ORDERREPOSITORY_H

#include "db/Database.h"
#include "menu/Order.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
    class OrderRepository {
    public:
        explicit OrderRepository(Database& database);

        bool createOrder(int userId, double totalAmount);
        std::optional<::Order> findOrderById(int id);
        std::vector<::Order> findOrdersByUser(int userId);
        std::vector<::Order> findAllOrders();
        bool updateOrderStatus(int id, const std::string& status);
        bool cancelOrder(int id);

        bool addOrderItem(int orderId, int menuItemId, int quantity, double price);
        std::vector<::OrderItem> findItemsByOrder(int orderId);

        bool createOrderWithItems(int userId,
                                   const std::vector<std::pair<int, int>>& items); // menuItemId, quantity

    private:
        Database& db_;
    };
}

#endif //PROJECT_ORDERREPOSITORY_H
