#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <string.h>

#define ARRAY_SIZE 1000000

int* threads_values;
int value = 42;

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

void* max(void *arg)
{
    thread_input* input = (thread_input*)arg;
    int nb = 0;
    for (int* i = input->begin ; i != input->end ; ++i)
        if (test_prime(*i)) ++nb;

    threads_values[input->id] = nb;
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "%s nb-threads\n", argv[0]);
        return -1;
    }

    int nb_threads = atoi(argv[1]);

    threads_values = (int*)malloc(nb_threads*sizeof(int));

    srand(0);
    int* values = (int*)malloc(ARRAY_SIZE*sizeof(int));
    for (int i = 0 ; i < ARRAY_SIZE ; ++i)
        values[i] = rand()%100000;

    pthread_t threads[nb_threads];
    thread_input thread_inputs[nb_threads];

    int *begin = values;
    int *end = values + ARRAY_SIZE;
    int block_size = ARRAY_SIZE/nb_threads;
    for (int i = 0 ; i < nb_threads ; ++i) {
        thread_inputs[i].id = i;
        thread_inputs[i].begin = begin;
        thread_inputs[i].end = begin + block_size;
        if (thread_inputs[i].end > end)
            thread_inputs[i].end = end;
        pthread_create(&threads[i], NULL, max, &thread_inputs[i]);
        begin += block_size; 
    }

    for (int i = 0 ; i < nb_threads ; ++i)
        pthread_join(threads[i], NULL);

    int nb_value = 0;
    for (int i = 0 ; i < nb_threads ; ++i)
         nb_value += threads_values[i];

    printf("total : %d\n", nb_value);

    free(threads_values);

    return 0;
}

