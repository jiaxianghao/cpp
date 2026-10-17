#include "check.hpp"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
// 练习域：金额单位为分；非负；所有中间计算均在 int 范围内。
struct Item { int price; int quantity; };
using Discount = std::function<int(int)>;
int legacy_quote(const std::vector<Item>& items, bool member) {
    int total = 0;
    for (const auto& item : items) total += item.price * item.quantity;
    if (member) total = total * 9 / 10;
    return total < 10000 ? total + 800 : total;
}
int subtotal(const std::vector<Item>& items) {
    int result = 0;
    for (const auto& item : items) result += item.price * item.quantity;
    return result;
}
int quote(const std::vector<Item>& items, const Discount& discount) {
    const int discounted = discount(subtotal(items));
    return discounted + (discounted < 10000 ? 800 : 0);
}
// 自有 vector 管理资源；无需手写析构、复制和移动（Rule of Zero）。
class Checkout {
    std::vector<int> receipts_;
public:
    void submit(const std::vector<Item>& items, const Discount& discount) {
        const int amount = quote(items, discount); // 策略失败时尚未修改内部状态。
        receipts_.push_back(amount); // vector<int> 插入失败保持原状。
    }
    const std::vector<int>& receipts() const { return receipts_; }
};
int main() {
    const Discount regular = [](int n) { return n; };
    const Discount member = [](int n) { return n * 9 / 10; };
    const std::vector<Item> cart{{2500, 2}, {5000, 1}};
    CHECK(quote(cart, regular) == 10000);
    CHECK(quote(cart, member) == 9800);
    for (int price = 0; price <= 12000; price += 13) {
        const std::vector<Item> sample{{price, 1}};
        CHECK(quote(sample, regular) == legacy_quote(sample, false));
        CHECK(quote(sample, member) == legacy_quote(sample, true));
    }
    Checkout checkout;
    checkout.submit(cart, regular);
    const auto before = checkout.receipts();
    bool caught = false;
    try {
        checkout.submit(cart, [](int) -> int { throw std::runtime_error("discount unavailable"); });
    } catch (const std::runtime_error&) { caught = true; }
    CHECK(caught && checkout.receipts() == before);
    checkout.submit(cart, member);
    CHECK((checkout.receipts() == std::vector<int>{10000, 9800}));
    std::cout << "regular=10000, member=9800; failed checkout leaves receipts unchanged\n";
}
