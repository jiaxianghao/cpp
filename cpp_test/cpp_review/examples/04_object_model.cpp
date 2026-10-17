#include "check.hpp"
#include <iostream>
#include <memory>
#include <string>
struct Plain { char c; int n; };
struct Base {
    std::string during_construction;
    Base() : during_construction(kind()) {}
    virtual std::string kind() const { return "Base"; }
    virtual ~Base() = default;
};
struct Derived : Base {
    int payload = 42;
    std::string kind() const override { return "Derived"; }
};
std::string by_value(Base b) { return b.kind(); }
std::string by_reference(const Base& b) { return b.kind(); }
int main() {
    Derived d;
    CHECK(d.during_construction == "Base");
    CHECK(by_value(d) == "Base");
    CHECK(by_reference(d) == "Derived");
    std::unique_ptr<Base> p = std::make_unique<Derived>();
    CHECK(p->kind() == "Derived");
    std::cout << "sizeof Plain=" << sizeof(Plain) << ", alignof Plain=" << alignof(Plain)
              << ", sizeof Base=" << sizeof(Base) << ", sizeof Derived=" << sizeof(Derived) << '\n';
    std::cout << "construction=Base; sliced=Base; reference=Derived\n";
    // 不读取所谓 vptr，不猜测虚表地址，不把布局当成标准规定。
}
