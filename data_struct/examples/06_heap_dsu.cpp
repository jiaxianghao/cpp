#include "check.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <vector>

// k <= 输入长度；返回最大的 k 个元素，升序排列，保留重复值。
std::vector<int> top_k(const std::vector<int>& a, std::size_t k) {
    std::priority_queue<int, std::vector<int>, std::greater<int>> heap;
    for (int x : a) {
        heap.push(x);
        if (heap.size() > k) heap.pop();
    }
    std::vector<int> result;
    while (!heap.empty()) { result.push_back(heap.top()); heap.pop(); }
    return result;
}
class DSU {
    std::vector<int> parent, size;
public:
    explicit DSU(int n) : parent(n), size(n, 1) { std::iota(parent.begin(), parent.end(), 0); }
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) std::swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return true;
    }
};
int main() {
    check(top_k({3, 1, 5, 5, 2}, 3) == std::vector<int>({3, 5, 5}), "top k duplicates");
    check(top_k({1, 2}, 0).empty() && top_k({}, 0).empty(), "zero k");
    DSU dsu(5);
    check(dsu.unite(0, 1) && dsu.unite(1, 2), "merge");
    check(!dsu.unite(0, 2) && dsu.find(0) == dsu.find(2), "already connected");
    check(dsu.find(0) != dsu.find(3), "separate component");
    std::cout << "top 3: 3 5 5; DSU connectivity checks passed\n";
}
