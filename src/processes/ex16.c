#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{
    //int fd = open("test.txt", O_RDWR);
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open("test.txt", O_RDWR);
        for (char i = '0' ; i <= '9' ; ++i) {
            write(fd, &i, sizeof(char));
            usleep(1);
        }
    } else if (pid != -1) {
        int fd = open("test.txt", O_RDWR);
        //wait(NULL);
        for (char c = 'a' ; c <= 'z' ; ++c) {
            write(fd, &c, sizeof(char));
            usleep(1);
        }
    } else {
        perror("Failed to fork");
        return 1;
    }
    return 0;
}
