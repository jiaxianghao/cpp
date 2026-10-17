#include <stdio.h>
extern int shared;
int add(int x);
int main(void) {
    printf("shared=%d add=%d\n", shared, add(5));
    return 0;
}
