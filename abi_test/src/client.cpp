#include "core.h"
#include <iostream>
int main() {
    const int cmd = latest_command();
    std::cout << "calculate(10)=" << calculate(10)
              << " CMD_NUM=" << CMD_NUM
              << " cmd=" << cmd
              << " name=" << command_name(cmd)
              << " accepted=" << valid(cmd) << '\n';
}
