#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

// 前缀和定义：prefix[i] 是原数组前 i 个元素的和。
std::vector<long long> prefix_sum(const std::vector<int>& a) {
    std::vector<long long> prefix(a.size() + 1, 0);
    for (std::size_t i = 0; i < a.size(); ++i) prefix[i + 1] = prefix[i] + a[i];
    return prefix;
}
// 非空数组的最大子数组和；循环后 best 是已扫描前缀中的最优答案。
long long max_subarray(const std::vector<int>& a) {
    if (a.empty()) throw std::invalid_argument("requires a nonempty array");
    long long ending = a[0], best = a[0];
    for (std::size_t i = 1; i < a.size(); ++i) {
        ending = std::max<long long>(a[i], ending + a[i]);
        best = std::max(best, ending);
    }
    return best;
}
long long brute_max(const std::vector<int>& a) {
    long long best = a[0];
    for (std::size_t l = 0; l < a.size(); ++l) {
        long long sum = 0;
        for (std::size_t r = l; r < a.size(); ++r) {
            sum += a[r];
            best = std::max(best, sum);
        }
    }
    return best;
}
int main() {
    check(prefix_sum({}) == std::vector<long long>{0}, "empty prefix");
    auto p = prefix_sum({2, -1, 3, 4});
    check(p[4] - p[1] == 6, "range [1,4)");
    check(max_subarray({-4, -2, -7}) == -2, "all negative");
    // 枚举小输入，对照独立的暴力实现。
    for (int code = 0; code < 625; ++code) {
        int x = code;
        std::vector<int> a(4);
        for (int& v : a) { v = x % 5 - 2; x /= 5; }
        check(max_subarray(a) == brute_max(a), "max subarray vs brute force");
    }
    std::cout << "[1,4) sum = " << p[4] - p[1] << "; max subarray = "
              << max_subarray({-2, 1, -3, 4, -1, 2, 1, -5, 4}) << '\n';
}
