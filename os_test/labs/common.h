#ifndef LAB_COMMON_H
#define LAB_COMMON_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pthread 函数返回错误码，不通过 errno 报错。 */
static inline void check_thread(int code)
{
    if (code != 0) {
        fprintf(stderr, "pthread: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }
}

static inline void die(const char *operation)
{
    perror(operation);
    exit(EXIT_FAILURE);
}
#endif
