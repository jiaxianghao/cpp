// 练习 09：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <algorithm>
#include <vector>
// 输入元素互异，输出所有子集；答案顺序不限，各子集内顺序不限。
std::vector<std::vector<int>> subsets(const std::vector<int>& /*a*/) {
    throw std::logic_error("TODO: implement subsets");
}
void canonicalize(std::vector<std::vector<int>>& a) {
    for (auto& row : a) std::sort(row.begin(), row.end());
    std::sort(a.begin(), a.end());
}
int main() {
    auto result = subsets({1,2,3});
    std::vector<std::vector<int>> expected{{},{1},{2},{3},{1,2},{1,3},{2,3},{1,2,3}};
    canonicalize(result); canonicalize(expected);
    check(result == expected, "all subsets exactly once");
    check(subsets({}) == std::vector<std::vector<int>>({{}}), "one empty subset");
    std::cout << "practice_09 passed\n";
}
