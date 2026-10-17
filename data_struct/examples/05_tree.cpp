#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>

struct Node {
    int key;
    std::unique_ptr<Node> left, right;
    explicit Node(int value) : key(value) {}
};
// 本例集合语义：重复键忽略。普通 BST 不保证平衡。
void insert(std::unique_ptr<Node>& root, int key) {
    if (!root) root = std::make_unique<Node>(key);
    else if (key < root->key) insert(root->left, key);
    else if (key > root->key) insert(root->right, key);
}
void inorder(const Node* root, std::vector<int>& out) {
    if (!root) return;
    inorder(root->left.get(), out);
    out.push_back(root->key);
    inorder(root->right.get(), out);
}
int height(const Node* root) {
    if (!root) return 0; // 按节点数计高：叶子为 1。
    return 1 + std::max(height(root->left.get()), height(root->right.get()));
}
std::vector<int> level_order(const Node* root) {
    std::vector<int> out;
    if (!root) return out;
    std::queue<const Node*> q;
    q.push(root);
    while (!q.empty()) {
        auto current = q.front(); q.pop();
        out.push_back(current->key);
        if (current->left) q.push(current->left.get());
        if (current->right) q.push(current->right.get());
    }
    return out;
}
int main() {
    std::unique_ptr<Node> root;
    check(height(root.get()) == 0 && level_order(root.get()).empty(), "empty tree");
    for (int x : {4, 2, 6, 1, 3, 5, 7, 4}) insert(root, x);
    std::vector<int> order;
    inorder(root.get(), order);
    check(order == std::vector<int>({1, 2, 3, 4, 5, 6, 7}), "BST inorder and duplicate policy");
    check(height(root.get()) == 3, "height");
    check(level_order(root.get()) == std::vector<int>({4, 2, 6, 1, 3, 5, 7}), "BFS");
    std::unique_ptr<Node> chain;
    for (int x : {1, 2, 3, 4}) insert(chain, x);
    check(height(chain.get()) == 4, "unbalanced BST");
    std::cout << "inorder:";
    for (int x : order) std::cout << ' ' << x;
    std::cout << "; height = " << height(root.get()) << '\n';
}
