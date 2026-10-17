#include "api.h"
#include <cstdio>
int cpp_add(int a, int b) { return a + b; }
int main() { std::printf("c=%d cpp=%d\n", c_add(2, 3), cpp_add(4, 5)); }
