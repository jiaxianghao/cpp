// 练习 10：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
using Interval = std::pair<int,int>;
// 合并闭区间 [l,r]，l <= r；相接也合并；输出按起点升序。
std::vector<Interval> merge_intervals(std::vector<Interval> /*intervals*/) {
    throw std::logic_error("TODO: implement merge_intervals");
}
int main() {
    check(merge_intervals({{8,10},{1,3},{2,6},{15,18}}) == std::vector<Interval>({{1,6},{8,10},{15,18}}), "unsorted overlap");
    check(merge_intervals({{1,4},{2,3},{4,5}}) == std::vector<Interval>({{1,5}}), "contained and touching");
    check(merge_intervals({}).empty(), "empty");
    check(merge_intervals({{-2,-2},{-2,0}}) == std::vector<Interval>({{-2,0}}), "point interval");
    std::cout << "practice_10 passed\n";
}
