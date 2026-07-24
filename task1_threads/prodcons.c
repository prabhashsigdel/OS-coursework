/* Producer-consumer demo with a bounded buffer.
 * Semaphores count empty/full slots, a mutex protects the buffer
 * itself. 1 producer + 2 consumers = 3 threads sharing the buffer.
 */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "demos.h"

#define BUF_MAX 10

static int buffer[BUF_MAX];
static int buf_size, n_items; /* chosen by the user */
static int in, out;

static sem_t empty_slots; /* how many free slots are left */
static sem_t full_slots;  /* how many items are waiting */
static pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;

static void *producer(void *arg)
{
    int i;
    (void)arg;

    for (i = 1; i <= n_items; i++) {
        sem_wait(&empty_slots); /* block if buffer is full */
        pthread_mutex_lock(&buf_lock);
        buffer[in] = i;
        printf("Producer: put item %d in slot %d\n", i, in);
        in = (in + 1) % buf_size;
        pthread_mutex_unlock(&buf_lock);
        sem_post(&full_slots);
        usleep(100000); /* 0.1s so the output is readable */
    }
    return NULL;
}

static void *consumer(void *arg)
{
    int id = *(int *)arg;
    int i, item;

    /* two consumers split the items; consumer 1 takes the odd one out */
    int my_share = (id == 1) ? n_items - n_items / 2 : n_items / 2;

    for (i = 0; i < my_share; i++) {
        sem_wait(&full_slots); /* block if buffer is empty */
        pthread_mutex_lock(&buf_lock);
        item = buffer[out];
        printf("    Consumer %d: took item %d from slot %d\n", id, item, out);
        out = (out + 1) % buf_size;
        pthread_mutex_unlock(&buf_lock);
        sem_post(&empty_slots);
        usleep(150000);
    }
    return NULL;
}

void prodcons_demo(void)
{
    pthread_t prod, cons1, cons2;
    int id1 = 1, id2 = 2;

    printf("\n--- Producer-consumer demo ---\n");
    buf_size = ask_int("Buffer size (2-10)", 5, 2, BUF_MAX);
    n_items = ask_int("Items to produce (2-30)", 10, 2, 30);
    printf("\nBuffer size %d, %d items, 1 producer + 2 consumers\n\n",
           buf_size, n_items);

    in = out = 0;
    sem_init(&empty_slots, 0, buf_size);
    sem_init(&full_slots, 0, 0);

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, &id1);
    pthread_create(&cons2, NULL, consumer, &id2);

    pthread_join(prod, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    printf("Producer-consumer demo finished.\n");
}
