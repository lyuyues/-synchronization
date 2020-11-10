#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
    pthread_mutex_t mutex;
    pthread_cond_t  match;
    pthread_cond_t  paper;
    pthread_cond_t  tobacco;
    pthread_cond_t  smoke;
};

struct Agent* createAgent() {
    struct Agent* agent = malloc (sizeof (struct Agent));
    pthread_mutex_init(&agent->mutex, NULL);
    pthread_cond_init(&agent->paper, NULL);
    pthread_cond_init(&agent->match, NULL);
    pthread_cond_init(&agent->tobacco, NULL);
    pthread_cond_init(&agent->smoke, NULL);
    return agent;
}

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

int wait_conditions [5];
pthread_cond_t wait_cond;

int intermediate_running = 0;
int smoke_running = 0;

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
    struct Agent* a = av;
    static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
    static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

    pthread_mutex_lock(&a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        srandom(time(NULL));
        int r = random() % 3;
        signal_count [matching_smoker [r]] ++;
        int c = choices [r];
        if (c & MATCH) {
            VERBOSE_PRINT("match available\n");
            pthread_cond_signal(&a->match);
        }
        if (c & PAPER) {
            VERBOSE_PRINT("paper available\n");
            pthread_cond_signal(&a->paper);
        }
        if (c & TOBACCO) {
            VERBOSE_PRINT("tobacco available\n");
            pthread_cond_signal(&a->tobacco);
        }
        VERBOSE_PRINT("agent is waiting for smoker to smoke\n");
        pthread_cond_wait(&a->smoke, &a->mutex);
    }
    pthread_mutex_unlock(&a->mutex);
    return NULL;
}

struct Smoker {
    enum Resource resource;
    pthread_mutex_t *p_mutex;
    pthread_cond_t  *p_smoke;
    pthread_cond_t  cond;
};

struct Smoker *createSmoker(
        pthread_mutex_t *p_mutex,
        pthread_cond_t  *p_smoke,
        enum Resource resource) {
    struct Smoker *smoker = malloc(sizeof(struct Smoker));
    smoker->p_mutex = p_mutex;
    smoker->p_smoke = p_smoke;
    smoker->resource = resource;
    pthread_cond_init(&smoker->cond, NULL);
    return smoker;
}

struct Intermediate {
    pthread_mutex_t *p_mutex;
    pthread_cond_t  **p_agent_conditions;
    pthread_cond_t  **p_smoker_conditions;
    enum Resource   *resources;
};

struct Intermediate *createIntermediate(
        pthread_mutex_t *p_mutex,
        pthread_cond_t  **p_agent_conditions,
        pthread_cond_t  **p_smoker_conditions,
        enum Resource   *resources) {
    struct Intermediate *intermediate = malloc(sizeof(struct Intermediate));
    intermediate->p_mutex = p_mutex;
    intermediate->p_agent_conditions = p_agent_conditions;
    intermediate->p_smoker_conditions = p_smoker_conditions;
    intermediate->resources = resources;
    return intermediate;
}

void *intermediate(void *i) {
    struct Intermediate *intermediate = i;

    enum Resource res = *(intermediate->resources);
    enum Resource res1 = *(intermediate->resources + 1);
    enum Resource res2 = *(intermediate->resources + 2);

    pthread_cond_t *p_agent_cond = *(intermediate->p_agent_conditions + res1);
    pthread_cond_t *p_smoke_cond = *(intermediate->p_smoker_conditions + res2);

    pthread_mutex_lock(intermediate->p_mutex);
    intermediate_running++;
    while (1) {
        VERBOSE_PRINT("Intermediate %s Step 1, waiting for %s\n", resource_name[res], resource_name[res1]);
        pthread_cond_wait(p_agent_cond, intermediate->p_mutex);
        VERBOSE_PRINT("Intermediate %s Step 1, %s acquired\n", resource_name[res], resource_name[res1]);

        wait_conditions[res1] = 1;
        pthread_cond_signal(&wait_cond);

        while (1) {
            if (!wait_conditions[res1]) {
                VERBOSE_PRINT("Intermediate %s Step 2, reset\n", resource_name[res]);
                pthread_cond_signal(p_smoke_cond);

                break;
            }

            if (wait_conditions[res2]) {
                wait_conditions[res1] = 0;
                wait_conditions[res2] = 0;
                VERBOSE_PRINT("Intermediate %s Step 2, %s acquired, reset\n", resource_name[res], resource_name[res2]);
                pthread_cond_signal(&wait_cond);
                break;
            } else {
                VERBOSE_PRINT("Intermediate %s Step 2, wait\n", resource_name[res]);
                pthread_cond_wait(&wait_cond, intermediate->p_mutex);
            }
        }
    }
    pthread_mutex_unlock(intermediate->p_mutex);
}

void *smoke(void *s) {
    struct Smoker *smoker = s;

    pthread_mutex_lock(smoker->p_mutex);
    smoke_running++;
    while (1) {
        VERBOSE_PRINT("Smoker %s waiting.\n", resource_name[smoker->resource]);
        pthread_cond_wait(&smoker->cond, smoker->p_mutex);
        VERBOSE_PRINT("Smoker %s smoking.\n", resource_name[smoker->resource]);

        smoke_count[smoker->resource]++;

        pthread_cond_signal(smoker->p_smoke);
    }
    pthread_mutex_unlock(smoker->p_mutex);
}

int main (int argc, char** argv) {
    struct Agent*  a = createAgent();

    pthread_cond_init(&wait_cond, NULL);

    struct Smoker *s1 = createSmoker(&a->mutex, &a->smoke, TOBACCO);
    struct Smoker *s2 = createSmoker(&a->mutex, &a->smoke, PAPER);
    struct Smoker *s3 = createSmoker(&a->mutex, &a->smoke, MATCH);

    pthread_cond_t *agent_conditions[] = {NULL, &a->match, &a->paper, NULL, &a->tobacco};
    pthread_cond_t *smoker_conditions[] = {NULL, &s3->cond, &s2->cond, NULL, &s1->cond};

    enum Resource resources1[] = {TOBACCO, PAPER, MATCH};
    struct Intermediate *i1 = createIntermediate(
        &a->mutex,
        agent_conditions,
        smoker_conditions,
        resources1);

    enum Resource resources2[] = {PAPER, MATCH, TOBACCO};
    struct Intermediate *i2 = createIntermediate(
        &a->mutex,
        agent_conditions,
        smoker_conditions,
        resources2);

    enum Resource resources3[] = {MATCH, TOBACCO, PAPER};
    struct Intermediate *i3 = createIntermediate(
        &a->mutex,
        agent_conditions,
        smoker_conditions,
        resources3);

    pthread_t thrd_list[7];
    pthread_create(&thrd_list[0], NULL, smoke, s1);
    pthread_create(&thrd_list[1], NULL, smoke, s2);
    pthread_create(&thrd_list[2], NULL, smoke, s3);
    pthread_create(&thrd_list[3], NULL, intermediate, i1);
    pthread_create(&thrd_list[4], NULL, intermediate, i2);
    pthread_create(&thrd_list[5], NULL, intermediate, i3);

    while (1) {
        pthread_mutex_lock(&a->mutex);
        if (intermediate_running == 3 && smoke_running == 3) {
            pthread_mutex_unlock(&a->mutex);
            break;
        }
        pthread_mutex_unlock(&a->mutex);
    }

    pthread_create(&thrd_list[6], NULL, agent, a);
    pthread_join(thrd_list[6], NULL);

    assert (signal_count [MATCH]   == smoke_count [MATCH]);
    assert (signal_count [PAPER]   == smoke_count [PAPER]);
    assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
    assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
    printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}