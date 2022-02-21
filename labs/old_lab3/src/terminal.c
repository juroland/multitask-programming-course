#include <unistd.h>
#include <stdio.h>

int main(void) {
    sleep(5);
    printf("START\n");

    char buffer[100];
    ssize_t nb_read = 0;
    while((nb_read = read(STDIN_FILENO, buffer, 100)) > 0)
    {
        write(STDOUT_FILENO, buffer, nb_read);
        sleep(1);
        printf("---\n");
    }

    return 0;
}
