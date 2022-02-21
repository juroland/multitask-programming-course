#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <string.h>

#define ARRAY_SIZE 1000

int test_prime_slow(int n) {
    if (n <= 1) return 0;

    for (int p = 2 ; p <= n/2 ; ++p) {
        if (!(n % p)) return 0;
    }

    return 1;
}

int count(int* values)
{
    int nb = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i)
        if (test_prime_slow(values[i])) ++nb;

    return nb;
}

int main()
{
    srand(0);
    int* values = (int*)malloc(ARRAY_SIZE*sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; ++i)
        values[i] = rand() % 100000;

    printf("total : %d\n", count(values));

    free(values);

    return 0;
}

