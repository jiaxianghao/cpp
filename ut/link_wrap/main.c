/*
 * Main program that uses the wrapped functions
 * Notice: We don't need to modify this code at all!
 * The linker will automatically redirect calls to our wrapper functions
 */

#include <stdio.h>
#include <unistd.h>

// Function declarations (from original_lib.c)
extern double get_current_time(void);
extern int get_random_number(void);

int main(void)
{
    printf("=== --wrap Example Demo ===\n\n");

    printf("Calling get_current_time():\n");
    double time1 = get_current_time();
    printf("Result: %.0f\n\n", time1);

    sleep(1);

    printf("Calling get_current_time() again:\n");
    double time2 = get_current_time();
    printf("Result: %.0f\n", time2);
    printf("Time difference: %.0f seconds\n\n", time2 - time1);

    printf("Calling get_random_number():\n");
    int rand1 = get_random_number();
    printf("Result: %d\n\n", rand1);

    printf("Calling get_random_number() again:\n");
    int rand2 = get_random_number();
    printf("Result: %d\n\n", rand2);

    printf("=== Demo Complete ===\n");
    printf("\nKey Points:\n");
    printf("1. main.c calls get_current_time() and get_random_number()\n");
    printf("2. Linker redirects these calls to __wrap_* functions\n");
    printf("3. Wrapper functions can call __real_* to access original functions\n");
    printf("4. No code changes needed in main.c!\n");

    return 0;
}

