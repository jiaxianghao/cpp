#include "check.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <utility>
struct Trace {
    std::string name;
    std::vector<std::string>& log;
    Trace(std::string n, std::vector<std::string>& l) : name(std::move(n)), log(l) {
        log.push_back("+" + name);
    }
    ~Trace() { log.push_back("-" + name); }
};
struct Pair {
    Trace first; // 成员始终按声明顺序初始化。
    Trace second;
    explicit Pair(std::vector<std::string>& log) : first("first", log), second("second", log) {}
};
int main() {
    std::vector<std::string> log;
    log.reserve(8); // 日志仅用于教学；生产析构函数不应依赖可能失败的日志分配。
    { Pair p(log); }
    CHECK((log == std::vector<std::string>{"+first", "+second", "-second", "-first"}));
    for (const auto& event : log) std::cout << event << '\n';
    int value = 7;
    const int* read_only = &value;
    int* const fixed_pointer = &value;
    *fixed_pointer = 9;
    CHECK(*read_only == 9);
    std::cout << "const view sees " << *read_only << '\n';
}
