#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d, group PID = %d)\n",
                getpid(), getppid(), getpgrp());
        sleep(5);
        return 0;
    } else if (pid != -1) {
        printf("Parent process (PID = %d, parent PID = %d, group PID = %d)\n",
                getpid(), getppid(), getpgrp());
    } else {
        perror("Failed to fork");
        return 1;
    }

    wait(NULL);

    return 0;
}
