#include "check.hpp"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

std::vector<int> prefix_function(const std::string& pattern) {
    std::vector<int> pi(pattern.size(), 0);
    for (std::size_t i = 1; i < pattern.size(); ++i) {
        int j = pi[i - 1];
        while (j > 0 && pattern[i] != pattern[j]) j = pi[j - 1];
        if (pattern[i] == pattern[j]) ++j;
        pi[i] = j;
    }
    return pi;
}
// 返回首次匹配下标；找不到返回 npos；空模式匹配位置 0。
std::size_t kmp_find(const std::string& text, const std::string& pattern) {
    if (pattern.empty()) return 0;
    auto pi = prefix_function(pattern);
    int j = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        while (j > 0 && text[i] != pattern[j]) j = pi[j - 1];
        if (text[i] == pattern[j]) ++j;
        if (static_cast<std::size_t>(j) == pattern.size()) return i + 1 - pattern.size();
    }
    return std::string::npos;
}
// 外部下标 0-based；内部 1-based。sum(end) 返回 [0,end) 的和。
class Fenwick {
    std::vector<long long> bit;
public:
    explicit Fenwick(int n) : bit(n + 1, 0) {}
    void add(int index, long long delta) {
        for (int i = index + 1; i < static_cast<int>(bit.size()); i += i & -i) bit[i] += delta;
    }
    long long sum(int end) const {
        long long result = 0;
        for (int i = end; i > 0; i -= i & -i) result += bit[i];
        return result;
    }
};
int main() {
    std::vector<std::string> texts{"", "a", "aaaaa", "ababaca", "hello world"};
    std::vector<std::string> patterns{"", "a", "aaa", "abaca", "world", "missing", "aaaaaa"};
    for (const auto& text : texts) for (const auto& pattern : patterns)
        check(kmp_find(text, pattern) == text.find(pattern), "KMP vs string::find");
    check(prefix_function("ababaca") == std::vector<int>({0, 0, 1, 2, 3, 0, 1}), "prefix fallback");
    Fenwick tree(5);
    std::vector<long long> a(5, 0);
    for (int step = 0; step < 30; ++step) {
        int index = step % 5, delta = step % 7 - 3;
        a[index] += delta; tree.add(index, delta);
        for (int l = 0; l <= 5; ++l) for (int r = l; r <= 5; ++r)
            check(tree.sum(r) - tree.sum(l) == std::accumulate(a.begin() + l, a.begin() + r, 0LL), "Fenwick vs array");
    }
    std::cout << "KMP match abaca in ababaca = " << kmp_find("ababaca", "abaca")
              << "; dynamic range sums verified\n";
}
