#include "thread_pool.hpp"

#include <iostream>
#include <numeric>
#include <vector>

int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 1; i <= 8; ++i) {
        results.emplace_back(pool.enqueue([i] {
            return i * i;
        }));
    }

    int sum = 0;
    for (auto& result : results) {
        sum += result.get();
    }

    std::cout << "sum of squares: " << sum << '\n';
    return 0;
}
