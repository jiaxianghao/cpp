#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>
int main() {
    std::vector<int> numbers{1, 2, 3, 4, 5, 6};
    const auto logical_end = std::remove_if(numbers.begin(), numbers.end(),
                                          [](int n) { return n % 2 == 0; });
    CHECK(numbers.size() == 6); // remove 不改变容器 size。
    CHECK(std::distance(numbers.begin(), logical_end) == 3);
    numbers.erase(logical_end, numbers.end());
    CHECK((numbers == std::vector<int>{1, 3, 5}));
    CHECK(std::accumulate(numbers.begin(), numbers.end(), 0) == 9);
    std::vector<std::string> names{"alice"};
    const std::size_t index = 0;
    names.reserve(names.capacity() + 1); // 旧指针/引用/迭代器已失效。
    CHECK(names[index] == "alice"); // 通过稳定索引重新访问，前提是没有改变元素位置。
    int threshold = 3;
    auto snapshot = [threshold](int n) { return n > threshold; };
    auto live = [&threshold](int n) { return n > threshold; };
    threshold = 10;
    CHECK(snapshot(5) && !live(5));
    std::cout << "erase-remove: 1 3 5; sum=9; captures differ\n";
}
