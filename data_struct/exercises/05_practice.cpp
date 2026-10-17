// 练习 05：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
struct Node { int value; const Node* left; const Node* right; };
// 按节点数计高度：空树 0，叶子 1。
int height(const Node* /*root*/) {
    throw std::logic_error("TODO: implement height");
}
int main() {
    Node leaf{3,nullptr,nullptr}, middle{2,&leaf,nullptr}, root{1,&middle,nullptr};
    check(height(nullptr) == 0, "empty");
    check(height(&leaf) == 1, "leaf");
    check(height(&root) == 3, "left chain");
    Node right{4,nullptr,&root};
    check(height(&right) == 4, "right child");
    std::cout << "practice_05 passed\n";
}
