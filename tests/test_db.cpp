#include <gtest/gtest.h>
#include "db/Database.h"
#include "db/MenuRepository.h"
#include "db/OrderRepository.h"
#include "db/UserRepository.h"

namespace {

int getCount(db::Database& database, const std::string& sql) {
    auto stmt = database.prepare(sql);
    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        throw db::DatabaseException("Failed to count rows");
    }
    return sqlite3_column_int(stmt.get(), 0);
}

int getLastInsertId(db::Database& database) {
    auto stmt = database.prepare("SELECT last_insert_rowid();");
    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        throw db::DatabaseException("Failed to get last insert id");
    }
    return sqlite3_column_int(stmt.get(), 0);
}

class DatabaseRepositoryTest : public ::testing::Test {
protected:
    DatabaseRepositoryTest()
        : database_(db::Database::DEFAULT_PATH),
          userRepository_(database_),
          menuRepository_(database_),
          orderRepository_(database_) {}

    void SetUp() override {
        database_.initializeSchema();
    }

    db::Database database_;
    db::UserRepository userRepository_;
    db::MenuRepository menuRepository_;
    db::OrderRepository orderRepository_;
};

TEST(DatabaseTest, ConstructorThrowsForInvalidPath) {
    EXPECT_THROW(db::Database("/definitely/missing/projectx/test.db"), db::DatabaseException);
}

TEST(DatabaseTest, ExecutePrepareAndTryExecuteWork) {
    db::Database database(db::Database::DEFAULT_PATH);

    EXPECT_TRUE(database.isOpen());
    EXPECT_FALSE(database.tryExecute("INVALID SQL"));

    database.execute("CREATE TABLE sample (id INTEGER PRIMARY KEY, value TEXT NOT NULL);");

    auto insertStmt = database.prepare("INSERT INTO sample (id, value) VALUES (?, ?);");
    sqlite3_bind_int(insertStmt.get(), 1, 1);
    sqlite3_bind_text(insertStmt.get(), 2, "ok", -1, SQLITE_STATIC);
    EXPECT_EQ(sqlite3_step(insertStmt.get()), SQLITE_DONE);

    auto selectStmt = database.prepare("SELECT value FROM sample WHERE id = ?;");
    sqlite3_bind_int(selectStmt.get(), 1, 1);
    ASSERT_EQ(sqlite3_step(selectStmt.get()), SQLITE_ROW);
    EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(selectStmt.get(), 0)), "ok");
}

TEST(DatabaseTest, ExecuteThrowsOnInvalidSql) {
    db::Database database(db::Database::DEFAULT_PATH);
    EXPECT_THROW(database.execute("INVALID SQL"), db::DatabaseException);
    EXPECT_THROW(database.prepare("SELECT * FROM"), db::DatabaseException);
}

TEST_F(DatabaseRepositoryTest, UserRepositorySupportsCrud) {
    ASSERT_TRUE(userRepository_.create(123456789LL, "alice"));

    auto byTelegramId = userRepository_.findByTelegramId(123456789LL);
    ASSERT_TRUE(byTelegramId.has_value());
    EXPECT_EQ(byTelegramId->username, "alice");

    auto byId = userRepository_.findById(byTelegramId->id);
    ASSERT_TRUE(byId.has_value());
    EXPECT_EQ(byId->telegramId, 123456789LL);

    auto allUsers = userRepository_.findAll();
    ASSERT_EQ(allUsers.size(), 1u);
    EXPECT_EQ(allUsers.front().username, "alice");

    EXPECT_TRUE(userRepository_.updateUsername(byTelegramId->id, "alice_new"));
    auto updated = userRepository_.findById(byTelegramId->id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->username, "alice_new");

    EXPECT_FALSE(userRepository_.updateUsername(9999, "ghost"));
    EXPECT_TRUE(userRepository_.remove(byTelegramId->id));
    EXPECT_FALSE(userRepository_.remove(byTelegramId->id));
    EXPECT_FALSE(userRepository_.findById(byTelegramId->id).has_value());
}

TEST_F(DatabaseRepositoryTest, UserRepositoryThrowsOnConstraintViolation) {
    ASSERT_TRUE(userRepository_.create(1LL, "alice"));
    EXPECT_THROW(userRepository_.create(1LL, "alice_dup"), db::DatabaseException);
}

TEST_F(DatabaseRepositoryTest, MenuRepositorySupportsCrud) {
    ASSERT_TRUE(menuRepository_.createCategory("Drinks", 1));
    auto categories = menuRepository_.findAllCategories();
    ASSERT_EQ(categories.size(), 1u);
    EXPECT_EQ(categories.front().get_name(), "Drinks");
    EXPECT_EQ(categories.front().get_cafe_id(), 1);

    int categoryId = categories.front().get_id();
    auto category = menuRepository_.findCategoryById(categoryId);
    ASSERT_TRUE(category.has_value());
    EXPECT_EQ(category->get_name(), "Drinks");
    EXPECT_EQ(category->get_cafe_id(), 1);

    ASSERT_TRUE(menuRepository_.createItem("Latte", "Coffee with milk", 4.5, categoryId));
    auto items = menuRepository_.findAllItems();
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items.front().get_name(), "Latte");
    EXPECT_DOUBLE_EQ(items.front().get_price(), 4.5);
    EXPECT_EQ(items.front().get_cafe_id(), 1);

    auto byCategory = menuRepository_.findItemsByCategory(categoryId);
    ASSERT_EQ(byCategory.size(), 1u);
    EXPECT_EQ(byCategory.front().get_category_id(), categoryId);

    EXPECT_TRUE(menuRepository_.updateItem(items.front().get_id(), "Flat White", "Double shot", 5.0));
    auto updated = menuRepository_.findItemById(items.front().get_id());
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->get_name(), "Flat White");
    EXPECT_DOUBLE_EQ(updated->get_price(), 5.0);

    EXPECT_FALSE(menuRepository_.updateItem(9999, "Ghost", "Ghost", 1.0));
    EXPECT_TRUE(menuRepository_.deleteItem(items.front().get_id()));
    EXPECT_FALSE(menuRepository_.deleteItem(items.front().get_id()));
    EXPECT_TRUE(menuRepository_.deleteCategory(categoryId));
    EXPECT_FALSE(menuRepository_.deleteCategory(categoryId));
}

TEST_F(DatabaseRepositoryTest, MenuRepositoryThrowsOnForeignKeyViolation) {
    EXPECT_THROW(menuRepository_.createItem("Broken", "No category", 1.0, 999), db::DatabaseException);
}

TEST_F(DatabaseRepositoryTest, OrderRepositorySupportsCrudAndItems) {
    ASSERT_TRUE(userRepository_.create(500LL, "bob"));
    ASSERT_TRUE(menuRepository_.createCategory("Food", 1));
    int userId = userRepository_.findByTelegramId(500LL)->id;
    int categoryId = menuRepository_.findAllCategories().front().get_id();

    ASSERT_TRUE(menuRepository_.createItem("Burger", "Beef", 12.5, categoryId));
    int menuItemId = menuRepository_.findAllItems().front().get_id();

    ASSERT_TRUE(orderRepository_.createOrder(userId, 12.5));
    int orderId = getLastInsertId(database_);

    auto order = orderRepository_.findOrderById(orderId);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->get_user_id(), userId);
    EXPECT_EQ(order->get_status(), "new");

    ASSERT_TRUE(orderRepository_.addOrderItem(orderId, menuItemId, 2, 12.5));
    auto items = orderRepository_.findItemsByOrder(orderId);
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items.front().get_quantity(), 2);
    EXPECT_DOUBLE_EQ(items.front().get_price(), 12.5);

    auto userOrders = orderRepository_.findOrdersByUser(userId);
    ASSERT_EQ(userOrders.size(), 1u);
    auto allOrders = orderRepository_.findAllOrders();
    ASSERT_EQ(allOrders.size(), 1u);

    EXPECT_TRUE(orderRepository_.updateOrderStatus(orderId, "ready"));
    EXPECT_TRUE(orderRepository_.cancelOrder(orderId));
    auto cancelled = orderRepository_.findOrderById(orderId);
    ASSERT_TRUE(cancelled.has_value());
    EXPECT_EQ(cancelled->get_status(), "cancelled");

    EXPECT_FALSE(orderRepository_.updateOrderStatus(9999, "ghost"));
}

TEST_F(DatabaseRepositoryTest, OrderRepositoryCreateOrderWithItemsCommitsTransaction) {
    ASSERT_TRUE(userRepository_.create(700LL, "carol"));
    ASSERT_TRUE(menuRepository_.createCategory("Desserts", 2));
    int userId = userRepository_.findByTelegramId(700LL)->id;
    int categoryId = menuRepository_.findAllCategories().front().get_id();

    ASSERT_TRUE(menuRepository_.createItem("Cake", "Chocolate", 7.5, categoryId));
    ASSERT_TRUE(menuRepository_.createItem("Tea", "Green", 2.5, categoryId));

    auto menuItems = menuRepository_.findAllItems();
    ASSERT_EQ(menuItems.size(), 2u);

    std::vector<std::pair<int, int>> items = {
        {menuItems[0].get_id(), 2},
        {menuItems[1].get_id(), 1}
    };

    ASSERT_TRUE(orderRepository_.createOrderWithItems(userId, items));

    auto orders = orderRepository_.findOrdersByUser(userId);
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_DOUBLE_EQ(orders.front().get_total_amount(), 17.5);
    EXPECT_EQ(orders.front().get_status(), "new");

    auto orderItems = orderRepository_.findItemsByOrder(orders.front().get_id());
    ASSERT_EQ(orderItems.size(), 2u);
}

TEST_F(DatabaseRepositoryTest, OrderRepositoryCreateOrderWithItemsRollsBackOnFailure) {
    ASSERT_TRUE(userRepository_.create(900LL, "dave"));
    ASSERT_TRUE(menuRepository_.createCategory("Hot", 3));
    int userId = userRepository_.findByTelegramId(900LL)->id;
    int categoryId = menuRepository_.findAllCategories().front().get_id();

    ASSERT_TRUE(menuRepository_.createItem("Soup", "Tomato", 6.0, categoryId));
    int validItemId = menuRepository_.findAllItems().front().get_id();

    std::vector<std::pair<int, int>> items = {
        {validItemId, 1},
        {999999, 1}
    };

    EXPECT_THROW(orderRepository_.createOrderWithItems(userId, items), db::DatabaseException);
    EXPECT_EQ(getCount(database_, "SELECT COUNT(*) FROM orders;"), 0);
    EXPECT_EQ(getCount(database_, "SELECT COUNT(*) FROM order_items;"), 0);
}

} 
