#include "check.hpp"
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <vector>

struct Node { int value; Node* next; };
// 节点由调用方持有；这里只改变链接，不分配、不释放。
Node* reverse(Node* head) {
    Node* previous = nullptr;
    while (head) {
        Node* next = head->next; // 必须先保存尚未处理的链。
        head->next = previous;
        previous = head;
        head = next;
    }
    return previous;
}
bool balanced(const std::string& s) {
    std::stack<char> st;
    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') st.push(c);
        else if (c == ')' || c == ']' || c == '}') {
            if (st.empty()) return false;
            char want = c == ')' ? '(' : (c == ']' ? '[' : '{');
            if (st.top() != want) return false;
            st.pop();
        }
    }
    return st.empty();
}
// 单调栈中存下标；栈内元素尚未找到右侧更大值。
std::vector<int> next_greater(const std::vector<int>& a) {
    std::vector<int> result(a.size(), -1);
    std::stack<std::size_t> st;
    for (std::size_t i = 0; i < a.size(); ++i) {
        while (!st.empty() && a[st.top()] < a[i]) {
            result[st.top()] = a[i];
            st.pop();
        }
        st.push(i);
    }
    return result;
}
int main() {
    Node c{3, nullptr}, b{2, &c}, a{1, &b};
    Node* head = reverse(&a);
    check(head == &c && c.next == &b && b.next == &a && a.next == nullptr, "reverse links");
    check(reverse(nullptr) == nullptr, "empty list");
    check(balanced("{a+[b*(c)]}") && !balanced("([)]") && !balanced("]"), "brackets");
    check(next_greater({2, 1, 2, 4, 3}) == std::vector<int>({4, 2, 4, -1, -1}), "next greater");
    check(next_greater({2, 2}) == std::vector<int>({-1, -1}), "equal is not greater");
    std::queue<int> q;
    q.push(10); q.push(20); q.pop();
    check(q.front() == 20, "FIFO");
    std::cout << "reversed: " << c.value << ' ' << b.value << ' ' << a.value
              << "; bracket and monotonic stack checks passed\n";
}
