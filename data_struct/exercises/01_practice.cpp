// 练习 01：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
// 返回每个合法半开区间 [l,r) 的和。构建一次前缀和。
std::vector<long long> range_sums(const std::vector<int>& /*a*/,
                                const std::vector<std::pair<int,int>>& /*queries*/) {
    throw std::logic_error("TODO: implement range_sums");
}
int main() {
    check(range_sums({2,-1,3,4}, {{1,4},{0,0},{0,4},{2,3}}) == std::vector<long long>({6,0,8,3}), "ranges");
    check(range_sums({}, {{0,0}}) == std::vector<long long>({0}), "empty");
    check(range_sums({2000000000,2000000000}, {{0,2}}) == std::vector<long long>({4000000000LL}), "wide sum");
    std::cout << "practice_01 passed\n";
}
