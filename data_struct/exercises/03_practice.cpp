// 练习 03：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <string>
// 按字节计算最长不重复连续子串长度。
std::size_t longest_unique(const std::string& /*s*/) {
    throw std::logic_error("TODO: implement longest_unique");
}
int main() {
    check(longest_unique("") == 0, "empty");
    check(longest_unique("abba") == 2, "left cannot retreat");
    check(longest_unique("pwwkew") == 3, "substring not subsequence");
    check(longest_unique("aaaa") == 1, "repeated");
    check(longest_unique("abcdef") == 6, "all distinct");
    std::cout << "practice_03 passed\n";
}
