#include "common.h"
#include <pthread.h>
#include <stdatomic.h>

enum { ROUNDS = 10000 };
static atomic_int counter = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t barrier;
static const char *mode;

static void rendezvous(void)
{
    int code = pthread_barrier_wait(&barrier);
    if (code != PTHREAD_BARRIER_SERIAL_THREAD) check_thread(code);
}

static void *worker(void *unused)
{
    (void)unused;
    for (int i = 0; i < ROUNDS; ++i) {
        if (strcmp(mode, "split") == 0) {
            int old = atomic_load(&counter);
            rendezvous(); /* 两个线程都读到旧值后，才允许写。 */
            atomic_store(&counter, old + 1);
            rendezvous(); /* 两个写都结束后，进入下一轮。 */
        } else if (strcmp(mode, "mutex") == 0) {
            check_thread(pthread_mutex_lock(&mutex));
            int old = atomic_load(&counter);
            atomic_store(&counter, old + 1);
            check_thread(pthread_mutex_unlock(&mutex));
        } else {
            atomic_fetch_add(&counter, 1);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2 || (strcmp(argv[1], "split") != 0 &&
                     strcmp(argv[1], "mutex") != 0 &&
                     strcmp(argv[1], "atomic") != 0)) {
        fprintf(stderr, "usage: %s split|mutex|atomic\n", argv[0]);
        return 2;
    }
    mode = argv[1];
    check_thread(pthread_barrier_init(&barrier, NULL, 2));
    pthread_t threads[2];
    for (int i = 0; i < 2; ++i)
        check_thread(pthread_create(&threads[i], NULL, worker, NULL));
    for (int i = 0; i < 2; ++i)
        check_thread(pthread_join(threads[i], NULL));
    printf("mode=%s expected=%d actual=%d\n", mode, 2 * ROUNDS, atomic_load(&counter));
    check_thread(pthread_barrier_destroy(&barrier));
    check_thread(pthread_mutex_destroy(&mutex));
    int model_result = strcmp(mode, "split") == 0 ? ROUNDS : 2 * ROUNDS;
    return atomic_load(&counter) == model_result ? 0 : 1;
}
