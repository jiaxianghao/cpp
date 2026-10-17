#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// 字节字符串；UTF-8 的一个汉字可能含多个字节，不等同于字符级去重。
std::size_t longest_unique(const std::string& s) {
    std::unordered_map<char, std::size_t> last;
    std::size_t left = 0, best = 0;
    for (std::size_t right = 0; right < s.size(); ++right) {
        auto it = last.find(s[right]);
        if (it != last.end()) left = std::max(left, it->second + 1);
        last[s[right]] = right;
        best = std::max(best, right - left + 1);
    }
    return best;
}
// 有负数也适用：此前出现过的 prefix - target 都能形成一个区间。
long long subarray_count(const std::vector<int>& a, long long target) {
    std::unordered_map<long long, long long> count{{0, 1}};
    long long prefix = 0, answer = 0;
    for (int x : a) {
        prefix += x;
        auto it = count.find(prefix - target);
        if (it != count.end()) answer += it->second;
        ++count[prefix]; // 查询必须在插入当前前缀之前。
    }
    return answer;
}
long long brute_count(const std::vector<int>& a, long long target) {
    long long count = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        long long sum = 0;
        for (std::size_t j = i; j < a.size(); ++j) { sum += a[j]; count += sum == target; }
    }
    return count;
}
int main() {
    check(longest_unique("") == 0 && longest_unique("abba") == 2, "window never retreats");
    check(longest_unique("abcabcbb") == 3, "unique substring");
    for (int code = 0; code < 243; ++code) {
        int x = code;
        std::vector<int> a(5);
        for (int& v : a) { v = x % 3 - 1; x /= 3; }
        for (int target = -2; target <= 2; ++target)
            check(subarray_count(a, target) == brute_count(a, target), "prefix vs brute force");
    }
    std::cout << "longest unique in abba = " << longest_unique("abba")
              << "; zero-sum subarrays in [1,-1,0] = " << subarray_count({1, -1, 0}, 0) << '\n';
}
