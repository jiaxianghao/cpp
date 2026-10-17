#include "check.hpp"
#include <iostream>
#include <queue>
#include <vector>

using Graph = std::vector<std::vector<int>>;
// 顶点编号为 [0,n)，source 合法。入队时标记，避免重复入队。
std::vector<int> bfs_distance(const Graph& graph, int source) {
    std::vector<int> distance(graph.size(), -1);
    std::queue<int> q;
    distance[source] = 0; q.push(source);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : graph[u]) if (distance[v] == -1) {
            distance[v] = distance[u] + 1;
            q.push(v);
        }
    }
    return distance;
}
struct Topology { bool acyclic; std::vector<int> order; };
Topology topological_sort(const Graph& graph) {
    std::vector<int> indegree(graph.size(), 0), order;
    for (const auto& edges : graph) for (int v : edges) ++indegree[v];
    std::queue<int> q;
    for (std::size_t u = 0; u < graph.size(); ++u) if (indegree[u] == 0) q.push(static_cast<int>(u));
    while (!q.empty()) {
        int u = q.front(); q.pop(); order.push_back(u);
        for (int v : graph[u]) if (--indegree[v] == 0) q.push(v);
    }
    return {order.size() == graph.size(), order};
}
int main() {
    Graph graph{{1, 2}, {3}, {3}, {}, {}};
    check(bfs_distance(graph, 0) == std::vector<int>({0, 1, 1, 2, -1}), "BFS unreachable");
    auto result = topological_sort(graph);
    check(result.acyclic, "DAG");
    std::vector<int> position(graph.size());
    for (std::size_t i = 0; i < result.order.size(); ++i) position[result.order[i]] = static_cast<int>(i);
    for (std::size_t u = 0; u < graph.size(); ++u)
        for (int v : graph[u]) check(position[u] < position[v], "edge order");
    check(!topological_sort(Graph{{1}, {0}}).acyclic, "cycle");
    check(topological_sort(Graph{}).acyclic, "empty DAG");
    std::cout << "BFS distances: 0 1 1 2 -1; topological constraints verified\n";
}
