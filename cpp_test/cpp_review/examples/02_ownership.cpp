#include "check.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
struct Resource {
    inline static int alive = 0;
    Resource() { ++alive; }
    ~Resource() { --alive; }
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};
struct Node { std::weak_ptr<Node> parent; };
int main() {
    try {
        auto owner = std::make_unique<Resource>();
        Resource* observer = owner.get(); // 借用，不能 delete，也不能比 owner 活得久。
        CHECK(observer != nullptr);
        auto next_owner = std::move(owner);
        CHECK(!owner && next_owner);
        throw std::runtime_error("simulated failure");
    } catch (const std::runtime_error&) {}
    CHECK(Resource::alive == 0);
    auto parent = std::make_shared<Node>();
    auto child = std::make_shared<Node>();
    child->parent = parent;
    CHECK(!child->parent.expired());
    parent.reset();
    CHECK(child->parent.expired());
    std::cout << "RAII cleaned resource; weak parent expired\n";
}
