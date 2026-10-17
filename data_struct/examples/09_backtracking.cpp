#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

void dfs(const std::vector<int>& a, std::vector<bool>& used, std::vector<int>& path,
         std::vector<std::vector<int>>& answer) {
    if (path.size() == a.size()) { answer.push_back(path); return; }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (used[i]) continue;
        // 相等元素在同一层只选第一个可用者，避免重复排列。
        if (i > 0 && a[i] == a[i - 1] && !used[i - 1]) continue;
        used[i] = true; path.push_back(a[i]);
        dfs(a, used, path, answer);
        path.pop_back(); used[i] = false;
    }
}
std::vector<std::vector<int>> unique_permutations(std::vector<int> a) {
    std::sort(a.begin(), a.end());
    std::vector<bool> used(a.size(), false);
    std::vector<int> path;
    std::vector<std::vector<int>> answer;
    dfs(a, used, path, answer);
    return answer;
}
int main() {
    for (std::vector<int> a : {std::vector<int>{}, {1}, {1, 1, 2}, {1, 2, 3, 3}}) {
        std::sort(a.begin(), a.end());
        std::set<std::vector<int>> expected;
        do { expected.insert(a); } while (std::next_permutation(a.begin(), a.end()));
        auto result = unique_permutations(a);
        check(result.size() == expected.size(), "no duplicates");
        check(std::set<std::vector<int>>(result.begin(), result.end()) == expected, "all permutations");
    }
    std::cout << "unique permutations of [1,1,2]:\n";
    for (const auto& row : unique_permutations({1, 1, 2})) {
        for (int x : row) std::cout << x << ' ';
        std::cout << '\n';
    }
}
