// 练习 12：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <string>
// 用 KMP 返回第一次匹配位置；不存在为 npos；空模式返回 0。
std::size_t kmp_find(const std::string& /*text*/, const std::string& /*pattern*/) {
    throw std::logic_error("TODO: implement kmp_find");
}
int main() {
    check(kmp_find("ababaca", "abaca") == 2, "fallback");
    check(kmp_find("aaaaa", "aaa") == 0, "repeated");
    check(kmp_find("abc", "abcd") == std::string::npos, "longer pattern");
    check(kmp_find("abc", "d") == std::string::npos, "missing");
    check(kmp_find("", "") == 0 && kmp_find("a", "") == 0, "empty pattern");
    std::cout << "practice_12 passed\n";
}
