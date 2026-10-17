#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// capacity >= 0，weight/value 等长，weight > 0；容量上限语义，可以不选。
long long knapsack(const std::vector<int>& weight, const std::vector<int>& value, int capacity) {
    std::vector<long long> dp(capacity + 1, 0);
    for (std::size_t i = 0; i < weight.size(); ++i)
        for (int c = capacity; c >= weight[i]; --c) // 倒序保证每件物品只用一次。
            dp[c] = std::max(dp[c], dp[c - weight[i]] + value[i]);
    return dp[capacity];
}
int lcs(const std::string& a, const std::string& b) {
    std::vector<std::vector<int>> dp(a.size() + 1, std::vector<int>(b.size() + 1, 0));
    for (std::size_t i = 1; i <= a.size(); ++i)
        for (std::size_t j = 1; j <= b.size(); ++j)
            dp[i][j] = a[i - 1] == b[j - 1] ? dp[i - 1][j - 1] + 1 : std::max(dp[i - 1][j], dp[i][j - 1]);
    return dp[a.size()][b.size()];
}
long long brute_knapsack(const std::vector<int>& w, const std::vector<int>& v, int capacity) {
    long long best = 0;
    for (unsigned mask = 0; mask < (1u << w.size()); ++mask) {
        int weight = 0; long long value = 0;
        for (std::size_t i = 0; i < w.size(); ++i) if (mask & (1u << i)) { weight += w[i]; value += v[i]; }
        if (weight <= capacity) best = std::max(best, value);
    }
    return best;
}
int main() {
    std::vector<int> w{1, 3, 4, 5}, v{2, 5, 7, 8};
    for (int capacity = 0; capacity <= 15; ++capacity)
        check(knapsack(w, v, capacity) == brute_knapsack(w, v, capacity), "DP vs subsets");
    check(knapsack({2}, {3}, 4) == 3, "item cannot be reused");
    check(knapsack({}, {}, 5) == 0, "no items");
    check(lcs("abcde", "ace") == 3 && lcs("abc", "def") == 0, "LCS");
    check(lcs("", "a") == 0 && lcs("aaaa", "aa") == 2, "empty/repeated LCS");
    std::cout << "knapsack capacity 7 = " << knapsack(w, v, 7) << "; LCS(abcde,ace) = 3\n";
}
