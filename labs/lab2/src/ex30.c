#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
         sleep(1);
}

int main(void)
{
    signal(SIGCHLD, &handle);

    // Create 100 zombie processes
    for (int i = 0 ; i < 100 ; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            sleep(10);
            return 0;
        } else if (pid == -1) {
            perror("create a child process");
            return 1;
        }
    }

    for (;;) pause();

    return 0;
}
