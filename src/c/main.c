#include <stdio.h>
#include "circle.h"

#define ARRAY_SIZE 10

static void print_hello() {
    printf("Hello, world !\n");
}

int main(int argc, char **argv) {
    int v[ARRAY_SIZE];
    print_hello();
    printf("%f \n", circle_area(2.0));
    return 0;
}
