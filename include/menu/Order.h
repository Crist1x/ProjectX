#ifndef PROJECTX_ORDER_H
#define PROJECTX_ORDER_H

#include "menu/MenuItem.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Order {
private:
    int id{};
    int user_id{};
    double total_amount{};
    std::string status;
    std::string created_at;
    std::vector<std::shared_ptr<Item>> items;
    double estimated_ready_time{};
    double total_preparation_time{};
    int assigned_barista_id{-1};

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
    const std::vector<std::shared_ptr<Item>>& get_items() const { return items; }
    double get_estimated_ready_time() const { return estimated_ready_time; }
    double get_total_preparation_time() const { return total_preparation_time; }
    int get_assigned_barista_id() const { return assigned_barista_id; }

    void set_id(int new_id) { id = new_id; }
    void set_user_id(int new_user_id) { user_id = new_user_id; }
    void set_total_amount(double new_total_amount) { total_amount = new_total_amount; }
    void set_status(const std::string& new_status) { status = new_status; }
    void set_created_at(const std::string& new_created_at) { created_at = new_created_at; }
    void add_item(const std::shared_ptr<Item>& item) { items.push_back(item); }
    void clear_items() { items.clear(); }
    void set_estimated_ready_time(double value) { estimated_ready_time = value; }
    void set_total_preparation_time(double value) { total_preparation_time = value; }
    void set_assigned_barista_id(int value) { assigned_barista_id = value; }
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
