// 练习 08：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
struct Edge { int u, v, weight; };
struct Forest { long long weight; int components; };
// 无向图，可不连通、可含负权、自环和重边；返回最小生成森林总权重与分量数。
Forest kruskal(int /*n*/, std::vector<Edge> /*edges*/) {
    throw std::logic_error("TODO: implement kruskal");
}
int main() {
    auto a = kruskal(4, {{0,1,1},{1,2,2},{0,2,5},{2,3,3},{0,3,10}});
    check(a.weight == 6 && a.components == 1, "connected");
    auto b = kruskal(4, {{0,1,-2},{0,1,8},{2,2,-9}});
    check(b.weight == -2 && b.components == 3, "negative parallel self-loop disconnected");
    auto c = kruskal(0, {});
    check(c.weight == 0 && c.components == 0, "empty graph");
    std::cout << "practice_08 passed\n";
}
