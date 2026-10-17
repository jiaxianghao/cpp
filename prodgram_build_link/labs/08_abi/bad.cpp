// Missing extern "C": asks the linker for a C++-mangled symbol.
int c_add(int, int);
int main() { return c_add(2, 3); }
