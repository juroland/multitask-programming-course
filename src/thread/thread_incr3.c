#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 2
#define SPACING 64

long glob[NTHREADS*SPACING] = {};

typedef struct{
    int id;
    long loops;
} thread_parameters;

/* Loop 'arg' times incrementing 'glob' */
void * threadFunc(void *arg)
{
    thread_parameters parameters = *((thread_parameters *) arg);
    long loc;
    while (parameters.loops-- > 0) {
        loc = glob[parameters.id];
        loc++;
        glob[parameters.id] = loc;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    long loops;
    loops = (argc > 1) ? atol(argv[1]) : 1000;

    pthread_t t[NTHREADS];
    thread_parameters parameters[NTHREADS];

    for (int i = 0; i < NTHREADS ; ++i) {
        parameters[i].id = i*SPACING;
        parameters[i].loops = loops;
        pthread_create(&t[i], NULL, threadFunc, &parameters[i]);
    }

    long total = 0;
    for (int i = 0; i < NTHREADS ; ++i) {
        pthread_join(t[i], NULL);
        total += glob[i];
    }

    printf("glob = %ld\n", total);

    return 0;
}
