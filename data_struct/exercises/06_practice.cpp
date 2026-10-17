// 练习 06：先读 WORKBOOK.md。只实现 TODO，保留并扩充 main 中的检查。
#include "check.hpp"
#include <iostream>
#include <utility>
#include <vector>
// 自行添加 parent/size 字段。n >= 0，find/unite 参数在 [0,n)。
class DSU {
public:
    explicit DSU(int /*n*/) { throw std::logic_error("TODO: initialize DSU"); }
    int find(int /*x*/) { throw std::logic_error("TODO: implement find"); }
    bool unite(int /*a*/, int /*b*/) { throw std::logic_error("TODO: implement unite"); }
};
int main() {
    DSU d(6);
    check(d.find(0) != d.find(1), "initial separate");
    check(d.unite(0,1) && d.unite(1,2), "merge");
    check(d.find(0) == d.find(2), "transitive");
    check(!d.unite(2,0) && !d.unite(3,3), "already connected");
    check(d.unite(4,5) && d.find(0) != d.find(4), "other component");
    std::cout << "practice_06 passed\n";
}
