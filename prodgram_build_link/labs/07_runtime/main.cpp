#include <cstdio>
#include <cstdlib>
struct Trace {
    const char *name;
    explicit Trace(const char *n) : name(n) { std::printf("construct %s\n", name); }
    ~Trace() { std::printf("destroy %s\n", name); }
};
Trace global("global");
void on_exit_callback() { std::puts("atexit callback"); }
int main(int argc, char **argv) {
    std::puts("enter main");
    std::atexit(on_exit_callback);
    Trace local("local");
    if (argc > 1 && argv[1][0] == 'e') std::exit(0);
    if (argc > 1 && argv[1][0] == 'q') {
        std::fflush(stdout);
        std::_Exit(0);
    }
    std::puts("return from main");
    return 0;
}
