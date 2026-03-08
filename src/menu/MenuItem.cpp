#include "MenuItem.h"

#include <utility>

Item::Item(int id, std::string  name, std::string  description, double price, int prep_time,
                   bool in_stock) : id(id), name(std::move(name)), description(std::move(description)), price(price), preparation_time(prep_time),
                   in_stock(in_stock) {}

// Геттеры
int Item::get_id() const { return id; }
const std::string& Item::get_name() const { return name; }
const std::string& Item::get_description() const { return description; }
double Item::get_price() const { return price; }
int Item::get_preparation_time() const { return preparation_time; }
bool Item::is_available() const { return in_stock; }
const std::map<std::string, std::string>& Item::get_attributes() const { return attributes;}

// Сеттеры
void Item::set_price(double new_price) { price = new_price; }
void Item::set_preparation_time(int time) { preparation_time = time; }
void Item::set_available(bool status) { in_stock = status; }
void Item::add_attribute(const std::string& key, const std::string& value) { attributes[key] = value; }