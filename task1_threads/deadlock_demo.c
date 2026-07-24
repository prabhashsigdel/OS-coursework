/* Deadlock prevention demo.
 * Two threads both need lock A and lock B. If each thread grabbed
 * them in a different order they could end up waiting on each other
 * forever (circular wait). The fix used here: every thread takes the
 * locks in the same fixed order, A then B, so deadlock cannot happen.
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "demos.h"

static pthread_mutex_t lock_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock_b = PTHREAD_MUTEX_INITIALIZER;

static void *task(void *arg)
{
    int id = *(int *)arg;
    int i;

    for (i = 0; i < 3; i++) {
        /* fixed lock ordering: always A before B */
        pthread_mutex_lock(&lock_a);
        printf("Thread %d: got lock A\n", id);
        usleep(50000);
        pthread_mutex_lock(&lock_b);
        printf("Thread %d: got lock B, doing work\n", id);
        usleep(50000);
        pthread_mutex_unlock(&lock_b);
        pthread_mutex_unlock(&lock_a);
        printf("Thread %d: released both locks\n", id);
        usleep(10000); /* small gap so the other thread gets a turn */
    }
    return NULL;
}

void deadlock_demo(void)
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    printf("\n--- Deadlock prevention demo ---\n");
    printf("Both threads always lock A before B (fixed ordering),\n");
    printf("so a circular wait is impossible.\n\n");

    pthread_create(&t1, NULL, task, &id1);
    pthread_create(&t2, NULL, task, &id2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Finished with no deadlock.\n");
}
