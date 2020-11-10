#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 1000000;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram[MAX_ITEMS+1];  // histogram [i] == # of times list stored i items

int items = 0;

// Initial spin lock
spinlock_t s_lock;

void *producer(void* v) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // TODO
        while (s_lock == 1)
            producer_wait_count++;
        spinlock_lock(&s_lock);

        if (items < MAX_ITEMS) {
            items++;
            histogram[items]++;

            assert(items >= 0 && items <= MAX_ITEMS);
        } else {
            i--; // add back the invalid interation
        }

        spinlock_unlock(&s_lock);
    }
    return NULL;
}

void *consumer(void* v) {
    for (int i = 0; i< NUM_ITERATIONS; i++) {
        // TODO
        while (s_lock == 1)
            consumer_wait_count++;
        spinlock_lock(&s_lock);

        if (items > 0){
            items--;
            histogram[items]++;

            assert(items >= 0 && items <= MAX_ITEMS);
        } else {
            i--; // add back the invalid interation
        }

         spinlock_unlock(&s_lock);
    }
    return NULL;
}

int main(int argc, char** argv) {
    uthread_t t[4];

    uthread_init(4);

    // TODO: Create Threads and Join
    spinlock_create(&s_lock);

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
        if(uthread_join(t[i], NULL ) != 0)
            printf("wait thread %d failed.",i);
    }

    printf("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
    printf("items value histogram:\n");
    int sum = 0;
    for (int i = 0; i <= MAX_ITEMS; i++) {
        printf("  items=%d, %d times\n", i, histogram[i]);
        sum += histogram [i];
    }
    assert(sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
