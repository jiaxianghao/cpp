#pragma once
#include <stdexcept>
#include <string>
// 不使用 assert：Release 构建中仍执行检查。
inline void check(bool condition, const char* expression, int line) {
    if (!condition) throw std::runtime_error(std::string("CHECK failed: ") + expression
                                           + " at line " + std::to_string(line));
}
#define CHECK(expression) check(static_cast<bool>(expression), #expression, __LINE__)
