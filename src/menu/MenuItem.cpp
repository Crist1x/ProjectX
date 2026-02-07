//
// Created by CRIST1X on 31.01.2026.
//
#include "menu/MenuItem.h"

#include <utility>

MenuItem::MenuItem(int id, std::string  name, std::string  description, double price, int prep_time,
                   bool in_stock) : id(id), name(std::move(name)), description(std::move(description)), price(price), preparation_time(prep_time),
                   in_stock(in_stock) {}

// Геттеры
int MenuItem::get_id() const { return id; }
const std::string& MenuItem::get_name() const { return name; }
const std::string& MenuItem::get_description() const { return description; }
double MenuItem::get_price() const { return price; }
int MenuItem::get_preparation_time() const { return preparation_time; }
bool MenuItem::is_available() const { return in_stock; }
const std::map<std::string, std::string>& MenuItem::get_attributes() const { return attributes;}

// Сеттеры
void MenuItem::set_price(double new_price) { price = new_price; }
void MenuItem::set_preparation_time(int time) { preparation_time = time; }
void MenuItem::set_available(bool status) { in_stock = status; }
void MenuItem::add_attribute(const std::string& key, const std::string& value) { attributes[key] = value; }