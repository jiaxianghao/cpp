#include <vector>
#include <unistd.h>

namespace {
template <std::size_t N>
void trace(const char (&message)[N])
{
    (void)::write(STDOUT_FILENO, message, N - 1);
}

struct Item {
    int value = 42;
    Item() { trace("  Item constructor\n"); }
    ~Item() { trace("  Item destructor\n"); }
};
}

int main()
{
    trace("--- new / delete ---\n");
    Item* item = new Item;
    bool valid = item->value == 42;
    delete item;

    trace("--- new[] / delete[] ---\n");
    Item* items = new Item[2];
    valid = valid && items[0].value == 42 && items[1].value == 42;
    delete[] items;

    trace("--- std::vector with its default allocator ---\n");
    {
        std::vector<int> values;
        values.reserve(3);
        values.push_back(10);
        values.push_back(20);
        values.push_back(30);
        valid = valid && values.size() == 3 && values[0] == 10 &&
                values[1] == 20 && values[2] == 30;
    }
    if (!valid)
        return 1;
    trace("All value checks passed.\n");
    return 0;
}
