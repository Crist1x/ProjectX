//
// Created by CRIST1X on 31.01.2026.
//

#ifndef PROJECTX_MENUITEM_H
#define PROJECTX_MENUITEM_H

#include <string>
#include <map>
#include "../../libs/json.hpp"

using json = nlohmann::json;

class MenuItem{
private:
    int id{};
    std::string name;
    std::string description;
    double price{};
    int preparation_time{}; // в минутах
    bool in_stock{};
    std::map<std::string, std::string> attributes;

public:
    MenuItem(int id, std::string  name, std::string  description,
             double price, int prep_time, bool available = true);

    // Геттеры
    int get_id() const;
    const std::string& get_name() const;
    const std::string& get_description() const;
    double get_price() const;
    int get_preparation_time() const;
    bool is_available() const;
    const std::map<std::string, std::string>& get_attributes() const;

    // Сеттеры
    void set_price(double new_price);
    void set_preparation_time(int time);
    void set_available(bool status);
    void add_attribute(const std::string& key, const std::string& value);

};

#endif //PROJECTX_MENUITEM_H
