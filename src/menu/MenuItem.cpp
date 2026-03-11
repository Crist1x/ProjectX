#include "menu/MenuItem.h"
#include <stdexcept>

Item::Item(int id, const std::string& name, const std::string& description,
           double price, int prep_time, int cafe_id, bool available)
    : id(id), name(name), description(description), price(price),
      preparation_time(prep_time), in_stock(available), cafe_id(cafe_id) {}

// Геттеры
int Item::get_id() const { return id; }
const std::string& Item::get_name() const { return name; }
const std::string& Item::get_description() const { return description; }
double Item::get_price() const { return price; }
int Item::get_preparation_time() const { return preparation_time; }
bool Item::is_available() const { return in_stock; }
int Item::get_cafe_id() const { return cafe_id; }
const std::map<std::string, std::string>& Item::get_attributes() const { return attributes; }

// Сеттеры
void Item::set_name(const std::string& new_name) {
    if (new_name.empty()) {
        throw std::invalid_argument("Item name cannot be empty");
    }
    name = new_name;
}
void Item::set_description(const std::string& new_description) {
    description = new_description;
}
void Item::set_price(double new_price) {
    if (new_price < 0) {
        throw std::invalid_argument("Price cannot be negative");
    }
    price = new_price;
}
void Item::set_preparation_time(int time) {
    if (time < 0) {
        throw std::invalid_argument("Preparation time cannot be negative");
    }
    preparation_time = time;
}
void Item::set_available(bool status) {
    in_stock = status;
}
void Item::set_cafe_id(int id) {
    cafe_id = id;
}
void Item::add_attribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

// JSON
json Item::toJson() const {
    return {
        {"id", id},
        {"name", name},
        {"description", description},
        {"price", price},
        {"preparation_time", preparation_time},
        {"in_stock", in_stock},
        {"cafe_id", cafe_id},
        {"attributes", attributes}
    };
}

Item Item::fromJson(const json& j) {
    Item item;
    item.id = j.value("id", -1);
    item.name = j.value("name", "");
    item.description = j.value("description", "");
    item.price = j.value("price", 0.0);
    item.preparation_time = j.value("preparation_time", 0);
    item.in_stock = j.value("in_stock", false);
    item.cafe_id = j.value("cafe_id", -1);

    if (j.contains("attributes")) {
        for (auto& [key, value] : j["attributes"].items()) {
            item.attributes[key] = value.get<std::string>();
        }
    }
    return item;
}

// Checking for validity
bool Item::isValid() const {
    return id > 0 &&
           !name.empty() &&
           price > 0 &&
           preparation_time > 0 &&
           cafe_id > 0;
}

std::string Item::to_telegram_format() const {
    std::string result = name + " - " + std::to_string(price) + "₽\n";
    result += "  _" + description + "_\n";
    result += "  ⏱ " + std::to_string(preparation_time) + " мин.";
    return result;
}