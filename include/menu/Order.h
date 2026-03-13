#ifndef PROJECTX_ORDER_H
#define PROJECTX_ORDER_H

#include <string>
#include <utility>

class Order {
private:
    int id{};
    int user_id{};
    double total_amount{};
    std::string status;
    std::string created_at;

public:
    Order() = default;
    Order(int id, int user_id, double total_amount, std::string status, std::string created_at)
        : id(id),
          user_id(user_id),
          total_amount(total_amount),
          status(std::move(status)),
          created_at(std::move(created_at)) {}

    int get_id() const { return id; }
    int get_user_id() const { return user_id; }
    double get_total_amount() const { return total_amount; }
    const std::string& get_status() const { return status; }
    const std::string& get_created_at() const { return created_at; }

    void set_status(const std::string& new_status) { status = new_status; }
};

class OrderItem {
private:
    int id{};
    int order_id{};
    int menu_item_id{};
    int quantity{};
    double price{};

public:
    OrderItem() = default;
    OrderItem(int id, int order_id, int menu_item_id, int quantity, double price)
        : id(id),
          order_id(order_id),
          menu_item_id(menu_item_id),
          quantity(quantity),
          price(price) {}

    int get_id() const { return id; }
    int get_order_id() const { return order_id; }
    int get_menu_item_id() const { return menu_item_id; }
    int get_quantity() const { return quantity; }
    double get_price() const { return price; }
};

#endif
