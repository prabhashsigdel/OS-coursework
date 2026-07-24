/* Race condition demo: N threads increment a shared counter.
 * First run has no locking so updates get lost, second run uses a
 * mutex and the total comes out right.
 */
#include <stdio.h>
#include <pthread.h>
#include "demos.h"

#define MAX_THREADS 8

static long counter;
static long increments; /* per thread, chosen by the user */
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
static int use_mutex; /* 0 = unsafe run, 1 = safe run */

static void *worker(void *arg)
{
    int id = *(int *)arg;
    long i;

    for (i = 0; i < increments; i++) {
        if (use_mutex) {
            pthread_mutex_lock(&counter_lock);
            counter++;
            pthread_mutex_unlock(&counter_lock);
        } else {
            counter++; /* unprotected - this is the race condition */
        }
    }
    printf("  thread %d done\n", id);
    return NULL;
}

static void run_counting(int nthreads, int with_mutex)
{
    pthread_t threads[MAX_THREADS];
    int ids[MAX_THREADS];
    int i;

    counter = 0;
    use_mutex = with_mutex;

    for (i = 0; i < nthreads; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    for (i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    printf("Expected: %ld, got: %ld %s\n",
           (long)nthreads * increments, counter,
           counter == (long)nthreads * increments
               ? "(correct)" : "(WRONG - updates lost to the race!)");
}

void race_demo(void)
{
    int nthreads;

    printf("\n--- Race condition demo ---\n");
    nthreads = ask_int("Number of threads (2-8)", 4, 2, MAX_THREADS);
    increments = ask_int("Increments per thread", 500000, 1000, 2000000);

    printf("\n%d threads each increment a shared counter %ld times.\n\n",
           nthreads, increments);

    printf("Run 1: WITHOUT mutex\n");
    run_counting(nthreads, 0);

    printf("\nRun 2: WITH mutex\n");
    run_counting(nthreads, 1);
}
