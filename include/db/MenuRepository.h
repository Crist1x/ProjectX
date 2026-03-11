#ifndef PROJECT_MENUREPOSITORY_H
#define PROJECT_MENUREPOSITORY_H

#include "db/Database.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
    struct MenuCategory {
        int id;
        std::string name;
    };

    struct MenuItem {
        int id;
        std::string name;
        std::string description;
        double price;
        int categoryId;
        std::string createdAt;
    };

    class MenuRepository {
    public:
        explicit MenuRepository(Database& database);

        //Categories
        bool createCategory(const std::string& name);
        std::optional<MenuCategory> findCategoryById(int id);
        std::vector<MenuCategory> findAllCategories();
        bool deleteCategory(int id);

        //Menu items
        bool createItem(const std::string& name,
                        const std::string& description,
                        double price,
                        int categoryId);
        std::optional<MenuItem> findItemById(int id);
        std::vector<MenuItem> findAllItems();
        std::vector<MenuItem> findItemsByCategory(int categoryId);
        bool updateItem(int id,
                        const std::string& name,
                        const std::string& description,
                        double price);
        bool deleteItem(int id);

    private:
        Database& db_;
    };
}

#endif //PROJECT_MENUREPOSITORY_H