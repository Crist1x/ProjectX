#include <gtest/gtest.h>
#include "menu/Menu.h"
#include "menu/MenuCategory.h"
#include "menu/MenuItem.h"

// тесты для класса блюд
TEST(MenuItemTest, CreationAndGetters) {
    Item item(1, "Латте", "Вкусный кофе", 150.0, 5, 1);
    EXPECT_EQ(item.get_id(), 1);
    EXPECT_EQ(item.get_name(), "Латте");
    EXPECT_DOUBLE_EQ(item.get_price(), 150.0);
    EXPECT_EQ(item.get_preparation_time(), 5);
    EXPECT_TRUE(item.is_available());
}

TEST(MenuItemTest, ValidationWorks) {
    Item valid_item(1, "Круассан", "Выпечка", 100.0, 3, 1);
    EXPECT_TRUE(valid_item.isValid());

    Item invalid_item(-1, "", "", -10.0, -5, -1);
    EXPECT_FALSE(invalid_item.isValid());
}

TEST(MenuItemTest, SettersThrowExceptions) {
    Item item(1, "Чай", "Черный", 50.0, 2, 1);

    EXPECT_THROW(item.set_price(-10.0), std::invalid_argument);
    EXPECT_THROW(item.set_name(""), std::invalid_argument);
    EXPECT_THROW(item.set_preparation_time(-5), std::invalid_argument);
}

// тесты для класса категорий
TEST(MenuCategoryTest, AddItemAndCount) {
    Category cat(1, "Напитки", "Горячие", 1);
    Item coffee(1, "Эспрессо", "Крепкий", 100.0, 2, 1);

    cat.add_item(coffee);
    EXPECT_EQ(cat.get_items_count(), 1);
}

TEST(MenuCategoryTest, CannotAddInvalidItem) {
    Category cat(1, "Напитки", "Горячие", 1);
    Item invalid_item(-1, "", "", -10.0, -5, -1);

    EXPECT_THROW(cat.add_item(invalid_item), std::invalid_argument);
}

TEST(MenuCategoryTest, CannotAddItemFromOtherCafe) {
    Category cat(1, "Еда", "Снеки", 1); // cafe_id = 1
    Item alien_item(1, "Бургер", "Сытный", 300.0, 10, 999); // cafe_id = 999

    // Ожидаем исключение, так как айди не совпадают
    EXPECT_THROW(cat.add_item(alien_item), std::invalid_argument);
}

TEST(MenuCategoryTest, PriceCalculations) {
    Category cat(1, "Десерты", "Сладкое", 1);
    cat.add_item(Item(1, "Пончик", "", 80.0, 1, 1));
    cat.add_item(Item(2, "Торт", "", 500.0, 15, 1));
    cat.add_item(Item(3, "Эклер", "", 120.0, 5, 1));

    EXPECT_DOUBLE_EQ(cat.get_min_price(), 80.0);
    EXPECT_DOUBLE_EQ(cat.get_max_price(), 500.0);
    EXPECT_EQ(cat.get_max_preparation_time(), 15);
}

// тесты для класса меню
TEST(MenuTest, AddCategoryAndCount) {
    Menu menu(1, "Столовая");
    Category cat(1, "Супы", "Первое", 1);
    cat.add_item(Item(1, "Борщ", "", 150.0, 10, 1));

    menu.add_category(cat);

    EXPECT_EQ(menu.get_categories_count(), 1);
    EXPECT_EQ(menu.get_total_items_count(), 1);
}

TEST(MenuTest, FindItemGlobally) {
    Menu menu(1, "Булочная");
    Category cat(1, "Выпечка", "", 1);
    cat.add_item(Item(42, "Ватрушка", "", 75.0, 2, 1));
    menu.add_category(cat);

    auto found_item = menu.find_item(42);
    ASSERT_NE(found_item, nullptr);
    EXPECT_EQ(found_item.value()->get_name(), "Ватрушка");

    auto not_found = menu.find_item(999);
    EXPECT_FALSE(not_found.has_value());
}

TEST(MenuTest, MenuPriceRange) {
    Menu menu(1, "Кафетерий");

    Category drinks(1, "Напитки", "", 1);
    drinks.add_item(Item(1, "Вода", "", 50.0, 1, 1));

    Category food(2, "Еда", "", 1);
    food.add_item(Item(2, "Стейк", "", 1000.0, 30, 1));

    menu.add_category(drinks);
    menu.add_category(food);

    EXPECT_DOUBLE_EQ(menu.get_total_price_range_min(), 50.0);
    EXPECT_DOUBLE_EQ(menu.get_total_price_range_max(), 1000.0);
}

TEST(MenuTest, TelegramFormatString) {
    Menu menu(1, "Тест Кафе");
    Category cat(1, "Тест Категория", "Описание", 1);
    cat.add_item(Item(1, "Блюдо", "Описание блюда", 100.0, 5, 1));
    menu.add_category(cat);

    std::string text = menu.to_telegram_format();

    // Проверяем, что в итоговой строке есть нужные куски текста
    EXPECT_NE(text.find("Тест Кафе"), std::string::npos);
    EXPECT_NE(text.find("Тест Категория"), std::string::npos);
    EXPECT_NE(text.find("Блюдо"), std::string::npos);
    EXPECT_NE(text.find("100.00"), std::string::npos);
}