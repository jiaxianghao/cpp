#include <cstddef>
#include <iostream>
// 用不同类型展示旧、新布局，不真的跨版本解引用，避免未定义行为。
struct VehicleV1 { int speed; int gear; };
struct VehicleV2 { int speed; int mode; int gear; };
int main() {
    std::cout << "v1: sizeof=" << sizeof(VehicleV1)
              << " gear offset=" << offsetof(VehicleV1, gear) << '\n';
    std::cout << "v2: sizeof=" << sizeof(VehicleV2)
              << " gear offset=" << offsetof(VehicleV2, gear) << '\n';
}
