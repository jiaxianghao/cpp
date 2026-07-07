#include "thread_pool.hpp"

#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <vector>

int add(int lhs, int rhs) {
    return lhs + rhs;
}

struct Multiplier {
    int factor;

    int multiply(int value) const {
        return factor * value;
    }

    int operator()(int value) const {
        return multiply(value);
    }
};

int main() {
    ThreadPool pool(4);

    auto free_function = pool.enqueue(add, 2, 3);

    Multiplier multiplier{4};
    auto member_function = pool.enqueue(&Multiplier::multiply, &multiplier, 5);
    auto function_object = pool.enqueue(multiplier, 6);

    int offset = 10;
    auto lambda = pool.enqueue([offset](int value) {
        return value + offset;
    }, 7);

    std::string message;
    auto void_lambda = pool.enqueue([&message] {
        message = "void task completed";
    });

    void_lambda.get();

    std::vector<std::future<int>> squares;
    for (int i = 1; i <= 4; ++i) {
        squares.emplace_back(pool.enqueue([i] {
            return i * i;
        }));
    }

    int square_sum = 0;
    for (auto& square : squares) {
        square_sum += square.get();
    }

    std::cout << "free function: " << free_function.get() << '\n';
    std::cout << "member function: " << member_function.get() << '\n';
    std::cout << "function object: " << function_object.get() << '\n';
    std::cout << "lambda with argument: " << lambda.get() << '\n';
    std::cout << "void lambda: " << message << '\n';
    std::cout << "sum of squares: " << square_sum << '\n';
    return 0;
}
