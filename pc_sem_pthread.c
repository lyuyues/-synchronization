#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_ITEMS 10
#define MAX_THREAD 4
const int NUM_ITERATIONS = 1000000;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram[MAX_ITEMS + 1];

sem_t mutex;
sem_t notFull;  //indicate producer space avialable
sem_t notEmpty; // indicate consumer content available
int items = 0;  // item = 0  number of item in buffer

void *producer(void *v);
void *consumer(void *v);

void *producer(void *v) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // TODO
        sem_wait(&notFull);

        sem_wait(&mutex);

        items++;
        histogram[items]++;

        assert(items >= 0 && items <= MAX_ITEMS);

        sem_post(&mutex);

        sem_post(&notEmpty);
    }
    return NULL;
}

void *consumer(void *v) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // TODO
        sem_wait(&notEmpty);

        sem_wait(&mutex);

        items--;
        histogram[items]++;

        assert(items >= 0 && items <= MAX_ITEMS);

        sem_post(&mutex);

        sem_post(&notFull);
    }
    return NULL;
}

int main (){
    sem_init(&mutex, 0, 1);
    sem_init(&notFull, 0, MAX_ITEMS);
    sem_init(&notEmpty, 0, 0);

    // TODO: Create Threads and Join
    pthread_t  thrd_list[MAX_THREAD]; // list of threads

    for (int i = 0; i < MAX_THREAD; i++) {
        if (i < NUM_PRODUCERS) {
            if (pthread_create(&thrd_list[i], NULL, producer, NULL))
                printf("%d Producer create failed.\n", i);
        } else {
            if (pthread_create(&thrd_list[i], NULL, consumer, NULL))
                printf("%d Consumer create failed.\n", i);
        }
    }

    for (int i = 0; i < MAX_THREAD; i++) {
        if (pthread_join(thrd_list[i], NULL) != 0)
            printf(" wait thread %d failed.", i);
    }

    printf("items value histogram:\n");
    int sum = 0;
    for (int i = 0; i <= MAX_ITEMS; i++) {
        printf("  items=%d, %d times\n", i, histogram[i]);
        sum += histogram [i];
    }
    assert(sum == sizeof (thrd_list) / sizeof (pthread_t) * NUM_ITERATIONS);
}
