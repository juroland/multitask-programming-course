#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{

    int fd = open("/dev/tty", O_RDWR);

    printf("Process (PID = %d, parent PID = %d, group PID = %d, session ID = %d, foreground group PID = %d)\n", getpid(), getppid(), getpgrp(), getsid(0), tcgetpgrp(fd));

    pause();

    return 0;
}
