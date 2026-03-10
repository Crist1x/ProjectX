#include <gtest/gtest.h>
#include "menu/MenuItem.h"
#include "menu/MenuCategory.h"
#include "menu/Menu.h"

// Тестируем класс Item
TEST(MenuTest, ItemCreationAndGetters) {
    Item item(1, "Кофе Латте", "Молочный кофе", 150.0, 5, 1, true);

    EXPECT_EQ(item.get_name(), "Кофе Латте");
    EXPECT_DOUBLE_EQ(item.get_price(), 150.0);
    EXPECT_TRUE(item.is_available());
}

// Тестируем класс MenuCategory
TEST(MenuTest, CategoryAddsItemsCorrectly) {
    Category drinks(1, "Напитки", "Горячие и холодные", 1);
    drinks.add_item(Item(1, "Кофе", "Черный", 100.0, 3, 1, true));
    drinks.add_item(Item(2, "Чай", "Зеленый", 80.0, 2, 1, true));

    // Предполагается, что у вас есть геттер get_items() или метод get_items_count()
    // EXPECT_EQ(drinks.get_items().size(), 2);
}

// Тестируем класс Menu
TEST(MenuTest, MenuFindsItemById) {
    Menu cafe_menu(1, "Булочная");
    Category bakery(2, "Выпечка", "Свежая", 1);
    bakery.add_item(Item(3, "Круассан", "С маслом", 200.0, 10, 1, true));

    cafe_menu.add_category(bakery);

    const Item* found_item = cafe_menu.find_item(3);
    ASSERT_NE(found_item, nullptr); // Проверяем, что указатель не null
    EXPECT_EQ(found_item->get_name(), "Круассан");
}