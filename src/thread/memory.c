#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>

void display_thread()
{
    size_t stacksize;
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("stack size : %zd\n", stacksize);
}

void* foo(void *arg)
{
    //char x[2000000] = {0};
    display_thread();
}

void* bar(void *arg)
{
    //char x[30000000] = {0};
    display_thread();
}

int main()
{
    int i = 1;
    size_t stacksize;
    struct rlimit rlim;
    bar(&i);

    pthread_t t1;

    void *res;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    getrlimit(RLIMIT_STACK, &rlim);
    printf("%zd\n", (size_t) rlim.rlim_cur);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("%zd\n", stacksize);
    pthread_attr_setstacksize(&attr, 4200000);
    pthread_create(&t1, &attr, foo, "thread");
    pthread_attr_destroy(&attr);

    pthread_join(t1, &res);

    return 0;
}

