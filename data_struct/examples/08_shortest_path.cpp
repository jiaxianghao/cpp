#include "check.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using Edge = std::pair<int, int>; // (to, weight)
using Graph = std::vector<std::vector<Edge>>;
constexpr long long INF = std::numeric_limits<long long>::max() / 4;
// 前置条件：非负 int 权值、合法顶点、路径长度 < INF。
std::vector<long long> dijkstra(const Graph& graph, int source) {
    using State = std::pair<long long, int>; // (distance, vertex)
    std::priority_queue<State, std::vector<State>, std::greater<State>> heap;
    std::vector<long long> distance(graph.size(), INF);
    distance[source] = 0; heap.push({0, source});
    while (!heap.empty()) {
        auto [d, u] = heap.top(); heap.pop();
        if (d != distance[u]) continue; // 同一顶点可能有旧候选项。
        for (auto [v, weight] : graph[u]) {
            if (d + weight < distance[v]) {
                distance[v] = d + weight;
                heap.push({distance[v], v});
            }
        }
    }
    return distance;
}
// 独立参考算法：本示例仅用非负权图验证，未提供负环检测。
std::vector<long long> bellman_reference(const Graph& graph, int source) {
    std::vector<long long> d(graph.size(), INF);
    d[source] = 0;
    for (std::size_t round = 1; round < graph.size(); ++round)
        for (std::size_t u = 0; u < graph.size(); ++u) if (d[u] != INF)
            for (auto [v, w] : graph[u]) if (d[u] + w < d[v]) d[v] = d[u] + w;
    return d;
}
int main() {
    Graph graph{{{1, 10}, {2, 1}}, {{3, 1}}, {{1, 2}, {3, 8}}, {}, {}};
    check(dijkstra(graph, 0) == std::vector<long long>({0, 3, 1, 4, INF}), "stale entry and unreachable");
    check(dijkstra(Graph{{{1, 0}}, {}}, 0)[1] == 0, "zero weight");
    // 枚举三顶点有向图：每条非自环边可缺失、权重 0 或权重 2。
    for (int code = 0; code < 729; ++code) {
        Graph g(3); int x = code;
        for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) if (u != v) {
            int state = x % 3; x /= 3;
            if (state) g[u].push_back({v, state == 1 ? 0 : 2});
        }
        check(dijkstra(g, 0) == bellman_reference(g, 0), "Dijkstra vs Bellman reference");
    }
    std::cout << "distances: 0 3 1 4 INF; 729 small graphs verified\n";
}
