#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
    sleep(5);
    printf("START\n");
    char buffer[100];
    ssize_t nb_read = 0;
    int fd = open("test.txt", O_RDWR);
    while((nb_read = read(fd, buffer, 2)) > 0)
    {
        printf("loop\n");
        write(STDOUT_FILENO, buffer, nb_read);
        sleep(1);
    }

    return 0;
}
