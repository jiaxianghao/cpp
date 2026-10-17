// 练习 02：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
// 输入非递减；返回逻辑长度 k，将去重结果存于 a[0..k)。不必 resize。
std::size_t deduplicate(std::vector<int>& /*a*/) {
    throw std::logic_error("TODO: implement deduplicate");
}
int main() {
    std::vector<int> a{1,1,2,2,3};
    auto k = deduplicate(a);
    check(k == 3 && a[0] == 1 && a[1] == 2 && a[2] == 3, "sorted duplicates");
    std::vector<int> empty, same{2,2,2}, one{9};
    check(deduplicate(empty) == 0, "empty");
    check(deduplicate(same) == 1 && same[0] == 2, "all equal");
    check(deduplicate(one) == 1 && one[0] == 9, "singleton");
    std::cout << "practice_02 passed\n";
}
