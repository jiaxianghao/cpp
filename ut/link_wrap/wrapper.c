/*
 * Wrapper functions using --wrap linker option
 * These functions will replace the original functions at link time
 */

#include <stdio.h>

// Declare the original functions (provided by linker)
double __real_get_current_time(void);
int __real_get_random_number(void);

// Wrapper for get_current_time
double __wrap_get_current_time(void)
{
    static int call_count = 0;
    call_count++;

    printf("[WRAPPER] get_current_time() called (count: %d)\n", call_count);

    // In this example, we add 1000 seconds to the real time
    // In real scenarios, you might return mock time for testing/replay
    double real_time = __real_get_current_time();
    double wrapped_time = real_time + 1000.0;

    printf("[WRAPPER] Real time: %.0f, Wrapped time: %.0f\n", real_time, wrapped_time);

    return wrapped_time;
}

// Wrapper for get_random_number
int __wrap_get_random_number(void)
{
    static int call_count = 0;
    call_count++;

    printf("[WRAPPER] get_random_number() called (count: %d)\n", call_count);

    // In this example, we always return 42 (for testing)
    // In real scenarios, you might return deterministic values for replay
    int original_value = __real_get_random_number();
    int wrapped_value = 42;

    printf("[WRAPPER] Original random: %d, Wrapped value: %d\n", original_value, wrapped_value);

    return wrapped_value;
}










