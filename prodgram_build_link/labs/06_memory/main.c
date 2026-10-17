#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int global_value = 7;
int global_zero;
static const char message[] = "read only";
int main(void) {
    int local = 9;
    void *small = malloc(32);
    void *large = malloc(4 * 1024 * 1024);
    if (!small || !large) { free(small); free(large); return 1; }
    printf("pid=%ld\nglobal=%p zero=%p constant=%p stack=%p\n",
           (long)getpid(), (void *)&global_value, (void *)&global_zero,
           (const void *)message, (void *)&local);
    printf("malloc(32)=%p malloc(4MiB)=%p\n", small, large);
    puts("--- /proc/self/maps ---");
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) { perror("maps"); free(small); free(large); return 1; }
    char line[512];
    while (fgets(line, sizeof line, maps)) fputs(line, stdout);
    fclose(maps);
    free(small);
    free(large);
    return 0;
}
