#include "db/OrderRepository.h"
#include <sqlite3.h>
#include <iostream>

namespace db {
OrderRepository::OrderRepository(Database& database) : db_(database) {}

bool OrderRepository::createOrder(int userId, double totalAmount) {
    const char* sql = "INSERT INTO orders (user_id, total_amount, status) VALUES (?, ?, 'new');";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_double(stmt, 2, totalAmount);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::optional<Order> OrderRepository::findOrderById(int id) {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    Order order;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        order.id = sqlite3_column_int(stmt, 0);
        order.userId = sqlite3_column_int(stmt, 1);
        order.totalAmount = sqlite3_column_double(stmt, 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        sqlite3_finalize(stmt);
        return order;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Order> OrderRepository::findOrdersByUser(int userId) {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Order> orders;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return orders;
    }

    sqlite3_bind_int(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Order order;
        order.id = sqlite3_column_int(stmt, 0);
        order.userId = sqlite3_column_int(stmt, 1);
        order.totalAmount = sqlite3_column_double(stmt, 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        orders.push_back(order);
    }

    sqlite3_finalize(stmt);
    return orders;
}

std::vector<Order> OrderRepository::findAllOrders() {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Order> orders;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return orders;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Order order;
        order.id = sqlite3_column_int(stmt, 0);
        order.userId = sqlite3_column_int(stmt, 1);
        order.totalAmount = sqlite3_column_double(stmt, 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        orders.push_back(order);
    }

    sqlite3_finalize(stmt);
    return orders;
}

bool OrderRepository::updateOrderStatus(int id, const std::string& status) {
    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool OrderRepository::cancelOrder(int id) {
    return updateOrderStatus(id, "cancelled");
}

bool OrderRepository::addOrderItem(int orderId, int menuItemId, int quantity, double price) {
    const char* sql = "INSERT INTO order_items (order_id, menu_item_id, quantity, price) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, orderId);
    sqlite3_bind_int(stmt, 2, menuItemId);
    sqlite3_bind_int(stmt, 3, quantity);
    sqlite3_bind_double(stmt, 4, price);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::vector<OrderItem> OrderRepository::findItemsByOrder(int orderId) {
    const char* sql = "SELECT id, order_id, menu_item_id, quantity, price FROM order_items WHERE order_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<OrderItem> items;

    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return items;
    }

    sqlite3_bind_int(stmt, 1, orderId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        OrderItem item;
        item.id = sqlite3_column_int(stmt, 0);
        item.orderId = sqlite3_column_int(stmt, 1);
        item.menuItemId = sqlite3_column_int(stmt, 2);
        item.quantity = sqlite3_column_int(stmt, 3);
        item.price = sqlite3_column_double(stmt, 4);
        items.push_back(item);
    }

    sqlite3_finalize(stmt);
    return items;
}

bool OrderRepository::createOrderWithItems(int userId,
                                            const std::vector<std::pair<int, int>>& items) {
    if (!db_.execute("BEGIN TRANSACTION;")) {
        std::cerr << "Failed to begin transaction" << std::endl;
        return false;
    }

    double totalAmount = 0.0;
    for (const auto& [menuItemId, quantity] : items) {
        const char* priceSql = "SELECT price FROM menu_items WHERE id = ?;";
        sqlite3_stmt* priceStmt = nullptr;

        if (sqlite3_prepare_v2(db_.getHandle(), priceSql, -1, &priceStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(priceStmt, 1, menuItemId);
            if (sqlite3_step(priceStmt) == SQLITE_ROW) {
                double price = sqlite3_column_double(priceStmt, 0);
                totalAmount += price * quantity;
            }
            sqlite3_finalize(priceStmt);
        }
    }

    const char* orderSql = "INSERT INTO orders (user_id, total_amount, status) VALUES (?, ?, 'new');";
    sqlite3_stmt* orderStmt = nullptr;

    if (sqlite3_prepare_v2(db_.getHandle(), orderSql, -1, &orderStmt, nullptr) != SQLITE_OK) {
        db_.execute("ROLLBACK;");
        return false;
    }

    sqlite3_bind_int(orderStmt, 1, userId);
    sqlite3_bind_double(orderStmt, 2, totalAmount);

    if (sqlite3_step(orderStmt) != SQLITE_DONE) {
        sqlite3_finalize(orderStmt);
        db_.execute("ROLLBACK;");
        return false;
    }

    int orderId = sqlite3_last_insert_rowid(db_.getHandle());
    sqlite3_finalize(orderStmt);

    for (const auto& [menuItemId, quantity] : items) {
        // Получаем цену блюда
        const char* priceSql = "SELECT price FROM menu_items WHERE id = ?;";
        sqlite3_stmt* priceStmt = nullptr;
        double price = 0.0;

        if (sqlite3_prepare_v2(db_.getHandle(), priceSql, -1, &priceStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(priceStmt, 1, menuItemId);
            if (sqlite3_step(priceStmt) == SQLITE_ROW) {
                price = sqlite3_column_double(priceStmt, 0);
            }
            sqlite3_finalize(priceStmt);
        }

        if (!addOrderItem(orderId, menuItemId, quantity, price)) {
            db_.execute("ROLLBACK;");
            return false;
        }
    }

    if (!db_.execute("COMMIT;")) {
        std::cerr << "Failed to commit transaction" << std::endl;
        db_.execute("ROLLBACK;");
        return false;
    }

    return true;
}
}