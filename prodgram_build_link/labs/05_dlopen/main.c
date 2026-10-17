#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s ./path/to/plugin.so\n", argv[0]);
        return EXIT_FAILURE;
    }
    void *handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!handle) { fprintf(stderr, "%s\n", dlerror()); return EXIT_FAILURE; }
    dlerror();
    /* POSIX dlsym function-pointer conversion; this is a Linux/POSIX lab. */
    int (*plugin_value)(void) = (int (*)(void))dlsym(handle, "plugin_value");
    const char *error = dlerror();
    if (error) {
        fprintf(stderr, "%s\n", error);
        dlclose(handle);
        return EXIT_FAILURE;
    }
    printf("plugin=%d\n", plugin_value());
    dlclose(handle);
    return EXIT_SUCCESS;
}
