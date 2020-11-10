#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
<<<<<<< HEAD
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000
=======
#include <time.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000000
>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
    uthread_mutex_t mutex;
    uthread_cond_t  match;
    uthread_cond_t  paper;
    uthread_cond_t  tobacco;
    uthread_cond_t  smoke;
};

struct Agent* createAgent() {
    struct Agent* agent = malloc (sizeof (struct Agent));
    agent->mutex   = uthread_mutex_create();
    agent->paper   = uthread_cond_create (agent->mutex);
    agent->match   = uthread_cond_create (agent->mutex);
    agent->tobacco = uthread_cond_create (agent->mutex);
    agent->smoke   = uthread_cond_create (agent->mutex);
    return agent;
}

<<<<<<< HEAD
//
// TODO
// You will probably need to add some procedures and struct etc.
//

=======
>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58
/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

<<<<<<< HEAD
=======
int wait_conditions [5];
uthread_cond_t wait_cond;

int intermediate_running = 0;
int smoke_running = 0;

>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58
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
<<<<<<< HEAD
    
    uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
=======

    uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        srandom(time(NULL));
>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58
        int r = random() % 3;
        signal_count [matching_smoker [r]] ++;
        int c = choices [r];
        if (c & MATCH) {
            VERBOSE_PRINT ("match available\n");
            uthread_cond_signal (a->match);
        }
        if (c & PAPER) {
            VERBOSE_PRINT ("paper available\n");
            uthread_cond_signal (a->paper);
        }
        if (c & TOBACCO) {
            VERBOSE_PRINT ("tobacco available\n");
            uthread_cond_signal (a->tobacco);
        }
        VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
        uthread_cond_wait (a->smoke);
    }
    uthread_mutex_unlock (a->mutex);
    return NULL;
}

<<<<<<< HEAD

void* smoker(void* av){
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        uthread_mutex_lock (a->mutex);
        for (int j = 1; j < 5; j++){
            if (j == 1){
                uthread_cond_wait(a->match);
                uthread_cond_wait(a->paper);
                smoke_count[j]++;
            }else if (j == 2) {
                uthread_cond_wait(a->match);
                uthread_cond_wait(a->tobacco);
                smoke_count[j]++;
            }else if (j == 4) {
                uthread_cond_wait(a->paper);
                uthread_cond_wait(a->tobacco);
                smoke_count[j]++;
            }else {
                continue;
            }
        uthread_cond_signal (a->smoke)
    }
    uthread_mutex_unlock (a->mutex);
    return NULL;
}
    
void* thirdparty(){
    
    
    
       return NULL;
    }

int main (int argc, char** argv) {
    uthread_init (7);
    struct Agent*  a = createAgent();
    // TODO
    
    for (int i = 0; i < 3; i++){
        uthread_join (uthread_create(smoker, NULL), 0);
    }
    uthread_join (uthread_create (agent, a), 0);
    
=======
struct Smoker {
    enum Resource resource;
    uthread_mutex_t mutex;
    uthread_cond_t  smoke;
    uthread_cond_t  cond;
};

struct Smoker *createSmoker(
        enum Resource resource,
        uthread_mutex_t mutex,
        uthread_cond_t  smoke) {
    struct Smoker *smoker = malloc(sizeof(struct Smoker));
    smoker->resource = resource;
    smoker->mutex = mutex;
    smoker->smoke = smoke;
    smoker->cond  = uthread_cond_create(mutex);
    return smoker;
}

struct Intermediate {
    uthread_mutex_t mutex;
    uthread_cond_t  *agent_conditions;
    uthread_cond_t  *smoker_conditions;
    enum Resource   *resources;
};

struct Intermediate *createIntermediate(
        uthread_mutex_t mutex,
        uthread_cond_t  *agent_conditions,
        uthread_cond_t  *smoker_conditions,
        enum Resource   *resources) {
    struct Intermediate *intermediate = malloc(sizeof(struct Intermediate));
    intermediate->mutex = mutex;
    intermediate->agent_conditions = agent_conditions;
    intermediate->smoker_conditions = smoker_conditions;
    intermediate->resources = resources;
    return intermediate;
}

void *intermediate(void *i) {
    struct Intermediate *intermediate = i;

    enum Resource res = *(intermediate->resources);
    enum Resource res1 = *(intermediate->resources + 1);
    enum Resource res2 = *(intermediate->resources + 2);

    uthread_cond_t agent_cond = *(intermediate->agent_conditions + res1);
    uthread_cond_t smoke_cond = *(intermediate->smoker_conditions + res2);

    uthread_mutex_lock(intermediate->mutex);
    intermediate_running++;
    while (1) {
        VERBOSE_PRINT("Intermediate %s Step 1, waiting for %s\n", resource_name[res], resource_name[res1]);
        uthread_cond_wait(agent_cond);
        VERBOSE_PRINT("Intermediate %s Step 1, %s acquired\n", resource_name[res], resource_name[res1]);

        wait_conditions[res1] = 1;
        uthread_cond_signal(wait_cond);

        while (1) {
            if (!wait_conditions[res1]) {
                VERBOSE_PRINT("Intermediate %s Step 2, reset\n", resource_name[res]);
                uthread_cond_signal(smoke_cond);

                break;
            }

            if (wait_conditions[res2]) {
                wait_conditions[res1] = 0;
                wait_conditions[res2] = 0;
                VERBOSE_PRINT("Intermediate %s Step 2, %s acquired, reset\n", resource_name[res], resource_name[res2]);
                uthread_cond_signal(wait_cond);
                break;
            } else {
                VERBOSE_PRINT("Intermediate %s Step 2, wait\n", resource_name[res]);
                uthread_cond_wait(wait_cond);
            }
        }
    }
    uthread_mutex_unlock(intermediate->mutex);
}

void *smoke(void *s) {
    struct Smoker *smoker = s;

    uthread_mutex_lock(smoker->mutex);
    smoke_running++;
    while (1) {
        VERBOSE_PRINT("Smoker %s waiting.\n", resource_name[smoker->resource]);
        uthread_cond_wait(smoker->cond);
        VERBOSE_PRINT("Smoker %s smoking.\n", resource_name[smoker->resource]);

        smoke_count[smoker->resource]++;

        uthread_cond_signal(smoker->smoke);
    }
    uthread_mutex_unlock(smoker->mutex);
}

int main (int argc, char** argv) {
    uthread_init (7);

    struct Agent*  a = createAgent();

    wait_cond = uthread_cond_create(a->mutex);

    struct Smoker *s1 = createSmoker(TOBACCO, a->mutex, a->smoke);
    struct Smoker *s2 = createSmoker(PAPER, a->mutex, a->smoke);
    struct Smoker *s3 = createSmoker(MATCH, a->mutex, a->smoke);

    uthread_cond_t agent_conditions[] = {NULL, a->match, a->paper, NULL, a->tobacco};
    uthread_cond_t smoker_conditions[] = {NULL, s3->cond, s2->cond, NULL, s1->cond};

    enum Resource resources1[] = {TOBACCO, PAPER, MATCH};
    struct Intermediate *i1 = createIntermediate(
        a->mutex,
        agent_conditions,
        smoker_conditions,
        resources1);

    enum Resource resources2[] = {PAPER, MATCH, TOBACCO};
    struct Intermediate *i2 = createIntermediate(
        a->mutex,
        agent_conditions,
        smoker_conditions,
        resources2);

    enum Resource resources3[] = {MATCH, TOBACCO, PAPER};
    struct Intermediate *i3 = createIntermediate(
        a->mutex,
        agent_conditions,
        smoker_conditions,
        resources3);

    uthread_create(smoke, s1);
    uthread_create(smoke, s2);
    uthread_create(smoke, s3);
    uthread_create(intermediate, i1);
    uthread_create(intermediate, i2);
    uthread_create(intermediate, i3);

    while (1) {
        uthread_mutex_lock(a->mutex);
        if (intermediate_running == 3 && smoke_running == 3) {
            uthread_mutex_unlock(a->mutex);
            break;
        }
        uthread_mutex_unlock(a->mutex);
    }

    uthread_join(uthread_create(agent, a), 0);

>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58
    assert (signal_count [MATCH]   == smoke_count [MATCH]);
    assert (signal_count [PAPER]   == smoke_count [PAPER]);
    assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
    assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
    printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
<<<<<<< HEAD
            smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}
=======
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}
>>>>>>> 5014b961bd5c0734e80df5b65050849f7f4b5c58
