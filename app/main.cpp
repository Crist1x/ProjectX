#include "menu/MenuItem.h"
#include "menu/MenuCategory.h"
#include "menu/Menu.h"
#include <iostream>
#include <memory>

int main() {
    Menu cafe_menu(1, "Булочная");

    Category drinks(1, "Напитки", "Горячие и холодные напитки", 1);

    drinks.add_item(Item(1, "Кофе Латте", "Молочный кофе", 150.0, 5, 1, true));
    drinks.add_item(Item(2, "Чай Зелёный", "Китайский чай", 100.0, 3, 1, true));

    // Категория "Выпечка"
    Category bakery(2, "Выпечка", "Свежая выпечка", 1);
    bakery.add_item(Item(3, "Круассан", "С маслом", 200.0, 10, 1, true));
    bakery.add_item(Item(4, "Пончик", "С шоколадом", 150.0, 5, 1, true));

    cafe_menu.add_category(drinks);
    cafe_menu.add_category(bakery);

    std::cout << cafe_menu.to_telegram_format() << std::endl;

    auto item_opt = cafe_menu.find_item(3);

    if (item_opt.has_value()) {
        std::cout << "Нашли: " << item_opt.value()->get_name()
                  << " за " << item_opt.value()->get_price() << "₽\n";
    } else {
        std::cout << "Упс! Такого блюда в меню нет.\n";
    }

    return 0;
}