#include "check.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
struct Part {
    inline static int alive = 0;
    Part() { ++alive; }
    ~Part() { --alive; }
};
struct Incomplete {
    inline static bool destructor_called = false;
    Part part;
    Incomplete() { throw std::runtime_error("construction failed"); }
    ~Incomplete() { destructor_called = true; }
};
struct Account {
    std::vector<int> entries{10};
    void append_basic(int value, bool fail) {
        entries.push_back(value);
        if (fail) throw std::runtime_error("validation failed");
    }
    void append_strong(int value, bool fail) {
        auto next = entries;
        next.push_back(value);
        if (fail) throw std::runtime_error("validation failed");
        entries.swap(next); // std::allocator 下提交不抛异常。
    }
};
int main() {
    try { Incomplete object; } catch (const std::runtime_error&) {}
    CHECK(Part::alive == 0 && !Incomplete::destructor_called);
    Account account;
    try { account.append_basic(20, true); } catch (const std::runtime_error&) {}
    CHECK((account.entries == std::vector<int>{10, 20}));
    const auto before = account.entries;
    try { account.append_strong(30, true); } catch (const std::runtime_error&) {}
    CHECK(account.entries == before);
    account.append_strong(30, false);
    CHECK(account.entries.back() == 30);
    std::cout << "partial object cleaned; basic changed state; strong rolled back\n";
}
