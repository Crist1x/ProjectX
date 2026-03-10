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

    const Item* item = cafe_menu.find_item(3);
    if (item) {
        std::cout << "Нашли: " << item->get_name()
                  << " за " << item->get_price() << "₽" << std::endl;
    }

    return 0;
}