#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    const char* greeting = std::getenv("GREETING");
    std::cout << (greeting ? greeting : "Hello") << ", "
              << (argc > 1 ? argv[1] : "Docker") << "!\n";
    return 0;
}
