#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <string.h>

#define ARRAY_SIZE 1000000

int* threads_values;

typedef struct {
    int *begin;
    int *end;
    int id;
} thread_input;

int test_prime(int value) {
    for (int i=2; i < value / 2; i++) {
        if (value % i == 0) {
            return 0;
        }
    }
    return 1;
}

void* count_prime(void *arg)
{
    (void)arg;
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "%s nb-threads\n", argv[0]);
        return -1;
    }

    int nb_threads = atoi(argv[1]);
    (void)nb_threads;

    srand(0);
    int* values = (int*)malloc(ARRAY_SIZE*sizeof(int));
    for (int i = 0 ; i < ARRAY_SIZE ; ++i)
        values[i] = rand()%100000;

    return 0;
}

