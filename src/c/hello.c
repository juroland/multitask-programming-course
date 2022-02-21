#include <stdio.h>

#define ARRAY_SIZE 10
#define PI 3.1415926536

static void print_hello() {
    printf("Hello, world !\n");
}

double circle_area(double r) {
    return PI * r * r; 
}

int main(int argc, char **argv) {
    int v[ARRAY_SIZE];
    print_hello();
    printf("%f \n", circle_area(2.0));
    return 0;
}
