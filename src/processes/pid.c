#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGE_SIZE 4096

char v[5*PAGE_SIZE] = {1, 2, 3};
char w[2*PAGE_SIZE];

int main() {
    //int n = PAGE_SIZE * 7;
    //char *x = (char*)malloc(n);
    //for (int i = 0; i < n; ++i)
    //    x[i] = (char)i;

    printf("My pid : %ld\n", (long)getpid());

    (void)getchar();
    return 0;
}
