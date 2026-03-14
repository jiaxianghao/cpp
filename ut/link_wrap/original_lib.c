/*
 * Original library function
 * This simulates a function from an external library
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Original function that we want to wrap
double get_current_time(void)
{
    return (double)time(NULL);
}

// Another function to demonstrate multiple wraps
int get_random_number(void)
{
    return rand() % 100;
}

