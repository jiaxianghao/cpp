#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
struct Value {
    inline static int copies = 0, moves = 0;
    int number;
    explicit Value(int n) : number(n) {}
    Value(const Value& other) : number(other.number) { ++copies; }
    Value(Value&& other) noexcept : number(other.number) { other.number = -1; ++moves; }
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;
};
int main() {
    Value source(42);
    auto&& reference = std::move(source);
    CHECK(source.number == 42 && Value::moves == 0);
    Value moved(std::move(reference));
    CHECK(moved.number == 42 && source.number == -1);
    const Value immutable(7);
    Value copied(std::move(immutable)); // const Value&& 不能绑定到 Value&&，选择复制。
    CHECK(copied.number == 7 && Value::copies == 1);
    std::vector<Value> values;
    values.reserve(1);
    values.emplace_back(1);
    Value::copies = Value::moves = 0;
    values.reserve(values.capacity() + 1); // 明确触发重分配。
    CHECK(values.front().number == 1);
    std::cout << "vector relocation: copies=" << Value::copies
              << ", moves=" << Value::moves << '\n';
    // 计数是实现观察，不把某一种重定位策略写成跨实现断言。
}
