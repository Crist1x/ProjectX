#include "db/OrderRepository.h"
#include <sqlite3.h>
#include <iostream>

namespace db {
OrderRepository::OrderRepository(Database& database) : db_(database) {}

bool OrderRepository::createOrder(int userId, double totalAmount) {
    const char* sql = "INSERT INTO orders (user_id, total_amount, status) VALUES (?, ?, 'new');";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, userId);
    sqlite3_bind_double(stmt.get(), 2, totalAmount);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to create order: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::optional<Order> OrderRepository::findOrderById(int id) {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, id);

    Order order;
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        order.id = sqlite3_column_int(stmt.get(), 0);
        order.userId = sqlite3_column_int(stmt.get(), 1);
        order.totalAmount = sqlite3_column_double(stmt.get(), 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 4));
        return order;
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to find order by id: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return std::nullopt;
}

std::vector<Order> OrderRepository::findOrdersByUser(int userId) {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders WHERE user_id = ?;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<Order> orders;

    sqlite3_bind_int(stmt.get(), 1, userId);

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        Order order;
        order.id = sqlite3_column_int(stmt.get(), 0);
        order.userId = sqlite3_column_int(stmt.get(), 1);
        order.totalAmount = sqlite3_column_double(stmt.get(), 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 4));
        orders.push_back(order);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch orders by user: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return orders;
}

std::vector<Order> OrderRepository::findAllOrders() {
    const char* sql = "SELECT id, user_id, total_amount, status, created_at FROM orders;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<Order> orders;

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        Order order;
        order.id = sqlite3_column_int(stmt.get(), 0);
        order.userId = sqlite3_column_int(stmt.get(), 1);
        order.totalAmount = sqlite3_column_double(stmt.get(), 2);
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        order.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 4));
        orders.push_back(order);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch all orders: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return orders;
}

bool OrderRepository::updateOrderStatus(int id, const std::string& status) {
    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_text(stmt.get(), 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 2, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to update order status: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return sqlite3_changes(db_.getHandle()) > 0;
}

bool OrderRepository::cancelOrder(int id) {
    return updateOrderStatus(id, "cancelled");
}

bool OrderRepository::addOrderItem(int orderId, int menuItemId, int quantity, double price) {
    const char* sql = "INSERT INTO order_items (order_id, menu_item_id, quantity, price) VALUES (?, ?, ?, ?);";
    StatementPtr stmt = db_.prepare(sql);

    sqlite3_bind_int(stmt.get(), 1, orderId);
    sqlite3_bind_int(stmt.get(), 2, menuItemId);
    sqlite3_bind_int(stmt.get(), 3, quantity);
    sqlite3_bind_double(stmt.get(), 4, price);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to add order item: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return true;
}

std::vector<OrderItem> OrderRepository::findItemsByOrder(int orderId) {
    const char* sql = "SELECT id, order_id, menu_item_id, quantity, price FROM order_items WHERE order_id = ?;";
    StatementPtr stmt = db_.prepare(sql);
    std::vector<OrderItem> items;

    sqlite3_bind_int(stmt.get(), 1, orderId);

    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        OrderItem item;
        item.id = sqlite3_column_int(stmt.get(), 0);
        item.orderId = sqlite3_column_int(stmt.get(), 1);
        item.menuItemId = sqlite3_column_int(stmt.get(), 2);
        item.quantity = sqlite3_column_int(stmt.get(), 3);
        item.price = sqlite3_column_double(stmt.get(), 4);
        items.push_back(item);
    }
    if (rc != SQLITE_DONE) {
        throw DatabaseException("Failed to fetch order items: " + std::string(sqlite3_errmsg(db_.getHandle())));
    }

    return items;
}

bool OrderRepository::createOrderWithItems(int userId,
                                            const std::vector<std::pair<int, int>>& items) {
    try {
        db_.execute("BEGIN TRANSACTION;");
    } catch (const DatabaseException& e) {
        std::cerr << "Failed to begin transaction: " << e.what() << std::endl;
        return false;
    }

    try {
        double totalAmount = 0.0;
        for (const auto& [menuItemId, quantity] : items) {
            const char* priceSql = "SELECT price FROM menu_items WHERE id = ?;";
            StatementPtr priceStmt = db_.prepare(priceSql);

            sqlite3_bind_int(priceStmt.get(), 1, menuItemId);
            int rc = sqlite3_step(priceStmt.get());
            if (rc == SQLITE_ROW) {
                double price = sqlite3_column_double(priceStmt.get(), 0);
                totalAmount += price * quantity;
            } else if (rc != SQLITE_DONE) {
                throw DatabaseException("Failed to read menu item price: " + std::string(sqlite3_errmsg(db_.getHandle())));
            }
        }

        const char* orderSql = "INSERT INTO orders (user_id, total_amount, status) VALUES (?, ?, 'new');";
        StatementPtr orderStmt = db_.prepare(orderSql);

        sqlite3_bind_int(orderStmt.get(), 1, userId);
        sqlite3_bind_double(orderStmt.get(), 2, totalAmount);

        if (sqlite3_step(orderStmt.get()) != SQLITE_DONE) {
            throw DatabaseException("Failed to create order with items: " + std::string(sqlite3_errmsg(db_.getHandle())));
        }

        int orderId = sqlite3_last_insert_rowid(db_.getHandle());

        for (const auto& [menuItemId, quantity] : items) {
            const char* priceSql = "SELECT price FROM menu_items WHERE id = ?;";
            StatementPtr priceStmt = db_.prepare(priceSql);
            double price = 0.0;

            sqlite3_bind_int(priceStmt.get(), 1, menuItemId);
            int rc = sqlite3_step(priceStmt.get());
            if (rc == SQLITE_ROW) {
                price = sqlite3_column_double(priceStmt.get(), 0);
            } else if (rc != SQLITE_DONE) {
                throw DatabaseException("Failed to read order item price: " + std::string(sqlite3_errmsg(db_.getHandle())));
            }

            if (!addOrderItem(orderId, menuItemId, quantity, price)) {
                db_.tryExecute("ROLLBACK;");
                return false;
            }
        }
    } catch (const DatabaseException&) {
        db_.tryExecute("ROLLBACK;");
        throw;
    }

    try {
        db_.execute("COMMIT;");
    } catch (const DatabaseException& e) {
        std::cerr << "Failed to commit transaction: " << e.what() << std::endl;
        db_.tryExecute("ROLLBACK;");
        return false;
    }

    return true;
}
}
