#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int s) {
    printf("SIGHUP here\n");
}

int main()
{
    signal(SIGHUP, &handler);

    printf("Process (PID = %d) (PGID = %d) (SID = %d)\n", getpid(), getpgrp(), getsid(0));

    if ( fork() ) {
        sleep(1);
    } else {
        while (1) {
            printf("Process (PID = %d) (PGID = %d) (SID = %d)\n", getpid(), getpgrp(), getsid(0));
            //pause();
            raise(SIGSTOP);
        }
    }

    return 0;
}
