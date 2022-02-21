#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

volatile int stop = 0;

void* loop(void *arg) {
    (void)arg;

    while (!stop) { /* ... */ }

    return 0;
}

int main()
{
    pthread_t t1;
    pthread_create(&t1, NULL, loop, NULL);

/*    long double total = 0;
    srand(0);
    for (int i = 0 ; i < 40000000 ; ++i) {
        double theta = (rand()/(double)RAND_MAX) * 2 * M_PI;
        total += fabs(sin(theta)/cos(theta));
    }
    */

   // printf("%Lf\n", total);
   //
   sleep(5);
   printf("end\n");

    stop = 1;

    pthread_join(t1, NULL);

    return 0;
}

