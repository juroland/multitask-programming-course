#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct someStruct { /* ... */ };

void * threadFunc(void *arg)
{
    struct someStruct *pbuf = (struct someStruct *) arg;
    (void)pbuf;
    /* Do some work with structure pointed to by 'pbuf' */

    sleep(2);

    printf("return from threadFunc\n");

    return 0;
}

int main()
{
    struct someStruct buf;
    pthread_t thr;

    pthread_create(&thr, NULL, threadFunc, (void *) &buf);


    printf("exit from main\n");

    pthread_exit(NULL);

    printf("return from main\n");

    return 0;
}
