#define _DEFAULT_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* Keep the returned pointer aligned for any ordinary C type. */
typedef union {
    struct {
        size_t mapping_size;
        size_t requested_size;
    } info;
    max_align_t alignment;
} Header;

void *malloc(size_t size)
{
    if (size > SIZE_MAX - sizeof(Header)) {
        errno = ENOMEM;
        return NULL;
    }
    size_t total = sizeof(Header) + size;
    Header *header = mmap(NULL, total, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (header == MAP_FAILED)
        return NULL;
    header->info.mapping_size = total;
    header->info.requested_size = size;
    return header + 1;
}

void free(void *ptr)
{
    if (!ptr)
        return;
    int saved_errno = errno;
    Header *header = (Header *)ptr - 1;
    munmap(header, header->info.mapping_size);
    errno = saved_errno;
}

void *calloc(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size) {
        errno = ENOMEM;
        return NULL;
    }
    /* Anonymous mmap pages are already zero-initialized. */
    return malloc(count * size);
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);
    if (!size) {
        free(ptr);
        return NULL;
    }
    Header *header = (Header *)ptr - 1;
    void *replacement = malloc(size);
    if (!replacement)
        return NULL;
    size_t old_size = header->info.requested_size;
    memcpy(replacement, ptr, old_size < size ? old_size : size);
    free(ptr);
    return replacement;
}
