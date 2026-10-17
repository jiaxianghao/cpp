// 练习 07：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
using Graph = std::vector<std::vector<int>>;
// 图非空、source 合法；不可达用 -1 表示。
std::vector<int> bfs_distance(const Graph& /*graph*/, int /*source*/) {
    throw std::logic_error("TODO: implement bfs_distance");
}
int main() {
    check(bfs_distance({{1,2},{3},{3},{},{}}, 0) == std::vector<int>({0,1,1,2,-1}), "diamond and isolated");
    check(bfs_distance({{1},{0}}, 0) == std::vector<int>({0,1}), "cycle");
    check(bfs_distance({{}}, 0) == std::vector<int>({0}), "single vertex");
    check(bfs_distance({{1},{}}, 1) == std::vector<int>({-1,0}), "directed reachability");
    std::cout << "practice_07 passed\n";
}
