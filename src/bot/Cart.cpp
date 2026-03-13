#include "bot/Cart.h"

void Cart::addItem(int menuItemId, const std::string& name, double price, int quantity) {
    if (items_.count(menuItemId)) {
        items_[menuItemId].quantity += quantity;
    } else {
        items_[menuItemId] = {menuItemId, name, price, quantity};
    }
}

void Cart::removeItem(int menuItemId) {
    items_.erase(menuItemId);
}

void Cart::updateQuantity(int menuItemId, int quantity) {
    if (quantity <= 0) {
        removeItem(menuItemId);
    } else if (items_.count(menuItemId)) {
        items_[menuItemId].quantity = quantity;
    }
}

void Cart::clear() {
    items_.clear();
}

std::vector<CartItem> Cart::getItems() const {
    std::vector<CartItem> result;
    for (const auto& [id, item] : items_) {
        result.push_back(item);
    }
    return result;
}

size_t Cart::getItemCount() const {
    size_t count = 0;
    for (const auto& [id, item] : items_) {
        count += item.quantity;
    }
    return count;
}

double Cart::getTotalAmount() const {
    double total = 0.0;
    for (const auto& [id, item] : items_) {
        total += item.getTotal();
    }
    return total;
}

bool Cart::isEmpty() const {
    return items_.empty();
}

std::vector<std::pair<int, int>> Cart::getItemPairs() const {
    std::vector<std::pair<int, int>> result;
    for (const auto& [id, item] : items_) {
        result.emplace_back(item.menuItemId, item.quantity);
    }
    return result;
}