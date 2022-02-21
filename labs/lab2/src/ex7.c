#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(void)
{
    int openFlags = O_CREAT | O_WRONLY | O_TRUNC | O_APPEND;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    pid_t pid = fork();

    // usleep(1);

    if (pid == 0) {
        usleep(3);
        int fd = open("out", openFlags, filePerms);
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(), getppid());
        for (char c = '0'; c <= '9'; ++c)
        {
            write(fd, &c, 1);
            usleep(2);
        }
    } else if (pid != -1) {
        int fd = open("out", openFlags, filePerms);
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
        for (char c = 'a'; c <= 'z'; ++c)
        {
            write(fd, &c, 1);
            usleep(1);
        }
    } else {
        perror("Failed to fork");
        return 1;
    }


    return 0;
}
