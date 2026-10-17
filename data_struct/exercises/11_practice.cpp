// 练习 11：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
// 面值为正整数，可重复使用；amount >= 0，不可达返回 -1。
int min_coins(const std::vector<int>& /*coins*/, int /*amount*/) {
    throw std::logic_error("TODO: implement min_coins");
}
int main() {
    check(min_coins({1,3,4}, 6) == 2, "greedy counterexample");
    check(min_coins({2}, 3) == -1, "unreachable");
    check(min_coins({}, 0) == 0 && min_coins({}, 7) == -1, "empty coins");
    check(min_coins({2,2,5}, 4) == 2, "duplicate denominations");
    check(min_coins({7}, 0) == 0, "zero amount");
    std::cout << "practice_11 passed\n";
}
