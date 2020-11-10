#include <stdio.h>
#include <stdlib.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_ITEM 10
#define MAX_THREAD 4
#define NUM_ITERATIONS 1000000
#define NUM_CONSUMERS 2
#define NUM_PRODUCERS 2

typedef struct {
    int items;                   // item = 0  number of item in buffer
    uthread_mutex_t mutex;
    uthread_cond_t notFull;     //indicate producer space avialable
    uthread_cond_t notEmpty;    // indicate consumer content available
} buffer_t;

void *producer(void *arg);
void *consumer(void *arg);
buffer_t *Buffer_Init();

int producer_wait_count = 0;
int consumer_wait_count = 0;
int histogram[MAX_ITEM + 1];

int main() {
    buffer_t *buffer = Buffer_Init();
    uthread_t t[MAX_THREAD];
    uthread_init(MAX_THREAD);

    for (int i = 0; i < NUM_CONSUMERS + NUM_PRODUCERS; i++) {
        if (i < NUM_PRODUCERS) {
            t[i] = uthread_create(producer, buffer);
            if (t[i] == NULL)
                printf("%d Producer create failed.\n", i);
        } else {
            t[i] = uthread_create(consumer, buffer);
            if (t[i] == NULL)
                printf("%d Consumer create failed.\n", i);
        }
    }

    for (int i = 0; i < NUM_CONSUMERS + NUM_PRODUCERS; i++) {
        if (uthread_join(t[i], NULL) != 0)
            printf("wait thread %d failed.", i);
    }

    printf("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
    printf("items value histogram:\n");
    int sum = 0;
    for (int i = 0; i <= MAX_ITEM; i++) {
        printf("  items=%d, %d times\n", i, histogram [i]);
        sum += histogram[i];
    }
    assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}

void *producer (void *arg) {
    buffer_t *buffer = (buffer_t *) arg;
    for (int k = 0; k < NUM_ITERATIONS; k++) {
        uthread_mutex_lock(buffer->mutex);

        while (buffer->items >= MAX_ITEM) {  //no space, wait for the signal from consumer
            uthread_cond_wait(buffer->notFull);
            producer_wait_count++;
        }

        buffer->items++;
        histogram[buffer->items]++;

        assert(buffer->items >= 0 && buffer->items <= MAX_ITEM);

        uthread_cond_signal(buffer->notEmpty);// prod signal consumer new conent available

        uthread_mutex_unlock(buffer->mutex );
    }
    return NULL;
}

void *consumer (void *arg) {
    buffer_t *buffer = (buffer_t *) arg;
    for (int k = 0; k < NUM_ITERATIONS; k++) {
        uthread_mutex_lock(buffer->mutex);

        while(buffer->items <= 0) { //no content, wait for the signal from producer
            uthread_cond_wait(buffer->notEmpty);
            consumer_wait_count++;
        }

        buffer->items--;
        histogram[buffer->items]++;

        assert(buffer->items >= 0 && buffer->items <= MAX_ITEM);

        uthread_cond_signal(buffer->notFull); // consumer signal producer new space available

        uthread_mutex_unlock(buffer->mutex);
    }
    return NULL;
}

buffer_t *Buffer_Init(){
    buffer_t *buffer;

    buffer = (buffer_t *) malloc(sizeof(buffer_t));
    if (buffer ==  NULL) {
        fprintf(stderr, "main: Buffer Init failed.\n");
        exit (1);
    }

    buffer->items = 0;
    buffer->mutex = uthread_mutex_create();
    buffer->notFull = uthread_cond_create(buffer->mutex);
    buffer->notEmpty = uthread_cond_create(buffer->mutex);

    return (buffer);
}

