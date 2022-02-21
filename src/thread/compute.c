#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <string.h>

#define ARRAY_SIZE 20000000

typedef struct {
    int *begin;
    int *end;
    long long int nb_prime;
} input_output_parameters;

int test_prime_slow(int n) {
    if (n <= 1) return 0;

    for (int p = 2 ; p <= n/2 ; ++p) {
        if (!(n % p)) return 0;
    }

    return 1;
}

int test_prime_fast(int n) {
    if (!(n & 1) || n < 2 ) return n == 2;

    for (int p = 3; p <= n/p; p += 2)
        if (!(n % p)) return 0;

    return 1;
}

void* count_prime(void *arg)
{
    input_output_parameters* param = (input_output_parameters*)arg;
    int nb = 0;
    for (int* i = param->begin ; i != param->end ; ++i)
        if (test_prime_slow(*i)) ++nb;

    param->nb_prime = nb;
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "%s nb-threads\n", argv[0]);
        return -1;
    }

    int nb_threads = atoi(argv[1]);

    srand(0);
    int* values = (int*)malloc(ARRAY_SIZE*sizeof(int));
    for (int i = 0 ; i < ARRAY_SIZE ; ++i)
        values[i] = rand()%100000;

    pthread_t threads[nb_threads];
    input_output_parameters threads_parameters[nb_threads];

    int *begin = values;
    int *end = values + ARRAY_SIZE;
    int block_size = ARRAY_SIZE/nb_threads;
    for (int i = 0 ; i < nb_threads ; ++i) {
        threads_parameters[i].nb_prime = 0;
        threads_parameters[i].begin = begin;
        threads_parameters[i].end = begin + block_size;
        if (i == nb_threads-1)
            threads_parameters[i].end = end;
        pthread_create(&threads[i], NULL, count_prime, &threads_parameters[i]);
        begin += block_size; 
    }

    for (int i = 0 ; i < nb_threads ; ++i)
        pthread_join(threads[i], NULL);

    int nb_value = 0;
    for (int i = 0 ; i < nb_threads ; ++i)
         nb_value += threads_parameters[i].nb_prime;

    printf("total : %d\n", nb_value);

    free(values);

    return 0;
}

