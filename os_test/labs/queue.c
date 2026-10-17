#include "common.h"
#include <pthread.h>

enum { COUNT = 10000 };
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static int slot, full;
static long sum;
static int ordered = 1;

static void *produce(void *unused)
{
    (void)unused;
    for (int i = 1; i <= COUNT; ++i) {
        check_thread(pthread_mutex_lock(&mutex));
        while (full) check_thread(pthread_cond_wait(&not_full, &mutex));
        slot = i;
        full = 1;
        check_thread(pthread_cond_signal(&not_empty));
        check_thread(pthread_mutex_unlock(&mutex));
    }
    return NULL;
}

static void *consume(void *unused)
{
    (void)unused;
    for (int i = 1; i <= COUNT; ++i) {
        check_thread(pthread_mutex_lock(&mutex));
        while (!full) check_thread(pthread_cond_wait(&not_empty, &mutex));
        if (slot != i) ordered = 0;
        sum += slot;
        full = 0;
        check_thread(pthread_cond_signal(&not_full));
        check_thread(pthread_mutex_unlock(&mutex));
    }
    return NULL;
}

int main(void)
{
    pthread_t producer, consumer;
    check_thread(pthread_create(&producer, NULL, produce, NULL));
    check_thread(pthread_create(&consumer, NULL, consume, NULL));
    check_thread(pthread_join(producer, NULL));
    check_thread(pthread_join(consumer, NULL));
    long expected = (long)COUNT * (COUNT + 1) / 2;
    printf("items=%d sum=%ld expected=%ld ordered=%d\n", COUNT, sum, expected, ordered);
    check_thread(pthread_cond_destroy(&not_empty));
    check_thread(pthread_cond_destroy(&not_full));
    check_thread(pthread_mutex_destroy(&mutex));
    return sum == expected && ordered && !full ? 0 : 1;
}
