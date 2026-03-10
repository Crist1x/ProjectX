#ifndef PROJECTX_MENUITEM_H
#define PROJECTX_MENUITEM_H

#include <string>
#include <map>
#include "../../cmake/json.hpp"

using json = nlohmann::json;

class Item {
private:
    int id{};
    std::string name;
    std::string description;
    double price{};
    int preparation_time{};  // в минутах
    bool in_stock{};
    std::map<std::string, std::string> attributes;
    int cafe_id{};

public:
    Item() = default;
    Item(int id, const std::string& name, const std::string& description,
         double price, int prep_time, int cafe_id, bool available = true);

    // Геттеры
    int get_id() const;
    const std::string& get_name() const;
    const std::string& get_description() const;
    double get_price() const;
    int get_preparation_time() const;
    bool is_available() const;
    int get_cafe_id() const;
    const std::map<std::string, std::string>& get_attributes() const;

    // Сеттеры
    void set_name(const std::string& new_name);
    void set_description(const std::string& new_description);
    void set_price(double new_price);
    void set_preparation_time(int time);
    void set_available(bool status);
    void set_cafe_id(int id);
    void add_attribute(const std::string& key, const std::string& value);

    json toJson() const;
    static Item fromJson(const json& j);

    bool isValid() const;
};

#endif