#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int s) {
    printf("SIGHUP here 1\n");
}

void handler2(int s) {
    printf("SIGHUP here 2\n");
}

int main(void)
{
    //signal(SIGHUP, &handler);
    //signal(SIGCONT, &handler2);

    printf("Process (PID = %d) (PGID = %d) (SID = %d)\n", getpid(), getpgrp(),
            getsid(0));
    fflush(stdout);

    if (fork() != 0) {
        sleep(1);
        //_exit(0);
       //wait(NULL);
    } else {
        while (1) {
            setpgid(0,0);
            printf("Process (PID = %d) (PGID = %d) (SID = %d)\n", getpid(),
                    getpgrp(), getsid(0));
            fflush(stdout);
            sleep(3);
            //raise(SIGSTOP);
        }
    }

    return 0;
}
