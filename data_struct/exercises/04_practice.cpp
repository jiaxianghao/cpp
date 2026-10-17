// 练习 04：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
// 非递减数组中第一个 > target 的下标，不存在返回 size()。
std::size_t upper_bound_index(const std::vector<int>& /*a*/, int /*target*/) {
    throw std::logic_error("TODO: implement upper_bound_index");
}
int main() {
    check(upper_bound_index({}, 1) == 0, "empty");
    check(upper_bound_index({1,2,2,4}, 2) == 3, "duplicates");
    check(upper_bound_index({1,2}, 0) == 0, "before all");
    check(upper_bound_index({1,2}, 2) == 2, "after all");
    check(upper_bound_index({2,2}, 1) == 0, "all greater");
    std::cout << "practice_04 passed\n";
}
