#include "check.hpp"
#include <iostream>
#include <vector>
struct Order { int unit_price; int quantity; bool member; };
// 金额以分计；练习域：非负金额、非负数量，乘积不超过 int。
int before(const Order& o) {
    int result = o.unit_price * o.quantity;
    if (o.member) result = result * 9 / 10;
    if (result < 10000) result += 800;
    return result;
}
int discounted_subtotal(const Order& o) {
    const int subtotal = o.unit_price * o.quantity;
    return o.member ? subtotal * 9 / 10 : subtotal;
}
int shipping_fee(int discounted) { return discounted < 10000 ? 800 : 0; }
int after(const Order& o) {
    const int subtotal = discounted_subtotal(o);
    return subtotal + shipping_fee(subtotal);
}
int main() {
    struct Case { Order order; int expected; };
    const std::vector<Case> cases{
        {{0, 0, false}, 800}, {{9999, 1, false}, 10799},
        {{10000, 1, false}, 10000}, {{10000, 1, true}, 9800},
        {{11111, 1, true}, 10799}, {{11112, 1, true}, 10000},
        {{101, 1, true}, 890}, {{2500, 4, false}, 10000}
    };
    for (const auto& c : cases) {
        CHECK(before(c.order) == c.expected);
        CHECK(after(c.order) == c.expected);
    }
    // 宽一些的差分检查：不能代替上面的独立期望值。
    for (int price = 0; price <= 12000; price += 17)
        for (int quantity = 0; quantity <= 5; ++quantity)
            for (bool member : {false, true})
                CHECK(before({price, quantity, member}) == after({price, quantity, member}));
    std::cout << "refactoring preserves discount rounding and shipping boundary\n";
}
