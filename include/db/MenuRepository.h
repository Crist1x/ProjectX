#ifndef PROJECT_MENUREPOSITORY_H
#define PROJECT_MENUREPOSITORY_H

#include "db/Database.h"
#include "menu/MenuCategory.h"
#include "menu/MenuItem.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
    class MenuRepository {
    public:
        explicit MenuRepository(Database& database);

        //Categories
        bool createCategory(const std::string& name, int cafeId);
        std::optional<Category> findCategoryById(int id);
        std::vector<Category> findAllCategories();
        bool deleteCategory(int id);

        //Menu items
        bool createItem(const std::string& name,
                        const std::string& description,
                        double price,
                        int categoryId);
        std::optional<Item> findItemById(int id);
        std::vector<Item> findAllItems();
        std::vector<Item> findItemsByCategory(int categoryId);
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
