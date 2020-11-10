#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 1000000;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram[MAX_ITEMS + 1];

typedef struct {
    int items;              // item = 0  number of item in buffer
    uthread_sem_t mutex;
    uthread_sem_t notFull;  //indicate producer space avialable
    uthread_sem_t notEmpty; // indicate consumer content available
} buffer_t;

void *producer(void *v);
void *consumer(void *v);
buffer_t *Buffer_Init();

buffer_t *buffer;

void *producer(void *v) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        uthread_sem_wait(buffer->notFull);

        uthread_sem_wait(buffer->mutex);

        histogram[++buffer->items]++;
        assert(buffer->items >= 0 && buffer->items <= MAX_ITEMS);

        uthread_sem_signal(buffer->mutex);

        uthread_sem_signal(buffer->notEmpty);
    }
    return NULL;
}

void *consumer(void *v) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        uthread_sem_wait(buffer->notEmpty);

        uthread_sem_wait(buffer->mutex);

        histogram[--buffer->items]++;
        assert(buffer->items >= 0 && buffer->items <= MAX_ITEMS);

        uthread_sem_signal(buffer->mutex);

        uthread_sem_signal(buffer->notFull);
    }
    return NULL;
}

int main () {
    uthread_t t[4];
    uthread_init (4);

    buffer = Buffer_Init();

    for (int i = 0; i < NUM_CONSUMERS + NUM_PRODUCERS; i++) {
        if (i < NUM_PRODUCERS) {
            t[i] = uthread_create(producer, NULL);
            if (t[i] == NULL)
                printf("%d Producer create failed.\n", i);
        } else {
            t[i] = uthread_create(consumer, NULL);
            if (t[i] == NULL)
                printf("%d Consumer create failed.\n", i);
        }
    }

    for (int i = 0; i < NUM_CONSUMERS + NUM_PRODUCERS; i++) {
        if(uthread_join(t[i], NULL) != 0)
            printf(" wait thread %d failed.", i);
    }

    printf("items value histogram:\n");
    int sum = 0;
    for (int i = 0; i <= MAX_ITEMS; i++) {
        printf("  items=%d, %d times\n", i, histogram [i]);
        sum += histogram[i];
    }
    assert(sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}


buffer_t *Buffer_Init(){
    buffer_t *buffer;

    buffer = (buffer_t *) malloc(sizeof(buffer_t));
    if (buffer ==  NULL) {
        fprintf (stderr, "main: Buffer Init failed.\n");
        exit(1);
    }

    buffer->items = 0;
    buffer->mutex = uthread_sem_create(1);
    buffer->notFull = uthread_sem_create(MAX_ITEMS);
    buffer->notEmpty = uthread_sem_create(0);

    return (buffer);
}
