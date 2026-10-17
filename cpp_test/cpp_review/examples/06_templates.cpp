#include "check.hpp"
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
std::string category(int&) { return "lvalue"; }
std::string category(int&&) { return "rvalue"; }
template<class T> std::string forward_category(T&& value) {
    return category(std::forward<T>(value));
}
template<class T> std::string lose_category(T&& value) {
    return category(value); // 有名字的变量表达式是左值。
}
template<class T> auto twice(T value) { return value + value; }
int main() {
    int n = 3;
    const int& ref = n;
    auto copy = ref;
    auto& alias = ref;
    static_assert(std::is_same_v<decltype(copy), int>);
    static_assert(std::is_same_v<decltype(alias), const int&>);
    static_assert(std::is_same_v<decltype((n)), int&>);
    CHECK(forward_category(n) == "lvalue");
    CHECK(forward_category(3) == "rvalue");
    CHECK(lose_category(3) == "lvalue");
    CHECK(twice(3) == 6);
    CHECK(twice(std::string("ab")) == "abab");
    std::cout << "forward: lvalue/rvalue; named parameter: lvalue\n";
}
