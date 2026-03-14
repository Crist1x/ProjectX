#ifndef PROJECTX_CART_H
#define PROJECTX_CART_H

#include <map>
#include <vector>
#include <cstdint>
#include <string>

struct CartItem {
    int menuItemId;
    std::string name;
    double price;
    int quantity;
    
    double getTotal() const { return price * quantity; }
};

class Cart {
public:
    Cart() = default;
    
    // Управление корзиной
    void addItem(int menuItemId, const std::string& name, double price, int quantity = 1);
    void removeItem(int menuItemId);
    void updateQuantity(int menuItemId, int quantity);
    void clear();
    
    // Информация
    std::vector<CartItem> getItems() const;
    size_t getItemCount() const;
    double getTotalAmount() const;
    bool isEmpty() const;
    
    // Для checkout
    std::vector<std::pair<int, int>> getItemPairs() const; // (menuItemId, quantity)

private:
    std::map<int, CartItem> items_;
};

#endif