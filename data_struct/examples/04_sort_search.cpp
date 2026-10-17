#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

// [left,right)；一次分配辅助数组，避免在每层重复分配。
void merge_sort_impl(std::vector<int>& a, std::vector<int>& temp, std::size_t left, std::size_t right) {
    if (right - left <= 1) return;
    auto mid = left + (right - left) / 2;
    merge_sort_impl(a, temp, left, mid);
    merge_sort_impl(a, temp, mid, right);
    auto i = left, j = mid, k = left;
    while (i < mid && j < right) temp[k++] = a[i] <= a[j] ? a[i++] : a[j++];
    while (i < mid) temp[k++] = a[i++];
    while (j < right) temp[k++] = a[j++];
    for (k = left; k < right; ++k) a[k] = temp[k];
}
void merge_sort(std::vector<int>& a) {
    std::vector<int> temp(a.size());
    merge_sort_impl(a, temp, 0, a.size());
}
// 前置条件：a 非递减；返回第一个 >= target 的位置，可为 a.size()。
std::size_t lower_bound_index(const std::vector<int>& a, int target) {
    std::size_t left = 0, right = a.size();
    while (left < right) {
        auto mid = left + (right - left) / 2;
        if (a[mid] < target) left = mid + 1;
        else right = mid;
    }
    return left;
}
int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> value(-10, 10);
    for (int n = 0; n < 100; ++n) {
        std::vector<int> a(n);
        for (int& x : a) x = value(rng);
        auto expected = a;
        std::sort(expected.begin(), expected.end());
        merge_sort(a);
        check(a == expected, "merge sort vs std::sort");
        for (int target = -12; target <= 12; ++target)
            check(lower_bound_index(a, target) == static_cast<std::size_t>(
                std::lower_bound(a.begin(), a.end(), target) - a.begin()), "lower_bound reference");
    }
    std::cout << "lower_bound([1,2,2,4],2) = " << lower_bound_index({1, 2, 2, 4}, 2)
              << "; sort and boundary checks passed\n";
}
