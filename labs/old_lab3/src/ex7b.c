#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void print_ids() 
{
    printf("Process (PID = %d, parent PID = %d, group PID = %d, session ID = %d, foreground group PID = %d)\n", getpid(), getppid(), getpgrp(), getsid(0), tcgetpgrp(0)); 
}

int main(void)
{
    print_ids();

    pid_t pid;
    if ((pid = fork()) == 0) {
       print_ids();
       setsid();
       while (1) {
           sleep(3);
           print_ids();
       }
    } else if(pid != -1) {
        wait(NULL);
    } else {
        perror("create child process");
        return 1;
    }

    pause();

    return 0;
}