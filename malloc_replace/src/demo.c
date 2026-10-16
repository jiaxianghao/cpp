#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *symbols[] = {"malloc", "free", "calloc", "realloc"};
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); ++i) {
        Dl_info info;
        void *symbol = dlsym(RTLD_DEFAULT, symbols[i]);
        assert(symbol && dladdr(symbol, &info));
        printf("%-7s -> %s\n", symbols[i], info.dli_fname);
    }

    char *message = malloc(32);
    assert(message);
    strcpy(message, "Hello, allocator!");

    char *larger = realloc(message, 128);
    assert(larger);
    assert(strcmp(larger, "Hello, allocator!") == 0);
    puts(larger);
    free(larger);

    int *numbers = calloc(8, sizeof(*numbers));
    assert(numbers);
    for (int i = 0; i < 8; ++i)
        assert(numbers[i] == 0);
    free(numbers);
    puts("malloc/realloc/calloc/free checks passed.");
    return 0;
}
