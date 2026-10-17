#pragma once
#include <stdexcept>
#include <string>
// 不用 assert：Release 构建也必须执行验证。
inline void check(bool condition, const std::string& description) {
    if (!condition) throw std::runtime_error(description);
}
