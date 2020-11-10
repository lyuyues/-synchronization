#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_ITEM 10
#define MAX_THREAD 10
#define PROD_NUM 5
#define CONS_NUM 5
#define NUM_ITERATION 1000000

typedef struct {
    int items;                  // item = 0  number of item in buffer
    pthread_mutex_t mutex;
    pthread_cond_t notFull;     // indicate producer space avialable
    pthread_cond_t notEmpty;    // indicate consumer content available
} buffer_t;

void *producer(void *arg);
void *consumer(void *arg);
buffer_t *Buffer_Init(void);

int producer_wait_count = 0;
int consumer_wait_count = 0;
int histogram[MAX_ITEM + 1];

int main() {
    buffer_t *buffer = Buffer_Init();

    pthread_t thrd_list[MAX_THREAD]; // list of threads

    for (int i = 0; i < MAX_THREAD; i++) {
        if (i < PROD_NUM) {
            if (pthread_create(&thrd_list[i], NULL, producer, buffer))
                printf( "%d Producer create failed.\n",i);
        } else {
            if (pthread_create(&thrd_list[i], NULL, consumer, buffer))
                printf( "%d Consumer create failed.\n", i);
        }
    }

    for (int i = 0; i < MAX_THREAD; i++) {
        if (pthread_join(thrd_list[i], NULL ) != 0)
            printf( " wait thread %d failed.",i);
    }

    printf("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
    printf("items value histogram:\n");
    int sum=0;
    for (int i = 0; i <= MAX_ITEM; i++) {
        printf("  items=%d, %d times\n", i, histogram [i]);
        sum += histogram [i];
    }
    assert(sum == MAX_THREAD * NUM_ITERATION);

    exit(EXIT_SUCCESS);
}

void *producer (void *arg) {
    buffer_t *buffer = (buffer_t *) arg;
    for(int k = 0; k < NUM_ITERATION; k++) {
        pthread_mutex_lock(&buffer->mutex);

        while (buffer->items >= MAX_ITEM) {  //no space, wait for the signal from consumer
            pthread_cond_wait(&buffer->notFull, &buffer->mutex);
            producer_wait_count++;
        }

        buffer->items++;
        histogram[buffer->items]++;

        assert(buffer->items >= 0 && buffer->items <= MAX_ITEM);

        pthread_cond_signal(&buffer->notEmpty);// prod signal consumer new conent available

        pthread_mutex_unlock(&buffer->mutex);
    }
}

void *consumer (void *arg) {
    buffer_t *buffer = (buffer_t *) arg;
    for(int k = 0; k < NUM_ITERATION; k++) {
        pthread_mutex_lock(&buffer->mutex);

        while (buffer->items <= 0) { //no content, wait for the signal from producer
            pthread_cond_wait(&buffer->notEmpty, &buffer->mutex);
            consumer_wait_count++;
        }

        buffer->items--;
        histogram[buffer->items]++;

        assert(buffer->items >= 0 && buffer->items <= MAX_ITEM);

        pthread_cond_signal(&buffer->notFull); // consumer signal producer new space available

        pthread_mutex_unlock(&buffer->mutex);
    }
}

buffer_t *Buffer_Init(void) {
    buffer_t *buffer;

    buffer = (buffer_t *) malloc(sizeof(buffer_t));
    if (buffer ==  NULL) {
        fprintf (stderr, "main: Buffer Init failed.\n");
        exit (1);
    }

    buffer->items = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->notFull,NULL);
    pthread_cond_init(&buffer->notEmpty,NULL);

    return (buffer);
}

