#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

long glob = 0;
pthread_mutex_t glob_mtx = PTHREAD_MUTEX_INITIALIZER;

/* Loop 'arg' times incrementing 'glob' */
void * threadFunc(void *arg)
{
    long loops = *((long *) arg);
    long loc;
    while (loops-- > 0) {
        pthread_mutex_lock(&glob_mtx);
        loc = glob;
        loc++;
        glob = loc;
        pthread_mutex_unlock(&glob_mtx);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    long loops;
    loops = (argc > 1) ? atol(argv[1]) : 1000;

    pthread_t t1, t2;

    pthread_create(&t1, NULL, threadFunc, &loops);

    pthread_create(&t2, NULL, threadFunc, &loops);

    pthread_join(t1, NULL);

    pthread_join(t2, NULL);

    printf("glob = %ld\n", glob);

    return 0;
}
